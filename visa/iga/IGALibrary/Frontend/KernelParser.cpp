/*========================== begin_copyright_notice ============================

Copyright (c) 2017-2021 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

============================= end_copyright_notice ===========================*/

#include "BufferedLexer.hpp"
#include "Floats.hpp"
#include "KernelParser.hpp"
#include "Lexemes.hpp"
#include "Parser.hpp"
#include "../IR/InstBuilder.hpp"
#include "../IR/Types.hpp"
#include "../strings.hpp"

#include <algorithm>
#include <limits>
#include <unordered_map>
#include <string>
#include <vector>


using namespace iga;

static const IdentMap<FlagModifier> FLAGMODS {
    {"lt", FlagModifier::LT},
    {"le", FlagModifier::LE},
    {"gt", FlagModifier::GT},
    {"ge", FlagModifier::GE},
    {"eq", FlagModifier::EQ},
    {"ne", FlagModifier::NE},
    {"ov", FlagModifier::OV},
    {"un", FlagModifier::UN},
    {"eo", FlagModifier::EO},
    // aliased by different operations
    {"ze", FlagModifier::EQ},
    {"nz", FlagModifier::NE},
};
static const IdentMap<FlagModifier> FLAGMODS_LEGACY {
    {"l", FlagModifier::LT},
    {"g", FlagModifier::GT},
    {"e", FlagModifier::EQ},
    {"o", FlagModifier::OV},
    {"u", FlagModifier::UN},
    {"z", FlagModifier::EQ},
};
static const IdentMap<Type> SRC_TYPES {
    {"b",  Type::B},
    {"ub", Type::UB},
    {"w",  Type::W},
    {"uw", Type::UW},
    {"d",  Type::D},
    {"ud", Type::UD},
    {"q",  Type::Q},
    {"uq", Type::UQ},
    {"hf", Type::HF},
    {"f",  Type::F},
    {"df", Type::DF},
    {"v",  Type::V},
    {"uv", Type::UV},
    {"vf", Type::VF},
    {"nf", Type::NF},
    ///////////////////////////////////////////
    // non-standard names
    {"u8",  Type::UB},
    {"u16", Type::UW},
    {"u32", Type::UD},
    {"u64", Type::UQ},
    {"s8",  Type::B},
    {"s16", Type::W},
    {"s32", Type::D},
    {"s64", Type::Q},
    {"f16", Type::HF},
    {"f32", Type::F},
    {"f64", Type::DF},
};
static const IdentMap<Type> DST_TYPES {
    {"b",  Type::B},
    {"ub", Type::UB},
    {"w",  Type::W},
    {"uw", Type::UW},
    {"d",  Type::D},
    {"ud", Type::UD},
    {"q",  Type::Q},
    {"uq", Type::UQ},
    //
    {"hf", Type::HF},
    {"f",  Type::F},
    {"df", Type::DF},
    {"nf", Type::NF},
    ///////////////////////////////////////////
    // non-standard names
    {"u8", Type::UB},
    {"u16", Type::UW},
    {"u32", Type::UD},
    {"u64", Type::UQ},
    {"s8", Type::B},
    {"s16", Type::W},
    {"s32", Type::D},
    {"s64", Type::Q},
    {"f16", Type::HF},
    {"f32", Type::F},
    {"f64", Type::DF},
};
static const IdentMap<MathMacroExt> MATHMACROREGS = {
    {"mme0", MathMacroExt::MME0},
    {"mme1", MathMacroExt::MME1},
    {"mme2", MathMacroExt::MME2},
    {"mme3", MathMacroExt::MME3},
    {"mme4", MathMacroExt::MME4},
    {"mme5", MathMacroExt::MME5},
    {"mme6", MathMacroExt::MME6},
    {"mme7", MathMacroExt::MME7},
    {"nomme", MathMacroExt::NOMME},
};
static const IdentMap<MathMacroExt> MATHMACROREGS_OLDSTYLE = {
    {"acc2", MathMacroExt::MME0},
    {"acc3", MathMacroExt::MME1},
    {"acc4", MathMacroExt::MME2},
    {"acc5", MathMacroExt::MME3},
    {"acc6", MathMacroExt::MME4},
    {"acc7", MathMacroExt::MME5},
    {"acc8", MathMacroExt::MME6},
    {"acc9", MathMacroExt::MME7},
    {"noacc", MathMacroExt::NOMME},
};


GenParser::GenParser(
    const Model &model,
    InstBuilder &handler,
    const std::string &inp,
    ErrorHandler &eh,
    const ParseOpts &pots)
    : Parser(inp,eh)
    , m_model(model)
    , m_builder(handler)
    , m_opts(pots)
{
    initSymbolMaps();
}
// ExecInfo = '(' ExecSize EmOffNm? ')'
//   where EmOffNm = '|' EmOff  (',' 'NM')?
//                 | '|' 'NM'
//         EmOff = 'M0' | 'M4' | ...
//         ExecSize = '1' | '2' | ... | '32'
void GenParser::ParseExecInfo(
    ExecSize dftExecSize,
    ExecSize &execSize,
    ChannelOffset &chOff)
{
    Loc execSizeLoc = m_execSizeLoc = NextLoc(0);
    Loc execOffsetLoc = NextLoc(0);
    // we are careful here since we might have things like:
    //    jmpi        (1*16)
    //    jmpi (1|M0) ...
    //    jmpi (1)    ...
    // We resolve that by looking ahead two symbols
    int execSizeVal = 1;
    if (LookingAt(LPAREN) && (LookingAtFrom(2,RPAREN) || LookingAtFrom(2,PIPE))) {
        Skip();
        execSizeLoc = NextLoc();
        ConsumeIntLitOrFail(execSizeVal, "expected SIMD width");

        if (Consume(PIPE)) {
            static const IdentMap<ChannelOffset> EM_OFFS {
                  {"M0", ChannelOffset::M0}
                , {"M4", ChannelOffset::M4}
                , {"M8", ChannelOffset::M8}
                , {"M12", ChannelOffset::M12}
                , {"M16", ChannelOffset::M16}
                , {"M20", ChannelOffset::M20}
                , {"M24", ChannelOffset::M24}
                , {"M28", ChannelOffset::M28}
            };
            execOffsetLoc = NextLoc();
            ConsumeIdentOneOfOrFail(
                EM_OFFS,
                chOff,
                "expected ChOff",
                "invalid ChOff");
            //if (m_chOff % m_execSize != 0) {
            //    Fail(execOffsetLoc,
            //        "invalid execution mask offset for execution size");
            //} else if (m_chOff + m_execSize > 32) {
            //    Fail(execOffsetLoc,
            //        "invalid execution mask offset for execution size");
            //}
        } else {
            chOff = ChannelOffset::M0;
        }
        ConsumeOrFail(RPAREN,"expected )");
    } else {
        if (m_opSpec && m_opSpec->hasImpicitEm()) {
            chOff = ChannelOffset::M0;
            execSizeVal = 1;
        } else if (m_opts.supportLegacyDirectives) {
            chOff = ChannelOffset::M0;
            execSizeVal = (int)dftExecSize;
        } else {
            FailT("expected '(' (start of execution size info)");
        }
    }

    switch (execSizeVal) {
    case 1: execSize = ExecSize::SIMD1; break;
    case 2: execSize = ExecSize::SIMD2; break;
    case 4: execSize = ExecSize::SIMD4; break;
    case 8: execSize = ExecSize::SIMD8; break;
    case 16: execSize = ExecSize::SIMD16; break;
    case 32: execSize = ExecSize::SIMD32; break;
    default: FailT("invalid SIMD width");
    }

    m_builder.InstExecInfo(
        execSizeLoc, execSize, execOffsetLoc, chOff);
}

Type GenParser::SendOperandDefaultType(int srcIx) const {
    auto t = srcIx == 1 ? Type::INVALID : Type::UD;
    if (srcIx < 0) {
        if (m_opSpec->hasImplicitDstType())
            t = m_opSpec->implicitDstType();
    } else {
        if (m_opSpec->hasImplicitSrcType(srcIx, false))
            t = m_opSpec->implicitSrcType(srcIx, false);
    }
    return t;
}

Type GenParser::ParseSendOperandTypeWithDefault(int srcIx) {
    // sends's second parameter doesn't have a valid type
    Type t = Type::INVALID;
    if (Consume(COLON)) {
        if (!LookingAt(IDENT)) {
            FailT("expected a send operand type");
        }
        if (!IdentLookupFrom(0, DST_TYPES, t)) {
            FailT("unexpected operand type for send");
        }
        Skip();
    } else {
        t = SendOperandDefaultType(srcIx);
    }
    return t;
}

bool GenParser::LookupReg(
    const std::string &str,
    const RegInfo*& ri,
    int& reg)
{
    ri = nullptr;
    reg = 0;

    // given something like "r13", parse the "r"
    // "cr0" -> "cr"
    size_t len = 0;
    while (len < str.length() && !isdigit(str[len]))
        len++;
    if (len == 0)
        return false;
    auto itr = m_regmap.find(str.substr(0,len));
    if (itr == m_regmap.end()) {
        return false;
    }
    ri = itr->second;
    reg = 0;
    if (ri->numRegs > 0) {
        // if it's a numbered register like "r13" or "cr1", then
        // parse the number part
        size_t off = len;
        while (off < str.size() && isdigit(str[off])) {
            char c = str[off++];
            reg = 10*reg + c - '0';
        }
        if (off < str.size()) {
            // we have something like "r13xyz"; we don't treat this
            // as a register, but fallback so it can be treated as an
            // identifier (an immediate reference)
            reg = 0;
            ri = nullptr;
            return false;
        }
    } // else it was something like "null" or "ip"
      // either way, we are done
    return true;
}

bool GenParser::PeekReg(const RegInfo*& regInfo, int& regNum) {
    const Token &tk = Next();
    if (tk.lexeme != IDENT) {
        return false;
    } else if (LookupReg(GetTokenAsString(tk), regInfo, regNum)) {
        // helpful translation that permits acc2-acc9 and translates them
        // to mme0-7 with a warning (or whatever they map to on the given
        // platform
        if (regInfo->regName == RegName::ARF_ACC &&
            regNum >= regInfo->numRegs)
        {
            const RegInfo *mme =
                m_model.lookupRegInfoByRegName(RegName::ARF_MME);
            IGA_ASSERT(mme != nullptr, "unable to find MMR on this platform");
            if (mme) {
                // switch acc to mme
                regInfo = mme;
                // adjust the reg num (e.g. acc2 -> mme0 on GEN8)
                regNum -= mme->regNumBase;
                WarningAtT(tk.loc,
                    "old-style access to mme via acc "
                    "(use mme", regNum,
                    " for acc", regNum + mme->regNumBase, ")");
            }
        }
        if (!regInfo->isRegNumberValid(regNum)) {
            WarningS(tk.loc, "register number out of bounds");
        }
        return true;
    } else {
        return false;
    }
}

bool GenParser::ConsumeReg(const RegInfo*& regInfo, int& regNum) {
    const Token &tk = Next();
    if (tk.lexeme != IDENT) {
        return false;
    }
    if (PeekReg(regInfo, regNum)) {
        Skip();
        return true;
    } else {
        return false;
    }
}

static bool isFloating(const ImmVal &v) {
    switch (v.kind) {
    case ImmVal::Kind::F16:
    case ImmVal::Kind::F32:
    case ImmVal::Kind::F64:
        return true;
    default:
        return false;
    }
}
static bool isSignedInt(const ImmVal &v) {
    switch (v.kind) {
    case ImmVal::Kind::S8:
    case ImmVal::Kind::S16:
    case ImmVal::Kind::S32:
    case ImmVal::Kind::S64:
        return true;
    default:
        return false;
    }
}
static bool isUnsignedInt(const ImmVal &v) {
    switch (v.kind) {
    case ImmVal::Kind::U8:
    case ImmVal::Kind::U16:
    case ImmVal::Kind::U32:
    case ImmVal::Kind::U64:
        return true;
    default:
        return false;
    }
}
static bool isIntegral(const ImmVal &v) {
    return isSignedInt(v) || isUnsignedInt(v);
}


// expression parsing
// &,|
// <<,>>
// +,-
// *,/,%
// -(unary neg)
bool GenParser::TryParseConstExpr(ImmVal &v, int srcOpIx) {
    if (parseBitwiseExpr(false, v)) {
        if (srcOpIx >= 0) {
            m_srcKinds[srcOpIx] = Operand::Kind::IMMEDIATE;
        }
        return true;
    }
    return false;
}
bool GenParser::TryParseIntConstExpr(ImmVal &v, const char *forWhat) {
    Loc loc = NextLoc();
    bool z = TryParseConstExpr(v);
    if (!z) {
        return false;
    } else if (!isIntegral(v)) {
        std::stringstream ss;
        if (forWhat) {
            ss << forWhat << " must be a constant integer expression";
        } else {
            ss << "expected constant integer expression";
        }
        FailS(loc,ss.str());
    }
    return true;
}

void GenParser::ensureIntegral(const Token &t, const ImmVal &v) {
    if (!isIntegral(v)) {
        FailS(t.loc, "argument to operator must be integral");
    }
}
void GenParser::checkNumTypes(
    const ImmVal &v1, const Token &op, const ImmVal &v2)
{
    if (isFloating(v1) && !isFloating(v2)) {
        FailAtT(op.loc, "right operand to ", GetTokenAsString(op),
            " must be floating point");
    } else if (isFloating(v2) && !isFloating(v1)) {
        FailAtT(op.loc, "left operand to ", GetTokenAsString(op),
            " must be floating point");
    }
}
// target must be float
void GenParser::checkIntTypes(
    const ImmVal &v1, const Token &op, const ImmVal &v2)
{
    if (isFloating(v1)) {
        FailAtT(op.loc, "left operand to ", GetTokenAsString(op),
            " must be integral");
    } else if (isFloating(v2)) {
        FailAtT(op.loc, "right operand to ", GetTokenAsString(op),
            " must be integral");
    }
}

ImmVal GenParser::evalBinExpr(
    const ImmVal &v1, const Token &op, const ImmVal &v2)
{
    bool isF = isFloating(v1) || isFloating(v2);
    bool isU = isUnsignedInt(v1) || isUnsignedInt(v2);
    ImmVal result = v1;
    switch (op.lexeme) {
    case AMP:
    case CIRC:
    case PIPE:
    case LSH:
    case RSH:
    case MOD:
        // integral only operations
        checkIntTypes(v1, op, v2);
        switch (op.lexeme) {
        case AMP:
            if (isU) {
                result.u64 &= v2.u64;
            } else {
                result.s64 &= v2.s64;
            }
            break;
        case CIRC:
            if (isU) {
                result.u64 ^= v2.u64;
            } else {
                result.s64 ^= v2.s64;
            }
            break;
        case PIPE:
            if (isU) {
                result.u64 |= v2.u64;
            } else {
                result.s64 |= v2.s64;
            }
            break;
        case LSH:
            if (isU) {
                result.u64 <<= v2.u64;
            } else {
                result.s64 <<= v2.s64;
            }
            break;
        case RSH:
            if (isU) {
                result.u64 >>= v2.u64;
            } else {
                result.s64 >>= v2.s64;
            }
            break;
        case MOD:
            if (isU) {
                result.u64 %= v2.u64;
            } else {
                result.s64 %= v2.s64;
            }
            break;
        default:
            break;
        }
        break;
    case ADD:
    case SUB:
    case MUL:
    case DIV:
        checkNumTypes(v1, op, v2);
        switch (op.lexeme) {
        case ADD:
            if (isF) {
                result.f64 += v2.f64;
            } else if (isU) {
                result.u64 += v2.u64;
            } else {
                result.s64 += v2.s64;
            }
            break;
        case SUB:
            if (isF) {
                result.f64 -= v2.f64;
            } else if (isU) {
                result.u64 -= v2.u64;
            } else {
                result.s64 -= v2.s64;
            }
            break;
        case MUL:
            if (isF) {
                result.f64 *= v2.f64;
            } else if (isU) {
                result.u64 *= v2.u64;
            } else {
                result.s64 *= v2.s64;
            }
            break;
        case DIV:
            if (isF) {
                result.f64 /= v2.f64;
            } else {
                if (v2.u64 == 0) {
                    FailS(op.loc, "(integral) division by zero");
                }
                if (isU) {
                    result.u64 /= v2.u64;
                } else {
                    result.s64 /= v2.s64;
                }
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return result;
}


// E -> E (('&'|'|') E)*
bool GenParser::parseBitwiseExpr(bool consumed, ImmVal &v) {
    if (!parseShiftExpr(consumed,v)) {
        return false;
    }
    while (LookingAtAnyOf(AMP, PIPE)) {
        Token t = Next(); Skip();
        ImmVal r;
        parseBitwiseExpr(true, r);
        v = evalBinExpr(v, t, r);
    }
    return true;
}
// E -> E (('<<'|'>>') E)*
bool GenParser::parseShiftExpr(bool consumed, ImmVal &v) {
    if (!parseAddExpr(consumed, v)) {
        return false;
    }
    while (LookingAtAnyOf(LSH, RSH)) {
        Token t = Next(); Skip();
        ImmVal r;
        parseAddExpr(true, r);
        v = evalBinExpr(v, t, r);
    }
    return true;
}
// E -> E (('+'|'-') E)*
bool GenParser::parseAddExpr(bool consumed, ImmVal &v) {
    if (!parseMulExpr(consumed, v)) {
        return false;
    }
    while (LookingAtAnyOf(ADD, SUB)) {
        Token t = Next(); Skip();
        ImmVal r;
        parseMulExpr(true, r);
        v = evalBinExpr(v, t, r);
    }
    return true;
}

// E -> E (('*'|'/'|'%') E)*
bool GenParser::parseMulExpr(bool consumed, ImmVal &v) {
    if (!parseUnExpr(consumed, v)) {
        return false;
    }
    while (LookingAtAnyOf({MUL, DIV, MOD})) {
        Token t = Next(); Skip();
        ImmVal r;
        parseUnExpr(true, r);
        v = evalBinExpr(v, t, r);
    }
    return true;
}

// E -> ('-'|'~') E
bool GenParser::parseUnExpr(bool consumed, ImmVal &v) {
    if (!LookingAtAnyOf(SUB, TILDE)) {
        if (!parsePrimary(consumed, v)) {
            return false;
        }
    } else {
        Token t = Next(); Skip();
        parsePrimary(true, v);
        switch (t.lexeme) {
        case SUB:
            v.Negate();
            break;
        case TILDE:
            ensureIntegral(t, v);
            v.u64 = ~v.u64;
            break;
        default:
            break;
        }
    }
    return true;
}

// special symbol (e.g. nan, inf, ...)
// grouped expression (E)
// literal
bool GenParser::parsePrimary(bool consumed, ImmVal &v) {
    Token t = Next();
    bool isQuietNaN = false;
    if (LookingAtIdentEq(t, "nan")) {
        WarningT("nan is deprecated, us snan(...) or qnan(...)");
        v.kind = ImmVal::Kind::F64;
        v.f64 = std::numeric_limits<double>::signaling_NaN();
        Skip();
    } else if ((isQuietNaN = LookingAtIdentEq(t, "qnan")) ||
        LookingAtIdentEq(t, "snan"))
    {
        auto nanSymLoc = NextLoc();
        Skip();
        if (Consume(LPAREN)) {
            auto payloadLoc = NextLoc();
            ImmVal payload;
            payload.u64 = 0;
            parseBitwiseExpr(true, payload);
            if (payload.u64 >= F64_QNAN_BIT) {
                FailS(payloadLoc, "NaN payload overflows");
            } else if (payload.u64 == 0 && !isQuietNaN) {
                // signaling NaN has a 0 in the high mantissa, so we must
                // ensure that at least one bit in the mantissa is set
                // otherwise the entire mantissa is 0's and the pattern
                // would be an "infinity" pattern, not a NaN
                FailS(payloadLoc, "NaN payload must be nonzero for snan");
            }
            ConsumeOrFail(RPAREN, "expected )");
            v.u64 = payload.u64 | F64_EXP_MASK;
            if (isQuietNaN) {
                v.u64 |= F64_QNAN_BIT;
            }
        } else {
            WarningS(nanSymLoc,
                "bare qnan and snan tokens deprecated"
                " (pass in a valid payload)");
            v.u64 = F64_EXP_MASK;
            if (isQuietNaN) {
                v.u64 |= F64_QNAN_BIT; // the quiet bit suffices
            } else {
                v.u64 |= 1; // set something other than the quiet bit
                            // non-zero in the payload
            }
        }
        v.kind = ImmVal::Kind::F64;
    } else if (LookingAtIdentEq(t, "inf")) {
        v.f64 = std::numeric_limits<double>::infinity();
        v.kind = ImmVal::Kind::F64;
        Skip();
    } else if (LookingAt(FLTLIT)) {
        ParseFltFrom(t.loc, v.f64);
        v.kind = ImmVal::Kind::F64;
        Skip();
    } else if (LookingAtAnyOf({INTLIT02, INTLIT10, INTLIT16})) {
        // we parse as unsigned, but tag as signed for negation etc...
        ParseIntFrom<uint64_t>(t.loc, v.u64);
        v.kind = ImmVal::Kind::S64;
        Skip();
    } else if (Consume(LPAREN)) {
        // (E)
        parseBitwiseExpr(true, v);
        Consume(RPAREN);
    } else if (LookingAt(IDENT)) {
        // TEST CASES
        // LABEL:
        // // jmpi  LABEL                    // passes
        // // goto (16) (2 + LABEL) LABEL    // fails
        // // goto (16) LABEL LABEL          // passes
        // // join (16) LABEL                // passes
        // // mov (1) r65:ud LABEL:ud        // fails
        // // mov (1) r65:ud (2 + LABEL):ud  // fails (poor diagnostic)
        if (m_opSpec && (m_opSpec->isBranching() || m_opSpec->is(Op::MOV))) {
            if (consumed) {
                //   jmpi (LABEL + 2)
                //         ^^^^^ already consumed LPAREN
                FailT("branching operands may not perform arithmetic on labels");
            } else {
                // e.g. jmpi  LABEL64
                //            ^^^^^^
                // This backs out so caller can cleanly treat this as
                // a branch label cleanly
                return false;
            }
        } else {
            // non branching op
            if (!consumed) {
                // e.g. mov (1) r13:ud   SYMBOL
                //                       ^^^^^^
                // we fail here since we don't know if we should treat SYMBOL
                // as relative or absolute
                FailT("non-branching operations may not reference symbols");
            } else {
                // end of a term where FOLLOW contains IDENT
                //   X + Y*Z  IDENT
                //            ^
                // this allows caller to back off and accept
                //  X + Y*Z as the total expression with lookahead IDENT
                return false;
                // FIXME: mov (1) r65:ud (LABEL + 2):ud
                //   Bad diagnostic "expected source type"
                // Either we cannot allow IDENT in any const expr's
                // follow set, or we must track more state through the
                // expression parse...
                // Maybe pass a bool, canFail around.
                //
                // NOTE: we could also keep a list of backpatches and
                // apply it after the parse.  But this would require
                // building a full expression tree and walking it after
                // all labels have been seen.
            }
        }
    } else {
        // something else: error unless we haven't consumed anything
        if (consumed) {
            FailT("syntax error in constant expression");
        }
        return false;
    }
    return true;
} // parsePrimary


void GenParser::initSymbolMaps()
{
    // map the register names
    // this maps just the non-number part.
    // e.g. with cr0, this maps "cr"; see LookupReg()
    int tableLen;
    const RegInfo *table = GetRegisterSpecificationTable(tableLen);
    for (int i = 0; i < tableLen; i++) {
        const RegInfo *ri = table + i;
        if (ri->supportedOn(platform())) {
            m_regmap[ri->syntax] = ri;
        }
    }
}


class KernelParser : GenParser
{
    // maps mnemonics and registers for faster lookup
    std::unordered_map<std::string,const OpSpec*>   opmap;

    ExecSize              m_defaultExecutionSize;
    Type                  m_defaultRegisterType;

    // instruction state
    bool                  m_hasWrEn;
    Type                  m_unifType;
    const Token          *m_unifTypeTk;
    RegRef                m_flagReg;
    ExecSize              m_execSize;
    ChannelOffset         m_chOff;

    Loc                   m_mnemonicLoc;
    Loc                   m_srcLocs[3];
    int                   m_sendSrcLens[2]; // send message lengths
    Loc                   m_sendSrcLenLocs[2]; // locations so we can referee
public:
    KernelParser(
        const Model &model,
        InstBuilder &handler,
        const std::string &inp,
        ErrorHandler &eh,
        const ParseOpts &pots)
        : GenParser(model, handler, inp, eh, pots)
        , m_defaultExecutionSize(ExecSize::SIMD1)
        , m_defaultRegisterType(Type::INVALID)
    {
        initSymbolMaps();
    }

    void ParseListing() {
        ParseProgram();
    }

    void initSymbolMaps() {
        // map mnemonics names to their ops
        // subops only get mapped by their fully qualified names in this pass
        for (const OpSpec *os : m_model.ops()) {
            if (os->isValid()) {
                opmap[os->mnemonic] = os;
            }
        }
    }


    // Program = (Label? Insts* (Label Insts))?
    void ParseProgram() {
        m_builder.ProgramStart();

        // parse default type directives
        if (m_opts.supportLegacyDirectives) {
            // e.g. .default_execution_size...
            ParseLegacyDirectives();
        }

        // first block doesn't need a label
        if (!LookingAtLabelDef() && !EndOfFile()) {
            ParseBlock(NextLoc(),""); // unnamed
        }
        // successive blocks need a label
        std::string label;
        Loc lblLoc = NextLoc();
        while (ConsumeLabelDef(label)) {
            ParseBlock(lblLoc, label);
            lblLoc = NextLoc();
        }
        if (!EndOfFile()) {
            FailT("expected instruction, block, or EOF");
        }

        m_builder.ProgramEnd();
    }

    // fix to support .default_execution_size
    bool ParseLegacyDirectives() {
        int parsed = 0;
        try {
            while (LookingAtSeq(Lexeme::DOT, Lexeme::IDENT)) {
                Skip();
                parsed++;
                if (ConsumeIdentEq("default_execution_size")) {
                    Consume(LPAREN);
                    auto loc = NextLoc();
                    int dftExecSize;
                    if (!ConsumeIntLit<int>(dftExecSize)) {
                        FailT("expected SIMD width (integral value)");
                    }
                    if (dftExecSize !=  1 &&
                        dftExecSize !=  2 &&
                        dftExecSize !=  4 &&
                        dftExecSize !=  8 &&
                        dftExecSize != 16 &&
                        dftExecSize != 32)
                    {
                        FailS(loc, "invalid default execution size; "
                            "must be 1, 2, 4, 8, 16, 32");
                    }
                    m_defaultExecutionSize = (ExecSize)dftExecSize;
                    Consume(RPAREN);
                    Consume(NEWLINE);
                } else if (ConsumeIdentEq("default_register_type")) {
                    m_defaultRegisterType = TryParseOpType(DST_TYPES);
                    if (m_defaultRegisterType == Type::INVALID) {
                        FailT("expected default register type");
                    }
                    Consume(NEWLINE);
                } else {
                    FailT("unexpected directive name");
                }
            } // while
        } catch (const SyntaxError &s) {
            RecoverFromSyntaxError(s);
            // returning true will make ParseBlock restart the loop
            // (so more directives get another shot)
            return true;
        }
        return parsed > 0;
    }

    void RecoverFromSyntaxError(const SyntaxError &s) {
        // record the error in this instruction
        m_errorHandler.reportError(s.loc, s.message);
        // bail if we've reached the max number of errors
        if (m_errorHandler.getErrors().size() >=
            m_opts.maxSyntaxErrors)
        {
            throw s;
        }
        // attempt resync by skipping until we pass a newline
        // DumpLookaheads();
        while (!Consume(Lexeme::NEWLINE) &&
            // !Consume(Lexeme::SEMI) &&
            //   don't sync on ; since this is typically part of a
            //   region rather than an operand separator
            //  ...     r12.0<8;8,1>:d
            //  ^ error        ^ syncs here otherwise
            !LookingAt(Lexeme::END_OF_FILE))
        {
            // DumpLookaheads();
            Skip();
        }
    }


    void ParseBlock(const Loc &lblLoc, const std::string &label) {
        m_builder.BlockStart(lblLoc, label);
        auto lastInst = NextLoc();
        while (true) {
            if (Consume(Lexeme::NEWLINE) || Consume(Lexeme::SEMI)) {
                continue;
            } else if (m_opts.supportLegacyDirectives &&
                ParseLegacyDirectives())
            {
                // .default_execution_size...
                continue;
            }
            // if we make it here, then we are not looking at a legacy
            // directive and we are not looking at a newline
            if (LookingAtLabelDef() || EndOfFile()) {
                break;
            }
            // not at EOF and not looking at the beginning of new block
            // so we better be looking at an instruction
            ParseInst();
            lastInst = NextLoc(-1); // peek at the end of the last instruction
        }

        m_builder.BlockEnd(ExtentTo(lblLoc,lastInst));
    }


    void ParseInst() {
        try {
            // parse one instruction
            // DumpLookaheads();
            ParseInstCanThrowException();
        } catch (SyntaxError &s) {
            // we resync here and return from ParseInst
            RecoverFromSyntaxError(s);
        }
    }


    // Instruction = RegularInst | LdStInst
    // RegularInst = Predication? Mnemonic UniformType? EMask
    //                 ConditionModifier? Operands InstOptions?
    // LdStInst = ... comes from Sends/Interface.hpp ...
    void ParseInstCanThrowException() {
        // ShowCurrentLexicalContext();
        const Loc startLoc = NextLoc();
        m_builder.InstStart(startLoc);

        m_flagReg = REGREF_INVALID;
        m_opSpec = nullptr;
        m_unifType = Type::INVALID;
        m_unifTypeTk = nullptr;
        m_mnemonicLoc = Loc::INVALID;
        for (size_t i = 0; i < sizeof(m_srcKinds)/sizeof(m_srcKinds[0]); i++) {
            m_srcLocs[i] = Loc::INVALID;
            m_srcKinds[i] = Operand::Kind::INVALID;
        }
        for (size_t i = 0;
            i < sizeof(m_sendSrcLens)/sizeof(m_sendSrcLens[0]);
            i++)
        {
            m_sendSrcLens[i] = -1;
            m_sendSrcLenLocs[i] = Loc::INVALID;
        }

        // (W&~f0) mov (8|M0) r1 r2
        // ^
        ParseWrEnPred();

        // (W&~f0) mov (8|M0) r1 r2
        //         ^
        //
        //         math.sqrt (8|M0) ...
        //         ^
        m_mnemonicLoc = NextLoc();
        m_opSpec = ParseMnemonic();
        if (m_opSpec) {
            // looking at a regular instruction (non special-ld-st inst)
            m_builder.InstOp(m_opSpec);
            FinishNonLdStInstBody();
        } else
        {
            FailS(m_mnemonicLoc, "invalid mnemonic");
        }

        // .... {...}
        ParseInstOpts();

        if (!LookingAt(Lexeme::NEWLINE) &&
            !LookingAt(Lexeme::SEMI) &&
            !LookingAt(Lexeme::END_OF_FILE))
        {
            FailAfterPrev("expected newline, ';', or EOF");
        }

        m_builder.InstEnd(ExtentToPrevEnd(startLoc));
    }


    // e.g.  add (8|M8) ...
    //           ^
    // e.g.  nop ...
    //           ^
    void FinishNonLdStInstBody() {
        // if (Consume(COLON)) {
        //    m_unifTypeTk = &Next(0);
        //    ConsumeIdentOneOfOrFail(
        //        dstTypes,
        //        m_unifType,
        //        "expected uniform type",
        //        "invalid uniform type");
        // }

        // (W&~f0) mov (8|M0) (le)f0.0  r1:f  r2:f
        //             ^
        ChannelOffset chOff;
        ParseExecInfo(m_defaultExecutionSize, m_execSize, chOff); // sets m_execSize

        ParseFlagModOpt();
        switch (m_opSpec->format) {
        case OpSpec::NULLARY:
            // nop ...
            // illegal ...
            break; // fallthrough to instruction options
        case OpSpec::BASIC_UNARY_REG:
        case OpSpec::BASIC_UNARY_REGIMM:
            ParseDstOp();
            ParseSrcOp(0);
            if (m_opSpec->format == OpSpec::BASIC_UNARY_REG &&
                m_srcKinds[0] != Operand::Kind::DIRECT &&
                m_srcKinds[0] != Operand::Kind::INDIRECT)
            {
                FailS(m_srcLocs[0], "src0 must be a register");
            }
            break;
        case OpSpec::SYNC_UNARY:
            // implicit destination
            ParseSrcOp(0);
            if (m_model.supportsWait() &&
                m_srcKinds[0] != Operand::Kind::DIRECT)
            {
                FailS(m_srcLocs[0], "src0 must be a notification register");
            }
            break;
        case OpSpec::SEND_UNARY:
            // <=Gen11
            ParseSendDstOp();
            ParseSendSrcOp(0, false);
            ParseSendDescsLegacy();
            break;
        case OpSpec::SEND_BINARY:
            if (platform() <= Platform::XE) {
                ParseSendInstructionLegacy();
            } else {
                IGA_ASSERT_FALSE("invalid format for platform");
            }
            break;
        case OpSpec::BASIC_BINARY_REG_IMM:
        case OpSpec::BASIC_BINARY_REG_REG:
        case OpSpec::BASIC_BINARY_REG_REGIMM:
            ParseDstOp();
            ParseSrcOp(0);
            ParseSrcOp(1);
            break;
        case OpSpec::MATH_BINARY_REG_REGIMM:
            ParseDstOp();
            ParseSrcOp(0);
            if (m_opSpec->getSourceCount(m_builder.getSubfunction()) > 1) {
                // math sometimes has only one operand
                ParseSrcOp(1);
            }
            break;
        case OpSpec::TERNARY_REGIMM_REG_REGIMM:
            ParseDstOp();
            ParseSrcOp(0);
            ParseSrcOp(1);
            ParseSrcOp(2);
            break;
        case OpSpec::JUMP_UNARY_IMM:
            ParseSrcOpLabel(0);
            break;
        case OpSpec::JUMP_UNARY_REGIMM:
            // e.g. jmpi or brd  (registers or labels)
            ParseSrcOp(0);
            break;
        case OpSpec::JUMP_UNARY_REG:
            // e.g. ret (..) r12
            ParseSrcOp(0);
            if (m_srcKinds[0] != Operand::Kind::DIRECT &&
                m_srcKinds[0] != Operand::Kind::INDIRECT)
            {
                FailS(m_srcLocs[0], "src0 must be a register");
            }
            break;
        case OpSpec::JUMP_BINARY_BRC: {
            //   brc (...) lbl lbl
            //   brc (...) reg null
            //
            // for legacy reasons we support a unary form
            //   brc (...) reg
            // we do add an explicit null parameter for src1
            // it's very possible there are some simulator tests floating
            // aroudn that require this old form.
            auto brcStart = NextLoc();
            ParseSrcOp(0);
            if (m_srcKinds[0] == Operand::Kind::IMMEDIATE ||
                m_srcKinds[0] == Operand::Kind::LABEL ||
                LookingAtIdentEq("null"))
            {
                ParseSrcOp(1);
            } else {
                // legacy format
                // add an explicit null operand
                //
                // trb: this is no longer a warning, but we still set the
                // parameter in the IR for the time being
                //
                // if (m_parseOpts.deprecatedSyntaxWarnings) {
                //    Warning(brcStart,
                //        "deprecated syntax: register version "
                //        "of brc takes an explicit null parameter");
                // }
                // m_handler.InstSrcOpRegDirect(
                //     1,
                //     brcStart,
                //     SrcModifier::NONE,
                //     RegName::ARF_NULL,
                //     REGREF_ZERO_ZERO,
                //     Region::SRC010,
                //     Type::UD);
            }
            break;
        }
        case OpSpec::JUMP_UNARY_CALL_REGIMM:
            // call (..) dst src
            // calla (..) dst src
            ParseDstOp();
            ParseSrcOp(0);
            break;
        case OpSpec::JUMP_BINARY_IMM_IMM:
            // e.g. else, cont, break, goto, halt
            ParseSrcOpLabel(0);
            ParseSrcOpLabel(1);
            break;
        default:
            IGA_ASSERT_FALSE("unhandled syntax class in parser");
        }
    }


    // Original binary send <=XE (sends, sendsc)
    void ParseSendInstructionLegacy() {
        ParseSendDstOp();
        ParseSendSrcOp(0, false);
        ParseSendSrcOp(1,
            m_model.supportsUnarySend() &&
            m_opts.supportLegacyDirectives);
        ParseSendDescsLegacy();
    }




    // Predication = ('(' WrEnPred ')')?
    //   where WrEnPred = WrEn
    //                  | Pred
    //                  | WrEn '&' Pred
    //         WrEn = 'W'
    //         Pred = '~'? FlagReg ('.' PreCtrl?)
    void ParseWrEnPred() {
        m_hasWrEn = false;
        if (Consume(LPAREN)) {
            const Loc nmLoc = NextLoc(0);
            if (ConsumeIdentEq("W")) {
                m_hasWrEn = true;
                m_builder.InstNoMask(nmLoc);
                if (Consume(AMP)) {
                    ParsePred();
                }
            } else {
                ParsePred();
            }
            ConsumeOrFail(RPAREN, "expected )");
        }
    }


    // Pred = '~'? FlagReg ('.' PreCtrl?)
    void ParsePred() {
        static const IdentMap<PredCtrl> PREDCTRLS {
            {"xyzw", PredCtrl::SEQ}, // lack of a specific pred control defaults to SEQ
            {"anyv", PredCtrl::ANYV},
            {"allv", PredCtrl::ALLV},
            {"any2h", PredCtrl::ANY2H},
            {"all2h", PredCtrl::ALL2H},
            {"any4h", PredCtrl::ANY4H},
            {"all4h", PredCtrl::ALL4H},
            {"any8h", PredCtrl::ANY8H},
            {"all8h", PredCtrl::ALL8H},
            {"any16h", PredCtrl::ANY16H},
            {"all16h", PredCtrl::ALL16H},
            {"any32h", PredCtrl::ANY32H},
            {"all32h", PredCtrl::ALL32H},
        };

        const Loc prLoc = NextLoc(0);
        bool predInv = Consume(TILDE);
        ParseFlagRegRef(m_flagReg);
        PredCtrl predCtrl = PredCtrl::NONE;
        if (Consume(DOT)) {
            ConsumeIdentOneOfOrFail(
                PREDCTRLS,
                predCtrl,
                "expected predication control",
                "invalid predication control");
        } else {
            predCtrl = PredCtrl::SEQ;
        }
        m_builder.InstPredication(prLoc, predInv, m_flagReg, predCtrl);
    }


    const OpSpec *TryConsumeMmenonic() {
        const Token &tk = Next();
        if (tk.lexeme != IDENT) {
            return nullptr;
        }
        const char *p = &m_lexer.GetSource()[tk.loc.offset];
        std::string s;
        s.reserve((size_t)tk.loc.extent + 1);
        for (size_t i = 0; i < tk.loc.extent; i++) {
            s += *p++;
        }
        auto itr = opmap.find(s);
        if (itr == opmap.end()) {
            return nullptr;
        } else {
            Skip();
            return itr->second;
        }
    }

#if 0
    // returns the number of characters off
    size_t similarityR(
        size_t mismatches, size_t shortestStrLen,
        const std::string &left, size_t li,
        const std::string &right, size_t ri)
    {
        if (mismatches >= shortestStrLen) { // everything mismatches
            return shortestStrLen;
        } else if (li == left.size() - 1) { // the rest of right mismatches
            return mismatches + right.size() - ri;
        } else if (ri == right.size() - 1) { // the rest of left mismatches
            return mismatches + left.size() - li;
        } else {
            if (left[li] != right[ri]) {
                mismatches++;
            }
            // 1. delete both
            auto db = similarityR(
                mismatches, shortestStrLen,
                left, li + 1,
                right, ri + 1);
            // 2. delete left
            auto dr = similarityR(
                mismatches, shortestStrLen,
                left, li,
                right, ri + 1);
            // 3. delete right
            auto dl = similarityR(
                mismatches, shortestStrLen,
                left, li + 1,
                right, ri);
            return std::min<size_t>(db, std::min<size_t>(dr, dl));
        }
    }
    // roughly the number of characters
    float similarity(const std::string &left, const std::string &right) {
        if (left.size() > 8 || right.size() > 8)
            return 0;
        auto shortestLen = std::min<size_t>(left.size(), right.size());
        size_t minEdits = similarityR(0, shortestLen, left, 0, right, 0);
        return 1.0f - (float)minEdits/(float)shortestLen;
    }
#endif
    void failWithUnexpectedSubfunction(
        const Loc &loc,
        const std::string &)
    {
        FailAtT(loc, "unexpected subfunction for op");
#if 0
        std::vector<std::pair<float,const char *>> matches;
        for (int i = (int)m_opSpec->op + 1;
            i < (int)m_opSpec->op + m_opSpec->subopsLength;
            i++)
        {
            const OpSpec &sf = m_model.lookupOpSpec((iga::Op)i);
            if (sf.isValid()) {
                auto sim = similarity(sfIdent, sf.mnemonic.str());
                std::cout << sf.mnemonic << ": " << sim << "\n";
                if (sim >= 0.66f) {
                    matches.emplace_back(sim, sf.mnemonic.str());
                }
            }
        }
        std::sort(matches.begin(), matches.end(),
            std::greater<std::pair<float,const char *>>());
        if (!matches.empty()) {
            ss << "; did you mean ";
            if (matches.size() > 2) {
                ss << "one of: ";
            }
            // only show the top five
            size_t len = std::min<size_t>(5, matches.size());
            for (size_t i = 0; i < len; i++) {
                if (i > 0 && i == len - 1) {
                    if (len > 2) {
                        ss << ", or ";
                    } else {
                        ss << " or ";
                    }
                } else {
                    ss << ", ";
                }
                ss << matches[i].second;
            }
        }
#endif
    }



    //
    // Mnemoninc
    //     = Ident SubMnemonic?
    //     | Ident BrCtl
    //   SubMnemoninc
    //     = '.' SfIdent
    //     | '.' HEX_INT | '.' DEC_INT
    //
    //   SfIdent =
    //     | BrnchCtl
    //     | SFID
    //     | SyncFc
    //     | ... other subfunction identifier
    const OpSpec *ParseMnemonic() {
        const Loc mnemonicLoc = NextLoc();
        const OpSpec *pOs = TryConsumeMmenonic();
        if (!pOs) {
            return nullptr;
        }
        m_builder.InstOp(pOs);
        // GED will reject this otherwise
        if (!m_hasWrEn && pOs->op == Op::JMPI) {
            WarningAtT(mnemonicLoc,
                "jmpi must have (W) specified (automatically adding)");
            m_builder.InstNoMask(mnemonicLoc);
        }

        // TODO: abstract this all into a something that's contained in
        // the IGA bxml enums files.
        //   e.g. use OpSpec::supportsSubfunction()
        //        bool Subfunction::FromString(Op, Subfunction&)
        if (pOs->supportsBranchCtrl()) {
            BranchCntrl brctl = BranchCntrl::OFF;
            if (Consume(DOT)) {
                if (!ConsumeIdentEq("b")) {
                    FailT("expected 'b' (branch control)");
                }
                brctl = BranchCntrl::ON;
            }
            m_builder.InstSubfunction(brctl);
        } else if (pOs->is(Op::MATH)) {
            // e.g. math.*, send.*, etc...
            m_builder.InstSubfunction(ParseSubfunctionFromBxmlEnum<MathFC>());
        } else if (pOs->is(Op::SYNC)) {
            m_builder.InstSubfunction(ParseSubfunctionFromBxmlEnum<SyncFC>());
        } else if (pOs->isOneOf(Op::SEND, Op::SENDC)) {
            if (platform() >= Platform::XE)
                m_builder.InstSubfunction(
                    ParseSubfunctionFromBxmlEnum<SFID>());
            // else: it's part of ExDesc
        } else {
            // isOldSend: pre XE will have a subfunction,
            // but we pull it from ExDesc
            bool isOldSend =
                platform() < Platform::XE || pOs->isSendOrSendsFamily();
            IGA_ASSERT(!pOs->supportsSubfunction() || isOldSend,
                "INTERNAL ERROR: subfunction expected");
        }

        if (LookingAt(DOT)) {
            // maybe an old condition modifier or saturation
            FlagModifier fm;
            if (LookingAtIdentEq(1, "sat")) {
                FailT("saturation flag prefixes destination operand: "
                    "e.g. op (..) (sat)dst ...");
            } else if (
                IdentLookupFrom(1, FLAGMODS, fm) ||
                IdentLookupFrom(1, FLAGMODS_LEGACY, fm))
            {
                FailT("conditional modifier follows execution mask "
                    "info: e.g. op (16|M0)  (le)f0.0 ...");
            } else {
                // didn't match flag modifier
                FailT("unexpected . (expected execution size)");
            }
        }

        return pOs;
    }

    template <typename T>
    T ParseSubfunctionFromBxmlEnum() {
        if (!Consume(DOT)) {
            FailAfterPrev("expected operation subfunction");
        }

        auto sfLoc = NextLoc();
        if (LookingAt(IDENT)) {
            auto loc = NextLoc();
            const std::string sfIdent = GetTokenAsString();
            Skip();
            auto x = FromSyntax<T>(sfIdent);
            if (x == T::INVALID) {
                FailAtT(loc, "invalid subfunction");
            }
            return x;
        } else if (LookingAtAnyOf(INTLIT10, INTLIT16)) {
            uint32_t val;
            (void)ConsumeIntLit(val);
            return (T)val;
        }
        FailAtT(sfLoc, "invalid subfunction");
        return (T)-1;
    }


    // FlagModifierOpt = '(' FlagModif ')' FlagReg
    void ParseFlagModOpt() {
        Loc loc = NextLoc();
        if (Consume(LBRACK)) {
            // try the full form [(le)f0.0]
            FlagModifier flagMod;
            if (!TryParseFlagModFlag(flagMod)) {
                FailT("expected flag modifier function");
            }
            ParseFlagModFlagReg();
            ConsumeOrFail(RBRACK, "expected ]");
            if (m_opts.deprecatedSyntaxWarnings) {
                WarningAtT(
                    loc,
                    "deprecated flag modifier syntax (omit the brackets)");
            }
            m_builder.InstFlagModifier(m_flagReg, flagMod);
        } else {
            // try the short form
            // (le)f0.0
            FlagModifier flagMod;
            if (!TryParseFlagModFlag(flagMod)) {
                // soft fail (we might be looking at "(sat)r0.0")
                return;
            }
            ParseFlagModFlagReg();
            m_builder.InstFlagModifier(m_flagReg, flagMod);
        }
    }
    // e.g. "(le)"
    bool TryParseFlagModFlag(FlagModifier &flagMod) {
        if (!LookingAt(LPAREN)) {
            return false;
        }
        if (!IdentLookupFrom(1, FLAGMODS, flagMod)) {
            Loc loc = NextLoc(1);
            if (!IdentLookupFrom(1, FLAGMODS_LEGACY, flagMod)) {
                return false;
            } else if (m_opts.deprecatedSyntaxWarnings) {
                // deprecated syntax
                WarningAtT(loc,
                    "deprecated flag modifier syntax: "
                    "use ", ToSyntax(flagMod), " for this function");
            }
        }
        Skip(2);
        ConsumeOrFail(RPAREN, "expected )");
        return true;
    }
    // e.g. "f0.1"
    void ParseFlagModFlagReg() {
        const Loc flregLoc = NextLoc();
        RegRef fr;
        ParseFlagRegRef(fr);
        if (m_flagReg.regNum != REGREF_INVALID.regNum &&
            (m_flagReg.regNum != fr.regNum ||
                m_flagReg.subRegNum != fr.subRegNum))
        {
            FailAtT(flregLoc,
                "flag register must be same for predication "
                "and flag modifier");
        }
        m_flagReg = fr;
    }


    // Examples:
    //   r13.4<2>:hf
    //   (sat)r13.0<1>:f
    //   r[a0.3,16]<1>:ud
    void ParseDstOp() {
        // We want to track the beginning of the "operand", not just the register.
        // That includes the saturate flag prefix(sat) on the dst.
        // That is we need:
        // (sat)r13.3:ud
        // ^    ^
        // | | not here
        // |
        // | here
        const Loc opStart = NextLoc(0);
        // ParseSatOpt increments m_offset.
        if (ParseSatOpt()) {
            m_builder.InstDstOpSaturate();
        }
        // Note that, if there is SAT token, opStart != regStart.
        // This is because m_offset changed.
        const Loc regStart = NextLoc(0);
        if (ConsumeIdentEq("r")) {
            ParseDstOpRegInd(opStart, 0);
        } else {
            const RegInfo *regInfo;
            int regNum;
            if (!ConsumeReg(regInfo, regNum)) {
                FailT("invalid destination register");
            }
            if (regInfo && !regInfo->isRegNumberValid(regNum)) {
                FailT("invalid destination register number "
                    "(", regInfo->syntax, " only has ", regInfo->numRegs,
                    " registers on this platform)");
            }
            if (LookingAt(LBRACK)) {
                ParseDstOpRegInd(opStart, regNum * 32);
            } else {
                assert(regInfo != nullptr);
                FinishDstOpRegDirSubRegRgnTy(
                    opStart, regStart, *regInfo, regNum);
            }
        }
    }


    // r13
    // null
    // r13:w (non-standard)
    void ParseSendDstOp() {
        const Loc regStart = NextLoc(0);
        if (ConsumeIdentEq("r")) {
            ParseDstOpRegInd(regStart, 0);
        } else {
            const RegInfo *ri = NULL;
            int regNum = 0;
            if (!ConsumeReg(ri, regNum)) {
                FailT("invalid send destination register");
            }
            if (!ri->isRegNumberValid(regNum)) {
                FailT("invalid destination register number "
                    "(", ri->syntax, " only has ", ri->numRegs,
                    " registers on this platform)");
            }
            if (LookingAt(LBRACK)) {
                FailT("this form of indirect (r3[a0.0,16]) is invalid for "
                    "send dst operand; use regular form: r[a0.0,16]");
            } else {
                FinishDstOpRegDirSubRegRgnTy(regStart, regStart, *ri, regNum);
            }
        }
    }


    // (sat)
    bool ParseSatOpt() {
        // TODO: expand to tokens so (sat) can be imm expr
        return Consume(SAT);
    }


    // .mme2 or .nomme
    MathMacroExt ParseMathMacroReg() {
        auto loc = NextLoc();
        MathMacroExt mme = MathMacroExt::INVALID;
        const char *EXPECTED =
            "expected math macro register "
            "(e.g. .mme0, ..., .mme7, or .nomme)";
        ConsumeOrFail(DOT, EXPECTED);
        if (ConsumeIdentOneOf<MathMacroExt>(MATHMACROREGS, mme)) {
            return mme;
        }
        // determine if we support the old-style
        bool supportOldStyleAcc2ToAcc7 = true;
        supportOldStyleAcc2ToAcc7 = platform() < Platform::XE;

        if (supportOldStyleAcc2ToAcc7) {
            if (ConsumeIdentOneOf<MathMacroExt>(MATHMACROREGS_OLDSTYLE, mme)) {
                // warn?
                WarningS(loc, "old-style math macro register (use mme)");
                return mme;
            }
        }

        // favor the new error message
        FailS(loc, EXPECTED);
        return mme;
    }


    // REG ('.' INT)? DstRgn (':' DstTy)?
    //   where DstRgn = '<' INT '>'
    //
    // E.g. r13.4<2>:t
    void FinishDstOpRegDirSubRegRgnTy(
        const Loc &opStart,
        const Loc &regnameLoc,
        const RegInfo &ri,
        int regNum)
    {
        // subregister or implicit accumulator operand
        // e.g. .4 or .acc3
        Loc subregLoc = NextLoc(0);
        int subregNum = 0;

        // <1>
        Region::Horz rgnHz = Region::Horz::HZ_1;

        MathMacroExt mmeReg = MathMacroExt::INVALID;
        if (m_opSpec->isSendOrSendsFamily() && Consume(DOT)) {
            ConsumeIntLitOrFail(subregNum, "expected subregister");
            // whine about them using a subregister on a send operand
            if (m_opts.deprecatedSyntaxWarnings) {
                WarningAtT(subregLoc,
                    "send operand subregisters have no effect"
                    " and are deprecated syntax");
            }
            if (m_opSpec->isSendOrSendsFamily() && LookingAt(LANGLE)) {
                if (m_opts.deprecatedSyntaxWarnings) {
                    // whine about them using a region
                    WarningT("send operand region has no effect and is"
                        " deprecated syntax");
                }
            }
            rgnHz = ParseDstOpRegion();
        } else if (m_builder.isMacroOp()) {
            // implicit accumulator operand
            mmeReg = ParseMathMacroReg();
        } else {
            // non-send with subregister.
            // regular subregister
            if (Consume(DOT)) {
                ConsumeIntLitOrFail(subregNum, "expected subregister");
            } else {
                // implicit subregister
                //  E.g. r12:d  ...
                subregNum = 0;
            }
        }

        // <1>
        rgnHz = ParseDstOpRegion();

        // :t
        Type dty = Type::INVALID;
        if (m_opSpec->isSendOrSendsFamily()) {
            // special handling for send types
            dty = ParseSendOperandTypeWithDefault(-1);
        } else {
            dty = ParseDstOpTypeWithDefault();
        }

        // ensure the subregister is not out of bounds
        if (dty != Type::INVALID) {
            int typeSize = TypeSizeInBits(dty)/8;
            if (!ri.isSubRegByteOffsetValid(
                regNum, subregNum * typeSize, m_model.getGRFByteSize())) {
                if (ri.regName == RegName::GRF_R) {
                    ErrorAtT(subregLoc,
                        "subregister out of bounds for data type ",
                        ToSyntax(dty));
                } else {
                    // Print warning for non-GRF register, in case for those
                    // valid but strange case e.g. null0.20:f
                    WarningAtT(subregLoc,
                        "subregister out of bounds for data type");
                }
            } else if (typeSize < ri.accGran) {
                WarningAtT(regnameLoc,
                    "access granularity too small for data type");
            }
        }

        if (m_builder.isMacroOp()) {
            m_builder.InstDstOpRegMathMacroExtReg(
                opStart, ri.regName, regNum, mmeReg, rgnHz, dty);
        } else {
            RegRef reg(regNum, subregNum);
            m_builder.InstDstOpRegDirect(
                opStart, ri.regName, reg, rgnHz, dty);
        }
    }


    // e.g. [a0.4,  16]
    //  or  [a0.4 + 16]
    //  or  [a0.4 - 16]
    void ParseIndOpArgs(RegRef &addrRegRef, int &addrOff) {
        ConsumeOrFail(LBRACK, "expected [");
        if (!ParseAddrRegRefOpt(addrRegRef)) {
            FailT("expected address subregister");
        }
        Loc addrOffLoc = NextLoc();
        if (Consume(COMMA)) {
            bool neg = Consume(SUB);
            ConsumeIntLitOrFail(addrOff, "expected indirect address offset");
            if (neg)
                addrOff *= -1;
        } else if (Consume(ADD)) {
            ConsumeIntLitOrFail(addrOff, "expected indirect address offset");
        } else if (Consume(SUB)) {
            ConsumeIntLitOrFail(addrOff, "expected indirect address offset");
            addrOff *= -1;
        } else {
            addrOff = 0;
        }

        // check if the imm offset out of bound
        int addroffMax = 511;
        int addroffMin = -512;
        if (addrOff < addroffMin || addrOff > addroffMax)
        {
            FailAtT(addrOffLoc,
                "immediate offset is out of range; must be in "
                "[", addroffMin, ",", std::to_string(addroffMax), "]");
        }

        ConsumeOrFail(RBRACK, "expected ]");
    }


    // '[' 'a0' '.' INT (',' INT)? ']' ('<' INT '>')?)? TY?
    // e.g [a0.3,16]<1>:t
    void ParseDstOpRegInd(const Loc &opStart, int baseAddr) {
        // [a0.4,16]
        int addrOff;
        RegRef addrRegRef;
        ParseIndOpArgs(addrRegRef, addrOff);
        addrOff += baseAddr;
        //
        // <1>
        Region::Horz rgnHz = ParseDstOpRegion();
        //
        // :t
        Type dty = ParseDstOpTypeWithDefault();
        m_builder.InstDstOpRegIndirect(
            opStart, addrRegRef, addrOff, rgnHz, dty);
    }


    // '<' INT '>'
    Region::Horz ParseDstOpRegion() {
        if (!LookingAt(LANGLE)) {
            if (m_opSpec->hasImplicitDstRegion(m_builder.isMacroOp())) {
                return m_opSpec->implicitDstRegion(m_builder.isMacroOp()).getHz();
            } else {
                return Region::Horz::HZ_1;
            }
        }

        Region::Horz rgnHz = Region::Horz::HZ_1;
        if (Consume(LANGLE)) {
            const Loc loc = NextLoc();
            int rgnHzInt;
            ConsumeIntLitOrFail(rgnHzInt, "destination region argument");
            switch (rgnHzInt) {
            case 1: rgnHz = Region::Horz::HZ_1; break;
            case 2: rgnHz = Region::Horz::HZ_2; break;
            case 4: rgnHz = Region::Horz::HZ_4; break;
            default:
                FailS(loc, "invalid destination region");
            }
            ConsumeOrFail(RANGLE,"expected >");
        }

        return rgnHz;
    }


    // E.g. 3 in "a0.3"
    bool ParseAddrRegRefOpt(RegRef& addrReg) {
        const RegInfo *ri;
        int regNum;
        if (!ConsumeReg(ri, regNum)) {
            return false;
        }
        if (ri->regName != RegName::ARF_A && regNum != 0) {
            FailT("expected a0");
        }
        if (!Consume(DOT)) {
            FailT("expected .");
        }
        addrReg.regNum = addrReg.subRegNum = 0;
        ConsumeIntLitOrFail(
            addrReg.subRegNum, "expected address register subregister");
        // TODO: range-check against RegInfo for "a"
        return true;
    }


    // same as ParseSrcOp, but semantically chekcs that it's a label
    void ParseSrcOpLabel(int srcOpIx) {
        ParseSrcOp(srcOpIx);
        if (m_srcKinds[srcOpIx] != Operand::Kind::LABEL &&
            m_srcKinds[srcOpIx] != Operand::Kind::IMMEDIATE)
        {
            FailAtT(m_srcLocs[srcOpIx],
                "src", srcOpIx, " must be an immediate label");
        }
    }


    void ParseSrcOp(int srcOpIx) {
        m_srcLocs[srcOpIx] = NextLoc();

        // TODO: sink this down to the immediate literal case specially
        // For now, we leave it here since TryParseConstExpr() will otherwise
        // fail on "-r12:f" would fail: - as unary negation then r12 fails to
        // resolve
        const Token regnameTk = Next();
        const RegInfo *regInfo;
        int regNum;

        // first try and parse as register
        m_lexer.Mark();
        bool pipeAbs = false;
        SrcModifier srcMods = ParseSrcModifierOpt(pipeAbs);
        if (ConsumeIdentEq("r")) {
            // canonical register indirect
            // r[a0.4,-32]
            m_srcKinds[srcOpIx] = Operand::Kind::INDIRECT;
            ParseSrcOpInd(srcOpIx, m_srcLocs[srcOpIx], srcMods, 0);
            // register, symbolic immediate, label, ...
            if (pipeAbs) {
                ConsumeOrFailAfterPrev(PIPE, "expected |");
            }
            if (!m_opSpec->supportsSourceModifiers() &&
                srcMods != SrcModifier::NONE)
            {
                FailS(m_srcLocs[srcOpIx], "source modifier not supported");
            }
        } else if (ConsumeReg(regInfo, regNum)) {
            // normal register access
            //   r13.3<0;1,0>
            //   acc3
            m_srcKinds[srcOpIx] = Operand::Kind::DIRECT;
            if (!m_opSpec->supportsSourceModifiers() &&
                srcMods != SrcModifier::NONE)
            {
                FailS(m_srcLocs[srcOpIx], "source modifier not supported");
            }
            FinishSrcOpRegDirSubRegRgnTy(
                srcOpIx,
                m_srcLocs[srcOpIx],
                regnameTk.loc,
                srcMods,
                *regInfo,
                regNum);
            if (pipeAbs) {
                ConsumeOrFailAfterPrev(PIPE, "expected |");
            }
        } else {
            // backtrack to before any "source modifier"
            m_lexer.Reset();
            if (pipeAbs) {
                Skip(1);
            }

            // try as constant expression
            ImmVal immVal;
            if (TryParseConstExpr(immVal, srcOpIx)) {
                // does not match labels
                m_srcKinds[srcOpIx] = Operand::Kind::IMMEDIATE;
                if (pipeAbs) {
                    immVal.Abs();
                }
                FinishSrcOpImmValue(
                    srcOpIx,
                    m_srcLocs[srcOpIx],
                    regnameTk,
                    immVal);
                if (pipeAbs) {
                    ConsumeOrFailAfterPrev(PIPE, "expected |");
                }
            } else {
                if (pipeAbs) {
                    FailS(regnameTk.loc, "unexpected |");
                }
                // failed constant expression without consuming any input
                if (LookingAt(IDENT)) {
                    // e.g. LABEL64
                    if (m_opSpec->isBranching() || m_opSpec->op == Op::MOV) {
                        m_srcKinds[srcOpIx] = Operand::Kind::LABEL;
                        std::string str = GetTokenAsString();
                        Skip(1);
                        FinishSrcOpImmLabel(
                            srcOpIx,
                            m_srcLocs[srcOpIx],
                            regnameTk.loc,
                            str);
                    } else {
                        // okay, we're out of ideas now
                        FailT("unbound identifier");
                    }
                } else {
                    // the token is not in the FIRST(srcop)
                    FailT("expected source operand");
                }
            }
        }
    }


    void ParseSrcOpInd(
        int srcOpIx,
        const Loc &opStart,
        const SrcModifier &srcMods,
        int baseOff)
    {
        m_srcKinds[srcOpIx] = Operand::Kind::INDIRECT;

        // [a0.4,16]
        int addrOff;
        RegRef addrRegRef;
        ParseIndOpArgs(addrRegRef, addrOff);
        addrOff += baseOff;

        // regioning... <V;W,H> or <V,H>
        Region rgn = ParseSrcOpRegionInd(srcOpIx);

        // :t
        Type sty = ParseSrcOpTypeWithDefault(srcOpIx, true);

        m_builder.InstSrcOpRegIndirect(
            srcOpIx, opStart, srcMods, addrRegRef, addrOff, rgn, sty);
    }


    // REG ('.' INT)? SrcRgn? (':' SrcTy)?
    //    ^ HERE
    //
    // E.g. r13.4<2>:f
    // E.g. r13.4<0;1,0>:f
    // E.g. r13.4<8;8,1>:f
    // E.g. r13.acc2:f
    // E.g. ce:ud
    // E.g. ip:ud
    // E.g. a0.1
    void FinishSrcOpRegDirSubRegRgnTy(
        int srcOpIx,
        const Loc &opStart,
        const Loc &regnameLoc,
        const SrcModifier &srcMod,
        const RegInfo &ri,
        int regNum)
    {
        // subregister or implicit accumulator operand
        // e.g. .4 or .acc3
        int subregNum;
        Region rgn;
        Loc subregLoc = NextLoc(1);
        MathMacroExt mme = MathMacroExt::INVALID;
        bool hasExplicitSubreg = false;
        if (m_builder.isMacroOp()) {
            // implicit accumulator operand
            // r13.acc2:f
            subregNum = 0;
            mme = ParseMathMacroReg();
            // region is implicitly <1;1,0>
            rgn = Region::SRC110;
            // below we can override it if we really really want to
        } else {
            // regular src with subregister
            // r13.4....
            //    ^
            if (Consume(DOT)) {
                ConsumeIntLitOrFail(subregNum, "expected subregister");
                hasExplicitSubreg = true;
            } else { // default to 0
                subregLoc = NextLoc(0);
                subregNum = 0;
            }
        }

        // for ternary ops <V;H> or <H>
        // for other regions <V;W,H>
        if (m_opSpec->isTernary()) {
            if (srcOpIx < 2) {
                rgn = ParseSrcOpRegionVH(srcOpIx, hasExplicitSubreg);
            } else {
                rgn = ParseSrcOpRegionH(srcOpIx, hasExplicitSubreg);
            }
        } else {
            rgn = ParseSrcOpRegionVWH(ri, srcOpIx, hasExplicitSubreg);
        }

        // :t
        Type sty = Type::INVALID;
        if (m_opSpec->isSendOrSendsFamily()) {
            sty = ParseSendOperandTypeWithDefault(srcOpIx);
        } else {
            sty = ParseSrcOpTypeWithDefault(srcOpIx, false);
        }

        if (sty != Type::INVALID) {
            // ensure the subregister is not out of bounds
            int typeSize = TypeSizeInBits(sty)/8;
            if (ri.isRegNumberValid(regNum) &&
                !ri.isSubRegByteOffsetValid(regNum, subregNum * typeSize, m_model.getGRFByteSize()))
            {
                // don't add an extra error if the parent register is
                // already out of bounds
                WarningS(subregLoc, "subregister out of bounds");
            } else if (typeSize < ri.accGran) {
                WarningS(regnameLoc, "register access granularity too small type");
            }
        }

        if (m_builder.isMacroOp()) {
            m_builder.InstSrcOpRegMathMacroExtReg(
                srcOpIx,
                opStart,
                srcMod,
                ri.regName,
                regNum,
                mme,
                rgn,
                sty);
        } else {
            RegRef reg((uint8_t)regNum, (uint8_t)subregNum);
            m_builder.InstSrcOpRegDirect(
                srcOpIx,
                opStart,
                srcMod,
                ri.regName,
                reg,
                rgn,
                sty);
        }
    }


    // '<' INT ';' INT ',' INT '>'     (VxWxH)
    Region ParseSrcOpRegionVWH(
        const RegInfo &ri, int srcOpIx, bool hasExplicitSubreg)
    {
        if (m_opSpec->hasImplicitSrcRegion(
            srcOpIx, m_execSize, m_builder.isMacroOp()))
        {
            if (!LookingAt(LANGLE)) {
                return m_opSpec->implicitSrcRegion(
                    srcOpIx, m_execSize, m_builder.isMacroOp());
            } else {
                WarningT(
                    m_opSpec->mnemonic.str(),
                    ".Src", srcOpIx, " region should be implicit");
            }
        }

        Region rgn = Region::SRC010;
        if (Consume(LANGLE)) {
            rgn.set(ParseRegionVert());
            ConsumeOrFailAfterPrev(SEMI, "expected ;");
            rgn.set(ParseRegionWidth());
            ConsumeOrFailAfterPrev(COMMA, "expected ,");
            rgn.set(ParseRegionHorz());
            ConsumeOrFailAfterPrev(RANGLE, "expected >");
        } else if (m_opSpec->isSendOrSendsFamily()) {
            rgn = defaultSendOperandRegion(ri.regName, srcOpIx);
        } else if (ri.supportsRegioning()) {
            // N.B. <1;1,0> won't coissue on PreGEN11
            rgn = hasExplicitSubreg || m_execSize == ExecSize::SIMD1 ?
                Region::SRC010 :
                Region::SRC110;
        } else {
            rgn = Region::SRC010;
        }
        return rgn;
    }

    // '<' INT ';' INT '>'   (CNL Align1 ternary)
    Region ParseSrcOpRegionVH(int srcOpIx, bool hasExplicitSubreg)
    {
        if (m_opSpec->hasImplicitSrcRegion(
            srcOpIx, m_execSize, m_builder.isMacroOp()))
        {
            if (!LookingAt(LANGLE)) {
                return m_opSpec->implicitSrcRegion(
                    srcOpIx, m_execSize, m_builder.isMacroOp());
            } else if (m_opts.deprecatedSyntaxWarnings) {
                WarningT(
                    m_opSpec->mnemonic.str(), ".Src", srcOpIx,
                    " region should be implicit");
            }
        }
        Region rgn = Region::SRC010;
        if (Consume(LANGLE)) {
            rgn.set(ParseRegionVert());
            ConsumeOrFailAfterPrev(SEMI, "expected ;");
            rgn.set(Region::Width::WI_INVALID);
            rgn.set(ParseRegionHorz());
            ConsumeOrFailAfterPrev(RANGLE, "expected >");
        } else {
            bool scalarAccess = hasExplicitSubreg || m_execSize == ExecSize::SIMD1;
            if (scalarAccess) {
                rgn = Region::SRC0X0;
            } else {
                // packed access
                rgn = platform() >= Platform::XE ?
                    Region::SRC1X0 : Region::SRC2X1; // most conservative mux
            }
        }
        return rgn;
    }


    // '<' INT '>'   (CNL Align1 ternary src2)
    Region ParseSrcOpRegionH(int srcOpIx, bool hasExplicitSubreg)
    {
        if (m_opSpec->hasImplicitSrcRegion(
            srcOpIx, m_execSize, m_builder.isMacroOp()))
        {
            if (!LookingAt(LANGLE)) {
                return m_opSpec->implicitSrcRegion(
                    srcOpIx, m_execSize, m_builder.isMacroOp());
            } else if (m_opts.deprecatedSyntaxWarnings) {
                WarningT(
                    m_opSpec->mnemonic.str(), ".Src", srcOpIx,
                    " region should be implicit");
            }
        }

        Region rgn;
        rgn.bits = 0; // needed to clear _padding
        if (Consume(LANGLE)) {
            rgn.set(
                Region::Vert::VT_INVALID,
                Region::Width::WI_INVALID,
                ParseRegionHorz());
            ConsumeOrFailAfterPrev(RANGLE, "expected >");
        } else {
            rgn = hasExplicitSubreg || m_execSize == ExecSize::SIMD1 ?
                Region::SRCXX0 :
                Region::SRCXX1;
        }
        return rgn;
    }

    Region defaultSendOperandRegion(RegName rn, int opIx) const {
        Region r = Region::INVALID;
        if (opIx < 0) {
            if (m_opSpec->hasImplicitDstRegion(m_builder.isMacroOp())) {
                r = m_opSpec->implicitDstRegion(m_builder.isMacroOp());
            }
        } else {
            if (m_opSpec->hasImplicitSrcRegion(
                opIx, m_execSize, m_builder.isMacroOp())) {
                r =  m_opSpec->implicitSrcRegion(
                    0, m_execSize, m_builder.isMacroOp());
            } else if (rn == RegName::ARF_NULL) {
                r = Region::SRC010;
            } else {
                r = Region::SRC110;
            }
        }
        return r;
    }

    // '<' INT ',' INT '>'             (VxH mode)
    // '<' INT ';' INT ',' INT '>'
    Region ParseSrcOpRegionInd(int srcOpIx) {
        if (m_opSpec->hasImplicitSrcRegion(
            srcOpIx, m_execSize, m_builder.isMacroOp()))
        {
            if (!LookingAt(LANGLE)) {
                return m_opSpec->implicitSrcRegion(
                    srcOpIx, m_execSize, m_builder.isMacroOp());
            } else if (m_opts.deprecatedSyntaxWarnings) {
                WarningT(m_opSpec->mnemonic.str(),
                    ".Src", srcOpIx, " region should be implicit");
            }
        }

        Region rgn;
        rgn.bits = 0;
        if (Consume(LANGLE)) {
            Loc arg1Loc = NextLoc();
            int arg1;
            ConsumeIntLitOrFail(arg1, "syntax error in source region");
            if (Consume(COMMA)) {
                // <W,H>
                rgn.set(Region::Vert::VT_VxH);
                switch(arg1) {
                case  1:
                case  2:
                case  4:
                case  8:
                case 16:
                    rgn.w = arg1;
                    break;
                default:
                    FailAtT(arg1Loc, "invalid region width");
                    rgn.set(Region::Width::WI_INVALID);
                }
                rgn.set(ParseRegionHorz());
            } else {
                // <V;W,H>
                ConsumeOrFailAfterPrev(SEMI, "expected ;");
                switch(arg1) {
                case  0:
                case  1:
                case  2:
                case  4:
                case  8:
                case 16:
                case 32:
                    rgn.v = arg1;
                    break;
                default:
                    FailAtT(arg1Loc, "invalid region vertical stride");
                    rgn.set(Region::Vert::VT_INVALID);
                }
                rgn.set(ParseRegionWidth());
                ConsumeOrFailAfterPrev(COMMA, "expected ,");
                rgn.set(ParseRegionHorz());
            }
            ConsumeOrFailAfterPrev(RANGLE, "expected >");
        } else {
            rgn = Region::SRC110;
        }
        return rgn;
    }


    Region::Vert ParseRegionVert() {
        Loc loc = NextLoc();
        int x;
        ConsumeIntLitOrFail(x, "syntax error in region (vertical stride)");
        Region::Vert vs;
        switch(x) {
        case  0: vs = Region::Vert::VT_0; break;
        case  1: vs = Region::Vert::VT_1; break;
        case  2: vs = Region::Vert::VT_2; break;
        case  4: vs = Region::Vert::VT_4; break;
        case  8: vs = Region::Vert::VT_8; break;
        case 16: vs = Region::Vert::VT_16; break;
        case 32: vs = Region::Vert::VT_32; break;
        default:
            FailAtT(loc, "invalid region vertical stride");
            vs = Region::Vert::VT_INVALID;
        }
        return vs;
    }


    Region::Width ParseRegionWidth() {
        Loc loc = NextLoc();
        int x;
        ConsumeIntLitOrFail(x, "syntax error in region (width)");
        Region::Width wi;
        switch(x) {
        case  1: wi = Region::Width::WI_1; break;
        case  2: wi = Region::Width::WI_2; break;
        case  4: wi = Region::Width::WI_4; break;
        case  8: wi = Region::Width::WI_8; break;
        case 16: wi = Region::Width::WI_16; break;
        default:
            FailAtT(loc, "invalid region width");
            wi = Region::Width::WI_INVALID;
        }
        return wi;
    }


    Region::Horz ParseRegionHorz() {
        Loc loc = NextLoc();
        int x;
        ConsumeIntLitOrFail(x, "syntax error in region (horizontal stride)");
        Region::Horz hz;
        switch(x) {
        case  0: hz = Region::Horz::HZ_0; break;
        case  1: hz = Region::Horz::HZ_1; break;
        case  2: hz = Region::Horz::HZ_2; break;
        case  4: hz = Region::Horz::HZ_4; break;
        default:
            FailAtT(loc, "invalid region horizontal stride");
            hz = Region::Horz::HZ_INVALID;
        }
        return hz;
    }


    void CheckLiteralBounds(
        const Loc &opStart,
        Type type,
        const ImmVal &val,
        int64_t mn,
        int64_t mx)
    {
        if (val.s64 < mn || val.s64 > mx) {
            WarningAtT(opStart,
                "literal is out of bounds for type ", ToSyntax(type));
        }
    }


    void FinishSrcOpImmValue(
        int srcOpIx,
        const Loc &opStart,
        const Token &valToken,
        ImmVal &val)
    {
        // convert to the underlying data type
        Type sty = ParseSrcOpTypeWithoutDefault(srcOpIx, true);


        switch (sty) {
        case Type::B:
        case Type::UB:
        case Type::W:
        case Type::UW:
        case Type::D:
        case Type::UD:
        case Type::Q:
        case Type::UQ:
        case Type::V:
        case Type::UV:
        case Type::VF:
            if (!val.isS64()) {
                ErrorAtT(opStart,
                    "literal must be integral for type ", ToSyntax(sty));
            }
            break;
        case Type::HF:
            if (val.isS64()) { // The value was parsed as integral
                if (valToken.lexeme == INTLIT10 && val.u64 != 0) {
                    // base10
                    // examples: 2:f or (1+2):f
                    FailAtT(opStart,
                        "immediate integer floating point literals must be "
                        "in hex or binary (e.g. 0x7F800000:f)");
                }
                // base2 or base16
                // e.g. 0x1234:hf  (preserve it)
                if (~0xFFFFull & val.u64) {
                    ErrorAtT(opStart, "hex literal too big for type");
                }
                // no copy needed, the half float is already encoded as such
            } else { // ==ImmVal::F64
                     // it's an fp64 literal, we need to narrow to fp16
                     //   e.g. "(1.0/10.0):hf"
                uint64_t DROPPED_PAYLOAD = ~((uint64_t)F16_MANT_MASK) &
                    (F64_MANT_MASK >> 1);
                if (IS_NAN(val.f64) && (val.u64 & DROPPED_PAYLOAD)) {
                    FailAtT(opStart, "NaN payload value overflows");
                }
                // uint64_t orginalValue = val.u64;
                val.u64 = ConvertDoubleToHalf(val.f64); // copy over u64 to clobber all
                val.kind = ImmVal::Kind::F16;
                // uint64_t newValue = FloatToBits(
                //    ConvertFloatToDouble(ConvertHalfToFloat(val.u16)));
                // if (orginalValue != newValue) {
                //    Warning(opStart,
                //        "precision lost in literal conversion to fp16");
                // }
            }
            break;
        case Type::F:
        case Type::DF:
            if (val.isS64()) {
                if (valToken.lexeme == INTLIT10 && val.u64 != 0) {
                    // base10
                    // examples: 2:f or (1+2):f
                    FailAtT(opStart,
                        "immediate integer floating point literals must be "
                        "in hex or binary (e.g. 0x7F800000:f)");
                }
                // base2 or base16 float bits
                // examples:
                //   0x3F000000:f (0.5)
                //   0x7F801010:f (qnan(0x01010))
                //   0xFFC00000:f (-qnan(...))
                //   0x7FC00000:f (qnan())
                //   0x7F800000:f (infinity)
                if (sty == Type::F) {
                    // preserve the bits
                    val.u32 = (uint32_t)val.s64;
                    val.kind = ImmVal::Kind::F32;
                } else {
                    // leave :df alone, bits are already set
                    val.kind = ImmVal::Kind::F64;
                }
            } else { // ==ImmVal::F64
                if (sty == Type::F) {
                    // parsed as double e.g. "3.1414:f"
                    // need to narrow to fp32
                    // any NaN payload must fit in the smaller mantissa
                    // The bits we will remove
                    //   ~((uint64_t)F32_MANT_MASK) &
                    //      (F64_MANT_MASK >> 1)
                    // the mantissa bits that will get truncated
                    uint64_t DROPPED_PAYLOAD = ~((uint64_t)F32_MANT_MASK) &
                        (F64_MANT_MASK >> 1);
                    if (IS_NAN(val.f64) && (val.u64 & DROPPED_PAYLOAD)) {
                        FailS(opStart, "NaN payload value overflows");
                    }
                    //
                    // Use a raw bitwise assignment; some compilers will clear
                    // the NaN bit by making an assignment
                    val.u64 = ConvertDoubleToFloatBits(val.f64);
                    val.kind = ImmVal::Kind::F32;
                    // the below would be wrong
                    //   val = ConvertDoubleToFloat(val.f64);
                    } // else: sty == Type::DF (nothing needed)
                }
            break;
        default:
            break;
        }

        // check literal bounds of integral values
        // literals may be signed floating in general
        switch (sty) {
        case Type::B:
            // if (val.kind == ImmVal::S64 &&
            //     ((val.u64 & 0xFFFFFFFFFFFFFF00ull) == 0xFFFFFFFFFFFFFF00ull))
            // {
            //     // negative value, but not too negative for 16 bits
            //     val.u64 &= 0xFFull;
            // }
            CheckLiteralBounds(opStart, sty, val, -128, 127);
            val.kind = ImmVal::Kind::S8;
            val.s64 = val.s8; // sign extend to a 64-bit value
            break;
        case Type::UB:
            // CheckLiteralBounds(opStart, sty, val, 0, 0xFF);
            val.kind = ImmVal::Kind::U8;
            val.u64 = val.u8; // could &= by 0xFF
            break;
        case Type::W:
            // if (val.kind == ImmVal::S64 &&
            //     ((val.u64 & 0xFFFFFFFFFFFF0000ull) == 0xFFFFFFFFFFFF0000ull))
            // {
            //     // negative value, but not too negative for 16 bits
            //    val.u64 &= 0xFFFFull;
            // }
            val.s64 = val.s16; // sign extend to a 64-bit value
            CheckLiteralBounds(opStart, sty, val, -32768, 32767);
            val.kind = ImmVal::Kind::S16;
            break;
        case Type::UW:
            // fails ~1:ub
            // CheckLiteralBounds(opStart, sty, val, 0, 0xFFFF);
            val.kind = ImmVal::Kind::U16;
            val.u64 = val.u16; // truncate to 16-bit: // could &= by 0xFFFF
            break;
        case Type::D:
            val.s64 = val.s32; // sign extend to a 64-bit value
            CheckLiteralBounds(opStart, sty, val, -2147483648ll, 2147483647ll);
            val.kind = ImmVal::Kind::S32;
            break;
        case Type::UD:
            // CheckLiteralBounds(opStart, sty, val, 0, 0xFFFFFFFF);
            // only check if input is signed??? Or reject signed input
            // if (val.kind == ImmVal::S64 &&
            //    ((val.u64 & 0xFFFFFFFF00000000ull) == 0xFFFFFFFF00000000ull))
            // {
            //     // negative value, but we can reprsent it as u32
            // }
            // val.u64 &= 0xFFFFFFFF;
            val.u64 = val.u32; // truncate top bits
            val.kind = ImmVal::Kind::U32;
            break;
        case Type::Q:
            // no conversion needed
            val.kind = ImmVal::Kind::S64;
            CheckLiteralBounds(opStart, sty, val,
                0x8000000000000000ll, 0x7FFFFFFFFFFFFFFFll);
            break;
        case Type::UQ:
            // no conversion needed
            val.kind = ImmVal::Kind::U64;
            break;
        case Type::HF:
            val.kind = ImmVal::Kind::F16;
            break;
        case Type::F:
            val.kind = ImmVal::Kind::F32;
            break;
        case Type::DF:
            val.kind = ImmVal::Kind::F64;
            break;
        case Type::UV:
        case Type::V:
        case Type::VF:
            val.kind = ImmVal::Kind::U32;
            break;
        default:
            break;
        }

        if (m_opSpec->isBranching()) {
            if (m_opSpec->isJipAbsolute()) {
                m_builder.InstSrcOpImmLabelAbsolute(
                    srcOpIx,
                    opStart,
                    val.s64,
                    sty);
            } else {
                m_builder.InstSrcOpImmLabelRelative(
                    srcOpIx,
                    opStart,
                    val.s64,
                    sty);
            }
        } else {
            m_builder.InstSrcOpImmValue(srcOpIx, opStart, val, sty);
        }
    }


    void FinishSrcOpImmLabel(
        int srcOpIx,
        const Loc &opStart,
        const Loc /* lblLoc */,
        const std::string &lbl)
    {
        Type type = ParseSrcOpTypeWithDefault(srcOpIx, true, true);
        m_builder.InstSrcOpImmLabel(srcOpIx, opStart, lbl, type);
    }


    // = '-' | '~' | '(abs)' | '-(abs)' | '~(abs)'
    // = or start of "|r3|" like
    SrcModifier ParseSrcModifierOpt(bool &pipeAbs) {
        SrcModifier srcMod = SrcModifier::NONE;
        if (Consume(SUB) || Consume(TILDE)) { // same lexeme as -
            srcMod = SrcModifier::NEG;
        }
        pipeAbs = LookingAt(PIPE);
        if (Consume(ABS) || Consume(PIPE)) {
            srcMod = srcMod == SrcModifier::NEG ?
                SrcModifier::NEG_ABS : SrcModifier::ABS;
        }
        return srcMod;
    }



    // e.g. "r13" or "r13:f"
    void ParseSendSrcOp(int srcOpIx, bool enableImplicitOperand) {
        m_srcLocs[srcOpIx] = NextLoc();

        const RegInfo *regInfo;
        int regNum;

        // For XE send now supports src1; sends goes away
        // To support some older syntax floating around
        // underneath IGA will insert a null src1 if needed
        if (enableImplicitOperand) {
            bool isSuccess = PeekReg(regInfo, regNum);
            if (!isSuccess || regInfo->regName == RegName::ARF_A) {
                m_builder.InstSrcOpRegDirect(
                    srcOpIx,
                    m_srcLocs[srcOpIx],
                    SrcModifier::NONE,
                    RegName::ARF_NULL,
                    REGREF_ZERO_ZERO,
                    Region::SRC010,
                    Type::INVALID);
                return;
            }
        }

#ifndef DISABLE_SENDx_IND_SRC_OPND
        ParseSrcOp(srcOpIx);
#else
        if (!ConsumeReg(regInfo, regNum)) {
            Fail("expected send operand");
        }
        auto dotLoc = NextLoc();
        if (Consume(DOT)) {
            int i;
            ConsumeIntLitOrFail(i, "expected subregister");
            if (m_parseOpts.deprecatedSyntaxWarnings) {
                Warning(dotLoc, "send instructions may not have subregisters");
            }
        }
        RegRef reg = {
            static_cast<uint16_t>(regNum),
            0
        };
        // gets the implicit region and warns against using explicit regioning
        Region rgn = ParseSrcOpRegionVWH(*regInfo, srcOpIx, false);
        // because we are trying to phase out source operands on send
        // instructions we handle them explicitly here
        Type sty = ParseSendOperandTypeWithDefault(srcOpIx);
        m_handler.InstSrcOpRegDirect(
            srcOpIx,
            m_srcLocs[srcOpIx],
            SrcModifier::NONE,
            *regInfo,
            reg,
            rgn,
            sty);
#endif
    }




    Type ParseDstOpTypeWithDefault() {
        if (m_opSpec->hasImplicitDstType()) {
            if (LookingAt(COLON)) {
                if (m_opts.deprecatedSyntaxWarnings)
                    WarningT("implicit type on dst should be omitted");
                // parse the type but ignore it
                ParseOpTypeWithDefault(DST_TYPES, "expected destination type");
            }
            // use the implicit type anyway
            return m_opSpec->implicitDstType();
        }
        return ParseOpTypeWithDefault(DST_TYPES, "expected destination type");
    }


    Type ParseSrcOpTypeWithDefault(
        int srcOpIx, bool immOrLbl, bool isLabel = false)
    {
        if (m_opSpec->hasImplicitSrcType(srcOpIx, immOrLbl)) {
            if (LookingAt(COLON)) {
                if (m_opts.deprecatedSyntaxWarnings)
                    WarningT(
                        "implicit type on src", srcOpIx, " should be omitted");
                // parse the type but ignore it
                ParseOpTypeWithDefault(SRC_TYPES, "expected source type");
            }
            // use the implicit type anyway
            return
                m_opSpec->implicitSrcType(srcOpIx, immOrLbl);
        } else if (m_opSpec->op == Op::MOV && immOrLbl && isLabel) {
            // support mov label without giving label's type
            return Type::UD;
        }

        return ParseOpTypeWithDefault(SRC_TYPES, "expected source type");
    }
    Type ParseSrcOpTypeWithoutDefault(int srcOpIx, bool immOrLbl) {
        if (m_opSpec->hasImplicitSrcType(srcOpIx, immOrLbl)) {
            if (LookingAt(COLON)) {
                if (m_opts.deprecatedSyntaxWarnings)
                    WarningT(
                        "implicit type on src", srcOpIx, " should be omitted");
                // parse the type but ignore it
                TryParseOpType(SRC_TYPES);
            }
            // use the implicit type anyway
            return m_opSpec->implicitSrcType(srcOpIx, immOrLbl);
        }
        Type t = TryParseOpType(SRC_TYPES);
        if (t == Type::INVALID &&
            !(m_opSpec->isBranching() &&
                !m_model.supportsSimplifiedBranches()))
        {
            FailT("expected source type");
        }
        return t;
    }
    Type ParseOpTypeWithDefault(
        const IdentMap<Type> types, const char *expected_err)
    {
        auto t = TryParseOpType(types);
        if (t == Type::INVALID) {
            if (m_defaultRegisterType != Type::INVALID) {
                t = m_defaultRegisterType;
            } else if (m_opSpec->isSendOrSendsFamily()) {
                t = Type::UD;
            } else if (m_opSpec->isBranching() &&
                m_model.supportsSimplifiedBranches())
            {
                // no more types for branching
                t = Type::UD;
            } else if (m_opSpec->is(Op::SYNC)) {
                // we allow implicit type for sync reg32 (grf or null)
                t = Type::UB;
            } else {
                FailT(expected_err);
            }
        }
        return t;
    }
    Type TryParseOpType(const IdentMap<Type> types) {
        if (!LookingAt(COLON)) {
            return Type::INVALID;
        }
        Type type = Type::INVALID;
        if (IdentLookupFrom(1, types, type)) {
            Skip(2);
        }
        return type;
    }



    //
    // (INTEXPR|AddrRegRef) (INTEXPR|AddrRegRef)
    void ParseSendDescsLegacy() {
        IGA_ASSERT(platform() <= Platform::XE,
            "wrong platform for function");


        const Loc exDescLoc = NextLoc();
        SendDesc exDesc;
        if (ParseAddrRegRefOpt(exDesc.reg)) {
            exDesc.type = SendDesc::Kind::REG32A;
            if (platform() < Platform::XE)
                m_builder.InstSubfunction(SFID::A0REG);
            // subfunc is already set as part of opcode (e.g. send.gtwy)
        } else {
            // constant integral expression
            ImmVal v;
            if (!TryParseConstExpr(v)) {
                FailT("expected extended send descriptor");
            }
            if (!v.isI64()) {
                FailAtT(exDescLoc,
                    "immediate descriptor expression must be integral");
            }
            exDesc.imm = (uint32_t)v.s64;
            exDesc.type = SendDesc::Kind::IMM;
            if (platform() >= Platform::XE && (exDesc.imm & 0xF)) {
                FailAtT(exDescLoc,
                    "ExDesc[3:0] must be 0's; SFID is expressed "
                    "as a function control value (e.g. send.dc0 ...)");
            }
            if (platform() < Platform::XE) {
                SFID sfid = sfidFromEncoding(platform(), exDesc.imm & 0xF);
                m_builder.InstSubfunction(sfid);
            }
        }

        if (LookingAt(COLON)) {
            FailT("extended message descriptor is typeless");
        }

        const Loc descLoc = NextLoc();
        SendDesc desc;
        if (ParseAddrRegRefOpt(desc.reg)) {
            desc.type = SendDesc::Kind::REG32A;
        } else {
            // constant integral expression
            ImmVal v;
            if (!TryParseConstExpr(v)) {
                FailT("expected extended send descriptor");
            }
            if (!v.isI64()) {
                FailAtT(descLoc,
                    "immediate descriptor expression must be integral");
            }
            desc.imm = (uint32_t)v.s64;
            desc.type = SendDesc::Kind::IMM;
        }

        m_builder.InstSendDescs(exDescLoc, exDesc, descLoc, desc);

        if (LookingAt(COLON)) {
            FailT("Message Descriptor is typeless");
        }
    }

    // ('{' InstOptOrDepInfo (',' IDENT)* '}')?
    // InstOptOrDepInfo = DepInfo | InstOpt
    //   InstOpt = 'AccWrEn' | 'Atomic' | ...
    //   DepInfo = 'NoDDChk' | 'NoDDClr' | ...
    void ParseInstOpts() {
        InstOptSet instOpts;
        instOpts.clear();

        if (Consume(LBRACE)) {
            if (!LookingAt(RBRACE))
                ParseInstOptOrFail(instOpts); // else: could be empty list "{}"
            while (Consume(COMMA)) {
                ParseInstOptOrFail(instOpts);
            }
            ConsumeOrFail(RBRACE, "expected }");
        }


        m_builder.InstOptsAdd(instOpts);
    }

    void ParseInstOptOrFail(InstOptSet &instOpts) {
        auto loc = NextLoc();
        if (!tryParseInstOptToken(instOpts) &&
            !tryParseInstOptDepInfo(instOpts))
        {
            FailAtT(loc, "invalid instruction option");
        }
    }

    bool tryParseInstOptToken(InstOptSet &instOpts) {
        auto loc = NextLoc();
        InstOpt newOpt = InstOpt::ACCWREN;
        if (ConsumeIdentEq("AccWrEn")) {
            newOpt = InstOpt::ACCWREN;
        } else if (ConsumeIdentEq("Atomic")) {
            if (platform() < Platform::GEN7) {
                FailAtT(loc, "Atomic mot supported on given platform");
            }
            newOpt = InstOpt::ATOMIC;
            if (instOpts.contains(InstOpt::SWITCH)) {
                FailAtT(loc, "Atomic mutually exclusive with Switch");
            } else if (instOpts.contains(InstOpt::NOPREEMPT)) {
                FailAtT(loc, "Atomic mutually exclusive with NoPreempt");
            }
        } else if (ConsumeIdentEq("Breakpoint")) {
            newOpt = InstOpt::BREAKPOINT;
        } else if (ConsumeIdentEq("Compacted")) {
            newOpt = InstOpt::COMPACTED;
            if (instOpts.contains(InstOpt::NOCOMPACT)) {
                FailAtT(loc, "Compacted mutually exclusive with "
                    "Uncompacted/NoCompact");
            }
        } else if (ConsumeIdentEq("EOT")) {
            newOpt = InstOpt::EOT;
            if (!m_opSpec->isSendOrSendsFamily()) {
                FailAtT(loc, "EOT is only allowed on send instructions");
            }
        } else if (ConsumeIdentEq("NoCompact") ||
            ConsumeIdentEq("Uncompacted"))
        {
            newOpt = InstOpt::NOCOMPACT;
            if (instOpts.contains(InstOpt::COMPACTED)) {
                FailAtT(loc, "Uncomapcted/NoCompact mutually exclusive "
                    "with Compacted");
            }
        } else if (ConsumeIdentEq("Serialize")) {
            newOpt = InstOpt::SERIALIZE;
        } else if (ConsumeIdentEq("NoMask")) {
            FailAtT(loc, "NoMask goes precedes predication as (W) for WrEn: "
                "e.g. (W) op (..) ...   or    (W&f0.0) op (..) ..");
        } else if (ConsumeIdentEq("H1")) {
            FailAtT(loc, "H1 is obsolete; use M0 in execution offset: "
                "e.g. op (16|M0) ...");
        } else if (ConsumeIdentEq("H2")) {
            FailAtT(loc, "H2 is obsolete; use M16 in execution offset: "
                "e.g. op (16|M16) ...");
        } else if (ConsumeIdentEq("Q1")) {
            FailAtT(loc, "Q1 is obsolete; use M0 in execution offset: "
                "e.g. op (8|M0) ...");
        } else if (ConsumeIdentEq("Q2")) {
            FailAtT(loc, "Q2 is obsolete; use M8 in execution offset: "
                "e.g. op (8|M8) ...");
        } else if (ConsumeIdentEq("Q3")) {
            FailAtT(loc, "Q3 is obsolete; use M16 in execution offset: "
                "e.g. op (8|M16) ...");
        } else if (ConsumeIdentEq("Q4")) {
            FailAtT(loc, "Q4 is obsolete; use M24 in execution offset: "
                "e.g. op (8|M24) ...");
        } else if (ConsumeIdentEq("N1")) {
            FailAtT(loc, "N1 is obsolete; use M0 in execution offset: "
                "e.g. op (4|M0) ...");
        } else if (ConsumeIdentEq("N2")) {
            FailAtT(loc, "N2 is obsolete; use M4 in execution offset: "
                "e.g. op (4|M4) ...");
        } else if (ConsumeIdentEq("N3")) {
            FailAtT(loc, "N3 is obsolete; use M8 in execution offset: "
                "e.g. op (4|M8) ...");
        } else if (ConsumeIdentEq("N4")) {
            FailAtT(loc, "N4 is obsolete; use M12 in execution offset: "
                "e.g. op (4|M12) ...");
        } else if (ConsumeIdentEq("N5")) {
            FailAtT(loc, "N5 is obsolete; use M16 in execution offset: "
                "e.g. op (4|M16) ...");
        } else if (ConsumeIdentEq("N6")) {
            FailAtT(loc, "N6 is obsolete; use M20 in execution offset: "
                "e.g. op (4|M20) ...");
        } else if (ConsumeIdentEq("N7")) {
            FailAtT(loc, "N7 is obsolete; use M24 in execution offset: "
                "e.g. op (4|M24) ...");
        } else if (ConsumeIdentEq("N8")) {
            FailAtT(loc, "N8 is obsolete; use M28 in execution offset: "
                "e.g. op (4|M28) ...");
        } else {
            return false;
        }

        if (!instOpts.add(newOpt)) {
            // adding the option doesn't change the set... (it's duplicate)
            FailAtT(loc, "duplicate instruction options");
        }
        return true;
    }

    bool tryParseInstOptDepInfo(InstOptSet &instOpts) {
        auto loc = NextLoc();
        if (LookingAt(IDENT)) {
            InstOpt newOpt = InstOpt::ACCWREN;
            bool isSwsbOpt = false;
            // classic instruction option that affects instruction dependency
            // scheduling etc...
            if (ConsumeIdentEq("NoDDChk")) {
                newOpt = InstOpt::NODDCHK;
                if (!m_model.supportsHwDeps()) {
                    FailAtT(loc, "NoDDChk not supported on given platform");
                }
            } else if (ConsumeIdentEq("NoDDClr")) {
                newOpt = InstOpt::NODDCLR;
                if (!m_model.supportsHwDeps()) {
                    FailAtT(loc, "NoDDClr not supported on given platform");
                }
            } else if (ConsumeIdentEq("NoPreempt")) {
                if (m_model.supportsNoPreempt()) {
                    newOpt = InstOpt::NOPREEMPT;
                } else {
                    FailAtT(loc, "NoPreempt not supported on given platform");
                }
            } else if (ConsumeIdentEq("NoSrcDepSet")) {
                if (m_model.supportNoSrcDepSet()) {
                    newOpt = InstOpt::NOSRCDEPSET;
                } else {
                    FailAtT(loc, "NoSrcDep not supported on given platform");
                }
            } else if (ConsumeIdentEq("Switch")) {
                newOpt = InstOpt::SWITCH;
                if (!m_model.supportsHwDeps()) {
                    WarningAtT(loc,
                        "ignoring unsupported instruction option {Switch}");
                }
            } else {
                return false; // unrecognized option
            }
            if (!isSwsbOpt && !instOpts.add(newOpt)) {
                // adding the option doesn't change the set... (it's a duplicate)
                FailAtT(loc, "duplicate instruction options");
            }
        } else if (Consume(DOLLAR)) {
            // sbid directive
            if (m_model.supportsHwDeps()) {
                FailAtT(loc, "software dependencies not supported on this platform");
            }
            int32_t sbid;
            ConsumeIntLitOrFail(sbid, "expected SBID token");
            // $4
            if (Consume(DOT)) {
                if (ConsumeIdentEq("dst")) {
                    // $4.dst
                    m_builder.InstDepInfoSBidDst(loc, sbid);
                } else if (ConsumeIdentEq("src")) {
                    // $4.src
                    m_builder.InstDepInfoSBidSrc(loc, sbid);
                } else {
                    FailT("invalid SBID directive expecting 'dst' or 'src'");
                }
            } else if (!LookingAtAnyOf(COMMA,RBRACE)) {
                FailT("syntax error in SBID directive"
                    " (expected '.' ',' or '}')");
            } else {
                // $4 (allocate/.set)
                m_builder.InstDepInfoSBidAlloc(loc, sbid);
            }
        } else if (Consume(AT)) {
            // min dependency distance @3
            if (m_model.supportsHwDeps()) {
                FailAtT(loc,
                    "software dependencies not supported on this platform");
            }
            int32_t dist;
            auto loc = NextLoc();
            ConsumeIntLitOrFail(dist,"expected register min distance value");
            m_builder.InstDepInfoDist(loc, SWSB::DistType::REG_DIST, dist);
        } else {
            return false;
        }
        return true;
    }

    // FlagRegRef = ('f0'|'f1'|'f2'|'f3') ('.' ('0'|'1'))?
    void ParseFlagRegRef(RegRef &freg) {
        if (!LookingAt(IDENT)) {
            FailT("expected flag register");
        }

        if (ConsumeIdentEq("f0")) {
            freg.regNum = 0;
        } else if (ConsumeIdentEq("f1")) {
            freg.regNum = 1;
        } else {
            FailT("unexpected flag register number");
        }

        if (LookingAtSeq(DOT, INTLIT10)) {
            // e.g. f0.1
            //        ^
            // protects predication's use in short predication code
            // (f0.any2h)
            Skip();
            auto srLoc = NextLoc();
            ConsumeIntLitOrFail(freg.subRegNum, "expected flag subregister");
            if (freg.subRegNum > 1) {
                ErrorAtT(srLoc, "flag sub-register out of bounds");
            }
        } else {
            // e.g. f0.any2h (treat as f0.0.any2h)
            freg.subRegNum = 0;
        }
    }


    bool ConsumeDstType(Type &dty) {
        return ConsumeIdentOneOf(DST_TYPES, dty);
    }


    bool ConsumeSrcType(Type &sty) {
        return ConsumeIdentOneOf(SRC_TYPES, sty);
    }


    // See LookingAtLabelDef for definition of a label
    bool ConsumeLabelDef(std::string &label) {
        if (LookingAtLabelDef()) {
            label = GetTokenAsString();
            (void)Skip(2);
            return true;
        }
        return false;
    }


    // Label = IDENT ':' (not followed by identifier)
    //   mov:ud (16) -- not a label
    //
    //   mov:       -- label
    //      ud (16) ...
    //   mov:       -- label
    //     (f0) ud (16) ...
    //
    //   lbl1:      -- label
    //   lbl2:
    //
    // For now if we get IDENT COLON IDENT, we resolve by:
    //  * if on same line, then it's an instruction
    //  * otherwise it's a label
    bool LookingAtLabelDef() {
        if (LookingAtSeq(IDENT,COLON)) {
            // ensure not a uniform type
            //   mov:ud (..)
            const Token &t2 = Next(2);
            if (t2.lexeme != IDENT || Next(0).loc.line != t2.loc.line) {
                return true;
            }
        }
        return false;
    }


}; // class KernelParser


///////////////////////////////////////////////////////////////////////////////
struct ParsedPayloadInfo {
    Loc regLoc = Loc::INVALID;
    int regNum = -1;
    RegName regName = RegName::INVALID;
    Loc regLenLoc = Loc::INVALID;
    int regLen = 0;
    bool regLenWasImplicit = false;
};
struct ParsedAddrOperand {
    Loc surfArgLoc = Loc::INVALID;
    SendDesc surfArg; // won't be set for flat
                         //
    Loc scaleLoc = Loc::INVALID;
    int scale = 1;
    //
    ParsedPayloadInfo payload;
    //
    Loc immOffsetLoc = Loc::INVALID;
    int immOffset = 0;
};




Kernel *iga::ParseGenKernel(
    const Model &m,
    const char *inp,
    iga::ErrorHandler &e,
    const ParseOpts &popts)
{
    Kernel *k = new Kernel(m);

    InstBuilder h(k, e);
    if (popts.swsbEncodeMode != SWSB_ENCODE_MODE::SWSBInvalidMode)
        h.setSWSBEncodingMode(popts.swsbEncodeMode);

    KernelParser p(m, h, inp, e, popts);
    try {
        p.ParseListing();
    } catch (SyntaxError) {
        // no need to handle it (error handler has already recorded the errors)
        delete k;
        return nullptr;
    }

    auto &insts = h.getInsts();
    auto blockStarts = Block::inferBlocks(
        e,
        k->getMemManager(),
        insts);
    int id = 1;
    for (auto bitr : blockStarts) {
        bitr.second->setID(id++);
        k->appendBlock(bitr.second);
    }

#if 0
    std::stringstream ss;
    ss << "PARSED BLOCKS\n\n";
    for (auto b : k->getBlockList()) {
        ss << "BLOCK[" << b->getID() << "] at pc " <<
            b->getPC() << ":\n";
        ss << "  targeted by {";
        int totalTargets = 0;
        for (auto b2 : k->getBlockList()) {
            for (auto i2 : b2->getInstList()) {
                for (unsigned srcIx = 0; srcIx < i2->getSourceCount(); srcIx++) {
                    const iga::Operand &src = i2->getSource(srcIx);
                    if (src.getTargetBlock() == b) {
                        if (totalTargets++ > 0)
                          ss << ", ";
                        ss << "." << i2->getPC();
                        break;
                    }
                }
            }
        }
        ss << "}\n";
        for (auto i : b->getInstList()) {
            ss << "  ." << i->getPC() << " is " <<
                i->getOpSpec().fullMnemonic << "\n";
        }
        ss << "\n";
    }
    std::cout << ss.str();
#endif

    return k;
}

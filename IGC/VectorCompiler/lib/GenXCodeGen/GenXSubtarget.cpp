/*========================== begin_copyright_notice ============================

Copyright (c) 2000-2021 Intel Corporation

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

//
// This file implements the GenX specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "IGC/common/StringMacros.hpp"

#include "GenXSubtarget.h"
#include "common/StringMacros.hpp"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "Probe/Assertion.h"

using namespace llvm;

#define DEBUG_TYPE "subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#define GET_SUBTARGETINFO_MC_DESC
#include "GenXGenSubtargetInfo.inc"

static cl::opt<bool>
    StackScratchMem("stack-scratch-mem",
                    cl::desc("Specify what surface should be used for stack"),
                    cl::init(true));

void GenXSubtarget::resetSubtargetFeatures(StringRef CPU, StringRef FS) {

  DumpRegAlloc = false;
  EmitCisa = false;
  HasLongLong = false;
  DisableJmpi = false;
  DisableVectorDecomposition = false;
  DisableJumpTables = false;
  WarnCallable = false;
  EmulateLongLong = false;
  HasAdd64 = false;
  UseMulDDQ = false;
  OCLRuntime = false;
  HasSwitchjmp = false;
  HasIntegerDivision = false;

  if (StackScratchMem)
    StackSurf = PreDefined_Surface::PREDEFINED_SURFACE_T255;
  else
    StackSurf = PreDefined_Surface::PREDEFINED_SURFACE_STACK;

  GenXVariant = llvm::StringSwitch<GenXTag>(CPU)
    .Case("HSW", GENX_HSW)
    .Case("BDW", GENX_BDW)
    .Case("CHV", GENX_CHV)
    .Case("SKL", GENX_SKL)
    .Case("BXT", GENX_BXT)
    .Case("KBL", GENX_KBL)
    .Case("GLK", GENX_GLK)
    .Case("CNL", GENX_CNL)
    .Case("ICLLP", GENX_ICLLP)
    .Case("TGLLP", GENX_TGLLP)
    .Case("DG1", GENX_DG1)
    .Default(GENX_SKL);


  std::string CPUName(CPU);
  if (CPUName.empty())
    CPUName = "generic";

  ParseSubtargetFeatures(CPUName, FS);
}

GenXSubtarget::GenXSubtarget(const Triple &TT, const std::string &CPU,
                             const std::string &FS)
    : GenXGenSubtargetInfo(TT, CPU, FS), TargetTriple(TT) {

  resetSubtargetFeatures(CPU, FS);
}


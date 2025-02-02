/*========================== begin_copyright_notice ============================

Copyright (c) 2021-2021 Intel Corporation

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
// Class provides methods needed for construct kernel names
// and represents state of overriding process.
//

#include <common/debug/Dump.hpp>
#include <common/shaderHash.hpp>
#include <common/shaderOverride.hpp>

#include "vc/igcdeps/ShaderOverride.h"

using Extensions = vc::ShaderOverrider::Extensions;
namespace {

class VC_IGCShaderOverrider final : public vc::ShaderOverrider {
  IGC::Debug::DumpName NamePrefix;
  PLATFORM const &Platform;
  IGC::Debug::OutputFolderName Folder;

  // Gets full kernel name:
  // VC_<HASH>_<ShaderName>
  IGC::Debug::DumpName composeShaderName(llvm::StringRef ShaderName) const;

  // Gets full path to file with extension:
  // "/path/to/shaderOverride/VC_<HASH>_<ShaderName>.Ext"
  std::string path(llvm::StringRef ShaderName,
                   vc::ShaderOverrider::Extensions Ext) const;

public:
  VC_IGCShaderOverrider(ShaderHash const &Hash, PLATFORM const &Platform);

  // Overrides .asm or .dat files
  bool override(void *&GenXBin, int &GenXBinSize, llvm::StringRef ShaderName,
                Extensions Ext) const override;
};

} // namespace

VC_IGCShaderOverrider::VC_IGCShaderOverrider(const ShaderHash &InHash,
                                             PLATFORM const &InPlatform)
    : NamePrefix{IGC::Debug::DumpName("VC").Hash(InHash)}, Platform{InPlatform},
      Folder{IGC::Debug::GetShaderOverridePath()} {}

bool VC_IGCShaderOverrider::override(void *&GenXBin, int &GenXBinSize,
                                     llvm::StringRef ShaderName,
                                     Extensions Ext) const {
  std::string const FullPath = path(ShaderName, Ext);
  bool Status = false;

  switch (Ext) {
  case Extensions::ASM:
    overrideShaderIGA(Platform, GenXBin, GenXBinSize, FullPath, Status);
    break;
  case Extensions::DAT:
    overrideShaderBinary(GenXBin, GenXBinSize, FullPath, Status);
    break;
  default:
    IGC_ASSERT_EXIT_MESSAGE(0, "Unexpected extension");
  }
  return Status;
}

IGC::Debug::DumpName addPostfix(const IGC::Debug::DumpName &Name,
                                llvm::StringRef Postfix) {
  return Name.PostFix(Postfix.str());
}

IGC::Debug::DumpName
VC_IGCShaderOverrider::composeShaderName(llvm::StringRef ShaderName) const {
  return addPostfix(NamePrefix, ShaderName);
}

std::string
VC_IGCShaderOverrider::path(llvm::StringRef ShaderName,
                            vc::ShaderOverrider::Extensions Ext) const {
  IGC::Debug::DumpName FullPath{composeShaderName(ShaderName)};

  switch (Ext) {
  case Extensions::VISAASM:
    FullPath = FullPath.Extension("visaasm");
    break;
  case Extensions::ASM:
    FullPath = FullPath.Extension("asm");
    break;
  case Extensions::DAT:
    FullPath = FullPath.Extension("dat");
    break;
  case Extensions::LL:
    FullPath = FullPath.Extension("ll");
    break;
  default:
    IGC_ASSERT_EXIT_MESSAGE(0, "Unexpected extension");
  }

  return FullPath.AbsolutePath(Folder);
}

namespace vc {
std::unique_ptr<ShaderOverrider>
createVC_IGCShaderOverrider(const ShaderHash &Hash, PLATFORM const &Platform) {
  return std::make_unique<VC_IGCShaderOverrider>(Hash, Platform);
}
} // namespace vc

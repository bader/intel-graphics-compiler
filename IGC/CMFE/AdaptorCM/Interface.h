/*========================== begin_copyright_notice ============================

Copyright (c) 2019-2021 Intel Corporation

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

#ifndef LLVM_FRONTEND_WRAPPER_INTERFACE_H
#define LLVM_FRONTEND_WRAPPER_INTERFACE_H

#include <string>
#include <vector>

#if defined _WIN32 || defined __CYGWIN__
#ifdef INTEL_CM_CLANGFE_LIB
#define INTEL_CM_CLANGFE_DLL_DECL __declspec(dllexport)
#else
#define INTEL_CM_CLANGFE_DLL_DECL __declspec(dllimport)
#endif
#else
#define INTEL_CM_CLANGFE_DLL_DECL
#endif

namespace Intel {
namespace CM {
namespace ClangFE {

struct IInputArgs {
  using StrT = std::string;
  template <typename T> struct FileT {
    T Name;
    T Src;
  };
  template <typename T> using SeqT = std::vector<T>;

  virtual const StrT &getSrc() const = 0;
  virtual const SeqT<StrT> &getCompOpts() const = 0;
  virtual const SeqT<FileT<StrT>> &getExtraFiles() const = 0;
  virtual ~IInputArgs(){};
};

struct IOutputArgs {
  using StrT = std::string;
  using BufferT = std::vector<char>;
  // numbers were taken from OpenCL
  enum class ErrT { SUCCESS = 0, COMPILE_PROGRAM_FAILURE = -15 };

  virtual ErrT getStatus() const = 0;
  virtual const BufferT &getIR() const = 0;
  virtual const StrT &getLog() const = 0;
  virtual void discard() = 0;
  virtual ~IOutputArgs(){};
};

struct IDriverInvocation {
  using StrT = std::string;
  using SeqStrT = std::vector<std::string>;

  enum class InputTypeT { SourceCM, LLVM_IR, SPIRV, NONE, OTHER };
  enum class OutputTypeT { S, SPIRV, LLVM_IR, LLVM_BC, PREPROC, OTHER };
  enum class TargetRuntimeT { CM, OCL, L0 };
  enum class BinaryFormatT { CM, OCL, ZE };

  virtual const SeqStrT& getFEArgs() const = 0;
  virtual const SeqStrT& getBEArgs() const = 0;

  virtual const InputTypeT& getInputType() const = 0;
  virtual const OutputTypeT& getOutputType() const = 0;
  virtual const TargetRuntimeT& getTargetRuntime() const = 0;
  virtual BinaryFormatT getBinaryFormat() const = 0;
  virtual bool getTimePasses() const = 0;

  virtual const StrT& getTargetArch() const = 0;
  virtual const StrT& getInputFilename() const = 0;
  virtual const StrT& getOutputFilename() const = 0;
  virtual const StrT& getTargetFeaturesStr() const = 0;

  virtual bool isHelp() const = 0;

  virtual void discard() = 0;
  virtual ~IDriverInvocation() {};
};

// this number should be increased whenever the public interface changes
static const int InterfaceVersion = 9;

} // namespace ClangFE
} // namespace CM
} // namespace Intel

extern "C" INTEL_CM_CLANGFE_DLL_DECL bool IntelCMClangFEIsShowVersionInvocation(
    Intel::CM::ClangFE::IDriverInvocation const *DriverInvocPtr);

extern "C" INTEL_CM_CLANGFE_DLL_DECL int
IntelCMClangFEGetInterfaceVersion();

extern "C" INTEL_CM_CLANGFE_DLL_DECL int IntelCMClangFEGetRevId(
    Intel::CM::ClangFE::IDriverInvocation const *DriverInvocPtr);

extern "C" INTEL_CM_CLANGFE_DLL_DECL const char *IntelCMClangFEGetVCApiOptions(
    Intel::CM::ClangFE::IDriverInvocation const *DriverInvocPtr);

extern "C" INTEL_CM_CLANGFE_DLL_DECL Intel::CM::ClangFE::IDriverInvocation *
IntelCMClangFEBuildDriverInvocation(int argc, const char * argv[]);

// Invokes clang with source text, options etc. passed in IInputArgs,
// returns result of compilation (status, log, IR, if there is such)
// in IOutputArgs.
//
// This function doesn't take ownership of InputArg, so it is user's
// responsibility to delete both IInputArgs and IOutputArgs.
extern "C" INTEL_CM_CLANGFE_DLL_DECL Intel::CM::ClangFE::IOutputArgs *
IntelCMClangFECompile(const Intel::CM::ClangFE::IInputArgs *InputArg);

#endif // LLVM_FRONTEND_WRAPPER_INTERFACE_H

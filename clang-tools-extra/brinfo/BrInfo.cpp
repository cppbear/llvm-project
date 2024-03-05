//===---- tools/extra/BrInfo.cpp ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  Usage:
//  brinfo <cmake-output-dir> <file1> <file2> ...
//
//  Where <cmake-output-dir> is a CMake build directory in which a file named
//  compile_commands.json exists (enable -DCMAKE_EXPORT_COMPILE_COMMANDS in
//  CMake to get this output).
//
//  <file1> ... specify the paths of files in the CMake source tree. This path
//  is looked up in the compile command database. If the path of a file is
//  absolute, it needs to point into CMake's source tree. If the path is
//  relative, the current working directory needs to be in the CMake source
//  tree and the file must be in a subdirectory of the current working
//  directory. "./" prefixes in the relative files will be automatically
//  removed, but the rest of a relative path must be a suffix of a path in
//  the compile command line database.
//
//  For example, to use brinfo on all files in a subtree of the
//  source tree, use:
//
//    /path/in/subtree $ find . -name '*.cpp'|
//        xargs brinfo /path/to/build
//
//===----------------------------------------------------------------------===//

// #include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
// #include "clang/Frontend/FrontendActions.h"
// #include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Execution.h"
// #include "clang/Tooling/Tooling.h"
// #include "llvm/Support/CommandLine.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Signals.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

namespace BrInfo {

class BrInfoASTConsumer : public ASTConsumer {
public:
  BrInfoASTConsumer(ASTContext *Context) {}

  bool isUserSourceCode(const std::string Filename) {
    if (Filename.empty())
      return false;
    if (Filename.find("usr/lib") != std::string::npos)
      return false;
    return true;
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    auto *TranslationUnitDecl = Context.getTranslationUnitDecl();
    auto Range = TranslationUnitDecl->decls();

    for (auto *Decl : Range) {
      std::string Filename =
          Context.getSourceManager().getFilename(Decl->getLocation()).str();

      if (isUserSourceCode(Filename) && Decl->getKind() == Decl::Function &&
          Decl->hasBody()) {
        // outs() << "Decl: " << Decl->getDeclKindName()
        //        << " Name: " << Decl->getAsFunction()->getNameAsString() << "\n";

        auto Cfg =
            CFG::buildCFG(Decl, Decl->getBody(), &Context, CFG::BuildOptions());
        Cfg->dumpCFGToDot(Context.getLangOpts(), "../DOT/",
                          Decl->getAsFunction()->getNameAsString(),
                          Decl->getAsFunction()->getNameAsString());
      }
    }
  }
};

class BrInfoAction : public ASTFrontendAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    return std::make_unique<BrInfoASTConsumer>(&CI.getASTContext());
  }
};

} // namespace BrInfo

// Set up the command line options
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::OptionCategory BrInfoCategory("brinfo options");

int main(int argc, const char **argv) {
  sys::PrintStackTraceOnErrorSignal(argv[0]);

  auto Executor = createExecutorFromCommandLineArgs(argc, argv, BrInfoCategory);

  if (!Executor) {
    errs() << toString(Executor.takeError()) << "\n";
    return 1;
  }

  auto Err = Executor->get()->execute(
      newFrontendActionFactory<BrInfo::BrInfoAction>());
  if (Err) {
    errs() << toString(std::move(Err)) << "\n";
  }
  Executor->get()->getToolResults()->forEachResult(
      [](StringRef Key, StringRef Value) {
        errs() << "----" << Key.str() << "\n" << Value.str() << "\n";
      });
}

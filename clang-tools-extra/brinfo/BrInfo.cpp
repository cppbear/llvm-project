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

#include "Analysis.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/Execution.h"
#include "llvm/Support/Signals.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

namespace BrInfo {

class BrInfoVisitor : public RecursiveASTVisitor<BrInfoVisitor> {
  ASTContext &Context;

public:
  explicit BrInfoVisitor(ASTContext &Context) : Context(Context) {}

  bool VisitFunctionDecl(FunctionDecl *Func) {
    if (Func->hasBody()) {
      auto BO = CFG::BuildOptions();
      BO.PruneTriviallyFalseEdges = true;

      auto Cfg =
          CFG::buildCFG(Func, Func->getBody(), &Context, BO);
      // Cfg->dumpCFGToDot(Context.getLangOpts(), "../DOT/",
      //                   Func->getAsFunction()->getNameAsString(),
      //                   Func->getAsFunction()->getNameAsString());
      // for (auto *Blk : Cfg->nodes()) {
      //   Blk->dump();
      // }
      Analysis Analysis(Cfg, Context);
      Analysis.getCondChain();
      Analysis.dumpCondChain();
    }
    return true;
  }
};

class BrInfoASTConsumer : public ASTConsumer {

  BrInfoVisitor Visitor;

public:
  BrInfoASTConsumer(ASTContext &Context) : Visitor(Context) {}

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
        Visitor.TraverseDecl(Decl);
      }
    }
  }
};

class BrInfoAction : public ASTFrontendAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    return std::make_unique<BrInfoASTConsumer>(CI.getASTContext());
  }
};

} // namespace BrInfo

// Set up the command line options
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::OptionCategory BrInfoCategory("brinfo options");

int main(int argc, const char **argv) {
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, BrInfoCategory);
  if (!ExpectedParser) {
    errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser &OptionParser = ExpectedParser.get();
  ClangTool Tool(OptionParser.getCompilations(),
                 OptionParser.getSourcePathList());
  return Tool.run(newFrontendActionFactory<BrInfo::BrInfoAction>().get());
}

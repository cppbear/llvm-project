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
#include "clang/Analysis/CFG.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/Execution.h"
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

// Set up the command line options
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::OptionCategory BrInfoCategory("brinfo options");
static cl::opt<std::string>
    FunctionName("f", cl::desc("Specify the function to analyze"),
                 cl::value_desc("string"), cl::cat(BrInfoCategory));

namespace BrInfo {

class BrInfoVisitor : public RecursiveASTVisitor<BrInfoVisitor> {
  ASTContext &Context;

public:
  explicit BrInfoVisitor(ASTContext &Context) : Context(Context) {}

  bool VisitFunctionDecl(FunctionDecl *Func) {
    if (Func->hasBody()) {
      if (!FunctionName.empty() &&
          Func->getNameAsString().compare(FunctionName.getValue()) == 0) {
        Func->getSourceRange().dump(Context.getSourceManager());
        auto BO = CFG::BuildOptions();
        BO.PruneTriviallyFalseEdges = true;
        auto Cfg = CFG::buildCFG(Func, Func->getBody(), &Context, BO);

        // Cfg->dumpCFGToDot(Context.getLangOpts(), "../DOT/",
        //                   Func->getNameAsString(), Func->getNameAsString());
        // for (CFGBlock *Blk : Cfg->nodes()) {
        //   for (CFGElement E : Blk->Elements) {
        //     if (std::optional<CFGStmt> S = E.getAs<CFGStmt>()) {
        //       S->getStmt()->dumpColor();
        //     }
        //   }
        // }

        Analysis Analysis(Cfg, Context);
        Analysis.getCondChains();
        Analysis.simplifyConds();
        Analysis.dumpCondChains();
      }
    }
    return true;
  }
};

class BrInfoASTConsumer : public ASTConsumer {

  BrInfoVisitor Visitor;
  std::string InFile;

public:
  BrInfoASTConsumer(ASTContext &Context, std::string InFile)
      : Visitor(Context), InFile(InFile) {}

  bool isUserSourceCode(const std::string Filename) {
    if (InFile.compare(Filename) == 0)
      return true;
    return false;
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    auto *TranslationUnitDecl = Context.getTranslationUnitDecl();
    auto Range = TranslationUnitDecl->decls();

    for (auto *Decl : Range) {
      std::string Filename =
          Context.getSourceManager().getFilename(Decl->getLocation()).str();

      if (isUserSourceCode(Filename)) {
        // outs() << "DeclKind: " << Decl->getDeclKindName() << "\n";
        Visitor.TraverseDecl(Decl);
      }
    }
  }
};

class BrInfoAction : public ASTFrontendAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    // outs() << "Processing " << InFile << "\n";
    return std::make_unique<BrInfoASTConsumer>(CI.getASTContext(),
                                               InFile.str());
  }
};

} // namespace BrInfo

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

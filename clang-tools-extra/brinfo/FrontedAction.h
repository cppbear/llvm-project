#pragma once

#include "Analysis.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Analysis/CFG.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/Execution.h"

using namespace clang::tooling;

extern cl::opt<string> FunctionName;
extern cl::opt<string> ClassName;
extern cl::opt<bool> DumpCFG;

namespace BrInfo {

class BrInfoVisitor : public RecursiveASTVisitor<BrInfoVisitor> {
  ASTContext &Context;

public:
  explicit BrInfoVisitor(ASTContext &Context) : Context(Context) {}

  bool VisitFunctionDecl(FunctionDecl *Func) {
    if (Func->isThisDeclarationADefinition()) {
      if (FunctionName.empty() ||
          FunctionName.compare(Func->getNameAsString()) == 0) {
        Func->getSourceRange().dump(Context.getSourceManager());
        outs() << "Kind: " << Func->Decl::getDeclKindName() << "\n";
        // Func->getBody()->dumpPretty(Context);
        // if (Func->getSourceRange()
        //         .printToString(Context.getSourceManager())
        //         .find("330:1") == string::npos) {
        //   return true;
        // }
        auto BO = CFG::BuildOptions();
        BO.PruneTriviallyFalseEdges = true;
        auto Cfg = CFG::buildCFG(Func, Func->getBody(), &Context, BO);

        if (DumpCFG)
          Cfg->dumpCFGToDot(Context.getLangOpts(),
                            "/home/chubei/workspace/DOT/",
                            Func->getNameAsString(), Func->getNameAsString());
        // for (CFGBlock *Blk : Cfg->nodes()) {
        //   for (CFGElement E : Blk->Elements) {
        //     if (optional<CFGStmt> S = E.getAs<CFGStmt>()) {
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
  string InFile;

public:
  BrInfoASTConsumer(ASTContext &Context, string InFile)
      : Visitor(Context), InFile(InFile) {}

  bool isUserSourceCode(const string Filename) {
    if (InFile.compare(Filename) == 0)
      return true;
    return false;
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    auto *TranslationUnitDecl = Context.getTranslationUnitDecl();
    auto Range = TranslationUnitDecl->decls();

    for (auto *Decl : Range) {
      string Filename =
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
  unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                            StringRef InFile) override {
    // outs() << "Processing " << InFile << "\n";
    return make_unique<BrInfoASTConsumer>(CI.getASTContext(), InFile.str());
  }
};

inline int run(ClangTool &Tool) {
  return Tool.run(newFrontendActionFactory<BrInfo::BrInfoAction>().get());
}

} // namespace BrInfo
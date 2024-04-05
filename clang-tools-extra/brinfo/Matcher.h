#include "Analysis.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Analysis/CFG.h"
#include "clang/Tooling/Execution.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

extern cl::opt<std::string> FunctionName;
extern cl::opt<std::string> ClassName;
extern cl::opt<bool> DumpCFG;

namespace BrInfo {

class FuncAnalyzer : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) override {
    if (const FunctionDecl *Func =
            Result.Nodes.getNodeAs<FunctionDecl>("func")) {
      Func->getSourceRange().dump(*Result.SourceManager);
      outs() << "Kind: " << Func->Decl::getDeclKindName() << "\n";
      outs() << "ReturnType: " << Func->getReturnType().getAsString() << "\n";
      auto BO = CFG::BuildOptions();
      BO.PruneTriviallyFalseEdges = true;
      auto Cfg = CFG::buildCFG(Func, Func->getBody(), Result.Context, BO);
      if (Cfg) {
        if (DumpCFG)
          Cfg->dumpCFGToDot(Result.Context->getLangOpts(),
                            "/home/chubei/workspace/DOT/",
                            Func->getNameAsString(), Func->getNameAsString());
        // for (CFGBlock *Blk : Cfg->nodes()) {
        //   Blk->dump();
        //   for (CFGElement E : Blk->Elements) {
        //     E.dump();
        //     if (std::optional<CFGStmt> S = E.getAs<CFGStmt>()) {
        //       S->getStmt()->dumpColor();
        //     }
        //   }
        //   auto Terminator = Blk->getTerminator();
        //   if (Terminator.isValid()) {
        //     Terminator.getStmt()->dumpColor();
        //   }
        // }

        Analysis Analysis(Cfg, *Result.Context);
        Analysis.getCondChains();
        Analysis.simplifyConds();
        Analysis.traceBack();
        Analysis.dumpCondChains();
      }
    }
  }
};

inline int run(ClangTool &Tool) {
  FuncAnalyzer Analyzer;
  MatchFinder Finder;
  if (!FunctionName.empty()) {
    if (!ClassName.empty()) {
      DeclarationMatcher FuncMatcher =
          cxxMethodDecl(isDefinition(), hasName(FunctionName),
                        ofClass(hasName(ClassName)))
              .bind("func");
      Finder.addMatcher(FuncMatcher, &Analyzer);
    } else {
      DeclarationMatcher FuncMatcher =
          functionDecl(isDefinition(), hasName(FunctionName)).bind("func");
      Finder.addMatcher(FuncMatcher, &Analyzer);
    }
  } else {
    DeclarationMatcher FuncMatcher = functionDecl(isDefinition()).bind("func");
    Finder.addMatcher(FuncMatcher, &Analyzer);
  }
  return Tool.run(newFrontendActionFactory(&Finder).get());
}

} // namespace BrInfo

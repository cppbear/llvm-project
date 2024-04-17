#include "Analysis.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Analysis/CFG.h"
#include "clang/Tooling/Execution.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

extern cl::opt<std::string> FunctionName;
extern cl::opt<std::string> ClassName;
extern cl::opt<std::string> ProjectPath;
extern cl::opt<bool> DumpCFG;

namespace BrInfo {

class FuncAnalysis : public MatchFinder::MatchCallback {
  Analysis &Analyzer;
  std::string FilePath;

public:
  FuncAnalysis(Analysis &Analyzer, std::string FilePath)
      : Analyzer(Analyzer), FilePath(FilePath) {}

  virtual void run(const MatchFinder::MatchResult &Result) override {
    if (const FunctionDecl *Func =
            Result.Nodes.getNodeAs<FunctionDecl>("func")) {
      std::string FileName =
          Result.SourceManager->getFilename(Func->getLocation()).str();
      // outs() << "Filename: " << FileName << "\n";
      // outs() << "FilePath: " << FilePath << "\n";
      if (FileName.find(FilePath) == std::string::npos)
        return;
      if (Func->getDeclKind() == Decl::CXXConstructor ||
          Func->getDeclKind() == Decl::CXXDestructor)
        return;
      // outs() << Func->Decl::getDeclKindName() << "\n";

      auto BO = CFG::BuildOptions();
      BO.PruneTriviallyFalseEdges = true;
      auto Cfg = CFG::buildCFG(Func, Func->getBody(), Result.Context, BO);
      if (Cfg) {
        if (DumpCFG)
          Cfg->dumpCFGToDot(Result.Context->getLangOpts(),
                            "/home/chubei/workspace/DOT/",
                            Func->getNameAsString(), Func->getNameAsString());
        /* for (CFGBlock *Blk : Cfg->nodes()) {
          Blk->dump();
          for (CFGElement E : Blk->Elements) {
            E.dump();
            if (std::optional<CFGStmt> S = E.getAs<CFGStmt>()) {
              S->getStmt()->dumpColor();
            }
          }
          auto Terminator = Blk->getTerminator();
          if (Terminator.isValid()) {
            Terminator.getStmt()->dumpColor();
          }
        } */

        Analyzer.init(Cfg.get(), Result.Context, Func);
        Analyzer.getCondChains();
        Analyzer.simplifyConds();
        Analyzer.traceBack();
        Analyzer.findContraInLastDef();
        Analyzer.getRequirements();
        Analyzer.clear();
      }
    }
  }
};

inline int run(ClangTool &Tool) {
  Analysis Analyzer;
  FuncAnalysis FuncAnalyzer(Analyzer, Tool.getSourcePaths()[0]);
  MatchFinder Finder;
  if (!FunctionName.empty()) {
    Analyzer.setType(Type::FUNC);
    if (!ClassName.empty()) {
      DeclarationMatcher FuncMatcher =
          cxxMethodDecl(isDefinition(), hasName(FunctionName),
                        ofClass(hasName(ClassName)))
              .bind("func");
      Finder.addMatcher(FuncMatcher, &FuncAnalyzer);
    } else {
      DeclarationMatcher FuncMatcher =
          functionDecl(isDefinition(), hasName(FunctionName)).bind("func");
      Finder.addMatcher(FuncMatcher, &FuncAnalyzer);
    }
  } else {
    Analyzer.setType(Type::FILE);
    DeclarationMatcher FuncMatcher = functionDecl(isDefinition()).bind("func");
    Finder.addMatcher(FuncMatcher, &FuncAnalyzer);
  }
  return Tool.run(newFrontendActionFactory(&Finder).get());
}

} // namespace BrInfo

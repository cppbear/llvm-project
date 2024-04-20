#include "Analysis.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Execution.h"

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
        Analyzer.analyze();
      }
    }
  }
};

inline std::string getFileName(std::string SourcePath) {
  size_t Pos = SourcePath.find_last_of("/");
  size_t Ext = SourcePath.find_last_of(".");
  if (Pos != std::string::npos && Ext != std::string::npos)
    return SourcePath.substr(Pos + 1, Ext - Pos - 1);
  if (Ext != std::string::npos)
    return SourcePath.substr(0, Ext);
  if (Pos != std::string::npos)
    return SourcePath.substr(Pos + 1);
  return SourcePath;
}

inline int run(ClangTool &Tool) {
  std::string FileName = getFileName(Tool.getSourcePaths()[0]);
  Analysis Analyzer;
  FuncAnalysis FuncAnalyzer(Analyzer, Tool.getSourcePaths()[0]);
  MatchFinder Finder;
  if (!FunctionName.empty()) {
    Analyzer.setType(AnalysisType::FUNC);
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
    Analyzer.setType(AnalysisType::FILE);
    DeclarationMatcher FuncMatcher = functionDecl(isDefinition()).bind("func");
    Finder.addMatcher(FuncMatcher, &FuncAnalyzer);
  }
  int Res = Tool.run(newFrontendActionFactory(&Finder).get());
  Analyzer.dumpResults(ProjectPath, FileName, ClassName, FunctionName);
  return Res;
}

} // namespace BrInfo

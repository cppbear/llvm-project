#include "Analysis.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Execution.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

extern cl::opt<std::string> FunctionName;
extern cl::opt<std::string> ClassName;
extern cl::opt<bool> DumpCFG;

namespace BrInfo {

class FuncAnalysis : public MatchFinder::MatchCallback {
  Analysis &Analyzer;
  std::string FilePath;
  std::string ProjectPath;

public:
  FuncAnalysis(Analysis &Analyzer, std::string FilePath,
               std::string ProjectPath)
      : Analyzer(Analyzer), FilePath(FilePath), ProjectPath(ProjectPath) {}

  virtual void run(const MatchFinder::MatchResult &Result) override {
    if (const FunctionDecl *Func =
            Result.Nodes.getNodeAs<FunctionDecl>("func")) {
      std::string FileName =
          Result.SourceManager->getFilename(Func->getLocation()).str();
      SmallVector<char, 128> RealPath;
      sys::fs::real_path(FileName, RealPath);
      std::string ResolvedPath(RealPath.begin(), RealPath.end());
      if (FilePath != ResolvedPath)
        return;
      if (Func->getDeclKind() == Decl::CXXConstructor ||
          Func->getDeclKind() == Decl::CXXDestructor)
        return;
      // outs() << Func->Decl::getDeclKindName() << "\n";

      auto BO = CFG::BuildOptions();
      BO.PruneTriviallyFalseEdges = true;
      // BO.AddEHEdges = true;
      auto Cfg = CFG::buildCFG(Func, Func->getBody(), Result.Context, BO);
      if (Cfg) {
        if (DumpCFG)
          Cfg->dumpCFGToDot(Result.Context->getLangOpts(), ProjectPath,
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
  if (Pos != std::string::npos)
    return SourcePath.substr(Pos + 1);
  return SourcePath;
}

inline int run(ClangTool &Tool, std::string ProjectPath) {
  std::string FileName = getFileName(Tool.getSourcePaths()[0]);
  SmallVector<char, 128> RealPath;
  sys::fs::real_path(Tool.getSourcePaths()[0], RealPath);
  std::string FilePath(RealPath.begin(), RealPath.end());
  Analysis Analyzer;
  FuncAnalysis FuncAnalyzer(Analyzer, FilePath, ProjectPath);
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
  Analyzer.dumpReqToJson(ProjectPath, FileName, ClassName, FunctionName);
  return Res;
}

} // namespace BrInfo

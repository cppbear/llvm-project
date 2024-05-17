#include "nlohmann/json.hpp"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>
#include <unordered_set>

using namespace clang;
using namespace clang::tooling;
using namespace llvm;
using namespace std;

using json = nlohmann::json;

static string RealProjectPath;

class IncludeFinder : public PPCallbacks {
private:
  unordered_set<string> &Includes;

public:
  IncludeFinder(unordered_set<string> &Includes) : Includes(Includes) {}

  void InclusionDirective(SourceLocation HashLoc, const Token &IncludeTok,
                          StringRef FileName, bool IsAngled,
                          CharSourceRange FilenameRange,
                          OptionalFileEntryRef File, StringRef SearchPath,
                          StringRef RelativePath, const Module *Imported,
                          SrcMgr::CharacteristicKind FileType) override {
    if (File) {
      string IncludePath = File->getName().str();
      SmallVector<char, 128> RealPath;
      sys::fs::real_path(IncludePath, RealPath);
      string RealIncludePath(RealPath.begin(), RealPath.end());
      if (RealIncludePath.find(RealProjectPath) != string::npos) {
        Includes.insert(RealIncludePath);
      }
    }
  }
};

class IncludeFinderConsumer : public ASTConsumer {
private:
  unordered_set<string> &Includes;
  Preprocessor &PP;

public:
  IncludeFinderConsumer(unordered_set<string> &Includes, Preprocessor &PP)
      : Includes(Includes), PP(PP) {}

  void Initialize(ASTContext &Context) override {
    PP.addPPCallbacks(make_unique<IncludeFinder>(Includes));
  }
};

class IncludeFinderAction : public ASTFrontendAction {
private:
  unordered_set<string> &Includes;

public:
  IncludeFinderAction(unordered_set<string> &Includes) : Includes(Includes) {}

  unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                            StringRef File) override {
    return make_unique<IncludeFinderConsumer>(Includes, CI.getPreprocessor());
  }
};

class IncludeFinderActionFactory : public FrontendActionFactory {
private:
  unordered_set<string> &Includes;

public:
  IncludeFinderActionFactory(unordered_set<string> &Includes)
      : Includes(Includes) {}

  unique_ptr<FrontendAction> create() override {
    return make_unique<IncludeFinderAction>(Includes);
  }
};

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::OptionCategory IncludeFinderCategory("include-finder options");
static cl::opt<string> ProjectPath("project", cl::Required,
                                   cl::desc("Specify the projrct path"),
                                   cl::value_desc("string"),
                                   cl::cat(IncludeFinderCategory));

void dumpIncludePath(unordered_set<string> &Includes) {
  string FilePath = RealProjectPath + "/includes.json";
  json Json;
  for (const string &Include : Includes) {
    Json.push_back(Include);
  }
  error_code EC;
  llvm::raw_fd_stream File(FilePath, EC);
  if (EC) {
    errs() << "Error: " << EC.message() << "\n";
    return;
  }
  File << Json.dump(4);
}

int main(int argc, const char **argv) {
  auto ExpectedParser =
      CommonOptionsParser::create(argc, argv, IncludeFinderCategory);
  if (!ExpectedParser) {
    errs() << ExpectedParser.takeError();
    return 1;
  }
  CommonOptionsParser &OptionParser = ExpectedParser.get();

  SmallVector<char, 128> RealPath;
  sys::fs::real_path(ProjectPath, RealPath);
  string ProjectPathStr(RealPath.begin(), RealPath.end());
  RealProjectPath = ProjectPathStr;

  ClangTool Tool(OptionParser.getCompilations(),
                 OptionParser.getSourcePathList());

  unordered_set<string> Includes;
  IncludeFinderActionFactory ActionFactory(Includes);

  Tool.run(&ActionFactory);

  // for (const string &Include : Includes) {
  //   errs() << Include << "\n";
  // }

  dumpIncludePath(Includes);
  return 0;
}

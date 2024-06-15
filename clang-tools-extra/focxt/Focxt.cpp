#include "CoutContext.h"
#include "GetAllContext.h"
#include "GetAllPath.h"
#include "nlohmann/json.hpp"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

// Set up the command line options
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::OptionCategory FocxtCategory("focxt options");
static cl::opt<std::string> ProjectPath("project",
                                        cl::desc("Specify the projrct path"),
                                        cl::value_desc("string"),
                                        cl::cat(FocxtCategory));
static cl::opt<std::string> BuildPath("build",
                                      cl::desc("Specify the build path"),
                                      cl::value_desc("string"),
                                      cl::cat(FocxtCategory));
cl::opt<std::string> FilePath("file", cl::desc("Specify the file to analyze"),
                              cl::value_desc("string"), cl::cat(FocxtCategory));
cl::opt<std::string> ClassName("class",
                               cl::desc("Specify the class to analyze"),
                               cl::value_desc("string"),
                               cl::cat(FocxtCategory));
cl::opt<std::string> FunctionName("function",
                                  cl::desc("Specify the function to analyze"),
                                  cl::value_desc("string"),
                                  cl::cat(FocxtCategory));

cl::opt<bool> TestFlag("may-test", cl::desc("Specify get may test"),
                       cl::cat(FocxtCategory));

cl::list<std::string> TestList("must-test",
                               cl::desc("Specify must test second parameters"),
                               cl::value_desc("string"),
                               cl::cat(FocxtCategory));

static std::vector<std::string> file_paths;

void get_all_files(std::string path) {
  for (auto entry : fs::directory_iterator(path)) {
    if (fs::is_regular_file(entry)) {
      std::string file_path = entry.path().string();
      size_t dotPos = file_path.find_last_of('.');
      if (dotPos != std::string::npos) {
        std::string extension = file_path.substr(dotPos);
        if (extension == ".c" || extension == ".h" || extension == ".cpp" ||
            extension == ".cxx" || extension == ".cc" || extension == ".C" ||
            extension == ".hpp" || extension == ".hxx" || extension == ".hh") {
          file_paths.push_back(entry.path().string());
        }
      }
    } else if (fs::is_directory(entry)) {
      std::string directorty_entry = entry.path().string();
      // std::cout << directorty_entry << std::endl;
      if (directorty_entry.find("/build") == std::string::npos &&
          directorty_entry.find("/llm_tests") == std::string::npos
          // &&
          // directorty_entry.find("test") == std::string::npos &&
          // directorty_entry.find("doc") == std::string::npos &&
          // directorty_entry.find("example") == std::string::npos
      ) {
        get_all_files(entry.path());
      }
    }
  }
}

std::string RealProjectPath;
std::string RealBuildPath;
std::string RealFilePath;
std::string RealClassName;
std::string RealFunctionName;
bool RealTestFlag;
std::vector<std::string> RealTestList;

int main(int argc, const char **argv) {
  cl::ParseCommandLineOptions(argc, argv);
  SmallVector<char, 128> RealPath;
  sys::fs::real_path(ProjectPath, RealPath);
  std::string ProjectPathStr(RealPath.begin(), RealPath.end());
  RealProjectPath = ProjectPathStr;
  sys::fs::real_path(BuildPath, RealPath);
  std::string BuildPathStr(RealPath.begin(), RealPath.end());
  RealBuildPath = BuildPathStr;
  sys::fs::real_path(FilePath, RealPath);
  std::string FilePathStr(RealPath.begin(), RealPath.end());
  std::string RealFilePath = FilePathStr;
  RealClassName = ClassName;
  RealFunctionName = FunctionName;
  RealTestFlag = TestFlag;
  RealTestList = TestList;
  get_all_files(RealProjectPath);
  AllContextPaths *all_context_paths = new AllContextPaths;
  for (auto file_path : file_paths) {
    std::vector<const char *> args{"context_path", "-p", RealBuildPath.c_str(),
                                   file_path.c_str()};
    int argc = args.size();
    const char **argv = args.data();
    auto ExpectedParser =
        CommonOptionsParser::create(argc, argv, FocxtCategory);
    if (!ExpectedParser) {
      errs() << ExpectedParser.takeError();
      return 1;
    }
    CommonOptionsParser &OptionParser = ExpectedParser.get();
    ClangTool Tool(OptionParser.getCompilations(),
                   OptionParser.getSourcePathList());
    get_all_paths(Tool, all_context_paths, RealProjectPath);
  }
  // all_context_paths->cout();
  std::vector<FileContext> file_contexts;
  for (auto file_path : file_paths) {
    std::vector<const char *> args{"file_context", "-p", RealBuildPath.c_str(),
                                   file_path.c_str()};
    int argc = args.size();
    const char **argv = args.data();
    auto ExpectedParser =
        CommonOptionsParser::create(argc, argv, FocxtCategory);
    if (!ExpectedParser) {
      errs() << ExpectedParser.takeError();
      return 1;
    }
    CommonOptionsParser &OptionParser = ExpectedParser.get();
    ClangTool Tool(OptionParser.getCompilations(),
                   OptionParser.getSourcePathList());
    OneFileContext one_file_context(Tool, all_context_paths, RealProjectPath,
                                    RealBuildPath, file_path);
    file_contexts.push_back(one_file_context.get_file_context());
  }
  json j;
  // int i = 0;
  for (auto file_context : file_contexts) {
    // file_context.cout();
    // i++;
    for (auto a_class : file_context.classes) {
      for (auto constructor : a_class.constructors) {
        if (constructor.function_body != "") {
          json temp_j = json::object();
          SignatureContext constructor_signature_context =
              get_signature_context(file_contexts, file_context.file_path,
                                    a_class.class_name, constructor.signature);
          json constructor_signatre_j = constructor_signature_context.get_j();
          temp_j[file_context.file_path] = json::object();
          temp_j[file_context.file_path]
                [a_class.class_name + "::" + a_class.class_name] =
                    json::object();
          temp_j[file_context.file_path]
                [a_class.class_name + "::" + a_class.class_name]["focal"] =
                    json::object();
          temp_j[file_context.file_path]
                [a_class.class_name + "::" + a_class.class_name]["focal"] =
                    constructor_signatre_j[file_context.file_path]
                                          [a_class.class_name +
                                           "::" + a_class.class_name];
          if (RealTestFlag) {
            std::vector<TestMacro> may_tests = get_may_tests(
                file_contexts, a_class.class_name, a_class.class_name);
            temp_j[file_context.file_path]
                  [a_class.class_name + "::" + a_class.class_name]["may_test"] =
                      json::object();
            for (auto may_test : may_tests) {
              std::pair<std::string, std::pair<std::string, std::string>>
                  may_test_class_signature =
                      all_context_paths->getTest(may_test.second_parameter);
              SignatureContext may_test_signature_context =
                  get_signature_context(file_contexts,
                                        may_test_class_signature.first,
                                        may_test_class_signature.second.first,
                                        may_test_class_signature.second.second);
              json may_test_signature_j = may_test_signature_context.get_j();
              temp_j[file_context.file_path]
                    [a_class.class_name + "::" + a_class.class_name]["may_test"]
                    [may_test.second_parameter] = json::object();
              for (auto [key, value] : may_test_signature_j.items()) {
                for (auto [key1, value1] : value.items()) {
                  temp_j[file_context.file_path]
                        [a_class.class_name + "::" + a_class.class_name]
                        ["may_test"][may_test.second_parameter] = value1;
                }
              }
              temp_j[file_context.file_path]
                    [a_class.class_name + "::" + a_class.class_name]["may_test"]
                    [may_test.second_parameter]["test_macro"] = json::object();
              temp_j[file_context.file_path]
                    [a_class.class_name + "::" + a_class.class_name]["may_test"]
                    [may_test.second_parameter]["test_macro"] =
                        may_test.test_mecro;
            }
          }
          j.merge_patch(temp_j);
        }
      }
      if (a_class.destructor.function_body != "") {
        std::string function_name = "~" + a_class.class_name;
        json temp_j = json::object();
        SignatureContext destructor_signature_context = get_signature_context(
            file_contexts, file_context.file_path, a_class.class_name,
            a_class.destructor.signature);
        json destructor_signatre_j = destructor_signature_context.get_j();
        temp_j[file_context.file_path] = json::object();
        temp_j[file_context.file_path]
              [a_class.class_name + "::" + function_name] = json::object();
        temp_j[file_context.file_path]
              [a_class.class_name + "::" + function_name]["focal"] =
                  json::object();
        temp_j[file_context.file_path][a_class.class_name +
                                       "::" + function_name]["focal"] =
            destructor_signatre_j[file_context.file_path]
                                 [a_class.class_name + "::" + function_name];
        if (RealTestFlag) {
          std::vector<TestMacro> may_tests =
              get_may_tests(file_contexts, a_class.class_name, function_name);
          temp_j[file_context.file_path]
                [a_class.class_name + "::" + function_name]["may_test"] =
                    json::object();
          for (auto may_test : may_tests) {
            std::pair<std::string, std::pair<std::string, std::string>>
                may_test_class_signature =
                    all_context_paths->getTest(may_test.second_parameter);
            SignatureContext may_test_signature_context = get_signature_context(
                file_contexts, may_test_class_signature.first,
                may_test_class_signature.second.first,
                may_test_class_signature.second.second);
            json may_test_signature_j = may_test_signature_context.get_j();
            temp_j[file_context.file_path]
                  [a_class.class_name + "::" + function_name]["may_test"]
                  [may_test.second_parameter] = json::object();
            for (auto [key, value] : may_test_signature_j.items()) {
              for (auto [key1, value1] : value.items()) {
                temp_j[file_context.file_path]
                      [a_class.class_name + "::" + function_name]["may_test"]
                      [may_test.second_parameter] = value1;
              }
            }
            temp_j[file_context.file_path]
                  [a_class.class_name + "::" + function_name]["may_test"]
                  [may_test.second_parameter]["test_macro"] = json::object();
            temp_j[file_context.file_path]
                  [a_class.class_name + "::" + function_name]["may_test"]
                  [may_test.second_parameter]["test_macro"] =
                      may_test.test_mecro;
          }
        }
        j.merge_patch(temp_j);
      }
      for (auto method : a_class.methods) {
        if (method.function_body != "") {
          json temp_j = json::object();
          SignatureContext method_signature_context =
              get_signature_context(file_contexts, file_context.file_path,
                                    a_class.class_name, method.signature);
          json method_signatre_j = method_signature_context.get_j();
          temp_j[file_context.file_path] = json::object();
          temp_j[file_context.file_path]
                [a_class.class_name + "::" + method.method_name] =
                    json::object();
          temp_j[file_context.file_path]
                [a_class.class_name + "::" + method.method_name]["focal"] =
                    json::object();
          temp_j[file_context.file_path][a_class.class_name +
                                         "::" + method.method_name]["focal"] =
              method_signatre_j[file_context.file_path]
                               [a_class.class_name + "::" + method.method_name];
          if (RealTestFlag) {
            std::vector<TestMacro> may_tests = get_may_tests(
                file_contexts, a_class.class_name, method.method_name);
            temp_j[file_context.file_path]
                  [a_class.class_name + "::" + method.method_name]["may_test"] =
                      json::object();
            for (auto may_test : may_tests) {
              std::pair<std::string, std::pair<std::string, std::string>>
                  may_test_class_signature =
                      all_context_paths->getTest(may_test.second_parameter);
              SignatureContext may_test_signature_context =
                  get_signature_context(file_contexts,
                                        may_test_class_signature.first,
                                        may_test_class_signature.second.first,
                                        may_test_class_signature.second.second);
              json may_test_signature_j = may_test_signature_context.get_j();
              temp_j[file_context.file_path]
                    [a_class.class_name + "::" + method.method_name]["may_test"]
                    [may_test.second_parameter] = json::object();
              for (auto [key, value] : may_test_signature_j.items()) {
                for (auto [key1, value1] : value.items()) {
                  temp_j[file_context.file_path]
                        [a_class.class_name + "::" + method.method_name]
                        ["may_test"][may_test.second_parameter] = value1;
                }
              }
              temp_j[file_context.file_path]
                    [a_class.class_name + "::" + method.method_name]["may_test"]
                    [may_test.second_parameter]["test_macro"] = json::object();
              temp_j[file_context.file_path]
                    [a_class.class_name + "::" + method.method_name]["may_test"]
                    [may_test.second_parameter]["test_macro"] =
                        may_test.test_mecro;
            }
          }
          j.merge_patch(temp_j);
        }
      }
    }
    for (auto function : file_context.functions) {
      if (function.function_body != "") {
        json temp_j = json::object();
        SignatureContext function_signature_context = get_signature_context(
            file_contexts, file_context.file_path, "class", function.signature);
        json function_signatre_j = function_signature_context.get_j();
        temp_j[file_context.file_path] = json::object();
        temp_j[file_context.file_path]["class::" + function.function_name] =
            json::object();
        temp_j[file_context.file_path]["class::" + function.function_name]
              ["focal"] = json::object();
        temp_j[file_context.file_path]["class::" + function.function_name]
              ["focal"] =
                  function_signatre_j[file_context.file_path]
                                     ["class::" + function.function_name];
        if (RealTestFlag) {
          std::vector<TestMacro> may_tests =
              get_may_tests(file_contexts, "class", function.function_name);
          temp_j[file_context.file_path]["class::" + function.function_name]
                ["may_test"] = json::object();
          for (auto may_test : may_tests) {
            std::pair<std::string, std::pair<std::string, std::string>>
                may_test_class_signature =
                    all_context_paths->getTest(may_test.second_parameter);
            SignatureContext may_test_signature_context = get_signature_context(
                file_contexts, may_test_class_signature.first,
                may_test_class_signature.second.first,
                may_test_class_signature.second.second);
            json may_test_signature_j = may_test_signature_context.get_j();
            temp_j[file_context.file_path]["class::" + function.function_name]
                  ["may_test"][may_test.second_parameter] = json::object();
            for (auto [key, value] : may_test_signature_j.items()) {
              for (auto [key1, value1] : value.items()) {
                temp_j[file_context.file_path]
                      ["class::" + function.function_name]["may_test"]
                      [may_test.second_parameter] = value1;
              }
            }
            temp_j[file_context.file_path]["class::" + function.function_name]
                  ["may_test"][may_test.second_parameter]["test_macro"] =
                      json::object();
            temp_j[file_context.file_path]["class::" + function.function_name]
                  ["may_test"][may_test.second_parameter]["test_macro"] =
                      may_test.test_mecro;
          }
        }
        j.merge_patch(temp_j);
      }
    }
  }
  // std::string json_path = RealProjectPath;
  // json_path = json_path + "/" + "allcontext" + ".json";
  // std::ofstream outFile;
  // outFile.open(json_path);
  // outFile << j.dump(4);
  // outFile.close();
  // cl::opt<std::string> FileName("file", cl::desc("Specify the file to
  // analyze"),
  //                               cl::value_desc("string"),
  //                               cl::cat(FocxtCategory));
  // cl::opt<std::string> ClassName(
  //     "class", cl::desc("Specify the class to analyze"),
  //     cl::value_desc("string"), cl::cat(FocxtCategory));
  // cl::opt<std::string> FunctionName(
  //     "function", cl::desc("Specify the function to analyze"),
  //     cl::value_desc("string"), cl::cat(FocxtCategory));
  if (RealFilePath == "") {
    std::string json_path = RealProjectPath;
    json_path = json_path + "/" + "project_cxt" + ".json";
    // j["type"] = "project";
    std::ofstream outFile;
    outFile.open(json_path);
    outFile << j.dump(4);
    outFile.close();
  } else if (RealFunctionName == "") {
    std::string json_path = RealProjectPath;
    std::string file_name =
        RealFilePath.substr(RealFilePath.find_last_of('/') + 1);
    json_path = json_path + "/" + "project_cxt" + ".json";
    json cout_j = json::object();
    cout_j[RealFilePath] = json::object();
    cout_j[RealFilePath] = j[RealFilePath];
    // cout_j["type"] = "file";
    std::ofstream outFile;
    outFile.open(json_path);
    outFile << cout_j.dump(4);
    outFile.close();
  } else {
    std::string json_path = RealProjectPath;
    std::string class_function_name;
    if (RealClassName == "") {
      class_function_name = "class::" + RealFunctionName;
      json_path = json_path + "/" + "project_cxt" + ".json";
    } else {
      class_function_name = RealClassName + "::" + RealFunctionName;
      json_path = json_path + "/" + "project_cxt" + ".json ";
    }
    json cout_j = json::object();
    if (RealTestFlag) {
      cout_j[RealFilePath] = json::object();
      cout_j[RealFilePath][class_function_name] = json::object();
      cout_j[RealFilePath][class_function_name]["may_test"] = json::object();
      cout_j[RealFilePath][class_function_name]["may_test"] =
          j[RealFilePath][class_function_name]["may_test"];
      for (auto a_test : RealTestList) {
        std::vector<TestMacro> must_test = get_must_test(file_contexts, a_test);
        if (must_test.size() != 0) {
          std::pair<std::string, std::pair<std::string, std::string>>
              must_test_class_signature =
                  all_context_paths->getTest(must_test[0].second_parameter);
          cout_j[RealFilePath][class_function_name]["must_test"] =
              json::object();
          SignatureContext must_test_signature_context = get_signature_context(
              file_contexts, must_test_class_signature.first,
              must_test_class_signature.second.first,
              must_test_class_signature.second.second);
          json must_test_signature_j = must_test_signature_context.get_j();
          for (auto [key, value] : must_test_signature_j.items()) {
            for (auto [key1, value1] : value.items()) {
              cout_j[RealFilePath][class_function_name]["must_test"]
                    [must_test[0].second_parameter] = json::object();
              cout_j[RealFilePath][class_function_name]["must_test"]
                    [must_test[0].second_parameter] = value1;
            }
          }
          cout_j[RealFilePath][class_function_name]["must_test"]
                [must_test[0].second_parameter]["test_macro"] = json::object();
          cout_j[RealFilePath][class_function_name]["must_test"]
                [must_test[0].second_parameter]["test_macro"] =
                    must_test[0].test_mecro;
        }
      }
    }
    // cout_j["type"] = "function";
    cout_j[RealFilePath][class_function_name]["focal"] = json::object();
    cout_j[RealFilePath][class_function_name]["focal"] =
        j[RealFilePath][class_function_name]["focal"];
    std::ofstream outFile;
    outFile.open(json_path);
    outFile << cout_j.dump(4);
    outFile.close();
  }
  delete all_context_paths;
  return 0;
}
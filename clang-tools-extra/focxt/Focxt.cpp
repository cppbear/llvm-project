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
          directorty_entry.find("/llm_tests") == std::string::npos) {
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
            std::vector<
                std::pair<std::string, std::pair<std::string, std::string>>>
                MayTest = all_context_paths->getSameTest(a_class.class_name);
            std::vector<std::string> test_macros =
                get_test_macro(file_context.file_path, a_class.class_name);
            temp_j[file_context.file_path]
                  [a_class.class_name + "::" + a_class.class_name]["may_test"] =
                      json::object();
            for (auto may_test : MayTest) {
              SignatureContext may_test_signature_context =
                  get_signature_context(file_contexts, may_test.first,
                                        may_test.second.first,
                                        may_test.second.second);
              json may_test_signature_j = may_test_signature_context.get_j();
              for (auto [key, value] : may_test_signature_j.items()) {
                for (auto [key1, value1] : value.items()) {
                  std::string true_key;
                  for (auto [key2, value2] : value1.items()) {
                    size_t begin = key2.find_first_of('_');
                    size_t end = key2.substr(begin + 1).find_first_of('_');
                    true_key = key2.substr(begin + 1, end);
                    temp_j[file_context.file_path]
                          [a_class.class_name + "::" + a_class.class_name]
                          ["may_test"][true_key] = json::object();
                  }
                  temp_j[file_context.file_path]
                        [a_class.class_name + "::" + a_class.class_name]
                        ["may_test"][true_key] = value1;
                }
              }
            }
            for (auto test_macro : test_macros) {
              std::string key =
                  test_macro.substr(test_macro.find_first_of(',') + 1,
                                    test_macro.find_first_of(')') -
                                        test_macro.find_first_of(',') - 1);
              key = key.substr(key.find_first_not_of(' '),
                               key.find_last_of(' ') -
                                   key.find_first_not_of(' '));
              temp_j[file_context.file_path]
                    [a_class.class_name + "::" + a_class.class_name]["may_test"]
                    [key]["test_macro"] = json::object();
              temp_j[file_context.file_path]
                    [a_class.class_name + "::" + a_class.class_name]["may_test"]
                    [key]["test_macro"] = test_macro;
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
          std::vector<
              std::pair<std::string, std::pair<std::string, std::string>>>
              MayTest = all_context_paths->getSameTest(function_name);
          std::vector<std::string> test_macros =
              get_test_macro(file_context.file_path, function_name);
          temp_j[file_context.file_path]
                [a_class.class_name + "::" + function_name]["may_test"] =
                    json::object();
          for (auto may_test : MayTest) {
            SignatureContext may_test_signature_context = get_signature_context(
                file_contexts, may_test.first, may_test.second.first,
                may_test.second.second);
            json may_test_signature_j = may_test_signature_context.get_j();
            for (auto [key, value] : may_test_signature_j.items()) {
              for (auto [key1, value1] : value.items()) {
                std::string true_key;
                for (auto [key2, value2] : value1.items()) {
                  size_t begin = key2.find_first_of('_');
                  size_t end = key2.substr(begin + 1).find_first_of('_');
                  true_key = key2.substr(begin + 1, end);
                  temp_j[file_context.file_path]
                        [a_class.class_name + "::" + function_name]["may_test"]
                        [true_key] = json::object();
                }
                temp_j[file_context.file_path]
                      [a_class.class_name + "::" + function_name]["may_test"]
                      [true_key] = value1;
              }
            }
          }
          for (auto test_macro : test_macros) {
            std::string key =
                test_macro.substr(test_macro.find_first_of(',') + 1,
                                  test_macro.find_first_of(')') -
                                      test_macro.find_first_of(',') - 1);
            key =
                key.substr(key.find_first_not_of(' '),
                           key.find_last_of(' ') - key.find_first_not_of(' '));
            temp_j[file_context.file_path]
                  [a_class.class_name + "::" + function_name]["may_test"][key]
                  ["test_macro"] = json::object();
            temp_j[file_context.file_path]
                  [a_class.class_name + "::" + function_name]["may_test"][key]
                  ["test_macro"] = test_macro;
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
            std::vector<
                std::pair<std::string, std::pair<std::string, std::string>>>
                MayTest = all_context_paths->getSameTest(method.method_name);
            std::vector<std::string> test_macros =
                get_test_macro(file_context.file_path, method.method_name);
            temp_j[file_context.file_path]
                  [a_class.class_name + "::" + method.method_name]["may_test"] =
                      json::object();
            for (auto may_test : MayTest) {
              SignatureContext may_test_signature_context =
                  get_signature_context(file_contexts, may_test.first,
                                        may_test.second.first,
                                        may_test.second.second);
              json may_test_signature_j = may_test_signature_context.get_j();
              for (auto [key, value] : may_test_signature_j.items()) {
                for (auto [key1, value1] : value.items()) {
                  std::string true_key;
                  for (auto [key2, value2] : value1.items()) {
                    size_t begin = key2.find_first_of('_');
                    size_t end = key2.substr(begin + 1).find_first_of('_');
                    true_key = key2.substr(begin + 1, end);
                    temp_j[file_context.file_path]
                          [a_class.class_name + "::" + method.method_name]
                          ["may_test"][true_key] = json::object();
                  }
                  temp_j[file_context.file_path]
                        [a_class.class_name + "::" + method.method_name]
                        ["may_test"][true_key] = value1;
                }
              }
            }
            for (auto test_macro : test_macros) {
              std::string key =
                  test_macro.substr(test_macro.find_first_of(',') + 1,
                                    test_macro.find_first_of(')') -
                                        test_macro.find_first_of(',') - 1);
              key = key.substr(key.find_first_not_of(' '),
                               key.find_last_of(' ') -
                                   key.find_first_not_of(' '));
              temp_j[file_context.file_path]
                    [a_class.class_name + "::" + method.method_name]["may_test"]
                    [key]["test_macro"] = json::object();
              temp_j[file_context.file_path]
                    [a_class.class_name + "::" + method.method_name]["may_test"]
                    [key]["test_macro"] = test_macro;
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
          std::vector<
              std::pair<std::string, std::pair<std::string, std::string>>>
              MayTest = all_context_paths->getSameTest(function.function_name);
          std::vector<std::string> test_macros =
              get_test_macro(file_context.file_path, function.function_name);
          temp_j[file_context.file_path]["class::" + function.function_name]
                ["may_test"] = json::object();
          for (auto may_test : MayTest) {
            SignatureContext may_test_signature_context = get_signature_context(
                file_contexts, may_test.first, may_test.second.first,
                may_test.second.second);
            json may_test_signature_j = may_test_signature_context.get_j();
            for (auto [key, value] : may_test_signature_j.items()) {
              for (auto [key1, value1] : value.items()) {
                std::string true_key;
                for (auto [key2, value2] : value1.items()) {
                  size_t begin = key2.find_first_of('_');
                  size_t end = key2.substr(begin + 1).find_first_of('_');
                  true_key = key2.substr(begin + 1, end);
                  temp_j[file_context.file_path]
                        ["class::" + function.function_name]["may_test"]
                        [true_key] = json::object();
                }
                temp_j[file_context.file_path]
                      ["class::" + function.function_name]["may_test"]
                      [true_key] = value1;
              }
            }
          }
          for (auto test_macro : test_macros) {
            std::string key =
                test_macro.substr(test_macro.find_first_of(',') + 1,
                                  test_macro.find_first_of(')') -
                                      test_macro.find_first_of(',') - 1);
            key =
                key.substr(key.find_first_not_of(' '),
                           key.find_last_of(' ') - key.find_first_not_of(' '));
            temp_j[file_context.file_path]["class::" + function.function_name]
                  ["may_test"][key]["test_macro"] = json::object();
            temp_j[file_context.file_path]["class::" + function.function_name]
                  ["may_test"][key]["test_macro"] = test_macro;
          }
        }
        j.merge_patch(temp_j);
      }
    }
    // if (i == 3) {
    //   break;
    // }
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
    std::ofstream outFile;
    outFile.open(json_path);
    outFile << j.dump(4);
    outFile.close();
  } else if (RealFunctionName == "") {
    std::string json_path = RealProjectPath;
    std::string file_name =
        RealFilePath.substr(RealFilePath.find_last_of('/') + 1);
    json_path = json_path + "/" + file_name + "_cxt" + ".json";
    json cout_j = json::object();
    cout_j = j[RealFilePath];
    cout_j["type"] = "file";
    std::ofstream outFile;
    outFile.open(json_path);
    outFile << cout_j.dump(4);
    outFile.close();
  } else {
    std::string json_path = RealProjectPath;
    std::string class_function_name;
    if (RealClassName == "") {
      class_function_name = "class::" + RealFunctionName;
      json_path = json_path + "/" + RealFunctionName + "_cxt" + ".json";
    } else {
      class_function_name = RealClassName + "::" + RealFunctionName;
      json_path = json_path + "/" + RealClassName + "_" + RealFunctionName +
                  "_cxt" + ".json ";
    }
    json cout_j = json::object();
    if (RealTestFlag) {
      cout_j["may_test"] = json::object();
      cout_j["may_test"] = j[RealFilePath][class_function_name]["may_test"];
      for (auto a_test : RealTestList) {
        std::vector<std::pair<std::string, std::pair<std::string, std::string>>>
            MustTest = all_context_paths->getDifferentTest(a_test);
        std::vector<std::string> test_macros =
            get_must_test_macro(RealFilePath, a_test);
        cout_j["must_test"] = json::object();
        for (auto must_test : MustTest) {
          SignatureContext must_test_signature_context = get_signature_context(
              file_contexts, must_test.first, must_test.second.first,
              must_test.second.second);
          json must_test_signature_j = must_test_signature_context.get_j();
          for (auto [key, value] : must_test_signature_j.items()) {
            for (auto [key1, value1] : value.items()) {
              std::string true_key;
              for (auto [key2, value2] : value1.items()) {
                size_t begin = key2.find_first_of('_');
                size_t end = key2.substr(begin + 1).find_first_of('_');
                true_key = key2.substr(begin + 1, end);
                cout_j["must_test"][true_key] = json::object();
              }
              cout_j["must_test"][true_key] = value1;
            }
          }
        }
        for (auto test_macro : test_macros) {
          std::string key =
              test_macro.substr(test_macro.find_first_of(',') + 1,
                                test_macro.find_first_of(')') -
                                    test_macro.find_first_of(',') - 1);
          key = key.substr(key.find_first_not_of(' '),
                           key.find_last_of(' ') - key.find_first_not_of(' '));
          cout_j["must_test"][key]["test_macro"] = json::object();
          cout_j["must_test"][key]["test_macro"] = test_macro;
        }
      }
    }
    cout_j["type"] = "function";
    cout_j["focal"] = json::object();
    cout_j["focal"] = j[RealFilePath][class_function_name]["focal"];
    std::ofstream outFile;
    outFile.open(json_path);
    outFile << cout_j.dump(4);
    outFile.close();
  }
  delete all_context_paths;
  return 0;
}
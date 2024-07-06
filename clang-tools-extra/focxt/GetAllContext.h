#pragma once
// #include "GetAllPath.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

struct GlobalVar {
  std::string global_var;
  std::string its_namespace;
  std::vector<Application> applications;
};

struct ADefine {
  std::string define_name;
  std::string define_body;
};

struct TestMacro {
  std::string second_parameter;
  std::string test_mecro;
};

struct InFileFunction {
  std::string class_name;
  std::string function_name;
  std::string signature;
};

class FileContext {
  std::string file_path;
  std::vector<std::string> includes;
  std::vector<ADefine> defines;
  std::vector<GlobalVar> global_vars;
  std::vector<InFileFunction> functions;
  std::vector<TestMacro> test_macros;

  void set_global_vars();
  void get_includes();
  void get_defines();
  void get_global_vars();
  void get_test_macros();
  void get_context();

public:
  FileContext() = default;
  FileContext(std::string file_path) : file_path(file_path) {}
  Class get_simple_class(std::string class_name);
  bool class_has_constructor(std::string class_name, std::string signature);
  Constructor class_get_constructor(std::string class_name,
                                    std::string signature);
  bool class_has_destructor(std::string class_name, std::string signature);
  Destructor class_get_destructor(std::string class_name,
                                  std::string signature);
  bool class_has_method(std::string class_name, std::string signature);
  Method class_get_method(std::string class_name, std::string signature);
  Function get_function(std::string signature);
  std::vector<TestMacro> get_may_test_macros(std::string class_name,
                                             std::string function_name);
  std::vector<TestMacro> get_must_test_macros(std::string second_parameter);
  void cout();
};

class FileContexts {
  std::vector<FileContext> file_contexts;

public:
  void push_back(FileContext file_context);
};

class GetFileContext {
  FileContexts file_contexts;
  std::string project_path;
  std::string compilation_database_path;
  std::vector<std::string> *file_paths;
  cl::OptionCategory FocxtCategory;

public:
  GetFileContext(std::string project_path,
                 std::string compilation_database_path,
                 std::vector<std::string> *file_paths,
                 cl::OptionCategory FocxtCategory)
      : project_path(project_path),
        compilation_database_path(compilation_database_path),
        file_paths(file_paths), FocxtCategory(FocxtCategory) {}
  void get_all_file_contexts();
  FileContexts get_file_contexts();
};
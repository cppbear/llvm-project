#pragma once
#include "GetAllPath.h"
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

struct Application {
  std::string file_path;
  std::string class_name;
  std::string signature;
  bool operator==(const Application &other) const {
    return file_path == other.file_path && class_name == other.class_name &&
           signature == other.signature;
  }
};

struct GlobalVar {
  std::string global_var;
  std::string its_namespace;
  std::vector<Application> applications;
};

struct Constructor {
  std::string signature;
  std::string function_body;
  std::vector<std::string> parameters;
  std::vector<Application> applications;
};

struct Destructor {
  std::string signature;
  std::string function_body;
  std::vector<Application> applications;
};

struct Method {
  std::string method_name;
  std::string signature;
  std::string function_body;
  std::vector<std::string> parameters;
  std::string return_type;
  std::vector<Application> applications;
};

struct Class {
  std::string class_name;
  std::string base_class;
  std::vector<Constructor> constructors;
  Destructor destructor;
  std::vector<std::string> fields;
  std::vector<Method> methods;
  std::string its_namespace;
};

struct Function {
  std::string function_name;
  std::string signature;
  std::string function_body;
  std::vector<std::string> parameters;
  std::string return_type;
  std::vector<Application> applications;
  std::string its_namespace;
};

struct ADefine {
  std::string define_name;
  std::string define_body;
};

struct TestMacro {
  std::string second_parameter;
  std::string test_mecro;
};

class FileContext {
public:
  std::string file_path;
  std::vector<std::string> includes;
  std::vector<ADefine> defines;
  std::vector<GlobalVar> global_vars;
  std::vector<Class> classes;
  std::vector<Function> functions;
  std::vector<TestMacro> test_macros;
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

class OneFileContext {
  ClangTool &Tool;
  AllContextPaths *all_context_paths;
  std::string project_path;
  std::string compilation_database_path;
  std::string file_path;
  FileContext file_context;

public:
  OneFileContext(ClangTool &Tool, AllContextPaths *all_context_paths,
                 std::string project_path,
                 std::string compilation_database_path, std::string file_path)
      : Tool(Tool), all_context_paths(all_context_paths),
        project_path(project_path),
        compilation_database_path(compilation_database_path),
        file_path(file_path), file_context(file_path) {
    set_global_vars();
    get_includes();
    get_defines();
    get_global_vars();
    get_test_macros();
    get_context();
  }
  void set_global_vars();
  void get_includes();
  void get_defines();
  void get_global_vars();
  void get_test_macros();
  void get_context();
  FileContext get_file_context();
};
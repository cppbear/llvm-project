#pragma once
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
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
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace clang;
using namespace clang::tooling;
using namespace llvm;

struct Application {
  std::string class_name;
  std::string signature;
  bool operator==(const Application &other) const {
    return class_name == other.class_name && signature == other.signature;
  }
  void cout() { std::cout << class_name << "\t" << signature << std::endl; }
};

struct Function {
  std::string function_name;
  std::string signature;
  bool is_template;
  std::vector<std::string> template_parameters;
  std::vector<std::vector<std::string>> specialization_parameters;
  std::string function_body;
  std::vector<std::string> parameters;
  std::string return_type;
  std::vector<Application> applications;
  std::string its_namespace;
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
  bool is_template;
  std::vector<std::string> template_parameters;
  std::vector<std::vector<std::string>> specialization_parameters;
  std::string function_body;
  std::vector<std::string> parameters;
  std::string return_type;
  std::vector<Application> applications;
};

struct Alias {
  std::string alias_name;
  std::string base_name;
};

struct Class {
  std::string class_name;
  std::string base_class;
  bool is_template;
  std::vector<std::string> template_parameters;
  std::vector<std::vector<std::string>> specialization_parameters;
  std::vector<std::string> fields;
  std::string its_namespace;
  std::vector<Alias> aliases;
  std::vector<Constructor> constructors;
  Destructor destructor;
  std::vector<Method> methods;
  std::vector<Application> applications;
};

class ClassesAndFunctions {
  std::vector<Class> classes;
  std::vector<Function> functions;

public:
  bool has_class(std::string class_name);
  void push_back_class(Class a_class);
  void push_back_specialization_parameter(
      std::string clas_name, std::string signature,
      std::vector<std::string> specialization_parameter);
  void push_back_applications(
      std::string class_name, std::string signature,
      std::vector<std::pair<std::string, std::string>> applications);
  std::vector<std::string> get_constructors(std::string class_name);
  std::string get_destructor(std::string class_name);
  bool has_function(std::string class_name, std::string signature);
  bool is_function_template(std::string class_name, std::string signature);
  void push_back_function(Function function);
  void push_back_constructor(std::string class_name, Constructor constructor);
  void push_back_destructor(std::string class_name, Destructor destructor);
  void push_back_method(std::string class_name, Method method);
  void cout();
};

class StmtVarFunctionVisitor
    : public RecursiveASTVisitor<StmtVarFunctionVisitor> {
private:
  ASTContext *Context;
  std::vector<Application> applications;

public:
  StmtVarFunctionVisitor(ASTContext *Context) : Context(Context) {}
  bool VisitCallExpr(CallExpr *Call);
  bool VisitVarDecl(VarDecl *Var);
  std::vector<Application> get_applications();
};

class GetClassesAndFunctions {
  ClassesAndFunctions classes_and_functions;
  // AllContextPaths *all_context_paths;
  std::string project_path;
  std::string compilation_database_path;
  std::vector<std::string> *file_paths;
  cl::OptionCategory FocxtCategory;

public:
  GetClassesAndFunctions(std::string project_path,
                         std::string compilation_database_path,
                         std::vector<std::string> *file_paths,
                         cl::OptionCategory FocxtCategory)
      : project_path(project_path),
        compilation_database_path(compilation_database_path),
        file_paths(file_paths), FocxtCategory(FocxtCategory) {}

  void get_definitions();
  ClassesAndFunctions get_classes_and_functions();
};
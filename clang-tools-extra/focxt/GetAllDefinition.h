#pragma once
#include "nlohmann/json.hpp"
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
using json = nlohmann::json;
using namespace clang;
using namespace clang::tooling;
using namespace llvm;

enum MyRecordType { ClassType, StructType, UnionType, EnumType };

struct Application {
  std::string class_name;
  std::string signature;
  bool is_direct;
  bool operator==(const Application &other) const {
    return class_name == other.class_name && signature == other.signature;
  }
  void cout() {
    std::cout << "application_class:\t" << class_name
              << "\tapplication_signature:\t" << signature << std::endl;
  }
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
  void cout() {
    std::cout << "function_name:\t" << function_name << "\tsignature:\t"
              << signature << "\tis_template:\t" << is_template
              << "\tnamespace:\t" << its_namespace << std::endl;
    if (is_template) {
      std::cout << "template_parameters:\t";
      for (auto template_parameter : template_parameters) {
        std::cout << template_parameter << "\t";
      }
      std::cout << std::endl;
      std::cout << "specialization_parameters:\t";
      for (auto specialization_parameter : specialization_parameters) {
        std::cout << "<\t";
        for (auto specialization_paramete : specialization_parameter) {
          std::cout << specialization_paramete << "\t";
        }
        std::cout << ">\t";
      }
      std::cout << std::endl;
    }
    std::cout << "function_body:\t" << function_body << std::endl;
    std::cout << "parameters:\t";
    for (auto parameter : parameters) {
      std::cout << parameter << "\t";
    }
    std::cout << std::endl;
    std::cout << "return_type:\t" << return_type << std::endl;
    std::cout << "applications:\t";
    for (auto application : applications) {
      application.cout();
    }
    std::cout << std::endl << std::endl;
  }
};

struct Constructor {
  std::string signature;
  std::string function_body;
  std::vector<std::string> parameters;
  std::vector<Application> applications;
  void cout() {
    std::cout << "signature:\t" << signature << std::endl;
    std::cout << "function_body:\t" << function_body << std::endl;
    std::cout << "parameters:\t";
    for (auto parameter : parameters) {
      std::cout << parameter << "\t";
    }
    std::cout << std::endl;
    std::cout << "applications:\t";
    for (auto application : applications) {
      application.cout();
    }
    std::cout << std::endl;
  }
};

struct Destructor {
  std::string signature;
  std::string function_body;
  std::vector<Application> applications;
  void cout() {
    std::cout << "signature:\t" << signature << std::endl;
    std::cout << "function_body:\t" << function_body << std::endl;
    std::cout << "applications:\t";
    for (auto application : applications) {
      application.cout();
    }
    std::cout << std::endl;
  }
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
  void cout() {
    std::cout << "method_name:\t" << method_name << "\tsignature:\t"
              << signature << "\tis_template:\t" << is_template << std::endl;
    if (is_template) {
      std::cout << "template_parameters:\t";
      for (auto template_parameter : template_parameters) {
        std::cout << template_parameter << "\t";
      }
      std::cout << std::endl;
      std::cout << "specialization_parameters:\t";
      for (auto specialization_parameter : specialization_parameters) {
        std::cout << "<\t";
        for (auto specialization_paramete : specialization_parameter) {
          std::cout << specialization_paramete << "\t";
        }
        std::cout << ">\t";
      }
      std::cout << std::endl;
    }
    std::cout << "function_body:\t" << function_body << std::endl;
    std::cout << "parameters:\t";
    for (auto parameter : parameters) {
      std::cout << parameter << "\t";
    }
    std::cout << std::endl;
    std::cout << "return_type:\t" << return_type << std::endl;
    std::cout << "applications:\t";
    for (auto application : applications) {
      application.cout();
    }
    std::cout << std::endl;
  }
};

struct Alias {
  std::string alias_name;
  std::string base_name;
  void cout() {
    std::cout << "using " << alias_name << " = " << base_name << std::endl;
  }
};

struct Class {
  MyRecordType record_type;
  std::string class_name;
  std::vector<std::string> base_class;
  bool is_template;
  std::vector<std::string> template_parameters;
  std::vector<std::vector<std::string>> specialization_parameters;
  std::vector<std::string> fields;
  std::vector<int> enum_int_constants;
  std::string its_namespace;
  std::vector<Alias> aliases;
  std::vector<Constructor> constructors;
  Destructor destructor;
  std::vector<Method> methods;
  std::vector<Application> applications;
  void cout() {
    if (record_type == MyRecordType::EnumType) {
      std::cout << "type:\tenum" << "\tclass_name:\t" << class_name
                << std::endl;
      std::cout << "fields:\t";
      for (auto field : fields) {
        std::cout << field << "\t";
      }
      std::cout << std::endl;
      std::cout << "enum_int_constants:\t";
      for (auto enum_int_constant : enum_int_constants) {
        std::cout << enum_int_constant << "\t";
      }
      std::cout << std::endl;
      std::cout << "applications:\t";
      for (auto application : applications) {
        application.cout();
      }
      std::cout << std::endl << std::endl;
    } else {
      std::string record_type_string;
      switch (record_type) {
      case MyRecordType::ClassType:
        record_type_string = "class";
        break;
      case MyRecordType::StructType:
        record_type_string = "struct";
        break;
      case MyRecordType::UnionType:
        record_type_string = "union";
        break;
      default:
        break;
      }
      std::cout << "type:\t" << record_type_string << "\tclass_name:\t"
                << class_name << "\tnamespace:\t" << its_namespace << std::endl;
      std::cout << "base_class:\t";
      for (auto base : base_class) {
        std::cout << base << "\t";
      }
      std::cout << std::endl;
      std::cout << "is_template:\t" << is_template << std::endl;
      if (is_template) {
        std::cout << "template_parameters:\t";
        for (auto template_parameter : template_parameters) {
          std::cout << template_parameter << "\t";
        }
        std::cout << std::endl;
        std::cout << "specialization_parameters:\t";
        for (auto specialization_parameter : specialization_parameters) {
          std::cout << "<\t";
          for (auto specialization_paramete : specialization_parameter) {
            std::cout << specialization_paramete << "\t";
          }
          std::cout << ">\t";
        }
        std::cout << std::endl;
      }
      std::cout << "fields:\t";
      for (auto field : fields) {
        std::cout << field << "\t";
      }
      std::cout << std::endl;
      std::cout << "alias:\t";
      for (auto alias : aliases) {
        alias.cout();
      }
      std::cout << std::endl;
      std::cout << "constructors:\t";
      for (auto constructor : constructors) {
        constructor.cout();
      }
      std::cout << std::endl;
      std::cout << "destructor:\t";
      destructor.cout();
      std::cout << std::endl;
      std::cout << "methods:\t";
      for (auto method : methods) {
        method.cout();
      }
      std::cout << std::endl;
      std::cout << "applications:\t";
      for (auto application : applications) {
        application.cout();
      }
      std::cout << std::endl << std::endl;
    }
  }
};

void update_applications(std::vector<Application> &applications);

class ClassesAndFunctions {
  std::vector<Class> classes;
  std::vector<Function> functions;

public:
  bool has_class(std::string class_name);
  MyRecordType get_class_type(std::string class_name);
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
  void push_back_function(Function function);
  void push_back_constructor(std::string class_name, Constructor constructor);
  void push_back_destructor(std::string class_name, Destructor destructor);
  void push_back_method(std::string class_name, Method method);
  void cout();
  void cout_nums();
  void update_all_applications();
  std::vector<Application> get_applications(std::string class_name,
                                            std::string signature);
  json get_simple_class(std::string class_name);
  void get_all_applications(std::vector<Application> *applications);
  json get_j(Application application);
  std::string get_function_body(std::string class_name, std::string signature);
  std::vector<std::string> get_application_classes(std::string type);
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
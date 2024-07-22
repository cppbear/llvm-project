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

struct ClassMethodandPath {
  std::string class_name;
  std::string signature;
  std::string path;
};

struct FunctionandPath {
  std::string signature;
  std::string path;
};

class AllContextPaths {
  std::vector<ClassMethodandPath> class_method_paths;
  std::vector<FunctionandPath> function_paths;

public:
  void append(ClassMethodandPath class_method_path);
  void append(FunctionandPath function_path);
  std::string getClassMethodPath(std::string class_name, std::string signature);
  std::string getFunctionPath(std::string signature);
  bool hasClassMethodPath(std::string class_name, std::string signature);
  bool hasFunctionPath(std::string signature);
  void cout();
  std::pair<std::string, std::pair<std::string, std::string>>
  getTest(std::string second_parameter);
  bool hasClass(std::string class_name);
  std::vector<std::pair<std::string, std::string>>
  getClassConstructor(std::string class_name);
  std::pair<std::string, std::string>
  getClassDestructor(std::string class_name);
};

void get_all_paths(ClangTool &Tool, AllContextPaths *all_context_paths,
                   std::string project_path);
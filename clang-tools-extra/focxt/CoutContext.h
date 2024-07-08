// #pragma once
// #include "GetAllContext.h"
// #include "clang/AST/ASTConsumer.h"
// #include "clang/AST/RecursiveASTVisitor.h"
// #include "clang/ASTMatchers/ASTMatchFinder.h"
// #include "clang/ASTMatchers/ASTMatchers.h"
// #include "clang/Frontend/CompilerInstance.h"
// #include "clang/Frontend/FrontendAction.h"
// #include "clang/Frontend/FrontendActions.h"
// #include "clang/Lex/Lexer.h"
// #include "clang/Tooling/CommonOptionsParser.h"
// #include "clang/Tooling/Tooling.h"
// #include "llvm/Support/CommandLine.h"
// #include <clang/AST/RecursiveASTVisitor.h>
// #include <filesystem>
// #include <fstream>
// #include <iostream>
// #include <nlohmann/json.hpp>
// #include <set>
// #include <sstream>
// #include <string>
// #include <vector>

// namespace fs = std::filesystem;
// using json = nlohmann::json;
// using namespace clang;
// using namespace clang::ast_matchers;
// using namespace clang::tooling;
// using namespace llvm;

// struct SignatureClass {
//   std::string class_name;
//   std::set<std::string> signatures;
// };

// struct SignatureFiles {
//   std::string file_path;
//   std::vector<SignatureClass> classes;
// };

// class SignatureApplication {
// public:
//   std::vector<SignatureFiles> files;
//   // bool has(std::string file_path, std::string class_name,
//   //          std::string signature);
//   void append(std::string file_path, std::string class_name,
//               std::string signature);
// };

// class SignatureContext {
// public:
//   std::string file_path;
//   std::string signature;
//   std::string class_name;
//   std::string function_name;
//   std::string function_body;
//   std::vector<std::string> includes;
//   std::vector<ADefine> defines;
//   std::vector<GlobalVar> global_vars;
//   std::vector<Class> classes;
//   std::vector<Function> functions;
//   json get_j();
// };

// void get_all_applications(std::vector<Application> *applications,
//                           std::vector<FileContext> &file_contexts,
//                           std::string file_path, std::string class_name,
//                           std::string signature);

// SignatureContext get_signature_context(std::vector<FileContext>
// &file_contexts,
//                                        std::string file_path,
//                                        std::string class_name,
//                                        std::string signature);

// std::vector<TestMacro> get_may_tests(std::vector<FileContext> &file_contexts,
//                                      std::string class_name,
//                                      std::string function_name);

// std::vector<TestMacro> get_must_test(std::vector<FileContext> &file_contexts,
//                                      std::string second_parameter);
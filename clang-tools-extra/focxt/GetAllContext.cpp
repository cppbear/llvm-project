#include "GetAllContext.h"
#include "GetSignature.h"
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

ClassesAndFunctions gac_classes_and_functions;
std::string gac_project_path;
std::string gac_file_path;
std::vector<ADefine> gac_defines;
std::vector<InFileFunction> gac_in_file_functions;
std::vector<InFileAlias> gac_alias;
std::vector<Application> gac_applications;
// std::vector<Class> gac_classes;
// std::vector<Function> gac_functions;

void FileContext::set_global_vars() {
  // ::gac_all_context_paths = all_context_paths;
  ::gac_file_path = file_path;
}

void FileContext::get_includes() {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    std::cerr << "Unable to open the focal file!" << "\n";
  }
  std::string line;
  while (getline(file, line)) {
    if (line.find("#include") != std::string::npos) {
      size_t start = line.find_first_of("\"<") + 1;
      size_t end = line.find_last_of("\">");
      if (start != std::string::npos && end != std::string::npos &&
          start < end) {
        std::string header = line.substr(start, end - start);
        includes.push_back(header);
      }
    }
  }
  file.close();
}

void FileContext::get_test_macros() {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    std::cerr << "Unable to open the focal file!" << "\n";
  }
  std::string line;
  bool is_inside_test_function = false;
  bool is_inside_block_comment = false;
  int n = 0;
  std::string a_macro;
  std::string key;
  while (getline(file, line)) {
    if (is_inside_block_comment) {
      if (line.find("*/") != std::string::npos) {
        is_inside_block_comment = false;
        line = line.substr(line.find("*/") + 2);
      } else {
        continue;
      }
    }
    size_t line_comment_pos = line.find("//");
    if (line_comment_pos != std::string::npos) {
      line = line.substr(0, line_comment_pos);
    }
    size_t block_comment_start = line.find("/*");
    if (block_comment_start != std::string::npos) {
      is_inside_block_comment = true;
      line = line.substr(0, block_comment_start);
    }
    if (line.find("TEST") != std::string::npos &&
        line.find("(") != std::string::npos &&
        line.find(",") != std::string::npos &&
        line.find(")") != std::string::npos) {
      key = line.substr(line.find_first_of(',') + 1,
                        line.find_first_of(')') - line.find_first_of(',') - 1);
      if (key != "") {
        key = key.substr(key.find_first_not_of(' '),
                         key.find_last_of(' ') - key.find_first_not_of(' '));
        is_inside_test_function = true;
      }
    }
    if (is_inside_test_function) {
      if (n == 0 && line.find('{') == std::string::npos) {
        a_macro = a_macro + line + "\n";
        continue;
      }
      a_macro = a_macro + line + "\n";
      for (int i = 0; i < line.size(); i++) {
        if (line[i] == '{') {
          n++;
        } else if (line[i] == '}') {
          n--;
        }
      }
      if (n == 0) {
        TestMacro a_test_macro = {key, a_macro};
        test_macros.push_back(a_test_macro);
        is_inside_test_function = false;
        a_macro = "";
        key = "";
        continue;
      }
    }
  }
  file.close();
}

class DefineGeter : public PPCallbacks {
private:
  Preprocessor &PP;
  ASTContext *Context;

public:
  explicit DefineGeter(Preprocessor &PP, ASTContext *Context)
      : PP(PP), Context(Context) {}

  void MacroDefined(const Token &MacroNameTok,
                    const MacroDirective *MD) override {
    if (const auto *MI = MD->getMacroInfo()) {
      clang::SourceLocation loc = MI->getDefinitionLoc();
      clang::SourceManager &SM = Context->getSourceManager();
      clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
      std::string file_path = presumedLoc.getFilename();
      if (file_path.find(gac_project_path) != std::string::npos) {
        std::string macroName = PP.getSpelling(MacroNameTok);
        std::string macroBody;
        for (auto it = MI->tokens_begin(); it != MI->tokens_end(); ++it) {
          macroBody += PP.getSpelling(*it) + " ";
        }
        // std::cout << macroName << " = " << macroBody << std::endl;
        ADefine a_define;
        a_define.define_name = macroName;
        a_define.define_body = macroBody;
        gac_defines.push_back(a_define);
      }
    }
  }
};

class DefineGeterASTConsumer : public ASTConsumer {
public:
  explicit DefineGeterASTConsumer(Preprocessor &PP, ASTContext *Context) {
    PP.addPPCallbacks(std::make_unique<DefineGeter>(PP, Context));
  }
};

class DefineGeterFrontendAction : public ASTFrontendAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef file) override {
    return std::unique_ptr<ASTConsumer>(
        new DefineGeterASTConsumer(CI.getPreprocessor(), &CI.getASTContext()));
  }
};

void FileContext::get_defines() {
  ::gac_defines.clear();
  Tool.run(newFrontendActionFactory<DefineGeterFrontendAction>().get());
  defines = ::gac_defines;
}

class GlobalVarMatchFinder : public MatchFinder::MatchCallback {
private:
  std::vector<GlobalVar> global_vars;
  std::vector<Application> applications;
  std::string focal_path;

public:
  GlobalVarMatchFinder(std::string focal_path) : focal_path(focal_path) {}

  void run(const MatchFinder::MatchResult &Result) override {
    if (const VarDecl *varDecl =
            Result.Nodes.getNodeAs<VarDecl>("globalvars")) {
      clang::SourceLocation loc = varDecl->getLocation();
      clang::SourceManager &SM = Result.Context->getSourceManager();
      clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
      std::string file_path = presumedLoc.getFilename();
      if (file_path.find(gac_project_path) != std::string::npos &&
          varDecl->hasGlobalStorage() && !varDecl->isStaticLocal()) {
        // varDecl->dump();
        GlobalVar global_var;
        const DeclContext *decl_context = varDecl->getDeclContext();
        bool b = 0;
        while (decl_context) {
          if (const NamespaceDecl *namespaceDecl =
                  dyn_cast<NamespaceDecl>(decl_context)) {
            b = 1;
            if (global_var.its_namespace == "") {
              global_var.its_namespace = namespaceDecl->getNameAsString();
            } else {
              global_var.its_namespace = namespaceDecl->getNameAsString() +
                                         "::" + global_var.its_namespace;
            }
          }
          decl_context = decl_context->getParent();
        }
        if (b == 0) {
          global_var.its_namespace = "";
        }
        const SourceManager &sourceManager = Result.Context->getSourceManager();
        SourceRange srcRange = varDecl->getSourceRange();
        srcRange.setEnd(
            Lexer::getLocForEndOfToken(srcRange.getEnd(), 0, sourceManager,
                                       Result.Context->getLangOpts()));
        bool Invalid = false;
        StringRef srcText = Lexer::getSourceText(
            CharSourceRange::getTokenRange(srcRange), sourceManager,
            Result.Context->getLangOpts(), &Invalid);
        std::string srcText_str = std::string(srcText);
        if (srcText_str[srcText_str.length() - 1] == ')' ||
            srcText_str[srcText_str.length() - 1] == ',' ||
            srcText_str[srcText_str.length() - 1] == ';') {
          srcText_str.erase(srcText_str.length() - 1);
        }
        global_var.global_var = std::string(srcText_str);
        QualType qualType = varDecl->getType();
        // if (qualType->isReferenceType()) {
        //   qualType = qualType.getNonReferenceType();
        // }
        // qualType = qualType.getUnqualifiedType();
        // qualType = qualType.getNonReferenceType();
        // qualType = qualType.getCanonicalType();
        // const clang::Type *typePtr = qualType.getTypePtr();
        // while (const TypedefType *typedefType =
        //            dyn_cast<TypedefType>(typePtr)) {
        //   const TypedefNameDecl *typedefDecl = typedefType->getDecl();
        //   qualType = typedefDecl->getUnderlyingType();
        // }
        std::vector<std::string> types =
            gac_classes_and_functions.get_application_classes(
                qualType.getAsString());
        for (auto type : types) {
          Application application;
          application.class_name = type;
          application.signature = "";
          applications.push_back(application);
          if (gac_classes_and_functions.has_class(type)) {
            std::vector<std::string> need_constructors =
                gac_classes_and_functions.get_constructors(type);
            for (int i = 0; i < need_constructors.size(); i++) {
              Application application;
              application.class_name = type;
              application.signature = need_constructors[i];
              applications.push_back(application);
            }
            std::string need_destructor =
                gac_classes_and_functions.get_destructor(type);
            if (need_destructor != "class") {
              Application application;
              application.class_name = type;
              application.signature = need_destructor;
              applications.push_back(application);
            }
          }
        }
        // Application application;
        // application.class_name = class_name;
        // application.signature = "";
        // applications.push_back(application);
        // if (gac_classes_and_functions.has_class(class_name)) {
        //   std::vector<std::string> need_constructors =
        //       gac_classes_and_functions.get_constructors(class_name);
        //   for (int i = 0; i < need_constructors.size(); i++) {
        //     Application application;
        //     application.class_name = class_name;
        //     application.signature = need_constructors[i];
        //     applications.push_back(application);
        //   }
        //   std::string need_destructor =
        //       gac_classes_and_functions.get_destructor(class_name);
        //   if (need_destructor != "class") {
        //     Application application;
        //     application.class_name = class_name;
        //     application.signature = need_destructor;
        //     applications.push_back(application);
        //   }
        // }
        if (varDecl->hasInit()) {
          const Expr *Expr = varDecl->getInit();
          if (Expr) {
            if (const CallExpr *Call = dyn_cast<CallExpr>(Expr)) {
              const FunctionDecl *Callee = Call->getDirectCallee();
              if (Callee) {
                Callee = Callee->getCanonicalDecl();
                if (Callee->getPrimaryTemplate()) {
                  Callee = Callee->getPrimaryTemplate()->getTemplatedDecl();
                }
                // Callee->dump();
                if (Callee->getTemplateInstantiationPattern()) {
                  Callee = Callee->getTemplateInstantiationPattern();
                  // InstantiatedCallee->dump();
                }
                if (Callee->isCXXClassMember()) {
                  const CXXRecordDecl *ParentClass =
                      dyn_cast<CXXRecordDecl>(Callee->getParent())
                          ->getCanonicalDecl();
                  std::string class_name = ParentClass->getNameAsString();
                  std::string function_name = Callee->getNameAsString();
                  std::string signature = get_signature(Callee);
                  // std::cout << class_name << std::endl;
                  // std::cout << signature << std::endl;
                  if (gac_classes_and_functions.has_function(class_name,
                                                             signature)) {
                    Application application;
                    application.class_name = class_name;
                    application.signature = signature;
                    applications.push_back(application);
                  }
                } else {
                  std::string function_name = Callee->getNameAsString();
                  std::string signature = get_signature(Callee);
                  // std::cout << signature << std::endl;
                  if (gac_classes_and_functions.has_function("class",
                                                             signature)) {
                    Application application;
                    application.class_name = "class";
                    application.signature = signature;
                    applications.push_back(application);
                  }
                }
              }
            }
          }
        }
        b = 0;
        for (int i = 0; i < global_vars.size(); i++) {
          if (global_var.its_namespace == global_vars[i].its_namespace &&
              global_var.global_var == global_vars[i].global_var) {
            b = 1;
            break;
          }
        }
        if (b == 0) {
          global_vars.push_back(global_var);
        }
      }
    }
  }

  const std::vector<GlobalVar> get_vars() { return global_vars; };
  const std::vector<Application> get_applications() { return applications; }
};

DeclarationMatcher globalvarMatcher =
    varDecl(hasGlobalStorage()).bind("globalvars");

void FileContext::get_global_vars() {
  GlobalVarMatchFinder Printer(file_path);
  MatchFinder Finder;
  Finder.addMatcher(globalvarMatcher, &Printer);
  Tool.run(newFrontendActionFactory(&Finder).get());
  global_vars = Printer.get_vars();
  std::vector<Application> global_vars_applications;
  for (auto application : global_vars_applications) {
    applications.push_back(application);
  }
}

class MethodFunctionVisitor
    : public RecursiveASTVisitor<MethodFunctionVisitor> {
public:
  explicit MethodFunctionVisitor(ASTContext *Context) : Context(Context) {}

  bool VisitTypeAliasDecl(TypeAliasDecl *TypeAlias) {
    clang::SourceLocation loc = TypeAlias->getLocation();
    clang::SourceManager &SM = Context->getSourceManager();
    clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
    std::string file_path = presumedLoc.getFilename();
    if (file_path.find(gac_project_path) != std::string::npos) {
      DeclContext *ParentContext = TypeAlias->getDeclContext();
      if (!isa<CXXRecordDecl>(ParentContext)) {
        InFileAlias alias;
        alias.alias_name = TypeAlias->getNameAsString();
        alias.base_name = TypeAlias->getUnderlyingType().getAsString();
        const DeclContext *context = TypeAlias->getDeclContext();
        bool b = 0;
        while (context) {
          if (const NamespaceDecl *namespaceDecl =
                  dyn_cast<NamespaceDecl>(context)) {
            b = 1;
            if (alias.its_namespace == "") {
              alias.its_namespace = namespaceDecl->getNameAsString();
            } else {
              alias.its_namespace =
                  namespaceDecl->getNameAsString() + "::" + alias.its_namespace;
            }
          }
          context = context->getParent();
        }
        if (b == 0) {
          alias.its_namespace = "";
        }
        // std::cout << alias.alias_name << " " << alias.base_name << std::endl;
        gac_alias.push_back(alias);
        QualType qualType = TypeAlias->getUnderlyingType();
        // qualType = qualType.getUnqualifiedType();
        // qualType = qualType.getNonReferenceType();
        // qualType = qualType.getCanonicalType();
        std::vector<std::string> types =
            gac_classes_and_functions.get_application_classes(
                qualType.getAsString());
        for (auto type : types) {
          Application application;
          application.class_name = type;
          application.signature = "";
          gac_applications.push_back(application);
          if (gac_classes_and_functions.has_class(type)) {
            std::vector<std::string> need_constructors =
                gac_classes_and_functions.get_constructors(type);
            for (int i = 0; i < need_constructors.size(); i++) {
              Application application;
              application.class_name = type;
              application.signature = need_constructors[i];
              gac_applications.push_back(application);
            }
            std::string need_destructor =
                gac_classes_and_functions.get_destructor(type);
            if (need_destructor != "class") {
              Application application;
              application.class_name = type;
              application.signature = need_destructor;
              gac_applications.push_back(application);
            }
          }
        }
        // Application application;
        // application.class_name = class_name;
        // application.signature = "";
        // gac_applications.push_back(application);
        // if (gac_classes_and_functions.has_class(class_name)) {
        //   std::vector<std::string> need_constructors =
        //       gac_classes_and_functions.get_constructors(class_name);
        //   for (int i = 0; i < need_constructors.size(); i++) {
        //     Application application;
        //     application.class_name = class_name;
        //     application.signature = need_constructors[i];
        //     gac_applications.push_back(application);
        //   }
        //   std::string need_destructor =
        //       gac_classes_and_functions.get_destructor(class_name);
        //   if (need_destructor != "class") {
        //     Application application;
        //     application.class_name = class_name;
        //     application.signature = need_destructor;
        //     gac_applications.push_back(application);
        //   }
        // }
      }
    }
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *Func) {
    if (Func->isThisDeclarationADefinition()) {
      clang::SourceLocation loc = Func->getLocation();
      clang::SourceManager &SM = Context->getSourceManager();
      clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
      std::string file_path = presumedLoc.getFilename();
      if (file_path == gac_file_path) {
        auto the_Func = Func->getCanonicalDecl();
        if (the_Func->getPrimaryTemplate()) {
          the_Func = the_Func->getPrimaryTemplate()->getTemplatedDecl();
        }
        // Callee->dump();
        if (the_Func->getTemplateInstantiationPattern()) {
          the_Func = the_Func->getTemplateInstantiationPattern();
          // InstantiatedCallee->dump();
        }
        if (!isa<CXXMethodDecl>(the_Func)) {
          std::string function_name = the_Func->getNameAsString();
          std::string signature = get_signature(the_Func);
          if (gac_classes_and_functions.has_function("class", signature)) {
            InFileFunction in_file_function;
            // std::string function_name;
            // std::string signature;
            // std::string function_body;
            // std::vector<std::string> parameters;
            // std::string return_type;
            // std::vector<Application> applications;
            // std::string its_namespace;
            // std::cout << "Function Declaration " << function_name <<
            // std::endl;
            in_file_function.class_name = "class";
            in_file_function.function_name = function_name;
            in_file_function.signature = signature;
            gac_in_file_functions.push_back(in_file_function);
          }
        } else {
          const CXXRecordDecl *ParentClass =
              dyn_cast<CXXRecordDecl>(the_Func->getParent())
                  ->getCanonicalDecl();
          std::string class_name = ParentClass->getNameAsString();
          if (gac_classes_and_functions.has_class(class_name)) {
            // std::cout << "Class Declaration " << class_name <<
            // std::endl;
            std::string signature = get_signature(the_Func);
            if (gac_classes_and_functions.has_function(class_name, signature)) {
              if (isa<CXXConstructorDecl>(the_Func)) {
                // std::cout << "Constructor Declaration " << class_name
                //           << std::endl;
                // std::string signature;
                // std::string function_body;
                // std::vector<std::string> parameters;
                // std::vector<Application> applications;
                InFileFunction in_file_function;
                in_file_function.class_name = class_name;
                in_file_function.function_name = class_name;
                in_file_function.signature = signature;
                gac_in_file_functions.push_back(in_file_function);
              } else if (isa<CXXDestructorDecl>(the_Func)) {
                // std::string signature;
                // std::string function_body;
                // std::vector<Application> applications;
                // std::cout << "Destructor Declaration " << class_name
                //           << std::endl;
                InFileFunction in_file_function;
                in_file_function.class_name = class_name;
                in_file_function.function_name = "~" + class_name;
                in_file_function.signature = signature;
                gac_in_file_functions.push_back(in_file_function);
              } else {
                // std::string method_name;
                // std::string signature;
                // std::string function_body;
                // std::vector<std::string> parameters;
                // std::string return_type;
                // std::vector<Application> applications;
                Method method;
                std::string function_name = Func->getNameAsString();
                // std::cout << "Method Declaration " << function_name
                //           << std::endl;
                InFileFunction in_file_function;
                in_file_function.class_name = class_name;
                in_file_function.function_name = function_name;
                in_file_function.signature = signature;
                gac_in_file_functions.push_back(in_file_function);
              }
            }
          }
        }
      }
    }
    return true;
  }

private:
  ASTContext *Context;
};

class MethodFunctionConsumer : public clang::ASTConsumer {
public:
  explicit MethodFunctionConsumer(ASTContext *Context) : Visitor(Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  MethodFunctionVisitor Visitor;
};

class MethodFunctionAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::make_unique<MethodFunctionConsumer>(&Compiler.getASTContext());
  }
};

void FileContext::get_context() {
  ::gac_in_file_functions.clear();
  ::gac_alias.clear();
  ::gac_applications.clear();
  Tool.run(newFrontendActionFactory<MethodFunctionAction>().get());
  functions = gac_in_file_functions;
  alias = gac_alias;
  for (auto application : gac_applications) {
    applications.push_back(application);
  }
}

// Class FileContext::get_simple_class(std::string class_name) {
//   for (auto a_class : classes) {
//     if (a_class.class_name == class_name) {
//       Class ret;
//       ret.class_name = a_class.class_name;
//       ret.base_class = a_class.base_class;
//       ret.fields = a_class.fields;
//       ret.its_namespace = a_class.its_namespace;
//       for (auto constructor : a_class.constructors) {
//         Constructor new_constructor;
//         new_constructor.signature = constructor.signature;
//         ret.constructors.push_back(new_constructor);
//       }
//       Destructor new_destructor;
//       new_destructor.signature = a_class.destructor.signature;
//       ret.destructor = new_destructor;
//       for (auto method : a_class.methods) {
//         Method new_method;
//         new_method.signature = method.signature;
//         ret.methods.push_back(new_method);
//       }
//       return ret;
//     }
//   }
// }

// bool FileContext::class_has_constructor(std::string class_name,
//                                         std::string signature) {
//   for (auto a_class : classes) {
//     if (a_class.class_name == class_name) {
//       for (auto constructor : a_class.constructors) {
//         if (constructor.signature == signature) {
//           return 1;
//         }
//       }
//       return 0;
//     }
//   }
//   return 0;
// }

// Constructor FileContext::class_get_constructor(std::string class_name,
//                                                std::string signature) {
//   for (auto a_class : classes) {
//     if (a_class.class_name == class_name) {
//       for (auto constructor : a_class.constructors) {
//         if (constructor.signature == signature) {
//           return constructor;
//         }
//       }
//     }
//   }
// }

// bool FileContext::class_has_destructor(std::string class_name,
//                                        std::string signature) {
//   for (auto a_class : classes) {
//     if (a_class.class_name == class_name) {
//       if (a_class.destructor.signature == signature) {
//         return 1;
//       }
//       return 0;
//     }
//   }
//   return 0;
// }
// Destructor FileContext::class_get_destructor(std::string class_name,
//                                              std::string signature) {
//   for (auto a_class : classes) {
//     if (a_class.class_name == class_name) {
//       return a_class.destructor;
//     }
//   }
// }

// bool FileContext::class_has_method(std::string class_name,
//                                    std::string signature) {
//   for (auto a_class : classes) {
//     if (a_class.class_name == class_name) {
//       for (auto method : a_class.methods) {
//         if (method.signature == signature) {
//           return 1;
//         }
//       }
//       return 0;
//     }
//   }
//   return 0;
// }
// Method FileContext::class_get_method(std::string class_name,
//                                      std::string signature) {
//   for (auto a_class : classes) {
//     if (a_class.class_name == class_name) {
//       for (auto method : a_class.methods) {
//         if (method.signature == signature) {
//           return method;
//         }
//       }
//     }
//   }
// }

// Function FileContext::get_function(std::string signature) {
//   for (auto function : functions) {
//     if (function.signature == signature) {
//       return function;
//     }
//   }
// }

// std::vector<TestMacro>
// FileContext::get_may_test_macros(std::string class_name,
//                                  std::string function_name) {
//   std::vector<TestMacro> ret;
//   for (auto a_test_macro : test_macros) {
//     if (class_name == "class") {
//       if (a_test_macro.second_parameter.find(function_name) !=
//           std::string::npos) {
//         ret.push_back(a_test_macro);
//       }
//     } else {
//       if (a_test_macro.second_parameter.find(class_name) != std::string::npos
//       &&
//           a_test_macro.second_parameter.find(function_name) !=
//               std::string::npos) {
//         ret.push_back(a_test_macro);
//       }
//     }
//   }
//   return ret;
// }

// std::vector<TestMacro>
// FileContext::get_must_test_macros(std::string second_parameter) {
//   std::vector<TestMacro> ret;
//   for (auto a_test_macro : test_macros) {
//     if (a_test_macro.second_parameter == second_parameter) {
//       ret.push_back(a_test_macro);
//       return ret;
//     }
//   }
//   return ret;
// }

// void FileContext::cout() {
//   std::cout << file_path << std::endl;
//   //   std::vector<std::string> includes;
//   //   std::vector<ADefine> defines;
//   //   std::vector<GlobalVar> global_vars;
//   //   std::vector<Class> classes;
//   //   std::vector<Function> functions;
//   std::cout << "includes:" << std::endl;
//   for (auto include : includes) {
//     std::cout << include << std::endl;
//   }
//   std::cout << "defines:" << std::endl;
//   for (auto define : defines) {
//     std::cout << define.define_name << " " << define.define_body <<
//     std::endl;
//   }
//   std::cout << "global_vars:" << std::endl;
//   for (auto global_var : global_vars) {
//     std::cout << global_var.global_var << " " << global_var.its_namespace
//               << std::endl;
//     applications_cout(global_var.applications);
//   }
//   std::cout << "classes:" << std::endl;
//   for (auto one_class : classes) {
//     std::cout << one_class.class_name << std::endl;
//   }
//   std::cout << "functions:" << std::endl;
//   for (auto function : functions) {
//     std::cout << function.function_name << std::endl;
//     applications_cout(function.applications);
//   }
// }

void FileContext::delete_repeated_applications() {
  for (auto it = applications.begin(); it != applications.end();) {
    bool b = 0;
    for (auto it2 = applications.begin(); it2 != applications.end(); ++it2) {
      if (it2 != it && it2->class_name == it->class_name &&
          it2->signature == it->signature) {
        b = 1;
        break;
      }
    }
    if (b == 1) {
      it = applications.erase(it);
    } else {
      ++it;
    }
  }
}

void FileContext::get_one_file_context() {
  set_global_vars();
  get_includes();
  get_defines();
  get_global_vars();
  get_test_macros();
  get_context();
  delete_repeated_applications();
}

json FileContext::get_file_j() {
  json j;
  j["includes"] = json::array();
  j["includes"] = includes;
  j["defines"] = json::object();
  for (auto define : defines) {
    j["defines"][define.define_name] = define.define_body;
  }
  for (auto global_var : global_vars) {
    if (j.find(global_var.its_namespace) == j.end()) {
      j[global_var.its_namespace] = json::object();
    }
    if (j[global_var.its_namespace].find("global_vars") ==
        j[global_var.its_namespace].end()) {
      j[global_var.its_namespace]["global_vars"] = json::array();
    }
    j[global_var.its_namespace]["global_vars"].push_back(global_var.global_var);
  }
  for (auto alias : alias) {
    if (j.find(alias.its_namespace) == j.end()) {
      j[alias.its_namespace] = json::object();
    }
    if (j[alias.its_namespace].find("alias") == j[alias.its_namespace].end()) {
      j[alias.its_namespace]["alias"] = json::array();
    }
    std::string alias_string =
        "using " + alias.alias_name + " = " + alias.base_name;
    j[alias.its_namespace]["alias"].push_back(alias_string);
  }
  return j;
}

json FileContext::get_j(bool test_flag) {
  json j;
  json file_j = get_file_j();
  j[file_path] = json::object();
  for (auto function : functions) {
    std::string class_function =
        function.class_name + "::" + function.function_name;
    if (j[file_path].find(class_function) == j[file_path].end()) {
      j[file_path][class_function] = json::object();
      j[file_path][class_function]["focal"] = json::object();
    }
    j[file_path][class_function]["focal"][function.signature] = json::object();
    j[file_path][class_function]["focal"][function.signature].merge_patch(
        file_j);
    j[file_path][class_function]["focal"][function.signature]["function_body"] =
        json::object();
    j[file_path][class_function]["focal"][function.signature]["function_body"] =
        gac_classes_and_functions.get_function_body(function.class_name,
                                                    function.signature);
    std::vector<Application> need_applications = applications;
    Application function_application;
    function_application.class_name = function.class_name;
    function_application.signature = function.signature;
    bool b = 0;
    for (auto application : need_applications) {
      if (application == function_application) {
        b = 1;
        break;
      }
    }
    if (b == 0) {
      need_applications.push_back(function_application);
    }
    gac_classes_and_functions.get_all_applications(&need_applications);
    std::vector<std::string> used_classes;
    for (auto application : need_applications) {
      if (application.class_name != "class") {
        bool b = 0;
        for (auto used_class : used_classes) {
          if (used_class == application.class_name) {
            b = 1;
            break;
          }
        }
        if (b == 0) {
          used_classes.push_back(application.class_name);
          j[file_path][class_function]["focal"][function.signature].merge_patch(
              gac_classes_and_functions.get_simple_class(
                  application.class_name));
        }
      }
      if (application.signature != "") {
        j[file_path][class_function]["focal"][function.signature].merge_patch(
            gac_classes_and_functions.get_j(application.class_name,
                                            application.signature));
      }
    }
  }
  return j;
}

void FileContexts::push_back(FileContext file_context) {
  file_contexts.push_back(file_context);
}

json FileContexts::get_j(bool test_flag) {
  json j;
  for (auto file_context : file_contexts) {
    j.merge_patch(file_context.get_j(test_flag));
  }
  return j;
  // for (auto file_context : file_contexts) {
  //   // file_context.cout();
  //   // i++;
  //   for (auto a_class : file_context.classes) {
  //     for (auto constructor : a_class.constructors) {
  //       if (constructor.function_body != "") {
  //         json temp_j = json::object();
  //         SignatureContext constructor_signature_context =
  //             get_signature_context(file_contexts, file_context.file_path,
  //                                   a_class.class_name,
  //                                   constructor.signature);
  //         json constructor_signatre_j =
  //         constructor_signature_context.get_j();
  //         temp_j[file_context.file_path] = json::object();
  //         temp_j[file_context.file_path]
  //               [a_class.class_name + "::" + a_class.class_name] =
  //                   json::object();
  //         temp_j[file_context.file_path]
  //               [a_class.class_name + "::" + a_class.class_name]["focal"] =
  //                   json::object();
  //         temp_j[file_context.file_path]
  //               [a_class.class_name + "::" + a_class.class_name]["focal"] =
  //                   constructor_signatre_j[file_context.file_path]
  //                                         [a_class.class_name +
  //                                          "::" + a_class.class_name];
  //         if (RealTestFlag) {
  //           std::vector<TestMacro> may_tests = get_may_tests(
  //               file_contexts, a_class.class_name, a_class.class_name);
  //           temp_j[file_context.file_path]
  //                 [a_class.class_name + "::" +
  //                 a_class.class_name]["may_test"] =
  //                     json::object();
  //           for (auto may_test : may_tests) {
  //             std::pair<std::string, std::pair<std::string, std::string>>
  //                 may_test_class_signature =
  //                     all_context_paths->getTest(may_test.second_parameter);
  //             SignatureContext may_test_signature_context =
  //                 get_signature_context(file_contexts,
  //                                       may_test_class_signature.first,
  //                                       may_test_class_signature.second.first,
  //                                       may_test_class_signature.second.second);
  //             json may_test_signature_j = may_test_signature_context.get_j();
  //             temp_j[file_context.file_path]
  //                   [a_class.class_name + "::" +
  //                   a_class.class_name]["may_test"]
  //                   [may_test.second_parameter] = json::object();
  //             for (auto [key, value] : may_test_signature_j.items()) {
  //               for (auto [key1, value1] : value.items()) {
  //                 temp_j[file_context.file_path]
  //                       [a_class.class_name + "::" + a_class.class_name]
  //                       ["may_test"][may_test.second_parameter] = value1;
  //               }
  //             }
  //             temp_j[file_context.file_path]
  //                   [a_class.class_name + "::" +
  //                   a_class.class_name]["may_test"]
  //                   [may_test.second_parameter]["test_macro"] =
  //                   json::object();
  //             temp_j[file_context.file_path]
  //                   [a_class.class_name + "::" +
  //                   a_class.class_name]["may_test"]
  //                   [may_test.second_parameter]["test_macro"] =
  //                       may_test.test_mecro;
  //           }
  //         }
  //         j.merge_patch(temp_j);
  //       }
  //     }
  //     if (a_class.destructor.function_body != "") {
  //       std::string function_name = "~" + a_class.class_name;
  //       json temp_j = json::object();
  //       SignatureContext destructor_signature_context =
  //       get_signature_context(
  //           file_contexts, file_context.file_path, a_class.class_name,
  //           a_class.destructor.signature);
  //       json destructor_signatre_j = destructor_signature_context.get_j();
  //       temp_j[file_context.file_path] = json::object();
  //       temp_j[file_context.file_path]
  //             [a_class.class_name + "::" + function_name] = json::object();
  //       temp_j[file_context.file_path]
  //             [a_class.class_name + "::" + function_name]["focal"] =
  //                 json::object();
  //       temp_j[file_context.file_path][a_class.class_name +
  //                                      "::" + function_name]["focal"] =
  //           destructor_signatre_j[file_context.file_path]
  //                                [a_class.class_name + "::" + function_name];
  //       if (RealTestFlag) {
  //         std::vector<TestMacro> may_tests =
  //             get_may_tests(file_contexts, a_class.class_name,
  //             function_name);
  //         temp_j[file_context.file_path]
  //               [a_class.class_name + "::" + function_name]["may_test"] =
  //                   json::object();
  //         for (auto may_test : may_tests) {
  //           std::pair<std::string, std::pair<std::string, std::string>>
  //               may_test_class_signature =
  //                   all_context_paths->getTest(may_test.second_parameter);
  //           SignatureContext may_test_signature_context =
  //           get_signature_context(
  //               file_contexts, may_test_class_signature.first,
  //               may_test_class_signature.second.first,
  //               may_test_class_signature.second.second);
  //           json may_test_signature_j = may_test_signature_context.get_j();
  //           temp_j[file_context.file_path]
  //                 [a_class.class_name + "::" + function_name]["may_test"]
  //                 [may_test.second_parameter] = json::object();
  //           for (auto [key, value] : may_test_signature_j.items()) {
  //             for (auto [key1, value1] : value.items()) {
  //               temp_j[file_context.file_path]
  //                     [a_class.class_name + "::" + function_name]["may_test"]
  //                     [may_test.second_parameter] = value1;
  //             }
  //           }
  //           temp_j[file_context.file_path]
  //                 [a_class.class_name + "::" + function_name]["may_test"]
  //                 [may_test.second_parameter]["test_macro"] = json::object();
  //           temp_j[file_context.file_path]
  //                 [a_class.class_name + "::" + function_name]["may_test"]
  //                 [may_test.second_parameter]["test_macro"] =
  //                     may_test.test_mecro;
  //         }
  //       }
  //       j.merge_patch(temp_j);
  //     }
  //     for (auto method : a_class.methods) {
  //       if (method.function_body != "") {
  //         json temp_j = json::object();
  //         SignatureContext method_signature_context =
  //             get_signature_context(file_contexts, file_context.file_path,
  //                                   a_class.class_name, method.signature);
  //         json method_signatre_j = method_signature_context.get_j();
  //         temp_j[file_context.file_path] = json::object();
  //         temp_j[file_context.file_path]
  //               [a_class.class_name + "::" + method.method_name] =
  //                   json::object();
  //         temp_j[file_context.file_path]
  //               [a_class.class_name + "::" + method.method_name]["focal"] =
  //                   json::object();
  //         temp_j[file_context.file_path][a_class.class_name +
  //                                        "::" + method.method_name]["focal"]
  //                                        =
  //             method_signatre_j[file_context.file_path]
  //                              [a_class.class_name + "::" +
  //                              method.method_name];
  //         if (RealTestFlag) {
  //           std::vector<TestMacro> may_tests = get_may_tests(
  //               file_contexts, a_class.class_name, method.method_name);
  //           temp_j[file_context.file_path]
  //                 [a_class.class_name + "::" +
  //                 method.method_name]["may_test"] =
  //                     json::object();
  //           for (auto may_test : may_tests) {
  //             std::pair<std::string, std::pair<std::string, std::string>>
  //                 may_test_class_signature =
  //                     all_context_paths->getTest(may_test.second_parameter);
  //             SignatureContext may_test_signature_context =
  //                 get_signature_context(file_contexts,
  //                                       may_test_class_signature.first,
  //                                       may_test_class_signature.second.first,
  //                                       may_test_class_signature.second.second);
  //             json may_test_signature_j = may_test_signature_context.get_j();
  //             temp_j[file_context.file_path]
  //                   [a_class.class_name + "::" +
  //                   method.method_name]["may_test"]
  //                   [may_test.second_parameter] = json::object();
  //             for (auto [key, value] : may_test_signature_j.items()) {
  //               for (auto [key1, value1] : value.items()) {
  //                 temp_j[file_context.file_path]
  //                       [a_class.class_name + "::" + method.method_name]
  //                       ["may_test"][may_test.second_parameter] = value1;
  //               }
  //             }
  //             temp_j[file_context.file_path]
  //                   [a_class.class_name + "::" +
  //                   method.method_name]["may_test"]
  //                   [may_test.second_parameter]["test_macro"] =
  //                   json::object();
  //             temp_j[file_context.file_path]
  //                   [a_class.class_name + "::" +
  //                   method.method_name]["may_test"]
  //                   [may_test.second_parameter]["test_macro"] =
  //                       may_test.test_mecro;
  //           }
  //         }
  //         j.merge_patch(temp_j);
  //       }
  //     }
  //   }
  //   for (auto function : file_context.functions) {
  //     if (function.function_body != "") {
  //       json temp_j = json::object();
  //       SignatureContext function_signature_context = get_signature_context(
  //           file_contexts, file_context.file_path, "class",
  //           function.signature);
  //       json function_signatre_j = function_signature_context.get_j();
  //       temp_j[file_context.file_path] = json::object();
  //       temp_j[file_context.file_path]["class::" + function.function_name] =
  //           json::object();
  //       temp_j[file_context.file_path]["class::" + function.function_name]
  //             ["focal"] = json::object();
  //       temp_j[file_context.file_path]["class::" + function.function_name]
  //             ["focal"] =
  //                 function_signatre_j[file_context.file_path]
  //                                    ["class::" + function.function_name];
  //       if (RealTestFlag) {
  //         std::vector<TestMacro> may_tests =
  //             get_may_tests(file_contexts, "class", function.function_name);
  //         temp_j[file_context.file_path]["class::" + function.function_name]
  //               ["may_test"] = json::object();
  //         for (auto may_test : may_tests) {
  //           std::pair<std::string, std::pair<std::string, std::string>>
  //               may_test_class_signature =
  //                   all_context_paths->getTest(may_test.second_parameter);
  //           SignatureContext may_test_signature_context =
  //           get_signature_context(
  //               file_contexts, may_test_class_signature.first,
  //               may_test_class_signature.second.first,
  //               may_test_class_signature.second.second);
  //           json may_test_signature_j = may_test_signature_context.get_j();
  //           temp_j[file_context.file_path]["class::" +
  //           function.function_name]
  //                 ["may_test"][may_test.second_parameter] = json::object();
  //           for (auto [key, value] : may_test_signature_j.items()) {
  //             for (auto [key1, value1] : value.items()) {
  //               temp_j[file_context.file_path]
  //                     ["class::" + function.function_name]["may_test"]
  //                     [may_test.second_parameter] = value1;
  //             }
  //           }
  //           temp_j[file_context.file_path]["class::" +
  //           function.function_name]
  //                 ["may_test"][may_test.second_parameter]["test_macro"] =
  //                     json::object();
  //           temp_j[file_context.file_path]["class::" +
  //           function.function_name]
  //                 ["may_test"][may_test.second_parameter]["test_macro"] =
  //                     may_test.test_mecro;
  //         }
  //       }
  //       j.merge_patch(temp_j);
  //     }
  //   }
  // }
}

void GetFileContext::get_all_file_contexts() {
  ::gac_project_path = project_path;
  ::gac_classes_and_functions = classes_and_functions;
  for (auto file_path : *file_paths) {
    std::vector<const char *> args{"file_context", "-p",
                                   compilation_database_path.c_str(),
                                   file_path.c_str()};
    int argc = args.size();
    const char **argv = args.data();
    auto ExpectedParser =
        CommonOptionsParser::create(argc, argv, FocxtCategory);
    if (!ExpectedParser) {
      errs() << ExpectedParser.takeError();
      exit(1);
    }
    CommonOptionsParser &OptionParser = ExpectedParser.get();
    ClangTool Tool(OptionParser.getCompilations(),
                   OptionParser.getSourcePathList());
    FileContext file_context(Tool, file_path);
    file_context.get_one_file_context();
    file_contexts.push_back(file_context);
  }
}

FileContexts GetFileContext::get_file_contexts() { return file_contexts; }

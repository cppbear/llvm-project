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

AllContextPaths *gac_all_context_paths;
std::string gac_project_path;
std::string gac_compilation_database_path;
std::string gac_file_path;
std::vector<ADefine> gac_defines;
std::vector<Class> gac_classes;
std::vector<Function> gac_functions;

void OneFileContext::set_global_vars() {
  ::gac_all_context_paths = all_context_paths;
  ::gac_project_path = project_path;
  ::gac_compilation_database_path = compilation_database_path;
  ::gac_file_path = file_path;
}

void applications_cout(std::vector<Application> &applications) {
  std::cout << "applications:" << std::endl;
  for (auto application : applications) {
    std::cout << application.file_path << " " << application.class_name << " "
              << application.signature << std::endl;
  }
}

void OneFileContext::get_includes() {
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
        file_context.includes.push_back(header);
      }
    }
  }
  file.close();
}

void OneFileContext::get_test_macros() {
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
      key = key.substr(key.find_first_not_of(' '),
                       key.find_last_of(' ') - key.find_first_not_of(' '));
      is_inside_test_function = true;
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
        file_context.test_macros.push_back(a_test_macro);
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

void OneFileContext::get_defines() {
  ::gac_defines.clear();
  Tool.run(newFrontendActionFactory<DefineGeterFrontendAction>().get());
  file_context.defines = ::gac_defines;
}

class GlobalVarMatchFinder : public MatchFinder::MatchCallback {
private:
  std::vector<GlobalVar> global_vars;
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
            global_var.its_namespace = namespaceDecl->getNameAsString();
            break;
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
        if (qualType->isReferenceType()) {
          qualType = qualType.getNonReferenceType();
        }
        qualType = qualType.getUnqualifiedType();
        qualType = qualType.getCanonicalType();
        // const clang::Type *typePtr = qualType.getTypePtr();
        // while (const TypedefType *typedefType =
        //            dyn_cast<TypedefType>(typePtr)) {
        //   const TypedefNameDecl *typedefDecl = typedefType->getDecl();
        //   qualType = typedefDecl->getUnderlyingType();
        // }
        std::string class_name = qualType.getAsString();
        if (gac_all_context_paths->hasClass(class_name)) {
          std::vector<std::pair<std::string, std::string>> need_constructors =
              gac_all_context_paths->getClassConstructor(class_name);
          for (int i = 0; i < need_constructors.size(); i++) {
            std::string function_name = class_name;
            std::string signature = need_constructors[i].first;
            std::string file_path = need_constructors[i].second;
            Application application;
            application.file_path = file_path;
            application.class_name = class_name;
            application.signature = signature;
            global_var.applications.push_back(application);
          }
          std::pair<std::string, std::string> need_destructor =
              gac_all_context_paths->getClassDestructor(class_name);
          if (need_destructor.first != "class") {
            std::string function_name = "~" + class_name;
            std::string signature = need_destructor.first;
            std::string file_path = need_destructor.second;
            Application application;
            application.file_path = file_path;
            application.class_name = class_name;
            application.signature = signature;
            global_var.applications.push_back(application);
          }
        }
        if (varDecl->hasInit()) {
          const Expr *Expr = varDecl->getInit();
          if (Expr) {
            if (const CallExpr *Call = dyn_cast<CallExpr>(Expr)) {
              const FunctionDecl *Callee = Call->getDirectCallee();
              if (Callee) {
                if (Callee->isCXXClassMember()) {
                  const CXXRecordDecl *ParentClass =
                      dyn_cast<CXXRecordDecl>(Callee->getParent());
                  std::string class_name = ParentClass->getNameAsString();
                  std::string function_name = Callee->getNameAsString();
                  std::string signature = get_signature(Callee);
                  // std::cout << class_name << std::endl;
                  // std::cout << signature << std::endl;
                  if (gac_all_context_paths->hasClassMethodPath(class_name,
                                                                signature)) {
                    std::string file_path =
                        gac_all_context_paths->getClassMethodPath(class_name,
                                                                  signature);
                    Application application;
                    application.file_path = file_path;
                    application.class_name = class_name;
                    application.signature = signature;
                    global_var.applications.push_back(application);
                  }
                } else {
                  std::string function_name = Callee->getNameAsString();
                  std::string signature = get_signature(Callee);
                  // std::cout << signature << std::endl;
                  if (gac_all_context_paths->hasFunctionPath(signature)) {
                    std::string file_path =
                        gac_all_context_paths->getFunctionPath(signature);
                    Application application;
                    application.file_path = file_path;
                    application.class_name = "class";
                    application.signature = signature;
                    global_var.applications.push_back(application);
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
};

DeclarationMatcher globalvarMatcher =
    varDecl(hasGlobalStorage()).bind("globalvars");

void OneFileContext::get_global_vars() {
  GlobalVarMatchFinder Printer(file_path);
  MatchFinder Finder;
  Finder.addMatcher(globalvarMatcher, &Printer);
  Tool.run(newFrontendActionFactory(&Finder).get());
  file_context.global_vars = Printer.get_vars();
}

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

bool StmtVarFunctionVisitor::VisitCallExpr(CallExpr *Call) {
  const FunctionDecl *Callee = Call->getDirectCallee();
  if (Callee) {
    if (Callee->isCXXClassMember()) {
      // std::string file_path;
      // std::string class_name;
      // std::string function_name;
      // std::string signature;
      const CXXRecordDecl *ParentClass =
          dyn_cast<CXXRecordDecl>(Callee->getParent());
      std::string class_name = ParentClass->getNameAsString();
      std::string function_name = Callee->getNameAsString();
      std::string signature = get_signature(Callee);
      // std::cout << "Class " << class_name << " Method " << function_name
      //           << " Call" << std::endl;
      // std::cout << class_name << std::endl;
      // std::cout << signature << std::endl;
      if (gac_all_context_paths->hasClassMethodPath(class_name, signature)) {
        std::string file_path =
            gac_all_context_paths->getClassMethodPath(class_name, signature);
        Application application;
        application.file_path = file_path;
        application.class_name = class_name;
        application.signature = signature;
        applications.push_back(application);
      }
    } else {
      // std::string file_path;
      // std::string class_name;
      // std::string function_name;
      // std::string signature;
      std::string function_name = Callee->getNameAsString();
      // std::cout << "Function Call " << function_name << std::endl;
      std::string signature = get_signature(Callee);
      // std::cout << signature << std::endl;
      if (gac_all_context_paths->hasFunctionPath(signature)) {
        std::string file_path =
            gac_all_context_paths->getFunctionPath(signature);
        Application application;
        application.file_path = file_path;
        application.class_name = "class";
        application.signature = signature;
        applications.push_back(application);
      }
    }
  }
  return true;
}

bool StmtVarFunctionVisitor::VisitVarDecl(VarDecl *Var) {
  if (Var) {
    QualType qualType = Var->getType();
    if (qualType->isReferenceType()) {
      qualType = qualType.getNonReferenceType();
    }
    qualType = qualType.getUnqualifiedType();
    qualType = qualType.getCanonicalType();
    // const clang::Type *typePtr = qualType.getTypePtr();
    // while (const TypedefType *typedefType = dyn_cast<TypedefType>(typePtr)) {
    //   const TypedefNameDecl *typedefDecl = typedefType->getDecl();
    //   qualType = typedefDecl->getUnderlyingType();
    // }
    std::string class_name = qualType.getAsString();
    if (gac_all_context_paths->hasClass(class_name)) {
      std::vector<std::pair<std::string, std::string>> need_constructors =
          gac_all_context_paths->getClassConstructor(class_name);
      for (int i = 0; i < need_constructors.size(); i++) {
        std::string function_name = class_name;
        std::string signature = need_constructors[i].first;
        std::string file_path = need_constructors[i].second;
        Application application;
        application.file_path = file_path;
        application.class_name = class_name;
        application.signature = signature;
        applications.push_back(application);
      }
      std::pair<std::string, std::string> need_destructor =
          gac_all_context_paths->getClassDestructor(class_name);
      if (need_destructor.first != "class") {
        std::string function_name = "~" + class_name;
        std::string signature = need_destructor.first;
        std::string file_path = need_destructor.second;
        Application application;
        application.file_path = file_path;
        application.class_name = class_name;
        application.signature = signature;
        applications.push_back(application);
      }
    }
  }
  return true;
}

std::vector<Application> StmtVarFunctionVisitor::get_applications() {
  return applications;
}

class MethodFunctionVisitor
    : public RecursiveASTVisitor<MethodFunctionVisitor> {
public:
  explicit MethodFunctionVisitor(ASTContext *Context) : Context(Context) {}

  bool VisitFunctionDecl(FunctionDecl *Func) {
    if (Func->isThisDeclarationADefinition()) {
      clang::SourceLocation loc = Func->getLocation();
      clang::SourceManager &SM = Context->getSourceManager();
      clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
      std::string file_path = presumedLoc.getFilename();
      if (file_path == gac_file_path) {
        if (!isa<CXXMethodDecl>(Func)) {
          std::string function_name = Func->getNameAsString();
          std::string signature = get_signature(Func);
          if (gac_all_context_paths->hasFunctionPath(signature)) {
            Function function;
            // std::string function_name;
            // std::string signature;
            // std::string function_body;
            // std::vector<std::string> parameters;
            // std::string return_type;
            // std::vector<Application> applications;
            // std::string its_namespace;
            // std::cout << "Function Declaration " << function_name <<
            // std::endl;
            function.function_name = function_name;
            function.signature = signature;
            const SourceManager &sourceManager = Context->getSourceManager();
            SourceRange srcRange_r = Func->getReturnTypeSourceRange();
            srcRange_r.setEnd(Lexer::getLocForEndOfToken(
                srcRange_r.getEnd(), 0, sourceManager, Context->getLangOpts()));
            bool Invalid = false;
            StringRef srcText_r = Lexer::getSourceText(
                CharSourceRange::getTokenRange(srcRange_r), sourceManager,
                Context->getLangOpts(), &Invalid);
            function.return_type = std::string(srcText_r);
            for (const ParmVarDecl *param : Func->parameters()) {
              SourceRange srcRange_p = param->getSourceRange();
              srcRange_p.setEnd(Lexer::getLocForEndOfToken(
                  srcRange_p.getEnd(), 0, sourceManager,
                  Context->getLangOpts()));
              StringRef srcText_p = Lexer::getSourceText(
                  CharSourceRange::getTokenRange(srcRange_p), sourceManager,
                  Context->getLangOpts(), &Invalid);
              std::string srcText_str = std::string(srcText_p);
              if (srcText_str[srcText_str.length() - 1] == ')' ||
                  srcText_str[srcText_str.length() - 1] == ',' ||
                  srcText_str[srcText_str.length() - 1] == ';') {
                srcText_str.erase(srcText_str.length() - 1);
              }
              function.parameters.push_back(srcText_str);
            }
            SourceRange srcRange = Func->getSourceRange();
            srcRange.setEnd(Lexer::getLocForEndOfToken(
                srcRange.getEnd(), 0, sourceManager, Context->getLangOpts()));
            StringRef srcText = Lexer::getSourceText(
                CharSourceRange::getTokenRange(srcRange), sourceManager,
                Context->getLangOpts(), &Invalid);
            function.function_body = srcText;
            const DeclContext *context = Func->getDeclContext();
            bool b = 0;
            while (context) {
              if (const NamespaceDecl *namespaceDecl =
                      dyn_cast<NamespaceDecl>(context)) {
                b = 1;
                function.its_namespace = namespaceDecl->getNameAsString();
                break;
              }
              context = context->getParent();
            }
            if (b == 0) {
              function.its_namespace = "";
            }
            StmtVarFunctionVisitor function_stmt_visitor(Context);
            function_stmt_visitor.TraverseStmt(Func->getBody());
            function.applications = function_stmt_visitor.get_applications();
            gac_functions.push_back(function);
          }
        } else {
          const CXXRecordDecl *ParentClass =
              dyn_cast<CXXRecordDecl>(Func->getParent());
          std::string class_name = ParentClass->getNameAsString();
          if (gac_all_context_paths->hasClass(class_name)) {
            // std::cout << "Class Declaration " << class_name << std::endl;
            bool b = 0;
            for (int i = 0; i < gac_classes.size(); i++) {
              if (class_name == gac_classes[i].class_name) {
                b = 1;
                break;
              }
            }
            if (b == 0) {
              // std::string class_name;
              // std::string base_class;
              // std::vector<Constructor> constructors;
              // Destructor destructor;
              // std::vector<std::string> fields;
              // std::vector<Method> methods;
              // std::string its_namespace;
              Class a_class;
              a_class.class_name = class_name;
              if (ParentClass->getNumBases() > 0) {
                a_class.base_class =
                    ParentClass->bases_begin()->getType().getAsString();
              }
              const DeclContext *context = ParentClass->getDeclContext();
              bool b = 0;
              while (context) {
                if (const NamespaceDecl *namespaceDecl =
                        dyn_cast<NamespaceDecl>(context)) {
                  b = 1;
                  a_class.its_namespace = namespaceDecl->getNameAsString();
                  break;
                }
                context = context->getParent();
              }
              if (b == 0) {
                a_class.its_namespace = "";
              }
              int i = 0;
              for (const auto *decl : ParentClass->decls()) {
                if (const CXXRecordDecl *Record =
                        dyn_cast<CXXRecordDecl>(decl)) {
                  if (i == 0) {
                    continue;
                  }
                  const SourceManager &sourceManager =
                      Context->getSourceManager();
                  SourceRange srcRange = Record->getSourceRange();
                  srcRange.setEnd(Lexer::getLocForEndOfToken(
                      srcRange.getEnd(), 0, sourceManager,
                      Context->getLangOpts()));
                  bool Invalid = false;
                  StringRef srcText = Lexer::getSourceText(
                      CharSourceRange::getTokenRange(srcRange), sourceManager,
                      Context->getLangOpts(), &Invalid);
                  // std::cout << std::string(srcText) << std::endl;
                  std::string srcText_str = std::string(srcText);
                  if (srcText_str[srcText_str.length() - 1] == ';') {
                    srcText_str.erase(srcText_str.length() - 1);
                  }
                  if (Record->isClass()) {
                    if (srcText.find('{') != std::string::npos) {
                      srcText_str.erase(srcText_str.find_first_of('{'));
                    }
                    a_class.fields.push_back(srcText_str);
                  } else {
                    a_class.fields.push_back(srcText_str);
                  }
                } else if (const FieldDecl *Field = dyn_cast<FieldDecl>(decl)) {
                  const SourceManager &sourceManager =
                      Context->getSourceManager();
                  SourceRange srcRange = Field->getSourceRange();
                  srcRange.setEnd(Lexer::getLocForEndOfToken(
                      srcRange.getEnd(), 0, sourceManager,
                      Context->getLangOpts()));
                  bool Invalid = false;
                  StringRef srcText = Lexer::getSourceText(
                      CharSourceRange::getTokenRange(srcRange), sourceManager,
                      Context->getLangOpts(), &Invalid);
                  // std::cout << std::string(srcText) << std::endl;
                  std::string srcText_str = std::string(srcText);
                  if (srcText_str[srcText_str.length() - 1] == ';') {
                    srcText_str.erase(srcText_str.length() - 1);
                  }
                  a_class.fields.push_back(srcText_str);
                } else if (const EnumDecl *Enum = dyn_cast<EnumDecl>(decl)) {
                  const SourceManager &sourceManager =
                      Context->getSourceManager();
                  SourceRange srcRange = Enum->getSourceRange();
                  srcRange.setEnd(Lexer::getLocForEndOfToken(
                      srcRange.getEnd(), 0, sourceManager,
                      Context->getLangOpts()));
                  bool Invalid = false;
                  StringRef srcText = Lexer::getSourceText(
                      CharSourceRange::getTokenRange(srcRange), sourceManager,
                      Context->getLangOpts(), &Invalid);
                  // std::cout << std::string(srcText) << std::endl;
                  std::string srcText_str = std::string(srcText);
                  if (srcText_str[srcText_str.length() - 1] == ';') {
                    srcText_str.erase(srcText_str.length() - 1);
                  }
                  a_class.fields.push_back(srcText_str);
                }
                i++;
              }
              i = 1;
              while (i < a_class.fields.size()) {
                if (a_class.fields[i].substr(
                        0, a_class.fields[i].find_last_of('}') + 1) ==
                    a_class.fields[i - 1].substr(
                        0, a_class.fields[i - 1].find_last_of('}') + 1)) {
                  a_class.fields.erase(a_class.fields.begin() + i - 1);
                } else {
                  i++;
                }
              }
              i = 0;
              while (i < a_class.fields.size()) {
                if (a_class.fields[i] == "union" ||
                    a_class.fields[i] == "struct" ||
                    a_class.fields[i] == "class" ||
                    a_class.fields[i] == "enum") {
                  a_class.fields.erase(a_class.fields.begin() + i);
                } else {
                  i++;
                }
              }
              gac_classes.push_back(a_class);
            }
            int i = 0;
            while (i < gac_classes.size()) {
              if (class_name == gac_classes[i].class_name) {
                break;
              }
              i++;
            }
            std::string signature = get_signature(Func);
            if (gac_all_context_paths->hasClassMethodPath(class_name,
                                                          signature)) {
              if (isa<CXXConstructorDecl>(Func)) {
                // std::cout << "Constructor Declaration " << class_name
                //           << std::endl;
                // std::string signature;
                // std::string function_body;
                // std::vector<std::string> parameters;
                // std::vector<Application> applications;
                Constructor constructor;
                const SourceManager &sourceManager =
                    Context->getSourceManager();
                bool Invalid = false;
                constructor.signature = signature;
                for (const ParmVarDecl *param : Func->parameters()) {
                  SourceRange srcRange_p = param->getSourceRange();
                  srcRange_p.setEnd(Lexer::getLocForEndOfToken(
                      srcRange_p.getEnd(), 0, sourceManager,
                      Context->getLangOpts()));
                  StringRef srcText_p = Lexer::getSourceText(
                      CharSourceRange::getTokenRange(srcRange_p), sourceManager,
                      Context->getLangOpts(), &Invalid);
                  std::string srcText_str = std::string(srcText_p);
                  if (srcText_str[srcText_str.length() - 1] == ')' ||
                      srcText_str[srcText_str.length() - 1] == ',' ||
                      srcText_str[srcText_str.length() - 1] == ';') {
                    srcText_str.erase(srcText_str.length() - 1);
                  }
                  constructor.parameters.push_back(srcText_str);
                }
                SourceRange srcRange = Func->getSourceRange();
                srcRange.setEnd(Lexer::getLocForEndOfToken(
                    srcRange.getEnd(), 0, sourceManager,
                    Context->getLangOpts()));
                StringRef srcText = Lexer::getSourceText(
                    CharSourceRange::getTokenRange(srcRange), sourceManager,
                    Context->getLangOpts(), &Invalid);
                constructor.function_body = srcText;
                StmtVarFunctionVisitor function_stmt_visitor(Context);
                function_stmt_visitor.TraverseStmt(Func->getBody());
                constructor.applications =
                    function_stmt_visitor.get_applications();
                gac_classes[i].constructors.push_back(constructor);
              } else if (isa<CXXDestructorDecl>(Func)) {
                // std::string signature;
                // std::string function_body;
                // std::vector<Application> applications;
                // std::cout << "Destructor Declaration " << class_name
                //           << std::endl;
                Destructor destructor;
                const SourceManager &sourceManager =
                    Context->getSourceManager();
                SourceRange srcRange = Func->getSourceRange();
                srcRange.setEnd(Lexer::getLocForEndOfToken(
                    srcRange.getEnd(), 0, sourceManager,
                    Context->getLangOpts()));
                bool Invalid = false;
                StringRef srcText = Lexer::getSourceText(
                    CharSourceRange::getTokenRange(srcRange), sourceManager,
                    Context->getLangOpts(), &Invalid);
                destructor.signature = signature;
                destructor.function_body = srcText;
                StmtVarFunctionVisitor function_stmt_visitor(Context);
                function_stmt_visitor.TraverseStmt(Func->getBody());
                destructor.applications =
                    function_stmt_visitor.get_applications();
                gac_classes[i].destructor = destructor;
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
                method.method_name = function_name;
                method.signature = signature;
                const SourceManager &sourceManager =
                    Context->getSourceManager();
                SourceRange srcRange_r = Func->getReturnTypeSourceRange();
                srcRange_r.setEnd(Lexer::getLocForEndOfToken(
                    srcRange_r.getEnd(), 0, sourceManager,
                    Context->getLangOpts()));
                bool Invalid = false;
                StringRef srcText_r = Lexer::getSourceText(
                    CharSourceRange::getTokenRange(srcRange_r), sourceManager,
                    Context->getLangOpts(), &Invalid);
                method.return_type = std::string(srcText_r);
                for (const ParmVarDecl *param : Func->parameters()) {
                  SourceRange srcRange_p = param->getSourceRange();
                  srcRange_p.setEnd(Lexer::getLocForEndOfToken(
                      srcRange_p.getEnd(), 0, sourceManager,
                      Context->getLangOpts()));
                  StringRef srcText_p = Lexer::getSourceText(
                      CharSourceRange::getTokenRange(srcRange_p), sourceManager,
                      Context->getLangOpts(), &Invalid);
                  std::string srcText_str = std::string(srcText_p);
                  if (srcText_str[srcText_str.length() - 1] == ')' ||
                      srcText_str[srcText_str.length() - 1] == ',' ||
                      srcText_str[srcText_str.length() - 1] == ';') {
                    srcText_str.erase(srcText_str.length() - 1);
                  }
                  method.parameters.push_back(srcText_str);
                }
                SourceRange srcRange = Func->getSourceRange();
                srcRange.setEnd(Lexer::getLocForEndOfToken(
                    srcRange.getEnd(), 0, sourceManager,
                    Context->getLangOpts()));
                StringRef srcText = Lexer::getSourceText(
                    CharSourceRange::getTokenRange(srcRange), sourceManager,
                    Context->getLangOpts(), &Invalid);
                method.function_body = srcText;
                StmtVarFunctionVisitor function_stmt_visitor(Context);
                function_stmt_visitor.TraverseStmt(Func->getBody());
                method.applications = function_stmt_visitor.get_applications();
                gac_classes[i].methods.push_back(method);
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

void OneFileContext::get_context() {
  ::gac_classes.clear();
  ::gac_functions.clear();
  Tool.run(newFrontendActionFactory<MethodFunctionAction>().get());
  file_context.classes = ::gac_classes;
  file_context.functions = ::gac_functions;
}

FileContext OneFileContext::get_file_context() { return file_context; }

Class FileContext::get_simple_class(std::string class_name) {
  for (auto a_class : classes) {
    if (a_class.class_name == class_name) {
      Class ret;
      ret.class_name = a_class.class_name;
      ret.base_class = a_class.base_class;
      ret.fields = a_class.fields;
      ret.its_namespace = a_class.its_namespace;
      for (auto constructor : a_class.constructors) {
        Constructor new_constructor;
        new_constructor.signature = constructor.signature;
        ret.constructors.push_back(new_constructor);
      }
      Destructor new_destructor;
      new_destructor.signature = a_class.destructor.signature;
      ret.destructor = new_destructor;
      for (auto method : a_class.methods) {
        Method new_method;
        new_method.signature = method.signature;
        ret.methods.push_back(new_method);
      }
      return ret;
    }
  }
}

bool FileContext::class_has_constructor(std::string class_name,
                                        std::string signature) {
  for (auto a_class : classes) {
    if (a_class.class_name == class_name) {
      for (auto constructor : a_class.constructors) {
        if (constructor.signature == signature) {
          return 1;
        }
      }
      return 0;
    }
  }
  return 0;
}

Constructor FileContext::class_get_constructor(std::string class_name,
                                               std::string signature) {
  for (auto a_class : classes) {
    if (a_class.class_name == class_name) {
      for (auto constructor : a_class.constructors) {
        if (constructor.signature == signature) {
          return constructor;
        }
      }
    }
  }
}

bool FileContext::class_has_destructor(std::string class_name,
                                       std::string signature) {
  for (auto a_class : classes) {
    if (a_class.class_name == class_name) {
      if (a_class.destructor.signature == signature) {
        return 1;
      }
      return 0;
    }
  }
  return 0;
}
Destructor FileContext::class_get_destructor(std::string class_name,
                                             std::string signature) {
  for (auto a_class : classes) {
    if (a_class.class_name == class_name) {
      return a_class.destructor;
    }
  }
}

bool FileContext::class_has_method(std::string class_name,
                                   std::string signature) {
  for (auto a_class : classes) {
    if (a_class.class_name == class_name) {
      for (auto method : a_class.methods) {
        if (method.signature == signature) {
          return 1;
        }
      }
      return 0;
    }
  }
  return 0;
}
Method FileContext::class_get_method(std::string class_name,
                                     std::string signature) {
  for (auto a_class : classes) {
    if (a_class.class_name == class_name) {
      for (auto method : a_class.methods) {
        if (method.signature == signature) {
          return method;
        }
      }
    }
  }
}

Function FileContext::get_function(std::string signature) {
  for (auto function : functions) {
    if (function.signature == signature) {
      return function;
    }
  }
}

std::vector<TestMacro>
FileContext::get_may_test_macros(std::string class_name,
                                 std::string function_name) {
  std::vector<TestMacro> ret;
  for (auto a_test_macro : test_macros) {
    if (class_name == "class") {
      if (a_test_macro.second_parameter.find(function_name) !=
          std::string::npos) {
        ret.push_back(a_test_macro);
      }
    } else {
      if (a_test_macro.second_parameter.find(class_name) != std::string::npos &&
          a_test_macro.second_parameter.find(function_name) !=
              std::string::npos) {
        ret.push_back(a_test_macro);
      }
    }
  }
  return ret;
}

std::vector<TestMacro>
FileContext::get_must_test_macros(std::string second_parameter) {
  std::vector<TestMacro> ret;
  for (auto a_test_macro : test_macros) {
    if (a_test_macro.second_parameter == second_parameter) {
      ret.push_back(a_test_macro);
      return ret;
    }
  }
  return ret;
}

void FileContext::cout() {
  std::cout << file_path << std::endl;
  //   std::vector<std::string> includes;
  //   std::vector<ADefine> defines;
  //   std::vector<GlobalVar> global_vars;
  //   std::vector<Class> classes;
  //   std::vector<Function> functions;
  std::cout << "includes:" << std::endl;
  for (auto include : includes) {
    std::cout << include << std::endl;
  }
  std::cout << "defines:" << std::endl;
  for (auto define : defines) {
    std::cout << define.define_name << " " << define.define_body << std::endl;
  }
  std::cout << "global_vars:" << std::endl;
  for (auto global_var : global_vars) {
    std::cout << global_var.global_var << " " << global_var.its_namespace
              << std::endl;
    applications_cout(global_var.applications);
  }
  std::cout << "classes:" << std::endl;
  for (auto one_class : classes) {
    std::cout << one_class.class_name << std::endl;
  }
  std::cout << "functions:" << std::endl;
  for (auto function : functions) {
    std::cout << function.function_name << std::endl;
    applications_cout(function.applications);
  }
}

void get_all_applications(std::set<Application> all_applications,
                          std::string a_file_path, std::string class_name,
                          std::string signature);
#include "GetAllPath.h"
#include "GetSignature.h"

void AllContextPaths::append(ClassMethodandPath class_method_path) {
  class_method_paths.push_back(class_method_path);
}

void AllContextPaths::append(FunctionandPath function_path) {
  function_paths.push_back(function_path);
}

std::string AllContextPaths::getClassMethodPath(std::string class_name,
                                                std::string signature) {
  for (auto class_method_path : class_method_paths) {
    if (class_method_path.class_name == class_name &&
        class_method_path.signature == signature) {
      return class_method_path.path;
    }
  }
}
std::string AllContextPaths::getFunctionPath(std::string signature) {
  for (auto function_path : function_paths) {
    if (function_path.signature == signature) {
      return function_path.path;
    }
  }
}

bool AllContextPaths::hasClassMethodPath(std::string class_name,
                                         std::string signature) {
  for (auto class_method_path : class_method_paths) {
    if (class_method_path.class_name == class_name &&
        class_method_path.signature == signature) {
      return 1;
    }
  }
  return 0;
}
bool AllContextPaths::hasFunctionPath(std::string signature) {
  for (auto function_path : function_paths) {
    if (function_path.signature == signature) {
      return 1;
    }
  }
  return 0;
}

void AllContextPaths::cout() {
  for (auto class_method_path : class_method_paths) {
    std::cout << "Class:" << class_method_path.class_name << "\t"
              << "Method:" << class_method_path.signature << "\t"
              << "Path:" << class_method_path.path << std::endl;
  }
  for (auto function_path : function_paths) {
    std::cout << "Function:" << function_path.signature << "\t"
              << "Path:" << function_path.path << std::endl;
  }
}

std::vector<std::pair<std::string, std::pair<std::string, std::string>>>
AllContextPaths::getSameTest(std::string function_name) {
  std::vector<std::pair<std::string, std::pair<std::string, std::string>>> ret;
  for (auto class_method_path : class_method_paths) {
    bool b = 0;
    for (int j = 0; j < ret.size(); j++) {
      if (ret[j].first == class_method_path.class_name) {
        b = 1;
      }
    }
    if (b == 1) {
      continue;
    }
    auto key = class_method_path.class_name;
    size_t begin = key.find_first_of('_');
    size_t end = key.substr(begin + 1).find_first_of('_');
    std::string true_key = key.substr(begin + 1, end);
    if (class_method_path.class_name.find("Test") != std::string::npos &&
        true_key.find(function_name) != std::string::npos &&
        class_method_path.signature.find("TestBody") != std::string::npos) {
      std::pair<std::string, std::string> class_function_name(
          class_method_path.class_name, class_method_path.signature);
      ret.push_back(std::pair<std::string, std::pair<std::string, std::string>>(
          class_method_path.path, class_function_name));
    }
  }
  return ret;
}

std::vector<std::pair<std::string, std::pair<std::string, std::string>>>
AllContextPaths::getDifferentTest(std::string second_parameter) {
  std::vector<std::pair<std::string, std::pair<std::string, std::string>>> ret;
  for (auto class_method_path : class_method_paths) {
    bool b = 0;
    for (int j = 0; j < ret.size(); j++) {
      if (ret[j].first == class_method_path.class_name) {
        b = 1;
      }
    }
    if (b == 1) {
      continue;
    }
    auto key = class_method_path.class_name;
    size_t begin = key.find_first_of('_');
    size_t end = key.substr(begin + 1).find_first_of('_');
    std::string true_key = key.substr(begin + 1, end);
    if (class_method_path.class_name.find("Test") != std::string::npos &&
        true_key == second_parameter &&
        class_method_path.signature.find("TestBody") != std::string::npos) {
      std::pair<std::string, std::string> class_function_name(
          class_method_path.class_name, class_method_path.signature);
      ret.push_back(std::pair<std::string, std::pair<std::string, std::string>>(
          class_method_path.path, class_function_name));
    }
  }
  return ret;
}

bool AllContextPaths::hasClass(std::string class_name) {
  for (auto class_method_path : class_method_paths) {
    if (class_method_path.class_name == class_name) {
      return 1;
    }
  }
  return 0;
}
std::vector<std::pair<std::string, std::string>>
AllContextPaths::getClassConstructor(std::string class_name) {
  std::vector<std::pair<std::string, std::string>> ret;
  for (auto class_method_path : class_method_paths) {
    if (class_method_path.class_name == class_name) {
      auto method_name = class_method_path.signature;
      size_t end = method_name.find_first_of('(');
      method_name = method_name.substr(0, end);
      size_t begin = method_name.find_last_of(':');
      method_name = method_name.substr(begin + 1);
      if (method_name == class_name) {
        ret.push_back(std::pair<std::string, std::string>(
            class_method_path.signature, class_method_path.path));
      }
    }
  }
  return ret;
}

std::pair<std::string, std::string>
AllContextPaths::getClassDestructor(std::string class_name) {
  for (auto class_method_path : class_method_paths) {
    if (class_method_path.class_name == class_name) {
      auto method_name = class_method_path.signature;
      size_t end = method_name.find_first_of('(');
      method_name = method_name.substr(0, end);
      size_t begin = method_name.find_last_of(':');
      method_name = method_name.substr(begin + 1);
      if (method_name == "~" + class_name) {
        return std::pair<std::string, std::string>(class_method_path.signature,
                                                   class_method_path.path);
      }
    }
  }
  return std::pair<std::string, std::string>("class", "class");
}

AllContextPaths *gap_context_paths;
std::string gap_project_path;

class FunctionMethodFinder : public RecursiveASTVisitor<FunctionMethodFinder> {
public:
  explicit FunctionMethodFinder(ASTContext *Context) : Context(Context) {}

  bool VisitFunctionDecl(FunctionDecl *Function) {
    if (Function->isThisDeclarationADefinition()) {
      clang::SourceLocation loc = Function->getLocation();
      clang::SourceManager &SM = Context->getSourceManager();
      clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
      std::string file_path = presumedLoc.getFilename();
      if (file_path.find(gap_project_path) != std::string::npos) {
        if (!isa<CXXMethodDecl>(Function)) {
          std::string signature = get_signature(Function);
          if (!gap_context_paths->hasFunctionPath(signature)) {
            FunctionandPath function_path;
            function_path.signature = signature;
            function_path.path = file_path;
            gap_context_paths->append(function_path);
          }
        } else {
          const CXXRecordDecl *ParentClass =
              dyn_cast<CXXRecordDecl>(Function->getParent());
          if (isa<CXXConstructorDecl>(Function)) {
            std::string class_name = ParentClass->getNameAsString();
            std::string signature = get_signature(Function);
            if (!gap_context_paths->hasClassMethodPath(class_name, signature)) {
              ClassMethodandPath class_method_path;
              class_method_path.class_name = class_name;
              class_method_path.signature = signature;
              class_method_path.path = file_path;
              gap_context_paths->append(class_method_path);
            }
          } else if (isa<CXXDestructorDecl>(Function)) {
            std::string class_name = ParentClass->getNameAsString();
            std::string signature = get_signature(Function);
            if (!gap_context_paths->hasClassMethodPath(class_name, signature)) {
              ClassMethodandPath class_method_path;
              class_method_path.class_name = class_name;
              class_method_path.signature = signature;
              class_method_path.path = file_path;
              gap_context_paths->append(class_method_path);
            }
          } else {
            std::string class_name = ParentClass->getNameAsString();
            std::string signature = get_signature(Function);
            if (!gap_context_paths->hasClassMethodPath(class_name, signature)) {
              ClassMethodandPath class_method_path;
              class_method_path.class_name = class_name;
              class_method_path.signature = signature;
              class_method_path.path = file_path;
              gap_context_paths->append(class_method_path);
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

class FunctionMethodConsumer : public clang::ASTConsumer {
public:
  explicit FunctionMethodConsumer(ASTContext *Context) : Visitor(Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  FunctionMethodFinder Visitor;
};

class FunctionMethodAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::make_unique<FunctionMethodConsumer>(&Compiler.getASTContext());
  }
};

void get_all_paths(ClangTool &Tool, AllContextPaths *all_context_paths,
                   std::string project_path) {
  gap_context_paths = all_context_paths;
  gap_project_path = project_path;
  Tool.run(newFrontendActionFactory<FunctionMethodAction>().get());
}
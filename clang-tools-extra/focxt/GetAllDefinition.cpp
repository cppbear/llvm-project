#include <GetAllDefinition.h>
#include <GetSignature.h>

std::string gad_project_path;
ClassesAndFunctions gad_classes_and_functions;

bool ClassesAndFunctions::has_class(std::string class_name) {
  for (auto a_class : classes) {
    if (a_class.class_name == class_name) {
      return 1;
    }
  }
  return 0;
}

void ClassesAndFunctions::push_back_class(Class a_class) {
  classes.push_back(a_class);
}

void ClassesAndFunctions::push_back_specialization_parameter(
    std::string class_name, std::string signature,
    std::vector<std::string> specialization_parameter) {
  if (signature == "") {
    for (auto &a_class : classes) {
      if (a_class.class_name == class_name) {
        bool b = 0;
        for (auto exist_specialization_parameter :
             a_class.specialization_parameters) {
          if (exist_specialization_parameter == specialization_parameter) {
            b = 1;
            break;
          }
        }
        if (b == 0) {
          a_class.specialization_parameters.push_back(specialization_parameter);
          for (auto specialization_type : specialization_parameter) {
            Application application;
            application.class_name = specialization_type;
            application.signature = "";
            a_class.applications.push_back(application);
            // if (this->has_class(specialization_type)) {
            //   std::vector<std::string> constructors =
            //       this->get_constructors(specialization_type);
            //   std::vector<std::pair<std::string, std::string>> applications;
            //   for (auto constructor : constructors) {
            //     applications.push_back(std::pair<std::string, std::string>(
            //         specialization_type, constructor));
            //   }
            //   std::string destructor = this->get_destructor(class_name);
            //   if (destructor != "class") {
            //     applications.push_back(std::pair<std::string, std::string>(
            //         specialization_type, destructor));
            //   }
            //   this->push_back_applications(class_name, "", applications);
            // }
          }
        }
      }
    }
  } else {
    if (class_name == "class") {
      for (auto &function : functions) {
        if (function.signature == signature) {
          bool b = 0;
          for (auto exist_specialization_parameter :
               function.specialization_parameters) {
            if (exist_specialization_parameter == specialization_parameter) {
              b = 1;
              break;
            }
          }
          if (b == 0) {
            function.specialization_parameters.push_back(
                specialization_parameter);
            for (auto specialization_type : specialization_parameter) {
              Application application;
              application.class_name = specialization_type;
              application.signature = "";
              function.applications.push_back(application);
              // if (this->has_class(specialization_type)) {
              //   std::vector<std::string> constructors =
              //       this->get_constructors(specialization_type);
              //   std::vector<std::pair<std::string, std::string>>
              //   applications; for (auto constructor : constructors) {
              //     applications.push_back(std::pair<std::string, std::string>(
              //         specialization_type, constructor));
              //   }
              //   std::string destructor = this->get_destructor(class_name);
              //   if (destructor != "class") {
              //     applications.push_back(std::pair<std::string, std::string>(
              //         specialization_type, destructor));
              //   }
              //   this->push_back_applications("class", signature,
              //   applications);
              // }
            }
          }
        }
      }
    } else {
      for (auto &a_class : classes) {
        if (a_class.class_name == class_name) {
          for (auto &method : a_class.methods) {
            if (method.signature == signature) {
              bool b = 0;
              for (auto exist_specialization_parameter :
                   method.specialization_parameters) {
                if (exist_specialization_parameter ==
                    specialization_parameter) {
                  b = 1;
                  break;
                }
              }
              if (b == 0) {
                method.specialization_parameters.push_back(
                    specialization_parameter);
                for (auto specialization_type : specialization_parameter) {
                  Application application;
                  application.class_name = specialization_type;
                  application.signature = "";
                  method.applications.push_back(application);
                  // if (this->has_class(specialization_type)) {
                  //   std::vector<std::string> constructors =
                  //       this->get_constructors(specialization_type);
                  //   std::vector<std::pair<std::string, std::string>>
                  //       applications;
                  //   for (auto constructor : constructors) {
                  //     applications.push_back(
                  //         std::pair<std::string, std::string>(
                  //             specialization_type, constructor));
                  //   }
                  //   std::string destructor =
                  //   this->get_destructor(class_name); if (destructor !=
                  //   "class") {
                  //     applications.push_back(
                  //         std::pair<std::string, std::string>(
                  //             specialization_type, destructor));
                  //   }
                  //   this->push_back_applications(class_name, signature,
                  //                                applications);
                  // }
                }
              }
            }
          }
        }
      }
    }
  }
}

void ClassesAndFunctions::push_back_applications(
    std::string class_name, std::string signature,
    std::vector<std::pair<std::string, std::string>> applications) {
  if (signature == "") {
    for (auto &a_class : classes) {
      if (a_class.class_name == class_name) {
        for (auto application : applications) {
          Application a_application;
          a_application.class_name = application.first;
          a_application.signature = application.second;
          a_class.applications.push_back(a_application);
        }
      }
    }
  } else {
    if (class_name == "class") {
      for (auto &function : functions) {
        if (function.signature == signature) {
          for (auto application : applications) {
            Application a_application;
            a_application.class_name = application.first;
            a_application.signature = application.second;
            function.applications.push_back(a_application);
          }
        }
      }
    } else {
      for (auto &a_class : classes) {
        if (a_class.class_name == class_name) {
          for (auto &method : a_class.methods) {
            if (method.signature == signature) {
              for (auto application : applications) {
                Application a_application;
                a_application.class_name = application.first;
                a_application.signature = application.second;
                method.applications.push_back(a_application);
              }
            }
          }
        }
      }
    }
  }
}

std::vector<std::string>
ClassesAndFunctions::get_constructors(std::string class_name) {
  std::vector<std::string> constructors;
  for (auto a_class : classes) {
    if (a_class.class_name == class_name) {
      for (auto constructor : a_class.constructors) {
        constructors.push_back(constructor.signature);
      }
      break;
    }
  }
  return constructors;
}

std::string ClassesAndFunctions::get_destructor(std::string class_name) {
  for (auto a_class : classes) {
    if (a_class.class_name == class_name &&
        a_class.destructor.signature != "") {
      return a_class.destructor.signature;
    }
    break;
  }
  return "class";
}

bool ClassesAndFunctions::has_function(std::string class_name,
                                       std::string signature) {
  if (class_name == "class") {
    for (auto function : functions) {
      if (function.signature == signature) {
        return 1;
      }
    }
    return 0;
  } else {
    for (auto a_class : classes) {
      if (a_class.class_name == class_name) {
        for (auto constructor : a_class.constructors) {
          if (constructor.signature == signature) {
            return 1;
          }
        }
        if (a_class.destructor.signature == signature) {
          return 1;
        }
        for (auto method : a_class.methods) {
          if (method.signature == signature) {
            return 1;
          }
        }
        return 0;
      }
    }
  }
  return 0;
}

void ClassesAndFunctions::push_back_function(Function function) {
  functions.push_back(function);
}

void ClassesAndFunctions::push_back_constructor(std::string class_name,
                                                Constructor constructor) {
  for (auto &a_class : classes) {
    if (a_class.class_name == class_name) {
      a_class.constructors.push_back(constructor);
    }
  }
}

void ClassesAndFunctions::push_back_destructor(std::string class_name,
                                               Destructor destructor) {
  for (auto &a_class : classes) {
    if (a_class.class_name == class_name) {
      a_class.destructor = destructor;
    }
  }
}

void ClassesAndFunctions::push_back_method(std::string class_name,
                                           Method method) {
  for (auto &a_class : classes) {
    if (a_class.class_name == class_name) {
      a_class.methods.push_back(method);
    }
  }
}

void ClassesAndFunctions::cout() {
  std::cout << "class:" << std::endl;
  for (auto a_class : classes) {
    std::cout << a_class.class_name << "\t" << a_class.base_class << "\t"
              << a_class.is_template << std::endl
              << "template_parameters:";
    for (auto template_parameter : a_class.template_parameters) {
      std::cout << template_parameter << "\t";
    }
    std::cout << std::endl << "specialization_parameters:";
    for (auto specialization_parameter : a_class.specialization_parameters) {
      std::cout << "<";
      for (auto specialization_type : specialization_parameter) {
        std::cout << specialization_type << ",";
      }
      std::cout << ">\t";
    }
    std::cout << std::endl << "fields:";
    for (auto field : a_class.fields) {
      std::cout << field << "\t";
    }
    std::cout << std::endl << "alias:";
    for (auto alias : a_class.aliases) {
      std::cout << "<" << alias.alias_name << "," << alias.base_name << ">\t";
    }
    std::cout << std::endl
              << "namespace:" << a_class.its_namespace << std::endl
              << "constructors:";
    for (auto constructor : a_class.constructors) {
      std::cout << constructor.signature << "\t";
      for (auto application : constructor.applications) {
        application.cout();
      }
      std::cout << std::endl;
    }
    std::cout << std::endl
              << "destructor:" << a_class.destructor.signature << "\t";
    for (auto application : a_class.destructor.applications) {
      application.cout();
    }
    std::cout << std::endl;
    std::cout << std::endl << "methods:";
    for (auto method : a_class.methods) {
      std::cout << method.signature << "\t" << method.is_template << std::endl
                << "template_parameters:";
      for (auto template_parameter : method.template_parameters) {
        std::cout << template_parameter << "\t";
      }
      std::cout << std::endl << "specialization_parameters:";
      for (auto specialization_parameter : method.specialization_parameters) {
        std::cout << "<";
        for (auto specialization_type : specialization_parameter) {
          std::cout << specialization_type << ",";
        }
        std::cout << ">\t";
      }
      for (auto application : method.applications) {
        application.cout();
      }
      std::cout << std::endl;
    }
    std::cout << std::endl << "applications:";
    for (auto application : a_class.applications) {
      application.cout();
    }
    std::cout << std::endl << std::endl;
  }
  std::cout << "functions:" << std::endl;
  for (auto function : functions) {
    std::cout << function.function_name << "\t" << function.signature << "\t"
              << function.is_template << std::endl
              << "template_parameters:";
    for (auto template_parameter : function.template_parameters) {
      std::cout << template_parameter << "\t";
    }
    std::cout << std::endl << "specialization_parameters:";
    for (auto specialization_parameter : function.specialization_parameters) {
      std::cout << "<";
      for (auto specialization_type : specialization_parameter) {
        std::cout << specialization_type << ",";
      }
      std::cout << ">\t";
    }
    std::cout << std::endl
              << "namespace:" << function.its_namespace << std::endl
              << "applications:";
    for (auto application : function.applications) {
      application.cout();
    }
    std::cout << std::endl << std::endl;
  }
}

void update_applications(std::vector<Application> &applications) {
  for (auto it = applications.begin(); it != applications.end();) {
    if (it->signature == "" &&
        !gad_classes_and_functions.has_class(it->class_name)) {
      it = applications.erase(it);
      continue;
    } else if (!gad_classes_and_functions.has_function(it->class_name,
                                                       it->signature)) {
      it = applications.erase(it);
      continue;
    } else {
      ++it;
      continue;
    }
  }
  for (auto it = applications.begin(); it != applications.end();) {
    if (it->signature == "") {
      std::string class_name = it->class_name;
      it = applications.erase(it);
      std::vector<std::string> need_applications;
      need_applications =
          gad_classes_and_functions.get_constructors(class_name);
      std::string need_destructor =
          gad_classes_and_functions.get_destructor(class_name);
      if (need_destructor != "class") {
        need_applications.push_back(need_destructor);
      }
      for (auto need_application : need_applications) {
        Application application;
        application.class_name = class_name;
        application.signature = need_application;
        applications.push_back(application);
      }
    } else {
      ++it;
    }
  }
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

void ClassesAndFunctions::update_all_applications() {
  for (auto &a_class : classes) {
    update_applications(a_class.applications);
    for (auto &constructor : a_class.constructors) {
      update_applications(constructor.applications);
    }
    update_applications(a_class.destructor.applications);
    for (auto &method : a_class.methods) {
      update_applications(method.applications);
    }
  }
  for (auto &function : functions) {
    update_applications(function.applications);
  }
}

bool StmtVarFunctionVisitor::VisitCallExpr(CallExpr *Call) {
  FunctionDecl *Callee = Call->getDirectCallee();
  if (Callee) {
    if (Callee->getPrimaryTemplate()) {
      Callee = Callee->getPrimaryTemplate()->getTemplatedDecl();
    }
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
      //   std::cout << class_name << std::endl;
      //   std::cout << signature << std::endl;
      // if (gad_classes_and_functions.has_function(class_name, signature)) {
      Application application;
      application.class_name = class_name;
      application.signature = signature;
      applications.push_back(application);
      // }
    } else {
      // std::string file_path;
      // std::string class_name;
      // std::string function_name;
      // std::string signature;
      std::string function_name = Callee->getNameAsString();
      // std::cout << "Function Call " << function_name << std::endl;
      std::string signature = get_signature(Callee);
      //   std::cout << signature << std::endl;
      // if (gad_classes_and_functions.has_function("class", signature)) {
      Application application;
      application.class_name = "class";
      application.signature = signature;
      applications.push_back(application);
      // }
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
    if (class_name.find(" ") != std::string::npos) {
      class_name = class_name.substr(class_name.find_last_of(" ") + 1);
    }
    Application application;
    application.class_name = class_name;
    application.signature = "";
    applications.push_back(application);
    // if (gad_classes_and_functions.has_class(class_name)) {
    //   std::vector<std::string> need_constructors =
    //       gad_classes_and_functions.get_constructors(class_name);
    //   for (int i = 0; i < need_constructors.size(); i++) {
    //     std::string function_name = class_name;
    //     std::string signature = need_constructors[i];
    //     Application application;
    //     application.class_name = class_name;
    //     application.signature = signature;
    //     applications.push_back(application);
    //   }
    //   std::string need_destructor =
    //       gad_classes_and_functions.get_destructor(class_name);
    //   if (need_destructor != "class") {
    //     std::string function_name = "~" + class_name;
    //     std::string signature = need_destructor;
    //     Application application;
    //     application.class_name = class_name;
    //     application.signature = signature;
    //     applications.push_back(application);
    //   }
    // }
  }
  return true;
}

std::vector<Application> StmtVarFunctionVisitor::get_applications() {
  return applications;
}

class FindClassesAndFunctionsVisitor
    : public RecursiveASTVisitor<FindClassesAndFunctionsVisitor> {
public:
  explicit FindClassesAndFunctionsVisitor(ASTContext *Context)
      : Context(Context) {}

  bool VisitClassTemplateDecl(ClassTemplateDecl *Declaration) {
    if (Declaration->isThisDeclarationADefinition()) {
      clang::SourceLocation loc = Declaration->getLocation();
      clang::SourceManager &SM = Context->getSourceManager();
      clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
      std::string file_path = presumedLoc.getFilename();
      if (file_path.find(gad_project_path) != std::string::npos) {
        // std::cout << Declaration->getNameAsString() << std::endl;
        // Declaration->dump();
        // std::cout << std::endl;
        std::string class_name = Declaration->getNameAsString();
        if (!gad_classes_and_functions.has_class(class_name)) {
          CXXRecordDecl *RecordDecl = Declaration->getTemplatedDecl();
          Class a_class;
          a_class.class_name = class_name;
          a_class.is_template = 1;
          auto template_parameters = Declaration->getTemplateParameters();
          for (auto template_parameter : template_parameters->asArray()) {
            a_class.template_parameters.push_back(
                template_parameter->getNameAsString());
          }
          if (RecordDecl->getNumBases() > 0) {
            a_class.base_class =
                RecordDecl->bases_begin()->getType().getAsString();
          }
          const DeclContext *context = RecordDecl->getDeclContext();
          bool b = 0;
          while (context) {
            if (const NamespaceDecl *namespaceDecl =
                    dyn_cast<NamespaceDecl>(context)) {
              b = 1;
              if (a_class.its_namespace == "") {
                a_class.its_namespace = namespaceDecl->getNameAsString();
              } else {
                a_class.its_namespace = namespaceDecl->getNameAsString() +
                                        "::" + a_class.its_namespace;
              }
            }
            context = context->getParent();
          }
          if (b == 0) {
            a_class.its_namespace = "";
          }
          int i = 0;
          for (const auto *decl : RecordDecl->decls()) {
            if (const CXXRecordDecl *Record = dyn_cast<CXXRecordDecl>(decl)) {
              if (i == 0) {
                continue;
              }
              const SourceManager &sourceManager = Context->getSourceManager();
              SourceRange srcRange = Record->getSourceRange();
              srcRange.setEnd(Lexer::getLocForEndOfToken(
                  srcRange.getEnd(), 0, sourceManager, Context->getLangOpts()));
              bool Invalid = false;
              StringRef srcText = Lexer::getSourceText(
                  CharSourceRange::getTokenRange(srcRange), sourceManager,
                  Context->getLangOpts(), &Invalid);
              // std::cout << std::string(srcText) << std::endl;
              std::string srcText_str = std::string(srcText);
              if (srcText_str[srcText_str.length() - 1] == ';') {
                srcText_str.erase(srcText_str.length() - 1);
              }
              // if (Record->isClass()) {
              //   if (srcText.find('{') != std::string::npos) {
              //     srcText_str.erase(srcText_str.find_first_of('{'));
              //   }
              //   a_class.fields.push_back(srcText_str);
              // } else {
              //   a_class.fields.push_back(srcText_str);
              // }
              a_class.fields.push_back(srcText_str);
            } else if (const FieldDecl *Field = dyn_cast<FieldDecl>(decl)) {
              std::string field_type = Field->getType().getAsString();
              if (field_type.find(" ") != std::string::npos) {
                field_type =
                    field_type.substr(field_type.find_last_of(" ") + 1);
              }
              Application application;
              application.class_name = field_type;
              application.signature = "";
              a_class.applications.push_back(application);
              const SourceManager &sourceManager = Context->getSourceManager();
              SourceRange srcRange = Field->getSourceRange();
              srcRange.setEnd(Lexer::getLocForEndOfToken(
                  srcRange.getEnd(), 0, sourceManager, Context->getLangOpts()));
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
              const SourceManager &sourceManager = Context->getSourceManager();
              SourceRange srcRange = Enum->getSourceRange();
              srcRange.setEnd(Lexer::getLocForEndOfToken(
                  srcRange.getEnd(), 0, sourceManager, Context->getLangOpts()));
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
            } else if (const TypeAliasDecl *TypeAlias =
                           dyn_cast<TypeAliasDecl>(decl)) {
              Alias alias;
              alias.alias_name = TypeAlias->getNameAsString();
              alias.base_name = TypeAlias->getUnderlyingType().getAsString();
              a_class.aliases.push_back(alias);
              Application application;
              application.class_name = alias.base_name;
              application.signature = "";
              a_class.applications.push_back(application);
              // if (gad_classes_and_functions.has_class(alias.base_name)) {
              //   std::vector<std::string> constructors =
              //       gad_classes_and_functions.get_constructors(alias.base_name);
              //   std::vector<std::pair<std::string, std::string>>
              //   applications; for (auto constructor : constructors) {
              //     applications.push_back(std::pair<std::string, std::string>(
              //         alias.base_name, constructor));
              //   }
              //   std::string destructor =
              //       gad_classes_and_functions.get_destructor(alias.base_name);
              //   if (destructor != "class") {
              //     applications.push_back(std::pair<std::string, std::string>(
              //         alias.base_name, destructor));
              //   }
              //   for (auto application : applications) {
              //     Application a_application;
              //     a_application.class_name = application.first;
              //     a_application.signature = application.second;
              //     a_class.applications.push_back(a_application);
              //   }
              // }
            }
            i++;
          }
          i = 1;
          while (i < a_class.fields.size()) {
            if (a_class.fields[i] == a_class.fields[i - 1] ||
                (a_class.fields[i].substr(
                     0, a_class.fields[i].find_last_of('}') + 1) != "" &&
                 a_class.fields[i - 1].substr(
                     0, a_class.fields[i - 1].find_last_of('}') + 1) != "" &&
                 a_class.fields[i].substr(
                     0, a_class.fields[i].find_last_of('}') + 1) ==
                     a_class.fields[i - 1].substr(
                         0, a_class.fields[i - 1].find_last_of('}') + 1)) ||
                a_class.fields[i].find(a_class.fields[i - 1]) !=
                    std::string::npos) {
              a_class.fields.erase(a_class.fields.begin() + i - 1);
            } else {
              i++;
            }
          }
          i = 0;
          while (i < a_class.fields.size()) {
            if (a_class.fields[i] == "union" || a_class.fields[i] == "struct" ||
                a_class.fields[i] == "class" || a_class.fields[i] == "enum") {
              a_class.fields.erase(a_class.fields.begin() + i);
            } else {
              i++;
            }
          }
          //   std::cout << a_class.class_name << "\t" << a_class.base_class
          //             << std::endl;
          //   for (auto template_parameter : a_class.template_parameters) {
          //     std::cout << template_parameter << "\t";
          //   }
          //   std::cout << std::endl;
          gad_classes_and_functions.push_back_class(a_class);
        }
        for (auto specialization : Declaration->specializations()) {
          std::vector<std::string> specialization_parameter;
          for (unsigned i = 0; i < specialization->getTemplateArgs().size();
               ++i) {
            const TemplateArgument &Arg =
                specialization->getTemplateArgs().get(i);
            if (Arg.getKind() == TemplateArgument::Type) {
              //   std::cout << "Specialization Argument: "
              //             << Arg.getAsType().getAsString() << "\n";
              std::string type_name = Arg.getAsType().getAsString();
              if (type_name.find(" ") != std::string::npos) {
                type_name = type_name.substr(type_name.find_last_of(" ") + 1);
              }
              specialization_parameter.push_back(type_name);
            }
          }
          gad_classes_and_functions.push_back_specialization_parameter(
              class_name, "", specialization_parameter);
        }
        // gad_classes_and_functions.cout();
      }
    }
    return true;
  }

  bool VisitCXXRecordDecl(CXXRecordDecl *Declaration) {
    if (Declaration->isThisDeclarationADefinition()) {
      clang::SourceLocation loc = Declaration->getLocation();
      clang::SourceManager &SM = Context->getSourceManager();
      clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
      std::string file_path = presumedLoc.getFilename();
      if (file_path.find(gad_project_path) != std::string::npos) {
        // std::cout << Declaration->getNameAsString() << std::endl;
        // Declaration->dump();
        // std::cout << std::endl;
        std::string class_name = Declaration->getNameAsString();
        if (!gad_classes_and_functions.has_class(class_name)) {
          Class a_class;
          a_class.class_name = class_name;
          a_class.is_template = 0;
          if (Declaration->getNumBases() > 0) {
            a_class.base_class =
                Declaration->bases_begin()->getType().getAsString();
          }
          const DeclContext *context = Declaration->getDeclContext();
          bool b = 0;
          while (context) {
            if (const NamespaceDecl *namespaceDecl =
                    dyn_cast<NamespaceDecl>(context)) {
              b = 1;
              if (a_class.its_namespace == "") {
                a_class.its_namespace = namespaceDecl->getNameAsString();
              } else {
                a_class.its_namespace = namespaceDecl->getNameAsString() +
                                        "::" + a_class.its_namespace;
              }
            }
            context = context->getParent();
          }
          if (b == 0) {
            a_class.its_namespace = "";
          }
          int i = 0;
          for (const auto *decl : Declaration->decls()) {
            if (const CXXRecordDecl *Record = dyn_cast<CXXRecordDecl>(decl)) {
              if (i == 0) {
                continue;
              }
              const SourceManager &sourceManager = Context->getSourceManager();
              SourceRange srcRange = Record->getSourceRange();
              srcRange.setEnd(Lexer::getLocForEndOfToken(
                  srcRange.getEnd(), 0, sourceManager, Context->getLangOpts()));
              bool Invalid = false;
              StringRef srcText = Lexer::getSourceText(
                  CharSourceRange::getTokenRange(srcRange), sourceManager,
                  Context->getLangOpts(), &Invalid);
              // std::cout << std::string(srcText) << std::endl;
              std::string srcText_str = std::string(srcText);
              if (srcText_str[srcText_str.length() - 1] == ';') {
                srcText_str.erase(srcText_str.length() - 1);
              }
              // if (Record->isClass()) {
              //   if (srcText.find('{') != std::string::npos) {
              //     srcText_str.erase(srcText_str.find_first_of('{'));
              //   }
              //   a_class.fields.push_back(srcText_str);
              // } else {
              //   a_class.fields.push_back(srcText_str);
              // }
              a_class.fields.push_back(srcText_str);
            } else if (const FieldDecl *Field = dyn_cast<FieldDecl>(decl)) {
              std::string field_type = Field->getType().getAsString();
              if (field_type.find(" ") != std::string::npos) {
                field_type =
                    field_type.substr(field_type.find_last_of(" ") + 1);
              }
              Application application;
              application.class_name = field_type;
              application.signature = "";
              a_class.applications.push_back(application);
              const SourceManager &sourceManager = Context->getSourceManager();
              SourceRange srcRange = Field->getSourceRange();
              srcRange.setEnd(Lexer::getLocForEndOfToken(
                  srcRange.getEnd(), 0, sourceManager, Context->getLangOpts()));
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
              const SourceManager &sourceManager = Context->getSourceManager();
              SourceRange srcRange = Enum->getSourceRange();
              srcRange.setEnd(Lexer::getLocForEndOfToken(
                  srcRange.getEnd(), 0, sourceManager, Context->getLangOpts()));
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
            } else if (const TypeAliasDecl *TypeAlias =
                           dyn_cast<TypeAliasDecl>(decl)) {
              Alias alias;
              alias.alias_name = TypeAlias->getNameAsString();
              alias.base_name = TypeAlias->getUnderlyingType().getAsString();
              a_class.aliases.push_back(alias);
              Application application;
              application.class_name = alias.base_name;
              application.signature = "";
              a_class.applications.push_back(application);
              // if (gad_classes_and_functions.has_class(alias.base_name)) {
              //   std::vector<std::string> constructors =
              //       gad_classes_and_functions.get_constructors(alias.base_name);
              //   std::vector<std::pair<std::string, std::string>>
              //   applications; for (auto constructor : constructors) {
              //     applications.push_back(std::pair<std::string, std::string>(
              //         alias.base_name, constructor));
              //   }
              //   std::string destructor =
              //       gad_classes_and_functions.get_destructor(alias.base_name);
              //   if (destructor != "class") {
              //     applications.push_back(std::pair<std::string, std::string>(
              //         alias.base_name, destructor));
              //   }
              //   for (auto application : applications) {
              //     Application a_application;
              //     a_application.class_name = application.first;
              //     a_application.signature = application.second;
              //     a_class.applications.push_back(a_application);
              //   }
              // }
            }
            i++;
          }
          i = 1;
          while (i < a_class.fields.size()) {
            if (a_class.fields[i] == a_class.fields[i - 1] ||
                (a_class.fields[i].substr(
                     0, a_class.fields[i].find_last_of('}') + 1) != "" &&
                 a_class.fields[i - 1].substr(
                     0, a_class.fields[i - 1].find_last_of('}') + 1) != "" &&
                 a_class.fields[i].substr(
                     0, a_class.fields[i].find_last_of('}') + 1) ==
                     a_class.fields[i - 1].substr(
                         0, a_class.fields[i - 1].find_last_of('}') + 1)) ||
                a_class.fields[i].find(a_class.fields[i - 1]) !=
                    std::string::npos) {
              a_class.fields.erase(a_class.fields.begin() + i - 1);
            } else {
              i++;
            }
          }
          i = 0;
          while (i < a_class.fields.size()) {
            if (a_class.fields[i] == "union" || a_class.fields[i] == "struct" ||
                a_class.fields[i] == "class" || a_class.fields[i] == "enum") {
              a_class.fields.erase(a_class.fields.begin() + i);
            } else {
              i++;
            }
          }
          //   std::cout << a_class.class_name << "\t" << a_class.base_class
          //             << std::endl;
          //   for (auto template_parameter : a_class.template_parameters) {
          //     std::cout << template_parameter << "\t";
          //   }
          //   std::cout << std::endl;
          gad_classes_and_functions.push_back_class(a_class);
        }
        // gad_classes_and_functions.cout();
      }
    }
    return true;
  }

  bool VisitFunctionTemplateDecl(FunctionTemplateDecl *Declaration) {
    if (Declaration->isThisDeclarationADefinition()) {
      clang::SourceLocation loc = Declaration->getLocation();
      clang::SourceManager &SM = Context->getSourceManager();
      clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
      std::string file_path = presumedLoc.getFilename();
      if (file_path.find(gad_project_path) != std::string::npos) {
        //     std::cout << get_signature(Declaration->getAsFunction()) <<
        //     std::endl; Declaration->dump(); std::cout << std::endl;
        FunctionDecl *Func = Declaration->getTemplatedDecl();
        if (!isa<CXXMethodDecl>(Func)) {
          std::string function_name = Func->getNameAsString();
          std::string signature = get_signature(Func);
          if (!gad_classes_and_functions.has_function("class", signature)) {
            Function function;
            // std::vector<Application> applications;
            function.function_name = function_name;
            function.signature = signature;
            function.is_template = 1;
            auto template_parameters = Declaration->getTemplateParameters();
            for (auto template_parameter : template_parameters->asArray()) {
              function.template_parameters.push_back(
                  template_parameter->getNameAsString());
            }
            const SourceManager &sourceManager = Context->getSourceManager();
            SourceRange srcRange_r = Func->getReturnTypeSourceRange();
            srcRange_r.setEnd(Lexer::getLocForEndOfToken(
                srcRange_r.getEnd(), 0, sourceManager, Context->getLangOpts()));
            bool Invalid = false;
            StringRef srcText_r = Lexer::getSourceText(
                CharSourceRange::getTokenRange(srcRange_r), sourceManager,
                Context->getLangOpts(), &Invalid);
            function.return_type = std::string(srcText_r);
            std::string return_type = Func->getReturnType().getAsString();
            if (return_type.find(" ") != std::string::npos) {
              return_type =
                  return_type.substr(return_type.find_last_of(" ") + 1);
            }
            Application application;
            application.class_name = return_type;
            application.signature = "";
            function.applications.push_back(application);
            for (const ParmVarDecl *param : Func->parameters()) {
              std::string param_type = param->getType().getAsString();
              if (param_type.find(" ") != std::string::npos) {
                param_type =
                    param_type.substr(param_type.find_last_of(" ") + 1);
              }
              Application application;
              application.class_name = param_type;
              application.signature = "";
              function.applications.push_back(application);
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
                if (function.its_namespace == "") {
                  function.its_namespace = namespaceDecl->getNameAsString();
                } else {
                  function.its_namespace = namespaceDecl->getNameAsString() +
                                           "::" + function.its_namespace;
                }
              }
              context = context->getParent();
            }
            if (b == 0) {
              function.its_namespace = "";
            }
            StmtVarFunctionVisitor function_stmt_visitor(Context);
            function_stmt_visitor.TraverseStmt(Func->getBody());
            function.applications = function_stmt_visitor.get_applications();
            gad_classes_and_functions.push_back_function(function);
          }
          for (FunctionDecl *Spec : Declaration->specializations()) {
            const TemplateArgumentList *TAL =
                Spec->getTemplateSpecializationArgs();
            if (TAL) {
              std::vector<std::string> specialization_parameter;
              for (const TemplateArgument &Arg : TAL->asArray()) {
                if (Arg.getKind() == TemplateArgument::Type) {
                  //   std::cout << "Template Argument Type: "
                  //             << Arg.getAsType().getAsString() << "\n";
                  std::string type_name = Arg.getAsType().getAsString();
                  if (type_name.find(" ") != std::string::npos) {
                    type_name =
                        type_name.substr(type_name.find_last_of(" ") + 1);
                  }
                  specialization_parameter.push_back(type_name);
                }
              }
              gad_classes_and_functions.push_back_specialization_parameter(
                  "class", signature, specialization_parameter);
            }
          }
        } else {
          const CXXRecordDecl *ParentClass =
              dyn_cast<CXXRecordDecl>(Func->getParent());
          std::string class_name = ParentClass->getNameAsString();
          if (gad_classes_and_functions.has_class(class_name)) {
            // std::cout << "Class Declaration " << class_name <<
            // std::endl;
            std::string signature = get_signature(Func);
            if (!gad_classes_and_functions.has_function(class_name,
                                                        signature)) {
              if (isa<CXXConstructorDecl>(Func)) {
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
                  std::string param_type = param->getType().getAsString();
                  if (param_type.find(" ") != std::string::npos) {
                    param_type =
                        param_type.substr(param_type.find_last_of(" ") + 1);
                  }
                  Application application;
                  application.class_name = param_type;
                  application.signature = "";
                  constructor.applications.push_back(application);
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
                gad_classes_and_functions.push_back_constructor(class_name,
                                                                constructor);
              } else if (isa<CXXDestructorDecl>(Func)) {
                // std::string signature;
                // std::string function_body;
                // std::vector<Application> applications;
                // std::cout << "Destructor Declaration " <<
                // class_name
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
                gad_classes_and_functions.push_back_destructor(class_name,
                                                               destructor);
              } else {
                Method method;
                std::string function_name = Func->getNameAsString();
                // std::cout << "Method Declaration " <<
                // function_name
                //           << std::endl;
                method.method_name = function_name;
                method.signature = signature;
                method.is_template = 1;
                auto template_parameters = Declaration->getTemplateParameters();
                for (auto template_parameter : template_parameters->asArray()) {
                  method.template_parameters.push_back(
                      template_parameter->getNameAsString());
                }
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
                std::string return_type = Func->getReturnType().getAsString();
                if (return_type.find(" ") != std::string::npos) {
                  return_type =
                      return_type.substr(return_type.find_last_of(" ") + 1);
                }
                Application application;
                application.class_name = return_type;
                application.signature = "";
                method.applications.push_back(application);
                for (const ParmVarDecl *param : Func->parameters()) {
                  std::string param_type = param->getType().getAsString();
                  if (param_type.find(" ") != std::string::npos) {
                    param_type =
                        param_type.substr(param_type.find_last_of(" ") + 1);
                  }
                  Application application;
                  application.class_name = param_type;
                  application.signature = "";
                  method.applications.push_back(application);
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
                gad_classes_and_functions.push_back_method(class_name, method);
              }
            }
            if (isa<CXXMethodDecl>(Func)) {
              // Declaration->dump();
              for (FunctionDecl *Spec : Declaration->specializations()) {
                const TemplateArgumentList *TAL =
                    Spec->getTemplateSpecializationArgs();
                if (TAL) {
                  std::vector<std::string> specialization_parameter;
                  for (const TemplateArgument &Arg : TAL->asArray()) {
                    if (Arg.getKind() == TemplateArgument::Type) {
                      //   std::cout << "Template Argument Type: "
                      //             << Arg.getAsType().getAsString() << "\n";
                      std::string type_name = Arg.getAsType().getAsString();
                      if (type_name.find(" ") != std::string::npos) {
                        type_name =
                            type_name.substr(type_name.find_last_of(" ") + 1);
                      }
                      specialization_parameter.push_back(type_name);
                    }
                  }
                  gad_classes_and_functions.push_back_specialization_parameter(
                      class_name, signature, specialization_parameter);
                }
              }
            }
          }
        }
        // gad_classes_and_functions.cout();
      }
    }
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *Declaration) {
    if (Declaration->isThisDeclarationADefinition()) {
      clang::SourceLocation loc = Declaration->getLocation();
      clang::SourceManager &SM = Context->getSourceManager();
      clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
      std::string file_path = presumedLoc.getFilename();
      if (file_path.find(gad_project_path) != std::string::npos) {
        //     std::cout << get_signature(Declaration->getAsFunction()) <<
        //     std::endl; Declaration->dump(); std::cout << std::endl;
        if (!isa<CXXMethodDecl>(Declaration)) {
          std::string function_name = Declaration->getNameAsString();
          std::string signature = get_signature(Declaration);
          if (!gad_classes_and_functions.has_function("class", signature)) {
            Function function;
            // std::vector<Application> applications;
            function.function_name = function_name;
            function.signature = signature;
            function.is_template = 0;
            const SourceManager &sourceManager = Context->getSourceManager();
            SourceRange srcRange_r = Declaration->getReturnTypeSourceRange();
            srcRange_r.setEnd(Lexer::getLocForEndOfToken(
                srcRange_r.getEnd(), 0, sourceManager, Context->getLangOpts()));
            bool Invalid = false;
            StringRef srcText_r = Lexer::getSourceText(
                CharSourceRange::getTokenRange(srcRange_r), sourceManager,
                Context->getLangOpts(), &Invalid);
            function.return_type = std::string(srcText_r);
            std::string return_type =
                Declaration->getReturnType().getAsString();
            if (return_type.find(" ") != std::string::npos) {
              return_type =
                  return_type.substr(return_type.find_last_of(" ") + 1);
            }
            Application application;
            application.class_name = return_type;
            application.signature = "";
            function.applications.push_back(application);
            for (const ParmVarDecl *param : Declaration->parameters()) {
              std::string param_type = param->getType().getAsString();
              if (param_type.find(" ") != std::string::npos) {
                param_type =
                    param_type.substr(param_type.find_last_of(" ") + 1);
              }
              Application application;
              application.class_name = param_type;
              application.signature = "";
              function.applications.push_back(application);
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
            SourceRange srcRange = Declaration->getSourceRange();
            srcRange.setEnd(Lexer::getLocForEndOfToken(
                srcRange.getEnd(), 0, sourceManager, Context->getLangOpts()));
            StringRef srcText = Lexer::getSourceText(
                CharSourceRange::getTokenRange(srcRange), sourceManager,
                Context->getLangOpts(), &Invalid);
            function.function_body = srcText;
            const DeclContext *context = Declaration->getDeclContext();
            bool b = 0;
            while (context) {
              if (const NamespaceDecl *namespaceDecl =
                      dyn_cast<NamespaceDecl>(context)) {
                b = 1;
                if (function.its_namespace == "") {
                  function.its_namespace = namespaceDecl->getNameAsString();
                } else {
                  function.its_namespace = namespaceDecl->getNameAsString() +
                                           "::" + function.its_namespace;
                }
              }
              context = context->getParent();
            }
            if (b == 0) {
              function.its_namespace = "";
            }
            StmtVarFunctionVisitor function_stmt_visitor(Context);
            function_stmt_visitor.TraverseStmt(Declaration->getBody());
            function.applications = function_stmt_visitor.get_applications();
            gad_classes_and_functions.push_back_function(function);
          }
        } else {
          const CXXRecordDecl *ParentClass =
              dyn_cast<CXXRecordDecl>(Declaration->getParent());
          std::string class_name = ParentClass->getNameAsString();
          if (gad_classes_and_functions.has_class(class_name)) {
            // std::cout << "Class Declaration " << class_name <<
            // std::endl;
            std::string signature = get_signature(Declaration);
            if (!gad_classes_and_functions.has_function(class_name,
                                                        signature)) {
              if (isa<CXXConstructorDecl>(Declaration)) {
                // std::string signature;
                // std::string function_body;
                // std::vector<std::string> parameters;
                // std::vector<Application> applications;
                Constructor constructor;
                const SourceManager &sourceManager =
                    Context->getSourceManager();
                bool Invalid = false;
                constructor.signature = signature;
                for (const ParmVarDecl *param : Declaration->parameters()) {
                  std::string param_type = param->getType().getAsString();
                  if (param_type.find(" ") != std::string::npos) {
                    param_type =
                        param_type.substr(param_type.find_last_of(" ") + 1);
                  }
                  Application application;
                  application.class_name = param_type;
                  application.signature = "";
                  constructor.applications.push_back(application);
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
                SourceRange srcRange = Declaration->getSourceRange();
                srcRange.setEnd(Lexer::getLocForEndOfToken(
                    srcRange.getEnd(), 0, sourceManager,
                    Context->getLangOpts()));
                StringRef srcText = Lexer::getSourceText(
                    CharSourceRange::getTokenRange(srcRange), sourceManager,
                    Context->getLangOpts(), &Invalid);
                constructor.function_body = srcText;
                StmtVarFunctionVisitor function_stmt_visitor(Context);
                function_stmt_visitor.TraverseStmt(Declaration->getBody());
                constructor.applications =
                    function_stmt_visitor.get_applications();
                gad_classes_and_functions.push_back_constructor(class_name,
                                                                constructor);
              } else if (isa<CXXDestructorDecl>(Declaration)) {
                // std::string signature;
                // std::string function_body;
                // std::vector<Application> applications;
                // std::cout << "Destructor Declaration " <<
                // class_name
                //           << std::endl;
                Destructor destructor;
                const SourceManager &sourceManager =
                    Context->getSourceManager();
                SourceRange srcRange = Declaration->getSourceRange();
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
                function_stmt_visitor.TraverseStmt(Declaration->getBody());
                destructor.applications =
                    function_stmt_visitor.get_applications();
                gad_classes_and_functions.push_back_destructor(class_name,
                                                               destructor);
              } else {
                Method method;
                std::string function_name = Declaration->getNameAsString();
                // std::cout << "Method Declaration " <<
                // function_name
                //           << std::endl;
                method.method_name = function_name;
                method.signature = signature;
                method.is_template = 0;
                const SourceManager &sourceManager =
                    Context->getSourceManager();
                SourceRange srcRange_r =
                    Declaration->getReturnTypeSourceRange();
                srcRange_r.setEnd(Lexer::getLocForEndOfToken(
                    srcRange_r.getEnd(), 0, sourceManager,
                    Context->getLangOpts()));
                bool Invalid = false;
                StringRef srcText_r = Lexer::getSourceText(
                    CharSourceRange::getTokenRange(srcRange_r), sourceManager,
                    Context->getLangOpts(), &Invalid);
                method.return_type = std::string(srcText_r);
                std::string return_type =
                    Declaration->getReturnType().getAsString();
                if (return_type.find(" ") != std::string::npos) {
                  return_type =
                      return_type.substr(return_type.find_last_of(" ") + 1);
                }
                Application application;
                application.class_name = return_type;
                application.signature = "";
                method.applications.push_back(application);
                for (const ParmVarDecl *param : Declaration->parameters()) {
                  std::string param_type = param->getType().getAsString();
                  if (param_type.find(" ") != std::string::npos) {
                    param_type =
                        param_type.substr(param_type.find_last_of(" ") + 1);
                  }
                  Application application;
                  application.class_name = param_type;
                  application.signature = "";
                  method.applications.push_back(application);
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
                SourceRange srcRange = Declaration->getSourceRange();
                srcRange.setEnd(Lexer::getLocForEndOfToken(
                    srcRange.getEnd(), 0, sourceManager,
                    Context->getLangOpts()));
                StringRef srcText = Lexer::getSourceText(
                    CharSourceRange::getTokenRange(srcRange), sourceManager,
                    Context->getLangOpts(), &Invalid);
                method.function_body = srcText;
                StmtVarFunctionVisitor function_stmt_visitor(Context);
                function_stmt_visitor.TraverseStmt(Declaration->getBody());
                method.applications = function_stmt_visitor.get_applications();
                gad_classes_and_functions.push_back_method(class_name, method);
              }
            }
          }
        }
        // gad_classes_and_functions.cout();
      }
    }
    return true;
  }

private:
  ASTContext *Context;
};

class FindClassesAndFunctionsConsumer : public clang::ASTConsumer {
public:
  explicit FindClassesAndFunctionsConsumer(ASTContext *Context)
      : Visitor(Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  FindClassesAndFunctionsVisitor Visitor;
};

class FindClassesAndFunctionsAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::make_unique<FindClassesAndFunctionsConsumer>(
        &Compiler.getASTContext());
  }
};

void GetClassesAndFunctions::get_definitions() {
  gad_project_path = project_path;
  for (auto file_path : *file_paths) {
    // std::cout << file_path << std::endl;
    std::vector<const char *> args{"context_path", "-p",
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
    Tool.run(newFrontendActionFactory<FindClassesAndFunctionsAction>().get());
  }
  gad_classes_and_functions.update_all_applications();
  classes_and_functions = gad_classes_and_functions;
}

ClassesAndFunctions GetClassesAndFunctions::get_classes_and_functions() {
  return classes_and_functions;
}

std::vector<Application>
ClassesAndFunctions::get_applications(std::string class_name,
                                      std::string signature) {
  if (class_name != "class") {
    for (auto a_class : classes) {
      for (auto constructor : a_class.constructors) {
        if (constructor.signature == signature) {
          return constructor.applications;
        }
      }
      if (a_class.destructor.signature == signature) {
        return a_class.destructor.applications;
      }
      for (auto method : a_class.methods) {
        if (method.signature == signature) {
          return method.applications;
        }
      }
    }
  } else {
    for (auto function : functions) {
      if (function.signature == signature) {
        return function.applications;
      }
    }
  }
}

void ClassesAndFunctions::get_all_applications(
    std::vector<Application> *applications) {
  for (auto application : *applications) {
    std::vector<Application> need_applications =
        get_applications(application.class_name, application.signature);
    for (auto need_application : need_applications) {
      bool b = 0;
      for (auto exist_application : *applications) {
        if (exist_application == need_application) {
          b = 1;
          break;
        }
      }
      if (b == 0) {
        applications->push_back(need_application);
      }
    }
  }
}

json ClassesAndFunctions::get_simple_class(std::string class_name) {
  json j;
  for (auto a_class : classes) {
    if (a_class.class_name == class_name) {
      j[a_class.its_namespace] = json::object();
      j[a_class.its_namespace]["class"] = json::object();
      j[a_class.its_namespace]["class"][a_class.class_name] = json::object();
      j[a_class.its_namespace]["class"][a_class.class_name]["base_class"] =
          json::object();
      j[a_class.its_namespace]["class"][a_class.class_name]["base_class"] =
          a_class.base_class;
      j[a_class.its_namespace]["class"][a_class.class_name]["is_template"] =
          json::object();
      if (a_class.is_template == 1) {
        j[a_class.its_namespace]["class"][a_class.class_name]["is_template"] =
            "true";
        j[a_class.its_namespace]["class"][a_class.class_name]
         ["template_parameters"] = json::array();
        j[a_class.its_namespace]["class"][a_class.class_name]
         ["template_parameters"] = a_class.template_parameters;
        j[a_class.its_namespace]["class"][a_class.class_name]
         ["specialization_parameters"] = json::array();
        for (auto specialization_parameter :
             a_class.specialization_parameters) {
          std::string specialization_parameter_string = "<";
          int i = 0;
          for (auto specialization_parameter_type : specialization_parameter) {
            if (i == 0) {
              specialization_parameter_string =
                  specialization_parameter_string +
                  specialization_parameter_type;
            } else {
              specialization_parameter_string =
                  specialization_parameter_string + "," +
                  specialization_parameter_type;
            }
            i++;
          }
          specialization_parameter_string =
              specialization_parameter_string + ">";
          j[a_class.its_namespace]["class"][a_class.class_name]
           ["specialization_parameters"]
               .push_back(specialization_parameter_string);
        }
      } else {
        j[a_class.its_namespace]["class"][a_class.class_name]["is_template"] =
            "false";
      }
      j[a_class.its_namespace]["class"][a_class.class_name]["alias"] =
          json::array();
      for (auto alias : a_class.aliases) {
        std::string alias_string =
            "using " + alias.alias_name + " = " + alias.base_name;
        j[a_class.its_namespace]["class"][a_class.class_name]["alias"]
            .push_back(alias_string);
      }
      j[a_class.its_namespace]["class"][a_class.class_name]["constructor"] =
          json::object();
      for (auto constructor : a_class.constructors) {
        j[a_class.its_namespace]["class"][a_class.class_name]["constructor"]
         [constructor.signature] = json::object();
        j[a_class.its_namespace]["class"][a_class.class_name]["constructor"]
         [constructor.signature]["function_body"] = json::object();
        j[a_class.its_namespace]["class"][a_class.class_name]["constructor"]
         [constructor.signature]["parameters"] = json::array();
      }
      j[a_class.its_namespace]["class"][a_class.class_name]["destructor"] =
          json::object();
      j[a_class.its_namespace]["class"][a_class.class_name]["destructor"]
       ["function_body"] = json::object();
      j[a_class.its_namespace]["class"][a_class.class_name]["destructor"]
       ["signature"] = json::object();
      j[a_class.its_namespace]["class"][a_class.class_name]["destructor"]
       ["signature"] = a_class.destructor.signature;
      j[a_class.its_namespace]["class"][a_class.class_name]["fields"] =
          json::array();
      j[a_class.its_namespace]["class"][a_class.class_name]["fields"] =
          a_class.fields;
      j[a_class.its_namespace]["class"][a_class.class_name]["methods"] =
          json::object();
      for (auto method : a_class.methods) {
        j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
         [method.signature] = json::object();
        j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
         [method.signature]["function_body"] = json::object();
        j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
         [method.signature]["parameters"] = json::array();
        j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
         [method.signature]["return_type"] = json::object();
      }
      return j;
    }
  }
}

json ClassesAndFunctions::get_j(std::string class_name, std::string signature) {
  json j;
  if (class_name != "class") {
    for (auto a_class : classes) {
      if (a_class.class_name == class_name) {
        bool b = 0;
        if (b == 0) {
          for (auto constructor : a_class.constructors) {
            if (constructor.signature == signature) {
              b = 1;
              j[a_class.its_namespace] = json::object();
              j[a_class.its_namespace]["class"] = json::object();
              j[a_class.its_namespace]["class"][a_class.class_name] =
                  json::object();
              j[a_class.its_namespace]["class"][a_class.class_name]
               ["constructor"] = json::object();
              j[a_class.its_namespace]["class"][a_class.class_name]
               ["constructor"][constructor.signature] = json::object();
              j[a_class.its_namespace]["class"][a_class.class_name]
               ["constructor"][constructor.signature]["function_body"] =
                   json::object();
              j[a_class.its_namespace]["class"][a_class.class_name]
               ["constructor"][constructor.signature]["function_body"] =
                   constructor.function_body;
              j[a_class.its_namespace]["class"][a_class.class_name]
               ["constructor"][constructor.signature]["parameters"] =
                   json::array();
              j[a_class.its_namespace]["class"][a_class.class_name]
               ["constructor"][constructor.signature]["parameters"] =
                   constructor.parameters;
              break;
            }
          }
        } else {
          break;
        }
        if (b == 0) {
          if (a_class.destructor.signature == signature) {
            b = 1;
            j[a_class.its_namespace] = json::object();
            j[a_class.its_namespace]["class"] = json::object();
            j[a_class.its_namespace]["class"][a_class.class_name] =
                json::object();
            j[a_class.its_namespace]["class"][a_class.class_name]
             ["destructor"] = json::object();
            j[a_class.its_namespace]["class"][a_class.class_name]["destructor"]
             ["function_body"] = json::object();
            j[a_class.its_namespace]["class"][a_class.class_name]["destructor"]
             ["function_body"] = a_class.destructor.function_body;
            j[a_class.its_namespace]["class"][a_class.class_name]["destructor"]
             ["signature"] = json::object();
            j[a_class.its_namespace]["class"][a_class.class_name]["destructor"]
             ["signature"] = a_class.destructor.signature;
          }
        } else {
          break;
        }
        if (b == 0) {
          for (auto method : a_class.methods) {
            if (method.signature == signature) {
              j[a_class.its_namespace] = json::object();
              j[a_class.its_namespace]["class"] = json::object();
              j[a_class.its_namespace]["class"][a_class.class_name] =
                  json::object();
              j[a_class.its_namespace]["class"][a_class.class_name]["methods"] =
                  json::object();
              j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
               [method.signature] = json::object();
              j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
               [method.signature]["function_body"] = json::object();
              j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
               [method.signature]["function_body"] = method.function_body;
              j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
               [method.signature]["parameters"] = json::array();
              j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
               [method.signature]["parameters"] = method.parameters;
              j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
               [method.signature]["is_template"] = json::object();
              if (method.is_template == 1) {
                j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
                 [method.signature]["is_template"] = "true";
                j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
                 [method.signature]["template_parameters"] = json::array();
                j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
                 [method.signature]["template_parameters"] =
                     method.template_parameters;
                j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
                 [method.signature]["specialization_parameters"] =
                     json::array();
                for (auto specialization_parameter :
                     method.specialization_parameters) {
                  std::string specialization_parameter_string = "<";
                  int i = 0;
                  for (auto specialization_parameter_type :
                       specialization_parameter) {
                    if (i == 0) {
                      specialization_parameter_string =
                          specialization_parameter_string +
                          specialization_parameter_type;
                    } else {
                      specialization_parameter_string =
                          specialization_parameter_string + "," +
                          specialization_parameter_type;
                    }
                    i++;
                  }
                  specialization_parameter_string =
                      specialization_parameter_string + ">";
                  j[a_class.its_namespace]["class"][a_class.class_name]
                   ["methods"][method.signature]["specialization_parameters"]
                       .push_back(specialization_parameter_string);
                }
              } else {
                j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
                 [method.signature]["is_template"] = "false";
              }
              j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
               [method.signature]["return_type"] = json::object();
              j[a_class.its_namespace]["class"][a_class.class_name]["methods"]
               [method.signature]["return_type"] = method.return_type;
              break;
            }
          }
        } else {
          break;
        }
      }
    }
  } else {
    for (auto function : functions) {
      if (function.signature == signature) {
        j[function.its_namespace] = json::object();
        j[function.its_namespace]["function"] = json::object();
        j[function.its_namespace]["function"][function.signature] =
            json::object();
        j[function.its_namespace]["function"][function.signature]
         ["function_body"] = json::object();
        j[function.its_namespace]["function"][function.signature]
         ["function_body"] = function.function_body;
        j[function.its_namespace]["function"][function.signature]
         ["parameters"] = json::array();
        j[function.its_namespace]["function"][function.signature]
         ["parameters"] = function.parameters;
        j[function.its_namespace]["function"][function.signature]
         ["is_template"] = json::object();
        if (function.is_template == 1) {
          j[function.its_namespace]["function"][function.signature]
           ["is_template"] = "true";
          j[function.its_namespace]["function"][function.signature]
           ["template_parameters"] = json::array();
          j[function.its_namespace]["function"][function.signature]
           ["template_parameters"] = function.template_parameters;
          j[function.its_namespace]["function"][function.signature]
           ["specialization_parameters"] = json::array();
          for (auto specialization_parameter :
               function.specialization_parameters) {
            std::string specialization_parameter_string = "<";
            int i = 0;
            for (auto specialization_parameter_type :
                 specialization_parameter) {
              if (i == 0) {
                specialization_parameter_string =
                    specialization_parameter_string +
                    specialization_parameter_type;
              } else {
                specialization_parameter_string =
                    specialization_parameter_string + "," +
                    specialization_parameter_type;
              }
              i++;
            }
            specialization_parameter_string =
                specialization_parameter_string + ">";
            j[function.its_namespace]["function"][function.signature]
             ["specialization_parameters"]
                 .push_back(specialization_parameter_string);
          }
        } else {
          j[function.its_namespace]["function"][function.signature]
           ["is_template"] = "false";
        }
        j[function.its_namespace]["function"][function.signature]
         ["return_type"] = json::object();
        j[function.its_namespace]["function"][function.signature]
         ["return_type"] = function.return_type;
        break;
      }
    }
  }
  return j;
}
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

std::string ClassesAndFunctions::get_function_body(std::string class_name,
                                                   std::string signature) {
  if (class_name != "class") {
    for (auto a_class : classes) {
      if (a_class.class_name == class_name) {
        for (auto constructor : a_class.constructors) {
          if (constructor.signature == signature) {
            return constructor.function_body;
          }
        }
        if (a_class.destructor.signature == signature) {
          return a_class.destructor.function_body;
        }
        for (auto method : a_class.methods) {
          if (method.signature == signature) {
            return method.function_body;
          }
        }
      }
    }
  } else {
    for (auto function : functions) {
      if (function.signature == signature) {
        return function.function_body;
      }
    }
  }
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
          // for (auto specialization_type : specialization_parameter) {
          //   Application application;
          //   application.class_name = specialization_type;
          //   application.signature = "";
          //   a_class.applications.push_back(application);
          //   // if (this->has_class(specialization_type)) {
          //   //   std::vector<std::string> constructors =
          //   //       this->get_constructors(specialization_type);
          //   //   std::vector<std::pair<std::string, std::string>>
          //   applications;
          //   //   for (auto constructor : constructors) {
          //   //     applications.push_back(std::pair<std::string,
          //   std::string>(
          //   //         specialization_type, constructor));
          //   //   }
          //   //   std::string destructor = this->get_destructor(class_name);
          //   //   if (destructor != "class") {
          //   //     applications.push_back(std::pair<std::string,
          //   std::string>(
          //   //         specialization_type, destructor));
          //   //   }
          //   //   this->push_back_applications(class_name, "", applications);
          //   // }
          // }
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
            // for (auto specialization_type : specialization_parameter) {
            //   Application application;
            //   application.class_name = specialization_type;
            //   application.signature = "";
            //   function.applications.push_back(application);
            //   // if (this->has_class(specialization_type)) {
            //   //   std::vector<std::string> constructors =
            //   //       this->get_constructors(specialization_type);
            //   //   std::vector<std::pair<std::string, std::string>>
            //   //   applications; for (auto constructor : constructors) {
            //   //     applications.push_back(std::pair<std::string,
            //   std::string>(
            //   //         specialization_type, constructor));
            //   //   }
            //   //   std::string destructor = this->get_destructor(class_name);
            //   //   if (destructor != "class") {
            //   //     applications.push_back(std::pair<std::string,
            //   std::string>(
            //   //         specialization_type, destructor));
            //   //   }
            //   //   this->push_back_applications("class", signature,
            //   //   applications);
            //   // }
            // }
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
                // for (auto specialization_type : specialization_parameter) {
                //   Application application;
                //   application.class_name = specialization_type;
                //   application.signature = "";
                //   method.applications.push_back(application);
                //   // if (this->has_class(specialization_type)) {
                //   //   std::vector<std::string> constructors =
                //   //       this->get_constructors(specialization_type);
                //   //   std::vector<std::pair<std::string, std::string>>
                //   //       applications;
                //   //   for (auto constructor : constructors) {
                //   //     applications.push_back(
                //   //         std::pair<std::string, std::string>(
                //   //             specialization_type, constructor));
                //   //   }
                //   //   std::string destructor =
                //   //   this->get_destructor(class_name); if (destructor !=
                //   //   "class") {
                //   //     applications.push_back(
                //   //         std::pair<std::string, std::string>(
                //   //             specialization_type, destructor));
                //   //   }
                //   //   this->push_back_applications(class_name, signature,
                //   //                                applications);
                //   // }
                // }
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
  std::cout << "classes:" << std::endl;
  for (auto a_class : classes) {
    a_class.cout();
  }
  std::cout << "functions:" << std::endl;
  for (auto function : functions) {
    function.cout();
  }
  // std::cout << "class:" << std::endl;
  // for (auto a_class : classes) {
  //   std::cout << a_class.class_name
  //             << "\t"
  //             // << a_class.base_class << "\t"
  //             << a_class.is_template << std::endl
  //             << "template_parameters:";
  //   for (auto template_parameter : a_class.template_parameters) {
  //     std::cout << template_parameter << "\t";
  //   }
  //   std::cout << std::endl << "specialization_parameters:";
  //   for (auto specialization_parameter : a_class.specialization_parameters) {
  //     std::cout << "<";
  //     for (auto specialization_type : specialization_parameter) {
  //       std::cout << specialization_type << ",";
  //     }
  //     std::cout << ">\t";
  //   }
  //   std::cout << std::endl << "fields:";
  //   for (auto field : a_class.fields) {
  //     std::cout << field << "\t";
  //   }
  //   std::cout << std::endl << "alias:";
  //   for (auto alias : a_class.aliases) {
  //     std::cout << "<" << alias.alias_name << "," << alias.base_name <<
  //     ">\t";
  //   }
  //   std::cout << std::endl
  //             << "namespace:" << a_class.its_namespace << std::endl
  //             << "constructors:";
  //   for (auto constructor : a_class.constructors) {
  //     std::cout << constructor.signature << "\t";
  //     for (auto application : constructor.applications) {
  //       application.cout();
  //     }
  //     std::cout << std::endl;
  //   }
  //   std::cout << std::endl
  //             << "destructor:" << a_class.destructor.signature << "\t";
  //   for (auto application : a_class.destructor.applications) {
  //     application.cout();
  //   }
  //   std::cout << std::endl;
  //   std::cout << std::endl << "methods:";
  //   for (auto method : a_class.methods) {
  //     std::cout << method.signature << "\t" << method.is_template <<
  //     std::endl
  //               << "template_parameters:";
  //     for (auto template_parameter : method.template_parameters) {
  //       std::cout << template_parameter << "\t";
  //     }
  //     std::cout << std::endl << "specialization_parameters:";
  //     for (auto specialization_parameter : method.specialization_parameters)
  //     {
  //       std::cout << "<";
  //       for (auto specialization_type : specialization_parameter) {
  //         std::cout << specialization_type << ",";
  //       }
  //       std::cout << ">\t";
  //     }
  //     for (auto application : method.applications) {
  //       application.cout();
  //     }
  //     std::cout << std::endl;
  //   }
  //   std::cout << std::endl << "applications:";
  //   for (auto application : a_class.applications) {
  //     application.cout();
  //   }
  //   std::cout << std::endl << std::endl;
  // }
  // std::cout << "functions:" << std::endl;
  // for (auto function : functions) {
  //   std::cout << function.function_name << "\t" << function.signature << "\t"
  //             << function.is_template << std::endl
  //             << "template_parameters:";
  //   for (auto template_parameter : function.template_parameters) {
  //     std::cout << template_parameter << "\t";
  //   }
  //   std::cout << std::endl << "specialization_parameters:";
  //   for (auto specialization_parameter : function.specialization_parameters)
  //   {
  //     std::cout << "<";
  //     for (auto specialization_type : specialization_parameter) {
  //       std::cout << specialization_type << ",";
  //     }
  //     std::cout << ">\t";
  //   }
  //   std::cout << std::endl
  //             << "namespace:" << function.its_namespace << std::endl
  //             << "applications:";
  //   for (auto application : function.applications) {
  //     application.cout();
  //   }
  //   std::cout << std::endl << std::endl;
  // }
}

std::vector<std::string>
ClassesAndFunctions::get_application_classes(std::string type) {
  std::vector<std::string> application_classes;
  for (auto a_class : classes) {
    if (a_class.class_name != "" &&
        type.find(a_class.class_name) != std::string::npos) {
      application_classes.push_back(a_class.class_name);
    }
  }
  return application_classes;
}

MyRecordType ClassesAndFunctions::get_class_type(std::string class_name) {
  for (auto a_class : classes) {
    if (a_class.class_name == class_name) {
      return a_class.record_type;
    }
  }
}

void update_applications(std::vector<Application> &applications) {
  for (auto it = applications.begin(); it != applications.end();) {
    if (it->signature == "" &&
        !gad_classes_and_functions.has_class(it->class_name)) {
      it = applications.erase(it);
      continue;
    }
    // else if (!gad_classes_and_functions.has_function(it->class_name,
    //                                                    it->signature)) {
    //   it = applications.erase(it);
    //   continue;
    // }
    else {
      ++it;
      continue;
    }
  }
  int i = 0;
  while (i < applications.size()) {
    auto a_application = applications[i];
    if (a_application.signature == ""
        // && gad_classes_and_functions.get_class_type(
        //                                it->class_name) !=
        //                                MyRecordType::EnumType
    ) {
      std::string class_name = a_application.class_name;
      // it = applications.erase(it);
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
    }
    i++;
  }
  for (auto it = applications.begin(); it != applications.end();) {
    // if (it->signature == "" &&
    //     !gad_classes_and_functions.has_class(it->class_name)) {
    //   it = applications.erase(it);
    //   continue;
    // }
    if (it->signature != "" && !gad_classes_and_functions.has_function(
                                   it->class_name, it->signature)) {
      it = applications.erase(it);
      continue;
    } else {
      ++it;
      continue;
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
    Callee = Callee->getCanonicalDecl();
    if (Callee->getPrimaryTemplate()) {
      Callee = Callee->getPrimaryTemplate()->getTemplatedDecl();
    }
    // Callee->dump();
    if (Callee->getTemplateInstantiationPattern()) {
      Callee = Callee->getTemplateInstantiationPattern();
      // InstantiatedCallee->dump();
    }
    // std::cout << std::endl;
    if (Callee->isCXXClassMember()) {
      // std::string file_path;
      // std::string class_name;
      // std::string function_name;
      // std::string signature;
      const CXXRecordDecl *ParentClass =
          dyn_cast<CXXRecordDecl>(Callee->getParent())->getCanonicalDecl();
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
    // // if (qualType->isReferenceType()) {
    // //   qualType = qualType.getNonReferenceType();
    // // }
    // qualType = qualType.getUnqualifiedType();
    // qualType = qualType.getNonReferenceType();
    // // qualType = qualType.getCanonicalType();
    // // const clang::Type *typePtr = qualType.getTypePtr();
    // // while (const TypedefType *typedefType =
    // dyn_cast<TypedefType>(typePtr)) {
    // //   const TypedefNameDecl *typedefDecl = typedefType->getDecl();
    // //   qualType = typedefDecl->getUnderlyingType();
    // // }
    // std::string class_name = get_application_type(qualType);
    std::vector<std::string> types =
        gad_classes_and_functions.get_application_classes(
            qualType.getAsString());
    // if (class_name.find(" ") != std::string::npos) {
    //   class_name = class_name.substr(class_name.find_last_of(" ") + 1);
    // }
    for (auto type : types) {
      Application application;
      application.class_name = type;
      application.signature = "";
      applications.push_back(application);
    }
    // Application application;
    // application.class_name = class_name;
    // application.signature = "";
    // applications.push_back(application);
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

  // bool VisitTypeDecl(TypeDecl *Declaration) {
  //   clang::SourceLocation loc = Declaration->getLocation();
  //   clang::SourceManager &SM = Context->getSourceManager();
  //   clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
  //   std::string file_path = presumedLoc.getFilename();
  //   if (file_path.find(gad_project_path) != std::string::npos) {
  //     Declaration->dump();
  //   }
  //   return true;
  // }

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
        auto CanonicalDeclaration = Declaration->getCanonicalDecl();
        // auto canonical_declaration = Declaration->getCanonicalDecl();
        // canonical_declaration->dump();
        // Declaration->dump();
        std::string class_name = CanonicalDeclaration->getNameAsString();
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
          if (RecordDecl->isClass()) {
            a_class.record_type = MyRecordType::ClassType;
          } else if (RecordDecl->isStruct()) {
            a_class.record_type = MyRecordType::StructType;
          } else if (RecordDecl->isUnion()) {
            a_class.record_type = MyRecordType::UnionType;
          }
          // if (RecordDecl->getNumBases() > 0) {
          //   a_class.base_class =
          //       RecordDecl->bases_begin()->getType().getAsString();
          // }
          for (auto its_base_class : RecordDecl->bases()) {
            a_class.base_class.push_back(
                its_base_class.getType().getAsString());
            QualType qualType = its_base_class.getType();
            // if (qualType->isReferenceType()) {
            //   qualType = qualType.getNonReferenceType();
            // }
            // qualType = qualType.getUnqualifiedType();
            // qualType = qualType.getNonReferenceType();
            // qualType = qualType.getCanonicalType();
            Application application;
            application.class_name = qualType.getAsString();
            application.signature = "";
            a_class.applications.push_back(application);
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
          for (const auto *decl : RecordDecl->decls()) {
            // if (decl->getCanonicalDecl() == CanonicalDeclaration) {
            //   continue;
            // }
            // if (const CXXRecordDecl *Record = dyn_cast<CXXRecordDecl>(decl))
            // {
            //   if (i == 0) {
            //     continue;
            //   }
            //   const SourceManager &sourceManager =
            //   Context->getSourceManager(); SourceRange srcRange =
            //   Record->getSourceRange();
            //   srcRange.setEnd(Lexer::getLocForEndOfToken(
            //       srcRange.getEnd(), 0, sourceManager,
            //       Context->getLangOpts()));
            //   bool Invalid = false;
            //   StringRef srcText = Lexer::getSourceText(
            //       CharSourceRange::getTokenRange(srcRange), sourceManager,
            //       Context->getLangOpts(), &Invalid);
            //   // std::cout << std::string(srcText) << std::endl;
            //   std::string srcText_str = std::string(srcText);
            //   if (srcText_str[srcText_str.length() - 1] == ';') {
            //     srcText_str.erase(srcText_str.length() - 1);
            //   }
            //   // if (Record->isClass()) {
            //   //   if (srcText.find('{') != std::string::npos) {
            //   //     srcText_str.erase(srcText_str.find_first_of('{'));
            //   //   }
            //   //   a_class.fields.push_back(srcText_str);
            //   // } else {
            //   //   a_class.fields.push_back(srcText_str);
            //   // }
            //   a_class.fields.push_back(srcText_str);
            // }
            if (const FieldDecl *Field = dyn_cast<FieldDecl>(decl)) {
              std::string field_type = Field->getType().getAsString();
              // if (field_type.find(" ") != std::string::npos) {
              //   field_type =
              //       field_type.substr(field_type.find_last_of(" ") + 1);
              // }
              // QualType qualType = Field->getType();
              // // if (qualType->isReferenceType()) {
              // //   qualType = qualType.getNonReferenceType();
              // // }
              // qualType = qualType.getUnqualifiedType();
              // qualType = qualType.getNonReferenceType();
              std::vector<std::string> types =
                  gad_classes_and_functions.get_application_classes(field_type);
              for (auto type : types) {
                Application application;
                application.class_name = type;
                application.signature = "";
                a_class.applications.push_back(application);
              }
              // qualType = qualType.getCanonicalType();
              // Application application;
              // application.class_name = get_application_type(qualType);
              // application.signature = "";
              // a_class.applications.push_back(application);
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
              if (srcText_str.find("{") != std::string::npos) {
                srcText_str.erase(srcText_str.find_first_of("{"),
                                  srcText_str.find_last_of("}") + 1 -
                                      srcText_str.find_first_of("{"));
              }
              if (srcText_str.find("class") != std::string::npos) {
                srcText_str.erase(srcText_str.find_first_of("class"), 6);
              }
              if (srcText_str.find("struct") != std::string::npos) {
                srcText_str.erase(srcText_str.find_first_of("struct"), 7);
              }
              if (srcText_str.find("enum") != std::string::npos) {
                srcText_str.erase(srcText_str.find_first_of("enum"), 5);
              }
              if (srcText_str.find("union") != std::string::npos) {
                srcText_str.erase(srcText_str.find_first_of("union"), 6);
              }
              a_class.fields.push_back(srcText_str);
            }
            // else if (const EnumDecl *Enum = dyn_cast<EnumDecl>(decl)) {
            //   const SourceManager &sourceManager =
            //   Context->getSourceManager(); SourceRange srcRange =
            //   Enum->getSourceRange();
            //   srcRange.setEnd(Lexer::getLocForEndOfToken(
            //       srcRange.getEnd(), 0, sourceManager,
            //       Context->getLangOpts()));
            //   bool Invalid = false;
            //   StringRef srcText = Lexer::getSourceText(
            //       CharSourceRange::getTokenRange(srcRange), sourceManager,
            //       Context->getLangOpts(), &Invalid);
            //   // std::cout << std::string(srcText) << std::endl;
            //   std::string srcText_str = std::string(srcText);
            //   if (srcText_str[srcText_str.length() - 1] == ';') {
            //     srcText_str.erase(srcText_str.length() - 1);
            //   }
            //   a_class.fields.push_back(srcText_str);
            // }
            else if (const TypeAliasDecl *TypeAlias =
                         dyn_cast<TypeAliasDecl>(decl)) {
              Alias alias;
              alias.alias_name = TypeAlias->getNameAsString();
              alias.base_name = TypeAlias->getUnderlyingType().getAsString();
              // QualType qualType = TypeAlias->getUnderlyingType();
              // if (qualType->isReferenceType()) {
              //   qualType = qualType.getNonReferenceType();
              // }
              // qualType = qualType.getUnqualifiedType();
              // qualType = qualType.getNonReferenceType();
              // qualType = qualType.getCanonicalType();
              a_class.aliases.push_back(alias);
              std::vector<std::string> types =
                  gad_classes_and_functions.get_application_classes(
                      alias.base_name);
              for (auto type : types) {
                Application application;
                application.class_name = type;
                application.signature = "";
                a_class.applications.push_back(application);
              }
              // Application application;
              // application.class_name = get_application_type(qualType);
              // application.signature = "";
              // a_class.applications.push_back(application);
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
          }
          // i = 1;
          // while (i < a_class.fields.size()) {
          //   if (a_class.fields[i] == a_class.fields[i - 1] ||
          //       (a_class.fields[i].substr(
          //            0, a_class.fields[i].find_last_of('}') + 1) != "" &&
          //        a_class.fields[i - 1].substr(
          //            0, a_class.fields[i - 1].find_last_of('}') + 1) != "" &&
          //        a_class.fields[i].substr(
          //            0, a_class.fields[i].find_last_of('}') + 1) ==
          //            a_class.fields[i - 1].substr(
          //                0, a_class.fields[i - 1].find_last_of('}') + 1)) ||
          //       a_class.fields[i].find(a_class.fields[i - 1]) !=
          //           std::string::npos) {
          //     a_class.fields.erase(a_class.fields.begin() + i - 1);
          //   } else {
          //     i++;
          //   }
          // }
          // i = 0;
          // while (i < a_class.fields.size()) {
          //   if (a_class.fields[i] == "union" || a_class.fields[i] == "struct"
          //   ||
          //       a_class.fields[i] == "class" || a_class.fields[i] == "enum")
          //       {
          //     a_class.fields.erase(a_class.fields.begin() + i);
          //   } else {
          //     i++;
          //   }
          // }
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
          std::vector<std::pair<std::string, std::string>> applications;
          for (unsigned i = 0; i < specialization->getTemplateArgs().size();
               ++i) {
            const TemplateArgument &Arg =
                specialization->getTemplateArgs().get(i);
            if (Arg.getKind() == TemplateArgument::Type) {
              //   std::cout << "Specialization Argument: "
              //             << Arg.getAsType().getAsString() << "\n";
              std::string type_name = Arg.getAsType().getAsString();
              // if (type_name.find(" ") != std::string::npos) {
              //   type_name = type_name.substr(type_name.find_last_of(" ") +
              //   1);
              // }
              specialization_parameter.push_back(type_name);
              // QualType qualType = Arg.getAsType();
              // if (qualType->isReferenceType()) {
              //   qualType = qualType.getNonReferenceType();
              // }
              // qualType = qualType.getUnqualifiedType();
              // qualType = qualType.getNonReferenceType();
              // qualType = qualType.getCanonicalType();
              std::vector<std::string> types =
                  gad_classes_and_functions.get_application_classes(type_name);
              for (auto type : types) {
                std::pair<std::string, std::string> application;
                application.first = type;
                application.second = "";
                applications.push_back(application);
              }
              // std::pair<std::string, std::string> application;
              // application.first = get_application_type(qualType);
              // application.second = "";
              // applications.push_back(application);
            }
          }
          gad_classes_and_functions.push_back_specialization_parameter(
              class_name, "", specialization_parameter);
          gad_classes_and_functions.push_back_applications(class_name, "",
                                                           applications);
        }
        // gad_classes_and_functions.cout();
      }
    }
    return true;
  }

  bool VisitRecordDecl(RecordDecl *Declaration) {
    if (Declaration->isThisDeclarationADefinition()) {
      // Declaration->dump();
      clang::SourceLocation loc = Declaration->getLocation();
      clang::SourceManager &SM = Context->getSourceManager();
      clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
      std::string file_path = presumedLoc.getFilename();
      if (file_path.find(gad_project_path) != std::string::npos) {
        if (auto CXXDeclaration = dyn_cast<CXXRecordDecl>(Declaration)) {
          // std::cout << Declaration->getNameAsString() << std::endl;
          // Declaration->dump();
          // std::cout << std::endl;
          auto CanonicalDeclaration = CXXDeclaration->getCanonicalDecl();
          // auto canonical_declaration = Declaration->getCanonicalDecl();
          // canonical_declaration->dump();
          // Declaration->dump();
          std::string class_name = CanonicalDeclaration->getNameAsString();
          if (!gad_classes_and_functions.has_class(class_name)) {
            Class a_class;
            a_class.class_name = class_name;
            a_class.is_template = 0;
            if (CXXDeclaration->isClass()) {
              a_class.record_type = MyRecordType::ClassType;
            } else if (CXXDeclaration->isStruct()) {
              a_class.record_type = MyRecordType::StructType;
            } else if (CXXDeclaration->isUnion()) {
              a_class.record_type = MyRecordType::UnionType;
            }
            // if (RecordDecl->getNumBases() > 0) {
            //   a_class.base_class =
            //       RecordDecl->bases_begin()->getType().getAsString();
            // }
            for (auto its_base_class : CXXDeclaration->bases()) {
              a_class.base_class.push_back(
                  its_base_class.getType().getAsString());
              QualType qualType = its_base_class.getType();
              // if (qualType->isReferenceType()) {
              //   qualType = qualType.getNonReferenceType();
              // }
              // qualType = qualType.getUnqualifiedType();
              // qualType = qualType.getNonReferenceType();
              // qualType = qualType.getCanonicalType();
              std::vector<std::string> types =
                  gad_classes_and_functions.get_application_classes(
                      qualType.getAsString());
              for (auto type : types) {
                Application application;
                application.class_name = type;
                application.signature = "";
                a_class.applications.push_back(application);
              }
              // Application application;
              // application.class_name = get_application_type(qualType);
              // application.signature = "";
              // a_class.applications.push_back(application);
            }
            const DeclContext *context = CXXDeclaration->getDeclContext();
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
            for (const auto *decl : CXXDeclaration->decls()) {
              // if (decl->getCanonicalDecl() == CanonicalDeclaration) {
              //   continue;
              // }
              // if (const CXXRecordDecl *Record =
              // dyn_cast<CXXRecordDecl>(decl))
              // {
              //   if (i == 0) {
              //     continue;
              //   }
              //   const SourceManager &sourceManager =
              //   Context->getSourceManager(); SourceRange srcRange =
              //   Record->getSourceRange();
              //   srcRange.setEnd(Lexer::getLocForEndOfToken(
              //       srcRange.getEnd(), 0, sourceManager,
              //       Context->getLangOpts()));
              //   bool Invalid = false;
              //   StringRef srcText = Lexer::getSourceText(
              //       CharSourceRange::getTokenRange(srcRange), sourceManager,
              //       Context->getLangOpts(), &Invalid);
              //   // std::cout << std::string(srcText) << std::endl;
              //   std::string srcText_str = std::string(srcText);
              //   if (srcText_str[srcText_str.length() - 1] == ';') {
              //     srcText_str.erase(srcText_str.length() - 1);
              //   }
              //   // if (Record->isClass()) {
              //   //   if (srcText.find('{') != std::string::npos) {
              //   //     srcText_str.erase(srcText_str.find_first_of('{'));
              //   //   }
              //   //   a_class.fields.push_back(srcText_str);
              //   // } else {
              //   //   a_class.fields.push_back(srcText_str);
              //   // }
              //   a_class.fields.push_back(srcText_str);
              // }
              if (const FieldDecl *Field = dyn_cast<FieldDecl>(decl)) {
                std::string field_type = Field->getType().getAsString();
                // if (field_type.find(" ") != std::string::npos) {
                //   field_type =
                //       field_type.substr(field_type.find_last_of(" ") + 1);
                // }
                // QualType qualType = Field->getType();
                // if (qualType->isReferenceType()) {
                //   qualType = qualType.getNonReferenceType();
                // }
                // qualType = qualType.getUnqualifiedType();
                // qualType = qualType.getNonReferenceType();
                // qualType = qualType.getCanonicalType();
                std::vector<std::string> types =
                    gad_classes_and_functions.get_application_classes(
                        field_type);
                for (auto type : types) {
                  Application application;
                  application.class_name = type;
                  application.signature = "";
                  a_class.applications.push_back(application);
                }
                // Application application;
                // application.class_name = get_application_type(qualType);
                // application.signature = "";
                // a_class.applications.push_back(application);
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
                if (srcText_str.find("{") != std::string::npos) {
                  srcText_str.erase(srcText_str.find_first_of("{"),
                                    srcText_str.find_last_of("}") + 1 -
                                        srcText_str.find_first_of("{"));
                }
                if (srcText_str.find("class") != std::string::npos) {
                  srcText_str.erase(srcText_str.find_first_of("class"), 6);
                }
                if (srcText_str.find("struct") != std::string::npos) {
                  srcText_str.erase(srcText_str.find_first_of("struct"), 7);
                }
                if (srcText_str.find("enum") != std::string::npos) {
                  srcText_str.erase(srcText_str.find_first_of("enum"), 5);
                }
                if (srcText_str.find("union") != std::string::npos) {
                  srcText_str.erase(srcText_str.find_first_of("union"), 6);
                }
                a_class.fields.push_back(srcText_str);
              }
              // else if (const EnumDecl *Enum = dyn_cast<EnumDecl>(decl)) {
              //   const SourceManager &sourceManager =
              //   Context->getSourceManager(); SourceRange srcRange =
              //   Enum->getSourceRange();
              //   srcRange.setEnd(Lexer::getLocForEndOfToken(
              //       srcRange.getEnd(), 0, sourceManager,
              //       Context->getLangOpts()));
              //   bool Invalid = false;
              //   StringRef srcText = Lexer::getSourceText(
              //       CharSourceRange::getTokenRange(srcRange), sourceManager,
              //       Context->getLangOpts(), &Invalid);
              //   // std::cout << std::string(srcText) << std::endl;
              //   std::string srcText_str = std::string(srcText);
              //   if (srcText_str[srcText_str.length() - 1] == ';') {
              //     srcText_str.erase(srcText_str.length() - 1);
              //   }
              //   a_class.fields.push_back(srcText_str);
              // }
              else if (const TypeAliasDecl *TypeAlias =
                           dyn_cast<TypeAliasDecl>(decl)) {
                Alias alias;
                alias.alias_name = TypeAlias->getNameAsString();
                alias.base_name = TypeAlias->getUnderlyingType().getAsString();
                // QualType qualType = TypeAlias->getUnderlyingType();
                // if (qualType->isReferenceType()) {
                //   qualType = qualType.getNonReferenceType();
                // }
                // qualType = qualType.getUnqualifiedType();
                // qualType = qualType.getNonReferenceType();
                // qualType = qualType.getCanonicalType();
                a_class.aliases.push_back(alias);
                std::vector<std::string> types =
                    gad_classes_and_functions.get_application_classes(
                        alias.base_name);
                for (auto type : types) {
                  Application application;
                  application.class_name = type;
                  application.signature = "";
                  a_class.applications.push_back(application);
                }
                // Application application;
                // application.class_name = get_application_type(qualType);
                // application.signature = "";
                // a_class.applications.push_back(application);
                // if (gad_classes_and_functions.has_class(alias.base_name)) {
                //   std::vector<std::string> constructors =
                //       gad_classes_and_functions.get_constructors(alias.base_name);
                //   std::vector<std::pair<std::string, std::string>>
                //   applications; for (auto constructor : constructors) {
                //     applications.push_back(std::pair<std::string,
                //     std::string>(
                //         alias.base_name, constructor));
                //   }
                //   std::string destructor =
                //       gad_classes_and_functions.get_destructor(alias.base_name);
                //   if (destructor != "class") {
                //     applications.push_back(std::pair<std::string,
                //     std::string>(
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
            }
            // i = 1;
            // while (i < a_class.fields.size()) {
            //   if (a_class.fields[i] == a_class.fields[i - 1] ||
            //       (a_class.fields[i].substr(
            //            0, a_class.fields[i].find_last_of('}') + 1) != "" &&
            //        a_class.fields[i - 1].substr(
            //            0, a_class.fields[i - 1].find_last_of('}') + 1) != ""
            //            &&
            //        a_class.fields[i].substr(
            //            0, a_class.fields[i].find_last_of('}') + 1) ==
            //            a_class.fields[i - 1].substr(
            //                0, a_class.fields[i - 1].find_last_of('}') + 1))
            //                ||
            //       a_class.fields[i].find(a_class.fields[i - 1]) !=
            //           std::string::npos) {
            //     a_class.fields.erase(a_class.fields.begin() + i - 1);
            //   } else {
            //     i++;
            //   }
            // }
            // i = 0;
            // while (i < a_class.fields.size()) {
            //   if (a_class.fields[i] == "union" || a_class.fields[i] ==
            //   "struct"
            //   ||
            //       a_class.fields[i] == "class" || a_class.fields[i] ==
            //       "enum")
            //       {
            //     a_class.fields.erase(a_class.fields.begin() + i);
            //   } else {
            //     i++;
            //   }
            // }
            //   std::cout << a_class.class_name << "\t" << a_class.base_class
            //             << std::endl;
            //   for (auto template_parameter : a_class.template_parameters) {
            //     std::cout << template_parameter << "\t";
            //   }
            //   std::cout << std::endl;
            gad_classes_and_functions.push_back_class(a_class);
          }
        }
        // gad_classes_and_functions.cout();
        else { // std::cout << Declaration->getNameAsString() << std::endl;
          // Declaration->dump();
          // std::cout << std::endl;
          auto CanonicalDeclaration = Declaration->getCanonicalDecl();
          std::string class_name = CanonicalDeclaration->getNameAsString();
          for (auto *D : CanonicalDeclaration->getDeclContext()->decls()) {
            if (auto *TD = dyn_cast<TypedefDecl>(D)) {
              if (auto *TRD = TD->getUnderlyingType()->getAsRecordDecl()) {
                if (TRD->getCanonicalDecl() == CanonicalDeclaration) {
                  class_name = TD->getNameAsString();
                  break;
                }
              }
            }
          }
          // auto canonical_declaration = Declaration->getCanonicalDecl();
          // CanonicalDeclaration->dump();
          // Declaration->dump();
          if (!gad_classes_and_functions.has_class(class_name)) {
            Class a_class;
            a_class.class_name = class_name;
            if (Declaration->isClass()) {
              a_class.record_type = MyRecordType::ClassType;
            } else if (Declaration->isStruct()) {
              a_class.record_type = MyRecordType::StructType;
            } else if (Declaration->isUnion()) {
              a_class.record_type = MyRecordType::UnionType;
            }
            // if (RecordDecl->getNumBases() > 0) {
            //   a_class.base_class =
            //       RecordDecl->bases_begin()->getType().getAsString();
            // }
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
            for (const auto *decl : Declaration->decls()) {
              // if (decl->getCanonicalDecl() == CanonicalDeclaration) {
              //   continue;
              // }
              // if (const CXXRecordDecl *Record =
              // dyn_cast<CXXRecordDecl>(decl))
              // {
              //   if (i == 0) {
              //     continue;
              //   }
              //   const SourceManager &sourceManager =
              //   Context->getSourceManager(); SourceRange srcRange =
              //   Record->getSourceRange();
              //   srcRange.setEnd(Lexer::getLocForEndOfToken(
              //       srcRange.getEnd(), 0, sourceManager,
              //       Context->getLangOpts()));
              //   bool Invalid = false;
              //   StringRef srcText = Lexer::getSourceText(
              //       CharSourceRange::getTokenRange(srcRange), sourceManager,
              //       Context->getLangOpts(), &Invalid);
              //   // std::cout << std::string(srcText) << std::endl;
              //   std::string srcText_str = std::string(srcText);
              //   if (srcText_str[srcText_str.length() - 1] == ';') {
              //     srcText_str.erase(srcText_str.length() - 1);
              //   }
              //   // if (Record->isClass()) {
              //   //   if (srcText.find('{') != std::string::npos) {
              //   //     srcText_str.erase(srcText_str.find_first_of('{'));
              //   //   }
              //   //   a_class.fields.push_back(srcText_str);
              //   // } else {
              //   //   a_class.fields.push_back(srcText_str);
              //   // }
              //   a_class.fields.push_back(srcText_str);
              // }
              if (const FieldDecl *Field = dyn_cast<FieldDecl>(decl)) {
                std::string field_type = Field->getType().getAsString();
                // if (field_type.find(" ") != std::string::npos) {
                //   field_type =
                //       field_type.substr(field_type.find_last_of(" ") + 1);
                // }
                // QualType qualType = Field->getType();
                // if (qualType->isReferenceType()) {
                //   qualType = qualType.getNonReferenceType();
                // }
                // qualType = qualType.getUnqualifiedType();
                // qualType = qualType.getNonReferenceType();
                // qualType = qualType.getCanonicalType();
                std::vector<std::string> types =
                    gad_classes_and_functions.get_application_classes(
                        field_type);
                for (auto type : types) {
                  Application application;
                  application.class_name = type;
                  application.signature = "";
                  a_class.applications.push_back(application);
                }
                // Application application;
                // application.class_name = get_application_type(qualType);
                // application.signature = "";
                // a_class.applications.push_back(application);
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
                if (srcText_str.find("{") != std::string::npos) {
                  srcText_str.erase(srcText_str.find_first_of("{"),
                                    srcText_str.find_last_of("}") + 1 -
                                        srcText_str.find_first_of("{"));
                }
                if (srcText_str.find("class") != std::string::npos) {
                  srcText_str.erase(srcText_str.find_first_of("class"), 6);
                }
                if (srcText_str.find("struct") != std::string::npos) {
                  srcText_str.erase(srcText_str.find_first_of("struct"), 7);
                }
                if (srcText_str.find("enum") != std::string::npos) {
                  srcText_str.erase(srcText_str.find_first_of("enum"), 5);
                }
                if (srcText_str.find("union") != std::string::npos) {
                  srcText_str.erase(srcText_str.find_first_of("union"), 6);
                }
                a_class.fields.push_back(srcText_str);
              }
              // else if (const EnumDecl *Enum = dyn_cast<EnumDecl>(decl)) {
              //   const SourceManager &sourceManager =
              //   Context->getSourceManager(); SourceRange srcRange =
              //   Enum->getSourceRange();
              //   srcRange.setEnd(Lexer::getLocForEndOfToken(
              //       srcRange.getEnd(), 0, sourceManager,
              //       Context->getLangOpts()));
              //   bool Invalid = false;
              //   StringRef srcText = Lexer::getSourceText(
              //       CharSourceRange::getTokenRange(srcRange), sourceManager,
              //       Context->getLangOpts(), &Invalid);
              //   // std::cout << std::string(srcText) << std::endl;
              //   std::string srcText_str = std::string(srcText);
              //   if (srcText_str[srcText_str.length() - 1] == ';') {
              //     srcText_str.erase(srcText_str.length() - 1);
              //   }
              //   a_class.fields.push_back(srcText_str);
              // }
              else if (const TypeAliasDecl *TypeAlias =
                           dyn_cast<TypeAliasDecl>(decl)) {
                Alias alias;
                alias.alias_name = TypeAlias->getNameAsString();
                alias.base_name = TypeAlias->getUnderlyingType().getAsString();
                // QualType qualType = TypeAlias->getUnderlyingType();
                // if (qualType->isReferenceType()) {
                //   qualType = qualType.getNonReferenceType();
                // }
                // qualType = qualType.getUnqualifiedType();
                // qualType = qualType.getNonReferenceType();
                // qualType = qualType.getCanonicalType();
                a_class.aliases.push_back(alias);
                std::vector<std::string> types =
                    gad_classes_and_functions.get_application_classes(
                        alias.base_name);
                for (auto type : types) {
                  Application application;
                  application.class_name = type;
                  application.signature = "";
                  a_class.applications.push_back(application);
                }
                // Application application;
                // application.class_name = get_application_type(qualType);
                // application.signature = "";
                // a_class.applications.push_back(application);
                // if (gad_classes_and_functions.has_class(alias.base_name)) {
                //   std::vector<std::string> constructors =
                //       gad_classes_and_functions.get_constructors(alias.base_name);
                //   std::vector<std::pair<std::string, std::string>>
                //   applications; for (auto constructor : constructors) {
                //     applications.push_back(std::pair<std::string,
                //     std::string>(
                //         alias.base_name, constructor));
                //   }
                //   std::string destructor =
                //       gad_classes_and_functions.get_destructor(alias.base_name);
                //   if (destructor != "class") {
                //     applications.push_back(std::pair<std::string,
                //     std::string>(
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
            }
            // i = 1;
            // while (i < a_class.fields.size()) {
            //   if (a_class.fields[i] == a_class.fields[i - 1] ||
            //       (a_class.fields[i].substr(
            //            0, a_class.fields[i].find_last_of('}') + 1) != "" &&
            //        a_class.fields[i - 1].substr(
            //            0, a_class.fields[i - 1].find_last_of('}') + 1) != ""
            //            &&
            //        a_class.fields[i].substr(
            //            0, a_class.fields[i].find_last_of('}') + 1) ==
            //            a_class.fields[i - 1].substr(
            //                0, a_class.fields[i - 1].find_last_of('}') + 1))
            //                ||
            //       a_class.fields[i].find(a_class.fields[i - 1]) !=
            //           std::string::npos) {
            //     a_class.fields.erase(a_class.fields.begin() + i - 1);
            //   } else {
            //     i++;
            //   }
            // }
            // i = 0;
            // while (i < a_class.fields.size()) {
            //   if (a_class.fields[i] == "union" || a_class.fields[i] ==
            //   "struct"
            //   ||
            //       a_class.fields[i] == "class" || a_class.fields[i] ==
            //       "enum")
            //       {
            //     a_class.fields.erase(a_class.fields.begin() + i);
            //   } else {
            //     i++;
            //   }
            // }
            //   std::cout << a_class.class_name << "\t" << a_class.base_class
            //             << std::endl;
            //   for (auto template_parameter : a_class.template_parameters) {
            //     std::cout << template_parameter << "\t";
            //   }
            //   std::cout << std::endl;
            gad_classes_and_functions.push_back_class(a_class);
          }
        }
      }
    }
    return true;
  }

  // bool VisitCXXRecordDecl(CXXRecordDecl *Declaration) {
  //   if (Declaration->isThisDeclarationADefinition()) {
  //     clang::SourceLocation loc = Declaration->getLocation();
  //     clang::SourceManager &SM = Context->getSourceManager();
  //     clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
  //     std::string file_path = presumedLoc.getFilename();
  //     if (file_path.find(gad_project_path) != std::string::npos) {
  //       // std::cout << Declaration->getNameAsString() << std::endl;
  //       // Declaration->dump();
  //       // std::cout << std::endl;
  //       auto canonical_declaration = Declaration->getCanonicalDecl();
  //       canonical_declaration->dump();
  //       Declaration->dump();
  //       std::string class_name = Declaration->getNameAsString();
  //       if (!gad_classes_and_functions.has_class(class_name)) {
  //         Class a_class;
  //         a_class.class_name = class_name;
  //         a_class.is_template = 0;
  //         // if (Declaration->getNumBases() > 0) {
  //         //   a_class.base_class =
  //         //       Declaration->bases_begin()->getType().getAsString();
  //         // }
  //         for (auto its_base_class : Declaration->bases()) {
  //           a_class.base_class.push_back(
  //               its_base_class.getType().getAsString());
  //         }
  //         const DeclContext *context = Declaration->getDeclContext();
  //         bool b = 0;
  //         while (context) {
  //           if (const NamespaceDecl *namespaceDecl =
  //                   dyn_cast<NamespaceDecl>(context)) {
  //             b = 1;
  //             if (a_class.its_namespace == "") {
  //               a_class.its_namespace = namespaceDecl->getNameAsString();
  //             } else {
  //               a_class.its_namespace = namespaceDecl->getNameAsString() +
  //                                       "::" + a_class.its_namespace;
  //             }
  //           }
  //           context = context->getParent();
  //         }
  //         if (b == 0) {
  //           a_class.its_namespace = "";
  //         }
  //         int i = 0;
  //         for (const auto *decl : Declaration->decls()) {
  //           if (i == 0) {
  //             continue;
  //           }
  //           // if (const CXXRecordDecl *Record =
  //           dyn_cast<CXXRecordDecl>(decl))
  //           // {
  //           //   if (i == 0) {
  //           //     continue;
  //           //   }
  //           //   const SourceManager &sourceManager =
  //           //   Context->getSourceManager(); SourceRange srcRange =
  //           //   Record->getSourceRange();
  //           //   srcRange.setEnd(Lexer::getLocForEndOfToken(
  //           //       srcRange.getEnd(), 0, sourceManager,
  //           //       Context->getLangOpts()));
  //           //   bool Invalid = false;
  //           //   StringRef srcText = Lexer::getSourceText(
  //           //       CharSourceRange::getTokenRange(srcRange), sourceManager,
  //           //       Context->getLangOpts(), &Invalid);
  //           //   // std::cout << std::string(srcText) << std::endl;
  //           //   std::string srcText_str = std::string(srcText);
  //           //   if (srcText_str[srcText_str.length() - 1] == ';') {
  //           //     srcText_str.erase(srcText_str.length() - 1);
  //           //   }
  //           //   // if (Record->isClass()) {
  //           //   //   if (srcText.find('{') != std::string::npos) {
  //           //   //     srcText_str.erase(srcText_str.find_first_of('{'));
  //           //   //   }
  //           //   //   a_class.fields.push_back(srcText_str);
  //           //   // } else {
  //           //   //   a_class.fields.push_back(srcText_str);
  //           //   // }
  //           //   a_class.fields.push_back(srcText_str);
  //           // }
  //           if (const FieldDecl *Field = dyn_cast<FieldDecl>(decl)) {
  //             std::string field_type = Field->getType().getAsString();
  //             if (field_type.find(" ") != std::string::npos) {
  //               field_type =
  //                   field_type.substr(field_type.find_last_of(" ") + 1);
  //             }
  //             Application application;
  //             application.class_name = field_type;
  //             application.signature = "";
  //             a_class.applications.push_back(application);
  //             const SourceManager &sourceManager =
  //             Context->getSourceManager(); SourceRange srcRange =
  //             Field->getSourceRange();
  //             srcRange.setEnd(Lexer::getLocForEndOfToken(
  //                 srcRange.getEnd(), 0, sourceManager,
  //                 Context->getLangOpts()));
  //             bool Invalid = false;
  //             StringRef srcText = Lexer::getSourceText(
  //                 CharSourceRange::getTokenRange(srcRange), sourceManager,
  //                 Context->getLangOpts(), &Invalid);
  //             // std::cout << std::string(srcText) << std::endl;
  //             std::string srcText_str = std::string(srcText);
  //             if (srcText_str[srcText_str.length() - 1] == ';') {
  //               srcText_str.erase(srcText_str.length() - 1);
  //             }
  //             a_class.fields.push_back(srcText_str);
  //           }
  //           // else if (const EnumDecl *Enum = dyn_cast<EnumDecl>(decl)) {
  //           //   const SourceManager &sourceManager =
  //           //   Context->getSourceManager(); SourceRange srcRange =
  //           //   Enum->getSourceRange();
  //           //   srcRange.setEnd(Lexer::getLocForEndOfToken(
  //           //       srcRange.getEnd(), 0, sourceManager,
  //           //       Context->getLangOpts()));
  //           //   bool Invalid = false;
  //           //   StringRef srcText = Lexer::getSourceText(
  //           //       CharSourceRange::getTokenRange(srcRange), sourceManager,
  //           //       Context->getLangOpts(), &Invalid);
  //           //   // std::cout << std::string(srcText) << std::endl;
  //           //   std::string srcText_str = std::string(srcText);
  //           //   if (srcText_str[srcText_str.length() - 1] == ';') {
  //           //     srcText_str.erase(srcText_str.length() - 1);
  //           //   }
  //           //   a_class.fields.push_back(srcText_str);
  //           // }
  //           else if (const TypeAliasDecl *TypeAlias =
  //                        dyn_cast<TypeAliasDecl>(decl)) {
  //             Alias alias;
  //             alias.alias_name = TypeAlias->getNameAsString();
  //             alias.base_name = TypeAlias->getUnderlyingType().getAsString();
  //             a_class.aliases.push_back(alias);
  //             Application application;
  //             application.class_name = alias.base_name;
  //             application.signature = "";
  //             a_class.applications.push_back(application);
  //             // if (gad_classes_and_functions.has_class(alias.base_name)) {
  //             //   std::vector<std::string> constructors =
  //             // gad_classes_and_functions.get_constructors(alias.base_name);
  //             //   std::vector<std::pair<std::string, std::string>>
  //             //   applications; for (auto constructor : constructors) {
  //             //     applications.push_back(std::pair<std::string,
  //             std::string>(
  //             //         alias.base_name, constructor));
  //             //   }
  //             //   std::string destructor =
  //             // gad_classes_and_functions.get_destructor(alias.base_name);
  //             //   if (destructor != "class") {
  //             //     applications.push_back(std::pair<std::string,
  //             std::string>(
  //             //         alias.base_name, destructor));
  //             //   }
  //             //   for (auto application : applications) {
  //             //     Application a_application;
  //             //     a_application.class_name = application.first;
  //             //     a_application.signature = application.second;
  //             //     a_class.applications.push_back(a_application);
  //             //   }
  //             // }
  //           }
  //           i++;
  //         }
  //         // i = 1;
  //         // while (i < a_class.fields.size()) {
  //         //   if (a_class.fields[i] == a_class.fields[i - 1] ||
  //         //       (a_class.fields[i].substr(
  //         //            0, a_class.fields[i].find_last_of('}') + 1) != "" &&
  //         //        a_class.fields[i - 1].substr(
  //         //            0, a_class.fields[i - 1].find_last_of('}') + 1) != ""
  //         &&
  //         //        a_class.fields[i].substr(
  //         //            0, a_class.fields[i].find_last_of('}') + 1) ==
  //         //            a_class.fields[i - 1].substr(
  //         //                0, a_class.fields[i - 1].find_last_of('}') + 1))
  //         ||
  //         //       a_class.fields[i].find(a_class.fields[i - 1]) !=
  //         //           std::string::npos) {
  //         //     a_class.fields.erase(a_class.fields.begin() + i - 1);
  //         //   } else {
  //         //     i++;
  //         //   }
  //         // }
  //         // i = 0;
  //         // while (i < a_class.fields.size()) {
  //         //   if (a_class.fields[i] == "union" || a_class.fields[i] ==
  //         "struct"
  //         //   ||
  //         //       a_class.fields[i] == "class" || a_class.fields[i] ==
  //         "enum")
  //         //       {
  //         //     a_class.fields.erase(a_class.fields.begin() + i);
  //         //   } else {
  //         //     i++;
  //         //   }
  //         // }
  //         //   std::cout << a_class.class_name << "\t" << a_class.base_class
  //         //             << std::endl;
  //         //   for (auto template_parameter : a_class.template_parameters) {
  //         //     std::cout << template_parameter << "\t";
  //         //   }
  //         //   std::cout << std::endl;
  //         gad_classes_and_functions.push_back_class(a_class);
  //       }
  //       // gad_classes_and_functions.cout();
  //     }
  //   }
  //   return true;
  // }

  bool VisitFunctionTemplateDecl(FunctionTemplateDecl *Declaration) {
    if (Declaration->isThisDeclarationADefinition()) {
      clang::SourceLocation loc = Declaration->getLocation();
      clang::SourceManager &SM = Context->getSourceManager();
      clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
      std::string file_path = presumedLoc.getFilename();
      if (file_path.find(gad_project_path) != std::string::npos) {
        //     std::cout << get_signature(Declaration->getAsFunction()) <<
        //     std::endl; Declaration->dump(); std::cout << std::endl;
        auto CanonicalDeclaration = Declaration->getCanonicalDecl();
        // auto canonical_declaration = Declaration->getCanonicalDecl();
        // canonical_declaration->dump();
        // Declaration->dump();
        FunctionDecl *Func = Declaration->getTemplatedDecl();
        FunctionDecl *CanonicalFunc = Func->getCanonicalDecl();
        if (!isa<CXXMethodDecl>(Func)) {
          std::string function_name = CanonicalDeclaration->getNameAsString();
          std::string signature = get_signature(CanonicalFunc);
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
            // if (return_type.find(" ") != std::string::npos) {
            //   return_type =
            //       return_type.substr(return_type.find_last_of(" ") + 1);
            // }
            // QualType qualType = Func->getReturnType();
            // std::cout << qualType.getAsString() << std::endl;
            // if (qualType->isReferenceType()) {
            //   qualType = qualType.getNonReferenceType();
            // }
            // qualType = qualType.getUnqualifiedType();
            // std::cout << qualType.getAsString() << std::endl;
            // qualType = qualType.getNonReferenceType();
            // std::cout << qualType.getAsString() << std::endl;
            // qualType = qualType.getLocalUnqualifiedType();
            // std::cout << qualType.getAsString() << std::endl;
            // qualType = qualType.getCanonicalType();
            // std::cout << qualType.getAsString() << std::endl;
            // qualType = qualType.getCanonicalType();
            std::vector<std::string> types =
                gad_classes_and_functions.get_application_classes(return_type);
            for (auto type : types) {
              Application application;
              application.class_name = type;
              application.signature = "";
              function.applications.push_back(application);
            }
            // Application application;
            // application.class_name = get_application_type(qualType);
            // application.signature = "";
            // function.applications.push_back(application);
            for (const ParmVarDecl *param : Func->parameters()) {
              std::string param_type = param->getType().getAsString();
              // if (param_type.find(" ") != std::string::npos) {
              //   param_type =
              //       param_type.substr(param_type.find_last_of(" ") + 1);
              // }
              // QualType qualType = param->getType();
              // std::cout << qualType.getAsString() << std::endl;
              // if (qualType->isReferenceType()) {
              //   qualType = qualType.getNonReferenceType();
              // }
              // qualType = qualType.getUnqualifiedType();
              // std::cout << qualType.getAsString() << std::endl;
              // qualType = qualType.getNonReferenceType();
              // std::cout << qualType.getAsString() << std::endl;
              // qualType = qualType.getLocalUnqualifiedType();
              // std::cout << qualType.getAsString() << std::endl;
              // qualType = qualType.getCanonicalType();
              // std::cout << qualType.getAsString() << std::endl;
              // qualType = qualType.getCanonicalType();
              std::vector<std::string> types =
                  gad_classes_and_functions.get_application_classes(param_type);
              for (auto type : types) {
                Application application;
                application.class_name = type;
                application.signature = "";
                function.applications.push_back(application);
              }
              // Application application;
              // application.class_name = get_application_type(qualType);
              // application.signature = "";
              // function.applications.push_back(application);
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
            std::vector<Application> stmt_applications =
                function_stmt_visitor.get_applications();
            for (auto application : stmt_applications) {
              function.applications.push_back(application);
            }
            gad_classes_and_functions.push_back_function(function);
          }
          for (FunctionDecl *Spec : Declaration->specializations()) {
            const TemplateArgumentList *TAL =
                Spec->getTemplateSpecializationArgs();
            std::vector<std::pair<std::string, std::string>> applications;
            if (TAL) {
              std::vector<std::string> specialization_parameter;
              for (const TemplateArgument &Arg : TAL->asArray()) {
                if (Arg.getKind() == TemplateArgument::Type) {
                  //   std::cout << "Template Argument Type: "
                  //             << Arg.getAsType().getAsString() << "\n";
                  std::string type_name = Arg.getAsType().getAsString();
                  // if (type_name.find(" ") != std::string::npos) {
                  //   type_name =
                  //       type_name.substr(type_name.find_last_of(" ") + 1);
                  // }
                  specialization_parameter.push_back(type_name);
                  // QualType qualType = Arg.getAsType();
                  // if (qualType->isReferenceType()) {
                  //   qualType = qualType.getNonReferenceType();
                  // }
                  // qualType = qualType.getUnqualifiedType();
                  // qualType = qualType.getNonReferenceType();
                  // qualType = qualType.getCanonicalType();
                  std::vector<std::string> types =
                      gad_classes_and_functions.get_application_classes(
                          type_name);
                  for (auto type : types) {
                    std::pair<std::string, std::string> application;
                    application.first = type;
                    application.second = "";
                    applications.push_back(application);
                  }
                  // std::pair<std::string, std::string> application;
                  // application.first = get_application_type(qualType);
                  // application.second = "";
                  // applications.push_back(application);
                }
              }
              gad_classes_and_functions.push_back_specialization_parameter(
                  "class", signature, specialization_parameter);
              gad_classes_and_functions.push_back_applications(
                  "class", signature, applications);
            }
          }
        } else {
          const CXXRecordDecl *CanonicalParentClass =
              dyn_cast<CXXRecordDecl>(Func->getParent())->getCanonicalDecl();
          std::string class_name = CanonicalParentClass->getNameAsString();
          if (gad_classes_and_functions.has_class(class_name)) {
            // std::cout << "Class Declaration " << class_name <<
            // std::endl;
            std::string signature = get_signature(CanonicalFunc);
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
                  // if (param_type.find(" ") != std::string::npos) {
                  //   param_type =
                  //       param_type.substr(param_type.find_last_of(" ") + 1);
                  // }
                  // QualType qualType = param->getType();
                  // if (qualType->isReferenceType()) {
                  //   qualType = qualType.getNonReferenceType();
                  // }
                  // qualType = qualType.getUnqualifiedType();
                  // qualType = qualType.getNonReferenceType();
                  // qualType = qualType.getCanonicalType();
                  std::vector<std::string> types =
                      gad_classes_and_functions.get_application_classes(
                          param_type);
                  for (auto type : types) {
                    Application application;
                    application.class_name = type;
                    application.signature = "";
                    constructor.applications.push_back(application);
                  }
                  // Application application;
                  // application.class_name = get_application_type(qualType);
                  // application.signature = "";
                  // constructor.applications.push_back(application);
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
                std::vector<Application> stmt_applications =
                    function_stmt_visitor.get_applications();
                for (auto application : stmt_applications) {
                  constructor.applications.push_back(application);
                }
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
                std::vector<Application> stmt_applications =
                    function_stmt_visitor.get_applications();
                for (auto application : stmt_applications) {
                  destructor.applications.push_back(application);
                }
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
                // if (return_type.find(" ") != std::string::npos) {
                //   return_type =
                //       return_type.substr(return_type.find_last_of(" ") + 1);
                // }
                // QualType qualType = Func->getReturnType();
                // if (qualType->isReferenceType()) {
                //   qualType = qualType.getNonReferenceType();
                // }
                // qualType = qualType.getUnqualifiedType();
                // qualType = qualType.getNonReferenceType();
                // qualType = qualType.getCanonicalType();
                std::vector<std::string> types =
                    gad_classes_and_functions.get_application_classes(
                        return_type);
                for (auto type : types) {
                  Application application;
                  application.class_name = type;
                  application.signature = "";
                  method.applications.push_back(application);
                }
                // Application application;
                // application.class_name = get_application_type(qualType);
                // application.signature = "";
                // method.applications.push_back(application);
                for (const ParmVarDecl *param : Func->parameters()) {
                  std::string param_type = param->getType().getAsString();
                  // if (param_type.find(" ") != std::string::npos) {
                  //   param_type =
                  //       param_type.substr(param_type.find_last_of(" ") + 1);
                  // }
                  // QualType qualType = param->getType();
                  // if (qualType->isReferenceType()) {
                  //   qualType = qualType.getNonReferenceType();
                  // }
                  // qualType = qualType.getUnqualifiedType();
                  // qualType = qualType.getNonReferenceType();
                  // qualType = qualType.getCanonicalType();
                  std::vector<std::string> types =
                      gad_classes_and_functions.get_application_classes(
                          param_type);
                  for (auto type : types) {
                    Application application;
                    application.class_name = type;
                    application.signature = "";
                    method.applications.push_back(application);
                  }
                  // Application application;
                  // application.class_name = get_application_type(qualType);
                  // application.signature = "";
                  // method.applications.push_back(application);
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
                std::vector<Application> stmt_applications =
                    function_stmt_visitor.get_applications();
                for (auto application : stmt_applications) {
                  method.applications.push_back(application);
                }
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
                  std::vector<std::pair<std::string, std::string>> applications;
                  for (const TemplateArgument &Arg : TAL->asArray()) {
                    if (Arg.getKind() == TemplateArgument::Type) {
                      //   std::cout << "Template Argument Type: "
                      //             << Arg.getAsType().getAsString() << "\n";
                      std::string type_name = Arg.getAsType().getAsString();
                      // if (type_name.find(" ") != std::string::npos) {
                      //   type_name =
                      //       type_name.substr(type_name.find_last_of(" ") +
                      //       1);
                      // }
                      specialization_parameter.push_back(type_name);
                      // QualType qualType = Arg.getAsType();
                      // if (qualType->isReferenceType()) {
                      //   qualType = qualType.getNonReferenceType();
                      // }
                      // qualType = qualType.getUnqualifiedType();
                      // qualType = qualType.getNonReferenceType();
                      // qualType = qualType.getCanonicalType();
                      std::vector<std::string> types =
                          gad_classes_and_functions.get_application_classes(
                              type_name);
                      for (auto type : types) {
                        std::pair<std::string, std::string> application;
                        application.first = type;
                        application.second = "";
                        applications.push_back(application);
                      }
                      // std::pair<std::string, std::string> application;
                      // application.first = get_application_type(qualType);
                      // application.second = "";
                      // applications.push_back(application);
                    }
                  }
                  gad_classes_and_functions.push_back_specialization_parameter(
                      class_name, signature, specialization_parameter);
                  gad_classes_and_functions.push_back_applications(
                      class_name, signature, applications);
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
        auto CanonicalDeclaration = Declaration->getCanonicalDecl();
        // auto canonical_declaration = Declaration->getCanonicalDecl();
        // canonical_declaration->dump();
        // Declaration->dump();
        if (!isa<CXXMethodDecl>(CanonicalDeclaration)) {
          std::string function_name = CanonicalDeclaration->getNameAsString();
          std::string signature = get_signature(CanonicalDeclaration);
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
            // if (return_type.find(" ") != std::string::npos) {
            //   return_type =
            //       return_type.substr(return_type.find_last_of(" ") + 1);
            // }
            // QualType qualType = FunctionDeclaration->getReturnType();
            // if (qualType->isReferenceType()) {
            //   qualType = qualType.getNonReferenceType();
            // }
            // qualType = qualType.getUnqualifiedType();
            // qualType = qualType.getNonReferenceType();
            // qualType = qualType.getCanonicalType(); std::set<std::string>
            // types;
            std::vector<std::string> types =
                gad_classes_and_functions.get_application_classes(return_type);
            for (auto type : types) {
              Application application;
              application.class_name = type;
              application.signature = "";
              function.applications.push_back(application);
            }
            // Application application;
            // application.class_name = get_application_type(qualType);
            // application.signature = "";
            // function.applications.push_back(application);
            for (const ParmVarDecl *param : Declaration->parameters()) {
              std::string param_type = param->getType().getAsString();
              // if (param_type.find(" ") != std::string::npos) {
              //   param_type =
              //       param_type.substr(param_type.find_last_of(" ") + 1);
              // }
              // QualType qualType = param->getType();
              // if (qualType->isReferenceType()) {
              //   qualType = qualType.getNonReferenceType();
              // }
              // qualType = qualType.getUnqualifiedType();
              // qualType = qualType.getNonReferenceType();
              // qualType = qualType.getCanonicalType();
              std::vector<std::string> types =
                  gad_classes_and_functions.get_application_classes(param_type);
              for (auto type : types) {
                Application application;
                application.class_name = type;
                application.signature = "";
                function.applications.push_back(application);
              }
              // Application application;
              // application.class_name = get_application_type(qualType);
              // application.signature = "";
              // function.applications.push_back(application);
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
            std::vector<Application> stmt_applications =
                function_stmt_visitor.get_applications();
            for (auto application : stmt_applications) {
              function.applications.push_back(application);
            }
            gad_classes_and_functions.push_back_function(function);
          }
        } else {
          const CXXRecordDecl *CanonicalParentClass =
              dyn_cast<CXXRecordDecl>(CanonicalDeclaration->getParent())
                  ->getCanonicalDecl();
          std::string class_name = CanonicalParentClass->getNameAsString();
          if (gad_classes_and_functions.has_class(class_name)) {
            // std::cout << "Class Declaration " << class_name <<
            // std::endl;
            std::string signature = get_signature(CanonicalDeclaration);
            if (!gad_classes_and_functions.has_function(class_name,
                                                        signature)) {
              if (isa<CXXConstructorDecl>(CanonicalDeclaration)) {
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
                  // if (param_type.find(" ") != std::string::npos) {
                  //   param_type =
                  //       param_type.substr(param_type.find_last_of(" ") + 1);
                  // }
                  // QualType qualType = param->getType();
                  // std::cout << qualType.getAsString() << std::endl;
                  // if (qualType->isReferenceType()) {
                  //   qualType = qualType.getNonReferenceType();
                  // }
                  // qualType = qualType.getUnqualifiedType();
                  // std::cout << qualType.getAsString() << std::endl;
                  // qualType = qualType.getNonReferenceType();
                  // std::cout << qualType.getAsString() << std::endl;
                  // qualType = qualType.getLocalUnqualifiedType();
                  // std::cout << qualType.getAsString() << std::endl;
                  // qualType = qualType.getCanonicalType();
                  // std::cout << qualType.getAsString() << std::endl;
                  // qualType = qualType.getCanonicalType();
                  std::vector<std::string> types =
                      gad_classes_and_functions.get_application_classes(
                          param_type);
                  for (auto type : types) {
                    Application application;
                    application.class_name = type;
                    application.signature = "";
                    constructor.applications.push_back(application);
                  }
                  // Application application;
                  // application.class_name = get_application_type(qualType);
                  // application.signature = "";
                  // constructor.applications.push_back(application);
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
                std::vector<Application> stmt_applications =
                    function_stmt_visitor.get_applications();
                for (auto application : stmt_applications) {
                  constructor.applications.push_back(application);
                }
                gad_classes_and_functions.push_back_constructor(class_name,
                                                                constructor);
              } else if (isa<CXXDestructorDecl>(CanonicalDeclaration)) {
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
                std::vector<Application> stmt_applications =
                    function_stmt_visitor.get_applications();
                for (auto application : stmt_applications) {
                  destructor.applications.push_back(application);
                }
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
                // if (return_type.find(" ") != std::string::npos) {
                //   return_type =
                //       return_type.substr(return_type.find_last_of(" ") + 1);
                // }
                // QualType qualType = FunctionDeclaration->getReturnType();
                // if (qualType->isReferenceType()) {
                //   qualType = qualType.getNonReferenceType();
                // }
                // qualType = qualType.getUnqualifiedType();
                // qualType = qualType.getNonReferenceType();
                // qualType = qualType.getCanonicalType();
                std::vector<std::string> types =
                    gad_classes_and_functions.get_application_classes(
                        return_type);
                for (auto type : types) {
                  Application application;
                  application.class_name = type;
                  application.signature = "";
                  method.applications.push_back(application);
                }
                // Application application;
                // application.class_name = get_application_type(qualType);
                // application.signature = "";
                // method.applications.push_back(application);
                for (const ParmVarDecl *param : Declaration->parameters()) {
                  std::string param_type = param->getType().getAsString();
                  // if (param_type.find(" ") != std::string::npos) {
                  //   param_type =
                  //       param_type.substr(param_type.find_last_of(" ") + 1);
                  // }
                  // QualType qualType = param->getType();
                  // if (qualType->isReferenceType()) {
                  //   qualType = qualType.getNonReferenceType();
                  // }
                  // qualType = qualType.getUnqualifiedType();
                  // qualType = qualType.getNonReferenceType();
                  // qualType = qualType.getCanonicalType();
                  std::vector<std::string> types =
                      gad_classes_and_functions.get_application_classes(
                          param_type);
                  for (auto type : types) {
                    Application application;
                    application.class_name = type;
                    application.signature = "";
                    method.applications.push_back(application);
                  }
                  // Application application;
                  // application.class_name = get_application_type(qualType);
                  // application.signature = "";
                  // method.applications.push_back(application);
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
                std::vector<Application> stmt_applications =
                    function_stmt_visitor.get_applications();
                for (auto application : stmt_applications) {
                  method.applications.push_back(application);
                }
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

  bool VisitEnumDecl(EnumDecl *Declaration) {
    if (Declaration->isThisDeclarationADefinition()) {
      clang::SourceLocation loc = Declaration->getLocation();
      clang::SourceManager &SM = Context->getSourceManager();
      clang::PresumedLoc presumedLoc = SM.getPresumedLoc(loc);
      std::string file_path = presumedLoc.getFilename();
      if (file_path.find(gad_project_path) != std::string::npos) {
        auto CanonicalDeclaration = Declaration->getCanonicalDecl();
        // Declaration->dump();
        std::string enum_name = CanonicalDeclaration->getNameAsString();
        for (auto *D : CanonicalDeclaration->getDeclContext()->decls()) {
          if (auto *TD = dyn_cast<TypedefDecl>(D)) {
            if (auto *TRD = TD->getUnderlyingType()->getAsTagDecl()) {
              if (auto *ETRD = dyn_cast<EnumDecl>(TRD)) {
                if (ETRD->getCanonicalDecl() == CanonicalDeclaration) {
                  enum_name = TD->getNameAsString();
                  break;
                }
              }
            }
          }
        }
        if (!gad_classes_and_functions.has_class(enum_name)) {
          Class a_class;
          a_class.class_name = enum_name;
          a_class.record_type = MyRecordType::EnumType;
          for (auto it = Declaration->enumerator_begin();
               it != Declaration->enumerator_end(); ++it) {
            EnumConstantDecl *ECD = *it;
            a_class.fields.push_back(ECD->getNameAsString());
            // std::cout << "Enumerator: " << ECD->getNameAsString() <<
            // std::endl;
            llvm::APSInt value = ECD->getInitVal();
            a_class.enum_int_constants.push_back(value.getExtValue());
            // std::cout << "Value: " << value.getSExtValue() << std::endl;
          }
          gad_classes_and_functions.push_back_class(a_class);
        }
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
  // gad_classes_and_functions.cout();
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
      if (a_class.class_name == class_name) {
        if (signature == "") {
          return a_class.applications;
        } else {
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
  int i = 0;
  while (i < applications->size()) {
    auto application = (*applications)[i];
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
    i++;
  }
}

json ClassesAndFunctions::get_simple_class(std::string class_name) {
  json j;
  for (auto a_class : classes) {
    if (a_class.class_name == class_name) {
      if (a_class.record_type != MyRecordType::EnumType) {
        std::string record_type;
        switch (a_class.record_type) {
        case MyRecordType::ClassType:
          record_type = "class";
          break;
        case MyRecordType::StructType:
          record_type = "struct";
          break;
        case MyRecordType::UnionType:
          record_type = "union";
          break;
        default:
          break;
        }
        j[a_class.its_namespace] = json::object();
        j[a_class.its_namespace][record_type] = json::object();
        j[a_class.its_namespace][record_type][a_class.class_name] =
            json::object();
        if (record_type == "class") {
          j[a_class.its_namespace][record_type][a_class.class_name]
           ["base_class"] = json::array();
          j[a_class.its_namespace][record_type][a_class.class_name]
           ["base_class"] = a_class.base_class;
        }
        j[a_class.its_namespace][record_type][a_class.class_name]
         ["is_template"] = json::object();
        if (a_class.is_template == 1) {
          j[a_class.its_namespace][record_type][a_class.class_name]
           ["is_template"] = "true";
          j[a_class.its_namespace][record_type][a_class.class_name]
           ["template_parameters"] = json::array();
          j[a_class.its_namespace][record_type][a_class.class_name]
           ["template_parameters"] = a_class.template_parameters;
          j[a_class.its_namespace][record_type][a_class.class_name]
           ["specialization_parameters"] = json::array();
          for (auto specialization_parameter :
               a_class.specialization_parameters) {
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
            j[a_class.its_namespace][record_type][a_class.class_name]
             ["specialization_parameters"]
                 .push_back(specialization_parameter_string);
          }
        } else {
          j[a_class.its_namespace][record_type][a_class.class_name]
           ["is_template"] = "false";
        }
        j[a_class.its_namespace][record_type][a_class.class_name]["alias"] =
            json::array();
        for (auto alias : a_class.aliases) {
          std::string alias_string =
              "using " + alias.alias_name + " = " + alias.base_name;
          j[a_class.its_namespace][record_type][a_class.class_name]["alias"]
              .push_back(alias_string);
        }
        j[a_class.its_namespace][record_type][a_class.class_name]
         ["constructor"] = json::object();
        for (auto constructor : a_class.constructors) {
          j[a_class.its_namespace][record_type][a_class.class_name]
           ["constructor"][constructor.signature] = json::object();
          j[a_class.its_namespace][record_type][a_class.class_name]
           ["constructor"][constructor.signature]["function_body"] =
               json::object();
          j[a_class.its_namespace][record_type][a_class.class_name]
           ["constructor"][constructor.signature]["parameters"] = json::array();
        }
        j[a_class.its_namespace][record_type][a_class.class_name]
         ["destructor"] = json::object();
        j[a_class.its_namespace][record_type][a_class.class_name]["destructor"]
         ["function_body"] = json::object();
        j[a_class.its_namespace][record_type][a_class.class_name]["destructor"]
         ["signature"] = json::object();
        j[a_class.its_namespace][record_type][a_class.class_name]["destructor"]
         ["signature"] = a_class.destructor.signature;
        j[a_class.its_namespace][record_type][a_class.class_name]["fields"] =
            json::array();
        j[a_class.its_namespace][record_type][a_class.class_name]["fields"] =
            a_class.fields;
        j[a_class.its_namespace][record_type][a_class.class_name]["methods"] =
            json::object();
        for (auto method : a_class.methods) {
          j[a_class.its_namespace][record_type][a_class.class_name]["methods"]
           [method.signature] = json::object();
          j[a_class.its_namespace][record_type][a_class.class_name]["methods"]
           [method.signature]["function_body"] = json::object();
          j[a_class.its_namespace][record_type][a_class.class_name]["methods"]
           [method.signature]["parameters"] = json::array();
          j[a_class.its_namespace][record_type][a_class.class_name]["methods"]
           [method.signature]["return_type"] = json::object();
        }
      } else {
        j[a_class.its_namespace] = json::object();
        j[a_class.its_namespace]["enum"] = json::object();
        j[a_class.its_namespace]["enum"][a_class.class_name] = json::object();
        j[a_class.its_namespace]["enum"][a_class.class_name]["enum_constants"] =
            json::array();
        j[a_class.its_namespace]["enum"][a_class.class_name]["enum_constants"] =
            a_class.fields;
        j[a_class.its_namespace]["enum"][a_class.class_name]
         ["enum_int_constants"] = json::array();
        j[a_class.its_namespace]["enum"][a_class.class_name]
         ["enum_int_constants"] = a_class.enum_int_constants;
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
        std::string record_type;
        switch (a_class.record_type) {
        case MyRecordType::ClassType:
          record_type = "class";
          break;
        case MyRecordType::StructType:
          record_type = "struct";
          break;
        case MyRecordType::UnionType:
          record_type = "union";
          break;
        default:
          break;
        }
        bool b = 0;
        if (b == 0) {
          for (auto constructor : a_class.constructors) {
            if (constructor.signature == signature) {
              b = 1;
              j[a_class.its_namespace] = json::object();
              j[a_class.its_namespace][record_type] = json::object();
              j[a_class.its_namespace][record_type][a_class.class_name] =
                  json::object();
              j[a_class.its_namespace][record_type][a_class.class_name]
               ["constructor"] = json::object();
              j[a_class.its_namespace][record_type][a_class.class_name]
               ["constructor"][constructor.signature] = json::object();
              j[a_class.its_namespace][record_type][a_class.class_name]
               ["constructor"][constructor.signature]["function_body"] =
                   json::object();
              j[a_class.its_namespace][record_type][a_class.class_name]
               ["constructor"][constructor.signature]["function_body"] =
                   constructor.function_body;
              j[a_class.its_namespace][record_type][a_class.class_name]
               ["constructor"][constructor.signature]["parameters"] =
                   json::array();
              j[a_class.its_namespace][record_type][a_class.class_name]
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
            j[a_class.its_namespace][record_type] = json::object();
            j[a_class.its_namespace][record_type][a_class.class_name] =
                json::object();
            j[a_class.its_namespace][record_type][a_class.class_name]
             ["destructor"] = json::object();
            j[a_class.its_namespace][record_type][a_class.class_name]
             ["destructor"]["function_body"] = json::object();
            j[a_class.its_namespace][record_type][a_class.class_name]
             ["destructor"]["function_body"] = a_class.destructor.function_body;
            j[a_class.its_namespace][record_type][a_class.class_name]
             ["destructor"]["signature"] = json::object();
            j[a_class.its_namespace][record_type][a_class.class_name]
             ["destructor"]["signature"] = a_class.destructor.signature;
          }
        } else {
          break;
        }
        if (b == 0) {
          for (auto method : a_class.methods) {
            if (method.signature == signature) {
              j[a_class.its_namespace] = json::object();
              j[a_class.its_namespace][record_type] = json::object();
              j[a_class.its_namespace][record_type][a_class.class_name] =
                  json::object();
              j[a_class.its_namespace][record_type][a_class.class_name]
               ["methods"] = json::object();
              j[a_class.its_namespace][record_type][a_class.class_name]
               ["methods"][method.signature] = json::object();
              j[a_class.its_namespace][record_type][a_class.class_name]
               ["methods"][method.signature]["function_body"] = json::object();
              j[a_class.its_namespace][record_type][a_class.class_name]
               ["methods"][method.signature]["function_body"] =
                   method.function_body;
              j[a_class.its_namespace][record_type][a_class.class_name]
               ["methods"][method.signature]["parameters"] = json::array();
              j[a_class.its_namespace][record_type][a_class.class_name]
               ["methods"][method.signature]["parameters"] = method.parameters;
              j[a_class.its_namespace][record_type][a_class.class_name]
               ["methods"][method.signature]["is_template"] = json::object();
              if (method.is_template == 1) {
                j[a_class.its_namespace][record_type][a_class.class_name]
                 ["methods"][method.signature]["is_template"] = "true";
                j[a_class.its_namespace][record_type][a_class.class_name]
                 ["methods"][method.signature]["template_parameters"] =
                     json::array();
                j[a_class.its_namespace][record_type][a_class.class_name]
                 ["methods"][method.signature]["template_parameters"] =
                     method.template_parameters;
                j[a_class.its_namespace][record_type][a_class.class_name]
                 ["methods"][method.signature]["specialization_parameters"] =
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
                  j[a_class.its_namespace][record_type][a_class.class_name]
                   ["methods"][method.signature]["specialization_parameters"]
                       .push_back(specialization_parameter_string);
                }
              } else {
                j[a_class.its_namespace][record_type][a_class.class_name]
                 ["methods"][method.signature]["is_template"] = "false";
              }
              j[a_class.its_namespace][record_type][a_class.class_name]
               ["methods"][method.signature]["return_type"] = json::object();
              j[a_class.its_namespace][record_type][a_class.class_name]
               ["methods"][method.signature]["return_type"] =
                   method.return_type;
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
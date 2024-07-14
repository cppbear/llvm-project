#include "GetSignature.h"

// std::string get_application_type(QualType qualtype) {
//   auto tp = qualtype.getTypePtrOrNull();
//   if (tp->getAsTagDecl()) {
//     auto tag_decl = tp->getAsTagDecl()->getCanonicalDecl();
//     if (dyn_cast<CXXRecordDecl>(tag_decl)) {
//       auto record_decl =
//       dyn_cast<CXXRecordDecl>(tag_decl)->getCanonicalDecl(); if
//       (record_decl->getDescribedClassTemplate()) {
//         auto template_decl =
//             record_decl->getDescribedClassTemplate()->getCanonicalDecl();
//         std::string return_string = template_decl->getNameAsString();
//         if (return_string.find(" ") != std::string::npos) {
//           return_string.erase(0, return_string.find_last_of(" ") + 1);
//         }
//         return return_string;
//       }
//       // return record_decl->getNameAsString();
//       std::string return_string = record_decl->getNameAsString();
//       if (return_string.find(" ") != std::string::npos) {
//         return_string.erase(0, return_string.find_last_of(" ") + 1);
//       }
//       return return_string;
//     }
//     // return tag_decl->getNameAsString();
//     std::string return_string = tag_decl->getNameAsString();
//     if (return_string.find(" ") != std::string::npos) {
//       return_string.erase(0, return_string.find_last_of(" ") + 1);
//     }
//     return return_string;
//   }
//   // return qualtype.getAsString();
//   std::string return_string = qualtype.getAsString();
//   if (return_string.find(" ") != std::string::npos) {
//     return_string.erase(0, return_string.find_last_of(" ") + 1);
//   }
//   return return_string;
// }

std::string get_signature(const FunctionDecl *function) {
  // if (function->getPrimaryTemplate()) {
  //   function = function->getPrimaryTemplate()->getTemplatedDecl();
  // }
  auto functiondecl = function->getCanonicalDecl();
  // function->dump();
  std::string signature;
  if (!isa<CXXMethodDecl>(functiondecl)) {
    signature = functiondecl->getReturnType().getAsString() + " ";
    signature = signature + functiondecl->getNameAsString() + "(";
    int i = 0;
    for (const ParmVarDecl *param : functiondecl->parameters()) {
      if (i > 0) {
        signature = signature + ", ";
      }
      signature = signature + param->getType().getAsString();
      i++;
    }
    signature = signature + ")";
  } else {
    const CXXRecordDecl *ParentClass =
        dyn_cast<CXXRecordDecl>(functiondecl->getParent())->getCanonicalDecl();
    // ParentClass->dump();
    // if (isa<ClassTemplateSpecializationDecl>(ParentClass)) {
    //   dyn_cast<ClassTemplateSpecializationDecl>(ParentClass)
    //       ->getTemplatedDecl()
    //       ->dump();
    // }
    if (isa<CXXConstructorDecl>(functiondecl)) {
      signature = signature + ParentClass->getNameAsString() +
                  "::" + functiondecl->getNameAsString() + "(";
      int i = 0;
      for (const ParmVarDecl *param : functiondecl->parameters()) {
        if (i > 0) {
          signature = signature + ", ";
        }
        signature = signature + param->getType().getAsString();
        i++;
      }
      signature = signature + ")";
    } else if (isa<CXXDestructorDecl>(functiondecl)) {
      signature = signature + ParentClass->getNameAsString() +
                  "::" + functiondecl->getNameAsString() + "(";
      int i = 0;
      for (const ParmVarDecl *param : functiondecl->parameters()) {
        if (i > 0) {
          signature = signature + ", ";
        }
        signature = signature + param->getType().getAsString();
        i++;
      }
      signature = signature + ")";
    } else {
      signature = functiondecl->getReturnType().getAsString() + " ";
      signature = signature + ParentClass->getNameAsString() +
                  "::" + functiondecl->getNameAsString() + "(";
      int i = 0;
      for (const ParmVarDecl *param : functiondecl->parameters()) {
        if (i > 0) {
          signature = signature + ", ";
        }
        signature = signature + param->getType().getAsString();
        i++;
      }
      signature = signature + ")";
    }
  }
  // std::cout << signature << std::endl;
  return signature;
}

// void get_application_types(QualType qualtype, std::set<std::string> &types) {
//   qualtype = qualtype.getUnqualifiedType();
//   std::cout << qualtype.getAsString() << std::endl;
//   auto type = qualtype.getTypePtrOrNull();
//   if (!type) {
//     return;
//   }
//   // Directly dealing with built-in and referencing the record types
//   if (type->isBuiltinType()) {
//     types.insert(qualtype.getAsString());
//     return;
//   } else if (const auto *recordType = type->getAs<RecordType>()) {
//     types.insert(recordType->getDecl()->getNameAsString());
//     return;
//   } else if (const auto *enumType = type->getAs<EnumType>()) {
//     types.insert(enumType->getDecl()->getNameAsString());
//     return;
//   }
//   // Dealing with pointers and references (unwrapping them)
//   if (type->isPointerType() || type->isReferenceType()) {
//     get_application_types(qualtype->getPointeeType(), types);
//   } else if (auto arrayType = dyn_cast<ArrayType>(type)) {
//     get_application_types(arrayType->getElementType(), types);
//     // get_application_types(arrayType->getElementType(), types);
//   }

//   // Specifying the template types
//   if (const auto *templateSpecializationType =
//           dyn_cast<TemplateSpecializationType>(type)) {
//     types.insert(templateSpecializationType->getTemplateName()
//                      .getAsTemplateDecl()
//                      ->getNameAsString());
//     for (const auto &arg : templateSpecializationType->template_arguments())
//     {
//       if (arg.getKind() == TemplateArgument::Type) {
//         get_application_types(arg.getAsType(), types);
//       }
//     }
//   }

//   // if (qualtype.isConstQualified()) {
//   //   get_application_types(qualtype.getLocalUnqualifiedType(), types);
//   // }
// }
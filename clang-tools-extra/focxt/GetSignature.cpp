#include "GetSignature.h"

std::string get_signature(const FunctionDecl *function) {
  function = function->getCanonicalDecl();
  // function->dump();
  std::string signature;
  if (!isa<CXXMethodDecl>(function)) {
    signature = function->getReturnType().getAsString() + " ";
    signature = signature + function->getNameAsString() + "(";
    int i = 0;
    for (const ParmVarDecl *param : function->parameters()) {
      if (i > 0) {
        signature = signature + ", ";
      }
      signature = signature + param->getType().getAsString();
      i++;
    }
    signature = signature + ")";
  } else {
    const CXXRecordDecl *ParentClass =
        dyn_cast<CXXRecordDecl>(function->getParent());
    // ParentClass->dump();
    // if (isa<ClassTemplateSpecializationDecl>(ParentClass)) {
    //   dyn_cast<ClassTemplateSpecializationDecl>(ParentClass)
    //       ->getTemplatedDecl()
    //       ->dump();
    // }
    if (isa<CXXConstructorDecl>(function)) {
      signature = signature + ParentClass->getNameAsString() +
                  "::" + function->getNameAsString() + "(";
      int i = 0;
      for (const ParmVarDecl *param : function->parameters()) {
        if (i > 0) {
          signature = signature + ", ";
        }
        signature = signature + param->getType().getAsString();
        i++;
      }
      signature = signature + ")";
    } else if (isa<CXXDestructorDecl>(function)) {
      signature = signature + ParentClass->getNameAsString() +
                  "::" + function->getNameAsString() + "(";
      int i = 0;
      for (const ParmVarDecl *param : function->parameters()) {
        if (i > 0) {
          signature = signature + ", ";
        }
        signature = signature + param->getType().getAsString();
        i++;
      }
      signature = signature + ")";
    } else {
      signature = function->getReturnType().getAsString() + " ";
      signature = signature + ParentClass->getNameAsString() +
                  "::" + function->getNameAsString() + "(";
      int i = 0;
      for (const ParmVarDecl *param : function->parameters()) {
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
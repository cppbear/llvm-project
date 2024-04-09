#include "Condition.h"

namespace BrInfo {

void BaseCond::findDeclRefExpr(const Stmt *S) {
  switch (S->getStmtClass()) {
  default:
    break;
  case Stmt::DeclRefExprClass: {
    const DeclRefExpr *DeclRef = cast<DeclRefExpr>(S);
    if (DeclRef->getDecl()->getKind() == Decl::Kind::Var) {
      const VarDecl *Var = cast<VarDecl>(DeclRef->getDecl());
      if (Var->hasLocalStorage()) {
        DeclRefExprList.push_back(DeclRef);
        ContainDeclRefExpr = true;
      }
    }
    break;
  }
  case Stmt::BinaryOperatorClass: {
    const BinaryOperator *BO = cast<BinaryOperator>(S);
    // if (BO->getOpcode() == BinaryOperatorKind::BO_NE) {
    //   IsNot = !IsNot;
    // }
    findDeclRefExpr(BO->getLHS()->IgnoreParenImpCasts());
    findDeclRefExpr(BO->getRHS()->IgnoreParenImpCasts());
    break;
  }
  case Stmt::UnaryOperatorClass: {
    const UnaryOperator *UO = cast<UnaryOperator>(S);
    // if (UO->getOpcode() == UnaryOperatorKind::UO_LNot) {
    //   IsNot = !IsNot;
    // }
    findDeclRefExpr(UO->getSubExpr()->IgnoreParenImpCasts());
    break;
  }
  }
}

void BaseCond::findCallExpr(const Stmt *S) {
  switch (S->getStmtClass()) {
  default:
    break;
  case Stmt::CXXMemberCallExprClass: {
    // const CallExpr *Call = cast<CallExpr>(S);
    // CallExprList.push_back(Call);
    // ContainCallExpr = true;
    // break;
  }
    LLVM_FALLTHROUGH;
  case Stmt::CallExprClass: {
    const CallExpr *Call = cast<CallExpr>(S);
    CallExprList.push_back(Call);
    ContainCallExpr = true;
    break;
  }
  case Stmt::BinaryOperatorClass: {
    const BinaryOperator *BO = cast<BinaryOperator>(S);
    findCallExpr(BO->getLHS()->IgnoreParenImpCasts());
    findCallExpr(BO->getRHS()->IgnoreParenImpCasts());
    break;
  }
  case Stmt::UnaryOperatorClass: {
    const UnaryOperator *UO = cast<UnaryOperator>(S);
    findCallExpr(UO->getSubExpr()->IgnoreParenImpCasts());
    break;
  }
  }
}

void BaseCond::setCondStr(const ASTContext &Context) {
  llvm::raw_string_ostream OS(CondStr);
  if (Cond->getStmtClass() == Stmt::BinaryOperatorClass &&
      cast<BinaryOperator>(Cond)->getOpcode() == BinaryOperatorKind::BO_NE) {
    IsNot = true;
    Expr *LHS = cast<BinaryOperator>(Cond)->getLHS()->IgnoreParenImpCasts();
    Expr *RHS = cast<BinaryOperator>(Cond)->getRHS()->IgnoreParenImpCasts();
    LHS->printPretty(OS, nullptr, Context.getPrintingPolicy());
    OS << " == ";
    RHS->printPretty(OS, nullptr, Context.getPrintingPolicy());
  } else if (Cond->getStmtClass() == Stmt::UnaryOperatorClass &&
             cast<UnaryOperator>(Cond)->getOpcode() ==
                 UnaryOperatorKind::UO_LNot) {
    IsNot = true;
    Expr *SubExpr =
        cast<UnaryOperator>(Cond)->getSubExpr()->IgnoreParenImpCasts();
    SubExpr->printPretty(OS, nullptr, Context.getPrintingPolicy());
  } else {
    Cond->printPretty(OS, nullptr, Context.getPrintingPolicy());
  }
  rtrim(CondStr);
}

void IfCond::dump(const ASTContext &Context) { Cond->dumpPretty(Context); }

// void IfCond::setCondStr(const ASTContext &Context) {
//   llvm::raw_string_ostream OS(CondStr);
//   if (Cond->getStmtClass() == Stmt::BinaryOperatorClass &&
//       cast<BinaryOperator>(Cond)->getOpcode() == BinaryOperatorKind::BO_NE) {
//     // IsNot = true;
//     Expr *LHS = cast<BinaryOperator>(Cond)->getLHS()->IgnoreParenImpCasts();
//     Expr *RHS = cast<BinaryOperator>(Cond)->getRHS()->IgnoreParenImpCasts();
//     LHS->printPretty(OS, nullptr, Context.getPrintingPolicy());
//     OS << " == ";
//     RHS->printPretty(OS, nullptr, Context.getPrintingPolicy());
//   } else if (Cond->getStmtClass() == Stmt::UnaryOperatorClass &&
//              cast<UnaryOperator>(Cond)->getOpcode() ==
//                  UnaryOperatorKind::UO_LNot) {
//     // IsNot = true;
//     Expr *SubExpr =
//         cast<UnaryOperator>(Cond)->getSubExpr()->IgnoreParenImpCasts();
//     SubExpr->printPretty(OS, nullptr, Context.getPrintingPolicy());
//   } else {
//     Cond->printPretty(OS, nullptr, Context.getPrintingPolicy());
//   }
//   rtrim(CondStr);
// }

std::string IfCond::toString(const ASTContext &Context) {
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  Cond->printPretty(OS, nullptr, Context.getPrintingPolicy());
  rtrim(Str);
  return Str;
}

void LoopCond::dump(const ASTContext &Context) { Cond->dumpPretty(Context); }

std::string LoopCond::toString(const ASTContext &Context) {
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  Cond->printPretty(OS, nullptr, Context.getPrintingPolicy());
  return OS.str();
}

void CaseCond::dump(const ASTContext &Context) {
  Cond->dumpPretty(Context);
  outs() << ": ";
  Case->dumpPretty(Context);
}

std::string CaseCond::toString(const ASTContext &Context) {
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  Cond->printPretty(OS, nullptr, Context.getPrintingPolicy());
  OS << ": ";
  Case->printPretty(OS, nullptr, Context.getPrintingPolicy());
  return OS.str();
}

void DefaultCond::dump(const ASTContext &Context) {
  Cond->dumpPretty(Context);
  outs() << ": ";
  for (auto *Case : Cases) {
    Case->dumpPretty(Context);
    outs() << " ";
  }
}

std::string DefaultCond::toString(const ASTContext &Context) {
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  Cond->printPretty(OS, nullptr, Context.getPrintingPolicy());
  OS << ": ";
  for (auto *Case : Cases) {
    Case->printPretty(OS, nullptr, Context.getPrintingPolicy());
    OS << " ";
  }
  return OS.str();
}

} // namespace BrInfo

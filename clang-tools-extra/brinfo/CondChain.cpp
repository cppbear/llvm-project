#include "CondChain.h"
#include "OrderedSet.h"

namespace BrInfo {

OrderedSet<const CallExpr *> getCallExpr(const Stmt *S) {
  OrderedSet<const CallExpr *> Set;
  switch (S->getStmtClass()) {
  default:
    break;
  case Stmt::CXXMemberCallExprClass: {
  }
    LLVM_FALLTHROUGH;
  case Stmt::CallExprClass: {
    if (!Set.contains(cast<CallExpr>(S))) {
      Set.insert(cast<CallExpr>(S));
    }
    break;
  }
  case Stmt::BinaryOperatorClass: {
    const BinaryOperator *BO = cast<BinaryOperator>(S);
    Set.merge(getCallExpr(BO->getLHS()->IgnoreParenImpCasts()));
    Set.merge(getCallExpr(BO->getRHS()->IgnoreParenImpCasts()));
    break;
  }
  case Stmt::UnaryOperatorClass: {
    const UnaryOperator *UO = cast<UnaryOperator>(S);
    Set.merge(getCallExpr(UO->getSubExpr()->IgnoreParenImpCasts()));
    break;
  }
  }
  return Set;
}

bool checkLiteralExpr(const Expr *Expr, bool IsNot, bool Flag) {
  // Expr->dumpColor();
  bool Res = true;
  switch (Expr->getStmtClass()) {
  default:
    break;
  case Stmt::CXXBoolLiteralExprClass: {
    const CXXBoolLiteralExpr *BLE = cast<CXXBoolLiteralExpr>(Expr);
    if (IsNot) {
      if (Flag == BLE->getValue()) {
        Res = false;
      }
    } else {
      if (Flag != BLE->getValue()) {
        Res = false;
      }
    }
    break;
  }
    // TODO: Handle other literal expressions, like Integer, Floating, NullPtr
  }
  return Res;
}

void simplify(CondSimp &CondSimp, const BinaryOperator *BO, bool Flag);

// derive the condition from the parent to the child
void deriveCond(CondSimp &CondSimp, bool Flag, BinaryOperator::Opcode Opcode,
                const Expr *Known, const Expr *Unknown) {
  bool Val = CondSimp.Map[Known];
  switch (Opcode) {
  case BinaryOperatorKind::BO_LAnd:
    if (Val) {
      CondSimp.Map[Unknown] = Flag;
      if (Unknown->getStmtClass() == Stmt::BinaryOperatorClass &&
          cast<BinaryOperator>(Unknown)->isLogicalOp()) {
        simplify(CondSimp, cast<BinaryOperator>(Unknown), Flag);
      } else {
        const Stmt *S = static_cast<const Stmt *>(Unknown);
        BaseCond *Cond = new IfCond(S);
        CondSimp.Cond = Cond;
        CondSimp.Flag = Flag;
      }
    } else if (Flag) {
      errs() << "Contradictory conditions\n";
    }
    break;
  case BinaryOperatorKind::BO_LOr:
    if (!Val) {
      CondSimp.Map[Unknown] = Flag;
      if (Unknown->getStmtClass() == Stmt::BinaryOperatorClass &&
          cast<BinaryOperator>(Unknown)->isLogicalOp()) {
        simplify(CondSimp, cast<BinaryOperator>(Unknown), Flag);
      } else {
        const Stmt *S = static_cast<const Stmt *>(Unknown);
        BaseCond *Cond = new IfCond(S);
        CondSimp.Cond = Cond;
        CondSimp.Flag = Flag;
      }
    } else if (!Flag) {
      errs() << "Contradictory conditions\n";
    }
    break;
  default:
    break;
  }
}

// transfer the condition from the children to the parent
bool transferCond(CondSimp &CondSimp, const Expr *Parent) {
  bool Res = false;
  if (Parent->getStmtClass() == Stmt::BinaryOperatorClass) {
    const BinaryOperator *BO = cast<BinaryOperator>(Parent);
    if (BO->isLogicalOp()) {
      Expr *LHS = BO->getLHS()->IgnoreParenImpCasts();
      Expr *RHS = BO->getRHS()->IgnoreParenImpCasts();
      if (CondSimp.Map.find(LHS) == CondSimp.Map.end()) {
        transferCond(CondSimp, LHS);
      }
      if (CondSimp.Map.find(RHS) == CondSimp.Map.end()) {
        transferCond(CondSimp, RHS);
      }
      bool LHSKnown = CondSimp.Map.find(LHS) != CondSimp.Map.end();
      bool RHSKnown = CondSimp.Map.find(RHS) != CondSimp.Map.end();
      switch (BO->getOpcode()) {
      case BinaryOperatorKind::BO_LAnd:
        if (LHSKnown && RHSKnown) {
          CondSimp.Map[Parent] = CondSimp.Map[LHS] && CondSimp.Map[RHS];
          Res = true;
        } else if ((LHSKnown && !CondSimp.Map[LHS]) ||
                   (RHSKnown && !CondSimp.Map[RHS])) {
          CondSimp.Map[Parent] = false;
          Res = true;
        }
        break;
      case BinaryOperatorKind::BO_LOr:
        if (LHSKnown && RHSKnown) {
          CondSimp.Map[Parent] = CondSimp.Map[LHS] || CondSimp.Map[RHS];
          Res = true;
        } else if ((LHSKnown && CondSimp.Map[LHS]) ||
                   (RHSKnown && CondSimp.Map[RHS])) {
          CondSimp.Map[Parent] = true;
          Res = true;
        }
        break;
      default:
        break;
      }
    }
  }
  return Res;
}

void simplify(CondSimp &CondSimp, const BinaryOperator *BO, bool Flag) {
  Expr *LHS = BO->getLHS()->IgnoreParenImpCasts();
  Expr *RHS = BO->getRHS()->IgnoreParenImpCasts();
  bool LHSKnown = CondSimp.Map.find(LHS) != CondSimp.Map.end();
  bool RHSKnown = CondSimp.Map.find(RHS) != CondSimp.Map.end();
  if (LHSKnown && !RHSKnown) {
    deriveCond(CondSimp, Flag, BO->getOpcode(), LHS, RHS);
  } else if (!LHSKnown && RHSKnown) {
    deriveCond(CondSimp, Flag, BO->getOpcode(), RHS, LHS);
  } else if (!LHSKnown && !RHSKnown) {
    bool Res = transferCond(CondSimp, LHS) || transferCond(CondSimp, RHS);
    if (Res) {
      simplify(CondSimp, BO, Flag);
    } else {
      errs() << "Unhandle condition\n";
    }
  }
}

StringList CondStatus::getLastDefStrVec(ASTContext *Context) {
  StringList StrVec;
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  if (!LastDefStmts.empty()) {
    for (const Stmt *S : LastDefStmts) {
      S->printPretty(OS, nullptr, Context->getPrintingPolicy());
      OS.flush();
      rtrim(Str);
      StrVec.push_back(Str);
      Str.clear();
    }
  }
  return StrVec;
}

void CondStatus::dump(ASTContext *Context) {
  if (Condition) {
    Condition->dump(Context);
    errs() << " is " << (Flag ? "true" : "false");
    if (!LastDefStmts.empty() || !ParmVars.empty()) {
      std::string Str;
      raw_string_ostream OS(Str);
      errs() << ", where: ";
      unsigned I = 0;
      for (const Stmt *S : LastDefStmts) {
        S->printPretty(OS, nullptr, Context->getPrintingPolicy());
        OS.flush();
        rtrim(Str);
        if (I++ > 0)
          errs() << ", ";
        errs() << Str;
        Str.clear();
      }
      I = 0;
      for (const ParmVarDecl *PVD : ParmVars) {
        if (I++ > 0)
          errs() << ", ";
        errs() << PVD->getNameAsString() << " is ParmVar";
      }
    }
  }
}

void CondChainInfo::analyze(ASTContext *Context) {
  simplifyConds();
  setCondStr(Context);
  findCallExprs();
  traceBack();
  findContra();
}

void CondChainInfo::simplifyConds() {
  CondSimp CondSimp;
  for (CondStatus &Cond : Chain) {
    if (Cond.Condition) {
      CondSimp.Cond = nullptr;
      const Stmt *S = Cond.Condition->getCond();
      CondSimp.Map[S] = Cond.Flag;

      if (S->getStmtClass() == Stmt::BinaryOperatorClass) {
        const BinaryOperator *BO = cast<BinaryOperator>(S);
        if (BO->isLogicalOp()) {
          simplify(CondSimp, BO, Cond.Flag);
        }
      }
      if (CondSimp.Cond) {
        Cond.Condition = CondSimp.Cond;
        Cond.Flag = CondSimp.Flag;
      }
    }
  }
}

void CondChainInfo::setCondStr(ASTContext *Context) {
  for (CondStatus &Cond : Chain) {
    if (Cond.Condition) {
      Cond.Condition->setCondStr(Context);
    }
  }
}

void CondChainInfo::findCallExprs() {
  OrderedSet<const CallExpr *> Set;
  for (CFGBlock *Blk : Path) {
    for (const CFGElement &E : *Blk) {
      if (std::optional<CFGStmt> S = E.getAs<CFGStmt>()) {
        const Stmt *Stmt = S->getStmt();
        Set.merge(getCallExpr(Stmt));
      }
    }
  }
  if (!Set.empty()) {
    for (const CallExpr *CE : Set.orderedElements()) {
      const FunctionDecl *FD = CE->getDirectCallee();
      if (FD) {
        FuncCallInfo[FD].push_back({CE, {}});
      }
    }
  }
}

void CondChainInfo::traceBack() {
  unsigned CondNum = Chain.size();
  for (unsigned J = 0; J < CondNum; ++J) {
    CondStatus &Cond = Chain[J];
    if (Cond.Condition) {
      if (Cond.Condition->containDeclRefExpr()) {
        for (const DeclRefExpr *DRE : Cond.Condition->getDeclRefExprList()) {
          const Stmt *LastDefStmt = findLastDefStmt(DRE, J);
          if (LastDefStmt) {
            // outs() << "Condition:\n";
            // Cond.Condition->getCond()->dumpColor();
            // outs() << "Value: " << (Cond.Flag ? "True" : "False") << "\n";
            // outs() << "DeclRef:\n";
            // DRE->dumpColor();
            // outs() << "TraceBack:\n";
            // LastDefStmt->dumpColor();
            bool Exam = examineLastDef(DRE, LastDefStmt,
                                       Cond.Condition->isNot(), Cond.Flag);
            // outs() << "Exam: " << Exam << "\n";
            if (Exam)
              Cond.LastDefStmts.insert(LastDefStmt);
            else {
              errs() << "Contradictory CondChain in traceBack()\n";
              // dumpCondChain(I);
              IsContra = true;
            };
          } else if (DRE->getDecl()->getKind() == Decl::Kind::ParmVar) {
            // handle ParmVar
            Cond.ParmVars.insert(cast<ParmVarDecl>(DRE->getDecl()));
          }
        }
      }
    }
  }
}

void CondChainInfo::findContra() {
  unsigned CondNum = Chain.size();
  for (unsigned J = 0; J < CondNum; ++J) {
    CondStatus &Cond = Chain[J];
    if (Cond.Condition) {
      // handle call expression in the condition
      if (Cond.Condition->containCallExpr()) {
        for (const CallExpr *CE : Cond.Condition->getCallExprList()) {
          IsContra = !setFuncCallInfo(Cond, CE);
          if (IsContra)
            return;
        }
      }
      // handle last definition and call expression in the trace back
      for (const Stmt *S : Cond.LastDefStmts) {
        switch (S->getStmtClass()) {
        default:
          break;
        case Stmt::DeclStmtClass: {
          //   S->dumpColor();
          const DeclStmt *DS = cast<DeclStmt>(S);
          for (const Decl *D : DS->decls()) {
            if (const VarDecl *VD = dyn_cast<VarDecl>(D)) {
              const Expr *Init = VD->getInit()->IgnoreParenImpCasts();
              if (Init) {
                if (Init->getStmtClass() == Stmt::CXXMemberCallExprClass ||
                    Init->getStmtClass() == Stmt::CallExprClass) {
                  IsContra = !setFuncCallInfo(Cond, cast<CallExpr>(Init));
                  if (IsContra)
                    return;
                } else {
                  IsContra = !setDefInfo(Cond, Init);
                  if (IsContra)
                    return;
                }
              }
            }
          }
          break;
        }
        case Stmt::BinaryOperatorClass: {
          const BinaryOperator *BO = cast<BinaryOperator>(S);
          if (BO->isAssignmentOp()) {
            Expr *RHS = BO->getRHS()->IgnoreParenImpCasts();
            if (RHS->getStmtClass() == Stmt::CXXMemberCallExprClass ||
                RHS->getStmtClass() == Stmt::CallExprClass) {
              IsContra = !setFuncCallInfo(Cond, cast<CallExpr>(RHS));
              if (IsContra)
                return;
            } else {
              IsContra = !setDefInfo(Cond, RHS);
              if (IsContra)
                return;
            }
          }
          break;
        }
        }
      }
      // handle ParmVars
      for (const ParmVarDecl *PVD : Cond.ParmVars) {
        IsContra = !setParmInfo(Cond, PVD);
        if (IsContra)
          return;
      }
    }
  }
}

bool CondChainInfo::setFuncCallInfo(CondStatus &Cond, const CallExpr *CE) {
  const FunctionDecl *FuncDecl = CE->getDirectCallee();
  bool Flag = Cond.Flag;
  if (Cond.Condition->isNot())
    Flag = !Flag;
  if (FuncCallInfo.find(FuncDecl) == FuncCallInfo.end()) {
    errs() << "Function " << FuncDecl->getNameAsString()
           << " is not in FuncCallInfo\n";
  }
  std::vector<CallExprInfo> &CallExprInfoList = FuncCallInfo[FuncDecl];
  for (CallExprInfo &CallExprInfo : CallExprInfoList) {
    if (CallExprInfo.CE == CE) {
      if (CallExprInfo.CondInfos.find(Cond.Condition->getCondStr()) ==
          CallExprInfo.CondInfos.end()) {
        CallExprInfo.CondInfos[Cond.Condition->getCondStr()] = Flag;
      } else if (CallExprInfo.CondInfos[Cond.Condition->getCondStr()] != Flag) {
        errs() << "Contradictory CondChain in setFuncCallInfo()\n";
        return false;
      }
      break;
    }
  }
  return true;
}

bool CondChainInfo::setDefInfo(CondStatus &Cond, const Stmt *S) {
  LastDefInfo::DefInfoMap &DefInfoMap = LastDefInfo.DefInfo;
  bool Flag = Cond.Flag;
  if (Cond.Condition->isNot())
    Flag = !Flag;
  if (DefInfoMap.find(S) == DefInfoMap.end() ||
      DefInfoMap[S].find(Cond.Condition->getCondStr()) == DefInfoMap[S].end()) {
    DefInfoMap[S][Cond.Condition->getCondStr()] = Flag;
  } else if (DefInfoMap[S][Cond.Condition->getCondStr()] != Flag) {
    errs() << "Contradictory CondChain in setDefInfo()\n";
    return false;
  }
  return true;
}

bool CondChainInfo::setParmInfo(CondStatus &Cond, const ParmVarDecl *PVD) {
  LastDefInfo::ParmInfoMap &ParmInfoMap = LastDefInfo.ParmInfo;
  bool Flag = Cond.Flag;
  if (Cond.Condition->isNot())
    Flag = !Flag;
  if (ParmInfoMap.find(PVD) == ParmInfoMap.end() ||
      ParmInfoMap[PVD].find(Cond.Condition->getCondStr()) ==
          ParmInfoMap[PVD].end()) {
    ParmInfoMap[PVD][Cond.Condition->getCondStr()] = Flag;
  } else if (ParmInfoMap[PVD][Cond.Condition->getCondStr()] != Flag) {
    errs() << "Contradictory CondChain in setParmVarInfo()\n";
    return false;
  }
  return true;
}

const Stmt *CondChainInfo::findLastDefStmt(const DeclRefExpr *DeclRef,
                                           unsigned Loc) {
  // outs() << "DeclRefExpr: ";
  // DeclRef->dumpPretty(Context);
  // outs() << "\n";
  bool Found = false;
  const Stmt *TraceBack = nullptr;
  for (auto It = Path.rend() - Loc; It != Path.rend() && !Found; ++It) {
    CFGBlock *Blk = *It;
    CFGElement *E = Blk->rbegin();
    if (Blk->getTerminator().isValid()) {
      E = E + 1;
    }
    for (; E != Blk->rend() && !Found; ++E) {
      if (std::optional<CFGStmt> S = E->getAs<CFGStmt>()) {
        const Stmt *Stmt = S->getStmt();
        // Stmt->dumpColor();
        switch (Stmt->getStmtClass()) {
        default:
          // errs() << "handleDeclRefExpr() unhandle: " <<
          // Stmt->getStmtClassName()
          //        << "\n";
          // Stmt->dumpColor();
          break;
        // case Stmt::CallExprClass: {
        //   errs() << "handleDeclRefExpr() unhandle: CallExpr\n";
        //   break;
        // }
        // case Stmt::CXXMemberCallExprClass: {
        //   errs() << "handleDeclRefExpr() unhandle: CXXMemberCallExpr\n";
        //   break;
        // }
        case Stmt::DeclStmtClass: {
          const DeclStmt *DS = cast<DeclStmt>(Stmt);
          for (const Decl *D : DS->decls()) {
            if (const VarDecl *VD = dyn_cast<VarDecl>(D)) {
              if (VD == DeclRef->getDecl()) {
                // outs() << "VarDecl:\n";
                // DS->dumpPretty(Context);
                Found = true;
                TraceBack = DS;
                break;
              }
            }
          }
          break;
        }
        case Stmt::BinaryOperatorClass: {
          const BinaryOperator *BO = cast<BinaryOperator>(Stmt);
          if (BO->isAssignmentOp()) {
            Expr *LHS = BO->getLHS()->IgnoreParenImpCasts();
            if (LHS->getStmtClass() == Stmt::DeclRefExprClass) {
              DeclRefExpr *DRE = cast<DeclRefExpr>(LHS);
              if (DRE->getDecl() == DeclRef->getDecl()) {
                // outs() << "Assignment:\n";
                // BO->dumpPretty(Context);
                // outs() << "\n";
                Found = true;
                TraceBack = BO;
              }
            }
          }
          break;
        }
        }
      }
    }
  }
  return TraceBack;
}

bool CondChainInfo::examineLastDef(const DeclRefExpr *DeclRef,
                                   const Stmt *LastDefStmt, bool IsNot,
                                   bool Flag) {
  bool Res = true;
  switch (LastDefStmt->getStmtClass()) {
  default:
    break;
  case Stmt::DeclStmtClass: {
    const DeclStmt *DS = cast<DeclStmt>(LastDefStmt);
    for (const Decl *D : DS->decls()) {
      if (const VarDecl *VD = dyn_cast<VarDecl>(D)) {
        if (VD == DeclRef->getDecl()) {
          Res = checkLiteralExpr(VD->getInit()->IgnoreParenImpCasts(), IsNot,
                                 Flag);
          break;
        }
      }
    }
    break;
  }
  case Stmt::BinaryOperatorClass: {
    const BinaryOperator *BO = cast<BinaryOperator>(LastDefStmt);
    Res = checkLiteralExpr(BO->getRHS()->IgnoreParenImpCasts(), IsNot, Flag);
    break;
  }
  }
  return Res;
}

void CondChainInfo::dumpFuncCallInfo() {
  for (auto &It : FuncCallInfo) {
    outs() << "Function: " << It.first->getNameAsString() << "\n";
    for (CallExprInfo &CEI : It.second) {
      outs() << "CallExpr: ";
      CEI.CE->dumpColor();
      outs() << "\n";
      for (auto &It : CEI.CondInfos) {
        outs() << "Condition: " << It.first << " Value: ";
        outs() << (It.second ? "True" : "False") << "\n";
      }
    }
  }
}

void CondChainInfo::dump(ASTContext *Context, unsigned Indent) {
  std::string IndentStr(Indent, ' ');
  errs() << IndentStr;
  unsigned CondNum = Chain.size();
  unsigned I = 0;
  for (unsigned J = 0; J < CondNum; ++J) {
    if (Chain[J].Condition) {
      if (I++ > 0)
        errs() << " \033[36m\033[1m->\033[0m ";
      Chain[J].dump(Context);
    }
  }
  errs() << "\n" + IndentStr;
  I = 0;
  for (CFGBlock *Blk : Path) {
    if (I++ > 0)
      errs() << " \033[36m\033[1m->\033[0m ";
    errs() << Blk->getBlockID();
  }
  errs() << "\n";
}

} // namespace BrInfo
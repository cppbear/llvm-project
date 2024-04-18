#include "Analysis.h"
#include "clang/Analysis/CFG.h"
#include <cassert>

namespace BrInfo {

void Analysis::clear() {
  Cfg = nullptr;
  Context = nullptr;
  FuncDecl = nullptr;
  Signature.clear();
  BlkChain.clear();
  LastDefList.clear();
  CallExprList.clear();
  ContraChains.clear();
  Parent = -1;
  CondMap.clear();
  TmpCond = {nullptr, false};
}

void Analysis::setType(Type T) {
  AnalysisType = T;
  switch (T) {
  case FILE:
    Results["type"] = "file";
    break;
  case FUNC:
    Results["type"] = "function";
    break;
  }
}

void Analysis::init(CFG *CFG, ASTContext *Context, const FunctionDecl *FD) {
  Cfg = CFG;
  this->Context = Context;
  FuncDecl = FD;
  setSignature();
  BlkChain.resize(Cfg->getNumBlockIDs());
  Parent = -1;
  BlkChain[Cfg->getEntry().getBlockID()].push_back(
      {{{nullptr, false, {}, {}}}, {&Cfg->getEntry()}});
}

void Analysis::analyze() {
  getCondChains();
  simplifyConds();
  traceBack();
  findContraInLastDef();
  setRequire();
  clear();
}

void Analysis::dfs(CFGBlock *Blk, BaseCond *Condition, bool Flag) {
  unsigned ID = Blk->getBlockID();

  // outs() << "Node: " << ID << " Parent: " << Parent << "\n";
  // if (Condition) {
  //   outs() << "    Cond: ";
  //   Condition->dumpPretty(Context);
  //   outs() << ": " << (Flag ? "True" : "False") << "\n";
  // }

  if (Parent != -1 && !BlkChain[Parent].empty()) {
    std::pair<CondChain, Path> Chain = BlkChain[Parent].back();
    auto CondChain = Chain.first;
    CondChain.push_back({Condition, Flag, {}, {}});
    auto Path = Chain.second;
    Path.push_back(Blk);
    BlkChain[ID].push_back({CondChain, Path});
  }

  Parent = ID;
  Stmt *Terminator = Blk->getTerminatorStmt();
  if (Terminator) {
    // FIXME: Handle Loop and Try-catch
    switch (Terminator->getStmtClass()) {
    default:
      errs() << "Terminator: " << Terminator->getStmtClassName() << "\n";
      break;
    case Stmt::BinaryOperatorClass:
      LLVM_FALLTHROUGH;
    case Stmt::IfStmtClass: {
      BaseCond *Cond = nullptr;
      Stmt *InnerCond = Blk->getTerminatorCondition();
      if (InnerCond) {
        Cond =
            new IfCond(cast<Expr>(InnerCond)->IgnoreParenImpCasts(), Context);
      }
      auto *I = Blk->succ_begin();
      if (Cond) {
        dfs(*I, Cond, true);
        Parent = ID;
        ++I;
        dfs(*I, Cond, false);
        Parent = ID;
      } else {
        dfs(*I, nullptr, false);
        Parent = ID;
      }
      break;
    }
    case Stmt::SwitchStmtClass: {
      CFGBlock *DefaultBlk = nullptr;
      Stmt *InnerCond = Blk->getTerminatorCondition();
      std::vector<Stmt *> Cases;
      for (auto I : Blk->succs()) {
        Stmt *Label = I->getLabel();
        assert(Label);
        BaseCond *Cond = nullptr;
        if (Label->getStmtClass() == Stmt::CaseStmtClass) {
          Stmt *Case = cast<CaseStmt>(Label)->getLHS();
          Cases.push_back(Case);
          if (InnerCond) {
            Cond = new CaseCond(cast<Expr>(InnerCond)->IgnoreParenImpCasts(),
                                Case, Context);
          }
          dfs(I, Cond, true);
          Parent = ID;
        } else {
          // Default case
          assert(Label->getStmtClass() == Stmt::DefaultStmtClass);
          DefaultBlk = I;
        }
      }
      if (DefaultBlk) {
        BaseCond *Cond = nullptr;
        if (InnerCond) {
          Cond = new DefaultCond(InnerCond, Cases, Context);
        }
        dfs(DefaultBlk, Cond, false);
        Parent = ID;
      }
      break;
    }
    case Stmt::BreakStmtClass:
      dfs(*Blk->succ_begin(), nullptr, false);
      Parent = ID;
      break;
    case Stmt::ForStmtClass: {
    }
      LLVM_FALLTHROUGH;
    case Stmt::WhileStmtClass: {
    }
      LLVM_FALLTHROUGH;
    case Stmt::DoStmtClass: {
      BaseCond *Cond = nullptr;
      Stmt *InnerCond = Blk->getTerminatorCondition();
      if (InnerCond) {
        Cond =
            new LoopCond(cast<Expr>(InnerCond)->IgnoreParenImpCasts(), Context);
      }
      auto *I = Blk->succ_begin();
      if (InnerCond) {
        dfs(*I, Cond, true);
        Parent = ID;
        ++I;
        dfs(*I, Cond, false);
        Parent = ID;
      } else {
        dfs(*I, nullptr, false);
        Parent = ID;
      }
      break;
    }
    }
  } else if (Blk->succ_size() == 1 && !Blk->getLoopTarget()) {
    dfs(*Blk->succ_begin(), nullptr, false);
    Parent = ID;
  } else if (Blk->getBlockID() != Cfg->getExit().getBlockID()) {
    // TODO: Handle back edges in loops
    outs() << "Unhandle Block:\n";
    Blk->dump();
  }
}

void Analysis::getCondChains() {
  // outs() << "============================\n";
  dfs(&Cfg->getEntry(), nullptr, false);
}

void Analysis::dumpBlkChain() {
  for (unsigned I = 0; I < BlkChain.size(); ++I) {
    dumpBlkChain(I);
  }
}

void Analysis::dumpBlkChain(unsigned ID) {
  outs() << "Block: " << ID << "\n";
  for (auto &Chain : BlkChain[ID]) {
    auto CondChain = Chain.first;
    auto Path = Chain.second;
    for (auto &Cond : CondChain) {
      if (Cond.Condition) {
        Cond.Condition->dump(Context);
        outs() << ":=" << (Cond.Flag ? "True" : "False") << " -> ";
      }
    }
    outs() << "\n";
    for (CFGBlock *Blk : Path) {
      outs() << Blk->getBlockID() << " ";
    }
    outs() << "\n";
  }
}

void Analysis::dumpTraceBack(CondStatus &Cond) {
  if (!Cond.LastDefStmts.empty() || !Cond.ParmVars.empty()) {
    errs() << "where: ";
    for (const Stmt *S : Cond.LastDefStmts) {
      S->dumpPretty(*Context);
    }
    for (const ParmVarDecl *PVD : Cond.ParmVars) {
      errs() << PVD->getNameAsString() << " is ParmVar, ";
    }
  }
}

std::vector<std::string>
Analysis::getLastDefStrVec(std::set<const Stmt *> &TraceBacks) {
  std::vector<std::string> StrVec;
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  if (!TraceBacks.empty()) {
    for (const Stmt *S : TraceBacks) {
      S->printPretty(OS, nullptr, Context->getPrintingPolicy());
      OS.flush();
      rtrim(Str);
      StrVec.push_back(Str);
      Str.clear();
    }
  }
  return StrVec;
}

// void Analysis::dumpCondChains() {
//   unsigned ID = Cfg->getExit().getBlockID();
//   auto CondChains = BlkChain[ID];
//   unsigned Size = CondChains.size();
//   for (unsigned I = 0; I < Size; ++I) {
//     auto CondChain = CondChains[I].first;
//     auto Path = CondChains[I].second;
//     outs() << "CondChain " << I << ":\n  ";
//     unsigned CondNum = CondChain.size();
//     for (unsigned J = 0; J < CondNum; ++J) {
//       CondStatus &Cond = CondChain[J];
//       if (Cond.Condition) {
//         Cond.Condition->dump(Context);
//         outs() << ": " << (Cond.Flag ? "True" : "False") << " ";
//         dumpTraceBack(I, J);
//         outs() << " -> ";
//       }
//     }
//     outs() << "\n  ";
//     for (CFGBlock *Blk : Path) {
//       outs() << Blk->getBlockID() << " ";
//     }
//     outs() << "\n";
//     // break;
//   }
// }

void Analysis::dumpCondChain(unsigned ID) {
  unsigned ExitID = Cfg->getExit().getBlockID();
  auto CondChains = BlkChain[ExitID];

  auto CondChain = CondChains[ID].first;
  auto Path = CondChains[ID].second;
  outs() << "CondChain " << ID << ":\n  ";
  unsigned CondNum = CondChain.size();
  for (unsigned J = 0; J < CondNum; ++J) {
    CondStatus &Cond = CondChain[J];
    if (Cond.Condition) {
      Cond.Condition->dump(Context);
      outs() << ": " << (Cond.Flag ? "True" : "False") << " ";
      dumpTraceBack(Cond);
      outs() << " -> ";
    }
  }
  outs() << "\n  ";
  for (CFGBlock *Blk : Path) {
    outs() << Blk->getBlockID() << " ";
  }
  outs() << "\n";
}

void Analysis::setSignature() {
  Signature = FuncDecl->getReturnType().getAsString() + " ";
  if (FuncDecl->isCXXClassMember()) {
    Signature +=
        cast<CXXRecordDecl>(FuncDecl->getParent())->getNameAsString() + "::";
  }
  Signature += FuncDecl->getNameAsString() + "(";
  int I = 0;
  for (ParmVarDecl *Param : FuncDecl->parameters()) {
    if (I++ > 0) {
      Signature += ", ";
    }
    Signature +=
        Param->getType().getAsString() + " " + Param->getNameAsString();
  }
  Signature += ")";
  // outs() << "Signature: " << Signature << "\n";
}

void Analysis::dumpResults(std::string ProjectPath, std::string FileName,
                           std::string ClassName, std::string FuncName) {
  std::string FilePath = ProjectPath;
  if (AnalysisType == Type::FILE) {
    FilePath += "/" + FileName + "_req.json";
  } else {
    FilePath += "/" + ClassName + "_" + FuncName + "_req.json";
  }
  std::error_code EC;
  llvm::raw_fd_stream File(FilePath, EC);
  if (EC) {
    errs() << "Error: " << EC.message() << "\n";
    return;
  }
  File << Results.dump(4);
}

std::string formatID(std::string ID) {
  std::string Str;
  size_t Size = ID.size();
  if (Size < 3) {
    Str = std::string(3 - Size, '0') + ID;
  } else {
    Str = ID;
  }
  return Str;
}

void Analysis::setRequire() {
  json Json;

  unsigned ExitID = Cfg->getExit().getBlockID();
  CondChains &CondChains = BlkChain[ExitID];
  unsigned Size = CondChains.size();
  std::string Require;
  llvm::raw_string_ostream OS(Require);
  std::vector<std::string> CondVec;
  std::set<CondStatus> CondStatusSet;
  for (unsigned ID = 0; ID < Size; ++ID) {
    if (ContraChains.find(ID) == ContraChains.end()) {
      std::string CondChainStr = formatID(std::to_string(ID));
      std::string Input = "";
      int I = 0;
      for (ParmVarDecl *Param : FuncDecl->parameters()) {
        if (I++ > 0) {
          Input += ", ";
        }
        Input += Param->getNameAsString() + " is a " + Param->getType().getAsString();
      }
      Json[CondChainStr]["input"] = Input;
      auto &CondChain = CondChains[ID].first;
      unsigned CondNum = CondChain.size();
      for (unsigned J = 0; J < CondNum; ++J) {
        CondStatus &Cond = CondChain[J];
        if (Cond.Condition) {
          if (CondStatusSet.find(Cond) == CondStatusSet.end()) {
            CondStatusSet.insert(Cond);
            OS << Cond.Condition->getCondStr() << " is ";
            if (Cond.Condition->isNot())
              OS << (Cond.Flag ? "false" : "true");
            else
              OS << (Cond.Flag ? "true" : "false");
            OS.flush();
            Json[CondChainStr]["precondition"].push_back(
                {{"condition", Require},
                 {"lastdef", getLastDefStrVec(Cond.LastDefStmts)}});
            Require.clear();
          }
        }
      }
      CondStatusSet.clear();
      std::string ClassName = "";
      if (FuncDecl->isCXXClassMember()) {
        ClassName =
            cast<CXXRecordDecl>(FuncDecl->getParent())->getNameAsString();
      }
      Json[CondChainStr]["class"] = ClassName;
      if (!LastDefList[ID].FuncCall.empty()) {
        auto &FuncCallMap = LastDefList[ID].FuncCall;
        for (auto &Func : FuncCallMap) {
          // TODO: In the order of these CallExpr in the source file
          for (auto &CallExpr : Func.second) {
            // CallExpr.first->printPretty(OS, nullptr,
            //                             Context.getPrintingPolicy());
            for (auto &Return : CallExpr.second) {
              OS << Return.first << " is ";
              OS << (Return.second ? "true" : "false");
              OS.flush();
              CondVec.push_back(Require);
              Require.clear();
            }
            Json[CondChainStr]["mock"].push_back(
                {{"function", Func.first}, {"condition", CondVec}});
            Require.clear();
            CondVec.clear();
          }
        }
      }
      std::string Result = "";
      if (FuncDecl->getReturnType() != Context->VoidTy) {
        std::vector<CFGBlock *> &Path = CondChains[ID].second;
        CFGBlock *Blk = Path[Path.size() - 2];
        if (!Blk->hasNoReturnElement()) {
          if (std::optional<CFGStmt> S = Blk->back().getAs<CFGStmt>()) {
            if (S->getStmt()->getStmtClass() == Stmt::ReturnStmtClass) {
              const ReturnStmt *RS = cast<ReturnStmt>(S->getStmt());
              RS->getRetValue()->printPretty(OS, nullptr,
                                             Context->getPrintingPolicy());
              OS.flush();
              Result += "a " + FuncDecl->getReturnType().getAsString() + " value: ";
              Result += Require;
              Require.clear();
            }
          }
        }
      }
      Json[CondChainStr]["result"] = Result;
      Require.clear();
    }
    // break;
  }
  Results[Signature] = Json;
}

void Analysis::simplifyConds() {
  unsigned ID = Cfg->getExit().getBlockID();
  CondChains &CondChains = BlkChain[ID];
  unsigned Size = CondChains.size();
  for (unsigned I = 0; I < Size; ++I) {
    CondMap.clear();
    auto &CondChain = CondChains[I].first;
    for (auto &Cond : CondChain) {
      if (Cond.Condition) {
        TmpCond.first = nullptr;
        const Stmt *S = Cond.Condition->getCond();
        CondMap[S] = Cond.Flag;

        if (S->getStmtClass() == Stmt::BinaryOperatorClass) {
          const BinaryOperator *BO = cast<BinaryOperator>(S);
          if (BO->isLogicalOp()) {
            simplify(BO, Cond.Flag);
          }
        }
        if (TmpCond.first) {
          Cond.Condition = TmpCond.first;
          Cond.Flag = TmpCond.second;
        }
      }
    }
    // break;
  }
}

void Analysis::simplify(const BinaryOperator *BO, bool Flag) {
  Expr *LHS = BO->getLHS()->IgnoreParenImpCasts();
  Expr *RHS = BO->getRHS()->IgnoreParenImpCasts();
  bool LHSKnown = CondMap.find(LHS) != CondMap.end();
  bool RHSKnown = CondMap.find(RHS) != CondMap.end();
  if (LHSKnown && !RHSKnown) {
    deriveCond(Flag, BO->getOpcode(), LHS, RHS);
  } else if (!LHSKnown && RHSKnown) {
    deriveCond(Flag, BO->getOpcode(), RHS, LHS);
  } else if (!LHSKnown && !RHSKnown) {
    bool Res = transferCond(LHS) || transferCond(RHS);
    if (Res) {
      simplify(BO, Flag);
    } else {
      errs() << "Unhandle condition\n";
    }
  }
}

// derive the condition from the parent to the child
void Analysis::deriveCond(bool Flag, BinaryOperator::Opcode Opcode,
                          const Expr *Known, const Expr *Unknown) {
  bool Val = CondMap[Known];
  switch (Opcode) {
  case BinaryOperatorKind::BO_LAnd:
    if (Val) {
      CondMap[Unknown] = Flag;
      if (Unknown->getStmtClass() == Stmt::BinaryOperatorClass &&
          cast<BinaryOperator>(Unknown)->isLogicalOp()) {
        simplify(cast<BinaryOperator>(Unknown), Flag);
      } else {
        const Stmt *S = static_cast<const Stmt *>(Unknown);
        BaseCond *Cond = new IfCond(S, Context);
        TmpCond = {Cond, Flag};
      }
    } else if (Flag) {
      errs() << "Contradictory conditions\n";
    }
    break;
  case BinaryOperatorKind::BO_LOr:
    if (!Val) {
      CondMap[Unknown] = Flag;
      if (Unknown->getStmtClass() == Stmt::BinaryOperatorClass &&
          cast<BinaryOperator>(Unknown)->isLogicalOp()) {
        simplify(cast<BinaryOperator>(Unknown), Flag);
      } else {
        const Stmt *S = static_cast<const Stmt *>(Unknown);
        BaseCond *Cond = new IfCond(S, Context);
        TmpCond = {Cond, Flag};
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
bool Analysis::transferCond(const Expr *Parent) {
  bool Res = false;
  if (Parent->getStmtClass() == Stmt::BinaryOperatorClass) {
    const BinaryOperator *BO = cast<BinaryOperator>(Parent);
    if (BO->isLogicalOp()) {
      Expr *LHS = BO->getLHS()->IgnoreParenImpCasts();
      Expr *RHS = BO->getRHS()->IgnoreParenImpCasts();
      if (CondMap.find(LHS) == CondMap.end()) {
        transferCond(LHS);
      }
      if (CondMap.find(RHS) == CondMap.end()) {
        transferCond(RHS);
      }
      bool LHSKnown = CondMap.find(LHS) != CondMap.end();
      bool RHSKnown = CondMap.find(RHS) != CondMap.end();
      switch (BO->getOpcode()) {
      case BinaryOperatorKind::BO_LAnd:
        if (LHSKnown && RHSKnown) {
          CondMap[Parent] = CondMap[LHS] && CondMap[RHS];
          Res = true;
        } else if ((LHSKnown && !CondMap[LHS]) || (RHSKnown && !CondMap[RHS])) {
          CondMap[Parent] = false;
          Res = true;
        }
        break;
      case BinaryOperatorKind::BO_LOr:
        if (LHSKnown && RHSKnown) {
          CondMap[Parent] = CondMap[LHS] || CondMap[RHS];
          Res = true;
        } else if ((LHSKnown && CondMap[LHS]) || (RHSKnown && CondMap[RHS])) {
          CondMap[Parent] = true;
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

void Analysis::traceBack() {
  unsigned ID = Cfg->getExit().getBlockID();
  CondChains &CondChains = BlkChain[ID];
  unsigned Size = CondChains.size();
  for (unsigned I = 0; I < Size; ++I) {
    auto &CondChain = CondChains[I].first;
    auto &Path = CondChains[I].second;
    // outs() << "CondChain " << I << ":\n";
    unsigned CondNum = CondChain.size();
    for (unsigned J = 0; J < CondNum; ++J) {
      CondStatus &Cond = CondChain[J];
      if (Cond.Condition) {
        if (Cond.Condition->containDeclRefExpr()) {
          for (const DeclRefExpr *DRE : Cond.Condition->getDeclRefExprList()) {
            const Stmt *LastDefStmt = findLastDefStmt(DRE, Path, J);
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
                errs() << "Contradictory CondChain " << I
                       << " in traceBack()\n";
                // dumpCondChain(I);
                ContraChains.insert(I);
              };
            } else if (DRE->getDecl()->getKind() == Decl::Kind::ParmVar) {
              // handle ParmVar
              Cond.ParmVars.insert(cast<ParmVarDecl>(DRE->getDecl()));
            }
          }
        }
      }
    }
    // break;
  }
}

bool Analysis::checkLiteralExpr(const Expr *Expr, bool IsNot, bool Flag) {
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

bool Analysis::examineLastDef(const DeclRefExpr *DeclRef,
                              const Stmt *LastDefStmt, bool IsNot, bool Flag) {
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
        }
      }
    }
    break;
  }
  case Stmt::BinaryOperatorClass: {
    const BinaryOperator *BO = cast<BinaryOperator>(LastDefStmt);
    if (BO->isAssignmentOp()) {
      Expr *LHS = BO->getLHS()->IgnoreParenImpCasts();
      if (LHS->getStmtClass() == Stmt::DeclRefExprClass) {
        DeclRefExpr *DRE = cast<DeclRefExpr>(LHS);
        if (DRE->getDecl() == DeclRef->getDecl()) {
          Res = checkLiteralExpr(BO->getRHS()->IgnoreParenImpCasts(), IsNot,
                                 Flag);
        }
      }
    }
    break;
  }
  }
  return Res;
}

const Stmt *Analysis::findLastDefStmt(const DeclRefExpr *DeclRef, Path &Path,
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

bool Analysis::setNonFuncCallInfo(LastDefInfo &Info, CondStatus &Cond,
                                  const Stmt *S, unsigned int CondChainID) {
  auto &NonFuncCallMap = Info.NonFuncCall;
  bool Flag = Cond.Flag;
  if (Cond.Condition->isNot())
    Flag = !Flag;
  if (NonFuncCallMap.find(S) == NonFuncCallMap.end() ||
      NonFuncCallMap[S].find(Cond.Condition->getCondStr()) ==
          NonFuncCallMap[S].end()) {
    NonFuncCallMap[S][Cond.Condition->getCondStr()] = Flag;
  } else if (NonFuncCallMap[S][Cond.Condition->getCondStr()] != Flag) {
    errs() << "Contradictory CondChain " << CondChainID
           << " in setNonFuncCallInfo()\n";
    // dumpCondChain(CondChainID);
    ContraChains.insert(CondChainID);
    return false;
  }
  return true;
}

bool Analysis::setFuncCallInfo(LastDefInfo &Info, CondStatus &Cond,
                               const CallExpr *CE, unsigned CondChainID) {
  auto &FuncCallMap = Info.FuncCall;
  std::string FuncName = CE->getDirectCallee()->getNameAsString();
  bool Flag = Cond.Flag;
  if (Cond.Condition->isNot())
    Flag = !Flag;
  if (FuncCallMap.find(FuncName) == FuncCallMap.end() ||
      FuncCallMap[FuncName].find(CE) == FuncCallMap[FuncName].end() ||
      FuncCallMap[FuncName][CE].find(Cond.Condition->getCondStr()) ==
          FuncCallMap[FuncName][CE].end()) {
    FuncCallMap[FuncName][CE][Cond.Condition->getCondStr()] = Flag;
  } else if (FuncCallMap[FuncName][CE][Cond.Condition->getCondStr()] != Flag) {
    errs() << "Contradictory CondChain " << CondChainID
           << " in setFuncCallInfo()\n";
    // dumpCondChain(CondChainID);
    ContraChains.insert(CondChainID);
    return false;
  }
  return true;
}

bool Analysis::setParmVarInfo(LastDefInfo &Info, CondStatus &Cond,
                              const ParmVarDecl *PVD, unsigned CondChainID) {
  auto &ParmVarMap = Info.ParmVar;
  bool Flag = Cond.Flag;
  if (Cond.Condition->isNot())
    Flag = !Flag;
  if (ParmVarMap.find(PVD) == ParmVarMap.end() ||
      ParmVarMap[PVD].find(Cond.Condition->getCondStr()) ==
          ParmVarMap[PVD].end()) {
    ParmVarMap[PVD][Cond.Condition->getCondStr()] = Flag;
  } else if (ParmVarMap[PVD][Cond.Condition->getCondStr()] != Flag) {
    errs() << "Contradictory CondChain " << CondChainID
           << " in setParmVarInfo()\n";
    // dumpCondChain(CondChainID);
    ContraChains.insert(CondChainID);
    return false;
  }
  return true;
}

void Analysis::findContraInLastDef() {
  unsigned ID = Cfg->getExit().getBlockID();
  CondChains &CondChains = BlkChain[ID];
  unsigned Size = CondChains.size();
  LastDefList.resize(Size);
  bool NoContra;
  for (unsigned I = 0; I < Size; ++I) {
    if (ContraChains.find(I) != ContraChains.end())
      continue;
    NoContra = true;
    LastDefInfo &Info = LastDefList[I];
    auto &CondChain = CondChains[I].first;
    // auto &Path = CondChains[I].second;
    // outs() << "CondChain " << I << ":\n";
    unsigned CondNum = CondChain.size();
    for (unsigned J = 0; J < CondNum; ++J) {
      CondStatus &Cond = CondChain[J];
      if (Cond.Condition) {
        // handle call expression in the condition
        if (Cond.Condition->containCallExpr()) {
          for (const CallExpr *CE : Cond.Condition->getCallExprList()) {
            NoContra = setFuncCallInfo(Info, Cond, CE, I);
            if (!NoContra)
              break;
          }
        }
        if (!NoContra)
          break;
        // handle last definition and call expression in the trace back
        for (const Stmt *S : Cond.LastDefStmts) {
          // S->dumpColor();
          switch (S->getStmtClass()) {
          default:
            // errs() << "traceBack() unhandle: " << S->getStmtClassName() <<
            // "\n"; S->dumpColor();
            break;
          case Stmt::DeclStmtClass: {
            const DeclStmt *DS = cast<DeclStmt>(S);
            for (const Decl *D : DS->decls()) {
              if (const VarDecl *VD = dyn_cast<VarDecl>(D)) {
                const Expr *Init = VD->getInit()->IgnoreParenImpCasts();
                if (Init) {
                  // auto Cond = CondChains[I].first[J];
                  if (Init->getStmtClass() == Stmt::CXXMemberCallExprClass ||
                      Init->getStmtClass() == Stmt::CallExprClass) {
                    NoContra =
                        setFuncCallInfo(Info, Cond, cast<CallExpr>(Init), I);
                    if (!NoContra)
                      break;
                  } else {
                    NoContra = setNonFuncCallInfo(Info, Cond, Init, I);
                    if (!NoContra)
                      break;
                  }
                }
              }
            }
            break;
          }
          case Stmt::BinaryOperatorClass: {
            const BinaryOperator *BO = cast<BinaryOperator>(S);
            if (BO->isAssignmentOp()) {
              // Expr *LHS = BO->getLHS()->IgnoreParenImpCasts();
              Expr *RHS = BO->getRHS()->IgnoreParenImpCasts();
              if (RHS->getStmtClass() == Stmt::CXXMemberCallExprClass ||
                  RHS->getStmtClass() == Stmt::CallExprClass) {
                NoContra = setFuncCallInfo(Info, Cond, cast<CallExpr>(RHS), I);
              } else {
                NoContra = setNonFuncCallInfo(Info, Cond, RHS, I);
              }
            }
            break;
          }
          }
          if (!NoContra)
            break;
        }
        if (!NoContra)
          break;
        // handle ParmVars
        for (const ParmVarDecl *PVD : Cond.ParmVars) {
          NoContra = setParmVarInfo(Info, Cond, PVD, I);
          if (!NoContra)
            break;
        }
        if (!NoContra)
          break;
      }
    }
    // break;
  }
}

} // namespace BrInfo
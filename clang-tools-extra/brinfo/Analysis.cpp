#include "Analysis.h"
#include "clang/AST/Expr.h"
#include "clang/Analysis/CFG.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>

namespace BrInfo {

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
    CondChain.push_back({Condition, Flag, {}});
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

// void Analysis::dumpTraceBack(unsigned CondChain, unsigned Cond) {
//   std::vector<const Stmt *> TraceBack = TraceBacks[CondChain][Cond];
//   if (!TraceBack.empty()) {
//     outs() << "where: ";
//     for (const Stmt *S : TraceBack) {
//       S->dumpPretty(Context);
//     }
//   }
// }

std::string Analysis::getTraceBackStr(std::vector<const Stmt *> &TraceBacks) {
  std::string Str;
  llvm::raw_string_ostream OS(Str);
  if (!TraceBacks.empty()) {
    OS << ", where ";
    for (const Stmt *S : TraceBacks) {
      S->printPretty(OS, nullptr, Context.getPrintingPolicy());
    }
  }
  rtrim(Str);
  return Str;
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

void Analysis::dumpRequirements() {
  std::string FilePath = "/home/chubei/workspace/utgen/test_requirements.txt";
  std::error_code EC;
  llvm::raw_fd_stream OS(FilePath, EC);
  if (EC) {
    errs() << "Error: " << EC.message() << "\n";
    return;
  }

  unsigned ID = Cfg->getExit().getBlockID();
  CondChains &CondChains = BlkChain[ID];
  unsigned Size = CondChains.size();
  for (unsigned I = 0; I < Size; ++I) {
    if (ContraChains.find(I) == ContraChains.end()) {
      auto &CondChain = CondChains[I].first;
      // auto &Path = CondChains[I].second;
      OS << "CondChain " << I << ":\n";
      unsigned CondNum = CondChain.size();
      for (unsigned J = 0; J < CondNum; ++J) {
        CondStatus &Cond = CondChain[J];
        if (Cond.Condition) {
          OS << "// Precondition: " << Cond.Condition->getCondStr() << " is ";
          if (Cond.Condition->isNot())
            OS << (Cond.Flag ? "False" : "True");
          else
            OS << (Cond.Flag ? "True" : "False");
          OS << getTraceBackStr(Cond.TraceBacks) << "\n";
        }
      }
      if (!CallReturns[I].empty()) {
        // TODO: get class name
        OS << "// Create a class MockReader\n";
        for (auto &CallReturn : CallReturns[I]) {
          OS << "// Mock ";
          CallReturn.first->printPretty(OS, nullptr,
                                        Context.getPrintingPolicy());
          OS << ", which makes ";
          for (auto &Return : CallReturn.second) {
            OS << Return.first << " is ";
            OS << (Return.second ? "True" : "False") << ", ";
          }
          OS << "\n";
        }
      }
    }
    // OS << "\n";
    // break;
  }
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
            const Stmt *TraceBack = handleDeclRefExpr(DRE, Path, J);
            if (TraceBack) {
              // outs() << "Condition:\n";
              // Cond.Condition->getCond()->dumpColor();
              // outs() << "Value: " << (Cond.Flag ? "True" : "False") << "\n";
              // outs() << "DeclRef:\n";
              // DRE->dumpColor();
              outs() << "TraceBack:\n";
              TraceBack->dumpColor();
              bool Exam = examineTraceBack(DRE, TraceBack,
                                           Cond.Condition->isNot(), Cond.Flag);
              // outs() << "Exam: " << Exam << "\n";
              if (Exam)
                Cond.TraceBacks.push_back(TraceBack);
              else {
                errs() << "Contradictory conditions\n";
                ContraChains.insert(I);
              };
            }
          }
        }
      }
    }
    // break;
  }
}

bool Analysis::checkLiteralExpr(const Expr *Expr, bool IsNot, bool Flag) {
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
  }
  return Res;
}

bool Analysis::examineTraceBack(const DeclRefExpr *DeclRef,
                                const Stmt *TraceBack, bool IsNot, bool Flag) {
  bool Res = true;
  switch (TraceBack->getStmtClass()) {
  default:
    break;
  case Stmt::DeclStmtClass: {
    const DeclStmt *DS = cast<DeclStmt>(TraceBack);
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
    const BinaryOperator *BO = cast<BinaryOperator>(TraceBack);
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

// FIXME: handle ParmVar, consider change CallReturnInfo and CallReturns
const Stmt *Analysis::handleDeclRefExpr(const DeclRefExpr *DeclRef, Path &Path,
                                        unsigned Loc) {
  // outs() << "DeclRefExpr: ";
  // DeclRef->dumpPretty(Context);
  // outs() << "\n";
  bool Found = false;
  const Stmt *TraceBack = nullptr;
  // if (DeclRef->getDecl()->getKind() == Decl::Kind::ParmVar) {
  //   return TraceBack;
  // }
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
          // errs() << "handleDeclRefExpr() unhandle: " << Stmt->getStmtClassName()
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

void Analysis::getCallReturnInfo(CallReturnInfo &Info, CondStatus &Cond,
                                 const CallExpr *CE, unsigned CondChainID) {
  bool Flag = Cond.Flag;
  if (Cond.Condition->isNot())
    Flag = !Flag;
  if (Info.find(CE) == Info.end()) {
    Info[CE][Cond.Condition->getCondStr()] = Flag;
  } else if (Info[CE].find(Cond.Condition->getCondStr()) == Info[CE].end()) {
    Info[CE][Cond.Condition->getCondStr()] = Flag;
  } else if (Info[CE][Cond.Condition->getCondStr()] != Flag) {
    errs() << "Contradictory conditions\n";
    ContraChains.insert(CondChainID);
  }
}

void Analysis::findCallReturn() {
  unsigned ID = Cfg->getExit().getBlockID();
  CondChains &CondChains = BlkChain[ID];
  unsigned Size = CondChains.size();
  CallReturns.resize(Size);
  for (unsigned I = 0; I < Size; ++I) {
    CallReturnInfo &Info = CallReturns[I];
    auto &CondChain = CondChains[I].first;
    // auto &Path = CondChains[I].second;
    // outs() << "CondChain " << I << ":\n";
    unsigned CondNum = CondChain.size();
    for (unsigned J = 0; J < CondNum; ++J) {
      CondStatus &Cond = CondChain[J];
      if (Cond.Condition) {
        // handle the call expression in the condition
        if (Cond.Condition->containCallExpr()) {
          for (const CallExpr *CE : Cond.Condition->getCallExprList()) {
            getCallReturnInfo(Info, Cond, CE, I);
          }
        }
        // handle the call expression in the trace back
        for (const Stmt *S : Cond.TraceBacks) {
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
                    getCallReturnInfo(Info, Cond, cast<CallExpr>(Init), I);
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
                getCallReturnInfo(Info, Cond, cast<CallExpr>(RHS), I);
              }
            }
            break;
          }
          }
        }
      }
    }
    // break;
  }
}

} // namespace BrInfo
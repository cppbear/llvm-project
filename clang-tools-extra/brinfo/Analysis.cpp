#include "Analysis.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/AST/Stmt.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>

namespace BrInfo {

void IfCond::dump(const ASTContext &Context) { Cond->dumpPretty(Context); }

void LoopCond::dump(const ASTContext &Context) { Cond->dumpPretty(Context); }

void CaseCond::dump(const ASTContext &Context) {
  Cond->dumpPretty(Context);
  outs() << ": ";
  Case->dumpPretty(Context);
}

void DefaultCond::dump(const ASTContext &Context) {
  Cond->dumpPretty(Context);
  outs() << ": ";
  for (auto *Case : Cases) {
    Case->dumpPretty(Context);
    outs() << " ";
  }
}

void Analysis::dfs(CFGBlock Blk, BaseCond *Condition, bool Flag) {
  unsigned ID = Blk.getBlockID();

  // outs() << "Node: " << ID << " Parent: " << Parent << "\n";
  // if (Condition) {
  //   outs() << "    Cond: ";
  //   Condition->dumpPretty(Context);
  //   outs() << ": " << (Flag ? "True" : "False") << "\n";
  // }

  if (Parent != -1 && !BlkChain[Parent].empty()) {
    std::pair<CondChain, Path> Chain = BlkChain[Parent].back();
    auto CondChain = Chain.first;
    CondChain.push_back({Condition, Flag});
    auto Path = Chain.second;
    Path.push_back(ID);
    BlkChain[ID].push_back({CondChain, Path});
  }

  Parent = ID;
  Stmt *Terminator = Blk.getTerminatorStmt();
  if (Terminator) {
    // FIXME: Handle Loop and Try-catch
    switch (Terminator->getStmtClass()) {
    default:
      errs() << "Terminator: " << Terminator->getStmtClassName() << "\n";
      break;
    case Stmt::BinaryOperatorClass: {
      // deal with shotcut binary operator
      // BinaryOperator *BO = cast<BinaryOperator>(Terminator);
      // auto OpCode = BO->getOpcode();
      // auto *I = Blk.succ_begin();
      // if (OpCode == BinaryOperatorKind::BO_LAnd) {
      //   dfs(**I, nullptr, false);
      //   Parent = ID;
      // } else if (OpCode == BinaryOperatorKind::BO_LOr) {
      //   ++I;
      //   dfs(**I, nullptr, false);
      //   Parent = ID;
      // } else {
      //   errs() << "Unhandled BinaryOperatorKind: " << OpCode << "\n";
      // }
      // break;
    }
      LLVM_FALLTHROUGH;
    case Stmt::IfStmtClass: {
      BaseCond *Cond = new IfCond(
          cast<Expr>(Blk.getTerminatorCondition())->IgnoreParenImpCasts());
      // BaseCond *Cond = new IfCond(Blk.getTerminatorCondition());
      auto *I = Blk.succ_begin();
      dfs(**I, Cond, true);
      Parent = ID;
      ++I;
      dfs(**I, Cond, false);
      Parent = ID;
      break;
    }
    case Stmt::SwitchStmtClass: {
      CFGBlock *DefaultBlk = nullptr;
      std::vector<Stmt *> Cases;
      for (auto I : Blk.succs()) {
        Stmt *Label = I->getLabel();
        assert(Label);
        BaseCond *Cond = nullptr;
        if (Label->getStmtClass() == Stmt::CaseStmtClass) {
          Stmt *Case = cast<CaseStmt>(Label)->getLHS();
          Cases.push_back(Case);
          Cond = new CaseCond(
              cast<Expr>(Blk.getTerminatorCondition())->IgnoreParenImpCasts(),
              Case);
          dfs(*I, Cond, true);
          Parent = ID;
        } else {
          // Default case
          assert(Label->getStmtClass() == Stmt::DefaultStmtClass);
          DefaultBlk = I;
        }
      }
      if (DefaultBlk) {
        BaseCond *Cond = new DefaultCond(
            cast<Expr>(Blk.getTerminatorCondition())->IgnoreParenImpCasts(),
            Cases);
        dfs(*DefaultBlk, Cond, false);
        Parent = ID;
      }
      break;
    }
    case Stmt::BreakStmtClass:
      dfs(**Blk.succ_begin(), nullptr, false);
      Parent = ID;
      break;
    case Stmt::ForStmtClass: {
    }
      LLVM_FALLTHROUGH;
    case Stmt::WhileStmtClass: {
    }
      LLVM_FALLTHROUGH;
    case Stmt::DoStmtClass: {
      BaseCond *Cond = new LoopCond(
          cast<Expr>(Blk.getTerminatorCondition())->IgnoreParenImpCasts());
      auto *I = Blk.succ_begin();
      dfs(**I, Cond, true);
      Parent = ID;
      ++I;
      dfs(**I, Cond, false);
      Parent = ID;
      break;
    }
    }
  } else if (Blk.succ_size() == 1 && !Blk.getLoopTarget()) {
    dfs(**Blk.succ_begin(), nullptr, false);
    Parent = ID;
  } else if (Blk.getBlockID() != Cfg->getExit().getBlockID()) {
    // TODO: Handle back edges in loops
    outs() << "Unhandle Block:\n";
    Blk.dump();
  }
}

void Analysis::getCondChain() {
  outs() << "============================\n";
  dfs(Cfg->getEntry(), nullptr, false);
}

void Analysis::dumpBlkChain() {
  for (unsigned I = 0; I < BlkChain.size(); ++I) {
    outs() << "Block: " << I << "\n";
    for (auto &Chain : BlkChain[I]) {
      auto CondChain = Chain.first;
      auto Path = Chain.second;
      for (auto &Cond : CondChain) {
        if (Cond.first) {
          Cond.first->dump(Context);
          outs() << ":=" << (Cond.second ? "True" : "False") << " -> ";
        }
      }
      outs() << "\n";
      for (auto &ID : Path) {
        outs() << ID << " ";
      }
      outs() << "\n";
    }
  }
}

void Analysis::dumpBlkChain(unsigned ID) {
  outs() << "Block: " << ID << "\n";
  for (auto &Chain : BlkChain[ID]) {
    auto CondChain = Chain.first;
    auto Path = Chain.second;
    for (auto &Cond : CondChain) {
      if (Cond.first) {
        Cond.first->dump(Context);
        outs() << ":=" << (Cond.second ? "True" : "False") << " -> ";
      }
    }
    outs() << "\n";
    for (auto &ID : Path) {
      outs() << ID << " ";
    }
    outs() << "\n";
  }
}

void Analysis::dumpCondChain() {
  unsigned ID = Cfg->getExit().getBlockID();
  auto CondChains = BlkChain[ID];
  unsigned Size = CondChains.size();
  for (unsigned I = 0; I < Size; ++I) {
    auto CondChain = CondChains[I].first;
    auto Path = CondChains[I].second;
    outs() << "CondChain " << I << ":\n  ";
    for (auto &Cond : CondChain) {
      if (Cond.first) {
        Cond.first->dump(Context);
        outs() << ": " << (Cond.second ? "True" : "False") << " -> ";
      }
    }
    outs() << "\n  ";
    for (auto &ID : Path) {
      outs() << ID << " ";
    }
    outs() << "\n";
    // break;
  }
}

void Analysis::condDerive() {
  unsigned ID = Cfg->getExit().getBlockID();
  auto &CondChains = BlkChain[ID];
  unsigned Size = CondChains.size();
  for (unsigned I = 0; I < Size; ++I) {
    CondMap.clear();
    auto &CondChain = CondChains[I].first;
    for (auto &Cond : CondChain) {
      if (Cond.first) {
        TmpCond.first = nullptr;
        const Stmt *S = Cond.first->getCond();
        CondMap[S] = Cond.second;

        if (S->getStmtClass() == Stmt::BinaryOperatorClass) {
          const BinaryOperator *BO = cast<BinaryOperator>(S);
          if (BO->isLogicalOp()) {
            handle(BO, Cond.second);
          }
        }
        if (TmpCond.first) {
          Cond = TmpCond;
        }
      }
    }
    // break;
  }
}

void Analysis::handle(const BinaryOperator *BO, bool Flag) {
  Expr *LHS = BO->getLHS()->IgnoreParenImpCasts();
  Expr *RHS = BO->getRHS()->IgnoreParenImpCasts();
  bool LHSKnown = CondMap.find(LHS) != CondMap.end();
  bool RHSKnown = CondMap.find(RHS) != CondMap.end();
  if (LHSKnown && !RHSKnown) {
    topDown(Flag, BO->getOpcode(), LHS, RHS);
  } else if (!LHSKnown && RHSKnown) {
    topDown(Flag, BO->getOpcode(), RHS, LHS);
  } else if (!LHSKnown && !RHSKnown) {
    bool Res = downTop(LHS) || downTop(RHS);
    if (Res) {
      handle(BO, Flag);
    } else {
      errs() << "Unhandle condition\n";
    }
  }
}

// derive the condition from the parent to the child
void Analysis::topDown(bool Flag, BinaryOperator::Opcode Opcode,
                       const Expr *Known, const Expr *Unknown) {
  bool Val = CondMap[Known];
  switch (Opcode) {
  case BinaryOperatorKind::BO_LAnd:
    if (Val) {
      CondMap[Unknown] = Flag;
      if (Unknown->getStmtClass() == Stmt::BinaryOperatorClass &&
          cast<BinaryOperator>(Unknown)->isLogicalOp()) {
        handle(cast<BinaryOperator>(Unknown), Flag);
      } else {
        const Stmt *S = static_cast<const Stmt *>(Unknown);
        BaseCond *Cond = new IfCond(S);
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
        handle(cast<BinaryOperator>(Unknown), Flag);
      } else {
        const Stmt *S = static_cast<const Stmt *>(Unknown);
        BaseCond *Cond = new IfCond(S);
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
bool Analysis::downTop(const Expr *Parent) {
  bool Res = false;
  if (Parent->getStmtClass() == Stmt::BinaryOperatorClass) {
    const BinaryOperator *BO = cast<BinaryOperator>(Parent);
    if (BO->isLogicalOp()) {
      Expr *LHS = BO->getLHS()->IgnoreParenImpCasts();
      Expr *RHS = BO->getRHS()->IgnoreParenImpCasts();
      if (CondMap.find(LHS) == CondMap.end()) {
        downTop(LHS);
      }
      if (CondMap.find(RHS) == CondMap.end()) {
        downTop(RHS);
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

} // namespace BrInfo
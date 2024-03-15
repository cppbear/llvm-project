#include "Analysis.h"
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

  if (Parent != -1) {
    CondChains Chains = BlkChain[Parent];
    for (auto &Chain : Chains) {
      auto CondChain = Chain.first;
      CondChain.push_back({Condition, Flag});
      auto Path = Chain.second;
      Path.push_back(ID);
      BlkChain[ID].insert({CondChain, Path});
    }
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
      // TODO: Handle && and || operators
      // whether to combine the conditions or not
      // break;
    }
      LLVM_FALLTHROUGH;
    case Stmt::IfStmtClass: {
      BaseCond *Cond = new IfCond(Blk.getTerminatorCondition());
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
          Cond = new CaseCond(Blk.getTerminatorCondition(), Case);
          dfs(*I, Cond, true);
          Parent = ID;
        } else {
          // Default case
          assert(Label->getStmtClass() == Stmt::DefaultStmtClass);
          DefaultBlk = I;
        }
      }
      if (DefaultBlk) {
        BaseCond *Cond = new DefaultCond(Blk.getTerminatorCondition(), Cases);
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
      BaseCond *Cond = new LoopCond(Blk.getTerminatorCondition());
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

void Analysis::dumpCondChain() {
  unsigned ID = Cfg->getExit().getBlockID();
  for (auto &Chain : BlkChain[ID]) {
    auto CondChain = Chain.first;
    auto Path = Chain.second;
    for (auto &Cond : CondChain) {
      if (Cond.first) {
        Cond.first->dump(Context);
        outs() << ": " << (Cond.second ? "True" : "False") << " -> ";
      }
    }
    outs() << "\n";
    for (auto &ID : Path) {
      outs() << ID << " ";
    }
    outs() << "\n";
  }
}

} // namespace BrInfo
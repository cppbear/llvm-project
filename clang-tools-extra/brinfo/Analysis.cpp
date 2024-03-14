#include "Analysis.h"
#include "clang/AST/Stmt.h"
#include "llvm/Support/raw_ostream.h"
#include <string>

namespace BrInfo {

// FIXME: Handle back edges in loops
void Analysis::dfs(CFGBlock Blk, Stmt *Condition, bool Flag) {
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

  // FIXME: Handle Caces
  Stmt *Label = Blk.getLabel();
  if (Label) {
    // outs() << Label->getStmtClassName() << "\n";
    // Label->dumpPretty(Context);
    auto Kind = Label->getStmtClass();
    if (Kind == Stmt::CaseStmtClass) {
      CaseStmt *S = cast<CaseStmt>(Label);
      S->getLHS()->dumpPretty(Context);
    } else if (Kind == Stmt::DefaultStmtClass) {
      // DefaultStmt *S = cast<DefaultStmt>(Label);
      // S->getLHS()->dumpPretty(Context);
    } else if (Kind == Stmt::CXXCatchStmtClass) {
    }
  }

  Parent = ID;
  Stmt *Terminator = Blk.getTerminatorStmt();
  if (Terminator) {
    // FIXME: Handle Switch
    if (Terminator->getStmtClass() == Stmt::SwitchStmtClass) {
      errs() << "SwitchStmt\n";
      Blk.getTerminatorCondition()->dumpPretty(Context);
      for (auto I : Blk.succs()) {
        dfs(*I, nullptr, false);
      }
    } else if (Blk.succ_size() == 2) {
      auto *I = Blk.succ_begin();
      dfs(**I, Blk.getTerminatorCondition(), true);
      Parent = ID;
      ++I;
      dfs(**I, Blk.getTerminatorCondition(), false);
      Parent = ID;
    } else {
      errs() << Terminator->getStmtClassName() << "\n";
    }
  } else if (Blk.succ_size() == 1) {
    dfs(**Blk.succ_begin(), nullptr, false);
    Parent = ID;
  } else if (!Blk.empty()) {
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
          Cond.first->dumpPretty(Context);
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
}

void Analysis::dumpCondChain() {
  unsigned ID = Cfg->getExit().getBlockID();
  for (auto &Chain : BlkChain[ID]) {
    auto CondChain = Chain.first;
    auto Path = Chain.second;
    for (auto &Cond : CondChain) {
      if (Cond.first) {
        Cond.first->dumpPretty(Context);
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
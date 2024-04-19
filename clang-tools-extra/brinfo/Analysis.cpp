#include "Analysis.h"
#include "OrderedSet.h"
#include "clang/Analysis/CFG.h"
#include <cassert>

namespace BrInfo {

void Analysis::clear() {
  Cfg = nullptr;
  Context = nullptr;
  FuncDecl = nullptr;
  Signature.clear();
  BlkChain.clear();
  // LastDefList.clear();
  // CallExprList.clear();
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
  // BlkChain[Cfg->getEntry().getBlockID()].push_back(
  //     CondChainInfo(&Cfg->getEntry()));
  BlkChain[Cfg->getEntry().getBlockID()].push_back(
      {{{nullptr, false, {}, {}}}, {&Cfg->getEntry()}, false, {}, {}});
}

void Analysis::analyze() {
  getCondChains();
  unsigned ExitID = Cfg->getExit().getBlockID();
  CondChainList &ChainList = BlkChain[ExitID];
  for (CondChainInfo &ChainInfo : ChainList) {
    ChainInfo.analyze(Context);
  }
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
    // std::pair<CondChain, Path> Chain = BlkChain[Parent].back();
    CondChainInfo &ChainInfo = BlkChain[Parent].back();
    CondChain Chain = ChainInfo.Chain;
    Chain.push_back({Condition, Flag, {}, {}});
    BlkPath Path = ChainInfo.Path;
    Path.push_back(Blk);
    BlkChain[ID].push_back({Chain, Path, false, {}, {}});
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
        Cond = new IfCond(cast<Expr>(InnerCond)->IgnoreParenImpCasts());
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
                                Case);
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
          Cond = new DefaultCond(InnerCond, Cases);
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
        Cond = new LoopCond(cast<Expr>(InnerCond)->IgnoreParenImpCasts());
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
  for (CondChainInfo &ChainInfo : BlkChain[ID]) {
    CondChain Chain = ChainInfo.Chain;
    BlkPath Path = ChainInfo.Path;
    for (CondStatus &Cond : Chain) {
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

// void Analysis::dumpTraceBack(CondStatus &Cond) {
//   if (!Cond.LastDefStmts.empty() || !Cond.ParmVars.empty()) {
//     errs() << "where: ";
//     for (const Stmt *S : Cond.LastDefStmts) {
//       S->dumpPretty(*Context);
//     }
//     for (const ParmVarDecl *PVD : Cond.ParmVars) {
//       errs() << PVD->getNameAsString() << " is ParmVar, ";
//     }
//   }
// }

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

// void Analysis::dumpCondChain(unsigned ID) {
//   unsigned ExitID = Cfg->getExit().getBlockID();
//   auto CondChains = BlkChain[ExitID];

//   CondChain CondChain = CondChains[ID].Chain;
//   BlkPath Path = CondChains[ID].Path;
//   outs() << "CondChain " << ID << ":\n  ";
//   unsigned CondNum = CondChain.size();
//   for (unsigned J = 0; J < CondNum; ++J) {
//     CondStatus &Cond = CondChain[J];
//     if (Cond.Condition) {
//       Cond.Condition->dump(Context);
//       outs() << ": " << (Cond.Flag ? "True" : "False") << " ";
//       dumpTraceBack(Cond);
//       outs() << " -> ";
//     }
//   }
//   outs() << "\n  ";
//   for (CFGBlock *Blk : Path) {
//     outs() << Blk->getBlockID() << " ";
//   }
//   outs() << "\n";
// }

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
  CondChainList &CondChains = BlkChain[ExitID];
  unsigned Size = CondChains.size();
  std::string Require;
  llvm::raw_string_ostream OS(Require);
  std::vector<std::string> CondVec;
  std::set<CondStatus> CondStatusSet;
  for (unsigned ID = 0; ID < Size; ++ID) {
    if (!CondChains[ID].IsContra) {
      std::string CondChainStr = formatID(std::to_string(ID));
      std::string Input = "";
      int I = 0;
      for (ParmVarDecl *Param : FuncDecl->parameters()) {
        if (I++ > 0) {
          Input += ", ";
        }
        Input += Param->getNameAsString() + " is a " +
                 Param->getType().getAsString();
      }
      Json[CondChainStr]["input"] = Input;
      CondChain &CondChain = CondChains[ID].Chain;
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
      // if (!LastDefList[ID].FuncCall.empty()) {
      //   auto &FuncCallMap = LastDefList[ID].FuncCall;
      //   for (auto &Func : FuncCallMap) {
      //     // TODO: In the order of these CallExpr in the source file
      //     for (auto &CallExpr : Func.second) {
      //       // CallExpr.first->printPretty(OS, nullptr,
      //       //                             Context.getPrintingPolicy());
      //       for (auto &Return : CallExpr.second) {
      //         OS << Return.first << " is ";
      //         OS << (Return.second ? "true" : "false");
      //         OS.flush();
      //         CondVec.push_back(Require);
      //         Require.clear();
      //       }
      //       Json[CondChainStr]["mock"].push_back(
      //           {{"function", Func.first}, {"condition", CondVec}});
      //       Require.clear();
      //       CondVec.clear();
      //     }
      //   }
      // }
      std::string Result = "";
      if (FuncDecl->getReturnType() != Context->VoidTy) {
        BlkPath &Path = CondChains[ID].Path;
        CFGBlock *Blk = Path[Path.size() - 2];
        if (!Blk->hasNoReturnElement()) {
          if (std::optional<CFGStmt> S = Blk->back().getAs<CFGStmt>()) {
            if (S->getStmt()->getStmtClass() == Stmt::ReturnStmtClass) {
              const ReturnStmt *RS = cast<ReturnStmt>(S->getStmt());
              RS->getRetValue()->printPretty(OS, nullptr,
                                             Context->getPrintingPolicy());
              OS.flush();
              Result +=
                  "a " + FuncDecl->getReturnType().getAsString() + " value: ";
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

} // namespace BrInfo
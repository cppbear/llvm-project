#include "Analysis.h"
#include "clang/AST/Type.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>

namespace BrInfo {

string formatID(string ID) {
  string Str;
  size_t Size = ID.size();
  if (Size < 3) {
    Str = string(3 - Size, '0') + ID;
  } else {
    Str = ID;
  }
  return Str;
}

void Analysis::setType(AnalysisType T) {
  Type = T;
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
  ColorOfBlk.resize(Cfg->getNumBlockIDs(), 0);
  Parent = -1;
  // BlkChain[Cfg->getEntry().getBlockID()].push_back(
  //     CondChainInfo(&Cfg->getEntry()));
  BlkChain[Cfg->getEntry().getBlockID()].push_back(
      {{{nullptr, false, {}, {}}}, {&Cfg->getEntry()}, false, {}, {}});
}

void Analysis::analyze() {
  extractCondChains();
  unsigned ExitID = Cfg->getExit().getBlockID();
  CondChainList &ChainList = BlkChain[ExitID];
  for (CondChainInfo &ChainInfo : ChainList) {
    ChainInfo.analyze(Context);
  }
  // dumpCondChains();
  condChainsToReqs();
  clear();
}

void Analysis::dumpReqToJson(string ProjectPath, string FileName,
                             string ClassName, string FuncName) {
  string FilePath = ProjectPath;
  if (Type == AnalysisType::FILE) {
    FilePath += "/" + FileName + "_req.json";
  } else {
    FilePath += "/" + (ClassName.empty() ? "" : ClassName + "_") + FuncName +
                "_req.json";
  }
  error_code EC;
  llvm::raw_fd_stream File(FilePath, EC);
  if (EC) {
    errs() << "Error: " << EC.message() << "\n";
    return;
  }
  File << Results.dump(4);
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
    Signature += Param->getType().getAsString();
  }
  Signature += ")";
  // outs() << "Signature: " << Signature << "\n";
}

void Analysis::extractCondChains() {
  dfsTraverseCFG(&Cfg->getEntry(), nullptr, false);
  toBlack();
  for (const CFGBlock *Try : Cfg->try_blocks()) {
    Parent = Try->getBlockID();
    CFGBlock *Blk = *Try->succ_begin();
    BlkChain[Parent].push_back(
        {{{nullptr, false, {}, {}}}, {Try}, false, {}, {}});
    dfsTraverseCFG(Blk, nullptr, false);
    toBlack();
  }
}

long Analysis::findBestCover(unordered_set<string> &Uncovered,
                             const CondChainList &CondChains,
                             vector<bool> &Used) {
  unsigned MaxCover = 0;
  long Index = -1;
  for (unsigned I = 0; I < CondChains.size(); ++I) {
    if (Used[I])
      continue;
    unsigned Cover = 0;
    for (const string &Cond : CondChains[I].getCondSet()) {
      if (Uncovered.find(Cond) != Uncovered.end()) {
        ++Cover;
      }
    }
    if (Cover > MaxCover) {
      MaxCover = Cover;
      Index = (long)I;
    }
  }
  return Index;
}

unordered_set<unsigned> Analysis::findMinCover() {
  unordered_set<string> AllElements;
  unordered_set<unsigned> Cover;
  unsigned ExitID = Cfg->getExit().getBlockID();
  CondChainList CondChains = BlkChain[ExitID];
  CondChains.erase(remove_if(CondChains.begin(), CondChains.end(),
                             [](const CondChainInfo &ChainInfo) {
                               return ChainInfo.IsContra;
                             }),
                   CondChains.end());

  for (CondChainInfo &ChainInfo : CondChains) {
    AllElements.insert(ChainInfo.getCondSet().begin(),
                       ChainInfo.getCondSet().end());
  }

  unordered_set<string> Uncovered(AllElements);
  vector<bool> Used(CondChains.size(), false);
  while (!Uncovered.empty()) {
    long Index = findBestCover(Uncovered, CondChains, Used);
    if (Index == -1)
      break;
    for (const string &Cond : CondChains[Index].getCondSet()) {
      Uncovered.erase(Cond);
    }
    Cover.insert(Index);
    Used[Index] = true;
  }
  return Cover;
}

void Analysis::condChainsToReqs() {
  json Json;

  unsigned ExitID = Cfg->getExit().getBlockID();
  CondChainList CondChains = BlkChain[ExitID];
  CondChains.erase(remove_if(CondChains.begin(), CondChains.end(),
                             [](const CondChainInfo &ChainInfo) {
                               return ChainInfo.IsContra;
                             }),
                   CondChains.end());
  unsigned Size = CondChains.size();
  unordered_set<unsigned> MinCover = findMinCover();

  Json["function"] = FuncDecl->getNameAsString();

  string FileName =
      Context->getSourceManager().getFilename(FuncDecl->getLocation()).str();
  FileName = FileName.substr(FileName.find_last_of("/") + 1);
  Json["file"] = FileName;

  string Input = "";
  int I = 0;
  for (ParmVarDecl *Param : FuncDecl->parameters()) {
    if (I++ > 0) {
      Input += ", ";
    }
    Input +=
        Param->getNameAsString() + " is a " + Param->getType().getAsString();
  }
  Json["input"] = Input;

  string ClassName = "";
  if (FuncDecl->isCXXClassMember()) {
    ClassName = cast<CXXRecordDecl>(FuncDecl->getParent())->getNameAsString();
  }
  Json["class"] = ClassName;

  for (unsigned ID = 0; ID < Size; ++ID) {
    // if (CondChains[ID].IsContra)
    //   continue;
    string CondChainStr = formatID(to_string(ID));
    string Result = "";
    if (FuncDecl->getReturnType() != Context->VoidTy) {
      Result = CondChains[ID].getReturnStr(
          Context, FuncDecl->getReturnType().getAsString());
    }
    json J = CondChains[ID].toTestReqs(Context);
    if (MinCover.find(ID) == MinCover.end())
      J["mincover"] = false;
    else
      J["mincover"] = true;
    J["result"] = Result;
    if (CondChains[ID].Path[0]->getBlockID() == Cfg->getEntry().getBlockID()) {
      J["incatch"] = false;
    } else {
      J["incatch"] = true;
    }
    Json["chains"][CondChainStr] = J;
  }
  Json["chains"]["size"] = Json["chains"].size();
  Json["chains"]["mincover"] = MinCover.size();
  Results[Signature] = Json;
}

void Analysis::clear() {
  Cfg = nullptr;
  Context = nullptr;
  FuncDecl = nullptr;
  Signature.clear();
  BlkChain.clear();
  ColorOfBlk.clear();
  Parent = -1;
}

void Analysis::toBlack() {
  for (unsigned char &Color : ColorOfBlk) {
    if (Color == 1) {
      Color = 2;
    }
  }
}

void Analysis::dfsTraverseCFG(CFGBlock *Blk, BaseCond *Condition, bool Flag) {
  unsigned ID = Blk->getBlockID();
  if (ColorOfBlk[ID] == 0)
    ColorOfBlk[ID] = 1;

  // errs() << "Block: " << ID << " Parent: " << Parent << "\n";

  if (ColorOfBlk[ID] == 2 && ID != Cfg->getExit().getBlockID()) {
    dfsTraverseCFG(&Cfg->getExit(), Condition, false);
    return;
  }

  if (Parent != -1 && !BlkChain[Parent].empty()) {
    CondChainInfo &ChainInfo = BlkChain[Parent].back();
    CondChain Chain = ChainInfo.Chain;
    BlkPath Path = ChainInfo.Path;
    // unsigned I = 0;
    // for (const CFGBlock *Blk : Path) {
    //   if (I++ > 0)
    //     errs() << " \033[36m\033[1m->\033[0m ";
    //   errs() << Blk->getBlockID();
    // }
    // errs() << "\n";
    BlkPath SortedPath = Path;
    llvm::sort(SortedPath.begin(), SortedPath.end());
    if (binary_search(SortedPath.begin(), SortedPath.end(), Blk)) {
      // Loop detected
      for (auto Succ : Blk->succs()) {
        if (!binary_search(SortedPath.begin(), SortedPath.end(), Succ)) {
          dfsTraverseCFG(Succ, Condition, false);
          return;
        }
      }
      return;
    }
    Chain.push_back({Condition, Flag, {}, {}});
    Path.push_back(Blk);
    BlkChain[ID].push_back({Chain, Path, false, {}, {}});
  }

  Parent = ID;
  Stmt *Terminator = Blk->getTerminatorStmt();
  if (Terminator) {
    switch (Terminator->getStmtClass()) {
    default:
      errs() << "Terminator: " << Terminator->getStmtClassName() << "\n";
      break;
    case Stmt::ConditionalOperatorClass:
    case Stmt::BinaryOperatorClass:
    case Stmt::IfStmtClass: {
      BaseCond *Cond = nullptr;
      Stmt *InnerCond = Blk->getTerminatorCondition();
      if (InnerCond) {
        Cond = new IfCond(cast<Expr>(InnerCond)->IgnoreParenImpCasts());
      }
      auto *I = Blk->succ_begin();
      if (Cond) {
        dfsTraverseCFG(*I, Cond, true);
        Parent = ID;
        ++I;
        dfsTraverseCFG(*I, Cond, false);
        Parent = ID;
      } else {
        dfsTraverseCFG(*I, nullptr, false);
        Parent = ID;
      }
      break;
    }
    case Stmt::SwitchStmtClass: {
      CFGBlock *DefaultBlk = nullptr;
      Stmt *InnerCond = Blk->getTerminatorCondition();
      vector<Stmt *> Cases;
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
          dfsTraverseCFG(I, Cond, true);
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
        dfsTraverseCFG(DefaultBlk, Cond, false);
        Parent = ID;
      }
      break;
    }
    case Stmt::BreakStmtClass:
      dfsTraverseCFG(*Blk->succ_begin(), nullptr, false);
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
        dfsTraverseCFG(*I, Cond, true);
        Parent = ID;
        ++I;
        dfsTraverseCFG(*I, Cond, false);
        Parent = ID;
      } else {
        dfsTraverseCFG(*I, nullptr, false);
        Parent = ID;
      }
      break;
    }
    case Stmt::ContinueStmtClass: {
      dfsTraverseCFG(*Blk->succ_begin(), nullptr, false);
      Parent = ID;
      break;
    }
    }
  } else if (Blk->succ_size() == 1) {
    dfsTraverseCFG(*Blk->succ_begin(), nullptr, false);
    Parent = ID;
  } else if (Blk->getBlockID() != Cfg->getExit().getBlockID()) {
    outs() << "Unhandle Block:\n";
    Blk->dump();
  }
}

void Analysis::dumpCondChains() {
  unsigned ExitID = Cfg->getExit().getBlockID();
  CondChainList &CondChains = BlkChain[ExitID];
  unsigned Size = CondChains.size();
  for (unsigned I = 0; I < Size; ++I) {
    errs() << "CondChain " << I << ":\n";
    CondChains[I].dump(Context, 2);
  }
}

void Analysis::dumpBlkChain(unsigned ID) {
  unsigned I = 0;
  for (CondChainInfo &ChainInfo : BlkChain[ID]) {
    errs() << "  Chain " << I++ << ":\n";
    ChainInfo.dump(Context, 4);
  }
}

void Analysis::dumpBlkChain() {
  for (unsigned I = 0; I < BlkChain.size(); ++I) {
    errs() << "Block: " << I << "\n";
    dumpBlkChain(I);
  }
}

} // namespace BrInfo
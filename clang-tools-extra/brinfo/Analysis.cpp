#include "Analysis.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/Type.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <sstream>

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

bool Analysis::init(CFG *CFG, ASTContext *Context, const FunctionDecl *FD) {
  string Loc = FD->getLocation().printToString(Context->getSourceManager());
  if (VisitedFuncs.find(Loc) == VisitedFuncs.end()) {
    VisitedFuncs.insert(Loc);
  } else {
    return false;
  }
  Cfg = CFG;
  this->Context = Context;
  FocalFunc = FD;
  setSignature();
  BlkChain.resize(Cfg->getNumBlockIDs());
  ColorOfBlk.resize(Cfg->getNumBlockIDs(), 0);
  Parent = -1;
  // BlkChain[Cfg->getEntry().getBlockID()].push_back(
  //     CondChainInfo(&Cfg->getEntry()));
  BlkChain[Cfg->getEntry().getBlockID()].push_back(
      {{{nullptr, false, {}, {}}}, {&Cfg->getEntry()}, false, {}, {}});
  return true;
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
  string FilePath = ProjectPath + "/llm_reqs";
  llvm::sys::fs::create_directories(FilePath);
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
  const FunctionDecl *CanonicalDecl = FocalFunc->getCanonicalDecl();
  Signature = CanonicalDecl->getReturnType().getAsString() + " ";
  if (CanonicalDecl->isCXXClassMember()) {
    Signature +=
        cast<CXXRecordDecl>(CanonicalDecl->getParent())->getNameAsString() +
        "::";
  }
  Signature += CanonicalDecl->getNameAsString() + "(";
  int I = 0;
  for (ParmVarDecl *Param : CanonicalDecl->parameters()) {
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

long Analysis::findBestCover(set<pair<const Stmt *, bool>> &Uncovered,
                             const CondChainList &CondChains,
                             vector<bool> &Used) {
  unsigned MaxCover = 0;
  long Index = -1;
  for (unsigned I = 0; I < CondChains.size(); ++I) {
    if (Used[I])
      continue;
    unsigned Cover = 0;
    for (const pair<const Stmt *, bool> &Cond : CondChains[I].getCondSet()) {
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
  set<pair<const Stmt *, bool>> AllElements;
  unordered_set<unsigned> Cover;
  unsigned ExitID = Cfg->getExit().getBlockID();
  CondChainList CondChains = BlkChain[ExitID];
  CondChains.erase(remove_if(CondChains.begin(), CondChains.end(),
                             [](const CondChainInfo &ChainInfo) {
                               return ChainInfo.IsContra;
                             }),
                   CondChains.end());

  for (CondChainInfo &ChainInfo : CondChains) {
    AllElements.merge(ChainInfo.getCondSet());
  }

  set<pair<const Stmt *, bool>> Uncovered(AllElements);
  vector<bool> Used(CondChains.size(), false);
  while (!Uncovered.empty()) {
    long Index = findBestCover(Uncovered, CondChains, Used);
    if (Index == -1)
      break;
    for (const pair<const Stmt *, bool> &Cond :
         CondChains[Index].getCondSet()) {
      Uncovered.erase(Cond);
    }
    Cover.insert(Index);
    Used[Index] = true;
  }
  return Cover;
}

vector<string> split(const string &Str, char Delim) {
  vector<string> Tokens;
  stringstream Ss(Str);
  string Token;
  while (getline(Ss, Token, Delim)) {
    if (!Token.empty())
      Tokens.push_back(Token);
  }
  return Tokens;
}

string getNamespace(const Decl *D) {
  const DeclContext *DC = D->getDeclContext();
  string Namespace;
  while (DC) {
    if (const NamespaceDecl *ND = dyn_cast<NamespaceDecl>(DC)) {
      if (!Namespace.empty())
        Namespace = ND->getNameAsString() + "::" + Namespace;
      else
        Namespace = ND->getNameAsString();
    }
    DC = DC->getParent();
  }
  if (Namespace.empty())
    return "";
  return Namespace;
}

void Analysis::condChainsToReqs() {
  json Json;

  string BeginLoc =
      FocalFunc->getBeginLoc().printToString(Context->getSourceManager());
  string EndLoc =
      FocalFunc->getEndLoc().printToString(Context->getSourceManager());
  string BeginLine = split(BeginLoc, ':')[1];
  string EndLine = split(EndLoc, ':')[1];
  Json["loc"] = {stoi(BeginLine), stoi(EndLine)};

  unsigned ExitID = Cfg->getExit().getBlockID();
  CondChainList CondChains = BlkChain[ExitID];
  CondChains.erase(remove_if(CondChains.begin(), CondChains.end(),
                             [](const CondChainInfo &ChainInfo) {
                               return ChainInfo.IsContra;
                             }),
                   CondChains.end());
  unsigned Size = CondChains.size();
  unordered_set<unsigned> MinCover = findMinCover();

  const FunctionDecl *CanonicalDecl = FocalFunc->getCanonicalDecl();
  Json["operator"] = CanonicalDecl->isOverloadedOperator();
  Json["name"] = CanonicalDecl->getNameAsString();
  Json["namespace"] = getNamespace(CanonicalDecl);

  string FilePath = Context->getSourceManager()
                        .getFilename(CanonicalDecl->getLocation())
                        .str();
  SmallVector<char, 128> RealPath;
  sys::fs::real_path(FilePath, RealPath);
  string RealFilePath(RealPath.begin(), RealPath.end());
  // FileName = FilePath.substr(FileName.find_last_of("/") + 1);
  Json["decl_file"] = RealFilePath;

  string Input = "";
  int I = 0;
  for (ParmVarDecl *Param : CanonicalDecl->parameters()) {
    if (I++ > 0) {
      Input += ", ";
    }
    Input +=
        Param->getNameAsString() + " is a " + Param->getType().getAsString();
  }
  Json["input"] = Input;

  string ClassName = "";
  if (CanonicalDecl->isCXXClassMember()) {
    ClassName =
        cast<CXXRecordDecl>(CanonicalDecl->getParent())->getNameAsString();
  }
  Json["class"] = ClassName;

  unsigned MinCoverNum = 0;
  for (unsigned ID = 0; ID < Size; ++ID) {
    // if (CondChains[ID].IsContra)
    //   continue;
    string CondChainStr = formatID(to_string(ID));
    string Result = "";
    if (CanonicalDecl->getReturnType() != Context->VoidTy) {
      Result = CondChains[ID].getReturnStr(
          Context, CanonicalDecl->getReturnType().getAsString());
    }
    json J = CondChains[ID].toTestReqs(Context);
    if (!MinCover.empty() && MinCover.find(ID) == MinCover.end())
      J["mincover"] = false;
    else {
      J["mincover"] = true;
      ++MinCoverNum;
    }
    J["result"] = Result;
    if (CondChains[ID].Path[0]->getBlockID() == Cfg->getEntry().getBlockID()) {
      J["incatch"] = false;
    } else {
      J["incatch"] = true;
    }
    Json["chains"][CondChainStr] = J;
  }
  Json["chains"]["size"] = Json["chains"].size();
  Json["chains"]["mincover"] = MinCoverNum;
  Results[Signature] = Json;
}

void Analysis::clear() {
  Cfg = nullptr;
  Context = nullptr;
  FocalFunc = nullptr;
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
      for (CFGBlock::AdjacentBlock Succ : Blk->succs()) {
        if (Succ.isReachable()) {
          if (!binary_search(SortedPath.begin(), SortedPath.end(), Succ)) {
            dfsTraverseCFG(Succ, Condition, false);
            return;
          }
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
        if (I->isReachable()) {
          dfsTraverseCFG(*I, Cond, true);
          Parent = ID;
        }
        if (Blk->succ_size() == 2) {
          ++I;
          if (I->isReachable()) {
            dfsTraverseCFG(*I, Cond, false);
            Parent = ID;
          }
        }
      } else {
        if (I->isReachable()) {
          dfsTraverseCFG(*I, nullptr, false);
          Parent = ID;
        }
      }
      break;
    }
    case Stmt::SwitchStmtClass: {
      CFGBlock *DefaultBlk = nullptr;
      Stmt *InnerCond = Blk->getTerminatorCondition();
      vector<Stmt *> Cases;
      for (auto I : Blk->succs()) {
        if (!I.isReachable())
          continue;
        Stmt *Label = I->getLabel();
        if (!Label) {
          DefaultBlk = I;
          continue;
        }
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
      if (Blk->succ_begin()->isReachable()) {
        dfsTraverseCFG(*Blk->succ_begin(), nullptr, false);
        Parent = ID;
      }
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
        if (I->isReachable()) {
          dfsTraverseCFG(*I, Cond, true);
          Parent = ID;
        }
        if (Blk->succ_size() == 2) {
          ++I;
          if (I->isReachable()) {
            dfsTraverseCFG(*I, Cond, false);
            Parent = ID;
          }
        }
      } else {
        if (I->isReachable()) {
          dfsTraverseCFG(*I, nullptr, false);
          Parent = ID;
        }
      }
      break;
    }
    case Stmt::ContinueStmtClass: {
      if (Blk->succ_begin()->isReachable()) {
        dfsTraverseCFG(*Blk->succ_begin(), nullptr, false);
        Parent = ID;
      }
      break;
    }
    }
  } else if (Blk->succ_size() == 1) {
    if (Blk->succ_begin()->isReachable()) {
      dfsTraverseCFG(*Blk->succ_begin(), nullptr, false);
      Parent = ID;
    }
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
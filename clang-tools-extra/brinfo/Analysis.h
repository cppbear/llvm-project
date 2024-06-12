#include "CondChain.h"
#include <unordered_set>

namespace BrInfo {

enum AnalysisType { FILE, FUNC };

const unsigned MaxChains = 1000;

struct BlkCond {
  long Parent = -1;
  const CFGBlock *Block = nullptr;
  BaseCond *Condition = nullptr;
  bool Flag = false;
};

class Analysis {

  using CondChainList = vector<CondChainInfo>;

private:
  CFG *Cfg;
  ASTContext *Context;
  const FunctionDecl *FocalFunc;
  AnalysisType Type;
  string Signature;
  json Results;
  unordered_set<string> VisitedFuncs;

  // vector<CondChainList> BlkChain;
  CondChainList CondChainForBlk;
  CondChainList CondChains;
  vector<unsigned char> ColorOfBlk;
  // long Parent;
  // bool DeathLoop = false;

  unordered_map<unsigned, unordered_set<unsigned>> LoopInner;

  void setSignature();
  void extractCondChains();
  long findBestCover(set<pair<const Stmt *, bool>> &Uncovered,
                     const CondChainList &CondChains, vector<bool> &Used);
  unordered_set<unsigned> findMinCover();
  void condChainsToReqs();
  void clear();

  void toBlack();
  void dfsTraverseCFGLoop(long Parent, CFGBlock *FirstBlk);
  // void dfsTraverseCFG(CFGBlock *Blk, BaseCond *Condition, bool Flag);
  void dumpCondChains();
  void dumpBlkChain(unsigned ID);
  void dumpBlkChain();

public:
  void setType(AnalysisType T);
  bool init(CFG *CFG, ASTContext *Context, const FunctionDecl *FD);
  string getSignature() { return Signature; }
  void analyze();
  void dumpReqToJson(string ProjectPath, string FileName, string ClassName,
                     string FuncName);
};

} // namespace BrInfo

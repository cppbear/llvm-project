#include "CondChain.h"
#include <unordered_set>

namespace BrInfo {

enum AnalysisType { FILE, FUNC };

class Analysis {

  using CondChainList = vector<CondChainInfo>;

private:
  CFG *Cfg;
  ASTContext *Context;
  const FunctionDecl *FuncDecl;
  AnalysisType Type;
  string Signature;
  json Results;

  vector<CondChainList> BlkChain;
  vector<unsigned char> ColorOfBlk;
  long Parent;

  void setSignature();
  void extractCondChains();
  void condChainsToReqs();
  void clear();

  void toBlack();
  void dfsTraverseCFG(CFGBlock *Blk, BaseCond *Condition, bool Flag);
  void dumpCondChains();
  void dumpBlkChain(unsigned ID);
  void dumpBlkChain();

public:
  void setType(AnalysisType T);
  void init(CFG *CFG, ASTContext *Context, const FunctionDecl *FD);
  string getSignature() { return Signature; }
  void analyze();
  void dumpReqToJson(string ProjectPath, string FileName, string ClassName,
                     string FuncName);
};

} // namespace BrInfo

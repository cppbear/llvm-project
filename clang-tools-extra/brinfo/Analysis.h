#include "CondChain.h"
#include <unordered_set>

using namespace clang;
using namespace llvm;

namespace BrInfo {

enum AnalysisType { FILE, FUNC };

class Analysis {

  using CondChainList = std::vector<CondChainInfo>;

private:
  CFG *Cfg;
  ASTContext *Context;
  const FunctionDecl *FuncDecl;
  AnalysisType Type;
  std::string Signature;
  json Results;

  std::vector<CondChainList> BlkChain;
  std::vector<unsigned char> ColorOfBlk;
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
  void analyze();
  void dumpReqToJson(std::string ProjectPath, std::string FileName,
                     std::string ClassName, std::string FuncName);
};

} // namespace BrInfo

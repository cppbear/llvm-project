#include "CondChain.h"
#include "nlohmann/json.hpp"
#include <map>
#include <memory>
#include <set>

using namespace clang;
using namespace llvm;
using json = nlohmann::json;

namespace BrInfo {

enum Type { FILE, FUNC };

class Analysis {

  using CondChainList = std::vector<CondChainInfo>;

private:
  CFG *Cfg;
  ASTContext *Context;
  const FunctionDecl *FuncDecl;
  Type AnalysisType;
  std::string Signature;
  json Results;

  std::vector<CondChainList> BlkChain;
  long Parent;
  std::map<const Stmt *, bool> CondMap;
  std::pair<BaseCond *, bool> TmpCond;

  // std::vector<LastDefInfo> LastDefList;
  std::set<unsigned> ContraChains;

  void dfs(CFGBlock *Blk, BaseCond *Condition, bool Flag);
  void dumpBlkChain();
  void dumpBlkChain(unsigned ID);
  std::vector<std::string>
  getLastDefStrVec(std::set<const Stmt *> &TraceBacks);

  void setSignature();
  void getCondChains();
  // void dumpCondChains();
  void dumpCondChain(unsigned ID);


  void setRequire();
  // void findContraInLastDef(CondChainInfo &ChainInfo);
  void clear();

public:
  Analysis() {}
  // Analysis(CFG *CFG, ASTContext *Context)
  //     : Cfg(CFG), Context(Context) {
  //   BlkChain.resize(Cfg->getNumBlockIDs());
  //   Parent = -1;
  //   BlkChain[Cfg->getEntry().getBlockID()].push_back(
  //       {{{nullptr, false, {}, {}}}, {&Cfg->getEntry()}});
  // }
  ~Analysis() {}
  void setType(Type T);
  void init(CFG *CFG, ASTContext *Context, const FunctionDecl *FD);
  void analyze();
  void dumpResults(std::string ProjectPath, std::string FileName,
                   std::string ClassName, std::string FuncName);
};

} // namespace BrInfo

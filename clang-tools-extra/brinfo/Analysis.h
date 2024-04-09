#include "Condition.h"
#include <map>
#include <memory>
#include <set>

using namespace clang;
using namespace llvm;

namespace BrInfo {

class Analysis {

  struct CondStatus {
    BaseCond *Condition = nullptr;
    bool Flag = false;
    std::vector<const Stmt *> TraceBacks;
  };

  using CondChain = std::vector<CondStatus>; // A chain of conditions
  using Path = std::vector<CFGBlock *>;      // A path of basic blocks
  using CondChains = std::vector<std::pair<CondChain, Path>>;
  using CallReturnInfo = std::map<const CallExpr *, std::map<std::string, bool>>;

  std::unique_ptr<CFG> &Cfg;
  ASTContext &Context;

  std::vector<CondChains> BlkChain;
  long Parent;
  std::map<const Stmt *, bool> CondMap;
  std::pair<BaseCond *, bool> TmpCond;
  // std::vector<std::vector<std::vector<const Stmt *>>> TraceBacks;
  // std::map<const Stmt *, std::map<std::string, bool>> CallReturn;
  std::vector<CallReturnInfo> CallReturns;
  std::set<unsigned> ContraChains;

  void dfs(CFGBlock *Blk, BaseCond *Condition, bool Flag);
  void dumpBlkChain();
  void dumpBlkChain(unsigned ID);
  // void dumpTraceBack(unsigned CondChain, unsigned Cond);
  std::string getTraceBackStr(std::vector<const Stmt *> &TraceBacks);
  void simplify(const BinaryOperator *BO, bool Flag);
  void deriveCond(bool Flag, BinaryOperator::Opcode Opcode, const Expr *Known,
                  const Expr *Unknown);
  bool transferCond(const Expr *Expr);

public:
  Analysis(std::unique_ptr<CFG> &CFG, ASTContext &Context)
      : Cfg(CFG), Context(Context) {
    BlkChain.resize(Cfg->getNumBlockIDs());
    Parent = -1;
    BlkChain[Cfg->getEntry().getBlockID()].push_back(
        {{{nullptr, false, {}}}, {&Cfg->getEntry()}});
  }
  ~Analysis() {}
  void getCondChains();
  // void dumpCondChains();
  void simplifyConds();
  void traceBack();
  const Stmt *handleDeclRefExpr(const DeclRefExpr *DeclRef, Path &Path,
                                unsigned Loc);
  void dumpRequirements();
  void findCallReturn();
  void getCallReturnInfo(CallReturnInfo &Info, CondStatus &Cond,
                         const CallExpr *CE, unsigned CondChainID);
};

} // namespace BrInfo

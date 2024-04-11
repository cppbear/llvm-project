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
    std::vector<const Stmt *> LastDefStmts;
    std::vector<const ParmVarDecl *> ParmVars;
  };

  struct LastDefInfo {
    using DefInfoMap = std::map<const Stmt *, std::map<std::string, bool>>;
    using ParmInfoMap = std::map<const ParmVarDecl *, std::map<std::string, bool>>;
    std::map<std::string, DefInfoMap> FuncCall;
    DefInfoMap NonFuncCall;
    ParmInfoMap ParmVar;
  };

  using CondChain = std::vector<CondStatus>; // A chain of conditions
  using Path = std::vector<CFGBlock *>;      // A path of basic blocks
  using CondChains = std::vector<std::pair<CondChain, Path>>;
  // using CallReturnInfo =
  //     std::map<const CallExpr *, std::map<std::string, bool>>;

  std::unique_ptr<CFG> &Cfg;
  ASTContext &Context;

  std::vector<CondChains> BlkChain;
  long Parent;
  std::map<const Stmt *, bool> CondMap;
  std::pair<BaseCond *, bool> TmpCond;
  // std::vector<std::vector<std::vector<const Stmt *>>> TraceBacks;
  // std::map<const Stmt *, std::map<std::string, bool>> CallReturn;
  // std::vector<CallReturnInfo> CallReturns;
  std::vector<LastDefInfo> LastDefList;
  std::set<unsigned> ContraChains;

  void dfs(CFGBlock *Blk, BaseCond *Condition, bool Flag);
  void dumpBlkChain();
  void dumpBlkChain(unsigned ID);
  // void dumpTraceBack(unsigned CondChain, unsigned Cond);
  std::vector<std::string> getLastDefStrVec(std::vector<const Stmt *> &TraceBacks);
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
        {{{nullptr, false, {}, {}}}, {&Cfg->getEntry()}});
  }
  ~Analysis() {}
  void getCondChains();
  // void dumpCondChains();
  void simplifyConds();
  void traceBack();
  bool checkLiteralExpr(const Expr *Expr, bool IsNot, bool Flag);
  bool examineLastDef(const DeclRefExpr *DeclRef, const Stmt *LastDefStmt,
                        bool IsNot, bool Flag);
  const Stmt *findLastDefStmt(const DeclRefExpr *DeclRef, Path &Path,
                                unsigned Loc);
  void dumpRequirements(std::string ClassName);
  void findContraInLastDef();
  void setNonFuncCallInfo(LastDefInfo &Info, CondStatus &Cond, const Stmt *S,
                          unsigned CondChainID);
  void setFuncCallInfo(LastDefInfo &Info, CondStatus &Cond, const CallExpr *CE,
                       unsigned CondChainID);
  void setParmVarInfo(LastDefInfo &Info, CondStatus &Cond, const ParmVarDecl *PVD,
                          unsigned CondChainID);
};

} // namespace BrInfo

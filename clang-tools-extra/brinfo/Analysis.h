#include "Condition.h"
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

  struct CondStatus {
    BaseCond *Condition = nullptr;
    bool Flag = false;
    std::set<const Stmt *> LastDefStmts;
    std::set<const ParmVarDecl *> ParmVars;

    // bool operator==(const CondStatus &RHS) const {
    //   if (!Condition || !RHS.Condition)
    //     return Condition == RHS.Condition;
    //   if (Condition->getCondStr() == RHS.Condition->getCondStr() &&
    //       Flag == RHS.Flag && LastDefStmts == RHS.LastDefStmts &&
    //       ParmVars == RHS.ParmVars)
    //     return true;
    //   return false;
    // }

    bool operator<(const CondStatus &RHS) const {
      if (!Condition || !RHS.Condition)
        return Condition < RHS.Condition;
      if (Condition->getCondStr() == RHS.Condition->getCondStr()) {
        bool Tmp = Flag;
        bool RHSTmp = RHS.Flag;
        if (Condition->isNot())
          Tmp = !Tmp;
        if (RHS.Condition->isNot())
          RHSTmp = !RHSTmp;
        if (Tmp == RHSTmp) {
          if (LastDefStmts == RHS.LastDefStmts)
            return ParmVars < RHS.ParmVars;
          return LastDefStmts < RHS.LastDefStmts;
        }
        return Tmp < RHSTmp;
      }
      return Condition->getCondStr() < RHS.Condition->getCondStr();
    }
  };

  struct LastDefInfo {
    using DefInfoMap = std::map<const Stmt *, std::map<std::string, bool>>;
    using ParmInfoMap =
        std::map<const ParmVarDecl *, std::map<std::string, bool>>;
    std::map<std::string, DefInfoMap> FuncCall;
    DefInfoMap NonFuncCall;
    ParmInfoMap ParmVar;
  };

  struct CallExprInfo {
    const CallExpr *CE;
    unsigned Order;
    std::map<std::string, bool> CondInfos;

    bool operator<(const CallExprInfo &RHS) const { return Order < RHS.Order; }
  };

  using CondChain = std::vector<CondStatus>; // A chain of conditions
  using Path = std::vector<CFGBlock *>;      // A path of basic blocks
  using CondChains = std::vector<std::pair<CondChain, Path>>;

  CFG *Cfg;
  ASTContext *Context;
  const FunctionDecl *FuncDecl;
  Type AnalysisType;
  std::string Signature;
  json Results;

  std::vector<CondChains> BlkChain;
  long Parent;
  std::map<const Stmt *, bool> CondMap;
  std::pair<BaseCond *, bool> TmpCond;

  std::vector<LastDefInfo> LastDefList;
  std::map<const FunctionDecl *, std::vector<CallExprInfo>> CallExprList;
  std::set<unsigned> ContraChains;

  void dfs(CFGBlock *Blk, BaseCond *Condition, bool Flag);
  void dumpBlkChain();
  void dumpBlkChain(unsigned ID);
  void dumpTraceBack(CondStatus &Cond);
  std::vector<std::string> getLastDefStrVec(std::set<const Stmt *> &TraceBacks);
  void simplify(const BinaryOperator *BO, bool Flag);
  void deriveCond(bool Flag, BinaryOperator::Opcode Opcode, const Expr *Known,
                  const Expr *Unknown);
  bool transferCond(const Expr *Expr);
  void setSignature();
  void getCondChains();
  // void dumpCondChains();
  void dumpCondChain(unsigned ID);
  void simplifyConds();
  void traceBack();
  bool checkLiteralExpr(const Expr *Expr, bool IsNot, bool Flag);
  bool examineLastDef(const DeclRefExpr *DeclRef, const Stmt *LastDefStmt,
                      bool IsNot, bool Flag);
  const Stmt *findLastDefStmt(const DeclRefExpr *DeclRef, Path &Path,
                              unsigned Loc);
  void setRequire();
  void findContraInLastDef();
  bool setNonFuncCallInfo(LastDefInfo &Info, CondStatus &Cond, const Stmt *S,
                          unsigned CondChainID);
  bool setFuncCallInfo(LastDefInfo &Info, CondStatus &Cond, const CallExpr *CE,
                       unsigned CondChainID);
  bool setParmVarInfo(LastDefInfo &Info, CondStatus &Cond,
                      const ParmVarDecl *PVD, unsigned CondChainID);
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

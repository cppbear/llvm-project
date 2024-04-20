#include "Condition.h"
#include "nlohmann/json.hpp"
#include <set>

using namespace clang;
using namespace llvm;
using json = nlohmann::json;

namespace BrInfo {

using StringList = std::vector<std::string>;

struct CondStatus {
  BaseCond *Condition = nullptr;
  bool Flag = false;
  std::set<const Stmt *> LastDefStmts;
  std::set<const ParmVarDecl *> ParmVars;

  //   bool operator==(const CondStatus &RHS) const {
  //     if (!Condition || !RHS.Condition)
  //       return Condition == RHS.Condition;
  //     if (Condition->getCondStr() == RHS.Condition->getCondStr() &&
  //         Flag == RHS.Flag && LastDefStmts == RHS.LastDefStmts &&
  //         ParmVars == RHS.ParmVars)
  //       return true;
  //     return false;
  //   }

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

  std::string toString();
  StringList getLastDefStrVec(ASTContext *Context);

  void dump(ASTContext *Context);
};

using CondChain = std::vector<CondStatus>; // A chain of conditions
using BlkPath = std::vector<CFGBlock *>;   // A path of basic blocks

// last definition information for a condition chain
struct LastDefInfo {
  using DefInfoMap =
      std::unordered_map<const Stmt *, std::map<std::string, bool>>;
  using ParmInfoMap =
      std::unordered_map<const ParmVarDecl *, std::map<std::string, bool>>;
  DefInfoMap DefInfo;
  ParmInfoMap ParmInfo;
};

// related conditions about a CallExpr
struct CallExprInfo {
  const CallExpr *CE = nullptr;
  std::unordered_map<std::string, bool> CondInfos;
};

struct CondChainInfo {
  CondChain Chain;
  BlkPath Path;
  bool IsContra = false;
  LastDefInfo LastDefInfo; // last definition information in a condition chain
  std::unordered_map<const FunctionDecl *, std::vector<CallExprInfo>>
      FuncCallInfo; // function call information in a condition chain

  void analyze(ASTContext *Context);
  void simplifyConds();
  void setCondStr(ASTContext *Context);
  void findCallExprs();
  void traceBack();
  void findContra();

  bool setFuncCallInfo(CondStatus &Cond, const CallExpr *CE);
  bool setDefInfo(CondStatus &Cond, const Stmt *S);
  bool setParmInfo(CondStatus &Cond, const ParmVarDecl *PVD);

  const Stmt *findLastDefStmt(const DeclRefExpr *DeclRef, unsigned Loc);
  bool examineLastDef(const DeclRefExpr *DeclRef, const Stmt *LastDefStmt,
                      CondStatus &CondStatus);

  void dumpFuncCallInfo();
  void dump(ASTContext *Context, unsigned Indent = 0);
  json toTestReqs(ASTContext *Context);
  std::string getReturnStr(ASTContext *Context, std::string ReturnType);
};

struct CondSimp {
  std::unordered_map<const Stmt *, bool> Map;
  BaseCond *Cond = nullptr;
  bool Flag = false;
};

} // namespace BrInfo
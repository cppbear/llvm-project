#include "clang/AST/ASTContext.h"
#include "clang/Analysis/CFG.h"
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace clang;
using namespace llvm;

namespace BrInfo {

inline void rtrim(std::string &S) {
  S.erase(std::find_if(
              S.rbegin(), S.rend(),
              [](unsigned char Ch) { return !std::isspace(Ch) && Ch != ';'; })
              .base(),
          S.end());
}

class BaseCond {
protected:
  const Stmt *Cond = nullptr;
  std::string CondStr;
  bool IsNot = false;

public:
  BaseCond(const Stmt *Cond) : Cond(Cond) {}
  virtual ~BaseCond() { Cond = nullptr; }
  virtual void dump(const ASTContext &Context) = 0;
  virtual std::string toString(const ASTContext &Context) = 0;
  const Stmt *getCond() { return Cond; }
  std::string getCondStr() { return CondStr; }
  bool isNot() { return IsNot; }
};

class IfCond : public BaseCond {
public:
  IfCond(const Stmt *Cond, const ASTContext &Context) : BaseCond(Cond) {
    setCondStr(Context);
  }
  virtual ~IfCond() {}
  void dump(const ASTContext &Context) override;
  void setCondStr(const ASTContext &Context);
  std::string toString(const ASTContext &Context) override;
};

class CaseCond : public BaseCond {
  Stmt *Case = nullptr;

public:
  CaseCond(Stmt *Cond, Stmt *Case, const ASTContext &Context)
      : BaseCond(Cond), Case(Case) {
    setCondStr(Context);
  }
  virtual ~CaseCond() { Case = nullptr; }
  void dump(const ASTContext &Context) override;
  void setCondStr(const ASTContext &Context){};
  std::string toString(const ASTContext &Context) override;
};

class DefaultCond : public BaseCond {
  std::vector<Stmt *> Cases;

public:
  DefaultCond(Stmt *Cond, std::vector<Stmt *> Cases, const ASTContext &Context)
      : BaseCond(Cond), Cases(Cases) {
    setCondStr(Context);
  }
  virtual ~DefaultCond() {}
  void dump(const ASTContext &Context) override;
  void setCondStr(const ASTContext &Context){};
  std::string toString(const ASTContext &Context) override;
};

class LoopCond : public BaseCond {
public:
  LoopCond(Stmt *Cond, const ASTContext &Context) : BaseCond(Cond) {
    setCondStr(Context);
  }
  virtual ~LoopCond() {}
  void dump(const ASTContext &Context) override;
  void setCondStr(const ASTContext &Context){};
  std::string toString(const ASTContext &Context) override;
};

class TryCond : public BaseCond {};

class Analysis {

  struct CondStatus {
    BaseCond *Condition = nullptr;
    bool Flag = false;
    std::vector<const Stmt *> TraceBacks;
  };

  using CondChain = std::vector<CondStatus>; // A chain of conditions
  using Path = std::vector<CFGBlock *>;      // A path of basic blocks
  using CondChains = std::vector<std::pair<CondChain, Path>>;
  using CallReturnInfo = std::map<const Stmt *, std::map<std::string, bool>>;

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
};

} // namespace BrInfo

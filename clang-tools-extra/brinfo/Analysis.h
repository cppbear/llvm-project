#include "clang/AST/ASTContext.h"
#include "clang/Analysis/CFG.h"
#include <map>
#include <memory>
#include <vector>

using namespace clang;
using namespace llvm;

namespace BrInfo {

class BaseCond {
protected:
  const Stmt *Cond;

public:
  BaseCond(const Stmt *Cond) : Cond(Cond) {}
  virtual ~BaseCond() { Cond = nullptr; }
  virtual void dump(const ASTContext &Context) = 0;
  const Stmt *getCond() { return Cond; }
};

class IfCond : public BaseCond {
public:
  IfCond(const Stmt *Cond) : BaseCond(Cond) {}
  virtual ~IfCond() {}
  void dump(const ASTContext &Context) override;
};

class CaseCond : public BaseCond {
  Stmt *Case;

public:
  CaseCond(Stmt *Cond, Stmt *Case) : BaseCond(Cond), Case(Case) {}
  virtual ~CaseCond() { Case = nullptr; }
  void dump(const ASTContext &Context) override;
};

class DefaultCond : public BaseCond {
  std::vector<Stmt *> Cases;

public:
  DefaultCond(Stmt *Cond, std::vector<Stmt *> Cases)
      : BaseCond(Cond), Cases(Cases) {}
  virtual ~DefaultCond() {}
  void dump(const ASTContext &Context) override;
};

class LoopCond : public BaseCond {
public:
  LoopCond(Stmt *Cond) : BaseCond(Cond) {}
  virtual ~LoopCond() {}
  void dump(const ASTContext &Context) override;
};

class TryCond : public BaseCond {};

using CondChain =
    std::vector<std::pair<BaseCond *, bool>>; // A chain of conditions
using Path = std::vector<CFGBlock *>;         // A path of basic blocks
using CondChains = std::vector<std::pair<CondChain, Path>>;

class Analysis {
  std::unique_ptr<CFG> &Cfg;
  ASTContext &Context;

  std::vector<CondChains> BlkChain;
  long Parent;
  std::map<const Stmt *, bool> CondMap;
  std::pair<BaseCond *, bool> TmpCond;
  std::vector<std::vector<std::vector<const Stmt *>>> TraceBacks;

  void dfs(CFGBlock *Blk, BaseCond *Condition, bool Flag);
  void dumpBlkChain();
  void dumpBlkChain(unsigned ID);
  void dumpTraceBack(unsigned CondChain, unsigned Cond);
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
        {{{nullptr, false}}, {&Cfg->getEntry()}});
  }
  ~Analysis() {}
  void getCondChains();
  void dumpCondChains();
  void simplifyConds();
  void traceBack();
  const Stmt *handleDeclRefExpr(const DeclRefExpr *DeclRef, Path &Path,
                                unsigned Loc);
};

} // namespace BrInfo

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Stmt.h"
#include "clang/Analysis/CFG.h"
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
  bool ContainDeclRefExpr = false;
  bool ContainCallExpr = false;
  std::vector<const DeclRefExpr *> DeclRefExprList;
  std::vector<const CallExpr *> CallExprList;
  void findDeclRefExpr(const Stmt *S);
  void findCallExpr(const Stmt *S);
  void setCondStr(const ASTContext &Context);

public:
  BaseCond(const Stmt *Cond, const ASTContext &Context) : Cond(Cond) {
    findDeclRefExpr(Cond);
    findCallExpr(Cond);
    setCondStr(Context);
  }
  virtual ~BaseCond() { Cond = nullptr; }
  virtual void dump(const ASTContext &Context) = 0;
  virtual std::string toString(const ASTContext &Context) = 0;
  const Stmt *getCond() { return Cond; }
  std::string getCondStr() { return CondStr; }
  bool isNot() { return IsNot; }
  bool containDeclRefExpr() { return ContainDeclRefExpr; }
  bool containCallExpr() { return ContainCallExpr; }
  std::vector<const DeclRefExpr *> &getDeclRefExprList() {
    return DeclRefExprList;
  }
  std::vector<const CallExpr *> &getCallExprList() { return CallExprList; }
};

class IfCond : public BaseCond {
public:
  IfCond(const Stmt *Cond, const ASTContext &Context)
      : BaseCond(Cond, Context) {
    // setCondStr(Context);
  }
  virtual ~IfCond() {}
  void dump(const ASTContext &Context) override;
//   void setCondStr(const ASTContext &Context);
  std::string toString(const ASTContext &Context) override;
};

class CaseCond : public BaseCond {
  Stmt *Case = nullptr;

public:
  CaseCond(Stmt *Cond, Stmt *Case, const ASTContext &Context)
      : BaseCond(Cond, Context), Case(Case) {
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
      : BaseCond(Cond, Context), Cases(Cases) {
    setCondStr(Context);
  }
  virtual ~DefaultCond() {}
  void dump(const ASTContext &Context) override;
  void setCondStr(const ASTContext &Context){};
  std::string toString(const ASTContext &Context) override;
};

class LoopCond : public BaseCond {
public:
  LoopCond(Stmt *Cond, const ASTContext &Context) : BaseCond(Cond, Context) {
    setCondStr(Context);
  }
  virtual ~LoopCond() {}
  void dump(const ASTContext &Context) override;
  void setCondStr(const ASTContext &Context){};
  std::string toString(const ASTContext &Context) override;
};

class TryCond : public BaseCond {};

} // namespace BrInfo

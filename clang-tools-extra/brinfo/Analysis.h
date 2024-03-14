#include "clang/AST/ASTContext.h"
#include "clang/Analysis/CFG.h"
#include <memory>
#include <set>
#include <utility>
#include <vector>

using namespace clang;
using namespace llvm;

namespace BrInfo {

using CondChain = std::vector<std::pair<Stmt *, bool>>; // A chain of conditions
using Path = std::vector<unsigned>;                     // A path of block IDs
using CondChains = std::set<std::pair<CondChain, Path>>;

class Analysis {
  std::unique_ptr<CFG> &Cfg;
  ASTContext &Context;

  std::vector<CondChains> BlkChain;
  long Parent;

  void dfs(CFGBlock Blk, Stmt *Condition, bool Flag);
  void dumpBlkChain();

public:
  Analysis(std::unique_ptr<CFG> &CFG, ASTContext &Context)
      : Cfg(CFG), Context(Context) {
    BlkChain.resize(Cfg->getNumBlockIDs());
    Parent = -1;
    BlkChain[Cfg->getEntry().getBlockID()].insert(
        {{{nullptr, false}}, {Cfg->getEntry().getBlockID()}});
  }
  ~Analysis() {}
  void getCondChain();
  void dumpCondChain();
};

} // namespace BrInfo

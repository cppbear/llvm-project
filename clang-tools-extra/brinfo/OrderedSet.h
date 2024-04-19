#include <unordered_set>
#include <vector>

namespace BrInfo {

template <typename T> class OrderedSet {
  std::vector<T> Vec;
  std::unordered_set<T> Set;

public:
  void insert(const T &Value) {
    if (Set.find(Value) != Set.end()) {
      return;
    }
    Vec.push_back(Value);
    Set.insert(Value);
  }

  bool contains(const T &Value) const { return Set.find(Value) != Set.end(); }

  const std::vector<T> &orderedElements() const { return Vec; }

  void merge(const OrderedSet<T> &Other) {
    for (const T &Value : Other.orderedElements()) {
      insert(Value);
    }
  }

  bool empty() const { return Vec.empty(); }
};
} // namespace BrInfo
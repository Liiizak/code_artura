#include <algorithm>
#include <iostream>
#include <optional>
#include <random>
#include <vector>

template <typename T>
class BinomialTree {
 public:
  struct Node {
    void Print() const {
      std::cout << value << " ";
      for (auto child : children_) {
        child->Print();
      }
    }
    Node(const T& value) : value(value) {}
    T value;
    std::vector<Node*> children_;
  };

  explicit BinomialTree<T>(const T& value) {
    root_ = new Node(value);
  }

  int GetSize() const {
    return 1 << rank_;
  }

  explicit BinomialTree<T>(Node* root) : root_(root) {}

  std::vector<BinomialTree<T>> GetChildrenAsTrees() {
    std::vector<BinomialTree<T>> child_trees;
    int rank = 0;
    for (Node* child_root : root_->children_) {
      child_trees.emplace_back(child_root);
      child_trees.back().SetRank(rank++);
    }
    return child_trees;
  }

  int GetRank() const {
    return rank_;
  }

  void SetRank(int new_rank) {
    rank_ = new_rank;
  }

  T GetRootValue() const {
    return root_->value;
  }

  Node* GetRoot() {
    return root_;
  }

  void RemoveRoot() {
    root_ = nullptr;
  }

  void MergeOtherTree(BinomialTree<T>& tree) {
    root_->children_.push_back(tree.GetRoot());
    ++rank_;
    tree.RemoveRoot();
  }

  void Print() const {
    std::cout << "Tree rank: " << rank_ << "\n";
    root_->Print();
    std::cout << "\n";
  }

 private:
  Node* root_ = nullptr;
  int rank_ = 0;
};

template <typename T>
BinomialTree<T> MergeBinomialTrees(BinomialTree<T>& left, BinomialTree<T>& right) {
  if (left.GetRootValue() < right.GetRootValue()) {
    left.MergeOtherTree(right);
    return left;
  } else {
    right.MergeOtherTree(left);
    return right;
  }
}

template <typename T>
class BinomialHeap {
 public:
  BinomialHeap() = default;

  explicit BinomialHeap<T>(const BinomialTree<T>& tree) {
    trees_.push_back(tree);
    size_ = 1;
  }

  void MergeOtherHeap(const BinomialHeap<T>& other) {
    if (trees_.empty()) {
      trees_ = other.GetTrees();
      size_ = other.size_;
      return;
    }
    if (other.trees_.empty()) {
      return;
    }
    std::optional<BinomialTree<T>> transfer;
    auto our_trees = GetTrees();
    auto other_trees = other.GetTrees();
    trees_.clear();
    size_ += other.size_;
    if (other_trees.size() > our_trees.size()) {
      std::swap(other_trees, our_trees);
    }
    trees_.resize(our_trees.size());
    for (int i = 0; i < other_trees.size(); ++i) {
      if (!transfer.has_value()) {
        if (our_trees[i].has_value() && !other_trees[i].has_value()) {
          trees_[i] = our_trees[i];
        } else if (!our_trees[i].has_value() && other_trees[i].has_value()) {
          trees_[i] = other_trees[i];
        } else if (our_trees[i].has_value() && other_trees[i].has_value()) {
          transfer = MergeBinomialTrees(our_trees[i].value(), other_trees[i].value());
          trees_[i] = std::nullopt;
        } else {
          trees_[i] = std::nullopt;
        }
      } else {
        if (our_trees[i].has_value() && !other_trees[i].has_value()) {
          trees_[i] = std::nullopt;
          transfer = MergeBinomialTrees(transfer.value(), our_trees[i].value());
        } else if (!our_trees[i].has_value() && other_trees[i].has_value()) {
          trees_[i] = std::nullopt;
          transfer = MergeBinomialTrees(transfer.value(), other_trees[i].value());
        } else if (our_trees[i].has_value() && other_trees[i].has_value()) {
          trees_[i] = transfer.value();
          our_trees[i] = MergeBinomialTrees(our_trees[i].value(), other_trees[i].value());
          transfer = our_trees[i];
        } else {
          trees_[i] = transfer.value();
          transfer = std::nullopt;
        }
      }
    }
    for (int i = other_trees.size(); i < our_trees.size(); ++i) {
      if (transfer.has_value()) {
        if (our_trees[i].has_value()) {
          transfer = MergeBinomialTrees(transfer.value(), our_trees[i].value());
        } else {
          trees_[i] = transfer.value();
          transfer = std::nullopt;
        }
      } else {
        if (our_trees[i].has_value()) {
          trees_[i] = our_trees[i].value();
        }
      }
    }
    if (transfer.has_value()) {
      trees_.push_back(transfer.value());
    }
  }

  void Insert(const T& value) {
    MergeOtherHeap(BinomialHeap<T>(BinomialTree<T>(value)));
  }

  T ExtractMin() {
    if (size_ == 0) {
      throw "Try to extract from empty heap!";
    }
    auto& min_tree = trees_[GetMinIndex()];
    T min_value = min_tree->GetRootValue();
    auto child_trees = min_tree->GetChildrenAsTrees();
    auto new_heap = BinomialHeap<T>(child_trees);
    size_ -= min_tree->GetSize();
    if (min_tree->GetSize() > size_) {
      trees_.pop_back();
    } else {
      min_tree = std::nullopt;
    }
    MergeOtherHeap(new_heap);
    return min_value;
  }

  void Print() const {
    std::cout << "Heap size: " << size_ << "\n";
    for (const auto& tree: trees_) {
      if (tree.has_value()) {
        tree->Print();
      }
    }
  }

 private:
  int GetMinIndex() const {
    int min_index = -1;
    T min_value;
    for (int i = 0; i < trees_.size(); ++i) {
      if (!trees_[i].has_value()) {
        continue;
      }
      if (min_index == -1) {
        min_index = i;
        min_value = trees_[i]->GetRootValue();
      } else if (trees_[i]->GetRootValue() < min_value) {
        min_index = i;
        min_value = trees_[i]->GetRootValue();
      }
    }
    return min_index;
  }

  std::vector<std::optional<BinomialTree<T>>> GetTrees() const {
    return trees_;
  }

  BinomialHeap(const std::vector<BinomialTree<T>> trees) {
    for (const auto& tree : trees) {
      trees_.push_back(tree);
    }
    size_ = (1 << trees_.size()) - 1;
  }

  int size_ = 0;
  std::vector<std::optional<BinomialTree<T>>> trees_;
};

std::vector<int> HeapSort(const std::vector<int>& values) {
  BinomialHeap<int> heap;
  int i = 0;
  for (const auto& elem : values) {
//    std::cout << "Before iteration number " << i++ << ":\n";
//    heap.Print();
//    std::cout << "\n";
    heap.Insert(elem);
  }
  std::vector<int> res;
  for (int i = 0; i < values.size(); ++i) {
//    std::cout << "Before iteration number " << i << ":\n";
//    heap.Print();
//    std::cout << "\n";
    res.push_back(heap.ExtractMin());
  }
  return res;
}

std::vector<int> GenVector(int len, int min_elem, int max_elem) {
  std::vector<int> v(len, 0);
  std::random_device rd;  // a seed source for the random number engine
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(min_elem, max_elem);
  for (int& elem : v) {
    elem = distrib(gen);
  }
  return v;
}

std::vector<int> GenVector(int len, int max_elem) {
  return GenVector(len, -max_elem, max_elem);
}

void Print(const std::vector<int>& v, bool with_size = false) {
  if (with_size) {
    std::cout << v.size() << '\n';
  }
  for (int i = 0; i < v.size(); ++i) {
    std::cout << v[i] << (i + 1 == v.size() ? '\n' : ' ');
  }
}


void Test(int n, int max_elem) {
  auto v = GenVector(n, max_elem);
  auto res = HeapSort(v);
  if (!std::is_sorted(res.begin(), res.end())) {
    std::cout << "Before:\n";
    Print(v);
    std::cout << "After:\n";
    Print(res);
    exit(1);
  }
}

int main(int argc, char* argv[]) {
  Test(10, 10);
  Test(100, 100);
  Test(100000, 1);
  Test(100000, 100);
  Test(100000, 1000);
  Test(100000, 1000000000);
}


#include "../src/internal/RBTree.hpp"
#include <algorithm>
#include <cassert>
#include <vector>
#include <random>

namespace oo_alloc {
namespace internal {
namespace tests {

static void populate_tree(RBTree& tree, std::vector<RBTree::Node *>& alloc_nodes, int cnt) {
  std::mt19937 rng(1337);
  std::uniform_int_distribution<std::size_t> size_dist(16, 1024);

  for (int i = 0; i < cnt; ++i) {
    RBTree::Node* dummy = new RBTree::Node();
    dummy->size = size_dist(rng);

    tree.insert(dummy);
    alloc_nodes.push_back(dummy);
  }
}

static void check_red_property(RBTree::Node* node, RBTree::Node* sentinel) {
  if (node == sentinel)
    return;

  if (node->red) {
    assert(!node->left->red && "Red node has a red left child");
    assert(!node->right->red && "Red node has a red right child");
  }

  check_red_property(node->left, sentinel);
  check_red_property(node->right, sentinel);
}

static int check_black_height(RBTree::Node* node, RBTree::Node* sentinel) {
  if (node == sentinel)
    return 1;

  int left_bh = check_black_height(node->left, sentinel);
  int right_bh = check_black_height(node->right, sentinel);

  assert(left_bh == right_bh && "Paths have different black heights");

  return left_bh + !node->red;
}

void test_rbtree_remove_invariants() {
  RBTree tree;
  std::vector<RBTree::Node*> nodes;

  populate_tree(tree, nodes, 500);

  assert(tree.m_root->red == false);
  check_red_property(tree.m_root, &tree.m_sentinel);
  check_black_height(tree.m_root, &tree.m_sentinel);

  std::mt19937 rng(1337); 
  std::shuffle(nodes.begin(), nodes.end(), rng);

  for (std::size_t i = 0; i < nodes.size(); ++i) {
      RBTree::Node* node_to_remove = nodes[i];
      
      tree.remove(node_to_remove);

      assert(tree.m_root->red == false && "Root became red during removal");
      check_red_property(tree.m_root, &tree.m_sentinel);
      check_black_height(tree.m_root, &tree.m_sentinel);

      delete node_to_remove;
  }

  assert(tree.empty() && "Tree not empty after deleting all nodes");
  assert(tree.m_root == &tree.m_sentinel && "Root should be sentinel when empty");
}

}
}
}

int main() {
  oo_alloc::internal::tests::test_rbtree_remove_invariants();
}

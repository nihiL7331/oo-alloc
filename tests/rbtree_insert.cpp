#include "../src/internal/RBTree.hpp"
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

static void cleanup(std::vector<RBTree::Node *>& alloc_nodes) {
  for (RBTree::Node* node : alloc_nodes)
    delete node;
  alloc_nodes.clear();
}

void test_rbtree_root_black() {
  RBTree tree;
  std::vector<RBTree::Node *> nodes;

  assert(!tree.m_root->red && "Root must be black");
  populate_tree(tree, nodes, 50);
  assert(!tree.m_root->red && "Root must be black");

  cleanup(nodes);
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

void test_rbtree_red_property() {
  RBTree tree;
  std::vector<RBTree::Node*> nodes;
  
  populate_tree(tree, nodes, 1000);
  
  check_red_property(tree.m_root, &tree.m_sentinel);

  cleanup(nodes);
}

static int check_black_height(RBTree::Node* node, RBTree::Node* sentinel) {
  if (node == sentinel)
    return 1;

  int left_bh = check_black_height(node->left, sentinel);
  int right_bh = check_black_height(node->right, sentinel);

  assert(left_bh == right_bh && "Paths have different black heights");

  return left_bh + !node->red;
}

void test_rbtree_black_height() {
  RBTree tree;
  std::vector<RBTree::Node*> nodes;
  
  for (std::size_t i = 1; i <= 100; ++i) {
      RBTree::Node* dummy = new RBTree::Node();
      dummy->size = i * 10;
      tree.insert(dummy);
      nodes.push_back(dummy);
  }

  check_black_height(tree.m_root, &tree.m_sentinel);

  cleanup(nodes);
}

}
}
}

int main() {
  oo_alloc::internal::tests::test_rbtree_root_black();
  oo_alloc::internal::tests::test_rbtree_red_property();
  oo_alloc::internal::tests::test_rbtree_black_height();
}

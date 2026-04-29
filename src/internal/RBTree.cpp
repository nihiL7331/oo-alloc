#include "internal/RBTree.hpp"
#include <cassert>

namespace oo_alloc {
namespace internal {

void RBTree::rotate_l(Node* node) {
  Node* child_r = node->right;

  // 1. place the left child of 'child_r'
  // in the place of 'child_r'
  node->right = child_r->left;
  child_r->left->parent = node;

  // 2. place 'child_r' in the place of 'node'
  child_r->parent = node->parent;

  // if 'node' has no parent, 'child_r' is the new root
  if (node->parent == &m_sentinel)
    m_root = child_r;
  // place child_r accordingly
  else if (node == node->parent->left)
    node->parent->left = child_r;
  else
    node->parent->right = child_r;

  // 3. place 'node' as the left child of 'child_r'
  // (move 'node' bottom-left)
  child_r->left = node;
  node->parent = child_r;
}

// mirrored version of rotate_l
void RBTree::rotate_r(Node* node) {
  Node* child_l = node->left;

  // 1. place the right child of 'child_l'
  // in the place of 'child_l'
  node->left = child_l->right;
  child_l->right->parent = node;

  // 2. place 'child_l' in the place of 'node'
  child_l->parent = node->parent;

  // if 'node' has no parent, 'child_l' is the new root
  if (node->parent == &m_sentinel)
    m_root = child_l;
  // place child_l accordingly
  else if (node == node->parent->right)
    node->parent->right = child_l;
  else
    node->parent->left = child_l;

  // 3. place 'node' as the left child of 'child_l'
  // (move 'node' bottom-right)
  child_l->right = node;
  node->parent = child_l;
}

void RBTree::insert_fix(Node* node) {
  (void)node;
  assert(false && "TODO");
}

void RBTree::delete_fix(Node* node, Node* node_parent) {
  (void)node; (void)node_parent;
  assert(false && "TODO");
}

void RBTree::transplant(Node* node1, Node* node2) {
  (void)node1; (void)node2;
  assert(false && "TODO");
}

void RBTree::insert(Node* node) {
  (void)node;
  assert(false && "TODO");
}

void RBTree::remove(Node* node) {
  (void)node;
  assert(false && "TODO");
}

RBTree::Node* RBTree::find_best(std::size_t req_size) const {
  (void)req_size;
  assert(false && "TODO");
}

}
}

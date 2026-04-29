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

// responsible for "fixing" the tree
// structure after insertion.
// it recolors nodes and performs rotations.
//
// when insert_fix is called, we're shortly
// after inserting a new red node.
// there's no certainty that it has only black children.
//
void RBTree::insert_fix(Node* node) {

}

void RBTree::delete_fix(Node* node, Node* node_parent) {
  (void)node; (void)node_parent;
  assert(false && "TODO");
}

void RBTree::transplant(Node* node1, Node* node2) {
  (void)node1; (void)node2;
  assert(false && "TODO");
}

// the implementation of insert
// for the red-black tree is similar
// to bst, but with some small tweaks
void RBTree::insert(Node* new_node) {
  Node* parent = &m_sentinel;
  Node* current = m_root;

  // 1. traverse down the tree,
  // searching for the target position
  // just like in bst
  while (current != &m_sentinel) {
    parent = current;
    if (new_node->size <= current->size)
      current = current->left;
    else
      current = current->right;
  }

  // 2. place the 'new_node'
  // also, just like in bst
  new_node->parent = parent;
  if (parent == &m_sentinel)
    m_root = new_node;
  else if (new_node->size <= parent->size)
    parent->left = new_node;
  else
    parent->right = new_node;

  // 3. set 'new_node's childern to sentinel
  // this differs from normal bsts
  new_node->left = &m_sentinel;
  new_node->right = &m_sentinel;

  // 3. set the 'new_node'
  // color to red, and "fix" the tree
  new_node->red = true;
  insert_fix(new_node);
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

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
// at this point, the tree violates at most ONE property:
// 1. 'new_node == m_root', violating 
//    'm_root->red = false' rule, 
//    OR
// 2. 'new_node->parent->red', violating 
//    'new_node->red => !new_node->parent->red'
void RBTree::insert_fix(Node* new_node) {
  Node* uncle = &m_sentinel;

  while (new_node->parent->red) {
    Node* parent = new_node->parent;
    Node* grandparent = parent->parent;

    if (parent == grandparent->left) {
      uncle = grandparent->right;

      // case 1: 'uncle' is red
      if (uncle->red) {
        parent->red = false;
        uncle->red = false;
        grandparent->red = true;
        new_node = grandparent;
      } else {
        // case 2: 'uncle' is black,
        //         'new_node' is a right child
        if (new_node == parent->right) {
          new_node = parent;
          rotate_l(new_node);
          // after rotation, 'new_node' is a left child
          // that's why case 2 falls through to case 3
        }

        // case 3: 'uncle' is black,
        //         'new_node' is a left child
        new_node->parent->red = false;
        new_node->parent->parent->red = true;
        rotate_r(new_node->parent->parent);
      } 
    } else {
      uncle = grandparent->left;

      // case 1: 'uncle' is red
      if (uncle->red) {
        parent->red = false;
        uncle->red = false;
        grandparent->red = true;
        new_node = grandparent;
      } else {
        // case 2: 'uncle' is black,
        //         'new_node' is a left child
        if (new_node == parent->left) {
          new_node = parent;
          rotate_r(new_node);
          // after rotation, 'new_node' is a right child
          // that's why case 2 falls through to case 3
        }

        // case 3: 'uncle' is black,
        //         'new_node' is a right child
        new_node->parent->red = false;
        new_node->parent->parent->red = true;
        rotate_l(new_node->parent->parent);
      } 
    }
  }
  m_root->red = false;
}

// this procedure restores the following red-black properties:
// - every node is either red or black,
// - the root is black,
// - if a node is red, then both its children are black.
// (via CLRS)
void RBTree::remove_fix(Node* replacement_node) {
  while (replacement_node != m_root && !replacement_node->red) {
    Node* parent = replacement_node->parent;

    if (replacement_node == parent->left) {
      Node* sibling = parent->right;

      // case 1: 
      // it's main purpose is to restructure
      // the tree in a way, where the properties
      // aren't altered, but 'sibling->red == false'.
      //
      // if 'sibling' is red, it must have black children,
      // so we can make 'sibling' black and 
      // 'parent' red, and perform a rotate_l on 'parent'.
      // the new 'sibling' is black.
      if (sibling->red) {
        sibling->red = false;
        parent->red = true;
        rotate_l(parent);
        sibling = parent->right;
      }

      // case 2:
      // 'sibling' and it's children are black,
      // make the 'sibling' red and re-iterate the loop.
      if (!sibling->left->red && !sibling->right->red) {
        sibling->red = true;
        replacement_node = parent;
      } else {

        // case 3:
        // 'sibling' and 'sibling->right' are black, 
        // 'sibling->left' is red.
        // swap the colors of 'sibling' and 'sibling->left',
        // perform a rotate_r on 'sibling' without violating
        // the properties.
        // it transforms case 3 to case 4.
        if (!sibling->right->red) {
          sibling->left->red = false;
          sibling->red = true;
          rotate_r(sibling);
          sibling = parent->right;
        }

        // case 4:
        // 'sibling' is black, 'sibling->right' is red.
        // we make the following color changes
        // and a rotate_l on 'parent', hence
        // removing the extra black on 'replacement_node'.
        sibling->red = parent->red;
        parent->red = false;
        sibling->right->red = false;
        rotate_l(parent);
        break;
      }
    } else { // mirror image of the first half
      Node* sibling = parent->left;

      if (sibling->red) {
        sibling->red = false;
        parent->red = true;
        rotate_r(parent);
        sibling = parent->left;
      }

      if (!sibling->left->red && !sibling->right->red) {
        sibling->red = true;
        replacement_node = parent;
      } else {
        if(!sibling->left->red) {
          sibling->right->red = false;
          sibling->red = true;
          rotate_l(sibling);
          sibling = parent->left;
        }

        sibling->red = parent->red;
        parent->red = false;
        sibling->left->red = false;
        rotate_r(parent);
        break;
      }
    }
  }

  replacement_node->red = false;
}

// this procedure acts as a helper for the 'delete' function. 
// it leaves `replaced_node` dangling,
// setting up `replacement_node` to take its place.
void RBTree::transplant(Node* replaced_node, Node* replacement_node) {
  // if 'replaced_node' has no parent,
  // 'replacement_node' is the new 'm_root'
  if (replaced_node->parent == &m_sentinel)
    m_root = replacement_node;
  // else place 'replacement_node' in 'replaced_node's position
  else if (replaced_node == replaced_node->parent->left)
    replaced_node->parent->left = replacement_node;
  else
    replaced_node->parent->right = replacement_node;

  // set `replacement_node`s parent
  replacement_node->parent = replaced_node->parent;
}

// Returns the minimum (left-most) node of a sub-tree.
RBTree::Node* RBTree::min(Node* root_node) {
  while (root_node->left != &m_sentinel)
    root_node = root_node->left;
  return root_node;
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

void RBTree::remove(Node* node_to_remove) {
  // 'replaced_node' stores the node to remove
  Node* replaced_node = node_to_remove;
  // 'replaced_node's color might change, so we store it
  bool orig_red = replaced_node->red;
  // `replacement_node` will move into 
  // `replaced_node`s position
  Node* replacement_node;

  // if 'node_to_remove' doesn't have a left child,
  // replace `node_to_remove` with its right child
  if (node_to_remove->left == &m_sentinel) {
    replacement_node = node_to_remove->right;
    transplant(node_to_remove, replacement_node);
  // if `node_to_remove` doesn't have a right child,
  // replace `node_to_remove` with its left child
  } else if (node_to_remove->right == &m_sentinel) {
    replacement_node = node_to_remove->left;
    transplant(node_to_remove, replacement_node);
  // if 'node_to_remove' has both children,
  // get its successor and transplant it to
  // `node_to_remove`s position.
  } else {
    // get 'node_to_remove's successor
    replaced_node = min(node_to_remove->right);
    orig_red = replaced_node->red;
    replacement_node = replaced_node->right;
    // remove successor from its original position
    if (replaced_node->parent == node_to_remove)
      replacement_node->parent = replaced_node;
    else {
      transplant(replaced_node, replaced_node->right);
      replaced_node->right = node_to_remove->right;
      replaced_node->right->parent = replaced_node;
    }
    // place it on the `node_to_remove` position
    transplant(node_to_remove, replaced_node);
    replaced_node->left = node_to_remove->left;
    replaced_node->left->parent = replaced_node;

    // copy `node_to_remove`s color to its replacement
    replaced_node->red = node_to_remove->red;
  }

  // if 'node_to_remove' was black,
  // might have violated red-black properties:
  // 1. possible red adjacent nodes,
  // 2. possible red nodes with red children,
  // 3. possible changes in black-height.
  if (!orig_red)
    remove_fix(replacement_node);
}

RBTree::Node* RBTree::find_best(std::size_t req_size) const {
  (void)req_size;
  assert(false && "TODO");
}

}
}

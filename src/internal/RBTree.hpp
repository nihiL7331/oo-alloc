#pragma once

#include <cstddef>

namespace oo_alloc {
namespace internal {

class RBTree {
public:
  struct Node {
    std::size_t size;
    bool red;
    Node* parent;
    Node* left;
    Node* right;
  };
  
private:
  Node* m_root;
  Node m_sentinel; // implementation concept from ItA

  void rotate_l(Node* node);
  void rotate_r(Node* node);
  void insert_fix(Node* node);
  void delete_fix(Node* node, Node* node_parent);
  void transplant(Node* node1, Node* node2);

public:
  RBTree() : m_root(nullptr) {
    m_sentinel.size = 0;
    m_sentinel.red = false;
    m_sentinel.parent = &m_sentinel;
    m_sentinel.left = &m_sentinel;
    m_sentinel.right = &m_sentinel;
  }

  void insert(Node* node);
  void remove(Node* node);
  Node* find_best(std::size_t req_size) const;

  bool empty() const { return m_root == nullptr; }
};

}
}

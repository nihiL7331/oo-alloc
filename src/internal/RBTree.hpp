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
  Node m_sentinel; // implementation concept from CLRS

  void rotate_l(Node* node) noexcept;
  void rotate_r(Node* node) noexcept;
  void insert_fix(Node* new_node) noexcept;
  void remove_fix(Node* replacement_node) noexcept;
  void transplant(Node* replaced_node, Node* replacement_node) noexcept;
  Node* min(Node* root_node) noexcept;

public:
  RBTree() : m_root(&m_sentinel) {
    m_sentinel.size = 0;
    m_sentinel.red = false;
    m_sentinel.parent = &m_sentinel;
    m_sentinel.left = &m_sentinel;
    m_sentinel.right = &m_sentinel;
  }

  void insert(Node* new_node) noexcept;
  void remove(Node* node_to_remove) noexcept;
  Node* find_best(std::size_t req_size) const noexcept;

  inline bool empty() const noexcept { return m_root == &m_sentinel; }
  inline void clear() noexcept { m_root = &m_sentinel; }
  inline Node* sentinel() noexcept { return &m_sentinel; }
};

}
}

#include "../src/internal/RBTree.hpp"
#include <cassert>

namespace oo_alloc {
namespace internal {
namespace tests {

void test_rbtree_root_black() {
    RBTree tree;
    assert(tree.m_root->red == false && "Invariant violation: Root must be black!");
}

static void check_red_property(RBTree::Node* node, RBTree::Node* sentinel) {
    if (node == sentinel) return;

    if (node->red) {
        assert(node->left->red == false && "Invariant violation: Red node has a red left child!");
        assert(node->right->red == false && "Invariant violation: Red node has a red right child!");
    }

    check_red_property(node->left, sentinel);
    check_red_property(node->right, sentinel);
}

void test_rbtree_red_property() {
    RBTree tree;
    
    check_red_property(tree.m_root, &tree.m_sentinel);
}

static int check_black_height(RBTree::Node* node, RBTree::Node* sentinel) {
    if (node == sentinel) {
        return 1;
    }

    int left_bh = check_black_height(node->left, sentinel);
    int right_bh = check_black_height(node->right, sentinel);

    assert(left_bh == right_bh && "Invariant violation: Paths have different black heights!");

    return left_bh + (node->red ? 0 : 1);
}

void test_rbtree_black_height() {
    RBTree tree;

    check_black_height(tree.m_root, &tree.m_sentinel);
}

}
}
}

int main() {
    oo_alloc::internal::tests::test_rbtree_root_black();
    oo_alloc::internal::tests::test_rbtree_red_property();
    oo_alloc::internal::tests::test_rbtree_black_height();
}

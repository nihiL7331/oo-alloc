#include "oo_alloc/FreeTreeAllocator.hpp"
#include <cassert>

int main() {
  oo_alloc::FreeTreeAllocator tree(16384);

  void* p1 = tree.alloc(128, 8);
  void* p2 = tree.alloc(128, 8);
  void* p3 = tree.alloc(128, 8);
  void* p4 = tree.alloc(128, 8);
  void* barrier = tree.alloc(128, 8); 

  assert(p1 && p2 && p3 && p4 && barrier && "Setup failed");

  tree.free(p2);
  
  void* p2_reclaimed = tree.alloc(128, 8);
  assert(p2_reclaimed == p2 && "Isolated free failed or didn't best-fit back into the same slot");
  
  tree.free(p3);
  tree.free(p2_reclaimed); 

  void* p23_merged = tree.alloc(256, 8);
  assert(p23_merged == p2_reclaimed && "Merge right failed");

  tree.free(p23_merged);
  tree.free(p4);

  void* p234_merged = tree.alloc(384, 8);
  assert(p234_merged == p2_reclaimed && "Merge left failed");

  tree.free(p234_merged);
  tree.free(p1);
  
  p1 = tree.alloc(128, 8);
  p2 = tree.alloc(128, 8);
  p3 = tree.alloc(128, 8);
  p4 = tree.alloc(128, 8);
  
  tree.free(p1);
  tree.free(p3);
  
  tree.free(p2);
  
  void* merge_both_test = tree.alloc(384, 8);
  assert(merge_both_test == p1 && "Merge both failed");
}

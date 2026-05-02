# ARENA:

# TEST 1
add_executable(arena_alphabet arena_alphabet.cpp)
target_link_libraries(arena_alphabet PRIVATE oo_alloc)
add_test(NAME ArenaAlphabet COMMAND arena_alphabet)
set_tests_properties(ArenaAlphabet PROPERTIES LABELS "arena")

# TEST 2
add_executable(arena_align arena_align.cpp)
target_link_libraries(arena_align PRIVATE oo_alloc)
add_test(NAME ArenaAlign COMMAND arena_align)
set_tests_properties(ArenaAlign PROPERTIES LABELS "arena")

# TEST 3
add_executable(arena_realloc arena_realloc.cpp)
target_link_libraries(arena_realloc PRIVATE oo_alloc)
add_test(NAME ArenaRealloc COMMAND arena_realloc)
set_tests_properties(ArenaRealloc PROPERTIES LABELS "arena")

# POOL:

# TEST 1
add_executable(pool_exhaust pool_exhaust.cpp)
target_link_libraries(pool_exhaust PRIVATE oo_alloc)
add_test(NAME PoolExhaust COMMAND pool_exhaust)
set_tests_properties(PoolExhaust PROPERTIES LABELS "pool")

# TEST 2
add_executable(pool_recycle pool_recycle.cpp)
target_link_libraries(pool_recycle PRIVATE oo_alloc)
add_test(NAME PoolRecycle COMMAND pool_recycle)
set_tests_properties(PoolRecycle PROPERTIES LABELS "pool")

# STACK:

# TEST 1
add_executable(stack_align stack_align.cpp)
target_link_libraries(stack_align PRIVATE oo_alloc)
add_test(NAME StackAlign COMMAND stack_align)
set_tests_properties(StackAlign PROPERTIES LABELS "stack")

# TEST 2
add_executable(stack_lifo stack_lifo.cpp)
target_link_libraries(stack_lifo PRIVATE oo_alloc)
add_test(NAME StackLIFO COMMAND stack_lifo)
set_tests_properties(StackLIFO PROPERTIES LABELS "stack")

# TEST 3
add_executable(stack_exhaust stack_exhaust.cpp)
target_link_libraries(stack_exhaust PRIVATE oo_alloc)
add_test(NAME StackExhaust COMMAND stack_exhaust)
set_tests_properties(StackExhaust PROPERTIES LABELS "stack")

# TEST 4
add_executable(stack_clear stack_clear.cpp)
target_link_libraries(stack_clear PRIVATE oo_alloc)
add_test(NAME StackClear COMMAND stack_clear)
set_tests_properties(StackClear PROPERTIES LABELS "stack")

# TEST 5
add_executable(stack_realloc stack_realloc.cpp)
target_link_libraries(stack_realloc PRIVATE oo_alloc)
add_test(NAME StackRealloc COMMAND stack_realloc)
set_tests_properties(StackRealloc PROPERTIES LABELS "stack")

# TRACK:

# TEST 1
add_executable(track_pass track_pass.cpp)
target_link_libraries(track_pass PRIVATE oo_alloc)
add_test(NAME TrackPass COMMAND track_pass)
set_tests_properties(TrackPass PROPERTIES LABELS "track")

# TEST 2
add_executable(track_peak track_peak.cpp)
target_link_libraries(track_peak PRIVATE oo_alloc)
add_test(NAME TrackPeak COMMAND track_peak)
set_tests_properties(TrackPeak PROPERTIES LABELS "track")

# TEST 3
add_executable(track_clear track_clear.cpp)
target_link_libraries(track_clear PRIVATE oo_alloc)
add_test(NAME TrackClear COMMAND track_clear)
set_tests_properties(TrackClear PROPERTIES LABELS "track")

# FREE:

# TEST 1
add_executable(free_ooo free_ooo.cpp)
target_link_libraries(free_ooo PRIVATE oo_alloc)
add_test(NAME FreeOOO COMMAND free_ooo)
set_tests_properties(FreeOOO PROPERTIES LABELS "free")

# TEST 2
add_executable(free_recover free_recover.cpp)
target_link_libraries(free_recover PRIVATE oo_alloc)
add_test(NAME FreeRecover COMMAND free_recover)
set_tests_properties(FreeRecover PROPERTIES LABELS "free")

# TEST 3
add_executable(free_merge free_merge.cpp)
target_link_libraries(free_merge PRIVATE oo_alloc)
add_test(NAME FreeMerge COMMAND free_merge)
set_tests_properties(FreeMerge PROPERTIES LABELS "free")

# TEST 4
add_executable(free_realloc free_realloc.cpp)
target_link_libraries(free_realloc PRIVATE oo_alloc)
add_test(NAME FreeRealloc COMMAND free_realloc)
set_tests_properties(FreeRealloc PROPERTIES LABELS "free")

# RBTREE (data structure):

# TEST 1
add_executable(rbtree_basic rbtree_basic.cpp)
target_link_libraries(rbtree_basic PRIVATE oo_alloc)
add_test(NAME RBTreeBasic COMMAND rbtree_basic)
set_tests_properties(RBTreeBasic PROPERTIES LABELS "rbtree")

# TEST 2
add_executable(rbtree_insert rbtree_insert.cpp)
target_link_libraries(rbtree_insert PRIVATE oo_alloc)
add_test(NAME RBTreeInsert COMMAND rbtree_insert)
set_tests_properties(RBTreeInsert PROPERTIES LABELS "rbtree")

# TEST 3
add_executable(rbtree_remove rbtree_remove.cpp)
target_link_libraries(rbtree_remove PRIVATE oo_alloc)
add_test(NAME RBTreeRemove COMMAND rbtree_remove)
set_tests_properties(RBTreeRemove PROPERTIES LABELS "rbtree")

# TREE

# TEST 1
add_executable(tree_init tree_init.cpp)
target_link_libraries(tree_init PRIVATE oo_alloc)
add_test(NAME TreeInit COMMAND tree_init)
set_tests_properties(TreeInit PROPERTIES LABELS "tree")

# TEST 2
add_executable(tree_alloc tree_alloc.cpp)
target_link_libraries(tree_alloc PRIVATE oo_alloc)
add_test(NAME TreeAlloc COMMAND tree_alloc)
set_tests_properties(TreeAlloc PROPERTIES LABELS "tree")

# TEST 3
add_executable(tree_coal tree_coal.cpp)
target_link_libraries(tree_coal PRIVATE oo_alloc)
add_test(NAME TreeCoal COMMAND tree_coal)
set_tests_properties(TreeCoal PROPERTIES LABELS "tree")

# TEST 4
add_executable(tree_realloc tree_realloc.cpp)
target_link_libraries(tree_realloc PRIVATE oo_alloc)
add_test(NAME TreeRealloc COMMAND tree_realloc)
set_tests_properties(TreeRealloc PROPERTIES LABELS "tree")

# BUDDY

# TEST 1
add_executable(buddy_init_clear buddy_init_clear.cpp)
target_link_libraries(buddy_init_clear PRIVATE oo_alloc)
add_test(NAME BuddyInitClear COMMAND buddy_init_clear)
set_tests_properties(BuddyInitClear PROPERTIES LABELS "buddy")

# TEST 2
add_executable(buddy_alloc buddy_alloc.cpp)
target_link_libraries(buddy_alloc PRIVATE oo_alloc)
add_test(NAME BuddyAlloc COMMAND buddy_alloc)
set_tests_properties(BuddyAlloc PROPERTIES LABELS "buddy")

# TEST 3
add_executable(buddy_free buddy_free.cpp)
target_link_libraries(buddy_free PRIVATE oo_alloc)
add_test(NAME BuddyFree COMMAND buddy_free)
set_tests_properties(BuddyFree PROPERTIES LABELS "buddy")

# TEST 4
add_executable(buddy_realloc buddy_realloc.cpp)
target_link_libraries(buddy_realloc PRIVATE oo_alloc)
add_test(NAME BuddyRealloc COMMAND buddy_realloc)
set_tests_properties(BuddyRealloc PROPERTIES LABELS "buddy")

# SLAB

# TEST 1
add_executable(slab_init slab_init.cpp)
target_link_libraries(slab_init PRIVATE oo_alloc)
add_test(NAME SlabInit COMMAND slab_init)
set_tests_properties(SlabInit PROPERTIES LABELS "slab")

# TARGETS
add_custom_target(test-all
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  DEPENDS 
    arena_alphabet arena_align arena_realloc
    pool_exhaust pool_recycle
    stack_align stack_lifo stack_exhaust stack_clear stack_realloc
    track_pass track_peak track_clear
    free_ooo free_recover free_merge free_realloc
    rbtree_basic rbtree_insert rbtree_remove
    tree_init tree_alloc tree_coal tree_realloc
    buddy_init_clear buddy_alloc buddy_free buddy_realloc
    slab_init
  COMMENT "Running all tests..."
)

add_custom_target(test-arena
  COMMAND ${CMAKE_CTEST_COMMAND} -L arena --output-on-failure
  DEPENDS arena_alphabet arena_align arena_realloc
  COMMENT "Running arena tests..."
)

add_custom_target(test-pool
  COMMAND ${CMAKE_CTEST_COMMAND} -L pool --output-on-failure
  DEPENDS pool_exhaust pool_recycle
  COMMENT "Running pool tests..."
)

add_custom_target(test-stack
  COMMAND ${CMAKE_CTEST_COMMAND} -L stack --output-on-failure
  DEPENDS stack_align stack_lifo stack_exhaust stack_clear stack_realloc
  COMMENT "Running stack tests..."
)

add_custom_target(test-track
  COMMAND ${CMAKE_CTEST_COMMAND} -L track --output-on-failure
  DEPENDS track_pass track_peak track_clear
  COMMENT "Running track tests..."
)

add_custom_target(test-free
  COMMAND ${CMAKE_CTEST_COMMAND} -L free --output-on-failure
  DEPENDS free_ooo free_recover free_merge free_realloc
  COMMENT "Running free-list tests..."
)

add_custom_target(test-rbtree
  COMMAND ${CMAKE_CTEST_COMMAND} -L rbtree --output-on-failure
  DEPENDS rbtree_basic rbtree_insert rbtree_remove
  COMMENT "Running rb-tree tests..."
)

add_custom_target(test-tree
  COMMAND ${CMAKE_CTEST_COMMAND} -L tree --output-on-failure
  DEPENDS tree_init tree_alloc tree_coal tree_realloc
  COMMENT "Running free-tree tests.."
)

add_custom_target(test-buddy
  COMMAND ${CMAKE_CTEST_COMMAND} -L buddy --output-on-failure
  DEPENDS buddy_init_clear buddy_alloc buddy_free buddy_realloc
  COMMENT "Running buddy tests.."
)

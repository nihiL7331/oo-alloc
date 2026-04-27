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

# POOL:

# TEST 1
add_executable(pool_bounce pool_bounce.cpp)
target_link_libraries(pool_bounce PRIVATE oo_alloc)
add_test(NAME PoolBounce COMMAND pool_bounce)
set_tests_properties(PoolBounce PROPERTIES LABELS "pool")

# TEST 2
add_executable(pool_exhaust pool_exhaust.cpp)
target_link_libraries(pool_exhaust PRIVATE oo_alloc)
add_test(NAME PoolExhaust COMMAND pool_exhaust)
set_tests_properties(PoolExhaust PROPERTIES LABELS "pool")

# TEST 3
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

# TARGETS
add_custom_target(test-all
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  DEPENDS arena_alphabet arena_align pool_bounce pool_exhaust pool_recycle
  COMMENT "Running all tests..."
)

add_custom_target(test-arena
  COMMAND ${CMAKE_CTEST_COMMAND} -L arena --output-on-failure
  DEPENDS arena_alphabet arena_align
  COMMENT "Running arena tests..."
)

add_custom_target(test-pool
  COMMAND ${CMAKE_CTEST_COMMAND} -L pool --output-on-failure
  DEPENDS pool_bounce pool_exhaust pool_recycle
  COMMENT "Running pool tests..."
)

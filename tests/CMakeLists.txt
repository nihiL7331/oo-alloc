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

# TARGETS
add_custom_target(test-all
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  COMMENT "Running all tests..."
)

add_custom_target(test-arena
  COMMAND ${CMAKE_CTEST_COMMAND} -L arena --output-on-failure
  COMMENT "Running arena tests..."
)

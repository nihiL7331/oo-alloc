# ARENA:

# TEST 1
add_executable(arena_alphabet arena_alphabet.cpp)
target_link_libraries(arena_alphabet PRIVATE oo_alloc)
add_test(NAME ArenaAlphabet COMMAND arena_alphabet)

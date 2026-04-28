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

# TARGETS
add_custom_target(test-all
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  DEPENDS 
    arena_alphabet arena_align arena_realloc
    pool_bounce pool_exhaust pool_recycle
    stack_align stack_lifo stack_exhaust stack_clear
    track_pass track_peak track_clear
    free_ooo free_recover free_merge
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

add_custom_target(test-stack
  COMMAND ${CMAKE_CTEST_COMMAND} -L stack --output-on-failure
  DEPENDS stack_align stack_lifo stack_exhaust stack_clear
  COMMENT "Running stack tests..."
)

add_custom_target(test-track
  COMMAND ${CMAKE_CTEST_COMMAND} -L track --output-on-failure
  DEPENDS track_pass track_peak track_clear
  COMMENT "Running track tests..."
)

add_custom_target(test-free
  COMMAND ${CMAKE_CTEST_COMMAND} -L free --output-on-failure
  DEPENDS free_ooo free_recover free_merge
  COMMENT "Running free-list tests..."
)

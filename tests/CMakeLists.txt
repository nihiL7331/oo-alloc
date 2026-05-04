if(MSVC)
    add_compile_options("/U_NDEBUG")
else()
    add_compile_options("-UNDEBUG")
endif()

set(ALL_TEST_TARGETS "")

function(add_allocator_suite CATEGORY_NAME)
  foreach(TEST_NAME IN LISTS ARGN)
    add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} PRIVATE oo_alloc)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_tests_properties(${TEST_NAME} PROPERTIES LABELS ${CATEGORY_NAME})
    set(ALL_TEST_TARGETS ${ALL_TEST_TARGETS} ${TEST_NAME} PARENT_SCOPE)
  endforeach()

  add_custom_target(test-${CATEGORY_NAME}
    COMMAND ${CMAKE_CTEST_COMMAND} -L ${CATEGORY_NAME} --output-on-failure
    DEPENDS ${ARGN}
    COMMENT "Running ${CATEGORY_NAME} tests..."
  )
endfunction()

add_allocator_suite(arena  arena_alphabet arena_align arena_realloc)
add_allocator_suite(pool   pool_exhaust pool_recycle)
add_allocator_suite(stack  stack_align stack_lifo stack_exhaust stack_clear stack_realloc)
add_allocator_suite(track  track_pass track_peak track_clear)
add_allocator_suite(free   free_ooo free_recover free_merge free_realloc)
add_allocator_suite(rbtree rbtree_basic rbtree_insert rbtree_remove)
add_allocator_suite(tree   tree_alloc tree_coal tree_realloc)
add_allocator_suite(buddy  buddy_init_clear buddy_alloc buddy_free buddy_realloc)
add_allocator_suite(slab   slab_alloc slab_free slab_realloc slab_clear)

add_custom_target(test-all
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS ${ALL_TEST_TARGETS}
    COMMENT "Running all tests..."
)

function(add_stress_test TEST_NAME)
  add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
  target_link_libraries(${TEST_NAME} PRIVATE oo_alloc)

  target_compile_options(${TEST_NAME} PRIVATE -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(${TEST_NAME} PRIVATE -fsanitize=address,undefined)

  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

  set_tests_properties(${TEST_NAME} PROPERTIES LABELS "stress" TIMEOUT 120)
endfunction()

add_stress_test(stress_chaos)
add_stress_test(stress_silo)
add_stress_test(stress_monotonic)

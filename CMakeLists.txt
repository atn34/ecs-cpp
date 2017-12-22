cmake_minimum_required (VERSION 2.6)
project (turtle)
set (CMAKE_CXX_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR} -g -std=c++11 -Wall -Wextra -Werror -Wshadow -Wnon-virtual-dtor -pedantic")

add_executable(turtle demo.cc)
target_link_libraries(turtle -lsfml-graphics -lsfml-window -lsfml-system)

add_executable(unit_tests movable_pointer_test.cc entity_test.cc circular_interval_test.cc)
target_compile_options(unit_tests PUBLIC -coverage -fsanitize=address -fno-omit-frame-pointer)
target_link_libraries(unit_tests -lasan -lgmock_main -lpthread -coverage)

add_executable(benchmark entity_benchmark.cc)
target_compile_options(benchmark PUBLIC -O2)
target_link_libraries(benchmark -lbenchmark -lpthread)

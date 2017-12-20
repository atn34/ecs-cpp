cmake_minimum_required (VERSION 2.6)
project (turtle)
set (CMAKE_CXX_FLAGS "-I${CMAKE_CURRENT_SOURCE_DIR} -coverage -std=c++11 -Wall -Wextra -Werror -Wshadow -Wnon-virtual-dtor -pedantic -g -fsanitize=address -fno-omit-frame-pointer")

add_executable(turtle demo.cc)
target_link_libraries (turtle -lsfml-graphics -lsfml-window -lsfml-system)

add_executable(unit_tests movable_pointer_test.cc entity_test.cc)
target_link_libraries (unit_tests -lgmock_main -lpthread -lasan)

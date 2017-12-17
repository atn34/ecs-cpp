cmake_minimum_required (VERSION 2.6)
set (CMAKE_CXX_FLAGS "-coverage -std=c++11 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -g")

add_executable(unit_tests movable_pointer_test.cc entity_test.cc)
target_link_libraries (unit_tests -lgmock_main -lpthread)

#!/bin/bash

set -euxo pipefail

${CXX:-g++} -coverage -std=c++11 -Wall -Wextra -Werror -Wshadow -Wnon-virtual-dtor -pedantic movable_pointer_test.cc entity_test.cc -lgmock_main -lpthread -g -O0

#!/bin/bash

set -euxo pipefail

mkdir -p build
cd build
cmake -G Ninja ..
ninja

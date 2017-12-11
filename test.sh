#!/bin/bash

set -euxo pipefail

./a.out
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory out
google-chrome out/index.html

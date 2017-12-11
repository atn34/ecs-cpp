#!/bin/bash

set -euxo pipefail

lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory out
google-chrome out/index.html


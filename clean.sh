#!/bin/bash

set -euxo pipefail

git clean -Xf
rm -rf out build

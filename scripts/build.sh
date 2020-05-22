#!/bin/bash

set -e

# Define C intermediate and js/wasm output dirs
build_c_dir=/tmp/marst-wasm-build
build_js_dir=app/algol-wasm

# Create these directories if they don't exist
mkdir -p $build_js_dir $build_c_dir

# Define C intermediate and js output paths
build_c=$build_c_dir/build.c
build_js=$build_js_dir/index.js

echo ""
echo ">>> Transpiling ALGOL60 -> C ..."
algol_source="alg/app.alg"
marst $algol_source -o $build_c


cat c/extern.c $build_c | sponge $build_c

tput setaf 3 # Set font to yellow
echo ">>> Compiling C -> WASM + JS ..."
emcc \
    $build_c \
    /libalgol-llvm/libalgol.bc \
    -I/usr/local/include \
    -o $build_js \
    -Wno-unsequenced \
    -s ASYNCIFY \

echo "$(tput setaf 2)>>> Build Complete!$(tput sgr 0)"
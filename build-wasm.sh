INPUT="src/app.alg"
mkdir -p /tmp/marst-wasm-build out
marst $INPUT -o /tmp/marst-wasm-build/app.c
rm -rf out/*
emcc /tmp/marst-wasm-build/app.c /libalgol-llvm/libalgol.bc -I/usr/local/include -o out/app.js

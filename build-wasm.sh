INPUT="marst-2.7/examples/primes.alg"

mkdir -p tmp/build
marst $INPUT -o tmp/build/app.c
rm -rf out/*
emcc tmp/build/app.c lib/libalgol.bc -I./marst-2.7 -o out/app.js

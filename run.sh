./marst-bin/marst marst-2.7/examples/primes.alg -o primes.c
clang primes.c -lalgol -lm -o primes -I./marst-2.7 -L/workspaces/algol-60-webassembly/marst-2.7/.libs

LD_LIBRARY_PATH=/workspaces/algol-60-webassembly/marst-2.7/.libs ./primes
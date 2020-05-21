ROOT=`dirname $(realpath $0)`

mkdir -p /tmp/alglib /libalgol-llvm
cd /tmp/alglib
emcc -c /tools/marst/marst-2.7/alglib*.c -emit-llvm
llvm-link alglib*.bc -o /libalgol-llvm/libalgol.bc
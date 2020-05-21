ROOT=`dirname $(realpath $0)`
echo $ROOT
mkdir -p $ROOT/tmp $ROOT/lib

cd tmp
emcc -c $ROOT/marst-2.7/alglib*.c -emit-llvm
llvm-link alglib*.bc -o $ROOT/lib/libalgol.bc
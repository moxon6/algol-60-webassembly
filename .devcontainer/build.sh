set -e 

cd "$(dirname "$0")"

# Fetch dependencies
./fetch-dependencies.sh

# Build and install standard version of marst
./marst.build.sh

# Build llvm version of libalgol
./libalgol.build.sh
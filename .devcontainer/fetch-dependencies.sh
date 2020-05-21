set -e

mkdir -p /tools/marst
cd /tools/marst

#TODO Add checksums
wget https://ftp.gnu.org/gnu/marst/marst-2.7.tar.gz
tar xvzf marst-2.7.tar.gz
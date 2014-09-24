#!/usr/bin/env bash

# Rough script for obtaining/building GRUB tools on Mac OS X
# Installs them in $HOME/opt/cross/... to match the cross-compiler tools
#
# I am not sure which versions of dependencies are necessary, but I have
# the following installed using Homebrew:
#
#   GNU automake
#   GNU autoconf
#   GNU flex
#   GNU bison

set -vex

export PREFIX=$HOME/opt/cross
build_dir=$HOME/src

test -d $build_dir || mkdir -p $build_dir

cd $build_dir

# First: Download GRUB
git clone git://git.savannah.gnu.org/grub.git

# Download and build objconv (also available @ http://www.agner.org/optimize/objconv.zip)
git clone https://github.com/vertis/objconv.git
cd objconv
g++ -o objconv -O2 src/*.cpp
# Install the objconv binary somewhere useful (annoying)
# I tried putting it in the GRUB folder and `configure` doesn't complain but compilation
# fails...
cp objconv /usr/local/bin/

# Build GRUB
cd $build_dir/grub
./autogen.sh
./configure --prefix=$PREFIX
make
make install

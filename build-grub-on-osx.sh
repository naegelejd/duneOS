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
#
# and I force linked flex/bison because grub needs newer versions.
#
# You will also need to build an i386-elf cross compiler (GCC) and make
# sure its binaries are on your $PATH

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

cd $build_dir
mkdir grub-build
cd grub-build
../grub/configure --disable-werror TARGET_CC=i386-elf-gcc TARGET_OBJCOPY=i386-elf-objcopy TARGET_STRIP=i386-elf-strip TARGET_NM=i386-elf-nm TARGET_RANLIB=i386-elf-ranlib --target=i386-elf --prefix=$PREFIX
make
make install

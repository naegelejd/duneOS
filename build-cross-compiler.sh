#!/usr/bin/env bash

# OS X instructions
# ------------------
# Install gcc 4.8 and 'select' it using macports
# Build binutils, gcc and its dependencies:

set -e

binutils_arch=binutils-2.23.2.tar.bz2
binutils_url=http://ftp.gnu.org/gnu/binutils/$binutils_arch

libiconv_arch=libiconv-1.14.tar.gz
libiconv_url=http://ftp.gnu.org/pub/gnu/libiconv/$libiconv_arch

gcc_ver=gcc-4.8.1
gcc_arch=$gcc_ver.tar.bz2
gcc_url=ftp://aeneas.mit.edu/pub/gnu/gcc/$gcc_ver/$gcc_arch

export PREFIX=$HOME/opt/cross
export TARGET=i386-elf
export PATH=$PREFIX/bin:$PATH
export CC=/opt/local/bin/gcc
export CXX=/opt/local/bin/g++
export CPP=/opt/local/bin/cpp
export LD=/opt/local/bin/gcc

test -d ~/src || mkdir ~/src

cd ~/src
curl -O $binutils_url
tar xjf $binutils_arch
mkdir binutils-build
cd binutils-build
../$binutils_arch/configure --target=$TARGET --prefix="$PREFIX" --disable-nls
make -j4
make install

cd ~/src
curl -O $gcc_url
tar xjf $gcc_arch
cd $gcc_ver
./contrib/download_prerequisites

cd ~/src
curl -O $libiconv_url
tar xzf $libiconv_arch
mv $libiconv_arch $gcc_ver/libiconv

cd ~/src
mkdir gcc-build
cd gcc-build
../$gcc_ver/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
make all-gcc -j4
make all-target-libgcc -j4
make install-gcc
make install-target-libgcc

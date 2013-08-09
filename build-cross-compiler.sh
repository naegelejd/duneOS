#!/usr/bin/env bash

# OS X instructions
# ------------------
# Install gcc 4.8 and 'select' it using macports
# Build binutils, gcc and its dependencies:

set -vex

build_dir=$HOME/src

binutils_ver=binutils-2.23.2
binutils_arch=$binutils_ver.tar.bz2
binutils_url=http://ftp.gnu.org/gnu/binutils/$binutils_arch

libiconv_ver=libiconv-1.14
libiconv_arch=$libiconv_ver.tar.gz
libiconv_url=http://ftp.gnu.org/pub/gnu/libiconv/$libiconv_arch

gcc_ver=gcc-4.8.1
gcc_arch=$gcc_ver.tar.bz2
gcc_url=ftp://aeneas.mit.edu/pub/gnu/gcc/$gcc_ver/$gcc_arch

export PREFIX=$HOME/opt/cross
export TARGET=i386-elf
export PATH=$PREFIX/bin:$PATH
export CC=gcc
export CXX=g++
export CPP=cpp
export LD=gcc

test -d $build_dir || mkdir $build_dir

cd $build_dir
test -f $binutils_arch || curl -O $binutils_url
test -d $binutils_ver || tar xjf $binutils_arch
test -d binutils-build || mkdir binutils-build
cd binutils-build
../$binutils_ver/configure --target=$TARGET --prefix="$PREFIX" --disable-nls
make -j4
make install

cd $build_dir
test -f $gcc_arch || curl -O $gcc_url
test -d $gcc_ver || tar xjf $gcc_arch
cd $gcc_ver
./contrib/download_prerequisites

cd $build_dir
test -f $libiconv_arch || curl -O $libiconv_url
test -d $libiconv_ver || tar xzf $libiconv_arch
test -d $gcc_ver/libiconv || mv $libiconv_ver $gcc_ver/libiconv

cd $build_dir
test -d gcc-build || mkdir gcc-build
cd gcc-build
../$gcc_ver/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
make all-gcc -j4
make all-target-libgcc -j4
make install-gcc
make install-target-libgcc

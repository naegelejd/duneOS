#!/usr/bin/env bash

set -vex

export PREFIX=$HOME/opt/cross
export TARGET=i386-elf
export PATH=$PREFIX/bin:$PATH
export CC=${CC:-gcc-5}
export CXX=${CXX:-g++-5}
export CPP=${CPP:-cpp-5}
export LD=${LD:-${CC}}

# ensure we have the md5sum utility to avoid re-downloading GCC and binutils
type md5sum 2>/dev/null || { echo "error: md5sum not found!" && exit 1; }

idonthave() {
    md5=$(md5sum "$1" 2>/dev/null | cut -f1 -d\ )
    if [ -f "$1" -a "$md5" = "$2" ]; then
        return 1
    fi
    return 0
}

build_dir=$HOME/src/cross

binutils_ver=binutils-2.25
binutils_arc=$binutils_ver.tar.bz2
binutils_md5="d9f3303f802a5b6b0bb73a335ab89d66"
binutils_url=ftp://ftp.gnu.org/gnu/binutils/$binutils_arc

# libiconv_ver=libiconv-1.14
# libiconv_arc=$libiconv_ver.tar.gz
# libiconv_md5="e34509b1623cec449dfeb73d7ce9c6c6"
# libiconv_url=ftp://ftp.gnu.org/gnu/libiconv/$libiconv_arc

gcc_ver=gcc-4.9.3
gcc_arc=$gcc_ver.tar.bz2
gcc_md5="6f831b4d251872736e8e9cc09746f327"
gcc_url=ftp://ftp.gnu.org/gnu/gcc/$gcc_ver/$gcc_arc

test -d $build_dir || mkdir -p $build_dir

cd $build_dir

# Download
if idonthave $binutils_arc $binutils_md5; then
    curl -LO $binutils_url
fi
if idonthave $gcc_arc $gcc_md5; then
    curl -LO $gcc_url
fi
# if idonthave $libiconv_arc $libiconv_md5; then
#     curl -LO $libiconv_url
# fi

# Unpack
cd $build_dir
test -d $binutils_ver || tar xjf $binutils_arc
test -d $gcc_ver || tar xjf $gcc_arc
# test -d $libiconv_ver || tar xzf $libiconv_arc

# Shuffle
cd $build_dir
# test -d $gcc_ver/libiconv || mv $libiconv_ver $gcc_ver/libiconv
cd $gcc_ver
# echo "language=c" > gcc/ada/config-lang.in
# cp -va gcc/ada/gcc-interface/Make-lang.in gcc/ada/Make-lang.in
./contrib/download_prerequisites

# Build
cd $build_dir
test -d binutils-build || mkdir binutils-build
cd binutils-build
../$binutils_ver/configure --target=$TARGET --prefix="$PREFIX" --disable-werror --disable-nls
make
make install

cd $build_dir
test -d gcc-build || mkdir gcc-build
cd gcc-build
../$gcc_ver/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

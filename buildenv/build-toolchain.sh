#!/bin/sh -e

apt update

# Tools and libraries required to build binutils and gcc
apt install -y build-essential libgmp3-dev libmpfr-dev libmpc-dev
# For making a bootable iso
apt install -y xorriso grub-pc-bin grub-common
# Others
apt install -y nasm wget

# We'll use the default x86_64-elf target
TARGET=x86_64-elf
BINUTILS_VERSION=2.37
GCC_VERSION=11.2.0

# Configure, make and install binutils
cd /opt
wget http://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.gz
tar -xf binutils-${BINUTILS_VERSION}.tar.gz
mkdir binutils-build && cd binutils-build
../binutils-${BINUTILS_VERSION}/configure \
  --target=${TARGET} \
  --disable-nls \
  --disable-werror \
  --with-sysroot \

make -j 4
make install

# Configure, make and install gcc and libgcc
cd /opt
wget http://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz
tar -xf gcc-${GCC_VERSION}.tar.gz
mkdir gcc-build && cd gcc-build
../gcc-${GCC_VERSION}/configure \
  --target=${TARGET} \
  --disable-nls \
  --enable-languages=c,c++ \
  --without-headers \

make all-gcc all-target-libgcc -j 4
make install-gcc install-target-libgcc

cd /

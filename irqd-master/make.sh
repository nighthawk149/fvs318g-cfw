#!/bin/bash
export PATH=/opt/arm-uclibc-3.4.6/bin/:$PATH
# general cross-compile
export CROSS_COMPILE=arm-linux-
export CC=${CROSS_COMPILE}gcc
#export CFLAGS=
# -mtune=arm920t not a valid identifier
# -O0 -msoft-float -D__GCC_FLOAT_NOT_NEEDED
export CXX=${CROSS_COMPILE}"g++" 
export AR=${CROSS_COMPILE}"ar" 
export AS=${CROSS_COMPILE}"as" 
export RANLIB=${CROSS_COMPILE}"ranlib" 
export LD=${CROSS_COMPILE}"ld"
export STRIP=${CROSS_COMPILE}"strip"
export KERNEL_DIR=../linux-2.6.16-star/

./configure --host=arm-linux --prefix=/home/nighthawk/work/fvs318g/irqd-master/install/
make-3.81
make-3.81 install

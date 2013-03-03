#!/bin/sh
LC_ALL=C
LANG=C

#export PATH=/usr/local/arm-linux-uclibc-3.3.6/bin:$PATH
#export PATH=/opt/star/kforwork:$PATH
#ROOTFSIMG=`pwd`/rootfs.gz
#ROOTFSIMG=/opt/star/images/ramdisk_2.4.27.img.gz
#ROOTFSIMG=/opt/star/output/ramfs.img.gz
ROOTFSIMG=../ramdisk.img.gz

rm -f arch/arm/boot/bootp/*.o
rm -f arch/arm/boot/bootp/bootp
rm -f arch/arm/boot/Image
rm -f arch/arm/boot/zImage
rm -f arch/arm/boot/bootpImage

make bootpImage INITRD=$ROOTFSIMG
cp -vf arch/arm/boot/bootpImage /tftpboot


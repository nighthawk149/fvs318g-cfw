export PATH="$PATH:/opt/arm-uclibc-3.4.6/bin/"

#./configure --host=arm-unknown-none --disable-dependency-tracking \
#LDFLAGS="-lm" \
#CC="arm-linux-gcc -std=gnu99" \
#CPP="arm-linux-gcc -E" \
#KERNEL_DIR="../linux-2.6.16-star/" \
#--enable-libipq --enable-devel \
#--enable-static

make KERNEL_DIR=../linux-2.6.16-star/ CC=arm-linux-gcc LD=export TGT_TOOL=arm-linux-ld AR=export TGT_TOOL=arm-linux-ar

#cp iptables/xtables-multi ../userfs/usr/sbin/

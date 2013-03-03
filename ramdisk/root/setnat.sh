#!/bin/sh

#NFPATH="/lib/modules/2.4.27-star/kernel/net/ipv4/netfilter"

#insmod $NFPATH/ip_tables    
#insmod $NFPATH/iptable_filter
#insmod $NFPATH/ip_conntrack
#insmod $NFPATH/ip_conntrack_ftp
#insmod $NFPATH/iptable_nat
#insmod $NFPATH/ipt_MASQUERADE.o
insmod str9100_shnat
ifconfig eth1 172.20.5.230
ifconfig eth0 192.168.1.101
ifconfig lo 127.0.0.1
route add default gw 172.20.1.150

iptables -A POSTROUTING -t nat -o eth1 -j MASQUERADE

echo "lanip 192.168.1.101" > /proc/str9100/shnat
echo "wanip 172.20.5.230" > /proc/str9100/shnat

echo 1 > /proc/sys/net/ipv4/ip_forward


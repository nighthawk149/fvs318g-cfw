#!/bin/sh

insmod str9100_shnat
LAN=192.168.200.1
WAN=172.200.1.1
#ifconfig eth0 $LAN
ifconfig eth1 $WAN
echo "debug enable" > /proc/str9100/shnat
echo "wanip $WAN" > /proc/str9100/shnat
echo "lanip $LAN" > /proc/str9100/shnat
#echo 1 > /proc/sys/net/ipv4/ip_forward
iptables -A POSTROUTING -t nat -o eth1 -j MASQUERADE
#iptables -t nat -A PREROUTING -p tcp -i eth1 --dport 21 -j DNAT --to-destination 192.168.200.10:21
IF0=eth0
IF1=wlan0
BR0_IP=192.168.200.1


#echo 1 > /proc/sys/net/ipv4/ip_forward

ifconfig $IF0 up
ifconfig $IF1 up
ip addr flush dev $IF0
ip addr flush dev $IF1
#ifconfig eth1 down
#brctl delbr br0

brctl addbr br0
brctl addif br0 $IF0
brctl addif br0 $IF1

ifconfig br0 $BR0_IP

echo "hnat enable"  > /proc/str9100/shnat 
echo "chgdscp enable"  > /proc/str9100/shnat
ifconfig fp up
# eth2 is pci net device
echo "fastpcitowan if:$IF1 gvid:1"> /proc/str9100/shnat
echo 1 > /proc/sys/net/ipv4/ip_forward

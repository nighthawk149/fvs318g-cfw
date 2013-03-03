#!/bin/bash
echo "1" > /proc/sys/net/ipv4/ip_forward
echo "1" > /proc/sys/net/ipv4/ip_dynaddr

echo "setup wan"
export EXT=eth1
ifconfig $EXT 192.168.1.30 netmask 255.255.255.0 up
route add default gw 192.168.1.1

echo "setup lan"
export LAN=eth0.1
export LAN_IP_RANGE=192.168.2.0/24
ifconfig $LAN 192.168.2.1 netmask 255.255.255.0 up


echo "cleaning iptables"
iptables -F
iptables -F -t nat
iptables -F -t mangle
iptables -X
iptables -t nat -X
iptables -t mangle -X
echo "done"

echo "dropping all by default"
iptables -P INPUT DROP
iptables -P OUTPUT DROP
iptables -P FORWARD DROP
echo "done"

echo "allowing loopback traffic"
iptables -A INPUT -i lo -j ACCEPT
iptables -A INPUT -i $LAN -j ACCEPT
iptables -A OUTPUT -o lo -j ACCEPT
iptables -A OUTPUT -o $LAN -j ACCEPT
echo "done"

echo "1"
iptables -A INPUT -p all -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT
echo "2"
iptables -A OUTPUT -p all -m conntrack --ctstate NEW,ESTABLISHED,RELATED -j ACCEPT
echo "3"
iptables -A FORWARD -p all -m conntrack --ctstate NEW,ESTABLISHED,RELATED -j ACCEPT
echo "4"
iptables -A FORWARD -p tcp -m conntrack --ctstate NEW -m tcp --dport 22 -j ACCEPT
echo "allowing traffic by port (tcp)"
iptables -A FORWARD -p tcp -m conntrack --ctstate NEW -m multiport --dports 80,443,pop3s,imaps,smtp,25565,5222,5001,5000  -j ACCEPT
echo "allowing traffic by port (udp)"
iptables -A FORWARD -p udp -m conntrack --ctstate NEW -m multiport --dports 80,443,25565,5222,5001 -j ACCEPT

echo "MTU fix"
iptables -I FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu

echo "droppping invalid packages"
iptables -A INPUT -m conntrack --ctstate INVALID -j DROP
iptables -A FORWARD -m conntrack --ctstate INVALID -j DROP
echo "done"

echo "dropping syn traffic"
iptables -A INPUT -p tcp ! --syn -m conntrack --ctstate NEW -j DROP
iptables -A OUTPUT -p tcp ! --syn -m conntrack --ctstate NEW -j DROP
echo "done"

echo "enabling lan to wan traffic"
iptables -A FORWARD -i $LAN -o $EXT -j ACCEPT

echo "disabling wan to lan traffic"
iptables -A FORWARD -i $EXT -o $LAN -j REJECT

echo "enabling masquerade 1..."
iptables -t nat -A POSTROUTING -o $EXT -s $LAN_IP_RANGE -j MASQUERADE
echo "... and 2"
iptables -t nat -A POSTROUTING -o $LAN -j SNAT --to-source 192.168.1.30



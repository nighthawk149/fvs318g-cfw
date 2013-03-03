#!/bin/bash
sudo rmmod r8169
sudo modprobe r8169
sudo ifconfig eth0 192.168.0.100 netmask 255.255.255.0 up

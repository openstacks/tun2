#!/bin/bash

ifconfig tun0 192.168.13.1 pointopoint 192.168.13.2
ifconfig tun1 192.168.13.2 pointopoint 192.168.13.1
ip route del 192.168.13.1 table local
ip route del 192.168.13.2 table local
ip route add local 192.168.13.1 dev tun0 table 13
ip route add local 192.168.13.2 dev tun1 table 13
ip rule add iif tun0 lookup 13
ip rule add iif tun1 lookup 13

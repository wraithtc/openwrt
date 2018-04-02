#!/bin/sh

lanip=$(uci get network.lan.ipaddr)

echo "address=/gw.3caretec.com/"$lanip >/etc/dnsmasq.conf
echo "address=/router.qtec.cn/"$lanip >> /etc/dnsmasq.conf

/etc/init.d/dnsmasq restart


iptables -t nat -I PREROUTING -m string --hex-string "router|04|qtec|02|cn" --algo bm -j DNAT --to-destination $lanip
iptables -t nat -I PREROUTING -m string --hex-string "gw|08|3caretec|03|com" --algo bm -j DNAT --to-destination $lanip


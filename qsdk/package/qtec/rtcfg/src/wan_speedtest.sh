#!/bin/sh

#for example *.sh eth0.2
eth=$1
RXpre=$(cat /proc/net/dev | grep $eth | tr : " " | awk '{print $2}')
TXpre=$(cat /proc/net/dev | grep $eth | tr : " " | awk '{print $10}')
sleep 1
RXnext=$(cat /proc/net/dev | grep $eth | tr : " " | awk '{print $2}')
TXnext=$(cat /proc/net/dev | grep $eth | tr : " " | awk '{print $10}')
#clear
#echo  -e  "\t RX `date +%k:%M:%S` TX"
RX=$((${RXnext}-${RXpre}))
TX=$((${TXnext}-${TXpre}))
RX="${RX}"
TX="${TX}"
echo -e "$eth  $RX   $TX "

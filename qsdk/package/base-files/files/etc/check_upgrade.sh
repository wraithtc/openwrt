#!/bin/sh
A='1'
while [ $A -le 10000 ]; do
	if [ -f "/tmp/img_download_ok" ]; then
		if [ -f "/tmp/upgrade_save_config" ]; then
			sysupgrade /tmp/firmware.img
		else
			sysupgrade -n /tmp/firmware.img
		fi
		exit
	fi
	sleep 1
done
#!/bin/sh

TEST=$(cat /proc/uptime  | sed -e 's/\.[0-9]* [0-9]*\.[0-9]*//g')

echo #########################$TEST########################

if [ $TEST -gt 120 ]; then
	echo #######################poweroff#######################
	poweroff
fi

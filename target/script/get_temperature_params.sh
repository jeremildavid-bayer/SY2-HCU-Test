#!/bin/bash
usage(){
	echo "Usage: $0"
	echo "Example: $0"
	exit 1
}

if [ $# -ne 0 ]; then
	usage
fi

sensors |grep °C


FILE=/sys/class/misc/hwm/FAN_Fan_Cpu/value

if [ -f "$FILE" ]; then
	FAN_SPEED=$(cat /sys/class/misc/hwm/FAN_Fan_Cpu/value)
	echo "$FAN_SPEED rpm"
else
	echo "-1 rpm"
fi

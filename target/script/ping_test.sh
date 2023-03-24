#!/bin/bash
usage(){
	echo ""
	echo "Usage: $0 [dest ip]"
	echo ""
	echo "Example: $0 192.168.11.3"
	echo ""
	exit 1
}

if [ $# -ne 1 ]; then
	usage
fi

dst_ip=$1
ping_output=$(ping $dst_ip -c 2 -q -w 4 -i 2 -s 10)

if [[ $ping_output == *" 0% packet loss"* ]]; then
	echo "ping ok"
else
	echo "ping failed"
fi



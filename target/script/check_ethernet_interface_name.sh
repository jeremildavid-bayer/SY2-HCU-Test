#!/bin/bash

print_usage(){
	echo ""
	echo "Usage: $0 [ethernet interface name]"
	echo ""
	echo "Example: $0 enp3s0"
	echo ""
}

if [ $# -ne 1 ]; then
	print_usage
	exit 1;
fi

i_name=$1
ethtool $i_name &> /dev/null
if [ $? -ne 0 ]; then
	echo $i_name is NOT available
    exit 1;
else
	echo $i_name is available
    exit 0;
fi


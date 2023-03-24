#!/bin/bash
usage(){
	echo ""
	echo "Usage: $0 [wifi interface]"
	echo ""
	echo "Example: $0 ath0"
	echo ""
	exit 1
}

if [ $# -ne 1 ]; then
	usage
fi

dst_wifi_interface=$1

iwconfig $dst_wifi_interface

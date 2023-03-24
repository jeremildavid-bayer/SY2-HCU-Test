#!/bin/bash
usage(){
	echo "Usage: $0"
	echo "Example: $0"
	exit 1
}

if [ $# -ne 0 ]; then
	usage
fi

# Platform sometimes resets the DateTime value underneath us... This is a hack to reverse this.
DATE_TIME=$(timedatectl show --value -p RTCTimeUSec)
timedatectl set-time "$DATE_TIME"

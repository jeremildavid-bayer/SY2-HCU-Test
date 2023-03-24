#!/bin/bash
usage(){
	echo ""
	echo "Usage: $0 [datetime]"
	echo ""
	echo "  datetime: date time string value with the format \"yyyy-MM-dd hh:mm:00\""
	echo ""
	echo "Example: $0 \"2017-11-21 03:27:00\""
	echo ""
	exit 1
}

if [ $# -ne 1 ]; then
	usage
fi

DATETIME_STR=$1

sudo timedatectl set-time "$DATETIME_STR"

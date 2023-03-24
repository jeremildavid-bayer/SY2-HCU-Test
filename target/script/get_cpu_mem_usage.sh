#!/bin/bash
usage(){
	echo ""
	echo "Usage: $0"
	echo ""
	exit 1
}

if [ $# -ne 0 ]; then
	usage
fi

top -n 1 -b | head -n 15


#!/bin/bash
usage(){
	echo "Usage: $0"
	echo "Example: $0"
	exit 1
}

if [ $# -ne 0 ]; then
	usage
fi

xdg-screensaver reset

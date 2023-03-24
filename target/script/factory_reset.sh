#!/bin/bash

usage(){
	echo "Usage: $0"
	echo "  Example: $0"
	exit 1
}

if [ $# -ne 0 ]; then
	usage
fi

rm -rf /IMAX_USER/db/*
rm -rf /IMAX_USER/log/*
rm -rf /IMAX_USER/temp/*
# The HW info should be persistent over factory reset
#rm -rf /IMAX_USER/info/*
rm -rf /IMAX_USER/cru/*
rm -rf /IMAX_USER/coredump/*

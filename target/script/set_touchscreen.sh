#!/bin/bash

# Enables or disables the touchscreen

usage() {
    echo "Usage: $0 [enable|disable]"
    echo "Example: $0 enable"
    exit 0
}

if [ $# -ne 1 ] 
then
    usage
fi

TOUCH_ID=$(xinput --list | grep touch -i | cut -d"=" -f2 | awk '{print $1}')

for i in $TOUCH_ID
do
    if [ "$1" == "enable" ]
    then
        xinput enable $i
    elif [ "$1" == "disable" ]
    then
        xinput disable $i
    else
        usage
    fi
done


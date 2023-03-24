#!/bin/bash
# This script will authorize/un-authorize any new USB devices from getting recognized
# Any usb devices already connected to the system will NOT be affected

print_usage() {
    echo ""
    echo "Usage: $0 [authorize/unauthorize]"
    echo ""
    echo "Example: $0 unauthorize"
    echo""
}

if [ $# -ne 1 ]; then
    print_usage
    exit 1;
fi

on_off="x"
if [ $1 == "authorize" ]; then
    on_off="1"
    # Restore mouose cursor
    sudo killall unclutter
elif [ $1 == "unauthorize" ]; then
    on_off="0"
fi

sudo bash -c "echo $on_off > /sys/bus/usb/devices/usb1/interface_authorized_default"
sudo bash -c "echo $on_off > /sys/bus/usb/devices/usb2/interface_authorized_default"

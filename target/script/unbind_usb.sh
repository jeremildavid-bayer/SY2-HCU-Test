#!/bin/bash
if [ $(ls -l /sys/bus/usb/devices | grep "1-2.2 " | wc -l) -ne 0 ];then 
    echo "disconnecting usb port 2";
    sudo bash -c "echo '1-2.2' > /sys/bus/usb/drivers/usb/unbind"
fi
if [ $(ls -l /sys/bus/usb/devices | grep "1-2.1 " | wc -l) -ne 0 ];then 
    echo "disconnecting usb port 1";
    sudo bash -c "echo '1-2.1' > /sys/bus/usb/drivers/usb/unbind"
fi


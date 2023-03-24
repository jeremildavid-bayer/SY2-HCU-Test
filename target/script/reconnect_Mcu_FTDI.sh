#!/bin/bash

# re-enable MCU USB data connection
# /dev/ttyUSB0 (MCU data connection) gets disabled for some reason during boot.. (check "dmesg |grep ttyUSB0")
# re-initialize this connection which will create /dev/ttyUSB0
# once /dev/ttyUSB0 is created, MCU can be directly communicated by command "screen /dev/ttyUSB0 115200"

USBDRIVERSPATH=/sys/bus/usb/drivers/usb
USBDEVICESPATH=/sys/bus/usb/devices
USBDEVICE="ttyUSB0"
FTDI_VENDOR_ID="0403" # can be found from lsusb command
FTDI_PRODUCT_ID="6001" # can be found from lsusb command 
usbPortNumber1=$(grep $FTDI_VENDOR_ID $USBDEVICESPATH/*/idVendor | grep -o "[0-9]-[0-9\.]*")
usbPortNumber2=$(grep $FTDI_PRODUCT_ID $USBDEVICESPATH/*/idProduct | grep -o "[0-9]-[0-9\.]*")

if [ "$usbPortNumber1" == "$usbPortNumber2" ]; then
    usbPortNumber="$usbPortNumber1"
else
    usbPortNumber=""
fi

echo "Re-binding Mcu FTDI USB connection..."

# hacky method. Keeping it here for reference
#usbPortNumber=$(dmesg |grep -m 1 -e "FTDI USB.*attached to $USBDEVICE" | awk '{print $4}' | tr -d :)

if [ "$usbPortNumber" == "" ]; then
    echo "Mcu USB Port Number not found!"
    exit 1
else
    #echo usbPortNumber=$usbPortNumber
    echo -n "$usbPortNumber" | sudo tee $USBDRIVERSPATH/unbind
    sleep 1
    echo -n "$usbPortNumber" | sudo tee $USBDRIVERSPATH/bind
fi

echo "Mcu FTDI USB connection initialized."

exit 0

#!/bin/bash
usage(){
	echo "Usage: $0 [filepath]"
	echo "	Example: $0 /media/user/USB_Stick/SY2_SRU_20xx_xxxx_DEV.imx.pkg"
	exit 1
}

if [ $# -ne 1 ]; then
	usage
fi

UPGRADE_PKG=$1
U20_VERSION=2022.0701

echo "UPGRADE_PKG=$UPGRADE_PKG"
cd /IMAX_USER/temp/upgrade
sudo rm -f FILE.INFO
unzip "$UPGRADE_PKG" FILE.INFO

if [ -e /IMAX_USER/temp/upgrade/FILE.INFO ]; then
	cat /IMAX_USER/temp/upgrade/FILE.INFO
	UPGRADE_VERSION=$(grep '<VERSION>' < /IMAX_USER/temp/upgrade/FILE.INFO | sed 's/<VERSION>//g' )
	if (( $(echo "$UPGRADE_VERSION < $U20_VERSION" | bc) )); then
		echo "ERROR: Incompatible version"
	else
		echo "INFO FILE read done"
	fi
else
	echo "ERROR: FILE.INFO does not exist"
fi

sudo rm -f FILE.INFO

#!/bin/bash

usage(){
	echo "Usage: $0 [Upgrade Package] [Destination Dir]"
	echo "	Example: $0 SY2_SRU_20xx_xxxx_DEV.imx.pkg /IMAX_USER/temp/upgrade"
	exit 1
}

if [ $# -ne 2 ]; then
	usage
fi

UPGRADE_PKG=$1
PATH_TARGET=$2

rm -rf $PATH_TARGET/*

# Check file exists
if [ -e $UPRGADE_PKG ]; then
	echo "Extracting $UPGRADE_PKG to $PATH_TARGET"
else
	echo "ERROR: could not find upgrade package."
	exit 1
fi

(unzip -o "$UPGRADE_PKG" -d "$PATH_TARGET") &&
(echo "Extraction Done")






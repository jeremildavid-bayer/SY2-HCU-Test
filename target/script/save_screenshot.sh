#!/bin/bash

usage(){
	echo "Usage: $0 [prefix] [x] [y] [width] [height]"
	echo "  Example: $0 EN 0 0 1024 700"
	exit 1
}

if [ $# -ne 5 ]; then
	usage
fi

FILE_PREFIX=$1
CROP_X=$2
CROP_Y=$3
CROP_W=$4
CROP_H=$5
PATH_USB=$(df |grep /media/user |awk '{for(i=6;i<=NF;i++){print $i}}')
PATH_USB=$(echo $PATH_USB | sed s/:/\\n/g) # Support name with spacing in the path
PATH_TARGET="$PATH_USB"/Centargo/ScreenShots
FILENAME="Centargo-SRU-ScreenShot-"$FILE_PREFIX-$(date '+%Y%m%d_%H%M%S.png');

if [ "$PATH_USB" == "" ]; then
	echo "NO USB FOUND"
elif [ -d "$PATH_USB" ];then
	echo "USB OK. Creating directory $PATH_TARGET"
	aplay /home/user/Imaxeon/media/screenshot.wav&
	sudo mkdir -vp "$PATH_TARGET"
	import -window root -crop ${CROP_W}x${CROP_H}+${CROP_X}+${CROP_Y} "$PATH_TARGET"/$FILENAME
	echo "file=$FILENAME, syncing.."
	sync
	echo "SCREENSHOT SAVE OK"
else
	echo "FAILED TO ACCESS $PATH_USB"
fi



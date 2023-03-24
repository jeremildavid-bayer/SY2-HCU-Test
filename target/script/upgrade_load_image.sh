#!/bin/bash

usage(){
	echo "Usage: $0 [image extracted path] [filepath]"
	echo "	Example: $0 /IMAX_USER/temp/upgrade /media/USB_Stick/IMX_SW.img.gz"
	exit 1
}

if [ $# -ne 2 ]; then
	usage
fi

PATH_UPGRADE=$1
PATH_UPGRADE_FILE=$2
PATH_POST_UPGRADE_SCRIPT="$PATH_UPGRADE/post_upgrade.sh"
PATH_LABEL="/dev/disk/by-label"
PATH_NEW_ACTIVE_LABEL="$PATH_LABEL/IMAX_INACTIVE"

echo "Upgrade started using $PATH_UPGRADE_FILE.."

# Check file exists
if [ -e $PATH_UPGRADE_FILE ]; then
	echo "$PATH_UPGRADE_FILE is OK"
else
	echo "ERROR: could not find upgrade file."
	exit 1
fi

# Ensure newPart is unmounted
echo "Unmounting $PATH_NEW_ACTIVE_LABEL.."
sudo umount $PATH_NEW_ACTIVE_LABEL

# Number is general partition size for sda1 and 2
updateFileSize=8062524000

# Get partition parameters
curDevPath=$(mount |grep " / " |awk '{print $1}')
nextDevPath=""
userDevPath=""

if [[ "$curDevPath" == *sd?2* ]]; then
	nextDevPath=${curDevPath//2/3}
	userDevPath=${curDevPath//2/4}
elif [[ "$curDevPath" == *sd?3* ]]; then
	nextDevPath=${curDevPath//3/2}
	userDevPath=${curDevPath//3/4}
else
	echo "ERROR: Failed to label partitions. CurDevPath=$curDevPath"
	exit 1
fi

# Write image to partition
echo "Writing image (size=$updateFileSize).."
echo "(gunzip -c $updateFile | pv -s $updateFileSize -f | sudo dd of=$PATH_NEW_ACTIVE_LABEL conv=noerror bs=4096)"
(gunzip -c $PATH_UPGRADE_FILE | pv -s $updateFileSize -f | sudo dd of=$PATH_NEW_ACTIVE_LABEL conv=noerror bs=4096) &&
(echo "Writing image done")

# Switch Active/Inactive Paritions
echo "Labeling active partition: ($curDevPath)<->($nextDevPath).."
sudo e2label $nextDevPath ""
sudo e2label $curDevPath ""
sudo e2label $nextDevPath IMAX_ACTIVE
sudo e2label $curDevPath IMAX_INACTIVE
sudo e2label $userDevPath IMAX_USER

# Perform post upgrade process
echo "Performing Post Upgrade Process. File=$PATH_POST_UPGRADE_SCRIPT"
if [ -e $PATH_POST_UPGRADE_SCRIPT ]; then
	$PATH_POST_UPGRADE_SCRIPT
else
	echo "WARNING: could not find post upgrade file."
fi

# Finish upgrade process
rm -rf $PATH_UPGRADE/*

echo "=========================================="
echo "ls -l /dev/disk/by-label:"
ls -l /dev/disk/by-label
echo ""
echo "=========================================="

echo ""
(echo "Upgrade completed. Please reboot.")





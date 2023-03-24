#!/bin/bash

# VERSION: V0.0.1

#------------------------------------------------------------
IMAGE_NAME="HCUInstaller.iso"
PATH_OUTPUT_DIR_DEFAULT="/home/user/Desktop"

echo "   This script will backup a SOURCE USB to a image file."
echo "   Press 'Enter' to continue. Press 'Ctrl + c' to Exit."
read

# Obtain total amount of devices visible to system
NUM_DEVICES=$(sudo fdisk -l | grep 'Disk /dev/sd' | wc -l)

touch /home/user/Desktop/disk.info
PATH_DISK_INFO="/home/user/Desktop/disk.info"
sudo fdisk -l | grep 'Disk /dev/sd' > $PATH_DISK_INFO

# If Number of devices greater than 2, display list of choices and 
# allow user to select the device
if [ $NUM_DEVICES -gt 1 ]; then
	# =============================SOURCE SELECTION========================
	declare -a SOURCE_ARRAY
	let count=0
	while read line; do
		SOURCE_ARRAY[$count]=$line
		((count++))
	done < $PATH_DISK_INFO
	echo "************************************************************"
	echo "**                  SOURCE USB SELECTION                  **"
	echo "**              Please select a Device Number             **"
	echo "************************************************************"
#	for (( i=0; i<$count; i++))
#	do
#		let j=${i}+1
#	done
	PS3='>Select Device Number to use as source: '
	select word in "${SOURCE_ARRAY[@]}"
	do
		if [[ -n $word ]]; then
			echo ""
			echo "Selected Source Device: $word"
			break
		else
			echo ""
			echo "Invalid Selection."
			read
			exit 1
		fi
	done
	a=( $word )
	SOURCE_DEVICE=${a[1]%?}
	PARTITION=$(sudo fdisk -l | grep $SOURCE_DEVICE'2')
	b=( $PARTITION )
	END_SECTOR=${b[2]}
	let DISK_SIZE=$END_SECTOR*512
	rm $PATH_DISK_INFO

	# =========================DESTINATION SELECTION========================
	echo ""
	echo ""
	echo "************************************************************"
	echo "**                DESTINATION PATH SELECTION              **"
	echo "**          Please type in a valid destination path       **"
	echo "************************************************************"
	echo -n ">Type in a destination output path (default: $PATH_OUTPUT_DIR_DEFAULT): "
	read PATH_OUTPUT_DIR

	if [ "$PATH_OUTPUT_DIR" = "" ]; then
	PATH_OUTPUT_DIR=$PATH_OUTPUT_DIR_DEFAULT
	fi

	if [ -d $PATH_OUTPUT_DIR ]; then
		echo "Creating pkg file to $PATH_OUTPUT_DIR."
	else	
		echo "ERROR: $PATH_OUTPUT_DIR does not exist"
		read
		exit 1
	fi

else
	echo ""
	echo "ERROR: No USB Device found. Reinsert devices and run the script again."
	exit 1		
fi

echo ""
echo "======= Chosen Device and Path ======"
echo "SOURCE: $SOURCE_DEVICE"
echo "DEST  : $PATH_OUTPUT_DIR/$IMAGE_NAME"
echo ""
echo "If this is correct, press 'Enter' to begin backup."
echo "If this is incorrect, press 'Ctrl + c' to exit."
read

# Mounting Source USB
sudo mkdir /mnt/platformsys
MNT_PATH="/mnt/platformsys"
sudo umount $SOURCE_DEVICE*
sudo mount $SOURCE_DEVICE'2' $MNT_PATH
sleep 2

# Create zero file to fill free space
echo ""
echo "Filling zero to unused space..." 
echo "sudo dd if=/dev/zero of=$MNT_PATH/zero.file"
sudo dd if=/dev/zero conv=noerror| pv -s $DISK_SIZE -f | sudo dd of=$MNT_PATH/zero.file

# Remove zero file
sudo rm -f $MNT_PATH/zero.file

# Create image
sudo umount $SOURCE_DEVICE'2'
echo ""
echo "Creating image file..."
echo "sudo dd if=$SOURCE_DEVICE bs=512 count=$END_SECTOR conv=sparse,noerror | pv -s $DISK_SIZE -f | sudo dd of=$PATH_OUTPUT_DIR/$IMAGE_NAME"
sudo dd if=$SOURCE_DEVICE bs=512 count=$END_SECTOR conv=sparse,noerror | pv -s $DISK_SIZE -f | sudo dd of=$PATH_OUTPUT_DIR/$IMAGE_NAME
sudo rm -r $MNT_PATH

echo ""
echo ""
echo "Image Creation Complete. Image file is created to $PATH_OUTPUT_DIR"
echo "Press ENTER to exit."
echo ""
read


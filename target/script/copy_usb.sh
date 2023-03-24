#!/bin/bash

#------------------------------------------------------------
PATH_DISK_INFO="/IMAX_USER/temp/disk.info"
#------------------------------------------------------------

echo "   This script will copy a SOURCE USB to a DESTINATION USB."
echo "   Press 'Enter' to continue. Press 'Ctrl + c' to Exit."
read

# Clear any old data
rm $PATH_DISK_INFO

# Obtain total amount of devices visible to system
NUM_DEVICES=$(sudo fdisk -l | grep 'Disk /dev' | wc -l)
sudo fdisk -l | grep 'Disk /dev' > $PATH_DISK_INFO

# If Number of devices greater than 2, display list of choices and 
# allow user to select the device
if [ $NUM_DEVICES -gt 2 ]; then
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
	for (( i=0; i<$count; i++))
	do
		let j=${i}+1
	done
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
	#SOURCE_SIZE=${a[4]}
	PARTITION=$(sudo fdisk -l | grep $SOURCE_DEVICE'2')
	b=( $PARTITION )
	COUNT_SIZE=${b[2]}
	let SOURCE_SIZE=$COUNT_SIZE*512
	#echo "$COUNT_SIZE"

	# =========================DESTINATION SELECTION========================
	declare -a DESTINATION_ARRAY
	let count=0
	while read line; do
		DESTINATION_ARRAY[$count]=$line
		((count++))
	done < $PATH_DISK_INFO
	echo ""
	echo ""
	echo "************************************************************"
	echo "**                DESTINATION USB SELECTION               **"
	echo "**              Please select a Device Number             **"
	echo "************************************************************"
	for (( i=0; i<$count; i++))
	do
		let j=${i}+1
	done
	PS3='>Select Device Number to use as destination: '
	select word in "${DESTINATION_ARRAY[@]}"
	do
		if [[ -n $word ]]; then
			echo ""
			echo "Selected Destination Device: $word"
			break
		else
			echo ""
			echo "ERROR: Invalid Selection."
			read
			exit 1
		fi
	done
	a=( $word )
	DESTINATION_DEVICE=${a[1]%?}
else
	echo ""
	echo "ERROR: Not Enough USB Devices found. Reinsert devices and run the script again."
	exit 1		
fi

echo ""
echo "======= Chosen Devices ======"
echo "SOURCE: $SOURCE_DEVICE"
echo "DEST  : $DESTINATION_DEVICE"
echo ""
echo "If this is correct, press 'Enter' to begin copying."
echo "If this is incorrect, press 'Ctrl + c' to exit."
read

# Unmount source and destination
sudo umount $SOURCE_DEVICE*
sudo umount $DESTINATION_DEVICE*

#echo "$SOURCE_SIZE"
echo ""
sleep 2
echo ""
echo "Copying $SOURCE_DEVICE to $DESTINATION_DEVICE.."
echo "CMD> sudo dd if=$SOURCE_DEVICE bs=512 count=$COUNT_SIZE conv=sync,noerror | pv -s $SOURCE_SIZE -f |  sudo dd of=$DESTINATION_DEVICE"
sudo dd if=$SOURCE_DEVICE bs=512 count=$COUNT_SIZE conv=sync,noerror | pv -s $SOURCE_SIZE -f |  sudo dd of=$DESTINATION_DEVICE
aplay /home/user/Imaxeon/Sound/done.wav
sleep 2
echo ""
echo "Copying complete."
echo "Press ENTER to exit."
echo ""
read




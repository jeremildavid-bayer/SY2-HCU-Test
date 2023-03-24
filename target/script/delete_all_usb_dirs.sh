#!/bin/bash
usage(){
	echo "Usage: $0"
	echo "Example: $0"
	exit 1
}

if [ $# -ne 0 ]; then
	usage
fi

PATH_USB_MOUNT_DIR="/media/user"

mountedDirs=$(lsblk |grep $PATH_USB_MOUNT_DIR |awk {'print $7'})
allDirs=$(ls $PATH_USB_MOUNT_DIR)

#echo "mountedDirs=$mountedDirs"
#echo "allDirs=$allDirs"

for dir in $allDirs
do
	pathDir="$PATH_USB_MOUNT_DIR/$dir"
	deleteDirOk=1
	for mountedDir in $mountedDirs
	do
		#echo "compare $pathDir with $mountedDir"
		if [ "$pathDir" = "$mountedDir" ]; then
			deleteDirOk=0
			break
		fi
	done

	if (( $deleteDirOk == 1 )); then
		echo "Deleting $pathDir.."
		sudo rm -rf $pathDir
	fi
done


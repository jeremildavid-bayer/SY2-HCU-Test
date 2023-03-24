#!/bin/bash

usage(){
	echo "Usage: $0"
	echo "  Example: $0"
	exit 1
}

nextDevLabel="IMAX_INACTIVE"
nextDevPath=$(cd /dev/disk/by-label ; ls -la | grep $nextDevLabel | awk '{print $11}'| xargs realpath)

UPGRADE_PATH="/IMAX_USER/temp/upgrade"
EFI_IMAGE="$UPGRADE_PATH/EFI.img.gz"

# Write EFI partition if exists
if [ -e $EFI_IMAGE ]; then 
	bootDevPath=${nextDevPath/[0-9]/1}
	echo "gunzip -c $EFI_IMAGE | sudo dd of=$bootDevPath conv=noerror bs=4096"
	(gunzip -c $EFI_IMAGE | sudo dd of=$bootDevPath conv=noerror bs=4096) &&
	(echo "Writing image done")
fi

echo "Post Install Process Complete"


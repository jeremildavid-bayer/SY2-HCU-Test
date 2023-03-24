#!/bin/bash
# This script will create a package from the inactive partition in the user space. This includes
# - copy inactive partion as dd
# - copyt the EFI,
# - copy 
# - generate sha256sum
# - zip the installation package
# 
TOTAL_IMAGE_SIZE=8062524000
PATH_IMAGE="/IMAX_USER/image"
IMAGE_NAME="IMX_SW.img.gz"
PARTITION=/dev/disk/by-label/IMAX_INACTIVE
mkdir -p $PATH_IMAGE
cd $PATH_IMAGE
rm *

echo "sudo dd if=$PARTITION conv=noerror bs=4096 | pv -s $TOTAL_IMAGE_SIZE -f | gzip -c >  $PATH_IMAGE/$IMAGE_NAME"
sudo dd if=$PARTITION conv=noerror bs=4096 | pv -s $TOTAL_IMAGE_SIZE -f | gzip -c >  $PATH_IMAGE/$IMAGE_NAME

echo "sudo dd if=/dev/sda1 bs=4096 | gzip -c > $PATH_IMAGE/EFI.img.gz"
sudo dd if=/dev/sda1 bs=4096 | gzip -c > $PATH_IMAGE/EFI.img.gz

echo "Compute sha256sum for ~/Imaxeon"
cd $PATH_IMAGE
mkdir -p temp
sudo mount -L IMAX_INACTIVE  temp/
cp temp/home/user/Imaxeon/doc/version.inf FILE.INFO
cp temp/home/user/Imaxeon/upgrade/*.* .
cd temp/home/user/Imaxeon/
find . -type f | xargs sha256sum  > Imaxeon.sha256.txt

cd $PATH_IMAGE
sudo umount temp
rmdir temp


# Create and Fill in FILE.INFO
echo -e "<NAME>$IMAGE_NAME" >> FILE.INFO
imgSize=(`stat -c %s $PATH_IMAGE/$IMAGE_NAME`)
echo -e "<SIZE>$(($imgSize / 1024 / 1024)) \n" >> FILE.INFO
echo -e "<SIZE_BYTES>$imgSize\n" >> FILE.INFO

VERSION=`cat FILE.INFO | grep "<VERSION>" |  cut -d ">" -f2`
BUILD_DATE=`cat FILE.INFO | grep "<BUILD_DATE>" |  cut -d ">" -f2`
BUILD_TYPE=`cat FILE.INFO | grep "<BUILD_TYPE>" |  cut -d ">" -f2`

MCU_VERSION_PREFIX=`cat FILE.INFO | grep "<MCU_VERSION_PREFIX>" |  cut -d ">" -f2`
MCU_COMMAND_VERSION=`cat FILE.INFO | grep "<MCU_COMMAND_VERSION>" |  cut -d ">" -f2`

# SY2_SRU_2022_1001_SQA.imx.pkg
OUT_NAME="SY2_SRU_${VERSION}_${BUILD_TYPE}_DATE_${BUILD_DATE}_MCU_${MCU_VERSION_PREFIX}_CLI_${MCU_COMMAND_VERSION}.imx.pkg"
PATH_OUTPUT=${PATH_IMAGE}/../${OUT_NAME}

# Generate package sha1sum:
ls -1 | xargs sha256sum | tee package.sha256.txt

# Package/zip FILE.INFO and img
echo ""
echo "Creating upgrade package.. at $PATH_OUTPUT"
[ -f $PATH_OUTPUT ] && rm $PATH_OUTPUT
zip $PATH_OUTPUT * -1 -v
echo "sha256 sum $PATH_OUTPUT"
sha256sum $PATH_OUTPUT | tee $PATH_OUTPUT.sha256.txt
ls -alsth $PATH_OUTPUT



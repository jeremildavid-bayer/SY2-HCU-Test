#!/bin/bash

echo "Cleaning up saved network connections..."
sudo rm /etc/NetworkManager/system-connections/*
currentConns=$(sudo nmcli -t -f NAME c show | cut -d : -f1)
echo $currentConns
for connection in $currentConns; do
    echo "connection=$connection"
    sudo nmcli connection delete $connection 2>/dev/null
done
echo ""
echo "Emptying Trash..."
rm -rf /home/user/.local/share/Trash/files/*
echo ""
echo "Generating EFI partition image..."
cd /home/user/Imaxeon/upgrade
sudo dd if=/dev/sda1 bs=4096 | gzip -c > EFI.img.gz
echo ""
echo "Clearing terminal history..."
echo "" > /home/user/.bash_history
echo ""
echo "Preparation done. Please make sure all the system keyboard shortcuts has been disabled."

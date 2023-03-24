#!/bin/bash
usage(){
	echo ""
	echo "Usage: $0"
	echo ""
	echo "Example: $0"
	echo ""
	exit 1
}

if [ $# -ne 0 ]; then
	usage
fi

# Read current time
dateTimeStr=$(date "+%Y-%m-%d%H:%M:%S")
echo "Current date time is $dateTimeStr"

# Synchronise time with local server
sudo ntpdate -u 192.168.11.2
sudo ntpdate -u 192.168.11.3

# Give few moment to synch
sleep 7

# Set system time
dateTimeStr=$(date "+%Y-%m-%d%H:%M:%S")
echo "Setting date time to $dateTimeStr"
sudo timedatectl set-time "$dateTimeStr"

echo "Done"

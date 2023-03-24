#!/bin/bash
usage(){
        echo "Usage: $0"
        echo "Example: $0"
	exit 1
}

WORKDIR=$(dirname $0)
IMAXEON_PATH="/home/user/Imaxeon"
FIRST_BOOT_INFO_FILE="$IMAXEON_PATH/temp/first_boot.inf"
USER=$(whoami)
GROUP=$(groups |awk {'print $1'})

(sudo rm -f $FIRST_BOOT_INFO_FILE)
clear

echo ""
echo ""
echo "==========================================================================="
echo "Setup User Filesystem"
(sudo mkdir -p /IMAX_USER/db) &&
(sudo mkdir -p /IMAX_USER/log) &&
(sudo mkdir -p /IMAX_USER/temp) &&
(sudo mkdir -p /IMAX_USER/temp/upgrade) &&
(sudo mkdir /var/log/samba)
(echo "DONE")

echo ""
echo ""
echo "==========================================================================="
echo "Setup User Files Permission"
(sudo chmod -R 777 /IMAX_USER/*)
(sudo chmod -R 777 /home/user/*)
(sudo chown -R $USER:$GROUP ~/Imaxeon)
(sudo chown -R $USER:$GROUP /IMAX_USER/)
(echo "DONE")

echo ""
echo ""
echo "==========================================================================="
echo "Initiate Config Files"
(rm /IMAX_USER/db/*.PROD);
(rm /IMAX_USER/db/capabilities.cfg);
(rm /IMAX_USER/db/lastAlerts.json);
(rm /IMAX_USER/db/lastDigest.json);
(rm /IMAX_USER/db/injection_plot*);
(rm /IMAX_USER/db/shutdown.inf);

(echo "DONE")

echo ""
echo ""
echo ""
touch $FIRST_BOOT_INFO_FILE
echo "Initial Setup Done."
echo ""



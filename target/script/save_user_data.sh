#!/bin/bash

usage(){
	echo "Usage: $0 [File Prefix] [option]"
	echo "  option: '-c' for crash log"
	echo ""          
	echo "  Example: $0"
	exit 1
}

analyse_cores() {
    # each crash might result in multiple core file from different processes(?) and the name can be different
    # e.g. core.QThread, or core.Wayland..
    CORES_DIR=$1
    PREFIX=$2
    echo "HERE ${CORES_DIR} ${PREFIX}"
    for f in ${CORES_DIR}/core.*;
    do        
        baseName=`basename $f`
        name=${PREFIX}-${baseName}
        mv $f $name        
        echo Rename $f to ${name}
        echo Crash log: ${name} >> /IMAX_USER/log/lastBacktrace.log
        echo Run gdb for ${name} 
	    gdb ~/Imaxeon/bin/HCU_StellinityDistro ${name}  -ex 'thread apply all bt' -ex quit > ${name}.bt.txt  2>&1
	    cat ${name}.bt.txt >> /IMAX_USER/log/lastBacktrace.log
	    dmesg > ${name}.dmesg.txt
	    # compress the file
        echo Compressing ${name} 	    
	    tar --remove-files -czf ${name}.tar.gz ${name} 	    
	    ls -alh
    done
}


if [ $# -lt 0 ]; then
	usage
fi

VIDEO_ENABLED=0
CRASH_LOG_MODE=0
PATH_SCREEN_SHOT="/IMAX_USER/Centargo-SRU-ScreenShot.png";
FILENAME_LOGS="CENTARGO-SRU-Logs-"$(date '+%Y%m%d_%H%M%S.tar.gz');
PATH_USB=$(df |grep /media/user |awk '{for(i=6;i<=NF;i++){print $i}}')
PATH_USB=$(echo $PATH_USB | sed s/:/\\n/g) # Support name with spacing in the path
PATH_TARGET="$PATH_USB"/Centargo/Logs
PATH_LOCAL=/IMAX_USER/crashlogs

# Get option keys
for key in "$@"
do
	case $key in
	-c)
		CRASH_LOG_MODE=1
		echo >> /IMAX_USER/log/lastBacktrace.log
		echo "===========================================================" >> /IMAX_USER/log/lastBacktrace.log
		FILENAME_LOGS="CENTARGO-SRU-CrashedLogs-"$(date '+%Y%m%d_%H%M%S.tar.gz');
        cat ~/Imaxeon/doc/version.inf >> /IMAX_USER/log/lastBacktrace.log
        PREFIX="/IMAX_USER/coredump/Crash-"$(date '+%Y%m%d_%H%M%S');
        analyse_cores /IMAX_USER/coredump ${PREFIX}
		echo "Crash Log Enabled"
		;;
	esac
done

# Crash log will be saved to local if no usb found
if [ -d "$PATH_USB" ];then
	echo "USB OK"
else
	echo "FAILED TO ACCESS USB $PATH_USB"	
	if (( $CRASH_LOG_MODE == 1 )); then
        PATH_TARGET=${PATH_LOCAL}
        echo "Save to ${PATH_TARGET}"
		# Don't clear last crash log before copying to USB 
		# rm -r $PATH_TARGET
	else
		exit
	fi
fi

echo "Creating directory $PATH_TARGET"
mkdir -vp "$PATH_TARGET"

# Capture screen shot
aplay /home/user/Imaxeon/media/screenshot.wav&
import -window root -crop 1920x1200+0+0 $PATH_SCREEN_SHOT

# Capture log files
TAR_LIST="/IMAX_USER/log/*  /IMAX_USER/db/* /IMAX_USER/cru/* /IMAX_USER/info/*  /IMAX_USER/coredump/* /home/user/Imaxeon/doc/* "
if (( $CRASH_LOG_MODE == 1 )); then
    # remove /home/user/Imaxeon/* because it take too much space 300MB
	tar -czf "$PATH_TARGET/$FILENAME_LOGS" ${TAR_LIST} /var/log/syslog /var/log/kern.log
	rm /IMAX_USER/coredump/*
else 
	tar -czf "$PATH_TARGET/$FILENAME_LOGS" ${TAR_LIST}  &&	rm /IMAX_USER/coredump/*

	# Move past coredump logs to USB 
	for f in ${PATH_LOCAL}/*.*;
    do
        # copy and remove it if success
        echo Copying crash log $f
        cp $f ${PATH_TARGET} && rm $f
    done
fi

rm $PATH_SCREEN_SHOT
echo "file=$FILENAME, syncing.."
sync
aplay /home/user/Imaxeon/media/screenshot.wav
if ( test -f "$PATH_TARGET/$FILENAME_LOGS" ); then
	echo "DATA SAVE OK - $PATH_TARGET/$FILENAME_LOGS"
else 
	echo "DATA SAVE FAILED"
fi


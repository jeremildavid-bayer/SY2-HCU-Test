#!/bin/bash
usage(){
	echo ""
	echo "Usage: $0 [duration]"
	echo ""
	echo "  duration: In seconds. 0 if disabled"
	echo ""
	echo "Example: $0 60"
	echo ""
	exit 1
}

if [ $# -ne 1 ]; then
	usage
fi

# reset idle timer first
xdg-screensaver reset

SLEEP_SECONDS=$1
#gsettings set org.gnome.desktop.session idle-delay $SLEEP_SECONDS
FILE_PATH="/home/user/.xscreensaver"

if [ "$SLEEP_SECONDS" -ne 0 ]; then
    SLEEP_MINUTES=$(($SLEEP_SECONDS/60))
    if [ "$SLEEP_MINUTES" -lt 10 ]; then
        SLEEP_MINUTES="0${SLEEP_MINUTES}"
    fi
    SLEEP_SECONDS_MOD=$(($SLEEP_SECONDS%60))
    if [ "$SLEEP_SECONDS_MOD" -lt 10 ]; then
        SLEEP_SECONDS_MOD="0${SLEEP_SECONDS_MOD}"
    fi
    SLEEP_TIME="00:${SLEEP_MINUTES}:${SLEEP_SECONDS_MOD}"
    sed -i '/mode/c\mode:     blank' $FILE_PATH
    sed -i "/timeout/c\timeout:     $SLEEP_TIME" $FILE_PATH
else
    sed -i '/mode/c\mode:     off' $FILE_PATH
fi
xscreensaver-command -restart &>/dev/null

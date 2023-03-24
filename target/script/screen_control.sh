#!/bin/bash
usage(){
	echo ""
	echo "Usage: $0 [option]"
	echo "  --refresh:      scans and returns the number of screens"
	echo "   --extend:      sets the two screens to extend mode"
	echo "   --mirror:      sets the screens to mirror mode"
	echo "   --single:      sets the display to a single screen"
	echo "     --help:      display help menu"
	echo ""
	echo "Example: $0 refresh"
	echo ""
	exit 1
}

checkScreens(){
	if [ $NUMBER_OF_SCREENS -lt $MINIMUM_SCREENS ]; then
		echo "ERROR: Not Enough screens"
		exit 0
	fi
}

if [ $# -ne 1 ]; then
	usage
fi

USER_COMMAND=$1

NUMBER_OF_SCREENS=$(xrandr -q | grep connected | egrep -v disconnected | wc -l)
MINIMUM_SCREENS=2
SCREEN1_NAME=$(xrandr -q | grep connected | egrep -v disconnected | sed -n 1p | awk '{print $1 }')
SCREEN2_NAME=$(xrandr -q | grep connected | egrep -v disconnected | sed  -n 2p | awk '{print $1 }')
#echo "Screen1 = $SCREEN1_NAME"
#echo "Screen2 = $SCREEN2_NAME"


case $USER_COMMAND in 
	--refresh)
		echo "refresh screens"
		echo "screens: $NUMBER_OF_SCREENS"
		;;
	--extend)
		checkScreens
		xrandr --output $SCREEN1_NAME --auto --output $SCREEN2_NAME --auto --right-of $SCREEN1_NAME
		;;
	--mirror)
		checkScreens
#		xrandr --output $SCREEN1_NAME --auto --output $SCREEN2_NAME --auto --same-as $SCREEN1_NAME
#		OUTPUT_LINE=$(cvt 1366 768 | sed -n 2p)
#		MODELINE="$(echo ${OUTPUT_LINE//Modeline/})"
		MODELINE=""
		MODE=""
		if [ $SCREEN2_NAME == "HDMI1" ];then
			MODELINE="\"1366x768_60.00\" 85.25 1368 1440 1576 1784 768 771 781 798 -hsync +vsync"
			MODE="\"1366x768_60.00\""
		else
			MODELINE="\"1024x600_60.00\" 49.00 1024 1072 1168 1312 600 603 613 624 -hsync +vsync"
			MODE="\"1024x600_60.00\""
		fi
		xrandr --newmode $MODELINE
		xrandr --addmode $SCREEN2_NAME $MODE		
		xrandr --output $SCREEN1_NAME --auto --output $SCREEN2_NAME --mode $MODE --pos 0x0
		;;
	--single)
		checkScreens
		xrandr --output $SCREEN1_NAME --auto --output $SCREEN2_NAME --off
		;;
	--help)
		usage
		;;
	*)
		usage
		;;
esac

exit 1




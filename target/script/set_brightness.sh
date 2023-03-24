#!/bin/bash

usage(){
	echo "Usage: $0 [brightness]"
	echo "  brightness: 0-5"
	echo "  Example: $0 1"
	exit 1
}

if [ $# -ne 1 ]; then
	usage
fi

# Using acpi API
# maxVal=$(cat /sys/class/backlight/intel_backlight/max_brightness)
# echo $maxVal
# val=$((($1 * $maxVal) / 5))
# echo "setting brightness $val.."
# sudo sh -c "echo $val > /sys/class/backlight/intel_backlight/brightness"

# Using xrandr
val=$(awk "BEGIN {print $1/5*0.77 + 0.04}")
echo xrandr --output eDP-1 --gamma .8:.8:.8 --brightness $val
xrandr --output eDP-1 --gamma .8:.8:.8 --brightness $val

echo "Done"




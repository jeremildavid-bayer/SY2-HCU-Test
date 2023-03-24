#!/bin/bash

#set -x

usage(){
	echo ""
	echo "Usage: $0 [command] [interface] [link_params]"
	echo ""
	echo "  command:       connect, disconnect"
	echo "  interface:     interface name, E.g. eth0, wlan0"
	echo "  link_params:   ip/netmask/ssid/key/country_code  #NOTE: params are optional, e.g. Put empty field for dhcp"
	echo ""
	echo "Example: $0 connect enp3s0 192.168.11.1/255.255.255.0                                 ## connect enp3s0 to ethernet host with static IP address"
	echo "         $0 connect enp3s0                                                            ## connect enp3s0 to ethernet host with DHCP mode"
	echo "         $0 disconnect enp3s0                                                         ## disconnect enp3s0"
	echo "         $0 connect ath0 192.168.11.1/255.255.255.0/BAYER_CENTARGO_SNssid/PWD123/841  ## connect wlp1s0 to wifi host with static IP address with country code 841"
	echo "         $0 connect ath0 //TPPW4G_9G0D/05822730/                                      ## connect wlp1s0 to wifi host with DHCP mode"
	echo ""
	exit 1
}

# Check minimum arguments
if [ $# -lt 2 ]; then
	usage
fi

command=$1
interface=$2
params=$3

PATH_TEMP_DIR="/IMAX_USER/temp"
PATH_WPA_CONFIG="$PATH_TEMP_DIR/wpa_supplicant.conf"
PATH_CONNECTION="$PATH_TEMP_DIR/network_connection.inf"
WIFI_MODE=0
DHCP_ENABLED=0
PATH_OPEN_SOURCE_SCRIPT="/home/user/Imaxeon/script/set_network_open.sh"

#----------------------------------------------------------------------------
# FUNCTION: Reload Wifi Driver
# args: 1=CountryCode
function reloadWifiDriver {
	countryCode=$1
	
	sudo killall wpa_supplicant
	# Avalue: Kill wpa_supplicant PID
	sudo ps -ef | grep wpa | grep -v grep | awk '{print $2}' | sudo xargs kill 2>/dev/null

	echo "Stop network-manager to manually configure the network interface."
	sudo service network-manager stop
	echo 8 | sudo tee /proc/sys/kernel/printk	

	if lsmod | grep "ath9k" &> /dev/null ; then
		echo "Unloading Default (Ath9k) driver.."
		sudo rmmod ath9k
		sudo rmmod ath9k_common
		sudo rmmod ath9k_hw
		sudo rmmod ath
		sudo rmmod mac80211
		echo "done "
	fi

	# Unload Silex driver
	if iwconfig "ath0" &> /dev/null ; then
		sudo wlanconfig ath0 destroy
		echo "Unloading Silex driver.."
		sudo rmmod umac
		sudo rmmod ath_dev
		sudo rmmod ath_dfs
		sudo rmmod ath_rate_atheros
		sudo rmmod ath_hal
		sudo rmmod asf
		sudo rmmod adf
		echo "done "
		sleep 1
	fi

	# Reload Silex Driver
	echo "Reloading Silex Driver.. (CountryCode=$countryCode)"
	sudo insmod /lib/firmware/pcean2/adf.ko
	sudo insmod /lib/firmware/pcean2/asf.ko
	sudo insmod /lib/firmware/pcean2/ath_hal.ko
	sudo insmod /lib/firmware/pcean2/ath_rate_atheros.ko
	sudo insmod /lib/firmware/pcean2/ath_dfs.ko
	sudo insmod /lib/firmware/pcean2/ath_dev.ko
	sudo insmod /lib/firmware/pcean2/umac.ko ath_cc=$countryCode
	sudo wlanconfig ath0 create wlandev wifi0 wlanmode sta &> /dev/null 
	echo "done"
}

#----------------------------------------------------------------------------
# FUNCTION: Disconnect an interface
# args: 1=interface
function disconnectInterface {
	paramIf=$1
	(sudo ip addr flush dev $paramIf) &&
	(sudo ip link set $paramIf down)
}

#----------------------------------------------------------------------------
# FUNCTION: Restart network services
# args: 1=if
function killNetwork {
    # don't touch the dev interface if defined in /IMAX_USER/db/dev.cfg
    devIf=""
    if [ -f /IMAX_USER/db/dev.cfg ]; then
         devIf=`cat /IMAX_USER/db/dev.cfg | grep "#interface: " | cut -d " " -f 2`
         echo "In development mode. Keep the network interface $devIf"
    fi
	while read iface
	do
		if [ "$iface" != "lo" ] && [ "$iface" != "$devIf" ]; then
			echo "disconnectInterface $iface"
			disconnectInterface $iface
		fi
	done < <(ls /sys/class/net)
}

#----------------------------------------------------------------------------
# FUNCTION: MAIN

# Save connection info - don't save in release
# echo "$command $interface $params" > $PATH_CONNECTION

# Prepare option
IFS='/'
read -a optArray <<< "$params"

optIp=${optArray[0]}
optNetMask=${optArray[1]}
optSsid=${optArray[2]}
optKey=${optArray[3]}
optCountryCode=${optArray[4]}

if [ "$optCountryCode" == "" ]; then
	# Use CountryCode for 'US' as default
	optCountryCode=841 
fi

# Check if the Country code is a number, if not then use open source script
re='^[0-9]+$'
if ! [[ $optCountryCode =~ $re ]]; then
    "$PATH_OPEN_SOURCE_SCRIPT" $command $interface "$params"
    exit 0
fi

echo "Option Params: IP=$optIp, NetMask=$optNetMask, SSID=$optSsid, CountryCode=$optCountryCode"
if [ -z $optIp ]; then
	DHCP_ENABLED=1
fi

if [ -n "$optSsid" ]; then
	WIFI_MODE=1
fi


# Handle command options
case $command in
	# ----------------------------------------------
	connect)

	# Clear any IP Addess from the current device
	(sudo ip addr flush dev $interface) &&
	(sleep 1)
	killNetwork
	
	if (( $WIFI_MODE == 1 )); then
		# Reload Driver
		reloadWifiDriver "$optCountryCode"
	
		# Release wifi soft block (some wifi chips require)
		(sudo rfkill unblock wifi; sudo rfkill unblock all);
		
		# Setup wpa_supplicant.conf
		if [ -n "$optSsid" ] && [ -n "$optKey" ]; then
		        echo ctrl_interface=/run/wpa_supplicant > "$PATH_WPA_CONFIG"
		        # ap_scan=2 does not work
		        #echo ap_scan=2 >> "$PATH_WPA_CONFIG"
			wpa_passphrase $optSsid $optKey | grep -v "#" >> "$PATH_WPA_CONFIG"
			sudo wpa_supplicant "-c$PATH_WPA_CONFIG" -Dathr "-i$interface" -B
		else
			usage
		fi
	fi

	if (( $DHCP_ENABLED == 1 )); then
		echo "Setting interface with DHCP.."
		sudo dhclient $interface
	else
		echo "Setting interface with static IP.."
		sudo ip address add $optIp/24 dev $interface
		sudo ip link set $interface up
	fi
	
	sudo ip link set lo up
	
	echo ""
	echo "----------------------------------------"
	echo "Network interfaces:"
	ip address show up
	echo "----------------------------------------"
	;;
	# ----------------------------------------------
	disconnect)
	disconnectInterface "$interface"
	;;
	# ----------------------------------------------
	*)
	echo "Unknown Command: $command"
	usage
	;;
esac


echo "Interface Status:"
ip r | grep $interface

echo "NETWORK SETUP COMPLETED"

exit 0



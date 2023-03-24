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

#echo $* >> /home/user/wifi.txt

command=$1
interface=$2
params=$3

PATH_TEMP_DIR="/IMAX_USER/temp"
PATH_TEMP_LOOPBACK_FILE="$PATH_TEMP_DIR/interface.temp"
PATH_TEMP_INTERFACE_FILE="$PATH_TEMP_DIR/interface_file.temp"
WIFI_MODE=0
DHCP_ENABLED=0


#----------------------------------------------------------------------------
# FUNCTION: Restart network services
# args: 1=if
function restartNetwork {
	paramIf=$1
	sleep 2
	
	# Avalue: Kill wpa_supplicant PID
	# sudo ps -ef | grep wpa | grep -v grep | awk '{print $2}' | sudo xargs kill 2>/dev/null

	# Braetec try different networking command restart
	echo -e "Stopping network.."
	sudo invoke-rc.d network-manager stop
	echo -e "Starting network.."
	sudo invoke-rc.d network-manager start 
	sudo systemctl restart network-manager.service
	sleep 2
}

#----------------------------------------------------------------------------
# FUNCTION: Set link - static
# args: 1=interface, 2=ip, 3=netmask, 4=ssid, 5=key
function setLinkStatic {
	paramIf=$1
	paramIp=$2
	paramNetMask=$3
	paramSsid=$4
	paramKey=$5
	paramName=$6
	paramCountryCode=$7

	if (( $WIFI_MODE == 1 )); then
		# WIFI MODE
		# echo "sudo nmcli connection add type wifi ifname $paramIf con-name $paramName autoconnect yes ssid $paramSsid 802-11-wireless.band bg 802-11-wireless-security.key-mgmt wpa-psk 802-11-wireless-security.psk $paramKey ipv4.addresses $paramIp/24 ipv4.method manual"
		sudo nmcli connection add type wifi ifname $paramIf con-name $paramName autoconnect yes ssid $paramSsid 802-11-wireless-security.key-mgmt wpa-psk 802-11-wireless-security.psk $paramKey ipv4.addresses $paramIp/24 ipv4.method manual
		sudo iw reg set $paramCountryCode
	else
		# echo "sudo nmcli connection add type 802-3-ethernet ifname $paramIf con-name $paramName ipv4.addresses $paramIp/24 ipv4.method manual"
		sudo nmcli connection add type 802-3-ethernet ifname $paramIf con-name $paramName ipv4.addresses $paramIp/24 ipv4.method manual
	fi

}

#----------------------------------------------------------------------------
# FUNCTION: Disconnect an interface
# args: 1=interface
function disconnectInterface {
	paramIf=$1
	sudo ifconfig $paramIf down
}


#----------------------------------------------------------------------------
# FUNCTION: MAIN

#Prepare option
IFS='/'
read -a optArray <<< "$params"

optIp=${optArray[0]}
optNetMask=${optArray[1]}
optSsid=${optArray[2]}
optKey=${optArray[3]}
optCountryCode=${optArray[4]}

conName="CRU-ETH"

if [ "$optCountryCode" == "" ]; then
	# Use CountryCode for 'US' as default
	optCountryCode=US 
fi

echo "Option Params: IP=$optIp, NetMask=$optNetMask, SSID=$optSsid, CountryCode=$optCountryCode"

if [ -z $optIp ]; then
	DHCP_ENABLED=1
fi

if [ -n "$optSsid" ]; then
	WIFI_MODE=1
fi

if (( $WIFI_MODE == 1 )); then
	# Reload Driver
	#reloadWifiDriver "$optCountryCode"
	conName=$optSsid
	
	# Release wifi soft block (some wifi chips require)
	(sudo rfkill unblock wifi; sudo rfkill unblock all);
	(sudo ip link set $interface down &>/dev/null);
fi

# Create temp interface file

# Handle command options
case $command in
	# ----------------------------------------------
	connect)
       
    #make sure the network manager name set
    sudo service network-manager start
    sudo nmcli radio wifi on 
    sudo ifconfig wlp1s0 down &>/dev/null
    sudo ip link set wlp1s0 name ath0 &>/dev/null
    
	#ed:  Delete network connection with current interface
	currentConns=$(sudo nmcli -t -f NAME,DEVICE c show | grep $interface | cut -d : -f1)
	IFS=$'\n'
	for connection in $currentConns; do
	    # echo "connection=$connection"
	    sudo nmcli connection delete $connection 2>/dev/null
	done
	# delete network connections with no device
	currentConns=$(sudo nmcli -t -f NAME,DEVICE c show | grep ":$" | cut -d : -f1)
	IFS=$'\n'
	for connection in $currentConns; do
	    # echo "connection=$connection"
	    sudo nmcli connection delete $connection 2>/dev/null
	done

	# Disconnect interface and clear any IP Addess from the current device
	#nmcli device disconnect $interface 2>/dev/null
	(sudo ip addr flush dev $interface) &&
	(sleep 5) # remain disconnected for 5 seconds to force CRU to disconnect


	if (( $DHCP_ENABLED == 1 )); then
		echo "Setting interface with DHCP.."
		if (( $WIFI_MODE == 1 )); then
		    sudo nmcli connection delete $conName 2>/dev/null
			(sudo nmcli connection add type wifi ifname $interface con-name $conName autoconnect yes ssid $optSsid 802-11-wireless-security.key-mgmt wpa-psk 802-11-wireless-security.psk $optKey ipv4.method auto) && 
			(sudo nmcli connection up $conName)
		else
			sudo dhclient $interface
		fi
	else
		echo "Setting interface with static IP.."
		setLinkStatic "$interface" "$optIp" "$optNetMask" "$optSsid" "$optKey" "$conName" "$optCountryCode"
		sudo nmcli connection up $conName &> /dev/null
	fi
	
	restartNetwork "$interface"
	echo ""
	
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

sleep 2
# echo "Interface Status:"
# sudo nmcli -p device show $interface

echo "NETWORK SETUP COMPLETED"

exit 0




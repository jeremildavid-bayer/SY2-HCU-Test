#!/usr/bin/sh
usage(){
    echo ""
    echo Set development enviroment to keep the reserved interface alive ${devIf} with ${defIP}
    echo "Usage:"
    echo "    $0 on|vm_as_hcu|vm_as_host|off"
    echo "    $0 on         - Remote debug with HCU with 2nd network, VM(192.168.11.51)<-wired-> (192.168.11.50) HCU <-wired/wireless-> CRU, "
    echo "    $0 vm_as_host - VM as debug host VM (192.168.11.51) as above"
    echo "    $0 vm_as_hcu  - VM as HCU  VM (192.168.11.1) <-wired/wireless-> CRU"
    echo "    $0 off        - turn off debugging (default)"
    exit 1
}


# Check minimum arguments
if [ $# -lt 1 ]; then
    usage
fi


generate_script(){
    devIf=$1
    devIP=$2
    echo Set HCU wired address for ${devIf} to ${devIP}
    echo "#interface: ${devIf}"                              > /IMAX_USER/db/dev.cfg
    echo "export devIf=${devIf}"                            >> /IMAX_USER/db/dev.cfg
    echo "sudo ip link set ${devIf} down"                   >> /IMAX_USER/db/dev.cfg
    echo "sudo ip address add  ${devIP}/24 dev ${devIf}"    >> /IMAX_USER/db/dev.cfg
    echo "sudo ip link set ${devIf} up"                     >> /IMAX_USER/db/dev.cfg
    echo "ip a show up"                                     >> /IMAX_USER/db/dev.cfg
    echo Generated /IMAX_USER/db/dev.cfg
    cat /IMAX_USER/db/dev.cfg
    # and run it
    chmod +x /IMAX_USER/db/dev.cfg
    /IMAX_USER/db/dev.cfg
}

HCU_IF=enp4s0
VM_IF=ens38

if [ "$1" = "on" ]; then
    echo Assuming this is the HCU. Setup for remote debugging.
    if [-f /IMAX_USER/db/dev.cfg ]; then
        echo "Already in debug mode"
    else
        generate_script ${HCU_IF} 192.168.88.50
    fi
elif [ "$1" = "vm_as_host" ]; then
    echo "Assuming this is on VM. VM as debug host VM (192.168.11.51)"
    generate_script ${VM_IF} 192.168.88.51
    sudo route add -net 192.168.11.0 netmask 255.255.255.0 gw 192.168.88.50
elif [ "$1" = "vm_as_hcu" ]; then
    echo Assuming this is on VM. VM will pretent to be HCU to connect through wire connection with CRU
    generate_script ${VM_IF} 192.168.11.1
elif [ $1 = "off" ]; then
    echo Remove the dev.cfg file. Restart the machine to take effect
    if [ -f /IMAX_USER/db/dev.cfg ]; then
        rm /IMAX_USER/db/dev.cfg
        sudo ip link set ${devIf} down
    fi
else
    usage
fi


#!/bin/bash

if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root"
    exit 1
fi

if ! mkdir -p /etc/daikonSplash ; then
    echo "Failed to create etc directory!"
    exit 1
fi

echo "Copying essential files..."

if cp ./daikon.png /etc/daikonSplash/daikon.png && chmod 644 /etc/daikonSplash/daikon.png ; then
    echo "Copied splash logo..."
else
    echo "Failed to copy splash logo!"
    exit 1
fi

if make ; then
    echo "Compiled binaries..."
else
    echo "Failed to compile binaries!"
    exit 1
fi

if cp ./main /bin/daikon_splash ; then
    echo "Copied binaries..."
else
    echo "Failed to copy binaries!"
    exit 1
fi

if cp ./scripts/hook.build /etc/initcpio/install/daikon_splash && chmod 644 /etc/initcpio/install/daikon_splash ; then
    echo "Copied build script..."
else
    echo "Failed to copied build script!"
    exit 1
fi

if cp ./scripts/hook.run /etc/initcpio/hooks/daikon_splash && chmod 644 /etc/initcpio/install/daikon_splash ; then
    echo "Copied runtime script..."
else
    echo "Failed to copied runtime script!"
    exit 1
fi

if ! grep -q "daikon_splash" /etc/mkinitcpio.conf ; then
    
    line=$(grep -n "^HOOKS=(base udev*" /etc/mkinitcpio.conf | cut -f1 -d:)
    text=$(grep "^HOOKS=(base udev*" /etc/mkinitcpio.conf)
    delimiter="udev "
    string=$text$delimiter
    
    myarray=()
    while [[ $string ]]; do
        myarray+=( "${string%%"$delimiter"*}" )
        string=${string#*"$delimiter"}
    done
    
    new_line=${myarray[0]}$delimiter"daikon_splash "${myarray[1]}
    
    if sed -i $line's/.*/'"$new_line"'/' /etc/mkinitcpio.conf; then
        echo "Added splash to hooks..."
    else
        echo "Failed add splash to hooks!"
        exit 1
    fi
else
    echo "Skipped adding to config file... (already configured)"
fi

echo "Creating ramdisk..."
if mkinitcpio -p linux > /dev/null ; then
    echo "Created ramdisk..."
else
    echo "Failed to create ramdisk!"
    exit 1
fi
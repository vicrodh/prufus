#!/bin/sh
echo "Creating usb...."
echo $1
echo $2

echo "Creating temporal directories.."
mkdir -p /tmp/prufus/iso
mkdir -p /tmp/prufus/usb

echo "Mounting USB and ISO"
mount -t udf -o loop $1 /tmp/prufus/iso
mount $2"1" /tmp/prufus/usb

echo "Copying regular installtion files.."
rsync -v -r -I --no-links --no-perms --no-owner --no-group --exclude sources/install.wim /tmp/prufus/iso/ /tmp/prufus/usb

echo "Copying big installation file.."
wimlib-imagex split /tmp/prufus/iso/sources/install.wim /tmp/prufus/usb/sources/install.wim 1024

echo "Syncronizing disk..."
time(sync)

echo "Synconization done"


echo "Cleaning..."
umount /tmp/prufus/iso
umount /tmp/prufus/usb

rm -r /tmp/prufus

echo "Booteable USB created!"
#watch -d grep -e Dirty: -e Writeback: /proc/meminfo

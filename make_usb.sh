#!/bin/sh
echo "Creating usb...."

echo 1 > /tmp/prufus/status

echo $1
echo $2

echo "Creating temporal directories.."
echo 2 > /tmp/prufus/status
mkdir -p /tmp/prufus/iso
mkdir -p /tmp/prufus/usb

echo "Mounting USB and ISO"
echo 3 > /tmp/prufus/status
mount -t udf -o loop $1 /tmp/prufus/iso
mount $2"1" /tmp/prufus/usb

rm -r /tmp/prufus/usb/* #temporal format

echo "Copying regular installtion files.."
echo 4 > /tmp/prufus/status
rsync -v -r -I --no-links --no-perms --no-owner --no-group --exclude sources/install.wim /tmp/prufus/iso/ /tmp/prufus/usb

touch /tmp/prufus/bigfile-info
echo "Copying big installation file.."
echo 5 > /tmp/prufus/status
nohup wimlib-imagex split /tmp/prufus/iso/sources/install.wim /tmp/prufus/usb/sources/install.wim 1024 > /tmp/prufus/bigfile-info

echo "Syncronizing disk..."
echo 6 > /tmp/prufus/status
time(sync)

echo "Synconization done"


echo "Cleaning..."
echo 7 > /tmp/prufus/status
umount /tmp/prufus/iso
umount /tmp/prufus/usb

echo "Booteable USB created!"
echo 8 > /tmp/prufus/status

rm -r /tmp/prufus

#watch -d grep -e Dirty: -e Writeback: /proc/meminfo

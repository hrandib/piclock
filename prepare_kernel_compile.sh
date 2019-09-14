#! /usr/bin/sh

# https://elinux.org/Raspberry_Pi_Kernel_Compilation
# https://github.com/notro/rpi-source/wiki

sudo -i
apt -y install raspberrypi-kernel-headers
apt -y install gcc make bc screen ncurses-dev git
apt-cache policy raspberrypi-kernel

#get installed version from https://github.com/raspberrypi/linux/releases/ e.g:

# cd /usr/src
# wget https://github.com/raspberrypi/linux/archive/raspberrypi-kernel_1.20190819-1.tar.gz
# KSRC="$(basename $_)"
# tar -xzf $KSRC
# ln -sf "/usr/src/${KSRC%.tar.gz} /lib/modules/$(uname -r)/build
# cd $_

#https://www.raspberrypi.org/forums/viewtopic.php?t=195965
modprobe configs
zcat /proc/config.gz > .config

#https://www.raspberrypi.org/documentation/linux/kernel/building.md

export KERNEL=kernel7
make bcm2709_defconfig
# enable building of BH1750 kernel module
 echo "CONFIG_BH1750=m" >> .config
# or make menuconfig
make prepare
make modules_prepare
make SUBDIRS=drivers/iio/light/ modules

# Enabling device
cp -f drivers/iio/light/bh1750.ko /lib/modules/$(uname -r)/kernel/drivers/iio/light/
depmod
modprobe bh1750
echo "bh1750" >> /etc/modules
echo "bh1750 0x23" > /sys/bus/i2c/devices/i2c-1/new_device

#echo "bmp280 0x76" > /sys/bus/i2c/devices/i2c-1/new_device
#echo "hdc1080 0x40" > /sys/bus/i2c/devices/i2c-1/new_device
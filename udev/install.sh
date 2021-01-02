#!/bin/sh

sudo cp 52-hackrf.rules /etc/udev/rules.d/.
sudo usermod -G plugdev -a $USER
sudo udevadm control --reload-rules

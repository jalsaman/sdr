#!/bin/sh

sudo cp *.rules /etc/udev/rules.d/.
sudo usermod -G plugdev -a $USER
sudo udevadm control --reload-rules

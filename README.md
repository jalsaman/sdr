# Software Defined Radio (SDR) Code


To install **GNU Radio** on Ubuntu use the following steps:

```bash
sudo apt install gnuradio
sudo apt install libcanberra-gtk-module libcanberra-gtk3-module
```

To install **Osmocom RTL2382U** Driver

```bash
git clone https://github.com/osmocom/rtl-sdr.git
mkdir build
cd build
cmake ../ -DINSTALL_UDEV_RULES=ON -DDETACH_KERNEL_DRIVER=ON
make
sudo make install
sudo ldconfig
```

To install **Osmocom GNU Radio Blocks** from source:

```bash
git clone git://git.osmocom.org/gr-osmosdr
cd gr-osmosdr/

mkdir build
cd build/
cmake ../

make
sudo make install
sudo ldconfig
```

To build API documentation:

```bash
cd build/
cmake ../ -DENABLE_DOXYGEN=1
make -C docs
```

To install the **Osmocom GNU Radio Source** Module:

```bash
sudo apt install gr-osmosdr
```
To install **HackRF**

```bash
sudo apt install hackrf
```
Building **HackRF tools from source**

```bash
git clone https://github.com/mossmann/hackrf.git
cd hackrf/host
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
```

To install **GQRX-SDR**

```bash
sudo apt install gqrx-sdr
```


To install **GQRX-SDR** from source

```bash
sudo apt install qt5-default
sudo apt install libqt5svg5

git clone https://github.com/csete/gqrx.git gqrx
cd gqrx
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig

gqrx --version
```
To install **RFCAT** from source

```bash
sudo apt install sdcc

git clone https://github.com/atlas0fd00m/rfcat.git
cd rfcat
sudo python setup.py install
```

To **allow non-root dongle access**
```bash
sudo cp etc/udev/rules.d/20-rfcat.rules /etc/udev/rules.d
sudo udevadm control --reload-rules
```

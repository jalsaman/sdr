# GNU Radio Code


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

to Build API documentation:

```bash
cd build/
cmake ../ -DENABLE_DOXYGEN=1
make -C docs
```

to install the **Osmocom GNU Radio Source** Module:

```bash
apt-get install gr-osmosdr
```

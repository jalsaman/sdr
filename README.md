# GNU Radio Code

To install GNU Radio on Ubuntu use the following steps:

```bash
sudo apt install gnuradio
sudo apt install libcanberra-gtk-module libcanberra-gtk3-module
```

To install osmocom Gnu Radio Blocks

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

to Build API documentation

```bash
cd build/
cmake ../ -DENABLE_DOXYGEN=1
make -C docs
```


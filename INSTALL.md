# Install museekd

Requirements:\
libxml++2.6-dev or libxml++1.0-dev\
libevent >= 1.3e\
GCC\
Python (by Python bindings)\
pyexpat (for musetup)\
CMake >= 2.8.3 (for building)\
QT >= 5 for museeq

Optional:\
libvorbis-dev\
libogg-dev\
SWIG (for the mucipher Python bindings)\

On Debian:
```shell
apt-get install swig pkg-config libxml2-dev libevent-dev libxml++2.6-dev qttools5-dev qtscript5-dev
```

Get binaries and help for Museek+ here: http://www.museek-plus.org


## Install using CMake

CMake is the recommended way to build everything

### Configure Features

Prefix the boolean variables with -D, set them to 1 to enable the feature, 0 to disable.\
Default values are between brackets.

#### Features
`EVERYTHING` (0): Enable all features.\
`MUSEEKD` (1): Build museekd soulseek daemon.\
`MUSETUP` (1): Build musetup configuration interface for museekd.\
`MUSEEQ` (1):  Build museeq Qt client.\
`MUSCAN` (1):  Build muscan shared file index generator.\
`MUCOUS` (0):  Build mucous curses client.\
`MURMUR` (0):  Build murmur PyGTK client.\
`PYMUCIPHER` (0):      Generate PyMucipher bindings.\
`PYTHON_BINDINGS` (0): Generate python bindings.\
`PYTHON_CLIENTS` (0):  Build python clients (mulog, museekchat, museekcontrol, musirc).\

#### Museeq options
  `BINRELOC` (0): Use binary relocation.\
  `RELOAD_TRANSLATIONS` (0): Update .ts files in src/museeq/translations.

#### Install Location

To customize the installation location, different variables can be set. For an exhaustive list please see the GNUInstallDirs module documentation which is part of cmake.

Examples:
`CMAKE_INSTALL_PREFIX`: Where museek+ should be installed (default is /usr/local)\
`CMAKE_INSTALL_MANDIR`: Where man files should be installed (default is share/man)\
`CMAKE_INSTALL_DATADIR`: Where data files should be installed (default is share). Museek will put it's data in a museek/ subfolder relative to this.\
Relative paths are appended to `CMAKE_INSTALL_PREFIX`. Absolute paths are also possible.

### Build Commands

```shell
$ cd /path/to/src
$ mkdir build/
$ cd build/
$ cmake -CMAKE_INSTALL_DPREFIX=/usr ..
(or) $ cmake -DEVERYTHING=1 -DCMAKE_INSTALL_PREFIX=/usr ..
(or) $ cmake -DMUCOUS=1 -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_MANDIR=/usr/share/man ..
$ make
(or) $ make VERBOSE=1
$ sudo make install
```

## Distutils (Python's built-in installation tool)

distutils doesn't provide for uninstalls, so be cautious with it.

Based on the desired feature, `cd` to the appropriate directory and issue the build commands.

### Feature Directories

PyMucipher (requires SWIG, Python)
```shell
$ cd Mucipher/PyMucipher/
```
Python Bindings
```shell
$ cd python-bindings/
```
Python Clients (mulog, museekchat, museekcontrol, musirc.py)\
Requires: PyMucipher or PyCrypto, Python Bindings
```shell
# cd python-clients/
```
Setup tools (musetup, musetup-gtk, musetup-qt)
```shell
# cd setup/
```
Mucous\
Requires: PyMucipher or PyCrypto, Python Bindings
```shell
# cd mucous/
```

### Build Commands

```shell
$ python setup.py build
$ sudo python setup.py install
(or) $ sudo python setup.py install --prefix=/usr/local
```

# Default config for museek
import os

# Build to build-arch-os dir
BUILDDIR=1

# Use optimizations (GCC only)
RELEASE=0

# Enable profiler build (GCC only)
PROFILE=0

# Install prefix
PREFIX='/usr/local'

# Where to install binaries
BINDIR=os.path.join('${PREFIX}','bin')

# Where data files are install to
DATADIR=os.path.join('${PREFIX}','share')

# Where man pages are installed
MANDIR=os.path.join('${PREFIX}','man', "man1")

# Debugging verbosity (debug, cycle, calltrace)
MULOG='debug'

# CCFLAGS (fPIC, Wall, pipe) Do not include the prefix "-"
CFLAGS='fPIC,Wall,pipe'

# Enable OGG Vorbis support (if available) in file scanner (muscan)
VORBIS=1

# Use epoll if available (linux 2.6+)
EPOLL=1

# Use relaytool?
RELAY=0

# Use binary relocation
BINRELOC=os.path.exists('/proc/self/maps')

# Install Mucous
MUCOUS=1

# Install Musetup-GTK
MUSETUPGTK=1

# Build the Qt GUI
MUSEEQ=1

# Only Build the Qt GUI and Mucipher
ONLYMUSEEQ=0

# Build translations for Museeq
MUSEEQTRANSLATIONS='fr,de,it,es,pl,pt_BR'

# Use QSA if available
QSA=1

# Relay QSA, WARNING: tends to require $QTDIR/lib to be in LD_LIBRARY_PATH
RELAY_QSA=0

# Where Qt is installed (should have lib and include subdirs, defaults to
# QTDIR environment option
QTDIR=''

# Which Qt library to use (qt or qt-mt)
QT_LIB='qt'

# Which moc (Qt meta object compiler) to use
QT_MOC=os.path.join('$QTDIR', 'bin', 'moc')

# Which uic (Qt user interface compiler) to use
QT_UIC=os.path.join('$QTDIR', 'bin', 'uic')

# Horrible hack to statically link against libxml++
LIBXMLPP_STATIC=0

# Import local settings from previous build
try:
    exec(open("mulocal.py").read())
except IOError:
    pass

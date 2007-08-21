# Copyright (C) 2007, Adam Cécile (Le_Vert) <gandalf@le-vert.net>
#
# This program is free software. It comes without any warranty, to
# the extent permitted by applicable law. You can redistribute it
# and/or modify it under the terms of the Do What The Fuck You Want
# To Public License, Version 2, as published by Sam Hocevar.

# DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
# Version 2, December 2004 
#                          
# Copyright (C) 2004 Sam Hocevar 
# 22 rue de Plaisance, 75014 Paris, France 
# Everyone is permitted to copy and distribute verbatim or modified 
# copies of this license document, and changing it is allowed as long 
# as the name is changed. 
#                                
# DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
# TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION 
#                                                 
# 0. You just DO WHAT THE FUCK YOU WANT TO. 
#
# You could choose, at your convenience, WTFPL 2 license or 3-clause
# BSD-style license.


# Path to the GTK+ python types, GTK+/PyGTK gcc flags and libraries to link with
DEFS	 = $(shell pkg-config --variable=defsdir pygtk-2.0)
GTKFLAGS = $(shell pkg-config --cflags gtk+-2.0 pygtk-2.0)
GTKLIBS	 = $(shell pkg-config --libs gtk+-2.0 pygtk-2.0)

# Optional flags you may use
CFLAGS   = -Wall -g -O2
PYTHON   = python
CC	 = gcc
INSTALL	 = install
PREFIX   = /usr/local
DESTDIR  = /

# Get python version, python includes dir and add required flags
PYVERS	 = $(shell $(PYTHON) -c "import sys; print sys.version[:3]")
PYPREFIX = $(shell $(PYTHON) -c "import sys; print sys.prefix")
PYINC    = $(PYPREFIX)/include/python$(PYVERS)
REQFLAGS = -I$(PYINC) -I. -fPIC

SRCS = trayicon.c trayiconmodule.c eggtrayicon.c
OBJS = $(SRCS:.c=.o)

all: trayicon.so

# Build python trayicon module
trayicon.so: $(OBJS)
	$(CC) $(GTKLIBS) $(CFLAGS) -shared $^ -o $@

# Build all objs
$(OBJS): $(SRCS)
	$(CC) $(GTKFLAGS) $(CFLAGS) $(REQFLAGS) -c $^

# Generate the C wrapper from the defs and our override file
trayicon.c: trayicon.defs trayicon.override
	pygtk-codegen-2.0 --prefix trayicon \
	--register $(DEFS)/gdk-types.defs \
	--register $(DEFS)/gtk-types.defs \
	--override trayicon.override \
	trayicon.defs > $@

# Install module...
install: trayicon.so
	$(INSTALL) -d $(DESTDIR)/$(PREFIX)/lib/python$(PYVERS)/site-packages/pymurmur
	$(INSTALL) -m 644 trayicon.so $(DESTDIR)/$(PREFIX)/lib/python$(PYVERS)/site-packages/pymurmur

# A rule to clean the generated files
clean:
	rm -f trayicon.so *.o trayicon.c *~

distclean: clean

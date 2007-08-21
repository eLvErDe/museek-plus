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


# Optional flags you may use
CFLAGS   = -Wall -g -O2
PYTHON   = python
CC       = gcc
INSTALL  = install
PREFIX   = /usr/local
DESTDIR  = /

all: build

build: build-python build-trayicon

build-python: setup.py
	$(PYTHON) setup.py build

# Build trayicon python module and pass all variables to its Makefile
build-trayicon: trayicon/Makefile
	$(MAKE) -C trayicon CFLAGS="$(CFLAGS)" \
	                    PYTHON="$(PYTHON)" \
	                    PREFIX="$(PREFIX)" \
			    DESTDIR="$(DESTDIR)" \
			    INSTALL="$(INSTALL)" \
			    CC="$(CC)"

install: install-python install-trayicon

install-python:
	$(PYTHON) setup.py install --prefix $(PREFIX) \
				   --root $(DESTDIR)
install-trayicon:
	$(MAKE) -C trayicon install CFLAGS="$(CFLAGS)" \
	                    PYTHON="$(PYTHON)" \
	                    PREFIX="$(PREFIX)" \
			    DESTDIR="$(DESTDIR)" \
			    INSTALL="$(INSTALL)" \
			    CC="$(CC)"

clean: clean-python clean-trayicon

clean-python:
	$(PYTHON) setup.py clean
	find . -name \*.pyc -exec rm -f {} \;
	rm -rf build/

clean-trayicon:
	$(MAKE) -C trayicon clean

// MooDriver - (C) 2006 M.Derezynski
//
// Code based on;
//
// ---
//
// giFTcurs - curses interface to giFT
// Copyright (C) 2001, 2002, 2003 Göran Weinholt <weinholt@dtek.chalmers.se>
// Copyright (C) 2003 Christian Häggström <chm@c00.info>
//

#ifndef _XCOMMON_H
#define _XCOMMON_H

int xconnect_unix (const char *path);
int xconnect_ip   (const char *host, const char *port);

#endif

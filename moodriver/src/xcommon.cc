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

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <glib.h>

#include "xcommon.hh"

int xconnect_unix (const char *path)
{
#ifdef AF_UNIX
	int sock;
	struct sockaddr_un saddr;

	if (path[0] != '/')
		return -1;
	/* This is a unix domain socket. */

	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
	  	g_critical("socket(): %s (%d)", g_strerror(errno), errno);
    }

	saddr.sun_family = AF_UNIX;
	g_strlcpy(saddr.sun_path, path, sizeof saddr.sun_path);

	if (connect(sock, (struct sockaddr *) &saddr, sizeof saddr) < 0)
  {
		g_message ("connect(): %s (%d)", g_strerror(errno), errno);
		close(sock);
		return -1;
	}
	return sock;
#else
	return -1;
#endif
}

int xconnect_ip (const char *hostname, const char *port)
{
	int sock;

#ifdef HAVE_GETADDRINFO
	struct addrinfo *res, *res0, hints = { 0, AF_UNSPEC, SOCK_STREAM };
	int e;

	if ((e = getaddrinfo(hostname, port, &hints, &res0))) {
# ifdef HAVE_GAI_STRERROR
		g_critical("getaddrinfo(%s:%s): %s (%d)", hostname, port, gai_strerror(e), e);
# else
		g_critical("getaddrinfo(%s:%s): Something wicked happened while resolving (%d)", hostname,
				   port, e);
# endif
	}

	for (sock = -1, res = res0; res; res = res->ai_next) {
		sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sock < 0)
			continue;

		if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
			close(sock);
			sock = -1;
			continue;
		}
		break;
	}
	freeaddrinfo(res0);

	if (sock < 0)
		g_message ("socket() or connect(): %s (%d)", g_strerror(errno), errno);
#else							/* No getaddrinfo -> fallback to IPv4 only */
	struct hostent *host;
	struct sockaddr_in saddr;
	struct servent *serv;

	/* This function fails in cygwin and on Tru64. */
	if ((serv = getservbyname(port, NULL)))
		saddr.sin_port = serv->s_port;	/* Already in network byteorder */
	else
		saddr.sin_port = htons(atoi(port));

	if (!(host = gethostbyname(hostname))) {
# ifdef HAVE_HSTRERROR
		g_critical("%s: %s (%d)", hostname, hstrerror(h_errno), h_errno);
# else
		g_critical("%s: Could not resolve hostname (%d)", hostname, h_errno);
# endif
	}

	memcpy(&saddr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
	saddr.sin_family = AF_INET;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		g_critical("socket(): %s (%d)", g_strerror(errno), errno);

	if (connect(sock, (struct sockaddr *) &saddr, sizeof saddr) < 0) {
		g_message ("connect(): %s (%d)", g_strerror(errno), errno);
		close(sock);
		return -1;
	}
#endif
	return sock;
}

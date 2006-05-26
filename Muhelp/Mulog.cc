/* Muhelp - Helper library for Museek
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <system.h>

#include <Muhelp/Mulog.hh>

void Mulog::operator() (const std::string& _class, const char* fmt, ...) {
	va_list ap;
	int n, size = 1024;
	char *p;

	if((p = (char *)malloc(size)) == NULL)
		return;

	while(1) {
		va_start(ap, fmt);
		n = vsnprintf(p, size, fmt, ap);
		va_end(ap);
		if(n > -1 && n < size) {
			std::list<MulogSink *>::iterator it = sinks.begin();
			for(; it != sinks.end(); ++it)
				(*it)->log(_class, p);
			free(p);
			return;
		}
		if(n > -1)
			size = n+1;
		else
			size *= 2;
		if((p = (char *)realloc(p, size)) == NULL)
			return;
	}
}

Mulog mulog;

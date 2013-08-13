/*  BMPx - The Dumb Music Player
 *  Copyright (C) 2005 BMPx development team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  $Id: main.c 336 2005-10-08 13:49:42Z mderezynski $
 */

#ifndef UTIL_H
#define UTIL_H

namespace Util
{
    char*
    hexify (char *pass, int len);

    std::string
    md5_hex (char* const data, size_t data_size);
}

#endif

/* Tools - Tools for Museek (muscan)
 *
 * Copyright (C) 2003-2004 Hyriand <hyriand@thegraveyard.org>
 * Copyright 2008 little blue poney <lbponey@users.sourceforge.net>
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

#include <stdlib.h>
#include "mp3.h"

#define ENDIAN(head) ((head >> 24) | ((head & 0x00ff0000) >> 8) | ((head & 0x0000ff00) << 8) | (head << 24))
#define AMT 8192

char check_header(uint32 head)
{
	if ((head & 0xffe00000) != 0xffe00000)
		return 0;
	if (! ((head >> 17) & 3))
		return 0;
	if (((head >> 12) & 0xf) == 0xf)
		return 0;
	if (! ((head >> 12) & 0xf))
		return 0;
	if (((head >> 10) & 0x3) == 0x3)
		return 0;
	if ((((head >> 19) & 1) == 1) && (((head >> 17) & 3) == 3) && ((head >> 16) & 1) == 1)
		return 0;
	if ((head & 0xffff0000) == 0xfffe0000)
		return 0;
	return -1;
}

uint32 find_header(FILE *f)
{
	unsigned char *buf = (unsigned char *)malloc(4);
	off_t start = 0, end = 4, i, read = 4;
	uint32 *w, head;
	
	if (fread(buf, 1, 4, f) < 4)
		return 0;
	
	w = (uint32 *)buf;
	head = ENDIAN(*w);
	if (check_header(head))
		return head;
	
	while (read > 0)
	{
		buf = (unsigned char *)realloc(buf, end + AMT);
		read = fread(&buf[end], 1, AMT, f);
		end += read;
		for (i = start; i < end; ++i)
		{
			if (buf[i] != 0xff)
				continue;
			if (i+4 > end)
			{
				start = i;
				break;
			}
			w = (uint32 *)&buf[i];
			head = ENDIAN(*w);
			if (check_header(head))
			{
				free(buf);
				return head;
			}
			i += 1;
			start = i;
		}
	}
	return 0;
}

typedef int bitrate_layer[16];
typedef bitrate_layer bitrate_mpeg[3];
bitrate_mpeg bitrate_table[2] =
{
    { /* MPEG-2 & 2.5 */
        {0,32,48,56, 64, 80, 96,112,128,144,160,176,192,224,256,-1}, /* Layer 1 */
        {0, 8,16,24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,-1}, /* Layer 2 */
        {0, 8,16,24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,-1}  /* Layer 3 */
    },

    { /* MPEG-1 */
        {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,-1}, /* Layer 1 */
        {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,-1}, /* Layer 2 */
        {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,-1}  /* Layer 3 */
    }
};

typedef int samplerate_mpeg[4];
samplerate_mpeg samplerate_table[4] = {
    { 11025, 12000,  8000, -1}, /* MPEG-2.5 */
    {    -1,    -1,    -1, -1}, /* reserved */
    { 22050, 24000, 16000, -1}, /* MPEG-2 */
    { 44100, 48000, 32000, -1}, /* MPEG-1 */
};

void parse_header(uint32 head, mp3info *info, off_t flen)
{
	info->mpeg_version = (head >> 19) & 3;
	info->layer = 4 - ((head >> 17) & 3);
	info->protection_bit = 1 - ((head >> 16) & 1);
	info->bitrate = (head >> 12) & 15;
	info->samplerate = (head >> 10) & 3;
	info->padding_bit = (head >> 9) & 1;
	info->private_bit = (head >> 8) & 1;
	info->mode = (head >> 6) & 3;
	info->mode_extension = (head >> 4) & 3;
	info->copyright = (head >> 3) & 1;
	info->original = (head >> 2) & 1;
	info->emphasis = head & 3;
	
	if (info->mpeg_version == 1)
		return;
	if (info->layer == 0)
		return;
	
	info->bitrate = bitrate_table[info->mpeg_version & 1][info->layer - 1][info->bitrate];
	if (info->bitrate == -1)
		return;
	
	info->samplerate = samplerate_table[info->mpeg_version][info->samplerate];
	if (info->samplerate == -1)
		return;
	
	if (info->layer == 1)
	{
		info->framelength = ((12.0 * (info->bitrate * 1000.0) / info->samplerate) + info->padding_bit) * 4.0;
		info->samplesperframe = 384.0;
	} else {
		info->framelength = (144.0 * (info->bitrate * 1000.0) / info->samplerate) + info->padding_bit;
		info->samplesperframe = 1152.0;
	}
	info->length = (flen / info->framelength) * (info->samplesperframe / info->samplerate);
	
	info->valid = 1;
}

void find_parse_Xing(FILE *f, mp3info *info)
{
	char buf[8192];
	off_t end, i;
	uint32 *tmp, bytes, frames;
	
	info->vbr = 0;
	
	fseek(f, 0, SEEK_SET);
	end = fread(buf, 1, 8192, f);
	for(i = 0; i < end; i++)
	{
		if (buf[i] != 'X')
			continue;
		if (strncmp(&buf[i], "Xing", 4) != 0)
			continue;
		tmp = (uint32 *)&buf[i+4];
		if (ENDIAN(*tmp) & 3)
		{
			tmp++;
			frames = ENDIAN(*tmp);
			tmp++;
			bytes = ENDIAN(*tmp);
			info->vbr = 1;
			info->length = frames * info->samplesperframe / info->samplerate;
			info->bitrate = (bytes * 8.0 / info->length) / 1000;
		}
		return;
	}
}

char mp3_scan(const char *filename, mp3info *info)
{
	uint32 head;
	FILE *f;
	off_t flen;
	
	info->valid = 0;
	
	f = fopen(filename, "rb");
	if (!f)
		return 0;
	
	fseek(f, 0, SEEK_END);
	flen = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	do {
		head = find_header(f);
		if (head)
			parse_header(head, info, flen);
	} while (! info->valid && head);
	if (info->valid)
		find_parse_Xing(f, info);
	fclose(f);
	
	return info->valid;
}

/* int main(void)
{
	mp3info info;
	mp3_scan("01.mp3", &info);
} */

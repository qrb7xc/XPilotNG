/* 
 * XPilotNG, an XPilot-like multiplayer space war game.
 *
 * Copyright (C) 2003 Darel Cullen <darelcullen@users.sourceforge.net>
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

#define icon_width 80
#define icon_height 100
static unsigned char icon_bits[] = {
   0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x6d, 0xad, 0x7c, 0x00,
   0x3e, 0x00, 0x00, 0xf8, 0xff, 0xc3, 0xda, 0xd4, 0x38, 0x80, 0xff, 0x00,
   0x00, 0x08, 0x00, 0x82, 0xff, 0xff, 0x38, 0x80, 0xe7, 0x00, 0x00, 0x08,
   0x00, 0x02, 0xb6, 0xf3, 0x38, 0xc0, 0xcf, 0x01, 0x00, 0xc8, 0x7f, 0x02,
   0xfc, 0x6d, 0x10, 0xc0, 0xa3, 0x01, 0x00, 0x48, 0x40, 0x02, 0xe8, 0x3e,
   0x10, 0xc0, 0x99, 0x01, 0x00, 0x48, 0x5f, 0x02, 0xf0, 0xee, 0x00, 0xc0,
   0x87, 0x01, 0x00, 0x48, 0x51, 0x02, 0xc0, 0xf7, 0x00, 0xc0, 0xd3, 0x01,
   0x00, 0x48, 0x51, 0x02, 0x80, 0xf7, 0x00, 0x80, 0xef, 0x00, 0x00, 0x48,
   0x5f, 0x02, 0x00, 0xff, 0x00, 0x80, 0xff, 0x00, 0x00, 0x48, 0x40, 0x02,
   0x00, 0xbc, 0x18, 0x00, 0x3e, 0x00, 0x00, 0xc8, 0x7f, 0x02, 0x00, 0xf8,
   0x18, 0x00, 0x04, 0x00, 0x00, 0x08, 0x00, 0x02, 0x00, 0xf0, 0x00, 0x00,
   0x04, 0x00, 0x00, 0x08, 0x00, 0x02, 0x00, 0xe0, 0x00, 0x00, 0x04, 0x00,
   0x00, 0xf8, 0xff, 0x03, 0x00, 0x80, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
   0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x0c,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x0c, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x0c, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0c, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0c, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc1, 0x01, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x80, 0xf0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x80, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0xbe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x80, 0x7f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0xc0, 0x6f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xc0, 0x00, 0xf0, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xe0, 0x00, 0xf8, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0,
   0x00, 0xe0, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0xca,
   0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xac, 0x40, 0xb5, 0x0f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x14, 0x2a, 0x05, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xde, 0x50, 0x75, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x7f, 0xc0, 0x2a, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x5b,
   0x18, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xdf, 0x80, 0x15,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xfd, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xff, 0xfc, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xc0, 0xff, 0xfe, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0,
   0xf3, 0xff, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0x5d,
   0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0xef, 0xff, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xf0, 0xef, 0x7f, 0xdf, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xfc, 0xd1, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
   0xff, 0xef, 0xfc, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xdf, 0xff,
   0xdd, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xeb, 0xbf, 0xfe,
   0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0xff, 0xff, 0xf7, 0xff, 0x00, 0x00,
   0x00, 0x00, 0xff, 0xff, 0xdf, 0xff, 0xb6, 0xfe, 0x00, 0x00, 0x00, 0xf8,
   0xff, 0xdf, 0xdf, 0xff, 0xdc, 0xef, 0x00, 0x00, 0xfc, 0x7f, 0xe4, 0xff,
   0xde, 0xd7, 0xff, 0xbb, 0x00, 0xf0, 0xe7, 0x3f, 0xff, 0xf7, 0xff, 0xff,
   0xbf, 0xfd, 0xff, 0xff, 0xf3, 0xef, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xf0, 0x01, 0xbe, 0x55, 0x12, 0xbe, 0xe8, 0x1c, 0x13, 0xbd, 0x81,
   0x01, 0x06, 0x00, 0x00, 0x88, 0x2d, 0xa5, 0x14, 0x85, 0x02, 0x03, 0x03,
   0x00, 0x00, 0x88, 0x2a, 0xa5, 0x14, 0x85, 0x04, 0x86, 0x01, 0x00, 0x00,
   0x88, 0xea, 0x9c, 0x14, 0x9d, 0x04, 0xcc, 0x00, 0x00, 0x00, 0x88, 0x2a,
   0x8c, 0x14, 0x85, 0x04, 0x78, 0x00, 0x00, 0x00, 0x88, 0x28, 0x94, 0x14,
   0x85, 0x04, 0x30, 0x00, 0x00, 0x00, 0x88, 0x28, 0xa4, 0xa4, 0x84, 0x02,
   0x78, 0x00, 0x00, 0x00, 0xbe, 0x28, 0x24, 0x43, 0xbc, 0x01, 0xcc, 0xc0,
   0x15, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86, 0x41, 0x15, 0x25,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc3, 0x15, 0x25, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x01, 0x46, 0x14, 0x25, 0x00, 0x01, 0x20, 0x08,
   0x52, 0xed, 0x01, 0x5e, 0x74, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00};

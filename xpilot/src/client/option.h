/* 
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2003 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef OPTION_H
#define OPTION_H

extern void Parse_options(int *argcp, char **argvp, char *realName, int *port,
			  int *my_team, bool *text, bool *list,
			  bool *join, bool *noLocalMotd,
			  char *nickName, char *dispName, char *hostName,
			  char *shut_msg);

extern void Get_xpilotrc_file(char *, unsigned);

/*
 * Macros for initalizing options.
 */


#endif
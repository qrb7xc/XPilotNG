/* 
 * XPilotNG, an XPilot-like multiplayer space war game.
 *
 * Copyright (C) 2000-2004 by
 *
 *      Uoti Urpala          <uau@users.sourceforge.net>
 *      Kristian S�derblom   <kps@users.sourceforge.net>
 *
 * Copyright (C) 1991-2001 by
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef SCORE_H
#define SCORE_H

#define ASTEROID_SCORE		-1436.0
#define CANNON_SCORE	    	-1436.0
#define WALL_SCORE	    	2000.0

#define RATE_SIZE	    	20
#define RATE_RANGE	    	1024

/* score.c */

void Score(player_t *pl, double points, clpos_t pos, const char *msg);
double Rate(double winner, double loser);
void Score_players(player_t *winner_pl, double winner_score, char *winner_msg,
		   player_t *loser_pl, double loser_score, char *loser_msg,
		   bool transfer_tag);

#endif

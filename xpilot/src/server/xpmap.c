/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WINDOWS
# include <sys/file.h>
#endif

#ifdef _WINDOWS
# include "NT/winServer.h"
#endif

#define SERVER
#include "version.h"
#include "config.h"
#include "serverconst.h"
#include "global.h"
#include "proto.h"
#include "map.h"
#include "bit.h"
#include "error.h"
#include "commonproto.h"

char xpmap_version[] = VERSION;


static void Xpmap_treasure_to_polygon(int);
static void Xpmap_target_to_polygon(int);
static void Xpmap_cannon_to_polygon(int);
static void Xpmap_wormhole_to_polygon(int);

#ifdef DEBUG
void Xpmap_print(void)			/* Debugging only. */
{
    int x, y;

    for (y = World.y - 1; y >= 0; y--) {
	for (x = 0; x < World.x; x++)
	    switch (World.block[x][y]) {
	    case SPACE:
		putchar(' ');
		break;
	    case BASE:
		putchar('_');
		break;
	    default:
		putchar('X');
		break;
	    }
	putchar('\n');
    }
}
#endif

static void Xpmap_extra_error(int line_num)
{
#ifndef SILENT
    static int prev_line_num, error_count;
    const int max_error = 5;

    if (line_num > prev_line_num) {
	prev_line_num = line_num;
	if (++error_count <= max_error) {
	    xpprintf("Map file contains extraneous characters on line %d\n",
		     line_num);
	}
	else if (error_count - max_error == 1) {
	    xpprintf("And so on...\n");
	}
    }
#endif
}


static void Xpmap_missing_error(int line_num)
{
#ifndef SILENT
    static int prev_line_num, error_count;
    const int max_error = 5;

    if (line_num > prev_line_num) {
	prev_line_num = line_num;
	if (++error_count <= max_error) {
	    xpprintf("Not enough map data on map data line %d\n", line_num);
	}
	else if (error_count - max_error == 1) {
	    xpprintf("And so on...\n");
	}
    }
#endif
}

/*
 * Grok block based map data.
 *
 * Create World.block using mapData.
 * Free mapData.
 */
void Xpmap_grok_map_data(void)
{
    int x, y, c;
    char *s;

    x = -1;
    y = World.y - 1;

    s = mapData;
    while (y >= 0) {

	x++;

	if (extraBorder && (x == 0 || x == World.x - 1
	    || y == 0 || y == World.y - 1)) {
	    if (x >= World.x) {
		x = -1;
		y--;
		continue;
	    } else {
		/* make extra border of solid rock */
		c = 'x';
	    }
	}
	else {
	    c = *s;
	    if (c == '\0' || c == EOF) {
		if (x < World.x) {
		    /* not enough map data on this line */
		    Xpmap_missing_error(World.y - y);
		    c = ' ';
		} else {
		    c = '\n';
		}
	    } else {
		if (c == '\n' && x < World.x) {
		    /* not enough map data on this line */
		    Xpmap_missing_error(World.y - y);
		    c = ' ';
		} else {
		    s++;
		}
	    }
	}
	if (x >= World.x || c == '\n') {
	    y--; x = -1;
	    if (c != '\n') {			/* Get rest of line */
		Xpmap_extra_error(World.y - y);
		while (c != '\n' && c != EOF) {
		    c = *s++;
		}
	    }
	    continue;
	}

	World.block[x][y] = c;
    }

    free(mapData);
    mapData = NULL;
}

void Xpmap_allocate_checks(void)
{
    int x, y;

    if (!BIT(World.rules->mode, TIMING))
	return;

    /* count the checks */
    for (y = 0; y < World.y; y++) {
	for (x = 0; x < World.x; x++) {
	    switch (World.block[x][y]) {
	    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	    case 'Y': case 'Z':
		World.NumChecks++;
		break;
	    default:
		break;
	    }
	}
    }

    if ((World.check = (clpos *)
	 malloc(OLD_MAX_CHECKS * sizeof(clpos))) == NULL) {
	error("Out of memory - checks");
	exit(-1);
    }
}


/*
 * Determining which team these belong to is done later,
 * in Find_closest_team().
 */
static void Xpmap_place_cannon(int x, int y, int dir)
{
    int cx = -1, cy = -1;

    switch (dir) {
    case DIR_UP:
	cx = (x + 0.5) * BLOCK_CLICKS;
	cy = (y + 0.333) * BLOCK_CLICKS;
	break;
    case DIR_LEFT:
	cx = (x + 0.667) * BLOCK_CLICKS;
	cy = (y + 0.5) * BLOCK_CLICKS;
	break;
    case DIR_RIGHT:
	cx = (x + 0.333) * BLOCK_CLICKS;
	cy = (y + 0.5) * BLOCK_CLICKS;
	break;
    case DIR_DOWN:
	cx = (x + 0.5) * BLOCK_CLICKS;
	cy = (y + 0.667) * BLOCK_CLICKS;
	break;
    }

    World.block[x][y] = CANNON;
    World.itemID[x][y] = Map_place_cannon(cx, cy, dir, TEAM_NOT_SET);
}

/*
 * The direction of the base should be so that it points
 * up with respect to the gravity in the region.  This
 * is fixed in Find_base_dir() when the gravity has
 * been computed.
 */
static void Xpmap_place_base(int x, int y, int team)
{
    int cx = BLOCK_CENTER(x), cy = BLOCK_CENTER(y);

    World.block[x][y] = BASE;
    World.itemID[x][y] = Map_place_base(cx, cy, DIR_UP, team);
}

static void Xpmap_place_fuel(int x, int y)
{
    int cx = BLOCK_CENTER(x), cy = BLOCK_CENTER(y);

    World.block[x][y] = FUEL;
    World.itemID[x][y] = Map_place_fuel(cx, cy, TEAM_NOT_SET);
}

static void Xpmap_place_treasure(int x, int y, bool empty)
{
    int cx = BLOCK_CENTER(x), cy = BLOCK_CENTER(y);

    World.block[x][y] = TREASURE;
    World.itemID[x][y] = Map_place_treasure(cx, cy, TEAM_NOT_SET, empty);
}

static void Xpmap_place_wormhole(int x, int y, wormType type)
{
    int cx = BLOCK_CENTER(x), cy = BLOCK_CENTER(y);

    World.block[x][y] = WORMHOLE;
    World.itemID[x][y] = Map_place_wormhole(cx, cy, type);
}

static void Xpmap_place_target(int x, int y)
{
    int cx = BLOCK_CENTER(x), cy = BLOCK_CENTER(y);

    World.block[x][y] = TARGET;
    World.itemID[x][y] = Map_place_target(cx, cy, TEAM_NOT_SET);
}

static void Xpmap_place_check(int x, int y, int index)
{
    int cx = BLOCK_CENTER(x), cy = BLOCK_CENTER(y);

    if (!BIT(World.rules->mode, TIMING)) {
	World.block[x][y] = SPACE;
    }

    World.block[x][y] = CHECK;
    World.itemID[x][y] = Map_place_check(cx, cy, index);
}

static void Xpmap_place_item_concentrator(int x, int y)
{
    int cx = BLOCK_CENTER(x), cy = BLOCK_CENTER(y);

    World.block[x][y] = ITEM_CONCENTRATOR;
    World.itemID[x][y] = Map_place_item_concentrator(cx, cy);
}

static void Xpmap_place_asteroid_concentrator(int x, int y)
{
    int cx = BLOCK_CENTER(x), cy = BLOCK_CENTER(y);

    World.block[x][y] = ASTEROID_CONCENTRATOR;
    World.itemID[x][y] = Map_place_asteroid_concentrator(cx, cy);
}

static void Xpmap_place_grav(int x, int y, DFLOAT force, int type)
{
    int cx = BLOCK_CENTER(x), cy = BLOCK_CENTER(y);

    World.block[x][y] = type;
    World.itemID[x][y] = Map_place_grav(cx, cy, force, type);
}

static void Xpmap_place_block(int x, int y, int type)
{
    World.block[x][y] = type;
}



/*
 * Change read tags to internal data, create objects
 */
void Xpmap_create_map_objects(void)
{
    int x, y;
    char c;

    for (x = 0; x < World.x; x++) {

	for (y = 0; y < World.y; y++) {

	    c = World.block[x][y];
	    World.itemID[x][y] = (unsigned short) -1;
	    
	    switch (c) {
	    case ' ':
	    case '.':
	    default:
		Xpmap_place_block(x, y, SPACE);
		break;
		
	    case 'x':
		Xpmap_place_block(x, y, FILLED);
		break;
	    case 's':
		Xpmap_place_block(x, y, REC_LU);
		break;
	    case 'a':
		Xpmap_place_block(x, y, REC_RU);
		break;
	    case 'w':
		Xpmap_place_block(x, y, REC_LD);
		break;
	    case 'q':
		Xpmap_place_block(x, y, REC_RD);
		break;
		    
	    case 'r':
		Xpmap_place_cannon(x, y, DIR_UP);
		break;
	    case 'd':
		Xpmap_place_cannon(x, y, DIR_LEFT);
		break;
	    case 'f':
		Xpmap_place_cannon(x, y, DIR_RIGHT);
		break;
	    case 'c':
		Xpmap_place_cannon(x, y, DIR_DOWN);
		break;
		
	    case '#':
		Xpmap_place_fuel(x, y);
		break;
		
	    case '*':
	    case '^':
		Xpmap_place_treasure(x, y, (c == '^'));
		break;
	    case '!':
		Xpmap_place_target(x, y);
		break;
	    case '%':
		Xpmap_place_item_concentrator(x, y);
		break;
	    case '&':
		Xpmap_place_asteroid_concentrator(x, y);
		break;
	    case '$':
		Xpmap_place_block(x, y, BASE_ATTRACTOR);
		break;
	    case '_':
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
		Xpmap_place_base(x, y, (int) (c - '0'));
		break;
		
	    case '+':
		Xpmap_place_grav(x, y, -GRAVS_POWER, POS_GRAV);
		break;
	    case '-':
		Xpmap_place_grav(x, y, GRAVS_POWER, NEG_GRAV);
		break;
	    case '>':
		Xpmap_place_grav(x, y, GRAVS_POWER, CWISE_GRAV);
		break;
	    case '<':
		Xpmap_place_grav(x, y, -GRAVS_POWER, ACWISE_GRAV);
		break;
	    case 'i':
		Xpmap_place_grav(x, y, GRAVS_POWER, UP_GRAV);
		break;
	    case 'm':
		Xpmap_place_grav(x, y, -GRAVS_POWER, DOWN_GRAV);
		break;
	    case 'k':
		Xpmap_place_grav(x, y, GRAVS_POWER, RIGHT_GRAV);
		break;
	    case 'j':
		Xpmap_place_grav(x, y, -GRAVS_POWER, LEFT_GRAV);
		break;
		
	    case '@':
		Xpmap_place_wormhole(x, y, WORM_NORMAL);
		break;
	    case '(':
		Xpmap_place_wormhole(x, y, WORM_IN);
		break;
	    case ')':
		Xpmap_place_wormhole(x, y, WORM_OUT);
		break;
		
	    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	    case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
	    case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
	    case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
	    case 'Y': case 'Z':
		Xpmap_place_check(x, y, (int) (c - 'A'));
		break;
		
	    case 'z':
		Xpmap_place_block(x, y, FRICTION);
		break;
		
	    case 'b':
		Xpmap_place_block(x, y, DECOR_FILLED);
		break;
	    case 'h':
		Xpmap_place_block(x, y, DECOR_LU);
		break;
	    case 'g':
		Xpmap_place_block(x, y, DECOR_RU);
		break;
	    case 'y':
		Xpmap_place_block(x, y, DECOR_LD);
		break;
	    case 't':
		Xpmap_place_block(x, y, DECOR_RD);
		break;
	    }
	}
    }
}



void Xpmap_find_map_object_teams(void)
{
    int i;

    /*
     * Determine which team a treasure belongs to.
     */
    if (BIT(World.rules->mode, TEAM_PLAY)) {
	unsigned short team = TEAM_NOT_SET;
	for (i = 0; i < World.NumTreasures; i++) {
	    team = Find_closest_team(World.treasures[i].pos.cx,
				     World.treasures[i].pos.cy);
	    World.treasures[i].team = team;
	    if (team == TEAM_NOT_SET) {
		error("Couldn't find a matching team for the treasure.");
	    } else {
		World.teams[team].NumTreasures++;
		if (!World.treasures[i].empty) {
		    World.teams[team].TreasuresLeft++;
		} else {
		    World.teams[team].NumEmptyTreasures++;
		}
	    }
	}
	for (i = 0; i < World.NumTargets; i++) {
	    team = Find_closest_team(World.targets[i].pos.cx,
				     World.targets[i].pos.cy);
	    if (team == TEAM_NOT_SET) {
		error("Couldn't find a matching team for the target.");
	    }
	    World.targets[i].team = team;
	}
	if (teamCannons) {
	    for (i = 0; i < World.NumCannons; i++) {
		team = Find_closest_team(World.cannon[i].pos.cx,
					 World.cannon[i].pos.cy);
		if (team == TEAM_NOT_SET) {
		    error("Couldn't find a matching team for the cannon.");
		}
		World.cannon[i].team = team;
	    }
	}
	
	for (i = 0; i < World.NumFuels; i++) {
	    team = Find_closest_team(World.fuel[i].pos.cx,
				     World.fuel[i].pos.cy);
	    if (team == TEAM_NOT_SET) {
		error("Couldn't find a matching team for fuelstation.");
	    }
	    World.fuel[i].team = team;
	}
    }

}



/*
 * Find the correct direction of the base, according to the gravity in
 * the base region.
 *
 * If a base attractor is adjacent to a base then the base will point
 * to the attractor.
 */
void Xpmap_find_base_direction(void)
{
    int	i;

    for (i = 0; i < World.NumBases; i++) {
	int	x = World.base[i].pos.cx / BLOCK_CLICKS,
		y = World.base[i].pos.cy / BLOCK_CLICKS,
		dir,
		att;
	double	dx = World.gravity[x][y].x,
		dy = World.gravity[x][y].y;

	if (dx == 0.0 && dy == 0.0) {	/* Undefined direction? */
	    dir = DIR_UP;	/* Should be set to direction of gravity! */
	} else {
	    dir = (int)findDir(-dx, -dy);
	    dir = ((dir + RES/8) / (RES/4)) * (RES/4);	/* round it */
	    dir = MOD2(dir, RES);
	}
	att = -1;
	/*BASES SNAP TO UPWARDS ATTRACTOR FIRST*/
        if (y == World.y - 1 && World.block[x][0] == BASE_ATTRACTOR
	    && BIT(World.rules->mode, WRAP_PLAY)) {  /*check wrapped*/
	    if (att == -1 || dir == DIR_UP) {
		att = DIR_UP;
	    }
	}
	if (y < World.y - 1 && World.block[x][y + 1] == BASE_ATTRACTOR) {
	    if (att == -1 || dir == DIR_UP) {
		att = DIR_UP;
	    }
	}
	/*THEN DOWNWARDS ATTRACTORS*/
        if (y == 0 && World.block[x][World.y-1] == BASE_ATTRACTOR
	    && BIT(World.rules->mode, WRAP_PLAY)) { /*check wrapped*/
	    if (att == -1 || dir == DIR_DOWN) {
		att = DIR_DOWN;
	    }
	}
	if (y > 0 && World.block[x][y - 1] == BASE_ATTRACTOR) {
	    if (att == -1 || dir == DIR_DOWN) {
		att = DIR_DOWN;
	    }
	}
	/*THEN RIGHTWARDS ATTRACTORS*/
	if (x == World.x - 1 && World.block[0][y] == BASE_ATTRACTOR
	    && BIT(World.rules->mode, WRAP_PLAY)) { /*check wrapped*/
	    if (att == -1 || dir == DIR_RIGHT) {
		att = DIR_RIGHT;
	    }
	}
	if (x < World.x - 1 && World.block[x + 1][y] == BASE_ATTRACTOR) {
	    if (att == -1 || dir == DIR_RIGHT) {
		att = DIR_RIGHT;
	    }
	}
	/*THEN LEFTWARDS ATTRACTORS*/
	if (x == 0 && World.block[World.x-1][y] == BASE_ATTRACTOR
	    && BIT(World.rules->mode, WRAP_PLAY)) { /*check wrapped*/
	    if (att == -1 || dir == DIR_LEFT) {
		att = DIR_LEFT;
	    }
	}
	if (x > 0 && World.block[x - 1][y] == BASE_ATTRACTOR) {
	    if (att == -1 || dir == DIR_LEFT) {
		att = DIR_LEFT;
	    }
	}
	if (att != -1) {
	    dir = att;
	}
	World.base[i].dir = dir;
    }
    for (i = 0; i < World.x; i++) {
	int j;
	for (j = 0; j < World.y; j++) {
	    if (World.block[i][j] == BASE_ATTRACTOR) {
		World.block[i][j] = SPACE;
	    }
	}
    }
}





/* number of vertices in polygon */
#define N (2 + 12)
static void Xpmap_treasure_to_polygon(int ind)
{
    int cx, cy, i, r, n;
    double angle;
    int polystyle, edgestyle;
    clpos pos[N + 1];
    treasure_t *treasure = &World.treasures[ind];

    polystyle = P_get_poly_id("treasure_ps");
    edgestyle = P_get_edge_id("treasure_es");

    /*printf(__FUNCTION__ ": team = %d\n", treasure->team);*/
    cx = treasure->pos.cx - BLOCK_CLICKS / 2;
    cy = treasure->pos.cy - BLOCK_CLICKS / 2;

    pos[0].cx = cx;
    pos[0].cy = cy;
    pos[1].cx = cx + (BLOCK_CLICKS - 1);
    pos[1].cy = cy;

    cx = treasure->pos.cx;
    cy = treasure->pos.cy;
    /* -1 is to ensure the vertices are inside the block */
    r = (BLOCK_CLICKS / 2) - 1;
    /* number of points in half circle */
    n = N - 2;

    for (i = 0; i < n; i++) {
	angle = (((double)i)/(n - 1)) * PI;
	pos[i + 2].cx = cx + r * cos(angle);
	pos[i + 2].cy = cy + r * sin(angle);
    }

    pos[N] = pos[0];

    /* create balltarget */
    P_start_balltarget(treasure->team);
    P_start_polygon(pos[0].cx, pos[0].cy, polystyle);
    for (i = 1; i <= N; i++)
	P_vertex(pos[i].cx, pos[i].cy, edgestyle); 
    P_end_polygon();
    P_end_balltarget();

    /* create ballarea */
    P_start_ballarea();
    P_start_polygon(pos[0].cx, pos[0].cy, polystyle);
    for (i = 1; i <= N; i++)
	P_vertex(pos[i].cx, pos[i].cy, edgestyle); 
    P_end_polygon();
    P_end_ballarea();
}
#undef N


static void Xpmap_block_polygon(int cx, int cy, int polystyle, int edgestyle)
{
    clpos pos[5];
    int i;

    cx = CLICK_TO_BLOCK(cx) * BLOCK_CLICKS;
    cy = CLICK_TO_BLOCK(cy) * BLOCK_CLICKS;

    pos[0].cx = cx;
    pos[0].cy = cy;
    pos[1].cx = cx + (BLOCK_CLICKS - 1);
    pos[1].cy = cy;
    pos[2].cx = cx + (BLOCK_CLICKS - 1);
    pos[2].cy = cy + (BLOCK_CLICKS - 1);
    pos[3].cx = cx;
    pos[3].cy = cy + (BLOCK_CLICKS - 1);
    pos[4] = pos[0];

    P_start_polygon(pos[0].cx, pos[0].cy, polystyle);
    for (i = 1; i <= 4; i++)
	P_vertex(pos[i].cx, pos[i].cy, edgestyle); 
    P_end_polygon();
}


static void Xpmap_target_to_polygon(int ind)
{
    int cx, cy, ps, es;
    target_t *targ = &World.targets[ind];

    /* too buggy currently */
    return ;

    ps = P_get_poly_id("target_ps");
    es = P_get_edge_id("target_es");

    cx = targ->pos.cx;
    cy = targ->pos.cy;

    /* create target polygon */
    P_start_target(targ->team, ind);
    Xpmap_block_polygon(cx, cy, ps, es);
    P_end_target();
}


static void Xpmap_cannon_polygon(cannon_t *cannon,
				 int polystyle, int edgestyle)
{
    clpos pos[4];
    int cx = cannon->pos.cx;
    int cy = cannon->pos.cy;
    int i;

    pos[0].cx = cx;
    pos[0].cy = cy;

    cx = CLICK_TO_BLOCK(cx) * BLOCK_CLICKS;
    cy = CLICK_TO_BLOCK(cy) * BLOCK_CLICKS;

    switch (cannon->dir) {
    case DIR_RIGHT:
	pos[1].cx = cx;
	pos[1].cy = cy + (BLOCK_CLICKS - 1);
	pos[2].cx = cx;
	pos[2].cy = cy;
	break;
    case DIR_UP:
	pos[1].cx = cx;
	pos[1].cy = cy;
	pos[2].cx = cx + (BLOCK_CLICKS - 1);
	pos[2].cy = cy;
	break;
    case DIR_LEFT:
	pos[1].cx = cx + (BLOCK_CLICKS - 1);
	pos[1].cy = cy;
	pos[2].cx = cx + (BLOCK_CLICKS - 1);
	pos[2].cy = cy + (BLOCK_CLICKS - 1);
	break;
    case DIR_DOWN:
	pos[1].cx = cx + (BLOCK_CLICKS - 1);
	pos[1].cy = cy + (BLOCK_CLICKS - 1);
	pos[2].cx = cx;
	pos[2].cy = cy + (BLOCK_CLICKS - 1);
	break;
    }
    pos[3] = pos[0];

    P_start_polygon(pos[0].cx, pos[0].cy, polystyle);
    for (i = 1; i <= 3; i++)
	P_vertex(pos[i].cx, pos[i].cy, edgestyle); 
    P_end_polygon();
}

/* someone write some code to rotate a cannon in any angle */
static void Xpmap_cannon_to_polygon(int ind)
{
    int ps, es;
    cannon_t *c = &World.cannon[ind];

    ps = P_get_poly_id("cannon_ps");
    es = P_get_edge_id("cannon_es");

    P_start_cannon(c->pos.cx, c->pos.cy, c->dir, c->team, ind);
    /* we need a more general function for this purpose */
    Xpmap_cannon_polygon(c, ps, es);
    P_end_cannon();
}

#define N 12
/* kps - do we want a polygon for out wormholes ??? */
static void Xpmap_wormhole_to_polygon(int ind)
{
    int cx, cy, ps, es, i, r;
    double angle;
    clpos pos[N + 1];
    wormhole_t *wormhole = &World.wormHoles[ind];

    /* too buggy currently */
    return ;

    if (wormhole->type == WORM_OUT)
	return;

    ps = P_get_poly_id("wormhole_ps");
    es = P_get_edge_id("wormhole_es");

    cx = wormhole->pos.cx;
    cy = wormhole->pos.cy;
    r = (BLOCK_CLICKS / 2) - 1;

    for (i = 0; i < N; i++) {
	angle = (((double)i)/ N) * 2 * PI;
	pos[i].cx = cx + r * cos(angle);
	pos[i].cy = cy + r * sin(angle);
    }
    pos[N] = pos[0];

    P_start_wormhole(ind);
    P_start_polygon(pos[0].cx, pos[0].cy, ps);
    for (i = 1; i <= N; i++)
	P_vertex(pos[i].cx, pos[i].cy, es); 
    P_end_polygon();
    P_end_wormhole();
}




/*
 * Add a wall polygon
 *
 * The polygon consists of a start block and and endblock and possibly
 * some full wall/fuel blocks in between. A total number of numblocks
 * blocks are part of the polygon and must be 1 or more. If numblocks
 * is one, the startblock and endblock are the same block.
 *
 * The block coordinates of the first block is (bx, by)
 *
 * The polygon will have 3 or 4 vertices.
 *
 * Idea: first assume the polygon is a rectangle, then move
 * the vertices depending on the start and end blocks.
 *
 * The vertex index:
 * 0: upper left vertex
 * 1: lower left vertex
 * 2: lower right vertex
 * 3: upper right vertex
 * 4: upper left vertex, second time
 */

static void Xpmap_wall_poly(int bx, int by, char startblock,
			    char endblock, int numblocks,
			    int polystyle, int edgestyle)
{
    int i;
    clpos pos[5]; /* positions of vertices */

    if (numblocks < 1)
	return;

    /* first assume we have a rectangle */
    /* kps - use -1 if you don't wan't the polygon corners to
     * overlap other polygons
     */
    pos[0].cx = bx * BLOCK_CLICKS;
    pos[0].cy = (by + 1) * BLOCK_CLICKS - 1;
    pos[1].cx = bx * BLOCK_CLICKS;
    pos[1].cy = by * BLOCK_CLICKS;
    pos[2].cx = (bx + numblocks) * BLOCK_CLICKS - 1;
    pos[2].cy = by * BLOCK_CLICKS;
    pos[3].cx = (bx + numblocks) * BLOCK_CLICKS - 1;
    pos[3].cy = (by + 1) * BLOCK_CLICKS - 1;
    
    /* move the vertices depending on the startblock and endblock */
    switch (startblock) {
    case FILLED:
    case REC_LU:
    case REC_LD:
    case FUEL:
	/* no need to move the leftmost 2 vertices */
	break;
    case REC_RU:
	/* move lower left vertex to the right */
	pos[1].cx += (BLOCK_CLICKS - 1);
	break;
    case REC_RD:
	/* move upper left vertex to the right */
	pos[0].cx += (BLOCK_CLICKS - 1);
	break;
    default:
	return;
    }
    
    switch (endblock) {
    case FILLED:
    case FUEL:
    case REC_RU:
    case REC_RD:
	/* no need to move the rightmost 2 vertices */
	break;
    case REC_LU:
	pos[2].cx -= (BLOCK_CLICKS - 1);
	break;
    case REC_LD:
	pos[3].cx -= (BLOCK_CLICKS - 1);
	break;
    default:
	return;
    }

    /*
     * Since we want to form a closed loop of line segments, the
     * last vertex must equal the first.
     */
    pos[4] = pos[0];

    P_start_polygon(pos[0].cx, pos[0].cy, polystyle);
    for (i = 1; i <= 4; i++)
	P_vertex(pos[i].cx, pos[i].cy, edgestyle); 
    P_end_polygon();
}

void Xpmap_walls_to_polygons(void)
{
    int x, y, x0 = 0;
    int numblocks = 0;
    int inside = false;
    char startblock = 0, endblock = 0, block;
    int maxblocks = POLYGON_MAX_OFFSET / BLOCK_CLICKS;
    int ps, es;

    ps = P_get_poly_id("wall_ps");
    es = P_get_edge_id("wall_es");

    /*
     * x, FILLED = solid wall
     * s, REC_LU = wall triangle pointing left and up 
     * a, REC_RU = wall triangle pointing right and up 
     * w, REC_LD = wall triangle pointing left and down
     * q, REC_RD = wall triangle pointing right and down
     * #, FUEL   = fuel block
     */

    for (y = World.y - 1; y >= 0; y--) {
	for (x = 0; x < World.x; x++) {
	    block = World.block[x][y];

	    if (!inside) {
		switch (block) {
		case FILLED:
		case REC_RU:
		case REC_RD:
		case FUEL:
		    x0 = x;
		    startblock = endblock = block;
		    inside = true;
		    numblocks = 1;
		    break;

		case REC_LU:
		case REC_LD:
		    Xpmap_wall_poly(x, y, block, block, 1, ps, es);
		    break;
		default:
		    break;
		}
	    } else {

		switch (block) {
		case FILLED:
		case FUEL:
		    numblocks++;
		    endblock = block;
		    break;

		case REC_RU:
		case REC_RD:
		    /* old polygon ends */
		    Xpmap_wall_poly(x0, y, startblock, endblock,
				    numblocks, ps, es);
		    /* and a new one starts */
		    x0 = x;
		    startblock = endblock = block;
		    numblocks = 1;
		    break;

		case REC_LU:
		case REC_LD:
		    numblocks++;
		    endblock = block;
		    Xpmap_wall_poly(x0, y, startblock, endblock,
				    numblocks, ps, es);
		    inside = false;
		    break;

		default:
		    /* none of the above, polygon ends */
		    Xpmap_wall_poly(x0, y, startblock, endblock,
				    numblocks, ps, es);
		    inside = false;
		    break;
		}
	    }

	    /*
	     * We don't want the polygon to have offsets
	     * that is too big
	     */
	    if (inside && numblocks == maxblocks) {
		Xpmap_wall_poly(x0, y, startblock, endblock,
				numblocks, ps, es);
		inside = false;
	    }

	}

	/* end of row */
	if (inside) {
	    Xpmap_wall_poly(x0, y, startblock, endblock,
			    numblocks, ps, es);
	    inside = false;
	}
    }
}


void Xpmap_blocks_to_polygons(void)
{
    extern int num_polys;
    int i;

    /* create edgestyles and polystyles */
    P_edgestyle("wall_es", -1, 0x2244EE, 0);
    P_polystyle("wall_ps", 0x0033AA, 0, P_get_edge_id("wall_es"), 0);

    P_edgestyle("treasure_es", -1, 0xFF0000, 0);
    P_polystyle("treasure_ps", 0xFF0000, 0, P_get_edge_id("treasure_es"), 0);

    P_edgestyle("target_es", 3, 0xFF7700, 0);
    P_polystyle("target_ps", 0xFF7700, 3, P_get_edge_id("target_es"), 0);

    P_edgestyle("cannon_es", 3, 0xFFFFFF, 0);
    P_polystyle("cannon_ps", 0xFFFFFF, 2, P_get_edge_id("cannon_es"), 0);

    P_edgestyle("wormhole_es", 2, 0x00FFFF, 0);
    P_polystyle("wormhole_ps", 0x00FFFF, 2, P_get_edge_id("wormhole_es"), 0);

    Xpmap_walls_to_polygons();

    /* kps - if you want to see the polygons in the client (use the polygon
     * protocol , do this */
    /*is_polygon_map = 1;*/
    if (polygonMode)
	is_polygon_map = TRUE;

    for (i = 0; i < World.NumTreasures; i++)
	Xpmap_treasure_to_polygon(i);

    for (i = 0; i < World.NumTargets; i++)
	Xpmap_target_to_polygon(i);

    for (i = 0; i < World.NumCannons; i++)
	Xpmap_cannon_to_polygon(i);

    for (i = 0; i < World.NumWormholes; i++)
	Xpmap_wormhole_to_polygon(i);

    xpprintf("Created %d polygons.\n", num_polys);
}

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

#ifndef	MAP_H
#define	MAP_H

#ifndef TYPES_H
/* need position */
#include "types.h"
#endif
#ifndef RULES_H
/* need rules_t */
#include "rules.h"
#endif
#ifndef ITEM_H
/* need NUM_ITEMS */
#include "item.h"
#endif

#define SPACE			0
#define BASE			1
#define FILLED			2
#define REC_LU			3
#define REC_LD			4
#define REC_RU			5
#define REC_RD			6
#define FUEL			7
#define CANNON			8
#define CHECK			9
#define POS_GRAV		10
#define NEG_GRAV		11
#define CWISE_GRAV		12
#define ACWISE_GRAV		13
#define WORMHOLE		14
#define TREASURE		15
#define TARGET			16
#define ITEM_CONCENTRATOR	17
#define DECOR_FILLED		18
#define DECOR_LU		19
#define DECOR_LD		20
#define DECOR_RU		21
#define DECOR_RD		22
#define UP_GRAV			23
#define DOWN_GRAV		24
#define RIGHT_GRAV		25
#define LEFT_GRAV		26
#define FRICTION		27
#define ASTEROID_CONCENTRATOR	28
#define BASE_ATTRACTOR		127

#define SPACE_BIT		(1 << SPACE)
#define BASE_BIT		(1 << BASE)
#define FILLED_BIT		(1 << FILLED)
#define REC_LU_BIT		(1 << REC_LU)
#define REC_LD_BIT		(1 << REC_LD)
#define REC_RU_BIT		(1 << REC_RU)
#define REC_RD_BIT		(1 << REC_RD)
#define FUEL_BIT		(1 << FUEL)
#define CANNON_BIT		(1 << CANNON)
#define CHECK_BIT		(1 << CHECK)
#define POS_GRAV_BIT		(1 << POS_GRAV)
#define NEG_GRAV_BIT		(1 << NEG_GRAV)
#define CWISE_GRAV_BIT		(1 << CWISE_GRAV)
#define ACWISE_GRAV_BIT		(1 << ACWISE_GRAV)
#define WORMHOLE_BIT		(1 << WORMHOLE)
#define TREASURE_BIT		(1 << TREASURE)
#define TARGET_BIT		(1 << TARGET)
#define ITEM_CONCENTRATOR_BIT	(1 << ITEM_CONCENTRATOR)
#define DECOR_FILLED_BIT	(1 << DECOR_FILLED)
#define DECOR_LU_BIT		(1 << DECOR_LU)
#define DECOR_LD_BIT		(1 << DECOR_LD)
#define DECOR_RU_BIT		(1 << DECOR_RU)
#define DECOR_RD_BIT		(1 << DECOR_RD)
#define UP_GRAV_BIT             (1 << UP_GRAV)
#define DOWN_GRAV_BIT           (1 << DOWN_GRAV)
#define RIGHT_GRAV_BIT          (1 << RIGHT_GRAV)
#define LEFT_GRAV_BIT           (1 << LEFT_GRAV)
#define FRICTION_BIT		(1 << FRICTION)
#define ASTEROID_CONCENTRATOR_BIT	(1 << ASTEROID_CONCENTRATOR)

#define DIR_RIGHT		0
#define DIR_UP			(RES/4)
#define DIR_LEFT		(RES/2)
#define DIR_DOWN		(3*RES/4)


typedef struct fuel {
    clpos	pos;
    double	fuel;
    unsigned	conn_mask;
    long	last_change;
    int		team;
} fuel_t;

typedef struct grav {
    clpos	pos;
    double	force;
    int		type;
} grav_t;

typedef struct base {
    clpos	pos;
    int		dir;
    int		ind;
    int		team;
} base_t;

typedef struct baseorder {
    int		base_idx;	/* Index in World.base[] */
    double	dist;		/* Distance to first checkpoint */
} baseorder_t;

typedef struct cannon {
    clpos	pos;
    int		dir;
    unsigned	conn_mask;
    long	last_change;
    int		item[NUM_ITEMS];
    struct player	*tractor_target_pl;
    bool	tractor_is_pressor;
    int		team;
    long	used;
    double	dead_time;
    double	damaged;
    double	tractor_count;
    double	emergency_shield_left;
    double	phasing_left;
    int		group;
} cannon_t;

typedef struct check {
    clpos	pos;
} check_t;

typedef struct item {
    double	prob;		/* Probability [0..1] for item to appear */
    int		max;		/* Max on world at a given time */
    int		num;		/* Number active right now */
    int		chance;		/* Chance [0..127] for this item to appear */
    double	cannonprob;	/* Relative probability for item to appear */
    int		min_per_pack;	/* minimum number of elements per item. */
    int		max_per_pack;	/* maximum number of elements per item. */
    int		initial;	/* initial number of elements per player. */
    int		limit;		/* max number of elements per player/cannon. */
} item_t;

typedef struct asteroid {
    double	prob;		/* Probability [0..1] for asteroid to appear */
    int		max;		/* Max on world at a given time */
    int		num;		/* Number active right now */
    int		chance;		/* Chance [0..127] for asteroid to appear */
} asteroid_t;

typedef enum { WORM_NORMAL, WORM_IN, WORM_OUT } wormType;

typedef struct wormhole {
    clpos	pos;
    int		lastdest;	/* last destination wormhole */
    double	countdown;	/* >0 warp to lastdest else random */
    bool	temporary;	/* wormhole was left by hyperjump */
    wormType	type;
    int		lastID;
    u_byte	lastblock;	/* block it occluded */
    u_byte	pad[3];
} wormhole_t;

typedef struct treasure {
    clpos	pos;
    bool	have;		/* true if this treasure has ball in it */
    int		team;		/* team of this treasure */
    int 	destroyed;	/* how often this treasure destroyed */
    bool	empty;		/* true if this treasure never had a ball */
} treasure_t;

typedef struct target {
    clpos	pos;
    int		team;
    double	dead_time;
    double	damage;
    unsigned	conn_mask;
    unsigned 	update_mask;
    long	last_change;
    int		group;
} target_t;

typedef struct team {
    int		NumMembers;		/* Number of current members */
    int		NumRobots;		/* Number of robot players */
    int		NumBases;		/* Number of bases owned */
    int		NumTreasures;		/* Number of treasures owned */
    int		NumEmptyTreasures;	/* Number of empty treasures owned */
    int		TreasuresDestroyed;	/* Number of destroyed treasures */
    int		TreasuresLeft;		/* Number of treasures left */
    int		SwapperId;		/* Player swapping to this full team */
    double	score;
    double	prev_score;
} team_t;

typedef struct item_concentrator {
    clpos	pos;
} item_concentrator_t;

typedef struct asteroid_concentrator {
    clpos	pos;
} asteroid_concentrator_t;

typedef struct friction_area {
    clpos	pos;
    double	friction_setting;	/* Setting from map */
    double	friction;		/* Changes with gameSpeed */
    int		group;
} friction_area_t;

extern bool is_polygon_map;

typedef struct {
    int		x, y;		/* Size of world in blocks */
    double	diagonal;	/* Diagonal length in blocks */
    int		width, height;	/* Size of world in pixels (optimization) */
    int		cwidth, cheight;/* Size of world in clicks */
    double	hypotenuse;	/* Diagonal length in pixels (optimization) */
    rules_t	*rules;
    char	name[MAX_CHARS];
    char	author[MAX_CHARS];
    char	dataURL[MAX_CHARS];

    u_byte	**block;	/* type of item in each block */

    vector	**gravity;

    item_t	items[NUM_ITEMS];

    asteroid_t	asteroids;

    team_t	teams[MAX_TEAMS];

    int		NumTeamBases;	/* How many 'different' teams are allowed */
    int		NumBases, MaxBases;
    base_t	*bases;
    baseorder_t	*baseorder;
    int		NumFuels, MaxFuels;
    fuel_t	*fuels;
    int		NumGravs, MaxGravs;
    grav_t	*gravs;
    int		NumCannons, MaxCannons;
    cannon_t	*cannons;
    int		NumChecks, MaxChecks;
    check_t	*checks;
    int		NumWormholes, MaxWormholes;
    wormhole_t	*wormholes;
    int		NumTreasures, MaxTreasures;
    treasure_t	*treasures;
    int		NumTargets, MaxTargets;
    target_t	*targets;
    int		NumItemConcs, MaxItemConcs;
    item_concentrator_t		*itemConcs;
    int		NumAsteroidConcs, MaxAsteroidConcs;
    asteroid_concentrator_t	*asteroidConcs;
    int		NumFrictionAreas, MaxFrictionAreas;
    friction_area_t		*frictionAreas;
} world_t;

extern world_t		World;

static inline void World_set_block(world_t *world, blpos blk, int type)
{
    assert (! (blk.bx < 0 || blk.bx >= world->x
	       || blk.by < 0 || blk.by >= world->y));
    world->block[blk.bx][blk.by] = type;
}

static inline int World_get_block(world_t *world, blpos blk)
{
    assert (! (blk.bx < 0 || blk.bx >= world->x
	       || blk.by < 0 || blk.by >= world->y));
    return world->block[blk.bx][blk.by];
}

static inline bool World_contains_clpos(world_t *world, clpos pos)
{
    if (pos.cx < 0 || pos.cx >= world->cwidth)
	return false;
    if (pos.cy < 0 || pos.cy >= world->cheight)
	return false;
    return true;
}

static inline clpos World_get_random_clpos(world_t *world)
{
    clpos pos;

    pos.cx = (int)(rfrac() * world->cwidth);
    pos.cy = (int)(rfrac() * world->cheight);

    return pos;
}

static inline int World_wrap_xclick(world_t *world, int cx)
{
    while (cx < 0)
	cx += world->cwidth;
    while (cx >= world->cwidth)
	cx -= world->cwidth;

    return cx;
}

static inline int World_wrap_yclick(world_t *world, int cy)
{
    while (cy < 0)
	cy += world->cheight;
    while (cy >= world->cheight)
	cy -= world->cheight;

    return cy;
}

static inline clpos World_wrap_clpos(world_t *world, clpos pos)
{
    pos.cx = World_wrap_xclick(world, pos.cx);
    pos.cy = World_wrap_yclick(world, pos.cy);

    return pos;
}


static inline base_t *Bases(world_t *world, int ind)
{
    return &world->bases[ind];
}

static inline fuel_t *Fuels(world_t *world, int ind)
{
    return &world->fuels[ind];
}

static inline cannon_t *Cannons(world_t *world, int ind)
{
    return &world->cannons[ind];
}

static inline check_t *Checks(world_t *world, int ind)
{
    return &world->checks[ind];
}

static inline grav_t *Gravs(world_t *world, int ind)
{
    return &world->gravs[ind];
}

static inline target_t *Targets(world_t *world, int ind)
{
    return &world->targets[ind];
}

static inline treasure_t *Treasures(world_t *world, int ind)
{
    return &world->treasures[ind];
}

static inline wormhole_t *Wormholes(world_t *world, int ind)
{
    return &world->wormholes[ind];
}

static inline asteroid_concentrator_t *AsteroidConcs(world_t *world, int ind)
{
    return &world->asteroidConcs[ind];
}

static inline item_concentrator_t *ItemConcs(world_t *world, int ind)
{
    return &world->itemConcs[ind];
}

static inline friction_area_t *FrictionAreas(world_t *world, int ind)
{
    return &world->frictionAreas[ind];
}

static inline team_t *Teams(world_t *world, int team)
{
    return &world->teams[team];
}

#endif

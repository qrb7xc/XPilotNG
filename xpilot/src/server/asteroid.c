/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *  	Kimiko Koopman        <kimiko@xpilot.org>
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

#include "xpserver.h"

char asteroid_version[] = VERSION;


/* list containing pointers to all asteroids */
static list_t	Asteroid_list = NULL;


/*
 * Prototypes.
 */
static void Make_asteroid(clpos pos, int size, int dir, DFLOAT speed);


/*
 * Return the asteroid list.
 */
list_t Asteroid_get_list(void)
{
    return Asteroid_list;
}


static bool Asteroid_add_to_list(wireobject *ast)
{
    list_iter_t		list_pos;
    bool		result = false;

    if (Asteroid_list == NULL)
	Asteroid_list = List_new();

    if (Asteroid_list != NULL) {
	list_pos = List_push_back(Asteroid_list, ast);
	if (list_pos != NULL)
	    result = true;
    }

    return result;
}


static bool Asteroid_remove_from_list(wireobject *ast)
{
    list_iter_t		list_pos;
    bool		result = false;

    if (Asteroid_list != NULL) {
	list_pos = List_find(Asteroid_list, ast);
	if (list_pos != List_end(Asteroid_list)) {
	    List_erase(Asteroid_list, list_pos);
	    result = true;
	}
    }

    return result;
}


/*
 * Breaks up an asteroid:
 * Asteroids of size > 1 break up into two asteroids of size n - 1
 * and some wreckage and debris.
 * Asteroids of size 1 break up into wreckage and debris only.
 * In both cases the sum of the masses of the smaller asteroids,
 * the wreckage and the debris should be about equal to the mass
 * of the original asteroid.
 */
void Break_asteroid(wireobject *asteroid)
{
    DFLOAT	mass, mass3;
    DFLOAT	speed, speed1, speed2, radius;
    DFLOAT	velx1, vely1, velx2, vely2;
    int		dir, dir1, dir2, split_dir;
    clpos	pos1, pos2;

    if (asteroid->size == 1) {
	mass = asteroid->mass / 2;
	Make_wreckage(asteroid->pos,
		      asteroid->vel,
		      NO_ID,
		      TEAM_NOT_SET,
		      mass / 20, mass / 3,
		      mass,
		      GRAVITY,
		      WHITE,
		      10,
		      0, RES-1,
		      5, 10,
		      3, 10);
	Make_debris(asteroid->pos,
		    asteroid->vel,
		    NO_ID,
		    TEAM_NOT_SET,
		    OBJ_DEBRIS,
		    mass,
		    GRAVITY,
		    RED,
		    8,
		    20 + 30 * rfrac(),
		    0, RES-1,
		    5, 10,
		    3, 10);
    } else {
	/* foo[12] refer to the mini-asteroids
	   foo3 refers to the wreckage and debris */
	speed = VECTOR_LENGTH(asteroid->vel);
	dir = (int)findDir(asteroid->vel.x, asteroid->vel.y);
	mass3 = asteroid->mass * ASTEROID_DUST_FACT;
	mass = ASTEROID_MASS(asteroid->size - 1);
	dir1 = MOD2((int)(dir
			  - ASTEROID_DELTA_DIR / 4
			  - (rfrac() * ASTEROID_DELTA_DIR / 4)), RES);
	dir2 = MOD2((int)(dir
			  + ASTEROID_DELTA_DIR / 4
			  + (rfrac() * ASTEROID_DELTA_DIR / 4)), RES);
	speed1 = (speed * (1 - ASTEROID_DUST_FACT))
		 / tcos(ABS(dir - dir1));
	speed2 = (speed * (1 - ASTEROID_DUST_FACT))
		 / tcos(ABS(dir2 - dir));
	velx1 = tcos(dir1) * speed1;
	vely1 = tsin(dir1) * speed1;
	velx2 = tcos(dir2) * speed2;
	vely2 = tsin(dir2) * speed2;
	split_dir = MOD2(dir - RES/4, RES);
	radius = ASTEROID_RADIUS(asteroid->size - 1);
	pos1.cx = WRAP_XCLICK(asteroid->pos.cx + tcos(split_dir) * radius);
	pos1.cy = WRAP_YCLICK(asteroid->pos.cy + tsin(split_dir) * radius);
	pos2.cx = WRAP_XCLICK(asteroid->pos.cx - tcos(split_dir) * radius);
	pos2.cy = WRAP_YCLICK(asteroid->pos.cy - tsin(split_dir) * radius);
	Make_asteroid(pos1, asteroid->size - 1, dir1, speed1);
	Make_asteroid(pos2, asteroid->size - 1, dir2, speed2);
	Make_wreckage(asteroid->pos,
		      asteroid->vel,
		      NO_ID,
		      TEAM_NOT_SET,
		      mass3 / 20, mass3 / 3,
		      mass3 / 2,
		      GRAVITY,
		      WHITE,
		      10,
		      0, RES-1,
		      5, 10,
		      3, 10);
	Make_debris(asteroid->pos,
		    asteroid->vel,
		    NO_ID,
		    TEAM_NOT_SET,
		    OBJ_DEBRIS,
		    mass3 / 2,
		    GRAVITY,
		    RED,
		    8,
		    20 + 30 * rfrac(),
		    0, RES-1,
		    5, 10,
		    3, 10);
    }

    if ((asteroidMaxItems > 0) && (rfrac() < asteroidItemProb)) {
	int	nitems = (int)(rfrac() * asteroidMaxItems) + 1;
	int	i;
	vector	vel;
	int	item, item_dir, num_per_pack;
	DFLOAT	item_speed;
	long	status;

	for (i = 0; i < nitems; i++) {
	    item = Choose_random_item();
	    item_dir = (int)(rfrac() * RES);
	    item_speed = rfrac() * 10;
	    vel.x = asteroid->vel.x + item_speed * tcos(item_dir);
	    vel.y = asteroid->vel.y + item_speed * tsin(item_dir);
	    status = GRAVITY;
	    if (rfrac() < randomItemProb)
		status |= RANDOM_ITEM;
	    if (World.items[item].min_per_pack
		== World.items[item].max_per_pack)
		num_per_pack = World.items[item].max_per_pack;
	    else
		num_per_pack = World.items[item].min_per_pack
		    + (int)(rfrac() * (1 + World.items[item].max_per_pack
				       - World.items[item].min_per_pack));

	    Make_item(asteroid->pos, vel,
		      item, num_per_pack,
		      status);
	}
    }

    sound_play_sensors(asteroid->pos, ASTEROID_BREAK_SOUND);

    World.asteroids.num -= 1 << (asteroid->size - 1);

    Asteroid_remove_from_list(asteroid);
}


/*
 * Creates an asteroid with the given characteristics.
 */
static void Make_asteroid(clpos pos, int size, int dir, DFLOAT speed)
{
    wireobject	*asteroid;
    DFLOAT	radius;

    if (NumObjs >= MAX_TOTAL_SHOTS)
	return;

    if (size < 1 || size > ASTEROID_MAX_SIZE)
	return;

    pos.cx = WRAP_XCLICK(pos.cx);
    pos.cy = WRAP_YCLICK(pos.cy);
    if (!INSIDE_MAP(pos.cx, pos.cy))
	return;

    /*
     * kps - here we should check that the asteroid shape is not inside
     * a polygon (or on top of an object?)
     */
    if (is_inside(pos.cx, pos.cy, NOTEAM_BIT | NONBALL_BIT, NULL) != NO_GROUP)
	return;

    asteroid = WIRE_PTR(Object_allocate());
    if (asteroid == NULL)
	return;

    asteroid->color = WHITE;
    asteroid->id = NO_ID;
    asteroid->team = TEAM_NOT_SET;
    asteroid->type = OBJ_ASTEROID;

    /* Position */
    Object_position_init_clicks(OBJ_PTR(asteroid), pos.cx, pos.cy);

    asteroid->vel.x = tcos(dir) * speed;
    asteroid->vel.y = tsin(dir) * speed;
    asteroid->acc.x = asteroid->acc.y = 0;
    asteroid->mass = ASTEROID_MASS(size);
    asteroid->life = ASTEROID_LIFE;
    asteroid->turnspeed = 0.02 + rfrac() * 0.05;
    asteroid->rotation = (int)(rfrac() * RES);
    asteroid->size = size;
    asteroid->info = (int)(rfrac() * 256);
    radius = ASTEROID_RADIUS(size) / CLICK;
    asteroid->pl_range = radius;
    asteroid->pl_radius = radius;
    asteroid->fusetime = frame_time + timeStep;
    asteroid->status = GRAVITY;
    CLEAR_MODS(asteroid->mods);

    if (Asteroid_add_to_list(asteroid) == true) {
	World.asteroids.num += 1 << (size - 1);
	Cell_add_object(OBJ_PTR(asteroid));
    }
    else
	Object_free_ptr(OBJ_PTR(asteroid));
}


/*
 * Tries to place a new asteroid on the map.
 * Calls Make_asteroid() to actually create the new asteroid
 */
/* kps - change this to use polygon based is_inside code */
static void Place_asteroid(void)
{
    int			place_count;
    int			bx, by;
    int			dir, dist;
    unsigned		space;
    int			okay;
    asteroid_concentrator_t	*con;
    clpos		pos;

    space = SPACE_BLOCKS;
    space &= ~(BASE_BIT | WORMHOLE_BIT);
    space |= FRICTION_BIT;
    /* would be dubious: space |= CANNON_BIT; */

    if (World.NumAsteroidConcs > 0 && rfrac() < asteroidConcentratorProb)
	con = AsteroidConcs((int)(rfrac() * World.NumAsteroidConcs));
    else
	con = NULL;

    /*
     * We bail out after some unsuccessful attempts to avoid wasting
     * too much time on crowded maps.
     */
    okay = false;
    for (place_count = 0; okay != true; place_count++) {
	if (place_count >= 10)
	    return;

	if (con) {
	    dir = (int)(rfrac() * RES);
	    dist = (int)(rfrac() * ((asteroidConcentratorRadius
				     * BLOCK_CLICKS) + 1));
	    pos.cx = con->pos.cx + dist * tcos(dir);
	    pos.cy = con->pos.cy + dist * tsin(dir);
	    pos.cx = WRAP_XCLICK(pos.cx);
	    pos.cy = WRAP_YCLICK(pos.cy);
	    if (!INSIDE_MAP(pos.cx, pos.cy))
		continue;
	} else {
	    pos.cx = (int)(rfrac() * World.cwidth);
	    pos.cy = (int)(rfrac() * World.cheight);
	}
	bx = CLICK_TO_BLOCK(pos.cx);
	by = CLICK_TO_BLOCK(pos.cy);

	/* kps - World.block must be removed */
	if (BIT(1U << World.block[bx][by], space)) {
	    int i, dpx, dpy;

	    okay = true;

	    for (i = 0; i < NumPlayers; i++) {
		player *pl = Players(i);
		if (IS_HUMAN_PTR(pl)) {
		    dpx = WRAP_DCX(pos.cx - pl->pos.cx);
		    dpy = WRAP_DCY(pos.cy - pl->pos.cy);
		    if (QUICK_LENGTH(dpx, dpy) < 2 * ASTEROID_MIN_DIST
			&& sqr(dpx) + sqr(dpy) < sqr(ASTEROID_MIN_DIST)) {
			/* too close to player */
			okay = false;
			break;
		    }
		}
	    }
	}
    }
    if (okay == true) {
	Make_asteroid(pos,
		      (int)(1 + rfrac() * ASTEROID_MAX_SIZE),
		      (int)(rfrac() * RES),
		      (DFLOAT)ASTEROID_START_SPEED);
    }
}


static void Asteroid_move(wireobject *wireobj)
{
    Move_object(OBJ_PTR(wireobj));
}


static void Asteroid_rotate(wireobject *wireobj)
{
    wireobj->rotation =
	(wireobj->rotation + (int) (wireobj->turnspeed * RES)) % RES;
}


/*
 * Called once each frame update to update everything
 * related to asteroids including creation, destruction,
 * rotation and movement.
 */
void Asteroid_update(void)
{
    int		num;
    list_t	list;
    list_iter_t	iter;
    wireobject	*asteroid;

    list = Asteroid_get_list();
    if (list) {
	/*
	 * if there are more asteroids than are wanted, mark
	 * all asteroids to be removed (by Delete_shot()),
	 * until enough of size 1 have been removed
	 * (only breaking of size 1 actually lowers the
	 * total number of asteroids)
	 * one iteration may not remove enough asteroids
	 * the rest are left until the next frame then
	 */
	num = World.asteroids.num;
	if (num > World.asteroids.max) {
	    for (iter = List_begin(list);
		 iter != List_end(list);
		 LI_FORWARD(iter)) {
		asteroid = (wireobject *) LI_DATA(iter);
		if (asteroid->life > 0) {
		    asteroid->life = 0;
		    if (asteroid->size == 1)
			num--;
		}
		if (num <= World.asteroids.max)
		    break;
	    }
	}

	/* rotate asteroids */
	for (iter = List_begin(list);
	     iter != List_end(list);
	     LI_FORWARD(iter)) {
	    asteroid = (wireobject *) LI_DATA(iter);
	    if (asteroid->life > 0)
		Asteroid_rotate(asteroid);
	}

	/* move asteroids */
	for (iter = List_begin(list);
	     iter != List_end(list);
	     LI_FORWARD(iter)) {
	    asteroid = (wireobject *) LI_DATA(iter);
	    if (asteroid->life > 0)
		Asteroid_move(asteroid);
	}
    }

    /* place new asteroid if room left */
    if (World.asteroids.chance > 0) {
	int incr = (1 << (ASTEROID_MAX_SIZE - 1));
	if (World.asteroids.num + incr < World.asteroids.max) {
	    if ((rfrac() * World.asteroids.chance) < 1.0f)
		Place_asteroid();
	}
    }
}

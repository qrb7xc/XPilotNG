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

#include "xpserver.h"

char player_version[] = VERSION;


bool		updateScores = true;

static int	playerArrayNumber;

/*
 * Get player with index 'ind' from Players array.
 */
player *Players(int ind)
{
    if (ind == -1)
	return NULL;

    if (ind < 0 || ind >= playerArrayNumber) {
	warn("Players: ind = %d, array size = %d\n",
	     ind, playerArrayNumber);
	return NULL;
    }
    return PlayersArray[ind];
}

/*
 * Get index in Players array for player with id 'id'.
 */
int GetInd(int id)
{
    if (id == NO_ID)
	return -1;

    /*
     * kps - in some places where we look at the id we don't
     * bother about observers (maybe observers should be
     * renamed to spectators).
     * This should be cleaned up in general.
     */
    if (id < 0 || id >= NUM_IDS + MAX_OBSERVERS + 1) {
	warn("GetInd: id = %d, array size = %d\n",
	     id, NUM_IDS + MAX_OBSERVERS + 1);
	return -1;
    }
    return GetIndArray[id];
}

/********* **********
 * Functions on player array.
 */

void Pick_startpos(player *pl)
{
    int		ind = GetInd(pl->id);
    int		i, num_free;
    int		pick = 0, seen = 0;
    static int	prev_num_bases = 0;
    static char	*free_bases = NULL;

    if (IS_TANK_PTR(pl)) {
	pl->home_base = Bases(0);
	return;
    }

    if (prev_num_bases != World.NumBases) {
	prev_num_bases = World.NumBases;
	if (free_bases != NULL)
	    free(free_bases);
	free_bases = (char *) malloc(World.NumBases * sizeof(*free_bases));
	if (free_bases == NULL) {
	    error("Can't allocate memory for free_bases");
	    End_game();
	}
    }

    num_free = 0;
    for (i = 0; i < World.NumBases; i++) {
	if (World.bases[i].team == pl->team) {
	    num_free++;
	    free_bases[i] = 1;
	} else
	    free_bases[i] = 0;	/* other team */
    }

    for (i = 0; i < NumPlayers; i++) {
	player *pl_i = Players(i);

	if (pl_i->id != pl->id && !IS_TANK_PTR(pl_i) && pl_i->home_base
		    && free_bases[pl_i->home_base->ind]) {
	    free_bases[pl_i->home_base->ind] = 0;	/* occupado */
	    num_free--;
	}
    }

    if (BIT(World.rules->mode, TIMING)) {	/* pick first free base */
	for (i = 0; i < World.NumBases; i++) {
	    /* kps - ng wants no base orders => change to if (free_bases[i]) */
	    if (free_bases[World.baseorder[i].base_idx])
		break;
	}
    } else {
	pick = (int)(rfrac() * num_free);
	seen = 0;
	for (i = 0; i < World.NumBases; i++) {
	    if (free_bases[i] != 0) {
		if (seen < pick)
		    seen++;
		else
		    break;
	    }
	}
    }

    if (i == World.NumBases) {
	error("Can't pick startpos (ind=%d,num=%d,free=%d,pick=%d,seen=%d)",
	      ind, World.NumBases, num_free, pick, seen);
	End_game();
    } else {
	pl->home_base = Bases(BIT(World.rules->mode, TIMING) ?
			      World.baseorder[i].base_idx : i);
	if (ind < NumPlayers) {
	    for (i = 0; i < observerStart + NumObservers; i++) {
		player *pl_i;

		if (i == NumPlayers) {
		    i = observerStart - 1;
		    continue;
		}
		pl_i = Players(i);
		if (pl_i->conn != NULL)
		    Send_base(pl_i->conn, pl->id, pl->home_base->ind);
	    }
	    if (BIT(pl->status, PLAYING) == 0)
		pl->count = RECOVERY_DELAY;
	    else if (BIT(pl->status, PAUSE|GAME_OVER))
		Go_home(pl);
	}
    }
}


void Go_home(player *pl)
{
    int			ind = GetInd(pl->id);
    int			i, dir, check;
    double		vx, vy, velo;
    clpos		pos;

    if (IS_TANK_PTR(pl)) {
	/*NOTREACHED*/
	/* Tanks have no homebase. */
	warn("BUG: gohome tank");
	return;
    }

    if (BIT(World.rules->mode, TIMING)
	&& pl->round
	&& !BIT(pl->status, GAME_OVER)) {
	if (pl->check)
	    check = pl->check - 1;
	else
	    check = World.NumChecks - 1;
	pos = Checks(check)->pos;
	vx = (rfrac() - 0.5) * 0.1;
	vy = (rfrac() - 0.5) * 0.1;
	velo = LENGTH(vx, vy);
	dir = pl->last_check_dir;
	dir = MOD2(dir + (int)((rfrac() - 0.5) * (RES / 8)), RES);
    } else if (pl->home_base != NULL) {
	pos = pl->home_base->pos;
	dir = pl->home_base->dir;
	vx = vy = velo = 0;
    } else
	pos.cx = pos.cy = dir = vx = vy = velo = 0;

    pl->dir = dir;
    Player_set_float_dir(pl, (double)dir);
    Player_position_init_clicks(
	pl, (int)(pos.cx + CLICK * vx),	(int)(pos.cy + CLICK * vy));
    pl->vel.x = vx;
    pl->vel.y = vy;
    pl->velocity = velo;
    pl->acc.x = pl->acc.y = 0.0;
    pl->turnacc = pl->turnvel = 0.0;
    memset(pl->last_keyv, 0, sizeof(pl->last_keyv));
    memset(pl->prev_keyv, 0, sizeof(pl->prev_keyv));
    Emergency_shield(pl, false);
    Player_used_kill(pl);

    if (playerStartsShielded != 0) {
	SET_BIT(pl->used, HAS_SHIELD);
	if (playerShielding == 0) {
	    pl->shield_time = SHIELD_TIME;
	    SET_BIT(pl->have, HAS_SHIELD);
	}
	if (BIT(pl->have, HAS_DEFLECTOR))
	    Deflector(pl, true);
    }
    CLR_BIT(pl->status, THRUSTING);
    pl->updateVisibility = 1;
    for (i = 0; i < NumPlayers; i++) {
	pl->visibility[i].lastChange = 0;
	Players(i)->visibility[ind].lastChange = 0;
    }

    if (IS_ROBOT_PTR(pl))
	Robot_go_home(pl);
}

/*
 * Compute the current sensor range for player `pl'.  This is based on the
 * amount of fuel, the number of sensor items (each one adds 25%), and the
 * minimum and maximum visibility limits in effect.
 */
void Compute_sensor_range(player *pl)
{
    static int		init = 0;
    static double	EnergyRangeFactor;

    if (!init) {
	if (minVisibilityDistance <= 0.0)
	    minVisibilityDistance = VISIBILITY_DISTANCE;
	else
	    minVisibilityDistance *= BLOCK_SZ;
	if (maxVisibilityDistance <= 0.0)
	    maxVisibilityDistance = World.hypotenuse;
	else
	    maxVisibilityDistance *= BLOCK_SZ;

	if (World.items[ITEM_FUEL].initial > 0.0) {
	    EnergyRangeFactor = minVisibilityDistance /
		(World.items[ITEM_FUEL].initial
		 * (1.0 + ((double)World.items[ITEM_SENSOR].initial * 0.25)));
	} else
	    EnergyRangeFactor = ENERGY_RANGE_FACTOR;
	init = 1;
    }

    pl->sensor_range = pl->fuel.sum * EnergyRangeFactor;
    pl->sensor_range *= (1.0 + ((double)pl->item[ITEM_SENSOR] * 0.25));
    if (pl->sensor_range < minVisibilityDistance)
	pl->sensor_range = minVisibilityDistance;
    if (pl->sensor_range > maxVisibilityDistance)
	pl->sensor_range = maxVisibilityDistance;
}

/*
 * Give ship one more tank, if possible.
 */
void Player_add_tank(player *pl, double tank_fuel)
{
    double		tank_cap, add_fuel;

    if (pl->fuel.num_tanks < MAX_TANKS) {
	pl->fuel.num_tanks++;
	tank_cap = TANK_CAP(pl->fuel.num_tanks);
	add_fuel = tank_fuel;
	LIMIT(add_fuel, 0.0, tank_cap);
	pl->fuel.sum += add_fuel;
	pl->fuel.max += tank_cap;
	pl->fuel.tank[pl->fuel.num_tanks] = add_fuel;
	pl->emptymass += TANK_MASS;
	pl->item[ITEM_TANK] = pl->fuel.num_tanks;
    }
}

/*
 * Remove a tank from a ship, if possible.
 */
void Player_remove_tank(player *pl, int which_tank)
{
    int			i, tank_ind;
    long		tank_fuel, tank_cap;

    if (pl->fuel.num_tanks > 0) {
	tank_ind = which_tank;
	LIMIT(tank_ind, 1, pl->fuel.num_tanks);
	pl->emptymass -= TANK_MASS;
	tank_fuel = pl->fuel.tank[tank_ind];
	tank_cap = TANK_CAP(tank_ind);
	pl->fuel.max -= tank_cap;
	pl->fuel.sum -= tank_fuel;
	pl->fuel.num_tanks--;
	if (pl->fuel.current > pl->fuel.num_tanks)
	    pl->fuel.current = 0;
	else {
	    for (i = tank_ind; i <= pl->fuel.num_tanks; i++)
		pl->fuel.tank[i] = pl->fuel.tank[i + 1];
	}
	pl->item[ITEM_TANK] = pl->fuel.num_tanks;
    }
}

void Player_hit_armor(player *pl)
{
    if (--pl->item[ITEM_ARMOR] <= 0)
	CLR_BIT(pl->have, HAS_ARMOR);
}

/*
 * Clear used bits.
 */
void Player_used_kill(player *pl)
{
    pl->used &= ~USED_KILL;
    if (!BIT(DEF_HAVE, HAS_SHIELD))
	CLR_BIT(pl->have, HAS_SHIELD);
}

/*
 * Give player the initial number of tanks and amount of fuel.
 * Upto the maximum allowed.
 */
static void Player_init_fuel(player *pl, double total_fuel)
{
    double		fuel = total_fuel;
    int			i;

    pl->fuel.num_tanks  = 0;
    pl->fuel.current    = 0;
    pl->fuel.max	= TANK_CAP(0);
    pl->fuel.sum	= MIN(fuel, pl->fuel.max);
    pl->fuel.tank[0]	= pl->fuel.sum;
    pl->emptymass	= ShipMass;
    pl->item[ITEM_TANK]	= pl->fuel.num_tanks;

    fuel -= pl->fuel.sum;

    for (i = 1; i <= World.items[ITEM_TANK].initial; i++) {
	Player_add_tank(pl, fuel);
	fuel -= pl->fuel.tank[i];
    }
}

int Init_player(int ind, shipshape_t *ship)
{
    player		*pl = Players(ind);
    bool		too_late = false;
    int			i;

    /*memset(pl, 0, sizeof(player));*/

    pl->vel.x = pl->vel.y = 0.0;
    pl->acc.x = pl->acc.y = 0.0;
    pl->dir = DIR_UP;
    Player_set_float_dir(pl, (double)pl->dir);

    pl->turnvel		= 0.0;
    pl->oldturnvel	= 0.0;
    pl->turnacc		= 0.0;
    pl->mass		= ShipMass;
    pl->emptymass	= ShipMass;

    for (i = 0; i < NUM_ITEMS; i++) {
	if (!BIT(1U << i, ITEM_BIT_FUEL | ITEM_BIT_TANK))
	    pl->item[i] = World.items[i].initial;
    }

    pl->fuel.sum = World.items[ITEM_FUEL].initial;
    Player_init_fuel(pl, pl->fuel.sum);

    /*
     * If you don't want to allow shipshapes because the shape
     * requirements have not been rewritten yet, set
     * allowShipShapes to false.
     */
    if (allowShipShapes && ship)
	pl->ship = ship;
    else {
	shipshape_t *tryship = Parse_shape_str(defaultShipShape);

	if (tryship)
	    pl->ship = tryship;
	else
	    pl->ship = Default_ship();
    }

    pl->power			= 55.0;
    pl->turnspeed		= 10.0;
    pl->turnresistance		= 0.0;
    pl->power_s			= 25.0;
    pl->turnspeed_s		= 7;
    pl->turnresistance_s	= 0;

    pl->check		= 0;
    pl->round		= 0;
    pl->time		= 0;
    pl->last_lap_time	= 0;
    pl->last_lap	= 0;
    pl->best_lap	= 0;
    pl->count		= -1;
    pl->shield_time	= 0;
    pl->last_wall_touch	= 0;

    pl->type		= OBJ_PLAYER;
    pl->type_ext	= 0;		/* assume human player */
    pl->shots		= 0;
    pl->missile_rack	= 0;
    pl->forceVisible	= 0;
    Compute_sensor_range(pl);
    pl->shot_time	= 0;
    pl->laser_time	= 0;
    pl->color		= WHITE;
    pl->score		= 0;
    pl->prev_score	= 0;
    pl->prev_check	= 0;
    pl->prev_round	= 0;
    pl->fs		= 0;
    pl->repair_target	= 0;
    pl->name[0]		= '\0';
    /*pl->auth_nick[0]	= 0;*/
    pl->num_pulses	= 0;
    pl->emergency_thrust_left = 0;
    pl->emergency_shield_left = 0;
    pl->phasing_left	= 0;
    pl->ecmcount	= 0;
    pl->damaged 	= 0;
    pl->stunned		= 0;

    pl->status		= PLAYING | GRAVITY | DEF_BITS;
    pl->have		= DEF_HAVE;
    pl->used		= DEF_USED;

    if (pl->item[ITEM_CLOAK] > 0)
	SET_BIT(pl->have, HAS_CLOAKING_DEVICE);

    CLEAR_MODS(pl->mods);
    for (i = 0; i < NUM_MODBANKS; i++)
	CLEAR_MODS(pl->modbank[i]);
    for (i = 0; i < LOCKBANK_MAX; i++)
	pl->lockbank[i] = -1;

    {
	static unsigned short	pseudo_team_no = 0;
	pl->pseudo_team = pseudo_team_no++;
    }
    pl->mychar		= ' ';
    pl->prev_mychar	= pl->mychar;
    pl->life		= World.rules->lives;
    pl->prev_life	= pl->life;
    pl->ball 		= NULL;

    pl->player_fps	= 50; /* Client should send a value after startup */

    Rank_ClearKills(pl);
    Rank_ClearDeaths(pl);

    pl->idleCount = 0;
    pl->flooding = -1;

    /*
     * If limited lives you will have to wait 'til everyone gets GAME OVER.
     *
     * Indeed you have to! (Mara)
     *
     * At least don't make the player wait for a new round if he's the
     * only one on the server. Mara's change (always too_late) meant
     * there was a round reset when the first player joined. -uau
     */
    if (BIT(World.rules->mode, LIMITED_LIVES) && NumPlayers > 0)
	too_late = true;

    if (too_late) {
	pl->mychar	= 'W';
	pl->prev_life = pl->life = 0;
	SET_BIT(pl->status, GAME_OVER);
    }

    pl->team		= TEAM_NOT_SET;

    pl->alliance	= ALLIANCE_NOT_SET;
    pl->prev_alliance	= ALLIANCE_NOT_SET;
    pl->invite		= NO_ID;

    pl->lock.tagged	= LOCK_NONE;
    pl->lock.pl_id	= 0;

    pl->robot_data_ptr	= NULL;

    pl->id		= peek_ID();
    GetIndArray[pl->id]	= ind;
    pl->conn		= NULL;
    pl->audio		= NULL;

    pl->lose_item	= 0;
    pl->lose_item_state	= 0;

    pl->shove_next = 0;
    for (i = 0; i < MAX_RECORDED_SHOVES; i++)
	pl->shove_record[i].pusher_id = NO_ID;

    pl->frame_last_busy	= frame_loops;

    pl->isowner = 0;
    pl->isoperator = 0;
    pl->privs = 0;

    pl->rectype = 0;

    return pl->id;
}


static player			*playerArray;
static struct _visibility	*visibilityArray;

void Alloc_players(int number)
{
    player *p;
    struct _visibility *t;
    size_t n = number;
    int i;

    /* kps - fix this so you can memset the player struct to 0 later */

    /* Allocate space for pointers */
    PlayersArray = (player **) calloc(n, sizeof(player *));

    /* Allocate space for all entries, all player structs */
    p = playerArray = (player *) calloc(n, sizeof(player));

    /* Allocate space for all visibility arrays, n arrays of n entries */
    t = visibilityArray =
	(struct _visibility *) calloc(n * n, sizeof(struct _visibility));

    if (!PlayersArray || !playerArray || !visibilityArray) {
	error("Not enough memory for Players.");
	exit(1);
    }

    for (i = 0; i < number; i++) {
	PlayersArray[i] = p++;
	PlayersArray[i]->visibility = t;
	/* Advance to next block/array */
	t += number;
    }

    playerArrayNumber = number;
}



void Free_players(void)
{
#if 1
    if (PlayersArray) {
	free(PlayersArray);
	PlayersArray = NULL;

	free(playerArray);
	free(visibilityArray);
    }
#else /* ng wants this, fix later */
    free(PlayersArray);
    free(playerArray);
    free(visibilityArray);
#endif
}



void Update_score_table(void)
{
    int			i, j, check;
    player		*pl;

    for (j = 0; j < NumPlayers; j++) {
	pl = Players(j);
	if (pl->score != pl->prev_score
	    || pl->life != pl->prev_life
	    || pl->mychar != pl->prev_mychar
	    || pl->alliance != pl->prev_alliance) {
	    pl->prev_score = pl->score;
	    pl->prev_life = pl->life;
	    pl->prev_mychar = pl->mychar;
	    pl->prev_alliance = pl->alliance;
	    for (i = 0; i < NumPlayers; i++) {
		player *pl_i = Players(i);
		if (pl_i->conn != NULL)
		    Send_score(pl_i->conn, pl->id, pl->score, (int)pl->life,
			       pl->mychar, pl->alliance);
	    }
	    for (i = 0; i < NumObservers; i++)
		Send_score(Players(i + observerStart)->conn, pl->id,
			   pl->score, (int)pl->life, pl->mychar, pl->alliance);
	}
	if (BIT(World.rules->mode, TIMING)) {
	    if (pl->check != pl->prev_check
		|| pl->round != pl->prev_round) {
		pl->prev_check = pl->check;
		pl->prev_round = pl->round;
		check = (pl->round == 0)
			    ? 0
			    : (pl->check == 0)
				? (World.NumChecks - 1)
				: (pl->check - 1);
		for (i = 0; i < NumPlayers; i++) {
		    player *pl_i = Players(i);
		    if (pl_i->conn != NULL)
			Send_timing(pl_i->conn, pl->id, check, pl->round);
		}
	    }
	}
    }
    if (BIT(World.rules->mode, TEAM_PLAY)) {
	team_t	*team;
	for (j = 0; j < MAX_TEAMS; j++) {
	    team = &(World.teams[j]);
	    if (team->score != team->prev_score) {
		team->prev_score = team->score;
		for (i = 0; i < NumPlayers; i++) {
		    player *pl_i = Players(i);
		    if (pl_i->conn != NULL)
			Send_team_score(pl_i->conn, j, team->score);
		}
	    }
	}
    }
    updateScores = false;
#ifdef _WINDOWS
    SendDialogUpdate();
#endif
}


void Reset_all_players(void)
{
    player		*pl;
    int			i, j;
    char		msg[MSG_LEN];

    updateScores = true;

    for (i = 0; i < NumPlayers; i++) {
	pl = Players(i);
	if (endOfRoundReset) {
	    if (BIT(pl->status, PAUSE))
		Player_death_reset(pl, false);
	    else {
		Kill_player(pl, false);
		if (pl != Players(i)) {
		    i--;
		    continue;
		}
	    }
	}
	CLR_BIT(pl->status, GAME_OVER);
	CLR_BIT(pl->have, HAS_BALL);
	Rank_ClearKills(pl);
	Rank_ClearDeaths(pl);
	/* This has already been changed to 'D' at this point*/
	/* not always - kps */
	/*xpprintf("%s %c \n", pl->name, pl->mychar);*/

	if (!BIT(pl->status, PAUSE) && pl->mychar != 'W')
	    Rank_AddRound(pl);
	pl->round = 0;
	pl->check = 0;
	pl->time = 0;
	pl->best_lap = 0;
	pl->last_lap = 0;
	pl->last_lap_time = 0;
	if (!BIT(pl->status, PAUSE)) {
	    pl->mychar = ' ';
	    pl->frame_last_busy = frame_loops;
	    pl->life = World.rules->lives;
	    if (BIT(World.rules->mode, TIMING))
		pl->count = RECOVERY_DELAY;
	}
	if (IS_TANK_PTR(pl))
	    pl->mychar = 'T';
	else if (IS_ROBOT_PTR(pl))
	    pl->mychar = 'R';
    }
    if (BIT(World.rules->mode, TEAM_PLAY)) {

	/* Detach any balls and kill ball */
	/* We are starting all over again */
	for (j = NumObjs - 1; j >= 0 ; j--) {
	    if (BIT(Obj[j]->type, OBJ_BALL)) {
		ballobject *ball = BALL_IND(j);
		ball->id = NO_ID;
		ball->life = 0;
		ball->owner = 0;	/* why not -1 ??? */
		CLR_BIT(ball->status, RECREATE);
		Delete_shot(j);
	    }
	}

	/* Reset the treasures */
	for (i = 0; i < World.NumTreasures; i++) {
	    treasure_t *treasure = Treasures(i);

	    treasure->destroyed = 0;
	    treasure->have = false;
	    Make_treasure_ball(treasure);
	}

	/* Reset the teams */
	for (i = 0; i < MAX_TEAMS; i++) {
	    team_t *team = Teams(i);

	    team->TreasuresDestroyed = 0;
	    team->TreasuresLeft = team->NumTreasures - team->NumEmptyTreasures;
	}

	if (endOfRoundReset) {
	    /* Reset the targets */
	    for (i = 0; i < World.NumTargets; i++) {
		target_t *targ = Targets(i);

		if (targ->damage != TARGET_DAMAGE || targ->dead_time > 0)
		    Target_restore_on_map(targ);
	    }
	}
    }

    if (endOfRoundReset) {
	for (i = 0; i < NumObjs; i++) {
	    object *obj = Obj[i];
	    if (BIT(obj->type, OBJ_SHOT|OBJ_MINE|OBJ_DEBRIS|OBJ_SPARK
			       |OBJ_CANNON_SHOT|OBJ_TORPEDO|OBJ_SMART_SHOT
			       |OBJ_HEAT_SHOT|OBJ_ITEM)) {
		obj->life = 0;
		if (BIT(obj->type, OBJ_TORPEDO|OBJ_SMART_SHOT|OBJ_HEAT_SHOT
				   |OBJ_CANNON_SHOT|OBJ_MINE))
		    /* Take care that no new explosions are made. */
		    obj->mass = 0;
	    }
	}
    }

    if (round_delay_send > 0)
	round_delay_send--;
    if (roundDelaySeconds) {
	/* Hold your horses! The next round will start in a few moments. */
	round_delay = roundDelaySeconds * FPS;
	/* Send him an extra seconds worth to be sure he gets the 0. */
	round_delay_send = round_delay+FPS;
	roundtime = -1;
	sprintf(msg, "Delaying %d seconds until start of next %s.",
		roundDelaySeconds,
		(BIT(World.rules->mode, TIMING)? "race" : "round"));
	Set_message(msg);
    } else
	roundtime = maxRoundTime * FPS;

    Update_score_table();
}


void Check_team_members(int team)
{
    player		*pl;
    team_t		*teamp;
    int			members, i;

    if (!BIT(World.rules->mode, TEAM_PLAY))
	return;

    for (members = i = 0; i < NumPlayers; i++) {
	pl = Players(i);
	if (!IS_TANK_PTR(pl) && pl->team == team && pl->home_base != NULL)
	    members++;
    }
    teamp = Teams(team);
    if (teamp->NumMembers != members) {
	warn("Server has reset team %d members from %d to %d",
	     team, teamp->NumMembers, members);
	for (i = 0; i < NumPlayers; i++) {
	    pl = Players(i);
	    if (!IS_TANK_PTR(pl) && pl->team == team && pl->home_base != NULL)
		warn("Team %d currently has player %d: \"%s\"",
		     team, i+1, pl->name);
	}
	teamp->NumMembers = members;
    }

    if (teamcup)
	teamcup_round_start();
}


static void Compute_end_of_round_values(double *average_score,
					int *num_best_players,
					double *best_ratio,
					int best_players[])
{
    int			i, n = 0;
    double		ratio;

    /* Initialize everything */
    *average_score = 0;
    *num_best_players = 0;
    *best_ratio = -1.0;

    /* Figure out what the average score is and who has the best kill/death */
    /* ratio for this round */
    for (i = 0; i < NumPlayers; i++) {
	player *pl_i = Players(i);

	if (IS_TANK_PTR(pl_i)
	    || (BIT(pl_i->status, PAUSE)
	       && pl_i->count <= 0)) {
	    continue;
	}
	n++;
	*average_score += pl_i->score;
	ratio = (double) pl_i->kills / (pl_i->deaths + 1);
	if (ratio > *best_ratio) {
	    *best_ratio = ratio;
	    best_players[0] = i;
	    *num_best_players = 1;
	} else if (ratio == *best_ratio)
	    best_players[(*num_best_players)++] = i;
    }
    if (n != 0)  /* Can this be 0? */
	*average_score /= n;
}


static void Give_best_player_bonus(double average_score,
				   int num_best_players,
				   double best_ratio,
				   int best_players[])
{
    int			i;
    double		points;
    char		msg[MSG_LEN];

    if (best_ratio == 0)
	sprintf(msg, "There is no Deadly Player.");
    else if (num_best_players == 1) {
	player *bp = Players(best_players[0]);

	sprintf(msg,
		"%s is the Deadliest Player with a kill ratio of %d/%d.",
		bp->name,
		bp->kills, bp->deaths);
	points = best_ratio * Rate(bp->score, average_score);
	Score(bp, points, bp->pos, "[Deadliest]");
    } else {
	msg[0] = '\0';
	for (i = 0; i < num_best_players; i++) {
	    player	*bp = Players(best_players[i]);
	    double	ratio = Rate(bp->score, average_score);
	    double	score = (ratio + num_best_players) / num_best_players;

	    if (msg[0]) {
		if (i == num_best_players - 1)
		    strcat(msg, " and ");
		else
		    strcat(msg, ", ");
	    }
	    if (strlen(msg) + 8 + strlen(bp->name) >= sizeof(msg)) {
		Set_message(msg);
		msg[0] = '\0';
	    }
	    strcat(msg, bp->name);
	    points = best_ratio * score;
	    Score(bp, points, bp->pos, "[Deadly]");
	}
	if (strlen(msg) + 64 >= sizeof(msg)) {
	    Set_message(msg);
	    msg[0] = '\0';
	}
	sprintf(msg + strlen(msg),
		" are the Deadly Players with kill ratios of %d/%d.",
		Players(best_players[0])->kills,
		Players(best_players[0])->deaths);
    }
    Set_message(msg);
}

static void Give_individual_bonus(player *pl, double average_score)
{
    double		ratio, points;

    ratio = (double) pl->kills / (pl->deaths + 1);
    points = ratio * Rate(pl->score, average_score);
    Score(pl, points, pl->pos, "[Winner]");
}


static void Count_rounds(void)
{
    char		msg[MSG_LEN];

    if (!roundsToPlay)
	return;

    ++roundsPlayed;

    sprintf(msg, " < Round %d out of %d completed. >",
	    roundsPlayed, roundsToPlay);
    Set_message(msg);
    /* only do the game over once */
    if (roundsPlayed == roundsToPlay)
	Game_Over();
}


void Team_game_over(int winning_team, const char *reason)
{
    int			i, j;
    double		average_score;
    int			num_best_players;
    int			*best_players;
    double		best_ratio;
    char		msg[MSG_LEN];

    if (!(best_players = (int *)malloc(NumPlayers * sizeof(int)))) {
	warn("no mem");
	End_game();
    }

    /* Figure out the average score and who has the best kill/death ratio */
    /* ratio for this round */
    Compute_end_of_round_values(&average_score,
				&num_best_players,
				&best_ratio,
				best_players);

    /* Print out the results of the round */
    if (winning_team != -1) {
	sprintf(msg, " < Team %d has won the round%s! >", winning_team,
		reason);
	sound_play_all(TEAM_WIN_SOUND);
    } else {
	sprintf(msg, " < We have a draw%s! >", reason);
	sound_play_all(TEAM_DRAW_SOUND);
    }
    Set_message(msg);

    /* Give bonus to the best player */
    Give_best_player_bonus(average_score,
			   num_best_players,
			   best_ratio,
			   best_players);

    /* Give bonuses to the winning team */
    if (winning_team != -1) {
	for (i = 0; i < NumPlayers; i++) {
	    player *pl_i = Players(i);

	    if (pl_i->team != winning_team)
		continue;

	    if (IS_TANK_PTR(pl_i)
		|| (BIT(pl_i->status, PAUSE)
		    && pl_i->count <= 0)
		|| (BIT(pl_i->status, GAME_OVER)
		    && pl_i->mychar == 'W'
		    && pl_i->score == 0)) {
		continue;
	    }
	    for (j = 0; j < num_best_players; j++) {
		if (i == best_players[j])
		    break;
	    }
	    if (j == num_best_players)
		Give_individual_bonus(pl_i, average_score);
	}
    }

    teamcup_round_end(winning_team);

    Reset_all_players();

    Count_rounds();

    free(best_players);

    /* Ranking */
    Rank_write_webpage();
    Rank_write_score_file();
    Rank_show_ranks();
}

void Individual_game_over(int winner)
{
    int			i, j;
    double		average_score;
    int			num_best_players;
    int			*best_players;
    double		best_ratio;
    char		msg[MSG_LEN];

    if (!(best_players = (int *)malloc(NumPlayers * sizeof(int)))) {
	warn("no mem");
	End_game();
    }

    /* Figure out what the average score is and who has the best kill/death */
    /* ratio for this round */
    Compute_end_of_round_values(&average_score, &num_best_players,
				&best_ratio, best_players);

    /* Print out the results of the round */
    if (winner == -1) {
	Set_message(" < We have a draw! >");
	sound_play_all(PLAYER_DRAW_SOUND);
    }
    else if (winner == -2) {
	Set_message(" < The robots have won the round! >");
	/* Perhaps this should be a different sound? */
	sound_play_all(PLAYER_WIN_SOUND);
    } else {
	sprintf(msg, " < %s has won the round! >", Players(winner)->name);
	Set_message(msg);
	sound_play_all(PLAYER_WIN_SOUND);
    }

    /* Give bonus to the best player */
    Give_best_player_bonus(average_score,
			   num_best_players,
			   best_ratio,
			   best_players);

    /* Give bonus to the winning player */
    if (winner >= 0) {
	for (i = 0; i < num_best_players; i++) {
	    if (winner == best_players[i])
		break;
	}
	if (i == num_best_players)
	    Give_individual_bonus(Players(winner), average_score);
    }
    else if (winner == -2) {
	for (j = 0; j < NumPlayers; j++) {
	    player *pl_j = Players(j);
	    if (IS_ROBOT_PTR(pl_j)) {
		for (i = 0; i < num_best_players; i++) {
		    if (j == best_players[i])
			break;
		}
		if (i == num_best_players)
		    Give_individual_bonus(pl_j, average_score);
	    }
	}
    }

    Reset_all_players();

    Count_rounds();

    free(best_players);
}

void Race_game_over(void)
{
    player		*pl;
    int			i,
			j,
			k,
			bestlap = 0,
			num_best_players = 0,
			num_active_players = 0,
			num_ordered_players = 0;
    int			*order;
    char		msg[MSG_LEN];

    /*
     * Reassign players's starting positions based upon
     * personal best lap times.
     */
    if ((order = (int *)malloc(NumPlayers * sizeof(int))) != NULL) {
	for (i = 0; i < NumPlayers; i++) {
	    pl = Players(i);
	    if (IS_TANK_PTR(pl))
		continue;
	    if (BIT(pl->status, PAUSE)
		|| (BIT(pl->status, GAME_OVER) && pl->mychar == 'W')
		|| pl->best_lap <= 0)
		j = i;
	    else {
		for (j = 0; j < i; j++) {
		    player *pl_j = Players(order[j]);
		    if (pl->best_lap < pl_j->best_lap)
			break;

		    if (BIT(pl_j->status, PAUSE)
			|| (BIT(pl_j->status, GAME_OVER)
			    && pl_j->mychar == 'W'))
			break;
		}
	    }
	    for (k = i - 1; k >= j; k--)
		order[k + 1] = order[k];
	    order[j] = i;
	    num_ordered_players++;
	}
	for (i = 0; i < num_ordered_players; i++) {
	    pl = Players(order[i]);
	    if (pl->home_base->ind != World.baseorder[i].base_idx) {
		pl->home_base = Bases(World.baseorder[i].base_idx);
		for (j = 0; j < observerStart + NumObservers; j++) {
		    if (j == NumPlayers) {
			if (NumObservers)
			    j = observerStart;
			else
			    break;
		    }
		    if (Players(j)->conn != NULL)
			Send_base(Players(j)->conn, pl->id,pl->home_base->ind);
		}
		if (BIT(pl->status, PAUSE))
		    Go_home(pl);
	    }
	}
	free(order);
    }

    for (i = NumPlayers - 1; i >= 0; i--)  {
	pl = Players(i);
	CLR_BIT(pl->status, RACE_OVER | FINISH);
	if (BIT(pl->status, PAUSE)
	    || (BIT(pl->status, GAME_OVER) && pl->mychar == 'W')
	    || IS_TANK_PTR(pl))
	    continue;
	num_active_players++;

	/* Kill any remaining players */
	if (!BIT(pl->status, GAME_OVER))
	    Kill_player(pl, false);
	else
	    Player_death_reset(pl, false);

	if (pl != Players(i))
	    continue;

	if ((pl->best_lap < bestlap || bestlap == 0) &&
	    pl->best_lap > 0) {
	    bestlap = pl->best_lap;
	    num_best_players = 0;
	}
	if (pl->best_lap == bestlap)
	    num_best_players++;
    }

    /* If someone completed a lap */
    if (bestlap > 0) {
	for (i = 0; i < NumPlayers; i++)  {
	    pl = Players(i);
	    if (BIT(pl->status, PAUSE)
		|| (BIT(pl->status, GAME_OVER) && pl->mychar == 'W')
		|| IS_TANK_PTR(pl))
		continue;

	    if (pl->best_lap == bestlap) {
		sprintf(msg,
			"%s %s the best lap time of %.2fs",
			pl->name,
			(num_best_players == 1) ? "had" : "shares",
			(double) bestlap / FPS);
		Set_message(msg);
		Score(pl, 5.0 + num_active_players, pl->pos,
		      (num_best_players == 1)
		      ? "[Fastest lap]" : "[Joint fastest lap]");
	    }
	}

	updateScores = true;
    }
    else if (num_active_players > NumRobots)
	Set_message("No-one even managed to complete one lap, you should be "
		    "ashamed of yourselves.");

    /* kps - ng swapped these two for some reason*/
    Count_rounds();

    Reset_all_players();
}


void Compute_game_status(void)
{
    int			i;
    player		*pl;
    char		msg[MSG_LEN];

    if (round_delay_send > 0)
	round_delay_send--;

    if (round_delay > 0) {
	if (!--round_delay) {
	    sprintf(msg, "%s starts now.",
		    (BIT(World.rules->mode, TIMING) ? "Race" : "Round"));
	    Set_message(msg);
	    roundtime = maxRoundTime * FPS;
	    /*
	     * Make sure players get the full 60 seconds
	     * of allowed idle time.
	     */
	    for (i = 0; i < NumPlayers; i++)
		Players(i)->frame_last_busy = frame_loops;
	}
    }

    if (roundtime > 0)
	roundtime--;

    if (BIT(World.rules->mode, TIMING)) {
	/*
	 * We need a completely separate scoring system for race mode.
	 * I'm not sure how race mode should interact with team mode,
	 * so for the moment race mode takes priority.
	 *
	 * Race mode and limited lives mode interact. With limited lives on,
	 * race ends after all players have completed the course, or have died.
	 * With limited lives mode off, the race ends when the first player
	 * completes the course - all remaining players are then killed to
	 * reset them.
	 *
	 * In limited lives mode, where the race can be run to completion,
	 * points are awarded not just to the winner but to everyone who
	 * completes the course (with more going to the winner). These
	 * points are awarded as the player crosses the line. At the end
	 * of the race, a bonus is awarded to the player with the fastest lap.
	 *
	 * In unlimited lives mode, just the winner and the holder of the
	 * fastest lap get points.
	 */

	player		*alive = NULL;
	int		num_alive_players = 0,
			num_active_players = 0,
			num_finished_players = 0,
			num_race_over_players = 0,
			num_waiting_players = 0,
			pos = 1,
			total_pts;
	double		pts;

	/*
	 * kps - ng wants to handle laps here, requires change in collision.c
	 * too, maybe I'll fix it later.
	 */
#if 0
	/* Handle finishing of laps */
	for (i = 0; i < NumPlayers; i++) {
	    pl = Players(i);
	    if (!BIT(pl->status, FINISH))
		continue;
	    pl->last_lap_time = pl->time - pl->last_lap;
	    if ((pl->best_lap > pl->last_lap_time || pl->best_lap == 0)
		&& pl->time != 0 && pl->round != 1) {
		pl->best_lap = pl->last_lap_time;
	    }
	    pl->last_lap = pl->time;
	    if (pl->round > raceLaps) {
		Player_death_reset(pl);
		pl->mychar = 'D';
		SET_BIT(pl->status, GAME_OVER);
		sprintf(msg, "%s finished the race. Last lap time: %.2fs. "
			"Personal race best lap time: %.2fs.",
			pl->name,
			(double) pl->last_lap_time / FPS,
			(double) pl->best_lap / FPS);
	    }
	    else if (pl->round > 1)
		sprintf(msg, "%s completes lap %d in %.2fs. "
			"Personal race best lap time: %.2fs.",
			pl->name,
			pl->round-1,
			(double) pl->last_lap_time / FPS,
			(double) pl->best_lap / FPS);
	    else {
		sprintf(msg, "%s starts lap 1 of %d", pl->name,
			raceLaps);
		CLR_BIT(pl->status, FINISH); /* no elimination from starting */
	    }
	    Set_message(msg);
	}
	if (eliminationRace) {
	    for (;;) {
		int pli, count = 0, lap = INT_MAX;
		player *pl_i;
		for (i = 0; i < NumPlayers; i++) {
		    pl = Players(i);
		    if (BIT(pl->status, FINISH) && pl->round < lap) {
			lap = pl->round;
			pli = i;
		    }
		}
		if (lap == INT_MAX)
		    break;
		pl_i = Players(pli);
		CLR_BIT(pl_i->status, FINISH);
		lap = 0;
		for (i = 0; i < NumPlayers; i++) {
		    pl = Players(i);
		    if (!Player_is_active(pl))
			continue;
		    if (pl->round < pl_i->round) {
			count++;
			if (pl->round > lap)
			    lap = pl->round;
		    }
		}
		if (pl_i->round < lap + count)
		    continue;
		for (i = 0; i < NumPlayers; i++) {
		    pl = Players(i);
		    if (!Player_is_active(pl))
			continue;
		    if (pl->round < pl_i->round) {
			Player_death_reset(pl);
			pl->mychar = 'D';
			SET_BIT(pl->status, GAME_OVER);
			if (count == 1) {
			    sprintf(msg, "%s was the last to complete lap "
				    "%d and is out of the race.",
				    pl->name, pl_i->round - 1);
			    Set_message(msg);
			}
			else {
			    sprintf(msg, "%s was the last to complete some "
				    "lap between %d and %d.", pl->name,
				    pl->round, pl_i->round - 1);
			    Set_message(msg);
			}
		    }
		}
	    }
	}
#endif

	/* First count the players */
	for (i = 0; i < NumPlayers; i++)  {
	    pl = Players(i);
	    if (BIT(pl->status, PAUSE)
		|| IS_TANK_PTR(pl)) {
		continue;
	    }
	    if (!BIT(pl->status, GAME_OVER))
		num_alive_players++;
	    else if (pl->mychar == 'W') {
		num_waiting_players++;
		continue;
	    }

	    if (BIT(pl->status, RACE_OVER)) {
		num_race_over_players++;
		pos++;
	    }
	    else if (BIT(pl->status, FINISH)) {
		if (pl->round > raceLaps)
		    num_finished_players++;
		else
		    CLR_BIT(pl->status, FINISH);
	    }
	    else if (!BIT(pl->status, GAME_OVER)) {
		alive = pl;
	    }

	    /*
	     * An active player is one who is:
	     *   still in the race.
	     *   reached the finish line just now.
	     *   has finished the race in a previous frame.
	     *   died too often.
	     */
	    num_active_players++;
	}
	if (num_active_players == 0 && num_waiting_players == 0)
	    return;

	/* Now if any players are unaccounted for */
	if (num_finished_players > 0) {
	    /*
	     * Ok, update positions. Everyone who finished the race in the last
	     * frame gets the current position.
	     */

	    /* Only play the sound for the first person to cross the finish */
	    if (pos == 1)
		sound_play_all(PLAYER_WIN_SOUND);

	    total_pts = 0;
	    for (i = 0; i < num_finished_players; i++)
		total_pts
		    += (10 + 2 * num_active_players) >> (pos - 1 + i);
	    pts = total_pts / num_finished_players;

	    for (i = 0; i < NumPlayers; i++)  {
		pl = Players(i);
		if (BIT(pl->status, PAUSE)
		    || (BIT(pl->status, GAME_OVER) && pl->mychar == 'W')
		    || IS_TANK_PTR(pl))
		    continue;

		if (BIT(pl->status, FINISH)) {
		    CLR_BIT(pl->status, FINISH);
		    SET_BIT(pl->status, RACE_OVER);
		    if (pts > 0) {
			sprintf(msg,
				"%s finishes %sin position %d "
				"scoring %.2f point%s.",
				pl->name,
				(num_finished_players == 1) ? "" : "jointly ",
				pos, pts,
				(pts == 1) ? "" : "s");
			Set_message(msg);
			sprintf(msg, "[Position %d%s]", pos,
				(num_finished_players == 1) ? "" : " (jointly)");
			Score(pl, pts, pl->pos, msg);
		    }
		    else {
			sprintf(msg,
				"%s finishes %sin position %d.",
				pl->name,
				(num_finished_players == 1) ? "" : "jointly ",
				pos);
			Set_message(msg);
		    }
		}
	    }
	}

	/*
	 * If the maximum allowed time for this race is over, end it.
	 */
	if (maxRoundTime > 0 && roundtime == 0) {
	    Set_message("Timer expired. Race ends now.");
	    Race_game_over();
	    return;
	}

	/*
	 * In limited lives mode, wait for everyone to die, except
	 * for the last player.
	 */
	if (BIT(World.rules->mode, LIMITED_LIVES)) {
	    if (num_alive_players > 1)
		return;
	    if (num_alive_players == 1 && num_active_players == 1)
		return;
	}
	/* !@# fix
	 * No meaningful messages / scores if someone wins by staying alive
	 */
	else if (num_finished_players == 0 || num_alive_players > 1)
	    return;

	Race_game_over();

    } else if (BIT(World.rules->mode, TEAM_PLAY)) {

	/* Do we have a winning team ? */

	enum TeamState {
	    TeamEmpty,
	    TeamDead,
	    TeamAlive
	}	team_state[MAX_TEAMS];
	int	num_dead_teams = 0;
	int	num_alive_teams = 0;
	int	winning_team = -1;

	for (i = 0; i < MAX_TEAMS; i++)
	    team_state[i] = TeamEmpty;

	for (i = 0; i < NumPlayers; i++) {
	    player *pl_i = Players(i);

	    if (IS_TANK_PTR(pl_i))
		/* Ignore tanks. */
		continue;
	    else if (BIT(pl_i->status, PAUSE))
		/* Ignore paused players. */
		continue;
#if 0
	    /* not all teammode maps have treasures. */
	    else if (World.teams[pl_i->team].NumTreasures == 0)
		/* Ignore players with no treasure troves */
		continue;
#endif
	    else if (BIT(pl_i->status, GAME_OVER)) {
		if (team_state[pl_i->team] == TeamEmpty) {
		    /* Assume all teammembers are dead. */
		    num_dead_teams++;
		    team_state[pl_i->team] = TeamDead;
		}
	    }
	    /*
	     * If the player is not paused and he is not in the
	     * game over mode and his team owns treasures then he is
	     * considered alive.
	     * But he may not be playing though if the rest of the team
	     * was genocided very quickly after game reset, while this
	     * player was still being transported back to his homebase.
	     */
	    else if (team_state[pl_i->team] != TeamAlive) {
		if (team_state[pl_i->team] == TeamDead)
		    /* Oops!  Not all teammembers are dead yet. */
		    num_dead_teams--;
		team_state[pl_i->team] = TeamAlive;
		++num_alive_teams;
		/* Remember a team which was alive. */
		winning_team = pl_i->team;
	    }
	}

	if (num_alive_teams > 1) {
	    char	*bp;
	    int		teams_with_treasure = 0;
	    int		team_win[MAX_TEAMS];
	    double	team_score[MAX_TEAMS];
	    int		winners;
	    int		max_destroyed = 0;
	    int		max_left = 0;
	    double	max_score = 0;
	    team_t	*team_ptr;

	    /*
	     * Game is not over if more than one team which have treasures
	     * still have one remaining in play.  Note that it is possible
	     * for max_destroyed to be zero, in the case where a team
	     * destroys some treasures and then all quit, and the remaining
	     * teams did not destroy any.
	     */
	    for (i = 0; i < MAX_TEAMS; i++) {
		team_score[i] = 0;
		if (team_state[i] != TeamAlive) {
		    team_win[i] = 0;
		    continue;
		}
		team_win[i] = 1;
		team_ptr = &(World.teams[i]);
		if (team_ptr->TreasuresDestroyed > max_destroyed)
		    max_destroyed = team_ptr->TreasuresDestroyed;
		if ((team_ptr->TreasuresLeft > 0) ||
		    (team_ptr->NumTreasures == team_ptr->NumEmptyTreasures))
		    teams_with_treasure++;
	    }

	    /*
	     * Game is not over if more than one team has treasure.
	     */
	    if ((teams_with_treasure > 1 || !max_destroyed)
		&& (roundtime != 0 || maxRoundTime <= 0))
		return;

	    if (maxRoundTime > 0 && roundtime == 0)
		Set_message("Timer expired. Round ends now.");

	    /*
	     * Find the winning team;
	     *	Team destroying most number of treasures;
	     *	If drawn; the one with most saved treasures,
	     *	If drawn; the team with the most points,
	     *	If drawn; an overall draw.
	     */
	    for (winners = i = 0; i < MAX_TEAMS; i++) {
		if (!team_win[i])
		    continue;
		if (World.teams[i].TreasuresDestroyed == max_destroyed) {
		    if (World.teams[i].TreasuresLeft > max_left)
			max_left = World.teams[i].TreasuresLeft;
		    winning_team = i;
		    winners++;
		} else
		    team_win[i] = 0;
	    }
	    if (winners == 1) {
		sprintf(msg, " by destroying %d treasures", max_destroyed);
		Team_game_over(winning_team, msg);
		return;
	    }

	    for (i = 0; i < NumPlayers; i++) {
		player *pl_i = Players(i);
		if (BIT(pl_i->status, PAUSE) || IS_TANK_PTR(pl_i))
		    continue;
		team_score[pl_i->team] += pl_i->score;
	    }

	    for (winners = i = 0; i < MAX_TEAMS; i++) {
		if (!team_win[i])
		    continue;
		if (World.teams[i].TreasuresLeft == max_left) {
		    if (team_score[i] > max_score)
			max_score = team_score[i];
		    winning_team = i;
		    winners++;
		} else
		    team_win[i] = 0;
	    }
	    if (winners == 1) {
		sprintf(msg,
			" by destroying %d treasures"
			" and successfully defending %d",
			max_destroyed, max_left);
		Team_game_over(winning_team, msg);
		return;
	    }

	    for (winners = i = 0; i < MAX_TEAMS; i++) {
		if (!team_win[i])
		    continue;
		if (team_score[i] == max_score) {
		    winning_team = i;
		    winners++;
		} else
		    team_win[i] = 0;
	    }
	    if (winners == 1) {
		sprintf(msg, " by destroying %d treasures, saving %d, and "
			"scoring %.2f points",
			max_destroyed, max_left, max_score);
		Team_game_over(winning_team, msg);
		return;
	    }

	    /* Highly unlikely */

	    sprintf(msg, " between teams ");
	    bp = msg + strlen(msg);
	    for (i = 0; i < MAX_TEAMS; i++) {
		if (!team_win[i])
		    continue;
		*bp++ = "0123456789"[i]; *bp++ = ','; *bp++ = ' ';
	    }
	    bp -= 2;
	    *bp = '\0';
	    Team_game_over(-1, msg);

	}
	else if (num_dead_teams > 0) {
	    if (num_alive_teams == 1)
		Team_game_over(winning_team, " by staying alive");
	    else
		Team_game_over(-1, " as everyone died");
	}
	else {
	    /*
	     * num_alive_teams <= 1 && num_dead_teams == 0
	     *
	     * There is a possibility that the game has ended because players
	     * quit, the game over state is needed to reset treasures.  We
	     * must count how many treasures are missing, if there are any
	     * the playing team (if any) wins.
	     */
	    int	j, treasures_destroyed;

	    for (treasures_destroyed = j = 0; j < MAX_TEAMS; j++)
		treasures_destroyed += (World.teams[j].NumTreasures
					- World.teams[j].NumEmptyTreasures
					- World.teams[j].TreasuresLeft);
	    if (treasures_destroyed)
		Team_game_over(winning_team, " by staying in the game");
	}

    } else {

	/* Do we have a winner ? (No team play) */
	int num_alive_players = 0;
	int num_active_players = 0;
	int num_alive_robots = 0;
	int num_active_humans = 0;
	int winner = -1;

	for (i = 0; i < NumPlayers; i++)  {
	    player *pl_i = Players(i);
	    if (BIT(pl_i->status, PAUSE) || IS_TANK_PTR(pl_i)) 
		continue;
	    if (!BIT(pl_i->status, GAME_OVER)) {
		num_alive_players++;
		if (IS_ROBOT_PTR(pl_i))
		    num_alive_robots++;
		winner = i; 	/* Tag player that's alive */
	    }
	    else if (IS_HUMAN_PTR(pl_i))
		num_active_humans++;
	    num_active_players++;
	}

	if (num_alive_players == 1 && num_active_players > 1)
	    Individual_game_over(winner);
	else if (num_alive_players == 0 && num_active_players >= 1)
	    Individual_game_over(-1);
	else if (num_alive_robots > 1
		    && num_alive_players == num_alive_robots
		    && num_active_humans > 0)
	    Individual_game_over(-2);
	else if (maxRoundTime > 0 && roundtime == 0) {
	    Set_message("Timer expired. Round ends now.");
	    Individual_game_over(-1);
	}
    }
}

void Delete_player(player *pl)
{
    int			ind = GetInd(pl->id);
    object		*obj;
    int			i, j,
			id = pl->id;

    /* call before important player structures are destroyed */
    Leave_alliance(pl);

    if (IS_ROBOT_PTR(pl))
	Robot_destroy(pl);
    if (pl->isoperator) {
	if (!--NumOperators && game_lock) {
	    game_lock = false;
	    Set_message(" < The game has been unlocked as "
			"the last operator left! >");
	}
    }

    /* Won't be swapping anywhere */
    for (i = MAX_TEAMS - 1; i >= 0; i--)
	if (World.teams[i].SwapperId == id)
	    World.teams[i].SwapperId = -1;
#if 0 /* kps - why "if 0" ? */
    if (pl->team != TEAM_NOT_SET)
	/* Swapping a queued player might be better */
	World.teams[pl->team].SwapperId = -1;
#endif

    /* Delete remaining shots */
    for (i = NumObjs - 1; i >= 0; i--) {
	obj = Obj[i];
	if (obj->id == id) {
	    if (obj->type == OBJ_BALL) {
		Delete_shot(i);
		BALL_PTR(obj)->owner = NO_ID;
	    }
	    else if (BIT(obj->type, OBJ_DEBRIS | OBJ_SPARK))
		/* Okay, so you want robot explosions to exist,
		 * even if the robot left the game. */
		obj->id = NO_ID;
	    else {
		if (!keepShots) {
		    obj->life = 0;
		    if (BIT(obj->type,
			    OBJ_CANNON_SHOT|OBJ_MINE|OBJ_SMART_SHOT
			    |OBJ_HEAT_SHOT|OBJ_TORPEDO))
			obj->mass = 0;
		}
	        obj->id = NO_ID;
		if (BIT(obj->type, OBJ_MINE))
		    MINE_PTR(obj)->owner = NO_ID;
	    }
	}
	else {
	    if (BIT(obj->type, OBJ_MINE)) {
		mineobject *mine = MINE_PTR(obj);
		if (mine->owner == id) {
		    mine->owner = NO_ID;
		    if (!keepShots) {
			obj->life = 0;
			obj->mass = 0;
		    }
		}
	    }
	    else if (BIT(obj->type, OBJ_CANNON_SHOT)) {
		if (!keepShots) {
		    obj->life = 0;
		    obj->mass = 0;
		}
	    }
	    else if (BIT(obj->type, OBJ_BALL)) {
		ballobject *ball = BALL_PTR(obj);
		if (ball->owner == id)
		    ball->owner = NO_ID;
	    }
	}
    }

    Free_ship_shape(pl->ship);

    sound_close(pl);

    NumPlayers--;
    if (IS_TANK_PTR(pl))
	NumPseudoPlayers--;

    /* Ranking. */
    if (IS_HUMAN_PTR(pl)) {
	Rank_save_score(pl);
   	if (NumPlayers == NumRobots + NumPseudoPlayers) {
	    Rank_write_webpage();
	    Rank_write_score_file();
   	}
    }

    if (pl->team != TEAM_NOT_SET && !IS_TANK_PTR(pl) && pl->home_base) {
	World.teams[pl->team].NumMembers--;
	TEAM_SCORE(pl->team, -(pl->score));	/* recalculate teamscores */
	if (IS_ROBOT_PTR(pl))
	    World.teams[pl->team].NumRobots--;
    }

    if (IS_ROBOT_PTR(pl))
	NumRobots--;

    /*
     * Swap entry no 'ind' with the last one.
     *
     * Change the PlayersArray[] pointer array to have Players(ind) point to
     * a valid player and move our leaving player to PlayersArray[NumPlayers].
     */
    pl				= Players(NumPlayers);	/* Swap pointers... */
    PlayersArray[NumPlayers]	= Players(ind);
    PlayersArray[ind]		= pl;
    pl				= Players(NumPlayers);	/* Restore pointer. */

    GetIndArray[Players(ind)->id] = ind;
    GetIndArray[Players(NumPlayers)->id] = NumPlayers;

    Check_team_members(pl->team);

    for (i = NumPlayers - 1; i >= 0; i--) {
	player *pl_i = Players(i);
	if (IS_TANK_PTR(pl_i)
	    && pl_i->lock.pl_id == id) {
	    /* remove tanks which were released by this player. */
	    if (keepShots)
		pl_i->lock.pl_id = NO_ID;
	    else
		Delete_player(pl_i);
	    continue;
	}
	if (BIT(pl_i->lock.tagged, LOCK_PLAYER|LOCK_VISIBLE)
	    && (pl_i->lock.pl_id == id || NumPlayers <= 1)) {
	    CLR_BIT(pl_i->lock.tagged, LOCK_PLAYER|LOCK_VISIBLE);
	    CLR_BIT(pl_i->used, HAS_TRACTOR_BEAM);
	}
	if (IS_ROBOT_PTR(pl_i)
	    && Robot_war_on_player(pl_i) == id)
	    Robot_reset_war(pl_i);

	for (j = 0; j < LOCKBANK_MAX; j++) {
	    if (pl_i->lockbank[j] == id)
		pl_i->lockbank[j] = -1;
	}
	for (j = 0; j < MAX_RECORDED_SHOVES; j++) {
	    if (pl_i->shove_record[j].pusher_id == id)
		pl_i->shove_record[j].pusher_id = NO_ID;
	}
    }

    for (i = NumPlayers - 1; i >= 0; i--) {
	player *pl_i = Players(i);
	if (pl_i->conn != NULL)
	    Send_leave(pl_i->conn, id);
	else if (IS_TANK_PTR(pl_i)) {
	    if (pl_i->lock.pl_id == id)
		Delete_player(pl_i);
	}
    }

    for (i = NumObservers - 1; i >= 0; i--)
	Send_leave(Players(i + observerStart)->conn, id);

    release_ID(id);
}

void Detach_ball(player *pl, ballobject *ball)
{
    int			i, cnt;

    if (ball == NULL || ball == pl->ball) {
	pl->ball = NULL;
	CLR_BIT(pl->used, HAS_CONNECTOR);
    }

    if (BIT(pl->have, HAS_BALL)) {
	for (cnt = i = 0; i < NumObjs; i++) {
	    object *obj = Obj[i];

	    if (obj->type == OBJ_BALL && obj->id == pl->id) {
		if (ball == NULL || ball == BALL_PTR(obj))
		    obj->id = NO_ID;
		    /* Don't reset owner so you can throw balls */
		else
		    cnt++;
	    }
	}
	if (cnt == 0)
	    CLR_BIT(pl->have, HAS_BALL);
	else
	    sound_play_sensors(pl->pos, DROP_BALL_SOUND);
    }
}

void Kill_player(player *pl, bool add_rank_death)
{
    /* Don't create an explosion if the player is being transported back
     * to home base after being killed. */
    if (BIT(pl->status, PLAYING))
	Explode_fighter(pl);
    Player_death_reset(pl, add_rank_death);
}

void Player_death_reset(player *pl, bool add_rank_death)
{
    double		minfuel;
    int			i;


    if (IS_TANK_PTR(pl)) {
	Delete_player(pl);
	return;
    }

    Detach_ball(pl, NULL);
    if (BIT(pl->used, HAS_AUTOPILOT) || BIT(pl->status, HOVERPAUSE)) {
	CLR_BIT(pl->status, HOVERPAUSE);
	Autopilot(pl, false);
    }

    pl->vel.x		= pl->vel.y	= 0.0;
    pl->acc.x		= pl->acc.y	= 0.0;
    pl->emptymass	= pl->mass	= ShipMass;
    pl->status		|= DEF_BITS;
    pl->status		&= ~(KILL_BITS);

    for (i = 0; i < NUM_ITEMS; i++) {
	if (!BIT(1U << i, ITEM_BIT_FUEL | ITEM_BIT_TANK))
	    pl->item[i] = World.items[i].initial;
    }

    pl->forceVisible	= 0;
    if (BIT(pl->status, PAUSE))
	pl->count = MAX(RECOVERY_DELAY, pl->count);
    else
	pl->count = RECOVERY_DELAY;
    pl->ecmcount	= 0;
    pl->emergency_thrust_left = 0;
    pl->emergency_shield_left = 0;
    pl->phasing_left	= 0;
    pl->damaged 	= 0;
    pl->stunned		= 0;
    pl->lock.distance	= 0;

    pl->fuel.sum       	= pl->fuel.sum * 0.90;	/* Loose 10% of fuel */
    minfuel		= World.items[ITEM_FUEL].initial;
    minfuel		+= rfrac() * (1.0 + minfuel) * 0.2;
    pl->fuel.sum	= MAX(pl->fuel.sum, minfuel);
    Player_init_fuel(pl, pl->fuel.sum);

    /*-BA Handle the combination of limited life games and
     *-BA robotLeaveLife by making a robot leave iff it gets
     *-BA eliminated in any round.  Means that robotLeaveLife
     *-BA is ignored, but that robotsLeave is still respected.
     *-KK Added check on race mode. Since in race mode everyone
     *-KK gets killed at the end of the round, all robots would
     *-KK be replaced in the next round. I don't think that's
     *-KK the Right Thing to do.
     *-KK Also, only check a robot's score at the end of the round.
     *-KK 27-2-98 Check on team mode too. It's very confusing to
     *-KK have different robots in your team every round.
     */

    if (!BIT(pl->status, PAUSE)) {

	if (add_rank_death)
	    Rank_AddDeath(pl);

	if (BIT(World.rules->mode, LIMITED_LIVES)) {
	    pl->life--;
	    if (pl->life == -1) {
		if (IS_ROBOT_PTR(pl)) {
		    if (!BIT(World.rules->mode, TIMING|TEAM_PLAY)
			|| (robotsLeave && pl->score < robotLeaveScore)) {
			Robot_delete(pl, false);
			return;
		    }
		}
		pl->life = 0;
		SET_BIT(pl->status, GAME_OVER);
		if (pl->mychar != 'W')
		    pl->mychar = 'D';
		Player_lock_closest(pl, false);
	    }
	}
	else
	    pl->life++;
    }

    pl->have	= DEF_HAVE;
    pl->used	|= DEF_USED;
    pl->used	&= ~(USED_KILL);
    pl->used	&= pl->have;
}

/* determines if two players are immune to eachother */
int Team_immune(int id1, int id2)
{
    player 	*pl1, *pl2;

    if (id1 == id2)
	/* owned stuff is never team immune */
	return 0;
    if (!teamImmunity)
	return 0;
    if (id1 == NO_ID || id2 == NO_ID)
	/* can't find owner for cannon stuff */
	return 0;

    pl1 = Player_by_id(id1);
    pl2 = Player_by_id(id2);

    if (TEAM(pl1, pl2))
	/* players are teammates */
	return 1;

    if (ALLIANCE(pl1, pl2))
	/* players are allies */
	return 1;

    return 0;
}

/* $Id$
 *
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-98 by
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

#ifndef	PROTO_H
#define	PROTO_H

#ifdef VMS
#include "strcasecmp.h"
#endif

/*
 * Prototypes for collision.c
 */
void Free_cells(void);
void Alloc_cells(void);
void Check_collision(void);
int wormXY(int x, int y);
void SCORE(int, int, int, int, const char *);

/*
 * Prototypes for id.c
 */
int peek_ID(void);
int request_ID(void);
void release_ID(int id);

/*
 * Prototypes for walls.c
 */
void Walls_init(void);
void Move_init(void);
void Move_object(int ind);
void Move_player(int ind);
void Turn_player(int ind);
int is_inside(int x, int y, int hit_mask);

/*
 * Prototypes for event.c
 */
int Handle_keyboard(int);
void Pause_player(int ind, int onoff);
int Player_lock_closest(int ind, int next);
bool team_dead(int team);

/*
 * Prototypes for map.c
 */
void Init_map(void);
void Free_map(void);
void Alloc_map(void);
void Grok_map(void);
void Compute_gravity(void);
DFLOAT Wrap_findDir(DFLOAT dx, DFLOAT dy);
DFLOAT Wrap_length(int dx, int dy);
DFLOAT Wrap_cfindDir(int dx, int dy);

/*
 * Prototypes for math.c
 */
int ON(char *optval);
int OFF(char *optval);
int mod(int x, int y);
int f2i(DFLOAT f);
DFLOAT findDir(DFLOAT x, DFLOAT y);
DFLOAT rfrac(void);
void Make_table(void);


/*
 * Prototypes for cmdline.c
 */
int Parse_list(int *index, char *buf);
int Parser(int argc, char **argv);
int Tune_option(char *opt, char *val);

/*
 * Prototypes for play.c
 */
void Thrust(int ind);
#ifdef TURN_FUEL
void Turn_thrust(int ind,int num_sparks);
#endif
void Recoil(object *ship, object *shot);
void Record_shove(player *pl, player *pusher, long time);
void Delta_mv(object *ship, object *obj);
void Obj_repel(object *obj1, object *obj2, int repel_dist);
void Item_damage(int ind, DFLOAT prob);
void Alloc_shots(int number);
void Free_shots(void);
void Tank_handle_detach(player*);
void Add_fuel(pl_fuel_t*, long);
void Update_tanks(pl_fuel_t *);
void Place_item(int type, int ind);
void Tractor_beam(int ind);
void General_tractor_beam(int ind, int x, int y,
			  int items, int target, bool pressor);
void Place_mine(int ind);
void Place_moving_mine(int ind);
void Place_general_mine(int ind, u_short team, long status, int x, int y,
  			DFLOAT vx, DFLOAT vy, modifiers mods);
void Detonate_mines(int ind);
char *Describe_shot(int type, long status, modifiers mods, int hit);
void Fire_ecm(int ind);
void Fire_general_ecm(int ind, u_short team, int x, int y);
void Connector_force(int ind);
void Fire_shot(int ind, int type, int dir);
void Fire_general_shot(int ind, u_short team, bool cannon, int x, int y,
		       int type, int dir, DFLOAT speed, modifiers mods,
		       int target);
void Fire_normal_shots(int ind);
void Fire_main_shot(int ind, int type, int dir);
void Fire_shot(int ind, int type, int dir);
void Fire_left_shot(int ind, int type, int dir, int gun);
void Fire_right_shot(int ind, int type, int dir, int gun);
void Fire_left_rshot(int ind, int type, int dir, int gun);
void Fire_right_rshot(int ind, int type, int dir, int gun);
void Make_treasure_ball(int treasure);
int Punish_team(int ind, int t_destroyed, int posx, int posy);
void Delete_shot(int ind);
void Fire_laser(int ind);
void Fire_general_laser(int ind, u_short team, int x, int y, int dir,
			modifiers mods);
void Do_deflector(int ind);
void Do_transporter(int ind);
void Do_general_transporter(int ind, int x, int y, int target,
			    int *item, long *amount);
void do_hyperjump(player *pl);
void do_lose_item(int ind);
void Move_smart_shot(int ind);
void Move_mine(int ind);
void Make_debris(
	    /* pos.x, pos.y   */ int  x, int y,
	    /* vel.x, vel.y   */ DFLOAT  velx,       DFLOAT vely,
	    /* owner id       */ int    id,
	    /* owner team     */ u_short team,
	    /* type           */ int    type,
	    /* mass           */ DFLOAT  mass,
	    /* status         */ long   status,
	    /* color          */ int    color,
	    /* radius         */ int    radius,
	    /* min,max debris */ int    min_debris, int    max_debris,
	    /* min,max dir    */ int    min_dir,    int    max_dir,
	    /* min,max speed  */ DFLOAT  min_speed,  DFLOAT  max_speed,
	    /* min,max life   */ int    min_life,   int    max_life
	    );
void Make_wreckage(
	    /* pos.x, pos.y   */ int x,                    int y,
	    /* vel.x, vel.y   */ DFLOAT velx,       DFLOAT vely,
	    /* owner id       */ int    id,
	    /* owner team     */ u_short team,
	    /* min,max mass   */ DFLOAT min_mass,   DFLOAT max_mass,
	    /* total mass     */ DFLOAT total_mass,
	    /* status         */ long   status,
	    /* color          */ int    color,
	    /* max wreckage   */ int    max_wreckage,
	    /* min,max dir    */ int    min_dir,    int    max_dir,
	    /* min,max speed  */ DFLOAT min_speed,  DFLOAT max_speed,
	    /* min,max life   */ int    min_life,   int    max_life
	    );
void Explode(int ind);
void Explode_fighter(int ind);
void Throw_items(int ind);
void Detonate_items(int ind);
void add_temp_wormholes(int xin, int yin, int xout, int yout, int ind);
void remove_temp_wormhole(int ind);

/*
 * Prototypes for cannon.c
 */
void Cannon_init(int ind);
void Cannon_add_item(int ind, int item, int amount);
void Cannon_throw_items(int ind);
void Cannon_check_fire(int ind);
int Cannon_select_weapon(int ind);
void Cannon_aim(int ind, int weapon, int *target, int *dir);
void Cannon_fire(int ind, int weapon, int target, int dir);

/*
 * Prototypes for player.c
 */
void Pick_startpos(int ind);
void Go_home(int ind);
void Compute_sensor_range(player *);
void Player_add_tank(int ind, long tank_fuel);
void Player_remove_tank(int ind, int which_tank);
void Player_hit_armor(int ind);
int Init_player(int ind, wireobj *ship);
void Alloc_players(int number);
void Free_players(void);
void Update_score_table(void);
void Check_team_members(int);
void Check_team_treasures(int);
void Compute_game_status(void);
void Delete_player(int ind);
void Detach_ball(int ind, int ball);
void Kill_player(int ind);
void Player_death_reset(int ind);
void Team_game_over(int winning_team, const char *reason);
void Individual_game_over(int winner);
void Race_game_over(void);

/*
 * Prototypes for rules.c
 */
void Tune_item_probs(void);
void Tune_item_packs(void);
void Set_initial_resources(void);
void Set_world_items(void);
void Set_world_rules(void);
void Set_misc_item_limits(void);

/*
 * Prototypes for server.c
 */
void End_game(void);
int Pick_team(void);
void Server_info(char *str, unsigned max_size);
void Log_game(const char *heading);
void Game_Over(void);
int plock_server(int onoff);
void tuner_plock(void);
void Main_loop(void);


/*
 * Prototypes for contact.c
 */
void Contact_cleanup(void);
void Contact_init(void);
void Contact(int fd, void *arg);
void Queue_loop(void);
void Set_deny_hosts(void);

/*
 * Prototypes for metaserver.c
 */
void Meta_send(char *mesg, int len);
int Meta_from(char *addr, int port);
void Meta_gone(void);
void Meta_init(void);
void Meta_update(int change);

/*
 * Prototypes for frame.c
 */
void Frame_update(void);
void Set_message(const char *message);
void Set_player_message(player *pl, const char *message);

/*
 * Prototypes for update.c
 */
void Update_radar_target(int);
void Update_objects(void);
void Emergency_thrust(int ind, int on);
void Emergency_shield(int ind, int on);
void Autopilot(int ind, int on);
void Phasing(int ind, int on);

/*
 * Prototypes for option.c
 */
void addOption(const char *name, const char *value, int override, void *def);
char *getOption(const char *name);
bool parseDefaultsFile(const char *filename);
bool parseMapFile(const char *filename);
void parseOptions(void);

/*
 * Prototypes for srecord.c
 */
void Init_recording(void);
void Handle_recording_buffers(void);

#endif

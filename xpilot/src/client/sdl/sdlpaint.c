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

#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"
#include "xpclient.h"
#include "sdlpaint.h"
#include "images.h"

char sdlpaint_version[] = VERSION;

/*
 * Globals.
 */
static double   time_counter = 0.0;

double	        hudRadarLimit;		/* Limit for hudradar drawing */
double          scale;                  /* The opengl scale factor */

/* function to reset our viewport after a window resize */
int Resize_Window( int width, int height )
{

    draw_width = width;
    draw_height = height;

    SDL_SetVideoMode( width,
		      height,
		      draw_depth, 
		      SDL_HWSURFACE | SDL_OPENGL | SDL_RESIZABLE ); 
    

    //    /* change to the projection matrix and set our viewing volume. */
    glMatrixMode( GL_PROJECTION );

    glLoadIdentity( );

    gluOrtho2D(0, draw_width, 0, draw_height);
    
    /* Make sure we're chaning the model view and not the projection */
    glMatrixMode( GL_MODELVIEW );
    
    /* Reset The View */
    glLoadIdentity( );

    /* Setup our viewport. */
    glViewport( 0, 0, ( GLint )draw_width, ( GLint )draw_height );

    return 0;
}

int Paint_init(void)
{
    if (Init_wreckage() == -1)
	return -1;
    
    if (Init_asteroids() == -1)
	return -1;

    if (Images_init() == -1) 
	return -1;

    scale = 0.7;
    scaleFactor = 1.0 / scale;

    return 0;
}

void Paint_cleanup(void)
{
    Images_cleanup();
}

/* kps - can we rather use Check_view_dimensions in paint.c ? */
#if 0
int Check_view_dimensions(void)
{
    int			width_wanted, height_wanted;
    int			srv_width, srv_height;

    /* TODO: once the scaling is implemented these should be computed
     * somehow. For the moment they equal to draw_*
    width_wanted = (int)(draw_width * scaleFactor + 0.5);
    height_wanted = (int)(draw_height * scaleFactor + 0.5);
    */
    width_wanted = draw_width / scale;
    height_wanted = draw_height / scale;

    srv_width = width_wanted;
    srv_height = height_wanted;
    LIMIT(srv_height, MIN_VIEW_SIZE, MAX_VIEW_SIZE);
    LIMIT(srv_width, MIN_VIEW_SIZE, MAX_VIEW_SIZE);
    if (server_display.view_width != srv_width ||
	server_display.view_height != srv_height) {
	if (Send_display(srv_width, 
			 srv_height, 
			 spark_rand, 
			 num_spark_colors))
	    return -1;
    }
    active_view_width = server_display.view_width;
    active_view_height = server_display.view_height;
    ext_view_x_offset = 0;
    ext_view_y_offset = 0;
    if (width_wanted > active_view_width) {
	ext_view_width = width_wanted;
	ext_view_x_offset = (width_wanted - active_view_width) / 2;
    } else {
	ext_view_width = active_view_width;
    }
    if (height_wanted > active_view_height) {
	ext_view_height = height_wanted;
	ext_view_y_offset = (height_wanted - active_view_height) / 2;
    } else {
	ext_view_height = active_view_height;
    }
    return 0;
}
#endif

void Paint_frame(void)
{
    Check_view_dimensions();

    world.x = FOOpos.x - (ext_view_width / 2);
    world.y = FOOpos.y - (ext_view_height / 2);
    realWorld = world;
    if (BIT(Setup->mode, WRAP_PLAY)) {
	if (world.x < 0 && world.x + ext_view_width < Setup->width)
	    world.x += Setup->width;
	else if (world.x > 0 && world.x + ext_view_width >= Setup->width)
	    realWorld.x -= Setup->width;
	if (world.y < 0 && world.y + ext_view_height < Setup->height)
	    world.y += Setup->height;
	else if (world.y > 0 && world.y + ext_view_height >= Setup->height)
	    realWorld.y -= Setup->height;
    }

    if (start_loops != end_loops)
	warn("Start neq. End (%ld,%ld,%ld)", start_loops, end_loops, loops);
    loops = end_loops;


    /*
     * If time() changed from previous value, assume one second has passed.
     */
    if (newSecond) {
	/* kps - improve */
	timePerFrame = 1.0 / clientFPS;

	/* TODO: move this somewhere else */
	/* check once per second if we are playing */
	if (newSecond && self && !strchr("PW", self->mychar))
	    played_this_round = true;
    }

    /*
     * Instead of using loops to determining if things are drawn this frame,
     * loopsSlow should be used. We don't want things to be drawn too fast
     * at high fps.
     */
    time_counter += timePerFrame;
    if (time_counter >= 1.0 / 12) {
	loopsSlow++;
	time_counter -= (1.0 / 12);
    }
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef((int)(-world.x * scale), (int)(-world.y * scale), 0);
    glScalef(scale, scale, scale);

    glClear(GL_COLOR_BUFFER_BIT);

    if (damaged <= 0) {
    	glMatrixMode(GL_MODELVIEW);
    	glLoadIdentity();
    	glTranslatef((int)(-world.x * scale), (int)(-world.y * scale), 0);
    	glScalef(scale, scale, scale);

    	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
    	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
    	Paint_world();

	if (oldServer) {
	    Paint_vfuel();
	    Paint_vdecor();
	    Paint_vcannon();
	    Paint_vbase();
	} else
	    Paint_objects();

	Paint_shots();
	Paint_ships();

    	glDisable(GL_BLEND);
	
	glMatrixMode(GL_MODELVIEW);/*do we need this?*/
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, draw_width, draw_height, 0);

    	Paint_meters();
    	Paint_HUD();
    	Paint_client_fps();

    	Paint_messages();       
    	Paint_score_objects();
	Radar_paint();

	glPopMatrix();

    }

    SDL_GL_SwapBuffers();
}

void Paint_score_start(void)
{
}

void Paint_score_entry(int entry_num, other_t *other, bool is_team)
{
    printf("%c %.1f %s %d\n", 
	   other->mychar, other->score, other->name, other->life);
}


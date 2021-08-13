/*
 * XPilotNG/SDL, an SDL/OpenGL XPilot client. Copyright (C) 2003-2004 by 
 *
 *      Juha Lindström       <juhal@users.sourceforge.net>
 *      Erik Andersson       <deity_at_home.se>
 *      Darel Cullen         <darelcullen@users.sourceforge.net>
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

#include "xpclient_sdl.h"

#include "text.h"
#include "console.h"
#include "sdlkeys.h"
#include "glwidgets.h"
#include "sdlpaint.h"
#include "sdlinit.h"
#include "scrap.h"

/* These are only needed for the polygon tessellation */
/* I'd like to move them to Paint_init/cleanup but because it */
/* is called before the map is ready I need separate functions */
/* for now.. */
extern int Gui_init(void);
extern void Gui_cleanup(void);

SDL_WindowFlags windowFlags = 0;
SDL_Window  *mainWindow = NULL;
SDL_GLContext glContext = NULL;

font_data gamefont;
font_data mapfont;
int gameFontSize;
int mapFontSize;
char *gamefontname;

/* ugly kps hack */
static bool file_exists(const char *path) 
{ 
  FILE *fp;

  if (!path) {
    return false; 
  } else {
    fp = fopen(path ? path : "", "r");
    if (fp) { 
      fclose(fp); 
      return true;
    }
    return false; 
  }
}

int Init_playing_windows(void)
{
    /*
    sdl_init_colors();
    Init_spark_colors();
    */
    if ( !AppendGLWidgetList(&MainWidget,Init_MainWidget(&gamefont)) ) {
	error("widget initialization failed");
	return -1;
    }
    if (Console_init()) {
	error("console initialization failed");
	return -1;
    }
    if (Gui_init()) {
	error("gui initialization failed");
	return -1;
    }

    return 0;
}

static bool find_size(int *w, int *h)
{
    SDL_DisplayMode mode;
    int i, d, best_i, best_d;

    int displayIndex = 0;
    int count = SDL_GetNumDisplayModes(displayIndex);
    
    best_i = 0;
    best_d = INT_MAX;
    for (i = 0; i < count; i++) {
	if (SDL_GetDisplayMode(displayIndex, i, &mode) != 0) {
	    error("SDL_GetDisplayMode failed: %s", SDL_GetError());
	    return false;
	}
	d = (mode.w - *w) * (mode.w - *w) + (mode.h - *h) * (mode.h - *h);
	if (d < best_d) {
	    best_d = d;
	    best_i = i;
	}
    }
    
    SDL_GetDisplayMode(displayIndex, i, &mode);
    *w = mode.w;
    *h = mode.h;
    return true;
}

int Init_window(void)
{
    int value;
    char defaultfontname[] = CONF_FONTDIR "FreeSansBoldOblique.ttf";
    bool gf_exists = true,df_exists = true,gf_init = false, mf_init = false;
    
    if (TTF_Init()) {
    	error("SDL_ttf initialization failed: %s", SDL_GetError());
    	return -1;
    }
    warn("SDL_ttf initialized.\n");

    Conf_print();

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        error("failed to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    atexit(SDL_Quit);

    num_spark_colors=8;

    /* the flags to pass to SDL_SetVideoMode */
    windowFlags  = SDL_WINDOW_OPENGL;          /* Enable OpenGL in SDL          */
#ifndef _WINDOWS
    windowFlags |= SDL_WINDOW_RESIZABLE;       /* Enable window resizing        */
#else
    windowFlags |= SDL_WINDOW_FULLSCREEN;
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

/* TODO
    if (windowFlags & SDL_WINDOW_FULLSCREEN)
      if (!find_size((int*)&draw_width, (int*)&draw_height))
      	windowFlags ^= SDL_WINDOW_FULLSCREEN;
*/

    if ((mainWindow = SDL_CreateWindow(TITLE,
    		 SDL_WINDOWPOS_UNDEFINED,
			 SDL_WINDOWPOS_UNDEFINED,
    		 draw_width,
			 draw_height,
 			 windowFlags )) == NULL) {
      error("Could not find a valid GLX visual for your display");
	  return -1;
    }
    
    glContext = SDL_GL_CreateContext(mainWindow);
    if (!glContext) {
        error("Could not create OpenGL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(mainWindow);
        return -1;
    }


    SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value);
    printf("RGB bpp %d/", value);
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE,&value);
    printf("%d/", value);
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &value);
    printf("%d ", value);
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &value);
    printf("Bit Depth is %d\n",value);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, draw_width, draw_height);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, draw_width, 0, draw_height);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* this prevents a freetype crash if you pass non existant fonts */
    if (!file_exists(gamefontname)) {
    	error("cannot find your game font '%s'.\n" \
            "Please check that it exists!",gamefontname);
    	xpprintf("Reverting to defaultfont '%s'\n",defaultfontname);
    	gf_exists = false;
    }
    if (!file_exists(defaultfontname)) {
    	error("cannot find the default font! '%s'" ,defaultfontname);
	df_exists = false;
    }
    
    if (!gf_exists && !df_exists) {
    	error("Failed to find any font files!\n" \
	    	"Probably you forgot to run 'make install',use '-TTFont <font.ttf>' argument" \
		" until you do");
	return -1;
    }
      
    if (gf_exists) {
    	if (fontinit(&gamefont,gamefontname,gameFontSize)) {
    	    error("Font initialization failed with %s", gamefontname);
	} else gf_init = true;
    }
    if (!gf_init && df_exists) {
    	if (fontinit(&gamefont,defaultfontname,gameFontSize)) {
    	    error("Default font initialization failed with %s", defaultfontname);
    	} else gf_init = true;
    }
    
    if (!gf_init) {
    	error("Failed to initialize any game font! (quitting)");
	return -1;
    }
    
    if (gf_exists) {
    	if (fontinit(&mapfont,gamefontname,mapFontSize)) {
    	    error("Font initialization failed with %s", gamefontname);
	} else mf_init = true;
    }
    if (!mf_init && df_exists) {
    	if (fontinit(&mapfont,defaultfontname,mapFontSize)) {
    	    error("Default font initialization failed with %s", defaultfontname);
    	} else mf_init = true;
    }

    if (!mf_init) {
    	error("Failed to initialize any map font! (quitting)");
	return -1;
    }

    /* Set up the clipboard */
    if ( init_scrap() < 0 ) {
    	error("Couldn't init clipboard: %s\n");
    }

    return 0;
}

/* function to reset our viewport after a window resize */
int Resize_Window( int width, int height )
{
    SDL_Rect b = {0,0,0,0};

	if (windowFlags & SDL_WINDOW_FULLSCREEN)
		if (!find_size(&width, &height))
			return -1;

    b.w = draw_width = width;
    b.h = draw_height = height;
    
    SetBounds_GLWidget(MainWidget,&b);
    
    /* change to the projection matrix and set our viewing volume. */
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


void Platform_specific_cleanup(void)
{
    Close_Widget(&MainWidget);
    Gui_cleanup();
    Console_cleanup();
    fontclean(&gamefont);
    fontclean(&mapfont);
    TTF_Quit();
    SDL_Quit();
}

static bool Set_geometry(xp_option_t *opt, const char *s)
{
    int w = 0, h = 0;

    if (s[0] == '=') {
	sscanf(s, "%*c%d%*c%d", &w, &h);
    } else {
	sscanf(s, "%d%*c%d", &w, &h);
    }
    if (w == 0 || h == 0) return false;
    if (mainWindow != NULL) {
	Resize_Window(w, h);
    } else {
	draw_width = w;
	draw_height = h;
    }
    return true;
}

static const char* Get_geometry(xp_option_t *opt)
{
    static char buf[20]; /* should be enough */
    snprintf(buf, 20, "%dx%d", draw_width, draw_height);
    return buf;
}

static bool Set_fontName(xp_option_t *opt, const char *value)
{
    UNUSED_PARAM(opt);
    XFREE(gamefontname);
    gamefontname = xp_safe_strdup(value);

    return true;
}

static const char *Get_fontName(xp_option_t *opt)
{
    UNUSED_PARAM(opt);
    return gamefontname;
}

static xp_option_t sdlinit_options[] = {
    XP_STRING_OPTION(
	"geometry",
	"1280x1024",
	NULL,
	0,
	Set_geometry, NULL, Get_geometry,
	XP_OPTFLAG_DEFAULT,
	"Set the initial window geometry.\n"),
    
     XP_INT_OPTION(
        "gameFontSize",
	16, 12, 32,
	&gameFontSize,
	NULL,
	XP_OPTFLAG_DEFAULT,
	"Height of font used for game strings.\n"),

    XP_INT_OPTION(
        "mapFontSize",
	16, 12, 64,
	&mapFontSize,
	NULL,
	XP_OPTFLAG_DEFAULT,
	"Height of font used for strings painted on the map.\n"),

    XP_STRING_OPTION(
	"TTFont",
	CONF_FONTDIR "FreeSansBoldOblique.ttf",
	NULL, 0,
	Set_fontName, NULL, Get_fontName,
	XP_OPTFLAG_DEFAULT,
	"Set the font to use.\n")
};

void Store_sdlinit_options(void)
{
    STORE_OPTIONS(sdlinit_options);
}

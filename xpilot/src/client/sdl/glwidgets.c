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
#include "xpclient.h"
#include "sdlpaint.h"
#include "images.h"
#include "text.h"
#include "glwidgets.h"

/*widget_list_t *ListAdd_GLWidget( widget_list_t *list, GLWidget *widget)
{
    return AppendListItem( list, widget->draw, *widget, widget->GuiReg, *widget, widget->GuiUnReg, *widget );
}*/


/* one size should fit all */
/* LI->data should be a SDL_Rect with the bounds */
void GuiReg(void *LI)
{
    GLWidget *widget = (GLWidget *)(((widget_list_t *)LI)->GuiRegData);
    widget->guiarea = register_guiarea(widget->bounds,
    	    	    	    	widget->button,
    	    	    	    	widget->buttondata,
			    	widget->motion,
				widget->motiondata,
			    	widget->hover,
				widget->hoverdata
				);    	    	    	    	
}

/* one size should fit all */
void GuiUnReg(void *LI)
{
    GLWidget *tmp = (GLWidget *)(((widget_list_t *)LI)->GuiUnRegData);
    unregister_guiarea(tmp->guiarea);
}

/* only supposed to take care of mallocs done on behalf of the
 * appropriate Init_<foo> function
 */
void Close_Widget (GLWidget *widget)
{
    switch (widget->WIDGET) {
    	case INTCHOOSERWIDGET:
	    free_string_texture( &(((IntChooserWidget *)widget->wid_info)->nametex) );
	    free_string_texture( &(((IntChooserWidget *)widget->wid_info)->valuetex) );
	    break;
	default: break;
    }
    free(widget->wid_info);
    free(widget);
}


/* Begin:  ArrowWidget*/
void button_ArrowWidget( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data);
void Paint_ArrowWidget(void *LI);

void button_ArrowWidget( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data)
{
    ArrowWidget *tmp = (ArrowWidget *)(((GLWidget *)data)->wid_info);
    if (state == SDL_PRESSED) {
	if (button == 1) {
    	    tmp->press = true;
	}
	if (button == 2) {
	    tmp->tap = true;
	    if ((!(tmp->locked)) && tmp->action) tmp->action(tmp->actiondata);
	}
    }
    if (state == SDL_RELEASED) {
	if (button == 1) {
    	    tmp->press = false;
	}
	if (button == 2) {
	    /*tmp->tap = false;*/
	}
    }
}

void Paint_ArrowWidget(void *LI)
{
    static int normalcolor  = 0xff0000ff;
    static int presscolor   = 0x00ff00ff;
    static int tapcolor     = 0xffffffff;
    static int lockcolor    = 0x880000ff;
     
    GLWidget *tmp = (GLWidget *)(((widget_list_t *)LI)->DrawData);
    SDL_Rect *b = &(tmp->bounds);
    ArrowWidget *wid_info = (ArrowWidget *)(tmp->wid_info);
    
    if (wid_info->locked) {
    	set_alphacolor( lockcolor );
    } else if (wid_info->press) {
    	if (wid_info->action) {
	    wid_info->action(wid_info->actiondata);
	}
	set_alphacolor( presscolor );
    } else if (wid_info->tap) {
    	set_alphacolor( tapcolor );
    	wid_info->tap = false;
    } else {
    	set_alphacolor( normalcolor );
    }
    
    ArrowWidget_dir_t dir = wid_info->direction;
    glBegin(GL_POLYGON);
    switch ( dir ) {
    	case RIGHTARROW:
	    glVertex2i(b->x 	    ,b->y);
	    glVertex2i(b->x 	    ,b->y+b->h);
	    glVertex2i(b->x + b->w  ,b->y+b->h/2);
	    break;
    	case UPARROW:
	    glVertex2i(b->x + b->w/2,b->y);
	    glVertex2i(b->x 	    ,b->y+b->h);
	    glVertex2i(b->x + b->w  ,b->y+b->h);
	    break;
    	case LEFTARROW:
	    glVertex2i(b->x + b->w  ,b->y);
	    glVertex2i(b->x 	    ,b->y+b->h/2);
	    glVertex2i(b->x + b->w  ,b->y+b->h);
	    break;
    	case DOWNARROW:
	    glVertex2i(b->x 	    ,b->y);
	    glVertex2i(b->x + b->w/2,b->y+b->h);
	    glVertex2i(b->x + b->w  ,b->y);
	    break;
	default:
	    error("Weird direction for ArrowWidget! (direction:%i)\n",dir);
    }
    glEnd();
}

GLWidget *Init_ArrowWidget(widget_list_t *list, ArrowWidget_dir_t direction,int width, int height,
    	     void (*action)( void *data), void *actiondata )
{
    GLWidget *tmp	= malloc(sizeof(GLWidget));
    if ( !tmp ) {
        error("Failed to malloc in Init_ArrowWidget");
	return NULL;
    }
    tmp->wid_info   	= malloc(sizeof(ArrowWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc in Init_ArrowWidget");
	return NULL;
    }
    tmp->WIDGET     	= ARROWWIDGET;
    ((ArrowWidget *)tmp->wid_info)->direction  = direction;
    tmp->bounds.x   	= 0;
    tmp->bounds.y   	= 0;
    tmp->bounds.w   	= width;
    tmp->bounds.h   	= height;
    ((ArrowWidget *)tmp->wid_info)->press = false;
    ((ArrowWidget *)tmp->wid_info)->tap = false;
    ((ArrowWidget *)tmp->wid_info)->locked = false;
    ((ArrowWidget *)tmp->wid_info)->action = action;
    ((ArrowWidget *)tmp->wid_info)->actiondata = actiondata;
    tmp->button     	= button_ArrowWidget;
    tmp->buttondata 	= tmp;
    tmp->motion     	= NULL;
    tmp->motiondata 	= NULL;
    tmp->hover	    	= NULL;
    tmp->hoverdata  	= NULL;
    tmp->listPtr    	= AppendListItem( list, Paint_ArrowWidget, tmp, GuiReg, tmp, GuiUnReg, tmp );
    return tmp;
}

/* End:  ArrowWidget*/

/* Begin: IntChooserWidget */
void IntChooserWidget_Add( void *data );
void IntChooserWidget_Subtract( void *data );
void Paint_IntChooserWidget(void *LI);

void IntChooserWidget_Add( void *data )
{
    IntChooserWidget *tmp = ((IntChooserWidget *)((GLWidget *)data)->wid_info);
    if (*(tmp->value) < tmp->max) {
    	if ( (*(tmp->value)) == tmp->min)
	    ((ArrowWidget *)tmp->leftarrow->wid_info)->locked = false;
    	++(*(tmp->value));
    	char valuetext[16];
	snprintf(valuetext,15,"%i",*(tmp->value));
	if(!render_text(tmp->font,valuetext,&(tmp->valuetex)))
	    error("Failed to make value (%i) texture for IntChooserWidget!\n",*(tmp->value));
    } else {
    	((ArrowWidget *)tmp->rightarrow->wid_info)->locked = true;
    }
}

void IntChooserWidget_Subtract( void *data )
{
    IntChooserWidget *tmp = ((IntChooserWidget *)((GLWidget *)data)->wid_info);
    if (*(tmp->value) > tmp->min) {
    	if ( (*(tmp->value)) == tmp->max)
	    ((ArrowWidget *)tmp->rightarrow->wid_info)->locked = false;
	--(*(tmp->value));
    	char valuetext[16];
	snprintf(valuetext,15,"%i",*(tmp->value));
	if(!render_text(tmp->font,valuetext,&(tmp->valuetex)))
	    error("Failed to make value (%i) texture for IntChooserWidget!\n",*(tmp->value));
    } else {
    	((ArrowWidget *)tmp->leftarrow->wid_info)->locked = true;
    }
}

void Paint_IntChooserWidget(void *LI)
{
    GLWidget *widget = (GLWidget *)(((widget_list_t *)LI)->DrawData);
    IntChooserWidget *wid_info = (IntChooserWidget *)(widget->wid_info);
    set_alphacolor(blueRGBA);
    glBegin(GL_QUADS);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y);
    	glVertex2i(widget->bounds.x+widget->bounds.w,widget->bounds.y+widget->bounds.h);
    	glVertex2i(widget->bounds.x 	    	    ,widget->bounds.y+widget->bounds.h);
    glEnd();
    disp_text(&(wid_info->nametex), whiteRGBA, LEFT, DOWN, widget->bounds.x+2, draw_height - widget->bounds.y, true);
    disp_text(&(wid_info->valuetex), greenRGBA, RIGHT, DOWN, wid_info->rightarrow->bounds.x-1/*value_>*/-2/*>_|*/, draw_height - widget->bounds.y, true );
}

GLWidget *Init_IntChooserWidget(widget_list_t *list, const char *name, font_data *font, int *value, int min, int max)
{
    GLWidget *tmp;
    tmp = malloc(sizeof(GLWidget));
    if ( !tmp ) {
        error("Failed to malloc in Init_IntChooserWidget");
	return NULL;
    }
    tmp->wid_info   = malloc(sizeof(IntChooserWidget));
    if ( !(tmp->wid_info) ) {
    	free(tmp);
        error("Failed to malloc in Init_IntChooserWidget");
	return NULL;
    }
    IntChooserWidget *wid_info = tmp->wid_info;
    tmp->WIDGET     = INTCHOOSERWIDGET;
    if (render_text(font,name,&(wid_info->nametex))) {
    	char valuetext[16];
	snprintf(valuetext,15,"%i",*value);
    	if(render_text(font,valuetext,&(wid_info->valuetex))) {
    	    wid_info->font     = font;
	    wid_info->value    = value;
	    wid_info->min      = min;
	    wid_info->max      = max;
    	    /* this needs to be the first thing added, or it will cover the sub-parts */
	    tmp->listPtr = AppendListItem( list, Paint_IntChooserWidget, tmp, GuiReg, tmp, GuiUnReg, tmp );

	    if (!(wid_info->leftarrow  = Init_ArrowWidget(list,LEFTARROW,12,16,IntChooserWidget_Subtract,tmp))) {
	    	free_string_texture(&(wid_info->nametex));
	    	free_string_texture(&(wid_info->valuetex));
		free(tmp->wid_info);
		free(tmp);
		error("Init_IntChooserWidget couldn't init leftarrow!");
		return NULL;
	    } 	
	    if (*value == min)	((ArrowWidget *)(wid_info->leftarrow->wid_info))->locked = true;

    	    if (!(wid_info->rightarrow = Init_ArrowWidget(list,RIGHTARROW,12,16,IntChooserWidget_Add,tmp))) {
	    	free_string_texture(&(wid_info->nametex));
	    	free_string_texture(&(wid_info->valuetex));
		free(tmp->wid_info);
		free(tmp);
	    	error("Init_IntChooserWidget couldn't init rightarrow!");
		return NULL;
	    }
	    if (*value == max)	((ArrowWidget *)(wid_info->rightarrow->wid_info))->locked = true;

    	    tmp->bounds.x   = 0;
    	    tmp->bounds.y   = 0;
    	    tmp->bounds.w   = 2+ wid_info->nametex.width +5/*text___<*/ + 27/*__value*/ + 2/*<_value_>*/
	    	    	     + wid_info->leftarrow->bounds.w + wid_info->rightarrow->bounds.w +2/*>_|*/;
    	    tmp->bounds.h   = wid_info->nametex.height;
    	    tmp->button     	= NULL;
    	    tmp->buttondata 	= NULL;
    	    tmp->motion     	= NULL;
    	    tmp->motiondata 	= NULL;
    	    tmp->hover	    	= NULL;
    	    tmp->hoverdata  	= NULL;
	    return tmp;
    	} else free_string_texture(&(wid_info->nametex));
    }
    free(tmp);
    error("Failed to initialize Init_IntChooserWidget %s (couldn't render text)",name);
    return NULL;
}

void SetBounds_GLWidget(GLWidget *widget, SDL_Rect *b)
{
    widget->bounds.x = b->x;
    widget->bounds.y = b->y;
    widget->bounds.w = b->w;
    widget->bounds.h = b->h;
    if(widget->guiarea) {
    	widget->guiarea->bounds.x = b->x;
    	widget->guiarea->bounds.y = b->y;
    	widget->guiarea->bounds.w = b->w;
    	widget->guiarea->bounds.h = b->h;
    }
    switch (widget->WIDGET) {
    	case INTCHOOSERWIDGET:
    	    {
    	    IntChooserWidget *tmp = (IntChooserWidget *)(widget->wid_info);
    	    static SDL_Rect rab;
    	    static SDL_Rect lab;
    
    	    lab.y = rab.y = widget->bounds.y + 1;
	    lab.h = rab.h = widget->bounds.h - 2;
	    lab.w = rab.w = rab.h;
    	    rab.x = widget->bounds.x + widget->bounds.w - rab.w -2/*>_|*/;
    	    lab.x = rab.x - 27/*_value*/ -2/*<_value_>*/ - lab.w;
	    
	    SetBounds_GLWidget(tmp->leftarrow,&lab);
	    SetBounds_GLWidget(tmp->rightarrow,&rab);
	    break;
	    }
	default: break;
    }
}

/* End: IntChooserWidget */


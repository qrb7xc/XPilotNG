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

#ifndef XPSERVER_H
#define XPSERVER_H

#define SERVER
#include "xpcommon.h"

#ifdef HAVE_EXPAT_H
#  include <expat.h>
#endif

#ifdef PLOCKSERVER
#  ifdef HAVE_SYS_MMAN_H
#    include <sys/mman.h>
#  elif defined HAVE_SYS_LOCK_H
#    include <sys/lock.h>
#  endif
#endif

#include "asteroid.h"
#include "auth.h"
#include "cannon.h"
#include "click.h"
#include "connection.h"
#include "defaults.h"
#include "global.h"
#include "map.h"
#include "metaserver.h"
#include "netserver.h"
#include "objpos.h"
#include "packet.h"
#include "proto.h"
#include "rank.h"
#include "recwrap.h"
#include "robot.h"
#include "saudio.h"
#include "sched.h"
#include "setup.h"
#include "score.h"
#include "server.h"
#include "serverconst.h"
#include "srecord.h"
#include "teamcup.h"
#include "tuner.h"
#include "walls.h"

#endif /* XPSERVER_H */

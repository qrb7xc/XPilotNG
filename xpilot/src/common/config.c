/* 
 * XPilot NG, a multiplayer space war game.
 *
 * Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
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

#include "xpcommon.h"

#include <stdlib.h>

char *Conf_homedir(void)
{
    static char conf[4096];
    static bool init = true;
	if (init)
	{
		const char* homedir = getenv("HOME");
		if (homedir != NULL)
		{
			const size_t homedir_len = strnlen(homedir, 4096);
			strncpy(conf, homedir, homedir_len);
		}
		else
		{
			// homedir unknown
			strcpy(conf, "");
		}
		init = false;
	}

	return conf;
}

void prepend_str(char *str, const char *prefix, size_t len)
{
	memmove(str + len, str, CONF_MAXLEN - len);
	strncpy(str, prefix, len);
}

void Conf_init()
{
	// fixup conf dirs
	const char* homedir = Conf_homedir();
	const size_t homedir_len = strnlen(homedir, CONF_MAXLEN);

	prepend_str(Conf_datadir(), homedir, homedir_len);
	prepend_str(Conf_defaults_file_name(), homedir, homedir_len);
	prepend_str(Conf_password_file_name(), homedir, homedir_len);
	//prepend_str(Conf_player_passwords_file_name(), homedir, homedir_len);
	prepend_str(Conf_mapdir(), homedir, homedir_len);
	prepend_str(Conf_fontdir(), homedir, homedir_len);
	prepend_str(Conf_font_file(), homedir, homedir_len);
    char *servermotdfile_env = getenv("XPILOTSERVERMOTD");
    if (servermotdfile_env != NULL)
    {
		strncpy(Conf_servermotdfile(), servermotdfile_env, CONF_MAXLEN);
	}
	else
	{
		prepend_str(Conf_servermotdfile(), homedir, homedir_len);
	}
	prepend_str(Conf_localmotdfile(), homedir, homedir_len);
	prepend_str(Conf_logfile(), homedir, homedir_len);
	prepend_str(Conf_ship_file(), homedir, homedir_len);
	prepend_str(Conf_texturedir(), homedir, homedir_len);
	prepend_str(Conf_sounddir(), homedir, homedir_len);
	prepend_str(Conf_soundfile(), homedir, homedir_len);
	prepend_str(Conf_robotfile(), homedir, homedir_len);
}

char *Conf_datadir(void)
{
    static char conf[CONF_MAXLEN] = CONF_DATADIR;

    return conf;
}

char *Conf_defaults_file_name(void)
{
    static char conf[CONF_MAXLEN] = CONF_DEFAULTS_FILE_NAME;

    return conf;
}

char *Conf_password_file_name(void)
{
    static char conf[CONF_MAXLEN] = CONF_PASSWORD_FILE_NAME;

    return conf;
}

#if 0
char *Conf_player_passwords_file_name(void)
{
    static char conf[CONF_MAXLEN] = CONF_PLAYER_PASSWORDS_FILE_NAME;

    return conf;
}
#endif

char *Conf_mapdir(void)
{
    static char conf[CONF_MAXLEN] = CONF_MAPDIR;

    return conf;
}

char *Conf_fontdir(void)
{
    static char conf[CONF_MAXLEN] = CONF_FONTDIR;

    return conf;
}

char conf_font_file_string[CONF_MAXLEN] = CONF_FONT_FILE;

char *Conf_font_file(void)
{
    return conf_font_file_string;
}

char *Conf_default_map(void)
{
    static char conf[] = CONF_DEFAULT_MAP;

    return conf;
}

char *Conf_servermotdfile(void)
{
    static char conf[CONF_MAXLEN] = CONF_SERVERMOTDFILE;

    return conf;
}

char *Conf_localmotdfile(void)
{
    static char conf[CONF_MAXLEN] = CONF_LOCALMOTDFILE;

    return conf;
}

char conf_logfile_string[CONF_MAXLEN] = CONF_LOGFILE;

char *Conf_logfile(void)
{
    return conf_logfile_string;
}

char conf_ship_file_string[CONF_MAXLEN] = CONF_SHIP_FILE;

char *Conf_ship_file(void)
{
    return conf_ship_file_string;
}

char conf_texturedir_string[CONF_MAXLEN] = CONF_TEXTUREDIR;

char *Conf_texturedir(void)
{
    return conf_texturedir_string;
}

char *Conf_localguru(void)
{
    static char conf[CONF_MAXLEN] = CONF_LOCALGURU;

    return conf;
}

char *Conf_robotfile(void)
{
    static char conf[CONF_MAXLEN] = CONF_ROBOTFILE;

    return conf;
}

char *Conf_zcat_ext(void)
{
    static char conf[CONF_MAXLEN] = CONF_ZCAT_EXT;

    return conf;
}

char *Conf_zcat_format(void)
{
    static char conf[CONF_MAXLEN] = CONF_ZCAT_FORMAT;

    return conf;
}

char *Conf_sounddir(void)
{
    static char conf[CONF_MAXLEN] = CONF_SOUNDDIR;

    return conf;
}

char conf_soundfile_string[CONF_MAXLEN] = CONF_SOUNDFILE;

char *Conf_soundfile(void)
{
    return conf_soundfile_string;
}


void Conf_print(void)
{
    warn("============================================================");
    warn("VERSION                   = %s", VERSION);
    warn("PACKAGE                   = %s", PACKAGE);

#ifdef DBE
    warn("DBE");
#endif
#ifdef MBX
    warn("MBX");
#endif
#ifdef PLOCKSERVER
    warn("PLOCKSERVER");
#endif
#ifdef DEVELOPMENT
    warn("DEVELOPMENT");
#endif

    warn("Conf_localguru()          = %s", Conf_localguru());
    warn("Conf_datadir()            = %s", Conf_datadir());
    warn("Conf_defaults_file_name() = %s", Conf_defaults_file_name());
    warn("Conf_password_file_name() = %s", Conf_password_file_name());
    warn("Conf_mapdir()             = %s", Conf_mapdir());
    warn("Conf_default_map()        = %s", Conf_default_map());
    warn("Conf_servermotdfile()     = %s", Conf_servermotdfile());
    warn("Conf_robotfile()          = %s", Conf_robotfile());
    warn("Conf_logfile()            = %s", Conf_logfile());
    warn("Conf_localmotdfile()      = %s", Conf_localmotdfile());
    warn("Conf_ship_file()          = %s", Conf_ship_file());
    warn("Conf_texturedir()         = %s", Conf_texturedir());
    warn("Conf_fontdir()            = %s", Conf_fontdir());
    warn("Conf_sounddir()           = %s", Conf_sounddir());
    warn("Conf_soundfile()          = %s", Conf_soundfile());
    warn("Conf_zcat_ext()           = %s", Conf_zcat_ext());
    warn("Conf_zcat_format()        = %s", Conf_zcat_format());
    warn("============================================================");
}

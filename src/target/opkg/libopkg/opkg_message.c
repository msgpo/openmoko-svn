/* opkg_message.c - the opkg package management system

   Copyright (C) 2003 Daniele Nicolodi <daniele@grinta.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/


#include "includes.h"
#include "opkg_conf.h"
#include "opkg_message.h"

opkg_message_callback opkg_cb_message = NULL;

void
opkg_message (opkg_conf_t * conf, message_level_t level, char *fmt, ...)
{
	va_list ap;
	char ts[256];

	va_start (ap, fmt);
	vsnprintf (ts,256,fmt, ap);
	va_end (ap);

	if (opkg_cb_message)
	{
		opkg_cb_message(conf,level,ts);
	}
	else
	{
	  char *level_s[5] = {"ERROR", "NOTICE", "INFO", "DEBUG", "DEBUG2"};
	  if (level <= conf->verbosity)
	    printf ("opkg-%s: %s", level_s[level], ts);
	}
}

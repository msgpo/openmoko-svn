/**
 *  @file filter-menu.h
 *  @brief The filter menu item
 *
 *  Copyright (C) 2006 First International Computer Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 *  @author Chaowei Song (songcw@fic-sh.com.cn)
 */
#ifndef _FIC_FILTER_MENU_H
#define _FIC_FILTER_MENU_H

#include <gtk/gtk.h>

#include <libmokoui/moko-paned-window.h>
#include "appmanager-data.h"

GtkMenu *filter_menu_new (ApplicationManagerData *appdata);

void filter_menu_add_item (GtkMenu *filtermenu, const gchar *name,
                           ApplicationManagerData *appdata);

#endif


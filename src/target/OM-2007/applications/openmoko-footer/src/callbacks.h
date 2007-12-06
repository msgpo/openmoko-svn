/**
 * @file callbacks.h
 * @brief callbacks of openmoko-taskmanager based on callbacks.h.
 * @author Sun Zhiyong
 * @date 2006-10
 *
 * Copyright (C) 2006 FIC-SH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

#include <gtk/gtk.h>

#include "main.h"
#include "misc.h"

//#define G_THREADS_ENABLED

#define TASK_MANAGER_PROPERTY_WIDTH    200
#define TASK_MANAGER_PROPERTY_HEIGHT   564
#define TASK_MANAGER_PROPERTY_X    0
#define TASK_MANAGER_PROPERTY_Y    45

/* footer */
gboolean footer_leftbutton_clicked(GtkWidget *widget, GdkEvent *event, MokoFooterApp *app);

gboolean footer_rightbutton_clicked(GtkWidget *widget, GdkEvent *event, gpointer user_data);

GdkFilterReturn target_window_event_filter_cb (GdkXEvent *xevent, 
    GdkEvent *event, gpointer user_data);

GdkFilterReturn root_window_event_filter_cb (GdkXEvent *xevent, 
    GdkEvent *event, gpointer user_data);

#endif
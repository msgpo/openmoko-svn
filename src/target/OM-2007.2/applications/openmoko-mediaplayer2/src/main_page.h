/*
 *  OpenMoko Media Player
 *   http://openmoko.org/
 *
 *  Copyright (C) 2007 by the OpenMoko team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**
 * @file main_page.h
 * Main UI handling
 */

#ifndef MAIN_PAGE_H
#define MAIN_PAGE_H

#include <gtk/gtk.h>

#define OMP_WIDGET_CAPTION_TRACK_TIME "%d:%.2d / %d:%.2d"
#define OMP_WIDGET_CAPTION_TRACK_NUM "%.3d / %.3d"
#define OMP_WIDGET_CAPTION_VOLUME "%d%%"

// Determines how many milliseconds the engine will seek if the FFWD/REW buttons are clicked
#define BUTTON_SEEK_DISTANCE 10000

GtkWidget *omp_main_page_create();

#endif

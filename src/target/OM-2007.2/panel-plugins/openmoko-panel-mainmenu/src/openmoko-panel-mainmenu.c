/*
 *  openmoko panel mainmenu
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include "buttonactions.h"
#include "stylusmenu.h"
#include "mokodesktop.h"
#include "mokodesktop_item.h"
#include "callbacks.h"
#include "fingermenu.h"
#include "dbus-conn.h"

#include <libmokopanelui2/moko-panel-applet.h>
#include <libmokoui/moko-window.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib.h>

#include <X11/Xatom.h>

#include <unistd.h>

/*typedef struct _MokoMainmenuApp MokoMainmenuApp;

struct _MokoMainmenuApp {
	MokoFingerMenu *fm;
	MokoStylusMenu *sm;
    MokoDesktopItem *top_item;
};*/

static GtkWidget *sm = NULL;
static MokoMainmenuApp *mma;

static void click(MokoPanelApplet * applet)
{
	g_debug("mainmenu click callback");
	if (sm)
		gtk_menu_popup(sm, NULL, NULL,
			       (GtkMenuPositionFunc) moko_menu_position_cb,
			       GTK_WIDGET(applet), 0,
			       gtk_get_current_event_time());
	else
		g_debug("no stylus menu objcet");
}

static void tap_hold(MokoPanelApplet * applet)
{
	g_debug("tap hold event callback");
/*	GtkWidget *widget = GTK_WIDGET(applet);
	Screen *screen = GDK_SCREEN_XSCREEN(gtk_widget_get_screen(widget));
	XEvent xev;

	xev.xclient.type = ClientMessage;
	xev.xclient.serial = 0;
	xev.xclient.send_event = True;
	xev.xclient.display = DisplayOfScreen(screen);
	xev.xclient.window = RootWindowOfScreen(screen);
	xev.xclient.message_type =
	    gdk_x11_get_xatom_by_name("_NET_SHOWING_DESKTOP");
	xev.xclient.format = 32;
	//TODO add support for toggle!?
	xev.xclient.data.l[0] = TRUE;
	xev.xclient.data.l[1] = 0;
	xev.xclient.data.l[2] = 0;
	xev.xclient.data.l[3] = 0;
	xev.xclient.data.l[4] = 0;

	XSendEvent(DisplayOfScreen(screen), RootWindowOfScreen(screen), False,
		   SubstructureRedirectMask | SubstructureNotifyMask, &xev);
*/
	mma = g_malloc0 (sizeof (MokoMainmenuApp));
    if (!mma)
    {
        g_error ("openmoko-mainmenu application initialize FAILED.");
		exit (0);
    }
    memset (mma, 0, sizeof (MokoMainmenuApp));

    if (!moko_dbus_connect_init ())
    {
        g_error ("Failed to initial dbus connection.");
		exit (0);
    }

 // moko_mainmenu_init (mma, argc, argv);

  /* Buid Root item, don't display */
  mma->top_item = mokodesktop_item_new_with_params ("Home",
						       NULL,
						       NULL,
						       ITEM_TYPE_ROOT );

  /* Build Lists (parse .directory and .desktop files) */
  mokodesktop_init(mma->top_item, ITEM_TYPE_CNT);

//  gtk_init();

  /*MokoFingerMenu object*/
  mma->fm = moko_finger_menu_new ();
  moko_finger_menu_build (mma->fm, mma->top_item);
  moko_finger_menu_show (mma->fm);

  gtk_main();

  if (mma)
  {
	g_free (mma);
  }

}

G_MODULE_EXPORT GtkWidget *mb_panel_applet_create(const char *id,
						  GtkOrientation orientation)
{
	g_debug("openmoko-panel-mainmenu new");

	MokoPanelApplet *applet = moko_panel_applet_new();
	g_debug("applet is %p", applet);
	moko_panel_applet_set_icon(applet, PKGDATADIR "/btn_menu.png", TRUE);
	g_signal_connect(applet, "clicked", G_CALLBACK(click), applet);
	g_signal_connect(applet, "tap-hold", G_CALLBACK(tap_hold), applet);
	gtk_widget_show_all(GTK_WIDGET(applet));
	sm = gtk_menu_new();
	gtk_widget_show(sm);

	MokoDesktopItem *top_item =
	    mokodesktop_item_new_with_params("Home", NULL, NULL,
					     ITEM_TYPE_ROOT);
	int ret = mokodesktop_init(top_item, ITEM_TYPE_CNT);
	moko_stylus_menu_build(GTK_MENU(sm), top_item);
	panel_mainmenu_install_watcher();

	return GTK_WIDGET(applet);
}

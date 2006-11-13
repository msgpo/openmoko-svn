/*
 *  Finger-Demo -- OpenMoko Demo Application
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 First International Computer Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include <libmokoui/moko-application.h>
#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-pixmap-container.h>

#include <gtk/gtkalignment.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkfixed.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenutoolbutton.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktoolbutton.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkentry.h>

#include <stdlib.h>

void cb_orange_button_clicked( GtkButton* button, MokoFingerWindow* window )
{
    g_debug( "openmoko-finger-demo: orange button clicked" );
}

int main( int argc, char** argv )
{
    g_debug( "openmoko-finger-demo starting up" );
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );

    /* application object */
    MokoApplication* app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "Finger-Demo" );

    /* main window */
    MokoFingerWindow* window = MOKO_FINGER_WINDOW(moko_finger_window_new());

    /* application menu */
    GtkMenu* appmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* closeitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Close" ));
    g_signal_connect( G_OBJECT(closeitem), "activate", G_CALLBACK(gtk_main_quit), NULL );
    gtk_menu_shell_append( appmenu, closeitem );
    moko_finger_window_set_application_menu( window, appmenu );

    /* connect close event */
    g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK( gtk_main_quit ), NULL );

    /* contents */
    GtkVBox* vbox = gtk_vbox_new( TRUE, 0 );
    GtkEntry* label1 = gtk_label_new( "Populate this area with finger widgets\nas you like..." );
    GtkEntry* label2 = gtk_label_new( "Click the finger button to enable or disable\nthe finger scrolling wheel\n\n\n" );

    GtkAlignment* align = gtk_alignment_new( 0.5, 0.5, 0.5, 0.5 );

    GtkButton* button = gtk_button_new();
    g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(cb_orange_button_clicked), window );
    gtk_widget_set_name( GTK_WIDGET(button), "mokofingerbutton-orange" );
    gtk_container_add( GTK_CONTAINER(align), GTK_WIDGET(button) );

    gtk_box_pack_start( vbox, GTK_WIDGET(label1), TRUE, TRUE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(align), TRUE, TRUE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(label2), TRUE, TRUE, 0 );

    moko_finger_window_set_contents( window, GTK_WIDGET(vbox) );

    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(window) );
    g_debug( "openmoko-finger-demo entering main loop" );
    gtk_main();
    g_debug( "openmoko-finger-demo left main loop" );

    return 0;
}

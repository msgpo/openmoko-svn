
/*  moko_search_bar.c
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 Vanille-Media
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
 *  Current Version: $Rev$ ($Date: 2006/10/05 17:38:14 $) [$Author: mickey $]
 */

#include "moko-search-bar.h"
#include <gtk/gtkalignment.h>
#include <gtk/gtkentry.h>

#define MOKO_SEARCH_BAR_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_SEARCH_BAR, MokoSearchBarPrivate))

typedef struct _MokoSearchBarPrivate MokoSearchBarPrivate;

struct _MokoSearchBarPrivate
{
    GtkAlignment* alignment;
    GtkEntry* entry;
};

static void
moko_search_bar_class_init (MokoSearchBarClass *klass);
static void
moko_search_bar_init (MokoSearchBar *self);


GType moko_search_bar_get_type (void) /* Typechecking */
{
    static GType self_type = 0;

    if (!self_type)
    {
        static const GTypeInfo f_info =
        {
            sizeof (MokoSearchBarClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) moko_search_bar_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (MokoSearchBar),
            0,
            (GInstanceInitFunc) moko_search_bar_init,
        };

        /* add the type of your parent class here */
        self_type = g_type_register_static(GTK_TYPE_TOOLBAR, "MokoSearchBar", &f_info, 0);
    }

    return self_type;
}

static void
moko_search_bar_class_init (MokoSearchBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MokoSearchBarPrivate));
}

static void
moko_search_bar_init (MokoSearchBar *self)
{
    MokoSearchBarPrivate* priv = MOKO_SEARCH_BAR_GET_PRIVATE(self);
    GtkToolItem* item = gtk_tool_item_new();
    gtk_widget_set_size_request( GTK_WIDGET(item), 320, 10 ); //FIXME get from style
    GtkEntry* entry = gtk_entry_new();
    gtk_widget_set_name( GTK_WIDGET(entry), "moko_search_entry" );
    gtk_entry_set_has_frame( entry, FALSE );
    gtk_entry_set_text( GTK_ENTRY(entry), "foo" );
    gtk_container_add( GTK_CONTAINER(item), GTK_WIDGET(entry) );
    gtk_toolbar_insert( self, GTK_TOOL_ITEM(item), 0 );
}

MokoSearchBar*
moko_search_bar_new (void)
{
    return GTK_WIDGET(g_object_new(moko_search_bar_get_type(), NULL));
}

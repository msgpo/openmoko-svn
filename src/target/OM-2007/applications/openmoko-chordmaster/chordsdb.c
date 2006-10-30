/*  chordsdb.c
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

#include "chordsdb.h"

G_DEFINE_TYPE (ChordsDB, chordsdb, G_TYPE_OBJECT);

#define CHORDSDB_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_CHORDSDB, ChordsDBPrivate))

typedef struct _ChordsDBPrivate
{
} ChordsDBPrivate;

/* forward declarations */
Chord* chord_new( gchar* name, gchar* frets )
{
    g_debug( "chord_new: adding new chord '%s' = '%s'", name, frets );
    Chord* chord = g_new( Chord, 1 );
    chord->name = g_strdup( name );
    chord->frets = g_strdup( frets );
    return chord;
}

static void
chordsdb_dispose (GObject *object)
{
    if (G_OBJECT_CLASS (chordsdb_parent_class)->dispose)
        G_OBJECT_CLASS (chordsdb_parent_class)->dispose (object);
}

static void
chordsdb_finalize (GObject *object)
{
    G_OBJECT_CLASS (chordsdb_parent_class)->finalize (object);
}

static void
chordsdb_class_init (ChordsDBClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    /* register private data */
    g_type_class_add_private (klass, sizeof (ChordsDBPrivate));

    /* hook virtual methods */
    /* ... */

    /* install properties */
    /* ... */

    object_class->dispose = chordsdb_dispose;
    object_class->finalize = chordsdb_finalize;

    //FIXME read from chords file
    klass->categories = g_slist_append( klass->categories, "C" );
    klass->categories = g_slist_append( klass->categories, "D" );
    klass->categories = g_slist_append( klass->categories, "E" );
    klass->categories = g_slist_append( klass->categories, "F" );
    klass->categories = g_slist_append( klass->categories, "G" );
    klass->categories = g_slist_append( klass->categories, "A" );
    klass->categories = g_slist_append( klass->categories, "B" );

    //FIXME read from chords file
    klass->chords = g_slist_append( klass->chords, chord_new( "A", "002220" ) );
    klass->chords = g_slist_append( klass->chords, chord_new( "A", "577655" ) );
    klass->chords = g_slist_append( klass->chords, chord_new( "A", "x02220" ) );
}

static void
chordsdb_init (ChordsDB *self)
{
}

ChordsDB*
chordsdb_new (void)
{
    return g_object_new (TYPE_CHORDSDB, NULL);
}

GSList* chordsdb_get_categories(ChordsDB* self)
{
    ChordsDBClass* klass = CHORDSDB_GET_CLASS(self);
    return klass->categories;
}

GSList* chordsdb_get_chords(ChordsDB* self )
{
    ChordsDBClass* klass = CHORDSDB_GET_CLASS(self);
    return klass->chords;
}

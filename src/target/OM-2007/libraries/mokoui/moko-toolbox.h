/*
 *  libmokoui -- OpenMoko Application Framework UI Library
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
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
 */
#ifndef _MOKO_TOOL_BOX_H_
#define _MOKO_TOOL_BOX_H_

#include "moko-pixmap-button.h"

#include <gtk/gtknotebook.h>
#include <gtk/gtkhbox.h>

#include <glib.h>
#include <glib-object.h>


G_BEGIN_DECLS

#define MOKO_TYPE_TOOL_BOX            (moko_tool_box_get_type())
#define MOKO_TOOL_BOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_TOOL_BOX, MokoToolBox))
#define MOKO_TOOL_BOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_TOOL_BOX, MokoToolBoxClass))
#define MOKO_IS_TOOL_BOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_TOOL_BOX))
#define MOKO_IS_TOOL_BOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_TOOL_BOX))

typedef struct _MokoToolBox       MokoToolBox;
typedef struct _MokoToolBoxClass  MokoToolBoxClass;

struct _MokoToolBox
{
    GtkNotebook parent;
    /* add pointers to new members here */
};

struct _MokoToolBoxClass
{
    /* add your parent class here */
    GtkNotebookClass parent_class;
    void (*moko_tool_box) (MokoToolBox *self);
};

GType          moko_tool_box_get_type();
GtkWidget*     moko_tool_box_new();
GtkWidget*     moko_tool_box_new_with_search();
void           moko_tool_box_clear(MokoToolBox* self);

/* add additional methods here */
GtkHBox* moko_tool_box_get_button_box( MokoToolBox* self );
MokoPixmapButton* moko_tool_box_add_action_button(MokoToolBox* self);

G_END_DECLS

#endif /* _MOKO_TOOL_BOX_H_ */

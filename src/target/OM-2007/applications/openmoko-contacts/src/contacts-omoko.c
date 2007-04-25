/*
 * Copyright (C) 2006-2007 by OpenMoko, Inc.
 * Written by OpenedHand Ltd <info@openedhand.com>
 * All Rights Reserved
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include "contacts-callbacks-ui.h"
#include "contacts-contact-pane.h"
#include "contacts-main.h"
#include "contacts-ui.h"
#include "contacts-omoko.h"
#include "contacts-groups-editor.h"
#include "contacts-callbacks-ebook.h"



static void moko_filter_changed (GtkWidget *widget, gchar *text, ContactsData *data);

/* these are specific to the omoko frontend */
static GtkMenu *filter_menu;

static void
fullname_changed_cb (ContactsContactPane *pane, EContact *contact, ContactsData *data)
{
	EContactListHash *hash;
	gchar *name = NULL;
	const gchar *uid;

	uid = e_contact_get_const (contact, E_CONTACT_UID);
	hash = g_hash_table_lookup (data->contacts_table, uid);
	name = e_contact_get (contact, E_CONTACT_FULL_NAME);

	if ((!name) || (g_utf8_strlen (name, -1) <= 0))
	{
		g_free (name);
		name = g_strdup (_("Unnamed"));
	}
	gtk_list_store_set (data->contacts_liststore, &hash->iter, CONTACT_NAME_COL, name, -1);

	g_free (name);
}

static void
cell_changed_cb (ContactsContactPane *pane, EContact *contact, ContactsData *data)
{
	EContactListHash *hash;
	gchar *cell = NULL;
	const gchar *uid;

	uid = e_contact_get_const (contact, E_CONTACT_UID);
	hash = g_hash_table_lookup (data->contacts_table, uid);
	cell = e_contact_get (contact, E_CONTACT_PHONE_MOBILE);

	if (!cell)
	{
		cell = g_strdup ("");
	}
	gtk_list_store_set (data->contacts_liststore, &hash->iter, CONTACT_CELLPHONE_COL, cell, -1);

	g_free (cell);
}

GtkWidget *
create_contacts_list (ContactsData *data)
{
	GtkWidget *treeview = moko_tree_view_new ();

	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview),
				 GTK_TREE_MODEL (data->contacts_filter));


	/* add columns to treeview */
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	/* name column */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Name"), renderer,
							"text", CONTACT_NAME_COL, NULL);
	gtk_tree_view_column_set_sort_column_id (column, CONTACT_NAME_COL);
	moko_tree_view_append_column (MOKO_TREE_VIEW (treeview), column);

	/* mobile column */
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Cell Phone"), renderer,
							"text", CONTACT_CELLPHONE_COL, NULL);
	gtk_tree_view_column_set_sort_column_id (column, CONTACT_CELLPHONE_COL);
	moko_tree_view_append_column (MOKO_TREE_VIEW (treeview), column);

	return treeview;
}

void
create_main_window (ContactsData *contacts_data)
{
	GtkWidget *contacts_menu_menu, *widget;
	GtkAccelGroup *accel_group;
	GtkWidget *moko_tool_box;
	GtkWidget *details_pane;
	GtkWidget *navigation_pane;

	ContactsUI *ui = contacts_data->ui;

	accel_group = gtk_accel_group_new ();

	//? MokoApplication* app = MOKO_APPLICATION(moko_application_get_instance());

	g_set_application_name ("Phone Book");

	ui->main_window = moko_paned_window_new ();
	gtk_window_set_title (GTK_WINDOW (ui->main_window), _("Contacts"));
	gtk_window_set_default_size (GTK_WINDOW (ui->main_window), -1, 300);
	gtk_window_set_icon_name (GTK_WINDOW (ui->main_window), "stock_contact");

	/* main menu */
	contacts_menu_menu = gtk_menu_new ();
	moko_paned_window_set_application_menu(MOKO_PANED_WINDOW (ui->main_window), GTK_MENU (contacts_menu_menu));

	widget = gtk_menu_item_new_with_label ("New Contact");
	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), GTK_WIDGET (widget));
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (contacts_new_cb), contacts_data);

	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), GTK_WIDGET (gtk_separator_menu_item_new()));

	widget = gtk_menu_item_new_with_label ("Import");
	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), GTK_WIDGET (widget));
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (contacts_import_cb), contacts_data);

	ui->contact_export = gtk_menu_item_new_with_label ("Export");
	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), GTK_WIDGET (ui->contact_export));
	g_signal_connect (G_OBJECT (ui->contact_export), "activate",
			  G_CALLBACK (contacts_export_cb), contacts_data);

	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), GTK_WIDGET (gtk_separator_menu_item_new()));

	widget = gtk_menu_item_new_with_label ("Add Group");
	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), GTK_WIDGET (widget));
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (contacts_groups_new_cb), contacts_data);

	ui->edit_menuitem = gtk_menu_item_new_with_label ("Edit Contact");
	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), GTK_WIDGET (ui->edit_menuitem));
	g_signal_connect (G_OBJECT (ui->edit_menuitem), "activate",
			  G_CALLBACK (contacts_edit_cb), contacts_data);

	ui->delete_menuitem = gtk_menu_item_new_with_label ("Delete Contact");
	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), GTK_WIDGET (ui->delete_menuitem));
	g_signal_connect (G_OBJECT (ui->delete_menuitem), "activate",
			  G_CALLBACK (contacts_delete_cb), contacts_data);

	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), GTK_WIDGET (gtk_separator_menu_item_new()));

	widget = gtk_menu_item_new_with_label ("Close");
	gtk_container_add (GTK_CONTAINER (contacts_menu_menu), GTK_WIDGET (widget));
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (gtk_main_quit), NULL);


	/*** end menu creation ***/

	/*** filter menu ***/

	filter_menu = (GtkMenu*) gtk_menu_new ();
	moko_paned_window_set_filter_menu (MOKO_PANED_WINDOW (ui->main_window), GTK_MENU(filter_menu));

	/*** contacts list ***/

	widget = create_contacts_list (contacts_data);
	g_object_unref (contacts_data->contacts_liststore);

	navigation_pane = moko_scrolled_pane_new ();
	moko_scrolled_pane_pack (MOKO_SCROLLED_PANE (navigation_pane), widget);
	moko_paned_window_set_navigation_pane (MOKO_PANED_WINDOW (ui->main_window), 
			navigation_pane);

	ui->contacts_treeview = widget;

	/* Connect signal for selection changed event */
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (ui->contacts_treeview));
	g_signal_connect (G_OBJECT (selection), "changed",
			  G_CALLBACK (contacts_selection_cb), contacts_data);

	/* Enable multiple select (for delete) */
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);


	/* tool box */
	moko_tool_box = moko_tool_box_new_with_search ();

	moko_paned_window_add_toolbox (MOKO_PANED_WINDOW (ui->main_window), MOKO_TOOL_BOX(moko_tool_box));
	ui->search_entry = (GtkWidget*)moko_tool_box_get_entry (MOKO_TOOL_BOX(moko_tool_box));

	/* FIXME? We seem to have to add these in reverse order at the moment */

	/* groups button */
	widget = GTK_WIDGET (moko_tool_box_add_action_button (MOKO_TOOL_BOX (moko_tool_box)));
	moko_pixmap_button_set_center_stock (MOKO_PIXMAP_BUTTON (widget),
						       "openmoko-action-button-group-icon");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (contacts_groups_pane_show), contacts_data);

	/* history button */
	widget = GTK_WIDGET (moko_tool_box_add_action_button (MOKO_TOOL_BOX (moko_tool_box)));
	moko_pixmap_button_set_center_stock (MOKO_PIXMAP_BUTTON (widget),
						       "openmoko-action-button-history-icon");

	/* edit button */
	widget = GTK_WIDGET (moko_tool_box_add_action_button (MOKO_TOOL_BOX (moko_tool_box)));
	moko_pixmap_button_set_center_stock (MOKO_PIXMAP_BUTTON (widget), GTK_STOCK_EDIT);
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (contacts_edit_cb), contacts_data);

	/* view button */
	widget = GTK_WIDGET (moko_tool_box_add_action_button (MOKO_TOOL_BOX (moko_tool_box)));
	moko_pixmap_button_set_center_stock (MOKO_PIXMAP_BUTTON (widget),
						       "openmoko-action-button-view-icon");
	g_signal_connect (G_OBJECT (widget), "clicked",
			  G_CALLBACK (contacts_view_cb), contacts_data);


	/*** contacts display ***/

	/* note book for switching between view/edit mode */
	ui->main_notebook = gtk_notebook_new ();

	details_pane = moko_scrolled_pane_new ();
	moko_scrolled_pane_pack_with_viewport (MOKO_SCROLLED_PANE (details_pane), GTK_WIDGET(ui->main_notebook));
	moko_paned_window_set_details_pane (MOKO_PANED_WINDOW (ui->main_window), details_pane);

	GTK_WIDGET_UNSET_FLAGS (ui->main_notebook, GTK_CAN_FOCUS);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (ui->main_notebook), FALSE);
	gtk_notebook_set_show_border (GTK_NOTEBOOK (ui->main_notebook), FALSE);

	/*** view mode ****/
	ui->contact_pane = contacts_contact_pane_new();

	g_signal_connect (ui->contact_pane, "fullname-changed", (GCallback) fullname_changed_cb, contacts_data);
	g_signal_connect (ui->contact_pane, "cell-changed", (GCallback) cell_changed_cb, contacts_data);

	contacts_contact_pane_set_editable (CONTACTS_CONTACT_PANE (ui->contact_pane), FALSE);
	/* The book view is set later when we get it back */
	gtk_notebook_append_page (GTK_NOTEBOOK (ui->main_notebook), ui->contact_pane, NULL);


	/*** groups mode ***/
	ui->groups_vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (ui->groups_vbox), 12);
	gtk_notebook_append_page (GTK_NOTEBOOK (ui->main_notebook), ui->groups_vbox, NULL);

	/*** connect signals ***/
	g_signal_connect ((gpointer) ui->main_window, "destroy",
			G_CALLBACK (gtk_main_quit),
			NULL);
	g_signal_connect_data ((gpointer) ui->contacts_treeview, "key_press_event",
			G_CALLBACK (contacts_treeview_search_cb),
			GTK_OBJECT (ui->search_entry),
			NULL, G_CONNECT_AFTER | G_CONNECT_SWAPPED);
	g_signal_connect ((gpointer) ui->search_entry, "changed",
			G_CALLBACK (contacts_search_changed_cb),
			contacts_data);
	g_signal_connect (G_OBJECT (ui->contacts_treeview), "row_activated",
			  G_CALLBACK (contacts_treeview_edit_cb), contacts_data);

	g_signal_connect (G_OBJECT (moko_paned_window_get_menubox (MOKO_PANED_WINDOW (ui->main_window))),
			"filter_changed", G_CALLBACK (moko_filter_changed), contacts_data);
	g_signal_connect (G_OBJECT(moko_tool_box), "searchbox_invisible",
			  G_CALLBACK (contacts_disable_search_cb), contacts_data);
	g_signal_connect (G_OBJECT(moko_tool_box), "searchbox_visible",
			  G_CALLBACK (contacts_enable_search_cb), contacts_data);

	/* temporary settings */
	GtkSettings *settings = gtk_settings_get_default ();
	g_object_set (settings,
			"gtk-theme-name", "openmoko-standard",
			NULL);


	gtk_widget_show_all (ui->main_window);
}

static void
moko_filter_changed (GtkWidget *widget, gchar *text, ContactsData *data)
{
	g_free (data->selected_group);
	data->selected_group = g_strdup (text);
	contacts_update_treeview (data);
}

void
contacts_ui_create (ContactsData *data)
{
	create_main_window (data);
}

static void
remove_menu_item (GtkWidget *menu_item, GtkWidget *menu)
{
	gtk_container_remove (GTK_CONTAINER (menu), menu_item);
}

static void
add_menu_item (gchar *group, GtkMenu *menu)
{
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_menu_item_new_with_label (group));
}

static void
container_remove (GtkWidget *child, GtkWidget *container)
{
	gtk_container_remove (GTK_CONTAINER (container), child);
}

void
contacts_ui_update_groups_list (ContactsData *data)
{

	/* update filter menu */
	gtk_container_foreach (GTK_CONTAINER (filter_menu), (GtkCallback)remove_menu_item, filter_menu);

	gtk_menu_shell_append (GTK_MENU_SHELL (filter_menu),
			gtk_menu_item_new_with_label (_("All")));
	gtk_menu_shell_append (GTK_MENU_SHELL (filter_menu),
			gtk_separator_menu_item_new ());
	g_list_foreach (data->contacts_groups, (GFunc)add_menu_item,
			filter_menu);
	gtk_menu_shell_append (GTK_MENU_SHELL (filter_menu),
			gtk_separator_menu_item_new ());
	gtk_menu_shell_append (GTK_MENU_SHELL (filter_menu),
			gtk_menu_item_new_with_label (_("Search Results")));

	gtk_widget_show_all (GTK_WIDGET (filter_menu));

	/* add group editor checkboxes */
	GtkWidget *widget;
	GList *cur;

	if (g_list_length (data->contacts_groups) == 0)
		gtk_box_pack_start (GTK_BOX (data->ui->groups_vbox), gtk_label_new (_("No Groups")), TRUE, TRUE, 0);
	else if (g_hash_table_size (data->groups_widgets_hash) == 0)
		/* there are groups defined, but no checkboxes yet, so make sure the pane is
		 * empty */
		gtk_container_foreach (GTK_CONTAINER (data->ui->groups_vbox), (GtkCallback) container_remove, data->ui->groups_vbox);

	for (cur = data->contacts_groups; cur; cur = g_list_next (cur))
	{
		if (g_hash_table_lookup (data->groups_widgets_hash, cur->data))
			continue;
		widget = gtk_check_button_new_with_label (cur->data);
		gtk_box_pack_start (GTK_BOX (data->ui->groups_vbox), widget, FALSE, FALSE, 0);
		g_hash_table_insert (data->groups_widgets_hash, cur->data, widget);
		g_signal_connect (G_OBJECT (widget), "toggled", G_CALLBACK (groups_checkbutton_cb), data);
		gtk_widget_show (widget);
	}


}


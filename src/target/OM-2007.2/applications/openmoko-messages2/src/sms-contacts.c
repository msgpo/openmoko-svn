/*
 *  openmoko-messages -- OpenMoko SMS Application
 *
 *  Authored by Chris Lord <chris@openedhand.com>
 *
 *  Copyright (C) 2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include "sms-contacts.h"
#include <libmokoui2/moko-finger-scroll.h>
#include <libmokoui2/moko-search-bar.h>
#include <config.h>

/* Following two functions taken from pimlico Contacts */
static void
contact_photo_size (GdkPixbufLoader * loader, gint width, gint height,
		    gpointer user_data)
{
	/* Max height of GTK_ICON_SIZE_DIALOG */
	gint iconwidth, iconheight;
	gtk_icon_size_lookup (GTK_ICON_SIZE_DIALOG, &iconwidth, &iconheight);
	
	gdk_pixbuf_loader_set_size (loader,
				    width / ((gdouble) height /
					     iconheight), iconheight);
}

GdkPixbuf *
contacts_load_photo (EContact *contact)
{
	EContactPhoto *photo;
	GdkPixbuf *pixbuf = NULL;
	
	/* Retrieve contact picture and resize */
	photo = e_contact_get (contact, E_CONTACT_PHOTO);
	if (photo) {
		GdkPixbufLoader *loader = gdk_pixbuf_loader_new ();
		if (loader) {
			g_signal_connect (G_OBJECT (loader),
					  "size-prepared",
					  G_CALLBACK (contact_photo_size),
					  NULL);
#if HAVE_PHOTO_TYPE
			switch (photo->type) {
			case E_CONTACT_PHOTO_TYPE_INLINED :
				gdk_pixbuf_loader_write (loader,
					photo->data.inlined.data,
					photo->data.inlined.length, NULL);
				break;
			case E_CONTACT_PHOTO_TYPE_URI :
			default :
				g_warning ("Cannot handle URI photos yet");
				g_object_unref (loader);
				loader = NULL;
				break;
			}
#else
			gdk_pixbuf_loader_write (loader, (const guchar *)
				photo->data, photo->length, NULL);
#endif
			if (loader) {
				gdk_pixbuf_loader_close (loader, NULL);
				pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
				if (pixbuf) g_object_ref (pixbuf);
				g_object_unref (loader);
			}
		}
		e_contact_photo_free (photo);
	}
	
	return pixbuf;
}

static void
contacts_store (SmsData *data, GtkTreeIter *iter, EContact *contact)
{
	GdkPixbuf *photo = contacts_load_photo (contact);
	gtk_list_store_set ((GtkListStore *)data->contacts_store, iter,
		COL_UID, e_contact_get_const (contact, E_CONTACT_UID),
		COL_NAME, e_contact_get_const (contact, E_CONTACT_FULL_NAME),
		COL_ICON, photo, -1);
	if (photo) g_object_unref (photo);
}

static void
contacts_added_cb (EBookView *ebookview, GList *contacts, SmsData *data)
{
	for (; contacts; contacts = contacts->next) {
		GtkTreeIter *iter;
		EContact *contact = (EContact *)contacts->data;
		
		if (!contact) continue;
		
		iter = g_slice_new (GtkTreeIter);
		gtk_list_store_append ((GtkListStore *)data->contacts_store,
			iter);
		contacts_store (data, iter, contact);
		g_hash_table_insert (data->contacts,
			e_contact_get (contact, E_CONTACT_UID), iter);
	}
}

static void
contacts_changed_cb (EBookView *ebookview, GList *contacts, SmsData *data)
{
	for (; contacts; contacts = contacts->next) {
		GtkTreeIter *iter;
		const gchar *uid;
		
		EContact *contact = (EContact *)contacts->data;
		
		if (!contact) continue;
		
		uid = e_contact_get_const (contact, E_CONTACT_UID);
		iter = g_hash_table_lookup (data->contacts, uid);
		if (iter) contacts_store (data, iter, contact);
	}
}

static void
contacts_removed_cb (EBookView *ebookview, GList *uids, SmsData *data)
{
	for (; uids; uids = uids->next) {
		GtkTreeIter *iter = g_hash_table_lookup (
			data->contacts, uids->data);

		if (!iter) continue;
		
		gtk_list_store_remove ((GtkListStore *)
			data->contacts_store, iter);
		g_hash_table_remove (data->contacts, uids->data);
	}
}

static void
free_iter_slice (GtkTreeIter *iter)
{
	g_slice_free (GtkTreeIter, iter);
}

static void
nophoto_filter_func (GtkTreeModel *model, GtkTreeIter *iter, GValue *value,
		     gint column, SmsData *data)
{
	GtkTreeIter real_iter;
	gpointer pointer;
	
	gtk_tree_model_filter_convert_iter_to_child_iter (
	     (GtkTreeModelFilter *)model, &real_iter, iter);
	
	gtk_tree_model_get (data->contacts_store, &real_iter,
		column, &pointer, -1);
	switch (column) {
	    case COL_UID :
	    case COL_NAME :
	    case COL_DETAIL :
		g_value_take_string (value, pointer);
		break;
	    case COL_ICON :
		if (pointer)
			g_value_take_object (value, pointer);
		else
			g_value_set_object (value, data->no_photo);
		break;
	}
}

GtkWidget *
sms_contacts_page_new (SmsData *data)
{
	EBookQuery *qrys[(E_CONTACT_LAST_PHONE_ID-E_CONTACT_FIRST_PHONE_ID)+1];
	GtkWidget *searchbar, *scroll, *vbox;
	GtkCellRenderer *renderer;
	EBookQuery *tel_query;
	EBookView *view;
	gint i;

	GError *error = NULL;
	
	/* Create query for all contacts with telephone numbers */
	/* FIXME: This query doesn't seem to work? */
	/*for (i = E_CONTACT_FIRST_PHONE_ID; i <= E_CONTACT_LAST_PHONE_ID; i++) {
		qrys[i - E_CONTACT_FIRST_PHONE_ID] =
			e_book_query_field_exists ((EContactField)i);
	}
	i -= E_CONTACT_FIRST_PHONE_ID;
	if (!(tel_query = e_book_query_or (i, qrys, TRUE))) {
		g_warning ("Error creating query");
		return gtk_label_new ("Error creating query");
	}*/
	tel_query = e_book_query_any_field_contains (NULL);
	
	/* Create/retrieve and open system address book */
	/* TODO: async opening? */
	if (!(data->ebook = e_book_new_system_addressbook (&error))) {
		g_warning ("Error retrieving system addressbook: %s",
			error->message);
		g_error_free (error);
		return gtk_label_new ("Error, see console output");
	}
	
	if (!e_book_open (data->ebook, FALSE, &error)) {
		g_warning ("Error opening system addressbook: %s",
			error->message);
		g_error_free (error);
		return gtk_label_new ("Error, see console output");
	}
	
	/* Get view on telephone number query */
	if (!(e_book_get_book_view (data->ebook, tel_query,
	      NULL, 0, &view, &error))) {
		g_warning ("Error retrieving addressbook view: %s",
			error->message);
		g_error_free (error);
		return gtk_label_new ("Error, see console output");
	}
	
	/* Get icon to use when no contact photo exists */
	data->no_photo = gtk_icon_theme_load_icon (
		gtk_icon_theme_get_default (), "stock_person", 48, 0, NULL);

	/* Create contacts model */
	data->contacts_store = gtk_list_store_new (COL_LAST,
		G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, GDK_TYPE_PIXBUF);
	data->contacts = g_hash_table_new_full (g_str_hash, g_str_equal,
		(GDestroyNotify)g_free, (GDestroyNotify)free_iter_slice);
	
	/* Create filter */
	data->contacts_filter = gtk_tree_model_filter_new (
		data->contacts_store, NULL);
	gtk_tree_model_filter_set_modify_func ((GtkTreeModelFilter *)
		data->contacts_filter, COL_LAST,
		(GType []){G_TYPE_STRING, G_TYPE_STRING,
			G_TYPE_STRING, GDK_TYPE_PIXBUF},
		(GtkTreeModelFilterModifyFunc)nophoto_filter_func, data, NULL);
	
	/* Create groups model */
	data->contacts_combo = gtk_combo_box_new_text ();
	
	/* Create search box */
	searchbar = moko_search_bar_new_with_combo (
		GTK_COMBO_BOX (data->contacts_combo));
	
	/* Create tree view */
	data->contacts_treeview = gtk_tree_view_new_with_model (
		data->contacts_filter);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (
		data->contacts_treeview), TRUE);
	gtk_tree_view_set_headers_visible (
		GTK_TREE_VIEW (data->contacts_treeview), FALSE);
	
	/* Create renderer and column */
	/* Slight abuse of the note cell renderer I suppose... */
	renderer = jana_gtk_cell_renderer_note_new ();
	g_object_set (G_OBJECT (renderer), "show_created", FALSE,
		"show_recipient", FALSE, NULL);
	
	gtk_tree_view_insert_column_with_attributes (
		GTK_TREE_VIEW (data->contacts_treeview),
		0, NULL, renderer, "author", COL_NAME,
		"body", COL_DETAIL, "icon", COL_ICON, NULL);
	
	g_signal_connect (data->contacts_treeview, "size-allocate",
		G_CALLBACK (jana_gtk_utils_treeview_resize), renderer);

	/* Pack treeview into a finger-scroll */
	scroll = moko_finger_scroll_new ();
	gtk_container_add (GTK_CONTAINER (scroll), data->contacts_treeview);
	
	/* Pack widgets into vbox and return */
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), searchbar, FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);
	gtk_widget_show_all (vbox);
	
	/* Start book view */
	g_signal_connect (view, "contacts-added",
		G_CALLBACK (contacts_added_cb), data);
	g_signal_connect (view, "contacts-changed",
		G_CALLBACK (contacts_changed_cb), data);
	g_signal_connect (view, "contacts-removed",
		G_CALLBACK (contacts_removed_cb), data);
	e_book_view_start (view);
	
	return vbox;
}


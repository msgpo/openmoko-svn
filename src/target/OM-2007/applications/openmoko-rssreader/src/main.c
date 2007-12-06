/*
 *  RSS Reader, a simple RSS reader
 *
 *  Copyright (C) 2007 Holger Hans Peter Freyther
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */
#define _GNU_SOURCE
#include "config.h"
#include <glib/gi18n.h>

#include "application-data.h"
#include "callbacks.h"
#include "rfcdate.h"

#include <libmokoui/moko-scrolled-pane.h>

#include <webkitgtkpage.h>
#include <webkitgtkglobal.h>

#include <string.h>
#include <assert.h>

#define ASSERT_X(x, error) assert(x)

/*
 * filter categories and such terms
 */
static gboolean
rss_filter_entries (GtkTreeModel *model, GtkTreeIter *iter, struct RSSReaderData *data)
{
    /*
     * filter the category
     */
    if ( !data->is_all_filter ) {
        gchar *category;
        gtk_tree_model_get (model, iter,  RSS_READER_COLUMN_CATEGORY, &category,  -1);

        /*
         * how does this happen?
         */
        if ( !category )
            return FALSE;

        if ( strcmp(category, data->current_filter) != 0 )
            return FALSE;

        g_free (category);
    }


    /*
     * filter the text according to the search now
     */
    if ( data->current_search_text ) {
        gchar *text;

        #define FILTER_SEARCH(column)                                      \
        gtk_tree_model_get (model, iter, column, &text, -1);               \
        if ( text && strcasestr (text, data->current_search_text) != NULL ) { \
            g_free (text);                                                 \
            return TRUE;                                                   \
        }

        FILTER_SEARCH(RSS_READER_COLUMN_AUTHOR)
        FILTER_SEARCH(RSS_READER_COLUMN_SUBJECT)
        FILTER_SEARCH(RSS_READER_COLUMN_SOURCE)
        FILTER_SEARCH(RSS_READER_COLUMN_LINK)
        FILTER_SEARCH(RSS_READER_COLUMN_TEXT)

        #undef FILTER_SEARCH
        return FALSE;
    }

    return TRUE;
}

/*
 * sort the dates according to zsort. Ideally they should sort ascending
 */
static gint
rss_sort_dates (GtkTreeModel *model, GtkTreeIter *_left, GtkTreeIter *_right, gpointer that)
{
    RSSRFCDate *left, *right;
    gtk_tree_model_get (model, _left,  RSS_READER_COLUMN_DATE, &left,  -1);
    gtk_tree_model_get (model, _right, RSS_READER_COLUMN_DATE, &right, -1);

    int result;
    if ( left == NULL )
        result = -1;
    else if ( right == NULL )
        result = 1;
    else
        result = rss_rfc_date_compare (left, right);

    if ( left )
        g_object_unref (left);
    if ( right )
        g_object_unref (right);

    return result;
}

static void
rss_cell_data_func (GtkTreeViewColumn *tree_column, GtkCellRenderer *renderer, GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
    RSSRFCDate *date;
    gtk_tree_model_get (tree_model, iter, RSS_READER_COLUMN_DATE, &date, -1);

    g_assert (date);
    g_object_set ( G_OBJECT(renderer), "text", rss_rfc_date_as_string(date), NULL);
    g_object_unref (G_OBJECT(date));
}

/*
 * setup the toolbar
 */
static void setup_toolbar( struct RSSReaderData *data ) {
    GtkButton *a;
    GtkWidget *anImage;
    data->box = MOKO_TOOL_BOX(moko_tool_box_new_with_search());
    gtk_widget_grab_focus( GTK_WIDGET(data->box) );
    g_signal_connect( G_OBJECT(data->box), "searchbox_visible",   G_CALLBACK(cb_searchbox_visible), data );
    g_signal_connect( G_OBJECT(data->box), "searchbox_invisible", G_CALLBACK(cb_searchbox_invisible), data );


    a = GTK_BUTTON(moko_tool_box_add_action_button( MOKO_TOOL_BOX(data->box) ));
    anImage = gtk_image_new_from_file( PKGDATADIR "/rssreader_refresh_all.png" );
    moko_pixmap_button_set_center_image( MOKO_PIXMAP_BUTTON(a), anImage );
    g_signal_connect( G_OBJECT(a), "clicked", G_CALLBACK(cb_refresh_all_button_clicked), data );

    a = GTK_BUTTON(moko_tool_box_add_action_button( MOKO_TOOL_BOX(data->box) ));
    anImage = gtk_image_new_from_file( PKGDATADIR "/rssreader_subscribe.png" );
    moko_pixmap_button_set_center_image( MOKO_PIXMAP_BUTTON(a), anImage );
    g_signal_connect( G_OBJECT(a), "clicked", G_CALLBACK(cb_subscribe_button_clicked), data );

    a = GTK_BUTTON(moko_tool_box_add_action_button( MOKO_TOOL_BOX(data->box)) );
    gtk_button_set_label( GTK_BUTTON(a), _("Up for rent") );
    a = GTK_BUTTON(moko_tool_box_add_action_button( MOKO_TOOL_BOX(data->box)) );
    gtk_button_set_label( GTK_BUTTON(a), _("Buy more Mate") );

    moko_paned_window_add_toolbox( MOKO_PANED_WINDOW(data->window), MOKO_TOOL_BOX(data->box) );
}

static void create_navigaton_area( struct RSSReaderData *data ) {
    data->feed_data = gtk_list_store_new( RSS_READER_NUM_COLS,
                                          G_TYPE_STRING /* Author    */,
                                          G_TYPE_STRING /* Subject   */,
                                          RSS_TYPE_RFC_DATE /* Date  */,
                                          G_TYPE_STRING /* Link      */,
                                          G_TYPE_STRING /* Text      */,
                                          G_TYPE_INT    /* Text_Type */,
                                          G_TYPE_STRING /* Category  */,
                                          G_TYPE_STRING /* Source    */ );

    /*
     * allow to filter for a search string
     */
    data->filter_model = GTK_TREE_MODEL_FILTER(gtk_tree_model_filter_new(GTK_TREE_MODEL(data->feed_data),NULL));
    gtk_tree_model_filter_set_visible_func (data->filter_model, (GtkTreeModelFilterVisibleFunc)rss_filter_entries, data, NULL);

    /*
     * Allow sorting of the base model
     */
    data->sort_model = GTK_TREE_MODEL_SORT(gtk_tree_model_sort_new_with_model( GTK_TREE_MODEL(data->filter_model) ));
    gtk_tree_sortable_set_sort_column_id( GTK_TREE_SORTABLE(data->sort_model), RSS_READER_COLUMN_DATE,    GTK_SORT_DESCENDING );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE(data->sort_model), RSS_READER_COLUMN_DATE, rss_sort_dates, NULL, NULL);

    data->treeView = MOKO_TREE_VIEW(moko_tree_view_new_with_model(GTK_TREE_MODEL(data->sort_model)));
    moko_paned_window_set_navigation_pane( MOKO_PANED_WINDOW(data->window), GTK_WIDGET(moko_tree_view_put_into_scrolled_window(data->treeView)) );

    /*
     * Only show the SUBJECT and DATE header
     */
    GtkCellRenderer *ren;
    GtkTreeViewColumn *column;
    ren = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
    column = GTK_TREE_VIEW_COLUMN(gtk_tree_view_column_new_with_attributes( _("Subject"), ren, "text", RSS_READER_COLUMN_SUBJECT, NULL));
    gtk_tree_view_column_set_expand( column, TRUE );
    gtk_tree_view_column_set_sizing( column, GTK_TREE_VIEW_COLUMN_FIXED );
    gtk_tree_view_column_set_sort_column_id( column, RSS_READER_COLUMN_SUBJECT );
    moko_tree_view_append_column( MOKO_TREE_VIEW(data->treeView), column );

    ren = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
    column = GTK_TREE_VIEW_COLUMN(gtk_tree_view_column_new_with_attributes( _("Date"), ren, NULL));

    gtk_tree_view_column_set_cell_data_func (column, ren, rss_cell_data_func, NULL, NULL);
    gtk_tree_view_column_set_expand( column, TRUE );
    gtk_tree_view_column_set_sizing( column, GTK_TREE_VIEW_COLUMN_FIXED );
    gtk_tree_view_column_set_sort_column_id( column, RSS_READER_COLUMN_DATE );
    moko_tree_view_append_column( MOKO_TREE_VIEW(data->treeView), column );

    /*
     * auto completion and selection updates
     */
    GtkTreeSelection *selection = GTK_TREE_SELECTION(gtk_tree_view_get_selection( GTK_TREE_VIEW(data->treeView) ));
    g_signal_connect( G_OBJECT(selection), "changed", G_CALLBACK(cb_treeview_selection_changed), data );

    GtkWidget *search_entry = GTK_WIDGET(moko_tool_box_get_entry(MOKO_TOOL_BOX(data->box)));
    g_signal_connect( G_OBJECT(data->treeView), "key_press_event", G_CALLBACK(cb_treeview_keypress_event), data );
    g_signal_connect( G_OBJECT(search_entry),   "changed", G_CALLBACK(cb_search_entry_changed), data );
}

static void create_details_area( struct RSSReaderData* data ) {
    data->textPage = WEBKIT_GTK_PAGE(webkit_gtk_page_new ());

    GtkWidget *scrollWindow = GTK_WIDGET(moko_scrolled_pane_new());
    moko_scrolled_pane_pack (MOKO_SCROLLED_PANE(scrollWindow), GTK_WIDGET (data->textPage));
    moko_paned_window_set_details_pane( MOKO_PANED_WINDOW(data->window), scrollWindow ) ;
}

/*
 * create the mainwindow
 */
static void setup_ui( struct RSSReaderData* data ) {
    data->window = MOKO_PANED_WINDOW(moko_paned_window_new());
    g_signal_connect( G_OBJECT(data->window), "delete_event", G_CALLBACK( gtk_main_quit ), NULL );

    /*
     * menu
     */
    data->menu = GTK_MENU(gtk_menu_new());
    GtkMenuItem *closeitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( _("Close")));
    g_signal_connect( G_OBJECT(closeitem), "activate", G_CALLBACK(gtk_main_quit), NULL );
    gtk_menu_shell_append( GTK_MENU_SHELL(data->menu), GTK_WIDGET(closeitem) );
    moko_paned_window_set_application_menu( MOKO_PANED_WINDOW(data->window), GTK_MENU(data->menu) );

    /*
     * filter
     */
    data->filter = GTK_MENU(gtk_menu_new());
    moko_paned_window_set_filter_menu( MOKO_PANED_WINDOW(data->window), GTK_MENU(data->filter) );
    data->menubox = MOKO_MENU_BOX(moko_paned_window_get_menubox( MOKO_PANED_WINDOW(data->window) ) );
    g_signal_connect( G_OBJECT(data->menubox), "filter_changed", G_CALLBACK(cb_filter_changed), data );


    /*
     * tool bar
     */
    setup_toolbar( data );
    create_navigaton_area( data );
    create_details_area( data );
}

int main( int argc, char** argv )
{
    /*
     * boiler plate code
     */
    g_debug( "openmoko-rssreader starting up" );

    /* i18n boiler plate */
    bindtextdomain ( GETTEXT_PACKAGE, RSSREADER_LOCALE_DIR );
    bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
    textdomain ( GETTEXT_PACKAGE );


    /*
     * initialize threads for fetching the RSS in the background
     */
    g_thread_init( NULL );
    gdk_threads_init();
    gdk_threads_enter();
    gtk_init( &argc, &argv );
    webkit_gtk_init ();

    struct RSSReaderData *data = g_new0( struct RSSReaderData, 1 );


    data->app = MOKO_APPLICATION( moko_application_get_instance() );
    g_set_application_name( _("FeedReader") );
    data->cache = MOKO_CACHE(moko_cache_new ("rss-reader"));

    setup_ui( data );

    /*
     * load data
     */
    data->is_all_filter = TRUE;
    refresh_categories( data );
    load_data_from_cache (data);
    moko_menu_box_set_active_filter( data->menubox, _("All") );

    gtk_widget_show_all( GTK_WIDGET(data->window) );
    gtk_widget_grab_focus (GTK_WIDGET(data->treeView));
    gtk_main();
    gdk_threads_leave();

    return 0;
}


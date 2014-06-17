/*
 * freeacq is the legal property of Víctor Enríquez Miguel. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */
#include <gtk/gtk.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqi18n.h"
#include "facqstreamview.h"

/*
 * GTK_STOCK_NEW -> GTK_STOCK_DISCONNECT -> GTK_STOCK_MEDIA_CONNECT -> 
 * GTK_STOCK_MEDIA_PLAY <-> GTK_STOCK_MEDIA_STOP
 *
 * No stream loaded: GTK_STOCK_NEW, all disabled, new enabled, open enabled,
 * preferences enabled, close disabled. "Create a new stream, or open
 * an existing stream from a file."
 *
 * New stream: GTK_STOCK_DISCONNECT, add enabled. preferences disabled "You must add a source,
 * optionally some operations, and a sink."
 *
 * Source added: GTK_STOCK_DISCONNECT, add enabled, remove enabled, clear
 * enabled. "You must add a source, optionally some operations, and a sink".
 *
 * Sink added: GTK_STOCK_CONNECT, disable add, remove enable, clear enabled,
 * play enabled. "Stream is ready to be started, press play to start the
 * acquisition."
 *
 * Play pushed: GTK_STOCK_MEDIA_PLAY, disable add, disable remove, disable
 * clear, disable play, enable stop. "Data acquisition is in progress, press
 * stop when desired"
 *
 * Play pushed with error: GTK_STOCK_STOP, disable stop, enable play, enable
 * remove, enable clear, disable add. "Some error happened while running the
 * stream."
 *
 * Stop pushed: GTK_STOCK_MEDIA_STOP, disable stop, enable play, enable remove,
 * enable clear, disable add. "Stream stopped, press play if you want to restart
 * the acquisition, clear to clear the stream, or remove to delete the latest
 * added item"
 */ 

/**
 * SECTION:facqstreamview
 * @short_description: Provides graphical items for viewing the state and items
 * of a stream.
 * @include:facqstreamview.h
 *
 * Provides graphical items for viewing the state and item of a stream.
 * Includes a top widget composed of a #GtkImage and a #GtkLabel for describing
 * the current state of the stream, and a table where the name and the
 * description of the different items added to the stream can be seen.
 * The table is a #GtkTreeView inside a #GtkScrolledWindow inside a #GtkFrame.
 * The model used for putting the data into the #GtkTreeView is a #GtkListStore.
 *
 * To create a new #FacqStreamView object you should use facq_stream_view_new(),
 * and for adding it to your application facq_stream_view_get_widget().
 *
 * The table shows the type of item, source, operation or sink, the name of the
 * item, and the description of the item. You can control this table with the
 * facq_stream_view_push_item() and facq_stream_view_pop_item() functions, or
 * clear the table with facq_stream_view_clear_data().
 *
 * In the top of the widget an image
 * shows the current state of the stream along a description guiding the user
 * in the next steps. You can change the image and the text according to the
 * current stream state with facq_stream_view_set_status().
 *
 * Finally to destroy the #FacqStreamView use facq_stream_view_free().
 */

/**
 * FacqStreamView:
 *
 * Hides internal details of the #FacqStreamView objects.
 */

/**
 * FacqStreamViewClass:
 *
 * Class for the #FacqStreamView objects.
 */

/**
 * FacqStreamViewStatus:
 * @FACQ_STREAM_VIEW_STATUS_NO_STREAM: No stream has been created.
 * @FACQ_STREAM_VIEW_STATUS_NEW_STREAM: An emptry stream has been created.
 * @FACQ_STREAM_VIEW_STATUS_WITH_SOURCE: The source has been added to the
 * stream.
 * @FACQ_STREAM_VIEW_STATUS_WITH_SINK: The sink has been added to the stream.
 * @FACQ_STREAM_VIEW_STATUS_PLAY: The stream is running.
 * @FACQ_STREAM_VIEW_STATUS_STOP: The stream is in stopped state.
 * @FACQ_STREAM_VIEW_STATUS_ERROR: Some error happened while running the stream.
 *
 * Enumerates the different states that a stream can have.
 */

/**
 * FacqStreamViewItemType:
 * @FACQ_STREAM_VIEW_ITEM_TYPE_SOURCE: The item is a source.
 * @FACQ_STREAM_VIEW_ITEM_TYPE_OPERATION: The item is an operation.
 * @FACQ_STREAM_VIEW_ITEM_TYPE_SINK: The item is a sink.
 *
 * Enumerates the different kind of items.
 */

G_DEFINE_TYPE(FacqStreamView,facq_stream_view,G_TYPE_OBJECT);

enum {
	PROP_0,
};

enum {
	TYPE_COLUMN,
	NAME_COLUMN,
	DETAILS_COLUMN,
	N_COLUMNS
};

struct _FacqStreamViewPrivate {
	GtkListStore *store;
	GtkWidget *list;
	GtkWidget *scroll_window;
	GtkWidget *image;
	GtkWidget *label;
#if GTK_MAJOR_VERSION > 2
	GtkWidget *grid;
#else
	GtkWidget *vbox;
#endif
	GtkWidget *frame;
	guint n_items;
};

/*****--- GObject magic ---*****/
static void facq_stream_view_constructed(GObject *self)
{
	FacqStreamView *view = FACQ_STREAM_VIEW(self);
	GtkCellRenderer *renderer = NULL;
	GtkTreeViewColumn *column = NULL;

	view->priv->store = gtk_list_store_new(N_COLUMNS,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);
	view->priv->list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(view->priv->store));
	view->priv->scroll_window = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(view->priv->scroll_window),
									GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(view->priv->scroll_window),
						GTK_POLICY_AUTOMATIC,
							GTK_POLICY_AUTOMATIC);

	gtk_container_add(GTK_CONTAINER(view->priv->scroll_window),view->priv->list);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Type"),renderer,"text",TYPE_COLUMN,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view->priv->list),column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Name"),renderer,"text",NAME_COLUMN,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view->priv->list),column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Details"),renderer,"text",DETAILS_COLUMN,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view->priv->list),column);

	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(view->priv->list));
	
	gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(view->priv->list),
				GTK_TREE_VIEW_GRID_LINES_BOTH);
#if GTK_MAJOR_VERSION > 2
	view->priv->grid = gtk_grid_new();
#else
	view->priv->vbox = gtk_vbox_new(FALSE,0);
#endif
#if GTK_MAJOR_VERSION > 2
	view->priv->image = gtk_image_new_from_icon_name("document-new",GTK_ICON_SIZE_DIALOG);
#else
	view->priv->image = gtk_image_new_from_stock(GTK_STOCK_NEW,GTK_ICON_SIZE_DIALOG);
#endif
	view->priv->frame = gtk_frame_new(_("Stream details:"));
	gtk_frame_set_shadow_type(GTK_FRAME(view->priv->frame),GTK_SHADOW_NONE);
	gtk_container_add(GTK_CONTAINER(view->priv->frame),view->priv->scroll_window);
	view->priv->label = gtk_label_new(_("Create a new stream, or open an existing stream from a file."));
	gtk_misc_set_alignment(GTK_MISC(view->priv->label),0.5,0.0);
	gtk_label_set_justify(GTK_LABEL(view->priv->label),GTK_JUSTIFY_LEFT);
	gtk_widget_set_size_request(view->priv->label,256,-1);
	gtk_label_set_line_wrap(GTK_LABEL(view->priv->label),TRUE);
	gtk_label_set_line_wrap_mode(GTK_LABEL(view->priv->label),PANGO_WRAP_WORD);
#if GTK_MAJOR_VERSION > 2
	gtk_grid_attach(GTK_GRID(view->priv->grid),view->priv->label,0,0,1,1);
	gtk_grid_attach(GTK_GRID(view->priv->grid),view->priv->image,0,1,1,1);
	gtk_grid_attach(GTK_GRID(view->priv->grid),view->priv->frame,0,2,1,1);
#else
	gtk_box_pack_start(GTK_BOX(view->priv->vbox),view->priv->label,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(view->priv->vbox),view->priv->image,FALSE,FALSE,3);
	gtk_box_pack_end(GTK_BOX(view->priv->vbox),view->priv->frame,TRUE,TRUE,0);
#endif
}

static void facq_stream_view_finalize(GObject *self)
{
	FacqStreamView *view = FACQ_STREAM_VIEW(self);

	if(GTK_IS_WIDGET(view->priv->image))
		gtk_widget_destroy(view->priv->image);

	if(GTK_IS_WIDGET(view->priv->label))
		gtk_widget_destroy(view->priv->label);

	if(GTK_IS_WIDGET(view->priv->list))
		gtk_widget_destroy(view->priv->list);

	if(GTK_IS_WIDGET(view->priv->scroll_window))
		gtk_widget_destroy(view->priv->scroll_window);

#if GTK_MAJOR_VERSION > 2
	if(GTK_IS_WIDGET(view->priv->grid))
		gtk_widget_destroy(view->priv->grid);
#else
	if(GTK_IS_WIDGET(view->priv->vbox))
		gtk_widget_destroy(view->priv->vbox);
#endif

	if(GTK_IS_WIDGET(view->priv->frame))
		gtk_widget_destroy(view->priv->frame);

	if(G_IS_OBJECT(view->priv->store))
		g_object_unref(G_OBJECT(view->priv->store));

	G_OBJECT_CLASS(facq_stream_view_parent_class)->finalize(self);
}

static void facq_stream_view_class_init(FacqStreamViewClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqStreamViewPrivate));

        object_class->constructed = facq_stream_view_constructed;
        object_class->finalize = facq_stream_view_finalize;
}

static void facq_stream_view_init(FacqStreamView *view)
{
	view->priv = G_TYPE_INSTANCE_GET_PRIVATE(view,FACQ_TYPE_STREAM_VIEW,FacqStreamViewPrivate);
	view->priv->n_items = 0;
}

static const gchar *item_type_to_human(FacqStreamViewItemType type)
{
	const gchar * const item_types[] = {
		N_(FACQ_ITEM_TYPE_SOURCE),
		N_(FACQ_ITEM_TYPE_OPERATION),
		N_(FACQ_ITEM_TYPE_SINK)
	};
	const gchar *ret = NULL;
	if(type < FACQ_STREAM_VIEW_ITEM_TYPE_N)
		ret = _(item_types[type]);
	return ret;
}

/****--- public methods ---*****/
/**
 * facq_stream_view_new:
 *
 * Creates a new #FacqStreamView object.
 *
 * Returns: A new #FacqStreamView object.
 */
FacqStreamView *facq_stream_view_new(void)
{
	return g_object_new(FACQ_TYPE_STREAM_VIEW,NULL);
}

/**
 * facq_stream_view_get_widget:
 * @view: A #FacqStreamView object.
 *
 * Returns the #FacqStreamView top widget.
 *
 * Returns: A #GtkWidget that you can add to your application.
 */
GtkWidget *facq_stream_view_get_widget(const FacqStreamView *view)
{
	g_return_val_if_fail(FACQ_IS_STREAM_VIEW(view),NULL);
#if GTK_MAJOR_VERSION > 2
	return view->priv->grid;
#else
	return view->priv->vbox;
#endif
}

/**
 * facq_stream_view_set_status:
 * @view: A #FacqStreamView object.
 * @status: A #FacqStreamViewStatus valid value.
 *
 * Sets the status of the #FacqStreamView object, changing the
 * icon in the top #GtkImage and the label.
 *
 */
void facq_stream_view_set_status(FacqStreamView *view,FacqStreamViewStatus status)
{
	g_return_if_fail(FACQ_IS_STREAM_VIEW(view));

	switch(status){
	case FACQ_STREAM_VIEW_STATUS_NO_STREAM:
#if GTK_MAJOR_VERSION > 2
		gtk_image_set_from_icon_name(GTK_IMAGE(view->priv->image),
						"document-new",GTK_ICON_SIZE_DIALOG);
#else
		gtk_image_set_from_stock(GTK_IMAGE(view->priv->image),
						GTK_STOCK_NEW,GTK_ICON_SIZE_DIALOG);
#endif
		gtk_label_set_text(GTK_LABEL(view->priv->label),
				_("Create a new stream, or open an existing stream from a file."));
	break;
	case FACQ_STREAM_VIEW_STATUS_NEW_STREAM:
#if GTK_MAJOR_VERSION > 2
		gtk_image_set_from_icon_name(GTK_IMAGE(view->priv->image),
						"network-offline",GTK_ICON_SIZE_DIALOG);
#else
		gtk_image_set_from_stock(GTK_IMAGE(view->priv->image),
						GTK_STOCK_DISCONNECT,GTK_ICON_SIZE_DIALOG);
#endif
		gtk_label_set_text(GTK_LABEL(view->priv->label),
				_("You must add a source, optionally some operations, and a sink."));
	break;
	case FACQ_STREAM_VIEW_STATUS_WITH_SOURCE:
#if GTK_MAJOR_VERSION > 2
		gtk_image_set_from_icon_name(GTK_IMAGE(view->priv->image),
						"network-offline",GTK_ICON_SIZE_DIALOG);
#else
		gtk_image_set_from_stock(GTK_IMAGE(view->priv->image),
						GTK_STOCK_DISCONNECT,GTK_ICON_SIZE_DIALOG);
#endif
		gtk_label_set_text(GTK_LABEL(view->priv->label),
				_("You must add a source, optionally some operations, and a sink."));
	break;
	case FACQ_STREAM_VIEW_STATUS_WITH_SINK:
#if GTK_MAJOR_VERSION > 2
		gtk_image_set_from_icon_name(GTK_IMAGE(view->priv->image),
						"network-wired",GTK_ICON_SIZE_DIALOG);
#else
		gtk_image_set_from_stock(GTK_IMAGE(view->priv->image),
						GTK_STOCK_CONNECT,GTK_ICON_SIZE_DIALOG);
#endif
		gtk_label_set_text(GTK_LABEL(view->priv->label),
				_("Stream is ready to be started, press play to start the acquisition."));
	break;
	case FACQ_STREAM_VIEW_STATUS_PLAY:
#if GTK_MAJOR_VERSION > 2
		gtk_image_set_from_icon_name(GTK_IMAGE(view->priv->image),
						"media-playback-start",GTK_ICON_SIZE_DIALOG);
#else
		gtk_image_set_from_stock(GTK_IMAGE(view->priv->image),
						GTK_STOCK_MEDIA_PLAY,GTK_ICON_SIZE_DIALOG);
#endif
		gtk_label_set_text(GTK_LABEL(view->priv->label),
				_("Data acquisition is in progress, press stop when desired"));
	break;
	case FACQ_STREAM_VIEW_STATUS_STOP:
#if GTK_MAJOR_VERSION > 2
		gtk_image_set_from_icon_name(GTK_IMAGE(view->priv->image),
						"media-playback-stop",GTK_ICON_SIZE_DIALOG);
#else
		gtk_image_set_from_stock(GTK_IMAGE(view->priv->image),
						GTK_STOCK_MEDIA_STOP,GTK_ICON_SIZE_DIALOG);
#endif
		gtk_label_set_text(GTK_LABEL(view->priv->label),
				_("Stream stopped, press play to start again"));
	break;
	case FACQ_STREAM_VIEW_STATUS_ERROR:
#if GTK_MAJOR_VERSION > 2
		gtk_image_set_from_icon_name(GTK_IMAGE(view->priv->image),
						"process-stop",GTK_ICON_SIZE_DIALOG);
#else
		gtk_image_set_from_stock(GTK_IMAGE(view->priv->image),
						GTK_STOCK_STOP,GTK_ICON_SIZE_DIALOG);
#endif
		gtk_label_set_text(GTK_LABEL(view->priv->label),
				_("Some error happened while running the stream"));
	break;
	default:
		return;
	}
}

/**
 * facq_stream_view_push_item:
 * @view: A #FacqStreamView object.
 * @type: A #FacqStreamViewItemType valid value.
 * @name: The item name.
 * @desc: The item description.
 *
 * Pushes the name and description of an item to the table of the
 * #FacqStreamView object.
 */
void facq_stream_view_push_item(FacqStreamView *view,FacqStreamViewItemType type,const gchar *name,const gchar *desc)
{
	GtkTreeIter iter;
	
	g_return_if_fail(FACQ_IS_STREAM_VIEW(view));

	gtk_list_store_append(view->priv->store,&iter);
	gtk_list_store_set(view->priv->store,&iter,
				TYPE_COLUMN,item_type_to_human(type),
				NAME_COLUMN,name,
				DETAILS_COLUMN,desc,
				-1);
	view->priv->n_items++;
}

/**
 * facq_stream_view_pop_item:
 * @view: A #FacqStreamView object.
 *
 * Removes the latest pushed item from the table if any.
 */
void facq_stream_view_pop_item(FacqStreamView *view)
{
	GtkTreeIter iter;
	gchar *path = NULL;

	g_return_if_fail(FACQ_IS_STREAM_VIEW(view));

	if(view->priv->n_items == 0)
		return;

	path = g_strdup_printf("%u",view->priv->n_items-1);
	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(view->priv->store),&iter,path);
	gtk_list_store_remove(view->priv->store,&iter);
	g_free(path);
	view->priv->n_items--;
}

/**
 * facq_stream_view_clear_data:
 * @view: A #FacqStreamView object.
 *
 * Clears the table showing the items in the stream.
 */
void facq_stream_view_clear_data(FacqStreamView *view)
{
	g_return_if_fail(FACQ_IS_STREAM_VIEW(view));
	
	if(GTK_IS_LIST_STORE(view->priv->store))
		gtk_list_store_clear(view->priv->store);
	
	view->priv->n_items = 0;
}

/**
 * facq_stream_view_free:
 * @view: A #FacqStreamView object.
 *
 * Destroys a no longer needed #FacqStreamView object.
 */
void facq_stream_view_free(FacqStreamView *view)
{
	g_return_if_fail(FACQ_IS_STREAM_VIEW(view));
	g_object_unref(G_OBJECT(view));
}

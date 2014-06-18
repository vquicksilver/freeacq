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
#include "facqi18n.h"
#include "facqstreamdialog.h"

/**
 * SECTION:facqstreamdialog
 * @include:facqstreamdialog.h
 * @short_description: Provides a Gtk dialog for setting the stream name.
 *
 * #FacqStreamDialog provides a Gtk dialog for setting the #FacqStream name.
 * It's used on #FacqCapture, when you press the Stream->New menu entry, for
 * asking the user the #FacqStream name.
 * It simply shows a dialog window with a #GtkEntry where the user can write
 * the #FacqStream name, and two buttons for canceling the operation or for
 * accepting it.
 *
 * For creating a new #FacqStreamDialog use facq_stream_dialog_new(), for
 * displaying the dialog to the user use facq_stream_dialog_run(), for getting
 * the input from the dialog use facq_stream_dialog_get_input(), and for
 * destroying the dialog use facq_stream_dialog_free().
 */

/**
 * FacqStreamDialog:
 *
 * Contains the internal field used by #FacqStreamDialog.
 */

/**
 * FacqStreamDialogClass:
 *
 * The #FacqStreamDialogClass structure.
 */

G_DEFINE_TYPE(FacqStreamDialog,facq_stream_dialog,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_TOP_WINDOW,
	PROP_NAME,
};

struct _FacqStreamDialogPrivate {
	GtkWidget *top_window;
	GtkWidget *dialog;
	GtkWidget *name_entry;
	gchar *name;
};

static void name_icons_callback(GtkEntry *entry,GtkEntryIconPosition icon_pos,GdkEvent *event,gpointer data)
{
	FacqStreamDialog *dialog = FACQ_STREAM_DIALOG(data);
	GtkEntryBuffer *buf = NULL;

	switch(icon_pos){
	case GTK_ENTRY_ICON_PRIMARY:
		buf = gtk_entry_get_buffer(entry);
		gtk_entry_buffer_delete_text(buf,0,-1);
	break;
	default:
		return;
	}
}

static void facq_stream_dialog_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqStreamDialog *dialog = FACQ_STREAM_DIALOG(self);

	switch(property_id){
	case PROP_TOP_WINDOW: dialog->priv->top_window = g_value_get_pointer(value);
	break;
	case PROP_NAME: dialog->priv->name = g_value_dup_string(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(dialog,property_id,pspec);
	}
}

static void facq_stream_dialog_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqStreamDialog *dialog = FACQ_STREAM_DIALOG(self);

	switch(property_id){
	case PROP_TOP_WINDOW: g_value_set_pointer(value,dialog->priv->top_window);
	break;
	case PROP_NAME: g_value_set_string(value,dialog->priv->name);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(dialog,property_id,pspec);
	}
}

static void facq_stream_dialog_constructed(GObject *self)
{
	FacqStreamDialog *dialog = FACQ_STREAM_DIALOG(self);
#if GTK_MAJOR_VERSION > 2
	GtkWidget *grid = NULL, *widget = NULL, *content_area = NULL;
#else
	GtkWidget *vbox = NULL, *table = NULL, *widget = NULL;
#endif


#if GTK_MAJOR_VERSION > 2
	dialog->priv->dialog =
		gtk_dialog_new_with_buttons(_("Stream name"),
				GTK_WINDOW(dialog->priv->top_window),
					GTK_DIALOG_MODAL |
						GTK_DIALOG_DESTROY_WITH_PARENT,
							_("_Cancel"),GTK_RESPONSE_CANCEL,
								_("_OK"),GTK_RESPONSE_OK,NULL);
#else
	dialog->priv->dialog = 
		gtk_dialog_new_with_buttons(_("Stream name"),
				GTK_WINDOW(dialog->priv->top_window),
					GTK_DIALOG_MODAL | 
						GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
								GTK_STOCK_OK,GTK_RESPONSE_OK,NULL);
#endif


#if GTK_MAJOR_VERSION > 2
	grid = gtk_grid_new();
	content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog->priv->dialog));
#else
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog->priv->dialog));
	table = gtk_table_new(1,2,FALSE);
#endif
	
	widget = gtk_label_new(_("Name:"));
	gtk_label_set_justify(GTK_LABEL(widget),GTK_JUSTIFY_LEFT);

#if GTK_MAJOR_VERSION > 2
	gtk_grid_attach(GTK_GRID(grid),widget,0,0,1,1);
#else
	gtk_table_attach_defaults(GTK_TABLE(table),widget,0,1,0,1);
#endif

	widget = gtk_entry_new();
	if(dialog->priv->name)
		gtk_entry_set_text(GTK_ENTRY(widget),dialog->priv->name);
	else
		gtk_entry_set_text(GTK_ENTRY(widget),_("Untitled stream"));

#if GTK_MAJOR_VERSION > 2
	gtk_entry_set_icon_from_icon_name(GTK_ENTRY(widget),GTK_ENTRY_ICON_PRIMARY,"edit-clear");
	gtk_entry_set_icon_from_icon_name(GTK_ENTRY(widget),GTK_ENTRY_ICON_SECONDARY,"edit-delete");
#else
	gtk_entry_set_icon_from_stock(GTK_ENTRY(widget),GTK_ENTRY_ICON_PRIMARY,GTK_STOCK_CLEAR);
	gtk_entry_set_icon_from_stock(GTK_ENTRY(widget),GTK_ENTRY_ICON_SECONDARY,GTK_STOCK_EDIT);
#endif

	gtk_entry_set_icon_activatable(GTK_ENTRY(widget),GTK_ENTRY_ICON_PRIMARY,TRUE);
	gtk_entry_set_icon_activatable(GTK_ENTRY(widget),GTK_ENTRY_ICON_SECONDARY,FALSE);
	gtk_entry_set_icon_sensitive(GTK_ENTRY(widget),GTK_ENTRY_ICON_PRIMARY,TRUE);
	g_signal_connect(widget,"icon-press",
				G_CALLBACK(name_icons_callback),dialog);
	dialog->priv->name_entry = widget;

#if GTK_MAJOR_VERSION > 2
	gtk_grid_attach(GTK_GRID(grid),widget,1,0,1,1);
	gtk_container_add(GTK_CONTAINER(content_area),grid);
#else
	gtk_table_attach_defaults(GTK_TABLE(table),widget,1,2,0,1);
	gtk_widget_show_all(table);
	gtk_container_add(GTK_CONTAINER(vbox),table);
#endif

}

static void facq_stream_dialog_finalize(GObject *self)
{
	FacqStreamDialog *dialog = FACQ_STREAM_DIALOG(self);

	if(GTK_IS_WIDGET(dialog->priv->name_entry))
		gtk_widget_destroy(dialog->priv->name_entry);

	if(GTK_IS_WIDGET(dialog->priv->dialog))
		gtk_widget_destroy(dialog->priv->dialog);

	if(dialog->priv->name)
		g_free(dialog->priv->name);

	G_OBJECT_CLASS(facq_stream_dialog_parent_class)->finalize(self);
}

static void facq_stream_dialog_class_init(FacqStreamDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqStreamDialogPrivate));

	object_class->set_property = facq_stream_dialog_set_property;
	object_class->get_property = facq_stream_dialog_get_property;
	object_class->constructed = facq_stream_dialog_constructed;
	object_class->finalize = facq_stream_dialog_finalize;

	g_object_class_install_property(object_class,PROP_TOP_WINDOW,
					g_param_spec_pointer("top-window",
							     "Top window",
							     "The application top window",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_NAME,
					g_param_spec_string("name",
							     "Name",
							     "The stream default name",
							     "Untitled stream",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

}

static void facq_stream_dialog_init(FacqStreamDialog *dialog)
{
	dialog->priv = G_TYPE_INSTANCE_GET_PRIVATE(dialog,FACQ_TYPE_STREAM_DIALOG,FacqStreamDialogPrivate);
	dialog->priv->top_window = NULL;
	dialog->priv->name = NULL;
	dialog->priv->name_entry = NULL;
	dialog->priv->dialog = NULL;
}

/**
 * facq_stream_dialog_new:
 * @top_window: A pointer to the top level #GtkWindow of the application.
 * @name: The default name for the #FacqStream.
 *
 * Creates a new #FacqStreamDialog object, using @top_window as parent window
 * and @name as the default stream name.
 *
 * Returns: A new #FacqStreamDialog object.
 */
FacqStreamDialog *facq_stream_dialog_new(GtkWidget *top_window,const gchar *name)
{
	return g_object_new(FACQ_TYPE_STREAM_DIALOG,"top-window",top_window,
						  "name",name,
						  NULL);
}

/**
 * facq_stream_dialog_run:
 * @dialog: A #FacqStreamDialog object.
 *
 * Displays the dialog to the user, so the user is able to interact with it.
 * When the dialog is showed the user isn't able to interact with the top level
 * window.
 *
 * Returns: %GTK_RESPONSE_OK if the user pressed the ok button, or
 * %GTK_RESPONSE_CANCEL if the user pressed the cancel button.
 */
gint facq_stream_dialog_run(FacqStreamDialog *dialog)
{
	g_return_val_if_fail(FACQ_IS_STREAM_DIALOG(dialog),-1);

	if( gtk_dialog_run(GTK_DIALOG(dialog->priv->dialog)) != GTK_RESPONSE_OK){
		return GTK_RESPONSE_CANCEL;
	}
	return GTK_RESPONSE_OK;
}

/**
 * facq_stream_dialog_get_input:
 * @dialog: A #FacqStreamDialog object.
 *
 * Gets the name of the stream desired by the user.
 *
 * Returns: The name of the stream or %NULL if any.
 */
gchar *facq_stream_dialog_get_input(const FacqStreamDialog *dialog)
{
	gchar *ret = NULL;
	GtkEntryBuffer *buf = NULL;

	buf = gtk_entry_get_buffer(GTK_ENTRY(dialog->priv->name_entry));
	if(gtk_entry_buffer_get_length(buf)){
		ret = g_strdup(gtk_entry_buffer_get_text(buf));
	} 

	return ret;
}

/**
 * facq_stream_dialog_free:
 * @dialog: A #FacqStreamDialog object.
 *
 * Destroys a no longer needed #FacqStreamDialog object.
 */
void facq_stream_dialog_free(FacqStreamDialog *dialog)
{
	g_return_if_fail(FACQ_IS_STREAM_DIALOG(dialog));
	g_object_unref(G_OBJECT(dialog));
}

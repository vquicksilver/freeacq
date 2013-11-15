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
#include "facqplugdialog.h"

/**
 * SECTION:facqplugdialog
 * @title:FacqPlugDialog:
 * @include:facqplugdialog.h
 * @short_description: A Gtk dialog for changing FacqPlug settings.
 *
 * This module provides a Gtk based dialog for changing the #FacqPlug settings,
 * that is the IP address and port, where the #FacqPlug is listening.
 *
 * For using the dialog first you have to create a new #FacqPlugDialog with
 * facq_plug_dialog_new(), after this you must call facq_plug_dialog_run() for
 * showing the dialog to the user. For getting the user input use
 * facq_plug_dialog_get_input() and finally for destroying it use
 * facq_plug_dialog_free().
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * #FacqPlugDialog uses a #GtkEntry and a #GtkSpinButton along a #GtkDialog.
 * </para>
 * </sect1>
 */

/**
 * FacqPlugDialog:
 *
 * Contains the inner data of the #FacqPlugDialog.
 */

/**
 * FacqPlugDialogClass:
 *
 * The class for the #FacqPlugDialog.
 */

G_DEFINE_TYPE(FacqPlugDialog,facq_plug_dialog,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_TOP_WINDOW,
	PROP_ADDRESS,
	PROP_PORT
};

struct _FacqPlugDialogPrivate {
	GtkWidget *top_window;
	GtkWidget *dialog;
	GtkWidget *address_entry;
	GtkWidget *spin_button;
	gchar *address;
	guint16 port;
};

static void address_icons_callback(GtkEntry *entry,GtkEntryIconPosition icon_pos,GdkEvent *event,gpointer data)
{
	FacqPlugDialog *dialog = FACQ_PLUG_DIALOG(data);
	GtkEntryBuffer *buf = NULL;

	switch(icon_pos){
	case GTK_ENTRY_ICON_PRIMARY:
		buf = gtk_entry_get_buffer(entry);
		gtk_entry_buffer_delete_text(buf,0,-1);
	break;
	case GTK_ENTRY_ICON_SECONDARY:
		gtk_entry_set_text(GTK_ENTRY(dialog->priv->address_entry),dialog->priv->address);
	break;
	default:
		return;
	}
}

static void port_icons_callback(GtkEntry *entry,GtkEntryIconPosition icon_pos,GdkEvent *event,gpointer data)
{
	FacqPlugDialog *dialog = FACQ_PLUG_DIALOG(data);
	GtkEntryBuffer *buf = NULL;

	switch(icon_pos){
	case GTK_ENTRY_ICON_PRIMARY:
		buf = gtk_entry_get_buffer(entry);
		gtk_entry_buffer_delete_text(buf,0,-1);
	break;
	case GTK_ENTRY_ICON_SECONDARY:
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog->priv->spin_button),dialog->priv->port);
	break;
	default:
		return;
	}
}

static void facq_plug_dialog_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqPlugDialog *dialog = FACQ_PLUG_DIALOG(self);

	switch(property_id){
	case PROP_TOP_WINDOW: dialog->priv->top_window = g_value_get_pointer(value);
	break;
	case PROP_ADDRESS: dialog->priv->address = g_value_dup_string(value);
	break;
	case PROP_PORT: dialog->priv->port = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(dialog,property_id,pspec);
	}
}

static void facq_plug_dialog_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqPlugDialog *dialog = FACQ_PLUG_DIALOG(self);

	switch(property_id){
	case PROP_TOP_WINDOW: g_value_set_pointer(value,dialog->priv->top_window);
	break;
	case PROP_ADDRESS: g_value_set_string(value,dialog->priv->address);
	break;
	case PROP_PORT: g_value_set_uint(value,dialog->priv->port);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(dialog,property_id,pspec);
	}
}

static void facq_plug_dialog_constructed(GObject *self)
{
	FacqPlugDialog *dialog = FACQ_PLUG_DIALOG(self);
	GtkWidget *vbox = NULL, *table = NULL, *widget = NULL;

	dialog->priv->dialog = 
		gtk_dialog_new_with_buttons("Plug preferences",
				GTK_WINDOW(dialog->priv->top_window),
					GTK_DIALOG_MODAL | 
						GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
								GTK_STOCK_OK,GTK_RESPONSE_OK,NULL);

	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog->priv->dialog));
	table = gtk_table_new(2,2,FALSE);
	
	widget = gtk_label_new("Address:");
	gtk_label_set_justify(GTK_LABEL(widget),GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table),widget,0,1,0,1);
	
	widget = gtk_entry_new();
	if(dialog->priv->address)
		gtk_entry_set_text(GTK_ENTRY(widget),dialog->priv->address);
	else
		gtk_entry_set_text(GTK_ENTRY(widget),"all");
	gtk_entry_set_icon_from_stock(GTK_ENTRY(widget),GTK_ENTRY_ICON_PRIMARY,GTK_STOCK_CLEAR);
	gtk_entry_set_icon_from_stock(GTK_ENTRY(widget),GTK_ENTRY_ICON_SECONDARY,GTK_STOCK_NETWORK);
	gtk_entry_set_icon_activatable(GTK_ENTRY(widget),GTK_ENTRY_ICON_PRIMARY,TRUE);
	gtk_entry_set_icon_activatable(GTK_ENTRY(widget),GTK_ENTRY_ICON_SECONDARY,TRUE);
	gtk_entry_set_icon_sensitive(GTK_ENTRY(widget),GTK_ENTRY_ICON_PRIMARY,TRUE);
	gtk_entry_set_icon_sensitive(GTK_ENTRY(widget),GTK_ENTRY_ICON_SECONDARY,TRUE);
	g_signal_connect(widget,"icon-press",
				G_CALLBACK(address_icons_callback),dialog);
	dialog->priv->address_entry = widget;
	gtk_table_attach_defaults(GTK_TABLE(table),widget,1,2,0,1);

	widget = gtk_label_new("Port:");
	gtk_label_set_justify(GTK_LABEL(widget),GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table),widget,0,1,1,2);
	
	widget = gtk_spin_button_new_with_range(0,65535,1);
	gtk_entry_set_icon_from_stock(GTK_ENTRY(widget),GTK_ENTRY_ICON_PRIMARY,GTK_STOCK_CLEAR);
	gtk_entry_set_icon_from_stock(GTK_ENTRY(widget),GTK_ENTRY_ICON_SECONDARY,GTK_STOCK_CONNECT);
	gtk_spin_button_set_increments(GTK_SPIN_BUTTON(widget),1,10);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget),0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),dialog->priv->port);
	g_signal_connect(GTK_ENTRY(widget),"icon-press",
				G_CALLBACK(port_icons_callback),dialog);
	dialog->priv->spin_button = widget;
	gtk_table_attach_defaults(GTK_TABLE(table),widget,1,2,1,2);

	gtk_widget_show_all(table);

	gtk_container_add(GTK_CONTAINER(vbox),table);
}

static void facq_plug_dialog_finalize(GObject *self)
{
	FacqPlugDialog *dialog = FACQ_PLUG_DIALOG(self);

	if(GTK_IS_WIDGET(dialog->priv->address_entry))
		gtk_widget_destroy(dialog->priv->address_entry);

	if(GTK_IS_WIDGET(dialog->priv->spin_button))
		gtk_widget_destroy(dialog->priv->spin_button);

	if(GTK_IS_WIDGET(dialog->priv->dialog))
		gtk_widget_destroy(dialog->priv->dialog);

	if(dialog->priv->address)
		g_free(dialog->priv->address);

	G_OBJECT_CLASS(facq_plug_dialog_parent_class)->finalize(self);
}

static void facq_plug_dialog_class_init(FacqPlugDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqPlugDialogPrivate));

	object_class->set_property = facq_plug_dialog_set_property;
	object_class->get_property = facq_plug_dialog_get_property;
	object_class->constructed = facq_plug_dialog_constructed;
	object_class->finalize = facq_plug_dialog_finalize;

	g_object_class_install_property(object_class,PROP_TOP_WINDOW,
					g_param_spec_pointer("top-window",
							     "Top window",
							     "The application top window",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_ADDRESS,
					g_param_spec_string("address",
							     "Address",
							     "The plug default address",
							     "127.0.0.1",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_PORT,
					g_param_spec_uint("port",
							  "Port",
							  "The plug default port",
							  0,
							  65535,
							  3000,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));
}

static void facq_plug_dialog_init(FacqPlugDialog *dialog)
{
	dialog->priv = G_TYPE_INSTANCE_GET_PRIVATE(dialog,FACQ_TYPE_PLUG_DIALOG,FacqPlugDialogPrivate);
	dialog->priv->top_window = NULL;
}

/**
 * facq_plug_dialog_new:
 * @top_window: The application top #GtkWindow.
 * @address: The default IP address.
 * @port: The default port.
 *
 * Creates a new #FacqPlugDialog object, it will show the default IP address and
 * port in the dialog window.
 *
 * Returns: a new #FacqPlugDialog object.
 */
FacqPlugDialog *facq_plug_dialog_new(GtkWidget *top_window,const gchar *address,guint16 port)
{
	return g_object_new(FACQ_TYPE_PLUG_DIALOG,"top-window",top_window,
						  "address",address,
						  "port",port,
						  NULL);
}

/**
 * facq_plug_dialog_run:
 * @dialog: A #FacqPlugDialog object.
 *
 * Displays the dialog to the user, allowing it to interact with the dialog
 * window.
 *
 * Returns: %GTK_RESPONSE_OK if the user pressed the OK button, or
 * %GTK_RESPONSE_CANCEL if the user pressed the CANCEL button.
 */
gint facq_plug_dialog_run(FacqPlugDialog *dialog)
{
	g_return_val_if_fail(FACQ_IS_PLUG_DIALOG(dialog),-1);

	if( gtk_dialog_run(GTK_DIALOG(dialog->priv->dialog)) != GTK_RESPONSE_OK){
		return GTK_RESPONSE_CANCEL;
	}
	return GTK_RESPONSE_OK;
}

/**
 * facq_plug_dialog_get_input:
 * @dialog: A #FacqPlugDialog object.
 * @port: (allow-none) (out caller-allocates): A pointer to a #guint16 variable.
 *
 * Retrieves the user input from the @dialog. The port info is written to the
 * @port variable if not %NULL and the IP address is returned by the function.
 *
 * Returns: The IP address chosen by the user, you must free it with g_free().
 */
gchar *facq_plug_dialog_get_input(const FacqPlugDialog *dialog,guint16 *port)
{
	gchar *ret = NULL;
	GtkEntryBuffer *buf = NULL;

	if(port)
		*port = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(dialog->priv->spin_button));

	buf = gtk_entry_get_buffer(GTK_ENTRY(dialog->priv->address_entry));
	if(gtk_entry_buffer_get_length(buf)){
		ret = g_strdup(gtk_entry_buffer_get_text(buf));
		if(g_strcmp0(ret,"all") == 0){
			g_free(ret);
			ret = NULL;
		}
	} else
		ret = NULL;

	return ret;
}

/**
 * facq_plug_dialog_free:
 * @dialog: A #FacqPlugDialog object.
 *
 * Destroys a no longer needed #FacqPlugDialog object.
 */
void facq_plug_dialog_free(FacqPlugDialog *dialog)
{
	g_return_if_fail(FACQ_IS_PLUG_DIALOG(dialog));
	g_object_unref(G_OBJECT(dialog));
}

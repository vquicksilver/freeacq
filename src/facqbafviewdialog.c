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
#include "facqbafviewdialog.h"

/**
 * SECTION:facqbafviewdialog
 * @short_description: Provides a simple gtk dialog for selecting time per page.
 * @include:facqbafviewdialog.h
 * @see_also: #GtkDialog,#GtkSpinButton
 *
 * #FacqBAFViewDialog provides a simple gtk dialog that can be used for asking
 * to the user how much time a page should display in the binary acquisition
 * file viewer application. 
 *
 * To create a new #FacqBAFViewDialog use facq_baf_view_dialog_new(),
 * to show the dialog to the user facq_baf_view_dialog_run(), to retrieve
 * the input from the user use facq_baf_view_dialog_get_input() and to
 * destroy the dialog use facq_baf_view_dialog_free().
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * A #FacqBAFViewDialog uses internally a #GtkDialog and #GtkTable inside
 * the content's area of the dialog. In the table a #GtkLabel, a #GtkSpinButton
 * and another #GtkLabel are attached.
 * </para>
 * </sect1>
 */

/**
 * FacqBAFViewDialog:
 *
 * Contains the private details of the #FacqBAFViewDialog object.
 */

/**
 * FacqBAFViewDialogClass:
 *
 * Class for all the #FacqBAFViewDialog objects.
 */
G_DEFINE_TYPE(FacqBAFViewDialog,facq_baf_view_dialog,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_TOP_WINDOW
};

struct _FacqBAFViewDialogPrivate {
	GtkWidget *top_window;
	GtkWidget *dialog;
	GtkWidget *spin_button;
};

static void clear_icon_callback(GtkEntry *entry,GtkEntryIconPosition icon_pos,GdkEvent *event,gpointer data)
{
	FacqBAFViewDialog *dialog = FACQ_BAF_VIEW_DIALOG(data);

	switch(icon_pos){
	case GTK_ENTRY_ICON_PRIMARY:
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog->priv->spin_button),10);
	break;
	default:
		return;
	}
}

static void facq_baf_view_dialog_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqBAFViewDialog *dialog = FACQ_BAF_VIEW_DIALOG(self);

	switch(property_id){
	case PROP_TOP_WINDOW: dialog->priv->top_window = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(dialog,property_id,pspec);
	}
}

static void facq_baf_view_dialog_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqBAFViewDialog *dialog = FACQ_BAF_VIEW_DIALOG(self);

	switch(property_id){
	case PROP_TOP_WINDOW: g_value_set_pointer(value,dialog->priv->top_window);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(dialog,property_id,pspec);
	}
}

static void facq_baf_view_dialog_constructed(GObject *self)
{
	FacqBAFViewDialog *dialog = FACQ_BAF_VIEW_DIALOG(self);
	GtkWidget *vbox = NULL, *table = NULL, *widget = NULL;

	dialog->priv->dialog = 
		gtk_dialog_new_with_buttons(_("Page preferences"),
				GTK_WINDOW(dialog->priv->top_window),
					GTK_DIALOG_MODAL | 
						GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
								GTK_STOCK_OK,GTK_RESPONSE_OK,NULL);

	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog->priv->dialog));
	table = gtk_table_new(1,3,FALSE);
	
	widget = gtk_label_new(_("Time per page:"));
	gtk_label_set_justify(GTK_LABEL(widget),GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table),widget,0,1,0,1);
	
	widget = gtk_spin_button_new_with_range(5,86400,1);
	gtk_entry_set_icon_from_stock(GTK_ENTRY(widget),GTK_ENTRY_ICON_PRIMARY,GTK_STOCK_CLEAR);
	gtk_spin_button_set_increments(GTK_SPIN_BUTTON(widget),1,10);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget),0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),10);
	g_signal_connect(GTK_ENTRY(widget),"icon-press",
				G_CALLBACK(clear_icon_callback),dialog);
	dialog->priv->spin_button = widget;
	gtk_table_attach_defaults(GTK_TABLE(table),widget,0,1,1,2);

	widget = gtk_label_new(_(" seconds"));
	gtk_label_set_justify(GTK_LABEL(widget),GTK_JUSTIFY_LEFT);
	gtk_table_attach_defaults(GTK_TABLE(table),widget,0,1,2,3);

	gtk_widget_show_all(table);

	gtk_container_add(GTK_CONTAINER(vbox),table);
}

static void facq_baf_view_dialog_finalize(GObject *self)
{
	FacqBAFViewDialog *dialog = FACQ_BAF_VIEW_DIALOG(self);

	if(GTK_IS_WIDGET(dialog->priv->spin_button))
		gtk_widget_destroy(dialog->priv->spin_button);

	if(GTK_IS_WIDGET(dialog->priv->dialog))
		gtk_widget_destroy(dialog->priv->dialog);

	G_OBJECT_CLASS(facq_baf_view_dialog_parent_class)->finalize(self);
}

static void facq_baf_view_dialog_class_init(FacqBAFViewDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqBAFViewDialogPrivate));

	object_class->set_property = facq_baf_view_dialog_set_property;
	object_class->get_property = facq_baf_view_dialog_get_property;
	object_class->constructed = facq_baf_view_dialog_constructed;
	object_class->finalize = facq_baf_view_dialog_finalize;

	g_object_class_install_property(object_class,PROP_TOP_WINDOW,
					g_param_spec_pointer("top-window",
							     "Top window",
							     "The application top window",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));
}

static void facq_baf_view_dialog_init(FacqBAFViewDialog *dialog)
{
	dialog->priv = G_TYPE_INSTANCE_GET_PRIVATE(dialog,FACQ_TYPE_BAF_VIEW_DIALOG,FacqBAFViewDialogPrivate);
	dialog->priv->top_window = NULL;
}

/**
 * facq_baf_view_dialog_new:
 * @top_window: The dialog top window, this will normally be the application top
 * window.
 *
 * Creates a new #FacqBAFViewDialog object that can be used later to display a
 * dialog to the user, allowing the user to select the number of seconds per
 * page.
 *
 * Returns: A #FacqBAFViewDialog object.
 */
FacqBAFViewDialog *facq_baf_view_dialog_new(GtkWidget *top_window)
{
	return g_object_new(FACQ_TYPE_BAF_VIEW_DIALOG,"top-window",top_window,NULL);
}

/**
 * facq_baf_view_dialog_run:
 * @dialog: A #FacqBAFViewDialog object.
 *
 * Displays the dialog to the user allowing the user to interact with it.
 * If the user press the OK button you can read the input value with
 * facq_baf_view_dialog_run_get_input().
 *
 * Returns: %GTK_RESPONSE_OK if the user press the OK button or
 * %GTK_RESPONSE_CANCEL if the user press the CANCEL button.
 */
gint facq_baf_view_dialog_run(FacqBAFViewDialog *dialog)
{
	g_return_val_if_fail(FACQ_IS_BAF_VIEW_DIALOG(dialog),-1);

	if( gtk_dialog_run(GTK_DIALOG(dialog->priv->dialog)) != GTK_RESPONSE_OK){
		return GTK_RESPONSE_CANCEL;
	}
	return GTK_RESPONSE_OK;
}

/**
 * facq_baf_view_dialog_get_input:
 * @dialog: A #FacqBAFViewDialog object.
 *
 * Retrieves the value selected by the user in the dialog, that is the
 * number of seconds per page.
 *
 * Returns: A #gdouble with the number of seconds per page.
 */
gdouble facq_baf_view_dialog_get_input(const FacqBAFViewDialog *dialog)
{
	gdouble ret = 10;

	ret = gtk_spin_button_get_value(GTK_SPIN_BUTTON(dialog->priv->spin_button));

	return ret;
}

/**
 * facq_baf_view_dialog_free:
 * @dialog: A #FacqBAFViewDialog object.
 *
 * Destroys a no longer needed #FacqBAFViewDialog object.
 */
void facq_baf_view_dialog_free(FacqBAFViewDialog *dialog)
{
	g_return_if_fail(FACQ_IS_BAF_VIEW_DIALOG(dialog));
	g_object_unref(G_OBJECT(dialog));
}

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
#include "facqcapturetoolbarcallbacks.h"
#include "facqcapturetoolbar.h"

/**
 * SECTION:facqcapturetoolbar
 * @short_description: Provides the capture's toolbar.
 * @include:facqcapturetoolbar.h
 *
 * This module provides the toolbar for the capture program, including
 * the add/remove/clear/play/stop buttons. Also allows to enable and disable
 * the buttons with an easy API.
 *
 * Take a look at the functions to see how to create, destroy, and manage the
 * status of the buttons, it should be easy ;) .
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * #FacqCaptureToolbar uses internally a #GtkToolbar and some #GtkToolButton
 * objects.
 * </para>
 * </sect1>
 */

/**
 * FacqCaptureToolbar:
 *
 * Includes the private details of #FacqCaptureToolbar.
 */

/**
 * FacqCaptureToolbarClass:
 *
 * Class for the #FacqCaptureToolbar objects.
 */

G_DEFINE_TYPE(FacqCaptureToolbar,facq_capture_toolbar,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_TOOLBAR,
	PROP_DATA
};

struct _FacqCaptureToolbarPrivate {
	GtkWidget *toolbar;
	gpointer data;
};

/*****--- GObject magic ---*****/
static void facq_capture_toolbar_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqCaptureToolbar *toolbar = FACQ_CAPTURE_TOOLBAR(self);

	switch(property_id){
	case PROP_TOOLBAR: toolbar->priv->toolbar = g_value_get_pointer(value);
	break;
	case PROP_DATA: toolbar->priv->data = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(toolbar,property_id,pspec);
	}
}

static void facq_capture_toolbar_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqCaptureToolbar *toolbar = FACQ_CAPTURE_TOOLBAR(self);

	switch(property_id){
	case PROP_TOOLBAR: g_value_set_pointer(value,toolbar->priv->toolbar);
	break;
	case PROP_DATA: g_value_set_pointer(value,toolbar->priv->toolbar);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(toolbar,property_id,pspec);
	}
}

static void facq_capture_toolbar_constructed(GObject *self)
{
	FacqCaptureToolbar *bar = FACQ_CAPTURE_TOOLBAR(self);

	GtkWidget *toolbar = NULL;
	GtkToolItem *toolitem = NULL;
	GtkSeparatorToolItem *separator = NULL;

	toolbar = gtk_toolbar_new();

#if GTK_MAJOR_VERSION > 2
	toolitem = gtk_tool_button_new(NULL,_("Add"));
#else
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ADD);
#endif
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,0);
	gtk_widget_set_sensitive(GTK_WIDGET(toolitem),FALSE);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_capture_toolbar_callback_add),bar->priv->data);

#if GTK_MAJOR_VERSION > 2
	toolitem = gtk_tool_button_new(NULL,_("Remove"));
#else
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_REMOVE);
#endif
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,1);
	gtk_widget_set_sensitive(GTK_WIDGET(toolitem),FALSE);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_capture_toolbar_callback_remove),bar->priv->data);

#if GTK_MAJOR_VERSION > 2
	toolitem = gtk_tool_button_new(NULL,_("Clear"));
#else
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_CLEAR);
#endif
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_capture_toolbar_callback_clear),bar->priv->data);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,2);
	gtk_widget_set_sensitive(GTK_WIDGET(toolitem),FALSE);

#if GTK_MAJOR_VERSION > 2
	toolitem = gtk_tool_button_new(NULL,_("Play"));
#else
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
#endif
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_capture_toolbar_callback_play),bar->priv->data);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,3);
	gtk_widget_set_sensitive(GTK_WIDGET(toolitem),FALSE);

#if GTK_MAJOR_VERSION > 2
	toolitem = gtk_tool_button_new(NULL,_("Stop"));
#else
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
#endif
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_capture_toolbar_callback_stop),bar->priv->data);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,4);
	gtk_widget_set_sensitive(GTK_WIDGET(toolitem),FALSE);

	gtk_widget_show_all(toolbar);

	bar->priv->toolbar = toolbar;
}

static void facq_capture_toolbar_finalize(GObject *self)
{
	FacqCaptureToolbar *toolbar = FACQ_CAPTURE_TOOLBAR(self);

	if(GTK_IS_WIDGET(toolbar->priv->toolbar))
		gtk_widget_destroy(toolbar->priv->toolbar);
	if(toolbar->priv->data)
		toolbar->priv->data = NULL;

	G_OBJECT_CLASS (facq_capture_toolbar_parent_class)->finalize (self);
}

static void facq_capture_toolbar_class_init(FacqCaptureToolbarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqCaptureToolbarPrivate));

	object_class->get_property = facq_capture_toolbar_get_property;
	object_class->set_property = facq_capture_toolbar_set_property;
	object_class->finalize = facq_capture_toolbar_finalize;
	object_class->constructed = facq_capture_toolbar_constructed;

	g_object_class_install_property(object_class,PROP_TOOLBAR,
					g_param_spec_pointer("toolbar",
							     "toolbar",
							     "A toolbar",
							     G_PARAM_READWRITE|
							     G_PARAM_CONSTRUCT_ONLY|
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_DATA,
					g_param_spec_pointer("data",
							     "data",
							     "Data for toolbar callbacks",
							     G_PARAM_READWRITE|
							     G_PARAM_CONSTRUCT_ONLY|
							     G_PARAM_STATIC_STRINGS));
}

static void facq_capture_toolbar_init(FacqCaptureToolbar *toolbar)
{
	toolbar->priv = G_TYPE_INSTANCE_GET_PRIVATE(toolbar,FACQ_TYPE_CAPTURE_TOOLBAR,FacqCaptureToolbarPrivate);
	toolbar->priv->toolbar = NULL;
	toolbar->priv->data = NULL;
}
/*****--- Private methods ---*****/
#if GTK_MAJOR_VERSION > 2
static void facq_capture_toolbar_change_toolitem(FacqCaptureToolbar *toolbar,const gchar *stock_id,gboolean sensitive)
{
	GList *list = NULL;
	GtkToolItem *item = NULL;
	const gchar *current_id = NULL;

	g_return_if_fail(FACQ_IS_CAPTURE_TOOLBAR(toolbar));
	list = gtk_container_get_children(GTK_CONTAINER(toolbar->priv->toolbar));
	
	while(list != NULL){
		item = list->data;
		if(GTK_IS_TOOL_BUTTON(item)){
			current_id = gtk_tool_button_get_stock_id(GTK_TOOL_BUTTON(item));
			if(g_strcmp0(stock_id,current_id) == 0){
				gtk_widget_set_sensitive(GTK_WIDGET(item),sensitive);
				break;
			}
		}
		list = list->next;
	}
}
#else

#endif

/*****--- Public methods ---*****/
/**
 * facq_capture_toolbar_new:
 * @data: A pointer to some data that you want to pass to the toolbar. 
 * This pointer will be used in the callback functions.
 *
 * Creates a new #FacqCaptureToolbar object. This will allow to retrieve later a 
 * #GtkWidget that is a #GtkToolbar with all the needed buttons by the capture
 * program, including buttons for adding/removing/clearing/starting and stopping
 * a #FacqStream.
 *
 * Returns: A new #FacqCaptureToolbar object.
 */
FacqCaptureToolbar *facq_capture_toolbar_new(gpointer data)
{
	return g_object_new(FACQ_TYPE_CAPTURE_TOOLBAR,"data",data,NULL);
}

/**
 * facq_capture_toolbar_get_widget:
 * @toolbar: A #FacqCaptureToolbar object.
 *
 * Gets the top level widget of the #FacqCaptureToolbar, so you can add
 * it to your application.
 *
 * Returns: A #GtkWidget that is a pointer to the toolbar widget.
 */
GtkWidget *facq_capture_toolbar_get_widget(FacqCaptureToolbar *toolbar)
{
	g_return_val_if_fail(FACQ_IS_CAPTURE_TOOLBAR(toolbar),NULL);
	return toolbar->priv->toolbar;
}

/**
 * facq_capture_toolbar_enable_add:
 * @toolbar: A #FacqCaptureToolbar object.
 *
 * Enables the add button in the toolbar, the user will be able to press it
 * after calling this function.
 */
void facq_capture_toolbar_enable_add(FacqCaptureToolbar *toolbar)
{
#if GTK_MAJOR_VERSION > 2

#else
	facq_capture_toolbar_change_toolitem(toolbar,GTK_STOCK_ADD,TRUE);
#endif
}

/**
 * facq_capture_toolbar_enable_remove:
 * @toolbar: A #FacqCaptureToolbar object.
 *
 * Enables the remove button in the toolbar, the user will be able to press it
 * after calling this function.
 */
void facq_capture_toolbar_enable_remove(FacqCaptureToolbar *toolbar)
{
#if GTK_MAJOR_VERSION > 2

#else
	facq_capture_toolbar_change_toolitem(toolbar,GTK_STOCK_REMOVE,TRUE);
#endif
}

/**
 * facq_capture_toolbar_enable_clear:
 * @toolbar: A #FacqCaptureToolbar object.
 *
 * Enable the clear button in the toolbar, the user will be able to press it
 * after calling this function.
 */
void facq_capture_toolbar_enable_clear(FacqCaptureToolbar *toolbar)
{
#if GTK_MAJOR_VERSION > 2

#else
	facq_capture_toolbar_change_toolitem(toolbar,GTK_STOCK_CLEAR,TRUE);
#endif
}

/**
 * facq_capture_toolbar_enable_play:
 * @toolbar: A #FacqCaptureToolbar object.
 *
 * Enables the play button in the toolbar, the user will be able to press it
 * after calling this function.
 */
void facq_capture_toolbar_enable_play(FacqCaptureToolbar *toolbar)
{
#if GTK_MAJOR_VERSION > 2

#else
	facq_capture_toolbar_change_toolitem(toolbar,GTK_STOCK_MEDIA_PLAY,TRUE);
#endif
}

/**
 * facq_capture_toolbar_enable_stop:
 * @toolbar: A #FacqCaptureToolbar object.
 *
 * Enables the stop button in the toolbar, the user will be able to press it
 * after calling this function.
 */
void facq_capture_toolbar_enable_stop(FacqCaptureToolbar *toolbar)
{
#if GTK_MAJOR_VERSION > 2

#else
	facq_capture_toolbar_change_toolitem(toolbar,GTK_STOCK_MEDIA_STOP,TRUE);
#endif
}

/**
 * facq_capture_toolbar_disable_add:
 * @toolbar: A #FacqCaptureToolbar object.
 *
 * Disable the add button in the toolbar, the user won't be able to press it
 * until the button is enabled again.
 */
void facq_capture_toolbar_disable_add(FacqCaptureToolbar *toolbar)
{
#if GTK_MAJOR_VERSION > 2

#else
	facq_capture_toolbar_change_toolitem(toolbar,GTK_STOCK_ADD,FALSE);
#endif
}

/**
 * facq_capture_toolbar_disable_remove:
 * @toolbar: A #FacqCaptureToolbar object.
 *
 * Disable the remove button in the toolbar, the user won't be able to press it
 * until the button is enabled again.
 */
void facq_capture_toolbar_disable_remove(FacqCaptureToolbar *toolbar)
{
#if GTK_MAJOR_VERSION > 2

#else
	facq_capture_toolbar_change_toolitem(toolbar,GTK_STOCK_REMOVE,FALSE);
#endif
}

/**
 * facq_capture_toolbar_disable_clear:
 * @toolbar: A #FacqCaptureToolbar object.
 *
 * Disable the clear button in the toolbar, the user won't be able to press it
 * until the button is enabled again.
 */
void facq_capture_toolbar_disable_clear(FacqCaptureToolbar *toolbar)
{
#if GTK_MAJOR_VERSION > 2

#else
	facq_capture_toolbar_change_toolitem(toolbar,GTK_STOCK_CLEAR,FALSE);
#endif
}

/**
 * facq_capture_toolbar_disable_play:
 * @toolbar: A #FacqCaptureToolbar object.
 *
 * Disables the play button in the toolbar, the user won't be able to press it
 * until the button is enabled again.
 */
void facq_capture_toolbar_disable_play(FacqCaptureToolbar *toolbar)
{
#if GTK_MAJOR_VERSION > 2

#else
	facq_capture_toolbar_change_toolitem(toolbar,GTK_STOCK_MEDIA_PLAY,FALSE);
#endif
}

/**
 * facq_capture_toolbar_disable_stop:
 * @toolbar: A #FacqCaptureToolbar object.
 *
 * Disables the stop button in the toolbar, the user won't be able to press it
 * until the button is enabled again.
 */
void facq_capture_toolbar_disable_stop(FacqCaptureToolbar *toolbar)
{
#if GTK_MAJOR_VERSION > 2

#else
	facq_capture_toolbar_change_toolitem(toolbar,GTK_STOCK_MEDIA_STOP,FALSE);
#endif
}

/**
 * facq_capture_toolbar_free:
 * @toolbar: A #FacqCaptureToolbar object.
 *
 * Destroys a no longer needed #FacqCaptureToolbar object.
 */
void facq_capture_toolbar_free(FacqCaptureToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_CAPTURE_TOOLBAR(toolbar));
	g_object_unref(G_OBJECT(toolbar));
}

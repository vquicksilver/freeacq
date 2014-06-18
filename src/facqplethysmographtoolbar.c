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
#include "facqplethysmographtoolbar.h"
#include "facqplethysmographtoolbarcallbacks.h"

/**
 * SECTION:facqplethysmographtoolbar
 * @title:FacqPlethysmographToolbar
 * @short_description:Provides the plethysmograph's toolbar
 * @include:facqplethysmograph.h
 *
 * Provides the toolbar that can be seen on the plethysmograph application,
 * including the disconnect and preferences buttons. Also allows to enable
 * and disable the buttons with an easy API.
 *
 * To create a new toolbar use facq_plethysmograph_toolbar_new(), to obtain
 * the #GtkToolbar widget use facq_plethysmograph_toolbar_get_widget(),
 * to enable or disable the preferences and the disconnect button use
 * facq_plethysmograph_toolbar_enable_plug_preferences(),
 * facq_plethysmograph_toolbar_disable_plug_preferences(),
 * facq_plethysmograph_toolbar_enable_disconnect() and
 * facq_plethysmograph_toolbar_disable_disconnect().
 * To destroy the toolbar use facq_plethysmograph_toolbar_free().
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * #FacqPlethysmographToolbar uses internally a #GtkToolbar and some
 * #GtkToolButton objects.
 * </para>
 * </sect1>
 *
 */

/**
 * FacqPlethysmographToolbar:
 *
 * Contains the private details of the #FacqPlethysmographToolbar objects.
 */

/**
 * FacqPlethysmographToolbarClass:
 *
 * Class for the #FacqPlethysmographToolbar objects.
 */

G_DEFINE_TYPE(FacqPlethysmographToolbar,facq_plethysmograph_toolbar,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_TOOLBAR,
	PROP_DATA
};

struct _FacqPlethysmographToolbarPrivate {
	GtkWidget *toolbar;
	gpointer data;
};

/*****--- GObject magic ---*****/
static void facq_plethysmograph_toolbar_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqPlethysmographToolbar *toolbar = FACQ_PLETHYSMOGRAPH_TOOLBAR(self);

	switch(property_id){
	case PROP_TOOLBAR: toolbar->priv->toolbar = g_value_get_pointer(value);
	break;
	case PROP_DATA: toolbar->priv->data = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(toolbar,property_id,pspec);
	}
}

static void facq_plethysmograph_toolbar_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqPlethysmographToolbar *toolbar = FACQ_PLETHYSMOGRAPH_TOOLBAR(self);

	switch(property_id){
	case PROP_TOOLBAR: g_value_set_pointer(value,toolbar->priv->toolbar);
	break;
	case PROP_DATA: g_value_set_pointer(value,toolbar->priv->toolbar);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(toolbar,property_id,pspec);
	}
}

static void facq_plethysmograph_toolbar_constructed(GObject *self)
{
	FacqPlethysmographToolbar *bar = FACQ_PLETHYSMOGRAPH_TOOLBAR(self);

	GtkWidget *toolbar = NULL;
	GtkToolItem *toolitem = NULL;
	GtkSeparatorToolItem *separator = NULL;

	toolbar = gtk_toolbar_new();

#if GTK_MAJOR_VERSION > 2
	toolitem = gtk_tool_button_new(NULL,_("_Add"));
#else
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_PREFERENCES);
#endif
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,0);
	gtk_widget_set_sensitive(GTK_WIDGET(toolitem),TRUE);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_plethysmograph_toolbar_callback_plug_preferences),bar->priv->data);

#if GTK_MAJOR_VERSION > 2
	toolitem = gtk_tool_button_new(NULL,_("Disconnect"));
#else
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_DISCONNECT);
#endif
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_plethysmograph_toolbar_callback_disconnect),bar->priv->data);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,1);
	gtk_widget_set_sensitive(GTK_WIDGET(toolitem),FALSE);

	gtk_widget_show_all(toolbar);

	bar->priv->toolbar = toolbar;
}

static void facq_plethysmograph_toolbar_finalize(GObject *self)
{
	FacqPlethysmographToolbar *toolbar = FACQ_PLETHYSMOGRAPH_TOOLBAR(self);

	if(GTK_IS_WIDGET(toolbar->priv->toolbar))
		gtk_widget_destroy(toolbar->priv->toolbar);
	if(toolbar->priv->data)
		toolbar->priv->data = NULL;

	G_OBJECT_CLASS (facq_plethysmograph_toolbar_parent_class)->finalize (self);
}

static void facq_plethysmograph_toolbar_class_init(FacqPlethysmographToolbarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqPlethysmographToolbarPrivate));

	object_class->get_property = facq_plethysmograph_toolbar_get_property;
	object_class->set_property = facq_plethysmograph_toolbar_set_property;
	object_class->finalize = facq_plethysmograph_toolbar_finalize;
	object_class->constructed = facq_plethysmograph_toolbar_constructed;

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

static void facq_plethysmograph_toolbar_init(FacqPlethysmographToolbar *toolbar)
{
	toolbar->priv = G_TYPE_INSTANCE_GET_PRIVATE(toolbar,FACQ_TYPE_PLETHYSMOGRAPH_TOOLBAR,FacqPlethysmographToolbarPrivate);
	toolbar->priv->toolbar = NULL;
	toolbar->priv->data = NULL;
}
/*****--- Private methods ---*****/
#if GTK_MAJOR_VERSION > 2

#else
static void facq_plethysmograph_toolbar_change_toolitem(FacqPlethysmographToolbar *toolbar,const gchar *stock_id,gboolean sensitive)
{
	GList *list = NULL;
	GtkToolItem *item = NULL;
	const gchar *current_id = NULL;

	g_return_if_fail(FACQ_IS_PLETHYSMOGRAPH_TOOLBAR(toolbar));
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
#endif

/*****--- Public methods ---*****/
/**
 * facq_plethysmograph_toolbar_new:
 * @data: A pointer to some data that you want to pass to the toolbar callbacks.
 *
 * Creates a new #FacqPlethysmograph toolbar object.
 *
 * Returns: A new #FacqPlethysmographToolbar object.
 */
FacqPlethysmographToolbar *facq_plethysmograph_toolbar_new(gpointer data)
{
	return g_object_new(FACQ_TYPE_PLETHYSMOGRAPH_TOOLBAR,"data",data,NULL);
}

/**
 * facq_plethysmograph_toolbar_get_widget:
 * @toolbar: A #FacqPlethysmographToolbar object.
 * 
 * Gets the #GtkToolbar from the #FacqPlethysmographToolbar so it can
 * be added to your application.
 *
 * Returns: A #GtkWidget pointing to the #GtkToolbar.
 */
GtkWidget *facq_plethysmograph_toolbar_get_widget(FacqPlethysmographToolbar *toolbar)
{
	g_return_val_if_fail(FACQ_IS_PLETHYSMOGRAPH_TOOLBAR(toolbar),NULL);
	return toolbar->priv->toolbar;
}

/**
 * facq_plethysmograph_toolbar_disable_disconnect:
 * @toolbar: A #FacqPlethysmographToolbar object.
 *
 * Disables the disconnect button on the #FacqPlethysmographToolbar,
 * that means that the user can't press it.
 */
void facq_plethysmograph_toolbar_disable_disconnect(FacqPlethysmographToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_PLETHYSMOGRAPH_TOOLBAR(toolbar));
#if GTK_MAJOR_VERSION > 2

#else
	facq_plethysmograph_toolbar_change_toolitem(toolbar,GTK_STOCK_DISCONNECT,FALSE);
#endif
}

/**
 * facq_plethysmograph_toolbar_disable_plug_preferences:
 * @toolbar: A #FacqPlethysmographToolbar object.
 *
 * Disables the plug preferences button on the #FacqPlethysmographToolbar,
 * that means that the user can't press it.
 */
void facq_plethysmograph_toolbar_disable_plug_preferences(FacqPlethysmographToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_PLETHYSMOGRAPH_TOOLBAR(toolbar));
#if GTK_MAJOR_VERSION > 2

#else
	facq_plethysmograph_toolbar_change_toolitem(toolbar,GTK_STOCK_PREFERENCES,FALSE);
#endif
}

/**
 * facq_plethysmograph_toolbar_enable_disconnect:
 * @toolbar: A #FacqPlethysmographToolbar object.
 *
 * Enables the disconnect button on the #FacqPlethysmographToolbar,
 * that means that the user can press it.
 */
void facq_plethysmograph_toolbar_enable_disconnect(FacqPlethysmographToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_PLETHYSMOGRAPH_TOOLBAR(toolbar));
#if GTK_MAJOR_VERSION > 2

#else
	facq_plethysmograph_toolbar_change_toolitem(toolbar,GTK_STOCK_DISCONNECT,TRUE);
#endif
}

/**
 * facq_plethysmograph_toolbar_enable_plug_preferences:
 * @toolbar: A #FacqPlethysmographToolbar object.
 *
 * Enables the plug preferences button on the #FacqPlethysmographToolbar,
 * that means that the user can press it.
 */
void facq_plethysmograph_toolbar_enable_plug_preferences(FacqPlethysmographToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_PLETHYSMOGRAPH_TOOLBAR(toolbar));
#if GTK_MAJOR_VERSION > 2

#else
	facq_plethysmograph_toolbar_change_toolitem(toolbar,GTK_STOCK_PREFERENCES,TRUE);
#endif
}

/**
 * facq_plethysmograph_toolbar_free:
 * @toolbar: A #FacqPlethysmographToolbar object.
 *
 * Destroys a no longer needed #FacqPlethysmographToolbar object.
 */
void facq_plethysmograph_toolbar_free(FacqPlethysmographToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_PLETHYSMOGRAPH_TOOLBAR(toolbar));
	g_object_unref(G_OBJECT(toolbar));
}

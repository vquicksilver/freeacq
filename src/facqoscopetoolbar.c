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
#include "facqoscopetoolbar.h"
#include "facqoscopetoolbarcallbacks.h"

/**
 * SECTION:facqoscopetoolbar
 * @short_description: Provides the toolbar for the oscilloscope application.
 * @include:facqoscopetoolbar.h
 *
 * Provides the toolbar for the oscilloscope application, including the
 * preferences, the disconnect, the zoom in, the zoom out, and the normal size
 * buttons. Also allows to enable and disable the buttons with an easy API.
 *
 * To create a new toolbar use facq_oscope_toolbar_new(), to obtain the
 * #GtkToolBar widget use facq_oscope_toolber_get_widget(), and to enable or
 * disable the buttons in the toolbar take a look at the various enable/disable
 * functions.
 *
 * Finally to destroy the #FacqOscopeToolbar you can facq_oscope_toolbar_free().
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * #FacqOscopeToolbar uses internally a #GtkToolBar and some #GtkToolButton
 * objects with a #GtkSeparatorToolItem.
 * </para>
 * </sect1>
 */

/**
 * FacqOscopeToolbar:
 *
 * Contains the private details of #FacqOscopeToolbar.
 */

/**
 * FacqOscopeToolbarClass:
 *
 * Class for all the #FacqOscopeToolbar objects.
 */

G_DEFINE_TYPE(FacqOscopeToolbar,facq_oscope_toolbar,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_TOOLBAR,
	PROP_DATA
};

struct _FacqOscopeToolbarPrivate {
	GtkWidget *toolbar;
	gpointer data;
};

/*****--- GObject magic ---*****/
static void facq_oscope_toolbar_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqOscopeToolbar *toolbar = FACQ_OSCOPE_TOOLBAR(self);

	switch(property_id){
	case PROP_TOOLBAR: toolbar->priv->toolbar = g_value_get_pointer(value);
	break;
	case PROP_DATA: toolbar->priv->data = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(toolbar,property_id,pspec);
	}
}

static void facq_oscope_toolbar_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqOscopeToolbar *toolbar = FACQ_OSCOPE_TOOLBAR(self);

	switch(property_id){
	case PROP_TOOLBAR: g_value_set_pointer(value,toolbar->priv->toolbar);
	break;
	case PROP_DATA: g_value_set_pointer(value,toolbar->priv->toolbar);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(toolbar,property_id,pspec);
	}
}

static void facq_oscope_toolbar_constructed(GObject *self)
{
	FacqOscopeToolbar *bar = FACQ_OSCOPE_TOOLBAR(self);

	GtkWidget *toolbar = NULL;
	GtkToolItem *toolitem = NULL;
	GtkSeparatorToolItem *separator = NULL;

	toolbar = gtk_toolbar_new();

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_PREFERENCES);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,0);
	gtk_widget_set_sensitive(GTK_WIDGET(toolitem),TRUE);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_oscope_toolbar_callback_preferences),bar->priv->data);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_DISCONNECT);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_oscope_toolbar_callback_disconnect),bar->priv->data);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,1);
	gtk_widget_set_sensitive(GTK_WIDGET(toolitem),FALSE);

	separator = GTK_SEPARATOR_TOOL_ITEM(gtk_separator_tool_item_new());
	gtk_separator_tool_item_set_draw(separator,TRUE);
	gtk_tool_item_set_expand(GTK_TOOL_ITEM(separator),FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),GTK_TOOL_ITEM(separator),2);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ZOOM_IN);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_oscope_toolbar_callback_zoom_in),bar->priv->data);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,3);
	
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ZOOM_OUT);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_oscope_toolbar_callback_zoom_out),bar->priv->data);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,4);
	
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ZOOM_100);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_oscope_toolbar_callback_zoom_100),bar->priv->data);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,5);

	gtk_widget_show_all(toolbar);

	bar->priv->toolbar = toolbar;
}

static void facq_oscope_toolbar_finalize(GObject *self)
{
	FacqOscopeToolbar *toolbar = FACQ_OSCOPE_TOOLBAR(self);

	if(GTK_IS_WIDGET(toolbar->priv->toolbar))
		gtk_widget_destroy(toolbar->priv->toolbar);
	if(toolbar->priv->data)
		toolbar->priv->data = NULL;

	G_OBJECT_CLASS (facq_oscope_toolbar_parent_class)->finalize (self);
}

static void facq_oscope_toolbar_class_init(FacqOscopeToolbarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqOscopeToolbarPrivate));

	object_class->get_property = facq_oscope_toolbar_get_property;
	object_class->set_property = facq_oscope_toolbar_set_property;
	object_class->finalize = facq_oscope_toolbar_finalize;
	object_class->constructed = facq_oscope_toolbar_constructed;

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

static void facq_oscope_toolbar_init(FacqOscopeToolbar *toolbar)
{
	toolbar->priv = G_TYPE_INSTANCE_GET_PRIVATE(toolbar,FACQ_TYPE_OSCOPE_TOOLBAR,FacqOscopeToolbarPrivate);
	toolbar->priv->toolbar = NULL;
	toolbar->priv->data = NULL;
}
/*****--- Private methods ---*****/
static void facq_oscope_toolbar_change_toolitem(FacqOscopeToolbar *toolbar,const gchar *stock_id,gboolean sensitive)
{
	GList *list = NULL;
	GtkToolItem *item = NULL;
	const gchar *current_id = NULL;

	g_return_if_fail(FACQ_IS_OSCOPE_TOOLBAR(toolbar));
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

/*****--- Public methods ---*****/
/**
 * facq_oscope_toolbar_new:
 * @data: A pointer to some data that you want to pass to the callback
 * functions, in the oscilloscope program a #FacqOscope object will be used
 * here.
 *
 * Creates a new #FacqOscopeToolbar object.
 *
 * Returns: a new #FacqOscopeToolbar.
 */
FacqOscopeToolbar *facq_oscope_toolbar_new(gpointer data)
{
	return g_object_new(FACQ_TYPE_OSCOPE_TOOLBAR,"data",data,NULL);
}

/**
 * facq_oscope_toolbar_get_widget:
 * @toolbar: A #FacqOscopeToolbar object.
 *
 * Gets the top level widget of the toolbar so you can add it to the
 * application.
 *
 * Returns: A #GtkWidget pointing to the #GtkToolBar.
 */
GtkWidget *facq_oscope_toolbar_get_widget(FacqOscopeToolbar *toolbar)
{
	g_return_val_if_fail(FACQ_IS_OSCOPE_TOOLBAR(toolbar),NULL);
	return toolbar->priv->toolbar;
}

/**
 * facq_oscope_toolbar_disable_disconnect:
 * @toolbar: A #FacqOscopeToolbar object.
 *
 * Disables the disconnect button in the toolbar, the user won't be able to
 * press it again until enabled again.
 */
void facq_oscope_toolbar_disable_disconnect(FacqOscopeToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_OSCOPE_TOOLBAR(toolbar));
	facq_oscope_toolbar_change_toolitem(toolbar,GTK_STOCK_DISCONNECT,FALSE);
}

/**
 * facq_oscope_toolbar_disable_preferences:
 * @toolbar: A #FacqOscopeToolbar object.
 *
 * Disables the preferences button in the toolbar, the user won't be able to
 * press it again until enabled again.
 */
void facq_oscope_toolbar_disable_preferences(FacqOscopeToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_OSCOPE_TOOLBAR(toolbar));
	facq_oscope_toolbar_change_toolitem(toolbar,GTK_STOCK_PREFERENCES,FALSE);
}

/**
 * facq_oscope_toolbar_disable_zoom_in:
 * @toolbar: A #FacqOscopeToolbar object.
 *
 * Disables the Zoom in button in the toolbar, the user won't be able to
 * press it again until enabled again.
 */
void facq_oscope_toolbar_disable_zoom_in(FacqOscopeToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_OSCOPE_TOOLBAR(toolbar));
	facq_oscope_toolbar_change_toolitem(toolbar,GTK_STOCK_ZOOM_IN,FALSE);
}

/**
 * facq_oscope_toolbar_disable_zoom_out:
 * @toolbar: A #FacqOscopeToolbar object.
 *
 * Disables the Zoom out button in the toolbar, the user won't be able to
 * press it again until enabled again.
 */
void facq_oscope_toolbar_disable_zoom_out(FacqOscopeToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_OSCOPE_TOOLBAR(toolbar));
	facq_oscope_toolbar_change_toolitem(toolbar,GTK_STOCK_ZOOM_OUT,FALSE);
}

/**
 * facq_oscope_toolbar_disable_zoom_home:
 * @toolbar: A #FacqOscopeToolbar object.
 *
 * Disables the Normal size button in the toolbar, the user won't be able to
 * press it again until enabled again.
 */
void facq_oscope_toolbar_disable_zoom_home(FacqOscopeToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_OSCOPE_TOOLBAR(toolbar));
	facq_oscope_toolbar_change_toolitem(toolbar,GTK_STOCK_ZOOM_100,FALSE);
}

/**
 * facq_oscope_toolbar_enable_disconnect:
 * @toolbar: A #FacqOscopeToolbar object.
 *
 * Enables the Disconnect button in the toolbar.
 */
void facq_oscope_toolbar_enable_disconnect(FacqOscopeToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_OSCOPE_TOOLBAR(toolbar));
	facq_oscope_toolbar_change_toolitem(toolbar,GTK_STOCK_DISCONNECT,TRUE);
}

/**
 * facq_oscope_toolbar_enable_preferences:
 * @toolbar: A #FacqOscopeToolbar object.
 *
 * Enables the Preferences button in the toolbar.
 */
void facq_oscope_toolbar_enable_preferences(FacqOscopeToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_OSCOPE_TOOLBAR(toolbar));
	facq_oscope_toolbar_change_toolitem(toolbar,GTK_STOCK_PREFERENCES,TRUE);
}

/**
 * facq_oscope_toolbar_enable_zoom_in:
 * @toolbar: A #FacqOscopeToolbar object.
 *
 * Enables the Zoom in button in the toolbar.
 */
void facq_oscope_toolbar_enable_zoom_in(FacqOscopeToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_OSCOPE_TOOLBAR(toolbar));
	facq_oscope_toolbar_change_toolitem(toolbar,GTK_STOCK_ZOOM_IN,TRUE);
}

/**
 * facq_oscope_toolbar_enable_zoom_out:
 * @toolbar: A #FacqOscopeToolbar object.
 *
 * Enables the Zoom out button in the toolbar.
 */
void facq_oscope_toolbar_enable_zoom_out(FacqOscopeToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_OSCOPE_TOOLBAR(toolbar));
	facq_oscope_toolbar_change_toolitem(toolbar,GTK_STOCK_ZOOM_OUT,TRUE);
}

/**
 * facq_oscope_toolbar_enable_zoom_home:
 * @toolbar: A #FacqOscopeToolbar object.
 *
 * Enables the Normal size button in the toolbar.
 */
void facq_oscope_toolbar_enable_zoom_home(FacqOscopeToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_OSCOPE_TOOLBAR(toolbar));
	facq_oscope_toolbar_change_toolitem(toolbar,GTK_STOCK_ZOOM_100,TRUE);
}

/**
 * facq_oscope_toolbar_free:
 * @toolbar: A #FacqOscopeToolbar object.
 *
 * Destroys a no longer needed #FacqOscopeToolbar object.
 */
void facq_oscope_toolbar_free(FacqOscopeToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_OSCOPE_TOOLBAR(toolbar));
	g_object_unref(G_OBJECT(toolbar));
}

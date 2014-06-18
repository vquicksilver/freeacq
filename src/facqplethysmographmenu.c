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
#include "facqplethysmographmenu.h"
#include "facqplethysmographmenucallbacks.h"

/**
 * SECTION:facqplethysmographmenu
 * @short_description: Provides the menu for the plethysmograph.
 * @title:FacqPlethysmographMenu
 * @include:facqplethysmographmenu.h
 *
 * Provides the menu for the plethysmograph application, including the
 * Plethysmograph, Plug and Help entries.
 *
 * To create the plethysmograph menu use facq_plethysmograph_menu_new(),
 * to obtain the toplevel widget so you can add the menu to the application use
 * facq_plethysmograph_menu_get_widget(), and to control the status of the
 * buttons in the menu you can use the following functions, 
 * facq_plethysmograph_menu_disable_plug_preferences() to disable the plug preferences button,
 * facq_plethysmograph_menu_disable_disconnect() to disable the disconnect
 * button, facq_plethysmograph_menu_enable_plug_preferences() to enable the plug
 * preferences button and facq_plethysmograph_menu_enable_disconnect() to enable
 * the disconnect button.
 *
 * Finally to destroy the menu simply call facq_plethysmograph_menu_free().
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * Internally #FacqPlethysmographMenu uses a #GtkMenuBar and various #GtkMenu
 * objects along various #GtkMenuItem objects.
 * </para>
 * </sect1>
 */

/**
 * FacqPlethysmographMenu:
 *
 * Contains the private details of the #FacqPlethysmographMenu objects.
 */

/**
 * FacqPlethysmographMenuClass:
 *
 * Class for the #FacqPlethysmographMenu objects.
 */
G_DEFINE_TYPE(FacqPlethysmographMenu,facq_plethysmograph_menu,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_MENUBAR,
	PROP_DATA
};

struct _FacqPlethysmographMenuPrivate {
	GtkWidget *menubar;
	gpointer data;
	GtkWidget *disconnect;
	GtkWidget *plug_preferences;
	GtkWidget *preferences;
};

/*****--- GObject magic ---*****/
static void facq_plethysmograph_menu_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqPlethysmographMenu *menu = FACQ_PLETHYSMOGRAPH_MENU(self);

	switch(property_id){
	case PROP_MENUBAR: g_value_set_pointer(value,menu->priv->menubar);
	break;
	case PROP_DATA: g_value_set_pointer(value,menu->priv->data);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(menu,property_id,pspec);
	}
}

static void facq_plethysmograph_menu_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqPlethysmographMenu *menu = FACQ_PLETHYSMOGRAPH_MENU(self);

	switch(property_id){
	case PROP_MENUBAR: menu->priv->menubar = g_value_get_pointer(value);
	break;
	case PROP_DATA: menu->priv->data = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(menu,property_id,pspec);
	}
}

static void facq_plethysmograph_menu_finalize(GObject *self)
{
	FacqPlethysmographMenu *menu = FACQ_PLETHYSMOGRAPH_MENU(self);

	if(GTK_IS_WIDGET(menu->priv->menubar))
		gtk_widget_destroy(menu->priv->menubar);

	G_OBJECT_CLASS(facq_plethysmograph_menu_parent_class)->finalize(self);
}

static void facq_plethysmograph_menu_constructed(GObject *self)
{
	FacqPlethysmographMenu *oscmenu = FACQ_PLETHYSMOGRAPH_MENU(self);
	GtkWidget *menubar = NULL , *menu = NULL , *menuitem = NULL;

	menubar = gtk_menu_bar_new();

	//Plethysmograph submenu
	menu = gtk_menu_new();

#if GTK_MAJOR_VERSION > 2
	menuitem = gtk_menu_item_new_with_label(_("_Quit"));
#else
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT,NULL);
#endif
	g_signal_connect(menuitem,"activate",
				G_CALLBACK(gtk_main_quit),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	gtk_widget_show(menuitem);

	menuitem = gtk_menu_item_new_with_label(_("Plethysmograph"));
	gtk_widget_show(menuitem);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	//Plug submenu
	menu = gtk_menu_new();

#if GTK_MAJOR_VERSION > 2
	menuitem = gtk_menu_item_new_with_label(_("_Preferences"));
#else
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES,NULL);
#endif
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
				G_CALLBACK(facq_plethysmograph_menu_callback_plug_preferences),oscmenu->priv->data);
	gtk_widget_show(menuitem);
	oscmenu->priv->plug_preferences = menuitem;

#if GTK_MAJOR_VERSION > 2
	menuitem = gtk_menu_item_new_with_label(_("Disconnect"));
#else
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_DISCONNECT,NULL);
#endif

	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	gtk_widget_show(menuitem);
	gtk_widget_set_sensitive(GTK_WIDGET(menuitem),FALSE);
	g_signal_connect(menuitem,"activate",
				G_CALLBACK(facq_plethysmograph_menu_callback_disconnect),oscmenu->priv->data);
	oscmenu->priv->disconnect = menuitem;

	menuitem = gtk_menu_item_new_with_label(_("Plug"));
	gtk_widget_show(menuitem);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	//Help submenu right justified
	menu = gtk_menu_new();

#if GTK_MAJOR_VERSION > 2
	menuitem = gtk_menu_item_new_with_label(_("_About"));
#else
        menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT,NULL);
#endif

        g_signal_connect(menuitem,"activate",
                G_CALLBACK(facq_plethysmograph_menu_callback_about),oscmenu->priv->data);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
        gtk_widget_show(menuitem);

        menuitem = gtk_menu_item_new_with_label(_("Help"));
#if GTK_MAJOR_VERSION > 2

#else
        gtk_menu_item_set_right_justified(GTK_MENU_ITEM(menuitem),TRUE);
#endif
        gtk_widget_show(menuitem);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	gtk_widget_show_all(menubar);

	oscmenu->priv->menubar = menubar;
}

static void facq_plethysmograph_menu_class_init(FacqPlethysmographMenuClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqPlethysmographMenuPrivate));

	object_class->set_property = facq_plethysmograph_menu_set_property;
	object_class->get_property = facq_plethysmograph_menu_get_property;
	object_class->constructed = facq_plethysmograph_menu_constructed;
	object_class->finalize = facq_plethysmograph_menu_finalize;

	g_object_class_install_property(object_class,PROP_MENUBAR,
					g_param_spec_pointer("menubar",
							     "The menubar",
							     "The menubar widget top container",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));
	
	g_object_class_install_property(object_class,PROP_DATA,
					g_param_spec_pointer("data",
							     "Data",
							     "A pointer to useful data for the callbacks",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));
}

static void facq_plethysmograph_menu_init(FacqPlethysmographMenu *menu)
{
	menu->priv = G_TYPE_INSTANCE_GET_PRIVATE(menu,FACQ_TYPE_PLETHYSMOGRAPH_MENU,FacqPlethysmographMenuPrivate);
	menu->priv->menubar = NULL;
	menu->priv->data = NULL;
}

/**
 * facq_plethysmograph_menu_new:
 * @data: A pointer to some data that will be passed to the callbacks,
 * in the plethysmograph this is a pointer to a #FacqPlethysmograph object.
 *
 * Creates a new #FacqPlethysmographMenu object.
 *
 * Returns: A new #FacqPlethysmographMenu object.
 */
FacqPlethysmographMenu *facq_plethysmograph_menu_new(gpointer data)
{
	return g_object_new(FACQ_TYPE_PLETHYSMOGRAPH_MENU,"data",data,NULL);
}

/**
 * facq_plethysmograph_menu_get_widget:
 * @menu: A #FacqPlethysmographMenu object.
 *
 * Gets the top level widget so you can add the menu to the application.
 *
 * Returns: A #GtkMenuBar, that is the top container for the menu.
 */
GtkWidget *facq_plethysmograph_menu_get_widget(const FacqPlethysmographMenu *menu)
{
	g_return_val_if_fail(FACQ_IS_PLETHYSMOGRAPH_MENU(menu),NULL);
	return menu->priv->menubar;
}

/**
 * facq_plethysmograph_menu_disable_plug_preferences:
 * @menu: A #FacqPlethysmographMenu object.
 *
 * Disables the plug preferences button, after calling this function the
 * user can't press the button.
 *
 */
void facq_plethysmograph_menu_disable_plug_preferences(FacqPlethysmographMenu *menu)
{
	g_return_if_fail(FACQ_IS_PLETHYSMOGRAPH_MENU(menu));
	gtk_widget_set_sensitive(menu->priv->plug_preferences,FALSE);
}

/**
 * facq_plethysmograph_menu_disable_disconnect:
 * @menu: A #FacqPlethysmographMenu object.
 *
 * Disables the disconnect button, after calling this function the
 * user can't press the button.
 */
void facq_plethysmograph_menu_disable_disconnect(FacqPlethysmographMenu *menu)
{
	g_return_if_fail(FACQ_IS_PLETHYSMOGRAPH_MENU(menu));
	gtk_widget_set_sensitive(menu->priv->disconnect,FALSE);
}

/**
 * facq_plethysmograph_menu_enable_plug_preferences:
 * @menu: A #FacqPlethysmographMenu object.
 *
 * Enables the plug preferences button, after calling this function the
 * user can press the button.
 */
void facq_plethysmograph_menu_enable_plug_preferences(FacqPlethysmographMenu *menu)
{
	g_return_if_fail(FACQ_IS_PLETHYSMOGRAPH_MENU(menu));
	gtk_widget_set_sensitive(menu->priv->plug_preferences,TRUE);
}

/**
 * facq_plethysmograph_menu_enable_disconnect:
 * @menu: A #FacqPlethysmographMenu object.
 *
 * Enables the disconnect button, after calling this function the user can
 * press the button.
 */
void facq_plethysmograph_menu_enable_disconnect(FacqPlethysmographMenu *menu)
{
	g_return_if_fail(FACQ_IS_PLETHYSMOGRAPH_MENU(menu));
	gtk_widget_set_sensitive(menu->priv->disconnect,TRUE);
}

/**
 * facq_plethysmograph_menu_free:
 * @menu: A #FacqPlethysmographMenu object.
 *
 * Destroys a no longer needed #FacqPlethysmographMenu object.
 */
void facq_plethysmograph_menu_free(FacqPlethysmographMenu *menu)
{
	g_return_if_fail(FACQ_IS_PLETHYSMOGRAPH_MENU(menu));
	g_object_unref(G_OBJECT(menu));
}

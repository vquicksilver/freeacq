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
#include "facqoscopemenu.h"
#include "facqoscopemenucallbacks.h"

/**
 * SECTION:facqoscopemenu
 * @short_description: Provides the menu for the oscilloscope application.
 * @include:facqoscopemenu.h
 *
 * Provides the menu for the oscilloscope application, including the
 * Oscilloscope, Plug, Zoom and Help submenus and entries.
 *
 * To create a #FacqOscopeMenu object you can use facq_oscope_menu_new(),
 * to obtain the top level widget so you can add the menu to the application use
 * facq_oscope_menu_get_widget(), and to control the status of the different
 * entries in the menu you can use the enable/disable functions.
 *
 * Finally to destroy a no longer needed #FacqOscopeMenu object call
 * facq_oscope_menu_free().
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * Internally #FacqOscopeMenu uses a #GtkMenuBar and various #GtkMenu objects
 * along various #GtkMenuItem objects.
 * </para>
 * </sect1>
 */

/**
 * FacqOscopeMenu:
 *
 * Contains the private details of #FacqOscopeMenu.
 */

/**
 * FacqOscopeMenuClass:
 *
 * Class for all the #FacqOscopeMenu objects.
 */

G_DEFINE_TYPE(FacqOscopeMenu,facq_oscope_menu,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_MENUBAR,
	PROP_DATA
};

struct _FacqOscopeMenuPrivate {
	GtkWidget *menubar;
	gpointer data;
	GtkWidget *disconnect;
	GtkWidget *preferences;
	GtkWidget *zoom_in;
	GtkWidget *zoom_out;
	GtkWidget *zoom_home;
};

/*****--- GObject magic ---*****/
static void facq_oscope_menu_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqOscopeMenu *menu = FACQ_OSCOPE_MENU(self);

	switch(property_id){
	case PROP_MENUBAR: g_value_set_pointer(value,menu->priv->menubar);
	break;
	case PROP_DATA: g_value_set_pointer(value,menu->priv->data);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(menu,property_id,pspec);
	}
}

static void facq_oscope_menu_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqOscopeMenu *menu = FACQ_OSCOPE_MENU(self);

	switch(property_id){
	case PROP_MENUBAR: menu->priv->menubar = g_value_get_pointer(value);
	break;
	case PROP_DATA: menu->priv->data = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(menu,property_id,pspec);
	}
}

static void facq_oscope_menu_finalize(GObject *self)
{
	FacqOscopeMenu *menu = FACQ_OSCOPE_MENU(self);

	if(GTK_IS_WIDGET(menu->priv->menubar))
		gtk_widget_destroy(menu->priv->menubar);

	G_OBJECT_CLASS(facq_oscope_menu_parent_class)->finalize(self);
}

static void facq_oscope_menu_constructed(GObject *self)
{
	FacqOscopeMenu *oscmenu = FACQ_OSCOPE_MENU(self);
	GtkWidget *menubar = NULL , *menu = NULL , *menuitem = NULL;

	menubar = gtk_menu_bar_new();

	//Oscilloscope submenu
	menu = gtk_menu_new();

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT,NULL);
	g_signal_connect(menuitem,"activate",
				G_CALLBACK(gtk_main_quit),NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	gtk_widget_show(menuitem);

	menuitem = gtk_menu_item_new_with_label(_("Oscilloscope"));
	gtk_widget_show(menuitem);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	//Plug submenu
	menu = gtk_menu_new();

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES,NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
				G_CALLBACK(facq_oscope_menu_callback_preferences),oscmenu->priv->data);
	gtk_widget_show(menuitem);
	oscmenu->priv->preferences = menuitem;

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_DISCONNECT,NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	gtk_widget_show(menuitem);
	gtk_widget_set_sensitive(GTK_WIDGET(menuitem),FALSE);
	g_signal_connect(menuitem,"activate",
				G_CALLBACK(facq_oscope_menu_callback_disconnect),oscmenu->priv->data);
	oscmenu->priv->disconnect = menuitem;

	menuitem = gtk_menu_item_new_with_label(_("Plug"));
	gtk_widget_show(menuitem);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	//Zoom submenu
	menu = gtk_menu_new();

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ZOOM_IN,NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
				G_CALLBACK(facq_oscope_menu_callback_zoom_in),oscmenu->priv->data);
	gtk_widget_show(menuitem);
	oscmenu->priv->zoom_in = menuitem;

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ZOOM_OUT,NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
				G_CALLBACK(facq_oscope_menu_callback_zoom_out),oscmenu->priv->data);
	gtk_widget_show(menuitem);
	oscmenu->priv->zoom_out = menuitem;

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ZOOM_100,NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
				G_CALLBACK(facq_oscope_menu_callback_zoom_100),oscmenu->priv->data);
	gtk_widget_show(menuitem);
	oscmenu->priv->zoom_home = menuitem;

	menuitem = gtk_menu_item_new_with_label(_("Zoom"));
	gtk_widget_show(menuitem);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	//Help submenu right justified
	menu = gtk_menu_new();

        menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT,NULL);
        g_signal_connect(menuitem,"activate",
                G_CALLBACK(facq_oscope_menu_callback_about),oscmenu->priv->data);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
        gtk_widget_show(menuitem);

        menuitem = gtk_menu_item_new_with_label(_("Help"));
        gtk_menu_item_set_right_justified(GTK_MENU_ITEM(menuitem),TRUE);
        gtk_widget_show(menuitem);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	gtk_widget_show_all(menubar);

	oscmenu->priv->menubar = menubar;
}

static void facq_oscope_menu_class_init(FacqOscopeMenuClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqOscopeMenuPrivate));

	object_class->set_property = facq_oscope_menu_set_property;
	object_class->get_property = facq_oscope_menu_get_property;
	object_class->constructed = facq_oscope_menu_constructed;
	object_class->finalize = facq_oscope_menu_finalize;

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

static void facq_oscope_menu_init(FacqOscopeMenu *menu)
{
	menu->priv = G_TYPE_INSTANCE_GET_PRIVATE(menu,FACQ_TYPE_OSCOPE_MENU,FacqOscopeMenuPrivate);
	menu->priv->menubar = NULL;
	menu->priv->data = NULL;
}

/**
 * facq_oscope_menu_new:
 * @data: A pointer to some data that you want to pass to the callback
 * functions. In the oscope application this is a pointer to a #FacqOscope
 * object.
 *
 * Creates a new #FacqOscopeMenu object.
 *
 * Returns: A new #FacqOscopeMenu object.
 */
FacqOscopeMenu *facq_oscope_menu_new(gpointer data)
{
	return g_object_new(FACQ_TYPE_OSCOPE_MENU,"data",data,NULL);
}

/**
 * facq_oscope_menu_get_widget:
 * @menu: A #FacqOscopeMenu object.
 *
 * Gets the top level widget so you can add the menu to the application.
 *
 * Returns: A #GtkWidget pointing to a #GtkMenuBar, 
 * that is the top container for the menu.
 */
GtkWidget *facq_oscope_menu_get_widget(const FacqOscopeMenu *menu)
{
	g_return_val_if_fail(FACQ_IS_OSCOPE_MENU(menu),NULL);
	return menu->priv->menubar;
}

/**
 * facq_oscope_menu_disable_preferences:
 * @menu: A #FacqOscopeMenu object.
 *
 * Disables the Preferences entry in the Plug submenu, the user won't be able to
 * press it until enabled again.
 */
void facq_oscope_menu_disable_preferences(FacqOscopeMenu *menu)
{
	g_return_if_fail(FACQ_IS_OSCOPE_MENU(menu));
	gtk_widget_set_sensitive(menu->priv->preferences,FALSE);
}

/**
 * facq_oscope_menu_disable_disconnect:
 * @menu: A #FacqOscopeMenu object.
 *
 * Disables the Disconnect entry in the Plug submenu, the user won't be able to
 * press it until enabled again.
 */
void facq_oscope_menu_disable_disconnect(FacqOscopeMenu *menu)
{
	g_return_if_fail(FACQ_IS_OSCOPE_MENU(menu));
	gtk_widget_set_sensitive(menu->priv->disconnect,FALSE);
}

/**
 * facq_oscope_menu_enable_preferences:
 * @menu: A #FacqOscopeMenu object.
 *
 * Enables the Preferences entry in the Plug submenu, the user will be able to
 * press it again.
 */
void facq_oscope_menu_enable_preferences(FacqOscopeMenu *menu)
{
	g_return_if_fail(FACQ_IS_OSCOPE_MENU(menu));
	gtk_widget_set_sensitive(menu->priv->preferences,TRUE);
}


/**
 * facq_oscope_menu_enable_zoom_in:
 * @menu: A #FacqOscopeMenu object.
 *
 * Enables the Zoom In entry in the Zoom submenu, the user will be able to
 * press it again.
 */
void facq_oscope_menu_enable_zoom_in(FacqOscopeMenu *menu)
{
	g_return_if_fail(FACQ_IS_OSCOPE_MENU(menu));
	gtk_widget_set_sensitive(menu->priv->zoom_in,TRUE);
}

/**
 * facq_oscope_menu_enable_zoom_out:
 * @menu: A #FacqOscopeMenu object.
 *
 * Enables the Zoom Out entry in the Zoom submenu, the user will be able to
 * press it again.
 */
void facq_oscope_menu_enable_zoom_out(FacqOscopeMenu *menu)
{
	g_return_if_fail(FACQ_IS_OSCOPE_MENU(menu));
	gtk_widget_set_sensitive(menu->priv->zoom_out,TRUE);
}

/**
 * facq_oscope_menu_enable_zoom_home:
 * @menu: A #FacqOscopeMenu object.
 *
 * Enables the Normal size entry in the Zoom submenu, the user will be able to
 * press it again.
 */
void facq_oscope_menu_enable_zoom_home(FacqOscopeMenu *menu)
{
	g_return_if_fail(FACQ_IS_OSCOPE_MENU(menu));
	gtk_widget_set_sensitive(menu->priv->zoom_home,TRUE);
}

/**
 * facq_oscope_menu_enable_disconnect:
 * @menu: A #FacqOscopeMenu object.
 *
 * Enables the Disconnect entry in the Plug submenu, the user will be able to
 * press it again.
 */
void facq_oscope_menu_enable_disconnect(FacqOscopeMenu *menu)
{
	g_return_if_fail(FACQ_IS_OSCOPE_MENU(menu));
	gtk_widget_set_sensitive(menu->priv->disconnect,TRUE);
}

/**
 * facq_oscope_menu_disable_zoom_in:
 * @menu: A #FacqOscopeMenu object.
 *
 * Disables the Zoom In entry in the Zoom submenu, the user won't be able to
 * press it until enabled again.
 */
void facq_oscope_menu_disable_zoom_in(FacqOscopeMenu *menu)
{
	g_return_if_fail(FACQ_IS_OSCOPE_MENU(menu));
	gtk_widget_set_sensitive(menu->priv->zoom_in,TRUE);
}

/**
 * facq_oscope_menu_disable_zoom_out:
 * @menu: A #FacqOscopeMenu object.
 *
 * Disables the Zoom Out entry in the Zoom submenu, the user won't be able to
 * press it until enabled again.
 */
void facq_oscope_menu_disable_zoom_out(FacqOscopeMenu *menu)
{
	g_return_if_fail(FACQ_IS_OSCOPE_MENU(menu));
	gtk_widget_set_sensitive(menu->priv->zoom_out,TRUE);
}

/**
 * facq_oscope_menu_disable_zoom_home:
 * @menu: A #FacqOscopeMenu object.
 *
 * Disables the Normal Size entry in the Zoom submenu, the user won't be able to
 * press it until enabled again.
 */
void facq_oscope_menu_disable_zoom_home(FacqOscopeMenu *menu)
{
	g_return_if_fail(FACQ_IS_OSCOPE_MENU(menu));
	gtk_widget_set_sensitive(menu->priv->zoom_home,TRUE);
}

/**
 * facq_oscope_menu_free:
 * @menu: A #FacqOscopeMenu object.
 *
 * Destroys a no longer needed #FacqOscopeMenu object.
 */
void facq_oscope_menu_free(FacqOscopeMenu *menu)
{
	g_return_if_fail(FACQ_IS_OSCOPE_MENU(menu));
	g_object_unref(G_OBJECT(menu));
}

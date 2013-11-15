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
#include "facqcapturemenucallbacks.h"
#include "facqcapturemenu.h"

/**
 * SECTION:facqcapturemenu
 * @short_description: Provides the capture's menu.
 * @include:facqcapturemenu.h
 *
 * Provides the menu for the capture application, including the Capture,
 * Stream, Control, Log and Help entries.
 *
 * To create a new #FacqCaptureMenu use the facq_capture_menu_new() function,
 * to obtain the top level widget so you can add it to the capture application
 * use facq_capture_menu_get_widget(), and to control the status of the entries
 * in the menu you can use the enable/disable function.
 *
 * Finally to destroy a no longer needed #FacqCaptureMenu use
 * facq_capture_menu_free().
 */

/**
 * FacqCaptureMenu:
 *
 * Hides all the private details of #FacqCaptureMenu.
 */

/**
 * FacqCaptureMenuClass:
 *
 * Class for all the #FacqCaptureMenu objects.
 */

G_DEFINE_TYPE(FacqCaptureMenu,facq_capture_menu,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_MENUBAR,
	PROP_DATA
};

struct _FacqCaptureMenuPrivate {
	GtkWidget *menubar;
	gpointer data;
	GtkWidget *add;
	GtkWidget *remove;
	GtkWidget *clear;
	GtkWidget *play;
	GtkWidget *stop;
	GtkWidget *preferences;
	GtkWidget *new;
	GtkWidget *open;
	GtkWidget *save_as;
	GtkWidget *close;
};

static void facq_capture_menu_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqCaptureMenu *menu = FACQ_CAPTURE_MENU(self);

	switch(property_id){
	case PROP_MENUBAR: menu->priv->menubar = g_value_get_pointer(value);
	break;
	case PROP_DATA: menu->priv->data = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(menu,property_id,pspec);
	}
}

static void facq_capture_menu_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqCaptureMenu *menu = FACQ_CAPTURE_MENU(self);

	switch(property_id){
	case PROP_MENUBAR: g_value_set_pointer(value,menu->priv->menubar);
	break;
	case PROP_DATA: g_value_set_pointer(value,menu->priv->data);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(menu,property_id,pspec);
	}
}

static void facq_capture_menu_constructed(GObject *self)
{
	FacqCaptureMenu *capmenu = FACQ_CAPTURE_MENU(self);
	GtkWidget *menubar = NULL , *menu = NULL, *menuitem = NULL;

	menubar = gtk_menu_bar_new();

	//Capture menu 
	menu = gtk_menu_new();

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT,NULL);
        g_signal_connect(menuitem,"activate",
                                G_CALLBACK(gtk_main_quit),NULL);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
        gtk_widget_show(menuitem);

	menuitem = gtk_menu_item_new_with_label(_("Capture"));
        gtk_widget_show(menuitem);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	//Stream
	menu = gtk_menu_new();

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES,NULL);
	capmenu->priv->preferences = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
				G_CALLBACK(facq_capture_menu_callback_preferences),capmenu->priv->data);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW,NULL);
	capmenu->priv->new = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_capture_menu_callback_new),capmenu->priv->data);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN,NULL);
	capmenu->priv->open = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_capture_menu_callback_open),capmenu->priv->data);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE_AS,NULL);
	capmenu->priv->save_as = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_capture_menu_callback_save_as),capmenu->priv->data);
	gtk_widget_set_sensitive(menuitem,FALSE);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_CLOSE,NULL);
	capmenu->priv->close = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_capture_menu_callback_close),capmenu->priv->data);
	gtk_widget_set_sensitive(GTK_WIDGET(menuitem),FALSE);
	gtk_widget_show(menuitem);

	menuitem = gtk_menu_item_new_with_label(_("Stream"));
	gtk_widget_show(menuitem);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	//Control
	menu = gtk_menu_new();
	
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_PLAY,NULL);
	capmenu->priv->play = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_capture_menu_callback_play),capmenu->priv->data);
	gtk_widget_set_sensitive(menuitem,FALSE);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_STOP,NULL);
	capmenu->priv->stop = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_capture_menu_callback_stop),capmenu->priv->data);
	gtk_widget_set_sensitive(GTK_WIDGET(menuitem),FALSE);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ADD,NULL);
	capmenu->priv->add = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_capture_menu_callback_add),capmenu->priv->data);
	gtk_widget_set_sensitive(GTK_WIDGET(menuitem),FALSE);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_REMOVE,NULL);
	capmenu->priv->remove = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_capture_menu_callback_remove),capmenu->priv->data);
	gtk_widget_set_sensitive(GTK_WIDGET(menuitem),FALSE);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_CLEAR,NULL);
	capmenu->priv->clear = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_capture_menu_callback_clear),capmenu->priv->data);
	gtk_widget_set_sensitive(GTK_WIDGET(menuitem),FALSE);
	gtk_widget_show(menuitem);

	menuitem = gtk_menu_item_new_with_label(_("Control"));
	gtk_widget_show(menuitem);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);
	
	//Log
	menu = gtk_menu_new();

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_FIND,NULL);
        gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem),_("Read"));
	g_signal_connect(menuitem,"activate",
			G_CALLBACK(facq_capture_menu_callback_log),capmenu->priv->data);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	gtk_widget_show(menuitem);

	menuitem = gtk_menu_item_new_with_label(_("Log"));
	gtk_widget_show(menuitem);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	//Help menu
	menu = gtk_menu_new();

        menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT,NULL);
        g_signal_connect(menuitem,"activate",
                G_CALLBACK(facq_capture_menu_callback_about),capmenu->priv->data);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
        gtk_widget_show(menuitem);

        menuitem = gtk_menu_item_new_with_label(_("Help"));
        gtk_menu_item_set_right_justified(GTK_MENU_ITEM(menuitem),TRUE);
        gtk_widget_show(menuitem);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	gtk_widget_show_all(menubar);

	capmenu->priv->menubar = menubar;
}

static void facq_capture_menu_finalize(GObject *self)
{
	FacqCaptureMenu *menu = FACQ_CAPTURE_MENU(self);

	if(GTK_IS_WIDGET(menu->priv->menubar))
		gtk_widget_destroy(menu->priv->menubar);

	G_OBJECT_CLASS(facq_capture_menu_parent_class)->finalize(self);
}

static void facq_capture_menu_class_init(FacqCaptureMenuClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqCaptureMenuPrivate));

	object_class->set_property = facq_capture_menu_set_property;
	object_class->get_property = facq_capture_menu_get_property;
	object_class->constructed = facq_capture_menu_constructed;
	object_class->finalize = facq_capture_menu_finalize;

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

static void facq_capture_menu_init(FacqCaptureMenu *menu)
{
	menu->priv = G_TYPE_INSTANCE_GET_PRIVATE(menu,FACQ_TYPE_CAPTURE_MENU,FacqCaptureMenuPrivate);
	menu->priv->data = NULL;
}

/***** Public methods *****/

/**
 * facq_capture_menu_new:
 * @data: A pointer to some data that will be passed to the callbacks,
 * in the capture this is a pointer to a #FacqCapture object.
 *
 * Creates a new #FacqCaptureMenu object.
 *
 * Returns: A new #FacqCaptureMenu object.
 */
FacqCaptureMenu *facq_capture_menu_new(gpointer data)
{
	return g_object_new(FACQ_TYPE_CAPTURE_MENU,"data",data,NULL);
}

/**
 * facq_capture_menu_get_widget:
 * @menu: A #FacqCaptureMenu object.
 *
 * Gets the top level widget so you can add the menu to the application.
 *
 * Returns: A #GtkMenuBar, that is the top level container for the menu.
 */
GtkWidget *facq_capture_menu_get_widget(const FacqCaptureMenu *menu)
{
	g_return_val_if_fail(FACQ_IS_CAPTURE_MENU(menu),NULL);
	return menu->priv->menubar;
}

/**
 * facq_capture_menu_enable_add:
 * @menu: A #FacqCaptureMenu object.
 *
 * Enable the Control->Add entry in the menu. The user will be able to
 * press it again.
 */
void facq_capture_menu_enable_add(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->add,TRUE);
}

/**
 * facq_capture_menu_enable_remove:
 * @menu: A #FacqCaptureMenu object.
 *
 * Enable the Control->Remove entry in the menu. The user will be able to
 * press it again.
 */
void facq_capture_menu_enable_remove(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->remove,TRUE);
}

/**
 * facq_capture_menu_enable_clear:
 * @menu: A #FacqCaptureMenu object.
 *
 * Enable the Control->Clear entry in the menu. The user will be able to
 * press it again.
 */
void facq_capture_menu_enable_clear(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->clear,TRUE);
}

/**
 * facq_capture_menu_enable_play:
 * @menu: A #FacqCaptureMenu object.
 *
 * Enable the Stream->Play entry in the menu. The user will be able to
 * press it again.
 */
void facq_capture_menu_enable_play(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->play,TRUE);
}

/**
 * facq_capture_menu_enable_stop:
 * @menu: A #FacqCaptureMenu object.
 *
 * Enable the Stream->Stop entry in the menu. The user will be able to
 * press it again.
 */
void facq_capture_menu_enable_stop(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->stop,TRUE);
}

/**
 * facq_capture_menu_enable_preferences:
 * @menu: A #FacqCaptureMenu object.
 *
 * Enable the Stream->Preferences entry in the menu. The user will be able to
 * press it again.
 */
void facq_capture_menu_enable_preferences(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->preferences,TRUE);
}

/**
 * facq_capture_menu_enable_new:
 * @menu: A #FacqCaptureMenu object.
 *
 * Enable the Stream->New entry in the menu. The user will be able to
 * press it again.
 */
void facq_capture_menu_enable_new(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->new,TRUE);
}

/**
 * facq_capture_menu_enable_open:
 * @menu: A #FacqCaptureMenu object.
 *
 * Enable the Stream->Open entry in the menu. The user will be able to
 * press it again.
 */
void facq_capture_menu_enable_open(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->open,TRUE);
}

/**
 * facq_capture_menu_enable_save_as:
 * @menu: A #FacqCaptureMenu object.
 *
 * Enables the Stream->Save as entry in the menu. The user will be able to
 * press it again.
 */
void facq_capture_menu_enable_save_as(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->save_as,TRUE);
}

/**
 * facq_capture_menu_enable_close:
 * @menu: A #FacqCaptureMenu object.
 *
 * Enables the Control->Close entry in the menu. The user will be able to
 * press it again.
 */
void facq_capture_menu_enable_close(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->close,TRUE);
}

/**
 * facq_capture_menu_disable_add:
 * @menu: A #FacqCaptureMenu object.
 *
 * Disable the Control->Add entry in the menu. The user won't be able to
 * press it until enabled again.
 */
void facq_capture_menu_disable_add(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->add,FALSE);
}

/**
 * facq_capture_menu_disable_remove:
 * @menu: A #FacqCaptureMenu object.
 *
 * Disable the Control->Remove entry in the menu. The user won't be able to
 * press it until enabled again.
 */
void facq_capture_menu_disable_remove(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->remove,FALSE);
}

/**
 * facq_capture_menu_disable_clear:
 * @menu: A #FacqCaptureMenu object.
 *
 * Disable the Control->Clear entry in the menu. The user won't be able to
 * press it until enabled again.
 */
void facq_capture_menu_disable_clear(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->clear,FALSE);
}

/**
 * facq_capture_menu_disable_play:
 * @menu: A #FacqCaptureMenu object.
 *
 * Disable the Control->Play entry in the menu. The user won't be able to
 * press it until enabled again.
 */
void facq_capture_menu_disable_play(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->play,FALSE);
}

/**
 * facq_capture_menu_disable_stop:
 * @menu: A #FacqCaptureMenu object.
 *
 * Disable the Control->Stop entry in the menu. The user won't be able
 * to press it until enabled again.
 */
void facq_capture_menu_disable_stop(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->stop,FALSE);
}

/**
 * facq_capture_menu_disable_preferences:
 * @menu: A #FacqCaptureMenu object.
 *
 * Disables the Stream->Preferences entry in the menu. The user won't be able
 * to press it until enabled again.
 */
void facq_capture_menu_disable_preferences(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->preferences,FALSE);
}

/**
 * facq_capture_menu_disable_new:
 * @menu: A #FacqCaptureMenu object.
 *
 * Disables the Stream->New entry in the menu. The user won't be able
 * to press it until enabled again.
 */
void facq_capture_menu_disable_new(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->new,FALSE);
}

/**
 * facq_capture_menu_disable_open:
 * @menu: A #FacqCaptureMenu object.
 *
 * Disables the Stream->Open entry in the menu. The user won't be able
 * to press it until enabled again.
 */
void facq_capture_menu_disable_open(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->open,FALSE);
}

/**
 * facq_capture_menu_disable_save_as:
 * @menu: A #FacqCaptureMenu object.
 *
 * Disables the Stream->Save as entry in the menu. The user won't be able
 * to press it until enabled again.
 */
void facq_capture_menu_disable_save_as(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->save_as,FALSE);
}

/**
 * facq_capture_menu_disable_close:
 * @menu: A #FacqCaptureMenu object.
 *
 * Disables the Stream->Close entry in the menu. The user won't be able to
 * press it until enabled again.
 */
void facq_capture_menu_disable_close(FacqCaptureMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->close,FALSE);
}

/**
 * facq_capture_menu_free:
 * @menu: A #FacqCaptureMenu object.
 *
 * Destroys a no longer needed #FacqCaptureMenu object.
 */
void facq_capture_menu_free(FacqCaptureMenu *menu)
{
	g_return_if_fail(FACQ_IS_CAPTURE_MENU(menu));
	g_object_unref(G_OBJECT(menu));
}

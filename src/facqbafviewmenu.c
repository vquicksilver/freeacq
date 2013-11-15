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
#include "facqbafviewmenucallbacks.h"
#include "facqbafviewmenu.h"

/**
 * SECTION:facqbafviewmenu
 * @short_description: Provides the menu for the binary acquisition file viewer
 * program.
 * @include:facqbafviewmenu.h
 *
 * Provides the menu for the binary acquisition file viewer application,
 * including the Viewer, File, Page, Zoom and Help submenus.
 *
 * To create a #FacqBAFViewMenu use facq_baf_view_menu_new(), to obtain the
 * top level widget so you can add it to the application use
 * facq_baf_view_menu_get_widget(), and to control the status of the different
 * entries in the submenus you can use the different enable/disable functions.
 *
 * Finally to destroy the menu simply call facq_baf_view_menu_free().
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * Internally #FacqBAFViewMenu uses a #GtkMenuBar and various #GtkMenu
 * objects along various #GtkMenuItem objects.
 * </para>
 * </sect1>
 */

/**
 * FacqBAFViewMenu:
 *
 * Contains the private details of #FacqBAFViewMenu.
 */

/**
 * FACQBAFViewMenuClass:
 *
 * Class for the #FacqBAFViewMenu objects.
 */
G_DEFINE_TYPE(FacqBAFViewMenu,facq_baf_view_menu,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_MENUBAR,
	PROP_DATA
};

struct _FacqBAFViewMenuPrivate {
	GtkWidget *menubar;
	gpointer data;
	GtkWidget *gotofirst;
	GtkWidget *gotolast;
	GtkWidget *goforward;
	GtkWidget *gobackward;
	GtkWidget *pagesetup;
	GtkWidget *close;
	GtkWidget *save_as;
	gdouble total_pages;
};

static void facq_baf_view_menu_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqBAFViewMenu *menu = FACQ_BAF_VIEW_MENU(self);

	switch(property_id){
	case PROP_MENUBAR: menu->priv->menubar = g_value_get_pointer(value);
	break;
	case PROP_DATA: menu->priv->data = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(menu,property_id,pspec);
	}
}

static void facq_baf_view_menu_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqBAFViewMenu *menu = FACQ_BAF_VIEW_MENU(self);

	switch(property_id){
	case PROP_MENUBAR: g_value_set_pointer(value,menu->priv->menubar);
	break;
	case PROP_DATA: g_value_set_pointer(value,menu->priv->data);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(menu,property_id,pspec);
	}
}

static void facq_baf_view_menu_constructed(GObject *self)
{
	FacqBAFViewMenu *vimenu = FACQ_BAF_VIEW_MENU(self);
	GtkWidget *menubar = NULL , *menu = NULL, *menuitem = NULL;

	menubar = gtk_menu_bar_new();

	//Viewer menu 
	menu = gtk_menu_new();

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT,NULL);
        g_signal_connect(menuitem,"activate",
                                G_CALLBACK(gtk_main_quit),NULL);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
        gtk_widget_show(menuitem);

	menuitem = gtk_menu_item_new_with_label(_("Viewer"));
        gtk_widget_show(menuitem);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	//File
	menu = gtk_menu_new();

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN,NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_baf_view_menu_callback_open),vimenu->priv->data);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE_AS,NULL);
	vimenu->priv->save_as = menuitem;
	gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem),_("Export"));
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_baf_view_menu_callback_save_as),vimenu->priv->data);
	gtk_widget_set_sensitive(menuitem,FALSE);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_CLOSE,NULL);
	vimenu->priv->close = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_baf_view_menu_callback_close),vimenu->priv->data);
	gtk_widget_set_sensitive(GTK_WIDGET(menuitem),FALSE);
	gtk_widget_show(menuitem);

	menuitem = gtk_menu_item_new_with_label(_("File"));
	gtk_widget_show(menuitem);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	//Page
	menu = gtk_menu_new();
	
	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_PAGE_SETUP,NULL);
	vimenu->priv->pagesetup = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_baf_view_menu_callback_page_setup),vimenu->priv->data);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_GOTO_FIRST,NULL);
	vimenu->priv->gotofirst = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_baf_view_menu_callback_goto_first),vimenu->priv->data);
	gtk_widget_set_sensitive(GTK_WIDGET(menuitem),FALSE);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_GO_BACK,NULL);
	vimenu->priv->gobackward = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_baf_view_menu_callback_go_back),vimenu->priv->data);
	gtk_widget_set_sensitive(GTK_WIDGET(menuitem),FALSE);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_GO_FORWARD,NULL);
	vimenu->priv->goforward = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_baf_view_menu_callback_go_forward),vimenu->priv->data);
	gtk_widget_set_sensitive(GTK_WIDGET(menuitem),FALSE);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_GOTO_LAST,NULL);
	vimenu->priv->gotolast = menuitem;
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_baf_view_menu_callback_goto_last),vimenu->priv->data);
	gtk_widget_set_sensitive(GTK_WIDGET(menuitem),FALSE);
	gtk_widget_show(menuitem);

	menuitem = gtk_menu_item_new_with_label(_("Page"));
	gtk_widget_show(menuitem);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	//Zoom
	menu = gtk_menu_new();

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ZOOM_IN,NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_baf_view_menu_callback_zoom_in),vimenu->priv->data);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ZOOM_OUT,NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_baf_view_menu_callback_zoom_out),vimenu->priv->data);
	gtk_widget_show(menuitem);

	menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ZOOM_100,NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
	g_signal_connect(menuitem,"activate",
                                G_CALLBACK(facq_baf_view_menu_callback_zoom_100),vimenu->priv->data);
	gtk_widget_show(menuitem);

	menuitem = gtk_menu_item_new_with_label(_("Zoom"));
	gtk_widget_show(menuitem);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	//Help menu
	menu = gtk_menu_new();

        menuitem = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT,NULL);
        g_signal_connect(menuitem,"activate",
                G_CALLBACK(facq_baf_view_menu_callback_about),vimenu->priv->data);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu),menuitem);
        gtk_widget_show(menuitem);

        menuitem = gtk_menu_item_new_with_label(_("Help"));
        gtk_menu_item_set_right_justified(GTK_MENU_ITEM(menuitem),TRUE);
        gtk_widget_show(menuitem);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem),menu);
        gtk_menu_shell_append(GTK_MENU_SHELL(menubar),menuitem);

	gtk_widget_show_all(menubar);

	vimenu->priv->menubar = menubar;
}

static void facq_baf_view_menu_finalize(GObject *self)
{
	FacqBAFViewMenu *menu = FACQ_BAF_VIEW_MENU(self);

	if(GTK_IS_WIDGET(menu->priv->save_as))
		gtk_widget_destroy(menu->priv->save_as);

	if(GTK_IS_WIDGET(menu->priv->close))
		gtk_widget_destroy(menu->priv->close);

	if(GTK_IS_WIDGET(menu->priv->gotofirst))
		gtk_widget_destroy(menu->priv->gotofirst);

	if(GTK_IS_WIDGET(menu->priv->goforward))
		gtk_widget_destroy(menu->priv->goforward);

	if(GTK_IS_WIDGET(menu->priv->gobackward))
		gtk_widget_destroy(menu->priv->gobackward);

	if(GTK_IS_WIDGET(menu->priv->gotolast))
		gtk_widget_destroy(menu->priv->gotolast);

	if(GTK_IS_WIDGET(menu->priv->pagesetup))
		gtk_widget_destroy(menu->priv->pagesetup);

	if(GTK_IS_WIDGET(menu->priv->menubar))
		gtk_widget_destroy(menu->priv->menubar);

	G_OBJECT_CLASS(facq_baf_view_menu_parent_class)->finalize(self);
}

static void facq_baf_view_menu_class_init(FacqBAFViewMenuClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqBAFViewMenuPrivate));

	object_class->set_property = facq_baf_view_menu_set_property;
	object_class->get_property = facq_baf_view_menu_get_property;
	object_class->constructed = facq_baf_view_menu_constructed;
	object_class->finalize = facq_baf_view_menu_finalize;

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

static void facq_baf_view_menu_init(FacqBAFViewMenu *menu)
{
	menu->priv = G_TYPE_INSTANCE_GET_PRIVATE(menu,FACQ_TYPE_BAF_VIEW_MENU,FacqBAFViewMenuPrivate);
	menu->priv->data = NULL;
	menu->priv->total_pages = 1;
}

/**
 * facq_baf_view_menu_new:
 * @data: A pointer to some data that you want to pass to the callback
 * functions. In the viewer application this is a pointer to a
 * #FacqBAFView object.
 *
 * Creates a new #FacqBAFViewMenu object.
 *
 * Returns: A new #FacqBAFViewMenu object.
 */
FacqBAFViewMenu *facq_baf_view_menu_new(gpointer data)
{
	return g_object_new(FACQ_TYPE_BAF_VIEW_MENU,"data",data,NULL);
}

/**
 * facq_baf_view_menu_get_widget:
 * @menu: A #FacqBAFViewMenu object.
 *
 * Gets the top level widget so you can add the menu to the application.
 *
 * Returns: A #GtkWidget pointing to a #GtkMenuBar, that is the top container
 * for the menu.
 */
GtkWidget *facq_baf_view_menu_get_widget(const FacqBAFViewMenu *menu)
{
	g_return_val_if_fail(FACQ_IS_BAF_VIEW_MENU(menu),NULL);
	return menu->priv->menubar;
}

/**
 * facq_baf_view_menu_set_total_pages:
 * @menu: A #FacqBAFViewMenu object.
 * @total_pages: The total number of pages.
 *
 * Sets the total number of pages that can be view from the file.
 * This allows to set the state of the entries in the Page submenu.
 */
void facq_baf_view_menu_set_total_pages(FacqBAFViewMenu *menu,gdouble total_pages)
{
	g_return_if_fail(FACQ_IS_BAF_VIEW_MENU(menu));
	menu->priv->total_pages = total_pages;
}

/**
 * facq_baf_view_menu_disable_navigation:
 * @menu: A #FacqBAFViewMenu object.
 *
 * Disables the go to first, go backward, go forward, go to last menu entries on
 * the pages submenu and enables the page setup entry. This should be called
 * each time a file is closed.
 */
void facq_baf_view_menu_disable_navigation(FacqBAFViewMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->gotofirst,FALSE);
	gtk_widget_set_sensitive(menu->priv->gotolast,FALSE);
	gtk_widget_set_sensitive(menu->priv->goforward,FALSE);
	gtk_widget_set_sensitive(menu->priv->gobackward,FALSE);
	gtk_widget_set_sensitive(menu->priv->pagesetup,TRUE);
}

/**
 * facq_baf_view_menu_goto_page:
 * @menu: A #FacqBAFViewMenu object.
 * @page_n: The page number.
 *
 * Enables the go to first, go backward, go forward, and go to last menu entries
 * on the pages submenu, and disable the page setup entry. This function should be called
 * after the file is open, and after calling
 * facq_baf_view_menu_set_total_pages().
 */
void facq_baf_view_menu_goto_page(FacqBAFViewMenu *menu,gdouble page_n)
{
	gtk_widget_set_sensitive(menu->priv->pagesetup,FALSE);

	if(menu->priv->total_pages == 1){
		gtk_widget_set_sensitive(menu->priv->gotofirst,FALSE);
		gtk_widget_set_sensitive(menu->priv->gotolast,FALSE);
		gtk_widget_set_sensitive(menu->priv->goforward,FALSE);
		gtk_widget_set_sensitive(menu->priv->gobackward,FALSE);
		return;
	}
	
	if(page_n == 1 && menu->priv->total_pages > 1){
		gtk_widget_set_sensitive(menu->priv->gotofirst,FALSE);
		gtk_widget_set_sensitive(menu->priv->gotolast,TRUE);
		gtk_widget_set_sensitive(menu->priv->goforward,TRUE);
		gtk_widget_set_sensitive(menu->priv->gobackward,FALSE);
		return;
	}
	if(page_n > 1 && page_n < menu->priv->total_pages){
		gtk_widget_set_sensitive(menu->priv->gotofirst,TRUE);
		gtk_widget_set_sensitive(menu->priv->gotolast,TRUE);
		gtk_widget_set_sensitive(menu->priv->goforward,TRUE);
		gtk_widget_set_sensitive(menu->priv->gobackward,TRUE);
		return;
	}
	if(page_n == menu->priv->total_pages && menu->priv->total_pages > 1){
		gtk_widget_set_sensitive(menu->priv->gotofirst,TRUE);
		gtk_widget_set_sensitive(menu->priv->gotolast,FALSE);
		gtk_widget_set_sensitive(menu->priv->goforward,FALSE);
		gtk_widget_set_sensitive(menu->priv->gobackward,TRUE);
		return;
	}
}

/**
 * facq_baf_view_menu_enable_close:
 * @menu: A #FacqBAFViewMenu object.
 *
 * Enables the close entry in the File submenu. This should be called after
 * opening a file.
 */
void facq_baf_view_menu_enable_close(FacqBAFViewMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->close,TRUE);
}

/**
 * facq_baf_view_menu_disable_close:
 * @menu: A #FacqBAFViewMenu object.
 *
 * Disables the close entry in the file submenu. This should be called after
 * closing a file.
 */
void facq_baf_view_menu_disable_close(FacqBAFViewMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->close,FALSE);
}

/**
 * facq_baf_view_menu_disable_save_as:
 * @menu: A #FacqBAFViewMenu object.
 *
 * Disables the Export entry in the File submenu, the user won't be able to
 * press it until enabled again.
 */
void facq_baf_view_menu_disable_save_as(FacqBAFViewMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->save_as,FALSE);
}

/**
 * facq_baf_view_menu_enable_save_as:
 * @menu: A #FacqBAFViewMenu object.
 *
 * Enables the Export entry in the File submenu, the user will be able to
 * press it.
 */
void facq_baf_view_menu_enable_save_as(FacqBAFViewMenu *menu)
{
	gtk_widget_set_sensitive(menu->priv->save_as,TRUE);
}

/**
 * facq_baf_view_menu_free:
 * @menu: A #FacqBAFViewMenu object.
 *
 * Destroys a no longer needed #FacqBAFViewMenu object.
 */
void facq_baf_view_menu_free(FacqBAFViewMenu *menu)
{
	g_return_if_fail(FACQ_IS_BAF_VIEW_MENU(menu));
	g_object_unref(G_OBJECT(menu));
}

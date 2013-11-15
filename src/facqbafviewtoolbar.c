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
#include "facqbafviewtoolbarcallbacks.h"
#include "facqbafviewtoolbar.h"

/**
 * SECTION:facqbafviewtoolbar
 * @short_description: Provides the toolbar for the binary acquisition file
 * viewer application.
 * @include:facqbafviewtoolbar.h
 *
 * Provides the toolbar for the binary acquisition file viewer application,
 * including the Page setup, First page, Previous page, The page spin button
 * with the total pages label, Next page, Last page, Zoom In, Zoom Out, 
 * and Normal size buttons.
 *
 * Too create a new #FacqBAFViewToolbar use facq_baf_view_toolbar_new()
 * function, to obtain the top level widget use facq_baf_view_toolbar_get_widget()
 * this will allow you to add the toolbar to the application.
 * You can control the information displayed on the toolbar and the status of
 * the buttons with the functions, facq_baf_view_toolbar_set_total_pages(),
 * facq_baf_view_toolbar_read_spin_button(), facq_baf_view_toolbar_goto_page(),
 * facq_baf_view_toolbar_disable_navigation().
 *
 * Finally the destroy the toolbar use facq_baf_view_toolbar_free().
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * A #FacqBAFViewToolbar uses the following objects to provide it's services.
 * A #GtkToolBar along with various #GtkToolItem objects, of #GtkToolButton
 * and #GtkSeparatorToolItem types.
 *
 * Note that the #GtkSpinButton and the label with the number of pages are 
 * packed inside #GtkToolItem objects too.
 * </para>
 * </sect1>
 */

/**
 * FacqBAFViewToolbar:
 *
 * Contains the private details of #FacqBAFViewToolbar.
 */

/**
 * FacqBAFViewToolbarClass:
 *
 * Class for all the #FacqBAFViewToolbar objects.
 */

G_DEFINE_TYPE(FacqBAFViewToolbar,facq_baf_view_toolbar,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_TOOLBAR,
	PROP_DATA
};

struct _FacqBAFViewToolbarPrivate {
	GtkWidget *toolbar;
	GtkWidget *spin_button;
	GtkWidget *label;
	gpointer data;
	gdouble total_pages;
};

/*****--- GObject magic ---*****/
static void facq_baf_view_toolbar_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqBAFViewToolbar *toolbar = FACQ_BAF_VIEW_TOOLBAR(self);

	switch(property_id){
	case PROP_TOOLBAR: toolbar->priv->toolbar = g_value_get_pointer(value);
	break;
	case PROP_DATA: toolbar->priv->data = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(toolbar,property_id,pspec);
	}
}

static void facq_baf_view_toolbar_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqBAFViewToolbar *toolbar = FACQ_BAF_VIEW_TOOLBAR(self);

	switch(property_id){
	case PROP_TOOLBAR: g_value_set_pointer(value,toolbar->priv->toolbar);
	break;
	case PROP_DATA: g_value_set_pointer(value,toolbar->priv->toolbar);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(toolbar,property_id,pspec);
	}
}

static void facq_baf_view_toolbar_constructed(GObject *self)
{
	FacqBAFViewToolbar *bar = FACQ_BAF_VIEW_TOOLBAR(self);

	GtkWidget *toolbar = NULL;
	GtkToolItem *toolitem = NULL;
	GtkSeparatorToolItem *separator = NULL;

	toolbar = gtk_toolbar_new();

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_PAGE_SETUP);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,0);
	gtk_widget_set_sensitive(GTK_WIDGET(toolitem),TRUE);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_baf_view_toolbar_callback_page_setup),bar->priv->data);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_GOTO_FIRST);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,1);
	gtk_widget_set_sensitive(GTK_WIDGET(toolitem),FALSE);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_baf_view_toolbar_callback_goto_first),bar->priv->data);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_GO_BACK);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_baf_view_toolbar_callback_go_back),bar->priv->data);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,2);
	gtk_widget_set_sensitive(GTK_WIDGET(toolitem),FALSE);

	toolitem = gtk_tool_item_new();
	bar->priv->spin_button = gtk_spin_button_new_with_range(1,1,1);
	g_signal_connect(GTK_SPIN_BUTTON(bar->priv->spin_button),"value-changed",
			G_CALLBACK(facq_baf_view_toolbar_callback_intro),bar->priv->data);
	gtk_container_add(GTK_CONTAINER(toolitem),bar->priv->spin_button);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,3);
	gtk_widget_set_sensitive(bar->priv->spin_button,FALSE);

	toolitem = gtk_tool_item_new();
	bar->priv->label = gtk_label_new(_(" of 1 pages"));
	gtk_container_add(GTK_CONTAINER(toolitem),bar->priv->label);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,4);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_GO_FORWARD);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_baf_view_toolbar_callback_go_forward),bar->priv->data);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,5);
	gtk_widget_set_sensitive(GTK_WIDGET(toolitem),FALSE);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_GOTO_LAST);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_baf_view_toolbar_callback_goto_last),bar->priv->data);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,6);
	gtk_widget_set_sensitive(GTK_WIDGET(toolitem),FALSE);

	separator = GTK_SEPARATOR_TOOL_ITEM(gtk_separator_tool_item_new());
	gtk_separator_tool_item_set_draw(separator,TRUE);
	gtk_tool_item_set_expand(GTK_TOOL_ITEM(separator),FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),GTK_TOOL_ITEM(separator),7);

	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ZOOM_IN);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_baf_view_toolbar_callback_zoom_in),bar->priv->data);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,8);
	
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ZOOM_OUT);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_baf_view_toolbar_callback_zoom_out),bar->priv->data);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,9);
	
	toolitem = gtk_tool_button_new_from_stock(GTK_STOCK_ZOOM_100);
	g_signal_connect(toolitem,"clicked",
			G_CALLBACK(facq_baf_view_toolbar_callback_zoom_100),bar->priv->data);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),toolitem,10);

	gtk_widget_show_all(toolbar);

	bar->priv->toolbar = toolbar;
}

static void facq_baf_view_toolbar_finalize(GObject *self)
{
	FacqBAFViewToolbar *toolbar = FACQ_BAF_VIEW_TOOLBAR(self);

	if(GTK_IS_WIDGET(toolbar->priv->toolbar))
		gtk_widget_destroy(toolbar->priv->toolbar);
	if(toolbar->priv->data)
		toolbar->priv->data = NULL;

	G_OBJECT_CLASS (facq_baf_view_toolbar_parent_class)->finalize (self);
}

static void facq_baf_view_toolbar_class_init(FacqBAFViewToolbarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqBAFViewToolbarPrivate));

	object_class->get_property = facq_baf_view_toolbar_get_property;
	object_class->set_property = facq_baf_view_toolbar_set_property;
	object_class->finalize = facq_baf_view_toolbar_finalize;
	object_class->constructed = facq_baf_view_toolbar_constructed;

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

static void facq_baf_view_toolbar_init(FacqBAFViewToolbar *toolbar)
{
	toolbar->priv = G_TYPE_INSTANCE_GET_PRIVATE(toolbar,FACQ_TYPE_BAF_VIEW_TOOLBAR,FacqBAFViewToolbarPrivate);
	toolbar->priv->toolbar = NULL;
	toolbar->priv->data = NULL;
	toolbar->priv->total_pages = 1;
}
/*****--- Private methods ---*****/
static void facq_baf_view_toolbar_change_toolitem(FacqBAFViewToolbar *toolbar,const gchar *stock_id,gboolean sensitive)
{
	GList *list = NULL;
	GtkToolItem *item = NULL;
	const gchar *current_id = NULL;

	g_return_if_fail(FACQ_IS_BAF_VIEW_TOOLBAR(toolbar));
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
 * facq_baf_view_toolbar_new:
 * @data: A pointer to some data that you want to pass to the callback
 * functions. In the binary acquisition file viewer application this will be
 * a #FacqBAFView object.
 *
 * Creates a new #FacqBAFViewToolbar object
 *
 * Returns: A new #FacqBAFViewToolbar object.
 */
FacqBAFViewToolbar *facq_baf_view_toolbar_new(gpointer data)
{
	return g_object_new(FACQ_TYPE_BAF_VIEW_TOOLBAR,"data",data,NULL);
}

/**
 * facq_baf_view_toolbar_get_widget:
 * @toolbar: A #FacqBAFViewToolbar object.
 *
 * Gets the top level widget of the #FacqBAFViewToolbar so you can add it to
 * the application.
 *
 * Returns: A #GtkWidget pointing to the toolbar's widget.
 *
 */
GtkWidget *facq_baf_view_toolbar_get_widget(FacqBAFViewToolbar *toolbar)
{
	g_return_val_if_fail(FACQ_IS_BAF_VIEW_TOOLBAR(toolbar),NULL);
	return toolbar->priv->toolbar;
}

/**
 * facq_baf_view_toolbar_set_total_pages:
 * @toolbar: A #FacqBAFViewToolbar object.
 * @pages: The number of pages to display. It should be an integer although the
 * type is a real double.
 *
 * Sets the range on the spin button (it will be [1,pages]),
 * sets the text on the label according to pages, and the total number of pages
 * to allow an intelligent control of the toolbar widgets.
 */
void facq_baf_view_toolbar_set_total_pages(FacqBAFViewToolbar *toolbar,gdouble pages)
{
	gchar *text = NULL;

	gtk_spin_button_set_range(GTK_SPIN_BUTTON(toolbar->priv->spin_button),
					1,pages);
	text = g_strdup_printf(_(" of %.0f pages"),pages);
	gtk_label_set_text(GTK_LABEL(toolbar->priv->label),text);
	if(text)
		g_free(text);
	toolbar->priv->total_pages = pages;
}

/**
 * facq_baf_view_toolbar_read_spin_button:
 * @toolbar: A #FacqBAFViewToolbar object.
 *
 * Gets the current value stored in the #GtkSpinButton in the toolbar.
 *
 * Returns: A #gdouble with the current value.
 */
gdouble facq_baf_view_toolbar_read_spin_button(FacqBAFViewToolbar *toolbar)
{
	gdouble ret = 1;

	ret = gtk_spin_button_get_value(GTK_SPIN_BUTTON(toolbar->priv->spin_button));
	return ret;
}

/**
 * facq_baf_view_toolbar_goto_page:
 * @toolbar: A #FacqBAFViewToolbar object.
 * @page_n: The number of page.
 *
 * Enables the buttons for going to first, going back, going forward, going
 * to last, and the spin button, depending on the page number, @page_n, allowing 
 * the selection of a page and disables the page setup button. 
 * This function should be called each time a page is displayed.
 */
void facq_baf_view_toolbar_goto_page(FacqBAFViewToolbar *toolbar,gdouble page_n)
{

	facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_PAGE_SETUP,FALSE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(toolbar->priv->spin_button),page_n);
	
	if(toolbar->priv->total_pages == 1){
		gtk_widget_set_sensitive(toolbar->priv->spin_button,FALSE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GOTO_FIRST,FALSE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GO_BACK,FALSE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GO_FORWARD,FALSE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GOTO_LAST,FALSE);
		return;
	}

	if(page_n == 1 && toolbar->priv->total_pages > 1){
		gtk_widget_set_sensitive(toolbar->priv->spin_button,TRUE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GOTO_FIRST,FALSE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GO_BACK,FALSE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GO_FORWARD,TRUE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GOTO_LAST,TRUE);
		return;
	}
	if(page_n > 1 && page_n < toolbar->priv->total_pages){
		gtk_widget_set_sensitive(toolbar->priv->spin_button,TRUE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GOTO_FIRST,TRUE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GO_BACK,TRUE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GO_FORWARD,TRUE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GOTO_LAST,TRUE);
		return;
	}
	if(page_n == toolbar->priv->total_pages && toolbar->priv->total_pages > 1){
		gtk_widget_set_sensitive(toolbar->priv->spin_button,TRUE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GOTO_FIRST,TRUE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GO_BACK,TRUE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GO_FORWARD,FALSE);
		facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GOTO_LAST,FALSE);
		return;
	}
}

/**
 * facq_baf_view_toolbar_disable_nagivation:
 * @toolbar: A #FacqBAFViewToolbar object.
 *
 * Disables the buttons for going to first, going back, going forward, and going
 * to last, and the spin button, and enables the page setup button. This
 * function should be called when the file is closed.
 */
void facq_baf_view_toolbar_disable_navigation(FacqBAFViewToolbar *toolbar)
{
	facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GOTO_FIRST,FALSE);
	facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GO_BACK,FALSE);
	facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GO_FORWARD,FALSE);
	gtk_widget_set_sensitive(toolbar->priv->spin_button,FALSE);
	facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_GOTO_LAST,FALSE);
	facq_baf_view_toolbar_change_toolitem(toolbar,GTK_STOCK_PAGE_SETUP,TRUE);
}

/**
 * facq_baf_view_toolbar_free:
 * @toolbar: A #FacqBAFViewToolbar object.
 *
 * Destroys a no longer needed #FacqBAFViewToolbar object.
 */
void facq_baf_view_toolbar_free(FacqBAFViewToolbar *toolbar)
{
	g_return_if_fail(FACQ_IS_BAF_VIEW_TOOLBAR(toolbar));
	g_object_unref(G_OBJECT(toolbar));
}

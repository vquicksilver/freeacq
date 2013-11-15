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
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqi18n.h"
#include "gdouble.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqcolor.h"
#include "facqlegend.h"

/**
 * SECTION:facqlegend
 * @short_description:provides a Gtk based legend.
 * @include: facqlegend.h
 *
 * This module provides a Gtk based legend for showing what color corresponds to
 * a physical channel, the units used by the channel, and the number of the
 * physical channel. It's used by #FacqOscope and #FacqBAFView objects.
 *
 * To create a new #FacqLegend object use facq_legend_new(), to set the channels
 * in the legend use facq_legend_set_data(), to clear the channels use
 * facq_legend_clear_data(), and to destroy the #FacqLegend object use
 * facq_legend_free().
 *
 * To add the widget to your application use facq_legend_get_widget().
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * Internally a #FacqLegend uses a #GtkListStore, a #GtkTreeView and a
 * #GtkScrolledWindow.
 * </para>
 * </sect1>
 */

/**
 * FacqLegend:
 *
 * Contains the internal details of the #FacqLegend objects.
 */

/**
 * FacqLegendClass:
 *
 * Class for the #FacqLegend objects.
 */
G_DEFINE_TYPE(FacqLegend,facq_legend,G_TYPE_OBJECT);

enum {
	PROP_0,
};

enum {
	COLOR_COLUMN,
	CHAN_COLUMN,
	UNIT_COLUMN,
	N_COLUMNS
};

struct _FacqLegendPrivate {
	GtkListStore *store;
	GtkWidget *list;
	GtkWidget *scroll_window;
};

/*****--- GObject magic ---*****/
static void facq_legend_constructed(GObject *self)
{
	FacqLegend *leg = FACQ_LEGEND(self);
	GtkCellRenderer *renderer = NULL;
	GtkTreeViewColumn *column = NULL;

	leg->priv->store = gtk_list_store_new(N_COLUMNS,GDK_TYPE_PIXBUF,G_TYPE_STRING,G_TYPE_STRING);
	leg->priv->list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(leg->priv->store));
	leg->priv->scroll_window = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(leg->priv->scroll_window),
									GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(leg->priv->scroll_window),
						GTK_POLICY_AUTOMATIC,
							GTK_POLICY_AUTOMATIC);

	gtk_container_add(GTK_CONTAINER(leg->priv->scroll_window),leg->priv->list);

	renderer = gtk_cell_renderer_pixbuf_new();
	column = gtk_tree_view_column_new_with_attributes(_("Color"),renderer,"pixbuf",COLOR_COLUMN,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(leg->priv->list),column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Channel"),renderer,"text",CHAN_COLUMN,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(leg->priv->list),column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Unit"),renderer,"text",UNIT_COLUMN,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(leg->priv->list),column);

	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(leg->priv->list));
	
	gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(leg->priv->list),
				GTK_TREE_VIEW_GRID_LINES_BOTH);

}

static void facq_legend_finalize(GObject *self)
{
	FacqLegend *leg = FACQ_LEGEND(self);

	if(GTK_IS_WIDGET(leg->priv->list))
		gtk_widget_destroy(leg->priv->list);

	if(GTK_IS_WIDGET(leg->priv->scroll_window))
		gtk_widget_destroy(leg->priv->scroll_window);

	if(G_IS_OBJECT(leg->priv->store))
		g_object_unref(G_OBJECT(leg->priv->store));

	G_OBJECT_CLASS(facq_legend_parent_class)->finalize(self);
}

static void facq_legend_class_init(FacqLegendClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqLegendPrivate));

	object_class->constructed = facq_legend_constructed;
        object_class->finalize = facq_legend_finalize;
}

static void facq_legend_init(FacqLegend *leg)
{
	leg->priv = G_TYPE_INSTANCE_GET_PRIVATE(leg,FACQ_TYPE_LEGEND,FacqLegendPrivate);
}

/****--- public methods ---*****/
/**
 * facq_legend_new:
 *
 * Creates a new #FacqLegend object.
 *
 * Returns: A new #FacqLegend object.
 */
FacqLegend *facq_legend_new(void)
{
	return g_object_new(FACQ_TYPE_LEGEND,NULL);
}

/**
 * facq_legend_get_widget:
 * @leg: A #FacqLegend object.
 * Returns the #GtkWidget for a #FacqLegend object, @leg.
 *
 * Returns: A #GtkWidget that you shouldn't manipulate simply add
 * it to your application.
 */
GtkWidget *facq_legend_get_widget(const FacqLegend *leg)
{
	g_return_val_if_fail(FACQ_IS_LEGEND(leg),NULL);
	return leg->priv->scroll_window;
}

/**
 * facq_legend_set_data:
 * @leg: A #FacqLegend object.
 * @stmd: A #FacqStreamData object.
 *
 * Sets the data that can be shown in the #FacqLegend.
 */
void facq_legend_set_data(FacqLegend *leg,const FacqStreamData *stmd)
{
	guint n_channels = 0, i = 0, chan = 0;
	FacqUnits unit = 0;
	GtkTreeIter iter;
	gchar *chan_text = NULL;
	const gchar *unit_text;
	GdkPixbuf *pixbuf = NULL;

	n_channels = MIN(256,facq_stream_data_get_n_channels(stmd));
	
	gtk_list_store_clear(leg->priv->store);
	for(i = 0;i < n_channels;i++){
		chan = facq_chanlist_get_io_chanspec(stmd->chanlist,i);
		chan = CR_CHAN(chan);
		unit = stmd->units[i];
		chan_text = g_strdup_printf("%u",chan);
		unit_text = facq_units_type_to_human(unit);
		pixbuf = facq_gdk_pixbuf_from_index(i);
		gtk_list_store_append(leg->priv->store,&iter);
		gtk_list_store_set(leg->priv->store,&iter,
					COLOR_COLUMN,pixbuf,
					CHAN_COLUMN,chan_text,
					UNIT_COLUMN,unit_text,
					-1);
		g_free(chan_text);
		g_object_unref(G_OBJECT(pixbuf));
	}
}

/**
 * facq_legend_clear_data:
 * @leg: A #FacqLegend object.
 *
 * Clears the data showed in the #FacqLegend widget.
 */
void facq_legend_clear_data(FacqLegend *leg)
{
	g_return_if_fail(FACQ_IS_LEGEND(leg));
	if(GTK_IS_LIST_STORE(leg->priv->store))
		gtk_list_store_clear(leg->priv->store);
}

/**
 * facq_legend_free:
 * @leg: A #FacqLegend object.
 *
 * Destroys a #FacqLegend object.
 */
void facq_legend_free(FacqLegend *leg)
{
	g_return_if_fail(FACQ_IS_LEGEND(leg));
	g_object_unref(G_OBJECT(leg));
}

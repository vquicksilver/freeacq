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
#include <gtkdatabox.h>
#include <gtkdatabox_grid.h>
#ifdef G_OS_WIN32
#include <gtkdatabox_points.h>
#endif
#include <gtkdatabox_lines.h>
#include <string.h>
#include "facqcolor.h"
#include "facqbafviewplot.h"

/**
 * SECTION:facqbafviewplot
 * @short_description: Plot for the viewer application.
 * @include:facqbafviewplot.h
 *
 * Provides the plot for the binary acquisition file viewer application.
 * To create a new #FacqBAFViewPlot use facq_baf_view_plot_new().
 * Before ploting any signal on the plot, you must setup it first with
 * facq_baf_view_plot_setup(). After this you should put slices to the plot with
 * facq_baf_view_plot_push_chunk() and when all the slices are pushed to the
 * plot use facq_baf_view_plot_draw_page() to plot them.
 */

/**
 * FacqBAFViewPlot:
 *
 * Contains the private details of #FacqBAFViewPlot.
 */

/**
 * FacqBAFViewPlotClass:
 *
 * Class for all the #FacqBAFViewPlot objects.
 */

G_DEFINE_TYPE(FacqBAFViewPlot,facq_baf_view_plot,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_PERIOD,
	PROP_N_CHANNELS,
	PROP_SAMPLES_PER_PAGE
};

struct _FacqBAFViewPlotPrivate {
	gdouble period;
	guint n_channels;
	guint samples_per_page;
	gfloat **samples;
	gfloat **copy_samples;
	gfloat *time;
	gfloat *copy_time;
	gfloat max;
	gfloat min;
	guint next_chunk;
	GtkWidget *databox;
	GtkWidget *table;
	GPtrArray *signal;
};

static gfloat **facq_baf_view_plot_new_samples(gsize samples_per_page,guint n_channels)
{
	gfloat **ret;
	guint stop = 256, i = 0;

	ret = (gfloat **)g_malloc0(MIN(stop,n_channels)*sizeof(gfloat *));
	
	for(i = 0; i < MIN(stop,n_channels); i++){
		ret[i] = (gfloat *)g_malloc0(samples_per_page*sizeof(gfloat));
	}

	return ret;
}

static void facq_baf_view_plot_copy_buffers(FacqBAFViewPlot *plot)
{
	guint i = 0;

	g_memmove(plot->priv->copy_time,
			plot->priv->time,
				sizeof(gfloat)
					*plot->priv->samples_per_page);
	for(i = 0;i < plot->priv->n_channels;i++){
		g_memmove(plot->priv->copy_samples[i],
				plot->priv->samples[i],
					sizeof(gfloat)
						*plot->priv->samples_per_page);
	}
}

static void facq_baf_view_plot_free_samples(gfloat **samples,guint n_channels)
{
	guint i = 0, stop = 256;

	if(!samples)
		return;

	for(i = 0; i < MIN(n_channels,stop) ; i++){
		g_free(samples[i]);
	}
	g_free(samples);
}

static gfloat *facq_baf_view_plot_time_new(guint samples_per_chan)
{
	gfloat *ret = NULL;

	ret = g_malloc0(sizeof(gfloat)*samples_per_chan);

	return ret;
}

static void facq_baf_view_plot_signal_destroy(gpointer graph)
{
	g_return_if_fail(GTK_DATABOX_IS_GRAPH(graph));
	g_object_unref(G_OBJECT(graph));
}

/***** GObject *****/

static void facq_baf_view_plot_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqBAFViewPlot *plot = FACQ_BAF_VIEW_PLOT(self);

	switch(property_id){
	case PROP_PERIOD: g_value_set_double(value,plot->priv->period);
	break;
	case PROP_N_CHANNELS: g_value_set_uint(value,plot->priv->n_channels);
	break;
	case PROP_SAMPLES_PER_PAGE: g_value_set_uint(value,plot->priv->samples_per_page);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(plot,property_id,pspec);
	}
}

static void facq_baf_view_plot_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqBAFViewPlot *plot = FACQ_BAF_VIEW_PLOT(self);

	switch(property_id){
	case PROP_PERIOD: plot->priv->period = g_value_get_double(value);
	break;
	case PROP_N_CHANNELS: plot->priv->n_channels = g_value_get_uint(value);
	break;
	case PROP_SAMPLES_PER_PAGE: plot->priv->samples_per_page = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(plot,property_id,pspec);
	}
}

static void facq_baf_view_plot_finalize(GObject *self)
{
	FacqBAFViewPlot *plot = FACQ_BAF_VIEW_PLOT(self);

	facq_baf_view_plot_free_samples(plot->priv->samples,plot->priv->n_channels);
	facq_baf_view_plot_free_samples(plot->priv->copy_samples,plot->priv->n_channels);
	
	if(plot->priv->time)
		g_free(plot->priv->time);
	if(plot->priv->copy_time)
		g_free(plot->priv->copy_time);

	if(GTK_IS_WIDGET(plot->priv->databox)){
		gtk_databox_graph_remove_all(GTK_DATABOX(plot->priv->databox));
		gtk_widget_destroy(plot->priv->databox);
	}
	if(GTK_IS_WIDGET(plot->priv->table))
		gtk_widget_destroy(plot->priv->table);

	G_OBJECT_CLASS(facq_baf_view_plot_parent_class)->finalize (self);
}

static void facq_baf_view_plot_constructed(GObject *self)
{
	FacqBAFViewPlot *plot = FACQ_BAF_VIEW_PLOT(self);
	GdkColor color;
	GtkDataboxGraph *grid = NULL;

	gtk_databox_create_box_with_scrollbars_and_rulers(&plot->priv->databox,
							  &plot->priv->table,
							  FALSE,FALSE,TRUE,TRUE);

	color.red = color.green = color.blue = 0;
	gtk_widget_modify_bg(plot->priv->databox,GTK_STATE_NORMAL,&color);

	color.red = color.blue = 0;
	color.green = 65535;
	grid = gtk_databox_grid_new(7,7,&color,1);
	gtk_databox_graph_add(GTK_DATABOX(plot->priv->databox),grid);

	color.red = color.blue = 0;
	color.green = 32767;
	grid = gtk_databox_grid_new(15,15,&color,1);
	gtk_databox_graph_add(GTK_DATABOX(plot->priv->databox),grid);

	gtk_databox_set_total_limits(GTK_DATABOX(plot->priv->databox),0,10,10,-10);

	gtk_databox_set_enable_zoom(GTK_DATABOX(plot->priv->databox),TRUE);
        gtk_databox_set_enable_selection(GTK_DATABOX(plot->priv->databox),TRUE);

	gtk_widget_set_size_request(plot->priv->table,512,256);

	gtk_widget_show_all(plot->priv->table);
}

static void facq_baf_view_plot_class_init(FacqBAFViewPlotClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqBAFViewPlotPrivate));

	object_class->set_property = facq_baf_view_plot_set_property;
	object_class->get_property = facq_baf_view_plot_get_property;
	object_class->constructed = facq_baf_view_plot_constructed;
	object_class->finalize = facq_baf_view_plot_finalize;

	g_object_class_install_property(object_class,PROP_PERIOD,
					g_param_spec_double("period",
						          "The period",
							  "The time between two samples",
							  1e-9,
							  G_MAXFLOAT,
							  1,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT |
							  G_PARAM_STATIC_STRINGS));
	
	g_object_class_install_property(object_class,PROP_N_CHANNELS,
					g_param_spec_uint("n-channels",
						          "The number of channels",
							  "The number of channels to plot",
							  1,
							  256,
							  1,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT |
							  G_PARAM_STATIC_STRINGS));
	
	g_object_class_install_property(object_class,PROP_SAMPLES_PER_PAGE,
					g_param_spec_uint("samples-per-page",
							  "Samples per page",
							  "The number of samples per page and channel",
							  1,
							  G_MAXUINT,
							  100,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));
}

static void facq_baf_view_plot_init(FacqBAFViewPlot *plot)
{
	plot->priv = G_TYPE_INSTANCE_GET_PRIVATE(plot,FACQ_TYPE_BAF_VIEW_PLOT,FacqBAFViewPlotPrivate);
	plot->priv->databox = NULL;
	plot->priv->table = NULL;
	plot->priv->n_channels = 0;
	plot->priv->samples = NULL;
	plot->priv->copy_samples = NULL;
	plot->priv->time = NULL;
	plot->priv->copy_time = NULL;
	plot->priv->samples_per_page = 0;
	plot->priv->next_chunk = 0;
}

/**
 * facq_baf_view_plot_new:
 *
 * Creates a new #FacqBAFViewPlot object.
 *
 * Returns: A new #FacqBAFViewPlot object.
 */
FacqBAFViewPlot *facq_baf_view_plot_new(void)
{
	return g_object_new(FACQ_TYPE_BAF_VIEW_PLOT,NULL);
}

/**
 * facq_baf_view_plot_setup:
 * @plot: A #FacqBAFViewPlot.
 * @samples_per_page: The number of slices per page, or samples per channel, per
 * page.
 * @period: The period between each sample, in seconds.
 * @n_channels: The number of channels to plot.
 *
 * Setups a #FacqBAFViewPlot.
 *
 * This function make the following changes in the #FacqBAFViewPlot,
 * first the function clears any previous graphs and the buffers,
 * calling facq_baf_view_plot_clear(). After this a new temporal buffer for the
 * samples and the time vector are allocated, with a size related to
 * the @samples_per_page and @n_channels parameters. Another buffer of equal
 * size is allocated to store the final content.
 * Then the function sets the number of samples per page,
 * the period and the number of channels, and resets the state
 * of some internal properties, like the slice counter, and
 * the maximum and minimum values.
 *
 * After calling this operation the facq_baf_view_plot_push() operation
 * can be called.
 *
 */
void facq_baf_view_plot_setup(FacqBAFViewPlot *plot,guint samples_per_page,gdouble period,guint n_channels)
{
	g_return_if_fail(FACQ_IS_BAF_VIEW_PLOT(plot));

	n_channels = MIN(256,n_channels);

	/* clear the graphs and the buffers */
	facq_baf_view_plot_clear(plot);

	/* allocate n_channels new buffers of size samples_per_page for the
	 * temporal buffer */
	plot->priv->copy_samples = facq_baf_view_plot_new_samples(samples_per_page,n_channels);
	plot->priv->copy_time = facq_baf_view_plot_time_new(samples_per_page);
	
	/* allocate n_channels new buffers of size samples_per_page for the
	 * final buffer */
	plot->priv->samples = facq_baf_view_plot_new_samples(samples_per_page,n_channels);
	plot->priv->time = facq_baf_view_plot_time_new(samples_per_page);
	
	/* set, samples_per_page, period and n_channels and other important
	 * values on the plot object */
	plot->priv->samples_per_page = samples_per_page;
	plot->priv->period = period;
	plot->priv->n_channels = n_channels;
	plot->priv->next_chunk = 0;
	plot->priv->max = plot->priv->min = 0;
	
	/*ready to push!*/
}

/**
 * facq_baf_view_plot_push_chunk:
 * @plot: A #FacqBAFViewPlot object.
 * @chunk: A pointer to an array of n_channels real numbers (A slice).
 *
 * Pushes a slice to the internal buffer of the #FacqBAFViewPlot,
 * increasing the chunk count. The maximum and minimum values are
 * updated to if needed.
 *
 * Be careful, and don't push more slices that the samples per page number, or
 * the slices won't get into the buffer.
 */
void facq_baf_view_plot_push_chunk(FacqBAFViewPlot *plot,gdouble *chunk)
{
	guint i = 0;
	
	/* don't push more samples if the buffer is full */
	if(plot->priv->next_chunk == plot->priv->samples_per_page)
		return;

	/* put each sample in the chunk to the temporal buffer */
	for(i = 0;i < plot->priv->n_channels;i++){
		plot->priv->samples[i][plot->priv->next_chunk] = (gfloat) chunk[i];
		plot->priv->max = MAX(plot->priv->max,chunk[i]);
		plot->priv->min = MIN(plot->priv->min,chunk[i]);
	}

	/* increase current_chunk */
	plot->priv->next_chunk++;
}

/**
 * facq_baf_view_plot_draw_page:
 * @plot: A #FacqBAFViewPlot object.
 * @n_page: the number of page that we want to plot, it should be >= 1. This
 * will be used to calculate the values of the times vector.
 *
 * You must call this function to update the plot with a new graph,
 * created from the slices stored in the temporal buffer, that will
 * be moved to the final buffer.
 *
 * Calculates the time array, calculating the time of each slice,
 * erases any previous graph in the plot, copies the slices pushed
 * to the temporal buffer to the final buffer, and creates new graphs
 * for the new slices pushed in the buffer. Finally the new graphs
 * are plotted, the temporal limits and amplitude limits are set and the
 * chunk counter is reset.
 *
 */
void facq_baf_view_plot_draw_page(FacqBAFViewPlot *plot,gdouble n_page)
{
	guint i = 0;
	gfloat initial_time = 0, period = 0;
	GdkColor color;
	GtkDataboxGraph *graph = NULL;

	g_return_if_fail(FACQ_IS_BAF_VIEW_PLOT(plot));
	g_return_if_fail(n_page >= 1);

	/* set the time array in the temporal array */
	initial_time = ((n_page-1)*plot->priv->samples_per_page*plot->priv->period);
	period = (gfloat)plot->priv->period;
	for(i = 0;i < plot->priv->samples_per_page;i++){
		plot->priv->time[i] = initial_time + i*period;
	}

	/* erase any previous existing graph */
	if(plot->priv->signal){
		for(i = 0;i < plot->priv->signal->len;i++){
			if(GTK_DATABOX_IS_GRAPH(g_ptr_array_index(plot->priv->signal,i))){
				gtk_databox_graph_remove(GTK_DATABOX(plot->priv->databox),
						g_ptr_array_index(plot->priv->signal,i));
			}
		}
		g_ptr_array_free(plot->priv->signal,TRUE);
		plot->priv->signal = NULL;
	}

	/* copy the temporal buffers copy_samples and copy_time to the final buffers */
	facq_baf_view_plot_copy_buffers(plot);

	/* create the new graphs for the final buffer */
	plot->priv->signal = g_ptr_array_new_with_free_func(
                                (GDestroyNotify)facq_baf_view_plot_signal_destroy);
	for(i = 0;i < plot->priv->n_channels;i++){
		facq_gdk_color_from_index(i,&color);
#ifdef G_OS_WIN32
		/* Performance fix for windows, gdk_draw_lines() is too slow
		 * when plotting with a large number of samples so we plot
		 * points instead, note that this is a windows only problem. */
		if(plot->priv->samples_per_page <= (2000.0/plot->priv->n_channels)){
			graph = gtk_databox_lines_new(plot->priv->next_chunk,
							plot->priv->copy_time,
								plot->priv->copy_samples[i],
									&color,1);
		}
		else {
			graph = gtk_databox_points_new(plot->priv->next_chunk,
							plot->priv->copy_time,
								plot->priv->copy_samples[i],
									&color,1);
		}
#else
		graph = gtk_databox_lines_new(plot->priv->next_chunk,
							plot->priv->copy_time,
								plot->priv->copy_samples[i],
									&color,1);
#endif
		g_ptr_array_add(plot->priv->signal,(gpointer) graph);
		/* add the graphs to the box */
		gtk_databox_graph_add(GTK_DATABOX(plot->priv->databox),graph);
	}

	/* set the limits */
	gtk_databox_set_total_limits(GTK_DATABOX(plot->priv->databox),
					plot->priv->time[0],
						plot->priv->time[plot->priv->samples_per_page-1],
							plot->priv->max+0.5,plot->priv->min-0.5);

	/* plot it! */
	gtk_widget_queue_draw(plot->priv->databox);

	/* set current_chunk to 0 so we can push more samples later */
	plot->priv->next_chunk = 0;
}

/**
 * facq_baf_view_plot_get_widget:
 * @plot: A #FacqBAFViewPlot object.
 *
 * Gets the top level widget for the #FacqBAFViewPlot, so you
 * can add it to your application.
 *
 * Returns: The top level widget a #GtkTable with the GtkDatabox object.
 */
GtkWidget *facq_baf_view_plot_get_widget(const FacqBAFViewPlot *plot)
{
	g_return_val_if_fail(FACQ_IS_BAF_VIEW_PLOT(plot),NULL);
	return plot->priv->table;
}

/**
 * facq_baf_view_plot_clear:
 * @plot: A #FacqBAFViewPlot object.
 *
 * Clears the content of a #FacqBAFViewPlot, erasing the plotted graphs.
 * The previously allocated memory during the setup phase is freed,
 * and the function triggers a redraw of the now, empty plot.
 *
 */
void facq_baf_view_plot_clear(FacqBAFViewPlot *plot)
{
	guint i = 0;
	
	g_return_if_fail(FACQ_IS_BAF_VIEW_PLOT(plot));

	/* remove the graphs from the plot and destroy them */
	if(plot->priv->signal){
		for(i = 0;i < plot->priv->signal->len;i++){
			if(GTK_DATABOX_IS_GRAPH(g_ptr_array_index(plot->priv->signal,i))){
				gtk_databox_graph_remove(GTK_DATABOX(plot->priv->databox),
						g_ptr_array_index(plot->priv->signal,i));
			}
		}
		g_ptr_array_free(plot->priv->signal,TRUE);
		plot->priv->signal = NULL;
	}

	/* free the previous buffers if any */
	if(plot->priv->samples)
		facq_baf_view_plot_free_samples(plot->priv->samples,
							plot->priv->n_channels);
	plot->priv->samples = NULL;
	
	if(plot->priv->copy_samples)
		facq_baf_view_plot_free_samples(plot->priv->copy_samples,
							plot->priv->n_channels);
	plot->priv->copy_samples = NULL;

	if(plot->priv->time)
		g_free(plot->priv->time);
	plot->priv->time = NULL;

	if(plot->priv->copy_time);
		g_free(plot->priv->copy_time);
	plot->priv->copy_time = NULL;

	gtk_widget_queue_draw(plot->priv->databox);
}

/**
 * facq_baf_view_plot_zoom_in:
 * @plot: A #FacqBAFViewPlot object.
 *
 * Zooms to the maximum the view on the plot.
 */
void facq_baf_view_plot_zoom_in(FacqBAFViewPlot *plot)
{
	gtk_databox_zoom_to_selection(GTK_DATABOX(plot->priv->databox));
}

/**
 * facq_baf_view_plot_zoom_out:
 * @plot: A #FacqBAFViewPlot object.
 *
 * Zooms out the view on the plot.
 */
void facq_baf_view_plot_zoom_out(FacqBAFViewPlot *plot)
{
	gtk_databox_zoom_out(GTK_DATABOX(plot->priv->databox));
}

/**
 * facq_baf_view_plot_zoom_home:
 * @plot: A #FacqBAFViewPlot object.
 *
 * Returns the view on the plot to the initial point.
 */
void facq_baf_view_plot_zoom_home(FacqBAFViewPlot *plot)
{
	gtk_databox_zoom_home(GTK_DATABOX(plot->priv->databox));
}

/**
 * facq_baf_view_plot_free:
 * @plot: A #FacqBAFViewPlot object.
 *
 * Destroys a no longer needed #FacqBAFViewPlot object,
 * freeing it's resources.
 */
void facq_baf_view_plot_free(FacqBAFViewPlot *plot)
{
	g_return_if_fail(FACQ_IS_BAF_VIEW_PLOT(plot));
	g_object_unref(G_OBJECT(plot));
}

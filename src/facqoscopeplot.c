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
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <gtk/gtk.h>
#include <gtkdatabox.h>
#include <gtkdatabox_grid.h>
#include <gtkdatabox_lines.h>
#ifdef G_OS_WIN32
#include <gtkdatabox_points.h>
#endif
#include <string.h>
#include "gdouble.h"
#include "facqlog.h"
#include "facqcolor.h"
#include "facqchunk.h"
#include "facqmisc.h"
#include "facqoscopeplot.h"

/**
 * SECTION:facqoscopeplot
 * @short_description: Provides the plot to the oscilloscope application.
 * @include:facqoscopeplot.h
 *
 * Provides the plot to the oscilloscope application.
 */

/**
 * FacqOscopePlot:
 *
 * Contains the private details of the #FacqOscopePlot.
 */

/**
 * FacqOscopePlotClass:
 *
 * Class for the #FacqOscopePlot objects.
 */

G_DEFINE_TYPE(FacqOscopePlot,facq_oscope_plot,G_TYPE_OBJECT);

GQuark facq_oscope_plot_error_quark(void)
{
        return g_quark_from_static_string("facq-oscope-plot-error-quark");
}


enum {
	PROP_0,
	PROP_PERIOD,
	PROP_N_CHANNELS
};

struct _FacqOscopePlotPrivate {
	GtkDataboxGrid *ugrid; //upper grid
	GtkDataboxGrid *lgrid; //lower grid
	gdouble period;
	guint n_channels;
	gfloat **samples;
	gfloat **copy_samples;
	gfloat *time;
	gfloat *copy_time;
	gfloat max;
	gfloat min;
	gfloat last_time;
	gsize samples_per_chan;
	guint next_slice;
	GtkWidget *databox;
	GtkWidget *table;
	GPtrArray *signal;
};

static gsize facq_oscope_plot_get_samples_per_chan(gdouble period,guint n_channels)
{
	gsize ret = 10;

	if(period < 1){
		ret = facq_misc_period_to_chunk_size(period,sizeof(gfloat),n_channels);
		ret /= (sizeof(gfloat)*n_channels);
	}
	else {
		if(period >= 1 && period < 60)
			ret = 1 + 60/period;
		else if(period >= 60 && period < 360 )
			ret = 1 + 360/period;
		else if(period >= 360 && period < 3600)
			ret = 1 + 3600/period;
		else if(period >= 3600 && period < 86400)
			ret = 1 + 86400/period;
		else if(period >= 86400 && period < 31536000)
			ret = 1 + 31536000/period;
		//so you are going to loose an entire decade looking at an
		//oscilloscope? ... anyway
		else if(period >= 31536000 && period < 315360000)
			ret = 1 + 315360000/period;
		//really are you planning to live more than a century? I don't 
		//think that your computer is going to last so long, but anyway...
		else if(period >= 3153600000 && period < (gdouble)31536000000)
			ret = 1 + 31536000000/period;
		//hello miss/mister inmortal and thanks for using my software, can I
		//be like you?.
		else if(period >= (gdouble)31536000000 && period < (gdouble)315360000000)
			ret = 1 + 31536000000/period;
		else {
			
		}
	}

#if ENABLE_DEBUG
	(period < 1) ? 
		facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
				"Period is %.9g seconds, using %"G_GSIZE_FORMAT" samples per channel",
				period,ret) 
	
	: 
	
		facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
				"Period is %.9g seconds, using %"G_GSIZE_FORMAT" samples and RT plotting mode",
				period,ret)
	;
#endif

	return ret;
}

static gfloat **facq_oscope_plot_new_samples(gsize samples_per_chan,guint n_channels)
{
	gfloat **ret;
	guint stop = 256, i = 0;

	ret = (gfloat **)g_malloc0(MIN(stop,n_channels)*sizeof(gfloat *));
	
	for(i = 0; i < MIN(stop,n_channels); i++){
		ret[i] = (gfloat *)g_malloc0(samples_per_chan*sizeof(gfloat));
	}

	return ret;
}

static void facq_oscope_plot_copy_buffers(FacqOscopePlot *plot)
{
	guint i = 0;

	/* copy time */
	g_memmove(plot->priv->copy_time,
			plot->priv->time,
				sizeof(gfloat)
					*plot->priv->samples_per_chan);

	/* copy samples */
	for(i = 0;i < plot->priv->n_channels;i++){
		g_memmove(plot->priv->copy_samples[i],
				plot->priv->samples[i],
					sizeof(gfloat)
						*plot->priv->samples_per_chan);
	}
}

static void facq_oscope_plot_free_samples(gfloat **samples,guint n_channels)
{
	guint i = 0, stop = 256;

	if(!samples)
		return;

	for(i = 0; i < MIN(n_channels,stop) ; i++){
		g_free(samples[i]);
	}
	g_free(samples);
}

static gfloat *facq_oscope_plot_time_new(guint samples_per_chan)
{
	gfloat *ret = NULL;

	ret = g_malloc0(sizeof(gfloat)*samples_per_chan);

	return ret;
}

static void facq_oscope_plot_signal_destroy(gpointer graph)
{
	g_return_if_fail(GTK_DATABOX_IS_GRAPH(graph));
	g_object_unref(G_OBJECT(graph));
}

/***** GObject *****/

static void facq_oscope_plot_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqOscopePlot *plot = FACQ_OSCOPE_PLOT(self);

	switch(property_id){
	case PROP_PERIOD: g_value_set_double(value,plot->priv->period);
	break;
	case PROP_N_CHANNELS: g_value_set_uint(value,plot->priv->n_channels);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(plot,property_id,pspec);
	}
}

static void facq_oscope_plot_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqOscopePlot *plot = FACQ_OSCOPE_PLOT(self);

	switch(property_id){
	case PROP_PERIOD: plot->priv->period = g_value_get_double(value);
	break;
	case PROP_N_CHANNELS: plot->priv->n_channels = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(plot,property_id,pspec);
	}
}

static void facq_oscope_plot_finalize(GObject *self)
{
	FacqOscopePlot *plot = FACQ_OSCOPE_PLOT(self);

	facq_oscope_plot_free_samples(plot->priv->samples,plot->priv->n_channels);
	facq_oscope_plot_free_samples(plot->priv->copy_samples,plot->priv->n_channels);
	
	if(plot->priv->time)
		g_free(plot->priv->time);
	
	if(plot->priv->copy_time)
		g_free(plot->priv->copy_time);

	if(GTK_IS_WIDGET(plot->priv->databox)){
		gtk_databox_graph_remove_all(GTK_DATABOX(plot->priv->databox));
		facq_oscope_plot_signal_destroy(plot->priv->ugrid);
		facq_oscope_plot_signal_destroy(plot->priv->lgrid);
		if(plot->priv->signal){
			g_ptr_array_free(plot->priv->signal,TRUE);
			plot->priv->signal = NULL;
		}
		gtk_widget_destroy(plot->priv->databox);
	}
	if(GTK_IS_WIDGET(plot->priv->table))
		gtk_widget_destroy(plot->priv->table);

	G_OBJECT_CLASS(facq_oscope_plot_parent_class)->finalize (self);
}

static void facq_oscope_plot_constructed(GObject *self)
{
	FacqOscopePlot *plot = FACQ_OSCOPE_PLOT(self);
	GdkColor color;
	GtkDataboxGraph *grid = NULL;

	gtk_databox_create_box_with_scrollbars_and_rulers(&plot->priv->databox,
							  &plot->priv->table,
							  FALSE,FALSE,TRUE,TRUE);

	color.red = color.green = color.blue = 0;
	gtk_widget_modify_bg(plot->priv->databox,GTK_STATE_NORMAL,&color);

	color.red = color.blue = 0;
	color.green = 65535;
	grid = gtk_databox_grid_new(19,9,&color,1);
	gtk_databox_graph_add(GTK_DATABOX(plot->priv->databox),grid);
	plot->priv->ugrid = GTK_DATABOX_GRID(grid);

	color.red = color.blue = 0;
	color.green = 32767;
	grid = gtk_databox_grid_new(39,19,&color,1);
	gtk_databox_graph_add(GTK_DATABOX(plot->priv->databox),grid);
	plot->priv->lgrid = GTK_DATABOX_GRID(grid);

	gtk_databox_set_total_limits(GTK_DATABOX(plot->priv->databox),0,1,10,-10);

	gtk_databox_set_enable_zoom(GTK_DATABOX(plot->priv->databox),TRUE);
        gtk_databox_set_enable_selection(GTK_DATABOX(plot->priv->databox),TRUE);

	gtk_widget_set_size_request(plot->priv->table,512,256);

	gtk_widget_show_all(plot->priv->table);
}

static void facq_oscope_plot_class_init(FacqOscopePlotClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqOscopePlotPrivate));

	object_class->set_property = facq_oscope_plot_set_property;
	object_class->get_property = facq_oscope_plot_get_property;
	object_class->constructed = facq_oscope_plot_constructed;
	object_class->finalize = facq_oscope_plot_finalize;

	g_object_class_install_property(object_class,PROP_PERIOD,
					g_param_spec_double("period",
						          "The period",
							  "The time between two samples",
							  1/1e9,
							  G_MAXDOUBLE,
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
}

static void facq_oscope_plot_init(FacqOscopePlot *plot)
{
	plot->priv = G_TYPE_INSTANCE_GET_PRIVATE(plot,FACQ_TYPE_OSCOPE_PLOT,FacqOscopePlotPrivate);
	plot->priv->databox = NULL;
	plot->priv->table = NULL;
	plot->priv->n_channels = 0;
	plot->priv->samples = NULL;
	plot->priv->copy_samples = NULL;
	plot->priv->time = NULL;
	plot->priv->copy_time = NULL;
	plot->priv->last_time = 0;
	plot->priv->samples_per_chan = 0;
	plot->priv->next_slice = 0;
}

/**
 * facq_oscope_plot_new:
 *
 * Creates a new #FacqOscopePlot object.
 *
 * Returns: A new #FacqOscopePlot object.
 */
FacqOscopePlot *facq_oscope_plot_new(void)
{
	return g_object_new(FACQ_TYPE_OSCOPE_PLOT,NULL);
}

/**
 * facq_oscope_plot_setup:
 * @plot: A #FacqOscopePlot object.
 * @period: The period in seconds.
 * @n_channels: The number of channels you want to plot at the same time. Values
 * greater than 256 will be treated as 256.
 * @err: (allow-none): It will be set in case of error if not %NULL.
 *
 * This function must be called before doing any drawing on the plot.
 * It allocates some internal buffers for storing the (x,y) value pairs
 * and does other initialization operations.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_oscope_plot_setup(FacqOscopePlot *plot,gdouble period,guint n_channels,GError **err)
{
	guint max_channels = 256, i = 0;
	gsize samples_per_chan = 0;
	GtkDataboxGraph *graph = NULL;
	GdkColor color;

	g_return_val_if_fail(FACQ_IS_OSCOPE_PLOT(plot),FALSE);

	if(period < 0.000000001){
		if(err)
			g_set_error(err,FACQ_OSCOPE_PLOT_ERROR,
					FACQ_OSCOPE_PLOT_ERROR_FAILED,"Period not supported");
		return FALSE;
	}
	n_channels = MIN(max_channels,n_channels);
	samples_per_chan = facq_oscope_plot_get_samples_per_chan(period,n_channels);

	/* Erase old graphs if any */
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

	/* prepare the buffers, for storing the x values (time) and the y values
	 * (samples), we must free it before allocating a new buffer if the
	 * buffer were previously allocated */
	if(plot->priv->samples)
		facq_oscope_plot_free_samples(plot->priv->samples,
							plot->priv->n_channels);
	if(plot->priv->copy_samples)
		facq_oscope_plot_free_samples(plot->priv->copy_samples,
							plot->priv->n_channels);

	plot->priv->samples = facq_oscope_plot_new_samples(samples_per_chan,n_channels);
	plot->priv->copy_samples = facq_oscope_plot_new_samples(samples_per_chan,n_channels);
	
	if(plot->priv->time)
		g_free(plot->priv->time);
	
	if(plot->priv->copy_time)
		g_free(plot->priv->copy_time);
		
	plot->priv->time = facq_oscope_plot_time_new(samples_per_chan);
	plot->priv->copy_time = facq_oscope_plot_time_new(samples_per_chan);
	/* put each value in place, and reset old values */

	plot->priv->period = period;
	plot->priv->n_channels = n_channels;
	plot->priv->last_time = 0;
	plot->priv->samples_per_chan = samples_per_chan;
	plot->priv->next_slice = 0;
	plot->priv->max = 0;
	plot->priv->min = 0;

	/* if period < 1e9 create the GtkDataBoxGraph objects. We don't need to
	 * recreate them later because modifying the buffers will modify the
	 * graphs. */
	if(period < 1){
		plot->priv->signal = g_ptr_array_new_with_free_func(
				(GDestroyNotify)facq_oscope_plot_signal_destroy);

		for(i = 0;i < n_channels;i++){
			facq_gdk_color_from_index(i,&color);
			graph = gtk_databox_lines_new(samples_per_chan,
							plot->priv->copy_time,
								plot->priv->copy_samples[i],
									&color,1);
			g_ptr_array_add(plot->priv->signal,(gpointer) graph);
		}
	}

	return TRUE;
}

static void facq_oscope_plot_process_chunk_fast(FacqOscopePlot *plot,FacqChunk *chunk)
{
	gdouble *slice = NULL;
	gsize i = 0, j = 0;

	if(plot->priv->period < 1){
		/* get the double precision samples */
		slice = (gdouble *) chunk->data;

		/* initialize max and min again */
		plot->priv->max = slice[0];
		plot->priv->min = slice[0];

#if ENABLE_DEBUG
		facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
				"Process chunk fast: max0:%g min0:%g samples_per_chan:%"G_GSIZE_FORMAT,
				plot->priv->max,plot->priv->min,plot->priv->samples_per_chan);
#endif

		/* samples_per_channel it's the same that n_slices. For each
		 * i-slice we have to put in each channel j-array the sample. 
		 * Also for each slice put the correct time value. */
		for(i = 0;i < plot->priv->samples_per_chan;i++){
			plot->priv->time[i] = 
				plot->priv->last_time + i*plot->priv->period;
			for(j = 0;j < plot->priv->n_channels;j++){
				plot->priv->samples[j][i] = (gfloat) slice[j];
				plot->priv->max = (gfloat) MAX(plot->priv->max,slice[j]);
				plot->priv->min = (gfloat) MIN(plot->priv->min,slice[j]);
			}
			/* point slice to the first sample of the next slice */
			slice = slice + plot->priv->n_channels;
		}

		/* copy samples to copy_samples and time to copy_time.
		 * This is easy for each channel we have an array that has
		 * samples_per_chan samples. */
#if ENABLE_DEBUG
		facq_log_write("Copying buffers",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
		facq_oscope_plot_copy_buffers(plot);
	
#if ENABLE_DEBUG
		facq_log_write("Buffers copied",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
		/* if this is the first time we draw the signals in the plot 
		 * we need to add the graphs to the plot */
		if(plot->priv->last_time == 0){
#if ENABLE_DEBUG
		facq_log_write("Adding graphs",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
			for(i = 0;i < plot->priv->n_channels;i++){
				gtk_databox_graph_add(
					GTK_DATABOX(plot->priv->databox),
						g_ptr_array_index(plot->priv->signal,i));
			}
		}

		/* set the limits on the gtkdatabox */
#if ENABLE_DEBUG
		facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
				"Setting limits, time [%g,%g] , amplitude [%g,%g]",
				plot->priv->last_time,
				plot->priv->time[plot->priv->samples_per_chan-1],
				plot->priv->min-0.5,
				plot->priv->max+0.5);
#endif
		gtk_databox_set_total_limits(GTK_DATABOX(plot->priv->databox),
					     plot->priv->last_time,
					     plot->priv->time[plot->priv->samples_per_chan-1],
					     plot->priv->max+0.5,
					     plot->priv->min-0.5);

		/* trigger redraw of the plot */
		gtk_widget_queue_draw(plot->priv->databox);

		/* update last_time */
		plot->priv->last_time = 
			plot->priv->time[plot->priv->samples_per_chan-1]
					+ plot->priv->period;

		/* done :) */
	}
}

/* This function deals with the plot when the sampling period between samples
 * is one second or greater. */
static void facq_oscope_plot_process_chunk_slow(FacqOscopePlot *plot,FacqChunk *chunk)
{
	gdouble *samples = NULL;
	guint i = 0;
	GdkColor color;
	GtkDataboxGraph *graph = NULL;

	/* Now the game changes, this time we will get a sample per channel in
	 * each chunk, that is the same that saying that each chunk only has a
	 * slice. We have to replot the graph for each slice. */
	samples = (gdouble *)chunk->data;

	if(plot->priv->next_slice == 0){
		plot->priv->max = samples[0];
		plot->priv->min = samples[0];
	}

	for(i = 0;i < plot->priv->n_channels;i++){
		plot->priv->samples[i][plot->priv->next_slice] = (gfloat) samples[i];
		plot->priv->max = MAX(plot->priv->max,samples[i]);
		plot->priv->min = MIN(plot->priv->min,samples[i]);
	}

	/* update the value of the correspondent x in time */
	plot->priv->time[plot->priv->next_slice] = 
		plot->priv->last_time + (plot->priv->next_slice*plot->priv->period);

#if ENABLE_DEBUG
	facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,"chunk:%u time:%g",
			 plot->priv->next_slice,
			 plot->priv->time[plot->priv->next_slice]);
#endif

	plot->priv->next_slice++;

	/* set the limits of the page */
	gtk_databox_set_total_limits(GTK_DATABOX(plot->priv->databox),
				     plot->priv->last_time,
				     (plot->priv->samples_per_chan-1)*
				     plot->priv->period +
				     plot->priv->last_time,
				     plot->priv->max+0.5,
				     plot->priv->min-0.5);

#if ENABLE_DEBUG
	facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
			 "Setting limits, time [%g,%g] , amplitude [%g,%g]",
			 plot->priv->last_time,
			 plot->priv->last_time + 
				(plot->priv->samples_per_chan-1)
					* plot->priv->period,
			 plot->priv->min-0.5,
			 plot->priv->max+0.5);
#endif

	/* copy the buffers */
	facq_oscope_plot_copy_buffers(plot);
	/* destroy the old graphs (if any) destroying the signal array*/
	if(plot->priv->signal){
		for(i = 0;i < plot->priv->n_channels;i++){
			gtk_databox_graph_remove(GTK_DATABOX(plot->priv->databox),
			g_ptr_array_index(plot->priv->signal,i));
		}
		g_ptr_array_free(plot->priv->signal,TRUE);
		plot->priv->signal = NULL;
	}
	
	/* create new graphs with the copy buffers */
	plot->priv->signal =
		g_ptr_array_new_with_free_func((GDestroyNotify)facq_oscope_plot_signal_destroy);

	for(i = 0;i < plot->priv->n_channels;i++){
		facq_gdk_color_from_index(i,&color);
		graph = gtk_databox_lines_new(plot->priv->next_slice,
						plot->priv->copy_time,
							plot->priv->copy_samples[i],
								&color,1);
		g_ptr_array_add(plot->priv->signal,(gpointer) graph);
		/* add the graphs to the box */
		gtk_databox_graph_add(GTK_DATABOX(plot->priv->databox),graph);
	}

	/* trigger redraw of the plot */
	gtk_widget_queue_draw(plot->priv->databox);

	/* if end of page */
	if(plot->priv->next_slice == plot->priv->samples_per_chan){
		plot->priv->last_time = 
			plot->priv->time[plot->priv->samples_per_chan-1] 
				+ plot->priv->period;
		plot->priv->next_slice = 0;
	}
}

/**
 * facq_oscope_plot_process_chunk:
 * @plot: A #FacqOscopePlot object.
 * @chunk: A #FacqChunk object.
 * 
 * Processes the chunk, @chunk doing a redraw of the plot.
 */
void facq_oscope_plot_process_chunk(FacqOscopePlot *plot,FacqChunk *chunk)
{
	if(plot->priv->period < 1)
		facq_oscope_plot_process_chunk_fast(plot,chunk);
	else 
		facq_oscope_plot_process_chunk_slow(plot,chunk);
}

/**
 * facq_oscope_plot_get_widget:
 * @plot: A #FacqOscopePlot object.
 *
 * Gets the top level widget, so it can be added to the application.
 *
 * Returns: A #GtkWidget pointing to the top level widget.
 */
GtkWidget *facq_oscope_plot_get_widget(const FacqOscopePlot *plot)
{
	g_return_val_if_fail(FACQ_IS_OSCOPE_PLOT(plot),NULL);
	return plot->priv->table;
}

/**
 * facq_oscope_plot_set_zoom:
 * @plot: A #FacqOscopePlot object.
 * @enable: %TRUE or %FALSE.
 *
 * Enables or disables the capability or selection an area on the plot and
 * doing zoom on it.
 */
void facq_oscope_plot_set_zoom(FacqOscopePlot *plot,gboolean enable)
{
	g_return_if_fail(FACQ_IS_OSCOPE_PLOT(plot));
	gtk_databox_set_enable_selection(GTK_DATABOX(plot->priv->databox),enable);
	gtk_databox_set_enable_zoom(GTK_DATABOX(plot->priv->databox),enable);
}

/**
 * facq_oscope_plot_zoom_in:
 * @plot: A #FacqOscopePlot object.
 *
 * Zooms in the view on a #FacqOscopePlot object.
 */
void facq_oscope_plot_zoom_in(FacqOscopePlot *plot)
{
	gtk_databox_zoom_to_selection(GTK_DATABOX(plot->priv->databox));
}

/**
 * facq_oscope_plot_zoom_out:
 * @plot: A #FacqOscopePlot object.
 *
 * Zooms out the view on a #FacqOscopePlot object.
 */
void facq_oscope_plot_zoom_out(FacqOscopePlot *plot)
{
	gtk_databox_zoom_out(GTK_DATABOX(plot->priv->databox));
}

/**
 * facq_oscope_plot_zoom_home:
 * @plot: A #FacqOscopePlot object.
 *
 * Restores the default view of a #FacqOscopePlot object.
 */
void facq_oscope_plot_zoom_home(FacqOscopePlot *plot)
{
	gtk_databox_zoom_home(GTK_DATABOX(plot->priv->databox));
}

/**
 * facq_oscope_plot_free:
 * @plot: A #FacqOscopePlot object.
 *
 * Destroys a no longer needed #FacqOscopePlot object.
 */
void facq_oscope_plot_free(FacqOscopePlot *plot)
{
	g_return_if_fail(FACQ_IS_OSCOPE_PLOT(plot));
	g_object_unref(G_OBJECT(plot));
}

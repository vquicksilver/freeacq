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
#include <glib.h>
#include <gio/gio.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "gdouble.h"
#include "facqnet.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"

/**
 * SECTION:facqstreamdata
 * @include:facqstreamdata.h
 * @short_description: Stores important attributes of the stream.
 * @title:FacqStreamData
 *
 * This module provides a class for storing important attributes of the
 * stream. The #FacqSource is the provider of this attributes, that can be read 
 * by any other element in the stream, like the #FacqOperation objects or like
 * the #FacqSink objects.
 *
 * #FacqStreamData objects, are usually created and owned by the #FacqSource
 * object. The data contained in the #FacqStreamData object describes how the
 * samples are (size of each samples when is read from the source), 
 * how many samples you can expect per slice, the period in seconds
 * between slices, the physical units of the samples per channel, the physical channels
 * where the samples were taken, and expected maximum and minimum values of the
 * samples per channel.
 *
 */

/**
 * FacqStreamData:
 * @bps: The number of bytes per sample before conversion.
 * @n_channels: The number of channels in the stream.
 * @period: The period between samples in seconds.
 * @units: The unit of the samples, see #FacqUnits.
 * @chanlist: A #FacqChanlist object with information of the channels.
 * @max: The maximum expected value of the samples.
 * @min: The minimum expected value of the samples.
 *
 * Contains the attributes of the #FacqStreamData objects.
 * You shouldn't modify this fields directly, you should use the
 * required method for each attribute.
 */

/**
 * FacqStreamDataClass:
 *
 * Class for the #FacqStreamData objects.
 */

G_DEFINE_TYPE(FacqStreamData,facq_stream_data,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_BPS,
	PROP_N_CHANNELS,
	PROP_PERIOD,
	PROP_UNITS,
	PROP_CHANLIST,
	PROP_MAX,
	PROP_MIN,
};

static void facq_stream_data_finalize(GObject *self)
{
	FacqStreamData *stmd = FACQ_STREAM_DATA(self);

	if(stmd->units)
		g_free(stmd->units);
	if(stmd->chanlist)
		facq_chanlist_free(stmd->chanlist);
	if(stmd->max)
		g_free(stmd->max);
	if(stmd->min)
		g_free(stmd->min);

	if(G_OBJECT_CLASS(facq_stream_data_parent_class)->finalize)
    		(*G_OBJECT_CLASS(facq_stream_data_parent_class)->finalize)(self);
}

static void facq_stream_data_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqStreamData *stmd = FACQ_STREAM_DATA(self);

	switch(property_id){
	case PROP_BPS: g_value_set_uint(value,stmd->bps);
	break;
	case PROP_N_CHANNELS: g_value_set_uint(value,stmd->n_channels);
	break;
	case PROP_PERIOD: g_value_set_double(value,stmd->period);
	break;
	case PROP_UNITS: g_value_set_pointer(value,stmd->units);
	break;
	case PROP_CHANLIST: g_value_set_pointer(value,stmd->chanlist);
	break;
	case PROP_MAX: g_value_set_pointer(value,stmd->max);
	break;
	case PROP_MIN: g_value_set_pointer(value,stmd->min);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (stmd, property_id, pspec);
	}
}

static void facq_stream_data_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqStreamData *stmd = FACQ_STREAM_DATA(self);

	switch(property_id){
	case PROP_BPS: stmd->bps = g_value_get_uint(value);
	break;
	case PROP_N_CHANNELS: stmd->n_channels = g_value_get_uint(value);
	break;
	case PROP_PERIOD: stmd->period = g_value_get_double(value);
	break;
	case PROP_UNITS: stmd->units = g_value_get_pointer(value);
	break;
	case PROP_CHANLIST: stmd->chanlist = g_value_get_pointer(value);
	break;
	case PROP_MAX: stmd->max = g_value_get_pointer(value);
	break;
	case PROP_MIN: stmd->min = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (stmd, property_id, pspec);
	}
}

static void facq_stream_data_class_init(FacqStreamDataClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property = facq_stream_data_set_property;
	object_class->get_property = facq_stream_data_get_property;
	object_class->finalize = facq_stream_data_finalize;

	g_object_class_install_property(object_class,PROP_BPS,
					g_param_spec_uint("bps",
							"bits per sample",
							"The number of bits per sample",
							1,
							G_MAXUINT,
							1,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_N_CHANNELS,
					g_param_spec_uint("n-channels",
							"Number of channels",
							"The number of channels beign sampled",
							1,
							G_MAXUINT,
							1,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_PERIOD,
					g_param_spec_double("period",
							"Time between samples in seconds",
							"The time between two samples in seconds",
							1e-9,
							G_MAXDOUBLE,
							1,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_UNITS,
					g_param_spec_pointer("units",
							     "Units",
							     "The list of per channel unit",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_CHANLIST,
					g_param_spec_pointer("chanlist",
							     "Channel list",
							     "The complete list of channels",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_MAX,
					g_param_spec_pointer("max",
							     "Maximun",
							     "The maximun amplitude value per channel",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));
	
	g_object_class_install_property(object_class,PROP_MIN,
					g_param_spec_pointer("min",
							     "Minimum",
							     "The minimum amplitude value per channel",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));
}

static void facq_stream_data_init(FacqStreamData *stmd)
{
	stmd->bps = 0;
	stmd->n_channels = 0;
	stmd->period = 0;
	stmd->chanlist = NULL;
	stmd->units = NULL;
	stmd->max = NULL;
	stmd->min = NULL;
}

/* Public methods */

/**
 * facq_stream_data_new:
 * @bps: The number of bits per sample, before doing any conversion on the data.
 * @n_channels: The number of I/O channels.
 * @period: The sampling period in seconds. 
 * @chanlist: A #FacqChanlist object.
 * @units: An array of #FacqUnits with a length equal to @n_channels, with the
 * corresponding unit per channel.
 * @max: An array of #gdouble with a length equal to @n_channels, with the
 * expected maximum value of the samples you are going to read.
 * @min: An array of #gdouble with a length equal to @n_channels, with the
 * expected minimum value of the samples you are going to read.
 *
 * Creates a new #FacqStreamData object.
 *
 * Returns: a new #FacqStreamData object.
 */
FacqStreamData *facq_stream_data_new(guint bps,guint n_channels,gdouble period,FacqChanlist *chanlist,FacqUnits *units,gdouble *max,gdouble *min)
{
	return g_object_new(FACQ_TYPE_STREAM_DATA,
			"bps",bps,
			"n-channels",n_channels,
			"period",period,
			"units",units,
			"chanlist",chanlist,
			"max",max,
			"min",min,
			NULL);
}

/**
 * facq_stream_data_get_bps:
 * @stmd: A #FacqStreamData object.
 *
 * Returns the value of the bps attribute.
 *
 * Returns: the value of the bytes per sample attribute.
 *
 */
guint facq_stream_data_get_bps(const FacqStreamData *stmd)
{
	g_return_val_if_fail(FACQ_IS_STREAM_DATA(stmd),0);
	return stmd->bps;
}

/**
 * facq_stream_data_get_n_channels:
 * @stmd: A #FacqStreamData object.
 *
 * Returns the value of the n_channels attribute.
 *
 * Returns: The value of the number of channels attribute.
 */
guint facq_stream_data_get_n_channels(const FacqStreamData *stmd)
{
	g_return_val_if_fail(FACQ_IS_STREAM_DATA(stmd),0);
	return stmd->n_channels;
}

/**
 * facq_stream_data_get_period:
 * @stmd: A #FacqStreamData object.
 *
 * Returns the value of the period attribute.
 *
 * Returns: The value of the period attribute.
 */
gdouble facq_stream_data_get_period(const FacqStreamData *stmd)
{
	g_return_val_if_fail(FACQ_IS_STREAM_DATA(stmd),0);
	return stmd->period;
}

/**
 * facq_stream_data_get_units:
 * @stmd: A #FacqStreamData object.
 *
 * Returns a constant pointer to the units array.
 * See #FacqUnits for more info.
 *
 * Returns: a constant pointer to the units array.
 */
const FacqUnits *facq_stream_data_get_units(const FacqStreamData *stmd)
{
	g_return_val_if_fail(FACQ_IS_STREAM_DATA(stmd),NULL);
	return stmd->units;
}

/**
 * facq_stream_data_get_chanlist:
 * @stmd: A #FacqStreamData object.
 *
 * Returns a read-only #FacqChanlist object, containing the information 
 * related to the physical channels used in the acquisition.
 *
 * Returns: A read-only #FacqChanlist object.
 */
const FacqChanlist *facq_stream_data_get_chanlist(const FacqStreamData *stmd)
{
	g_return_val_if_fail(FACQ_IS_STREAM_DATA(stmd),NULL);
	return stmd->chanlist;
}

/**
 * facq_stream_data_get_max:
 * @stmd: A #FacqStreamData object.
 *
 * Returns a read-only real array, with the expected (by the source)
 * maximum values of each channel.
 *
 * Returns: a read-only real array, you shouldn't try to free it.
 */
const gdouble *facq_stream_data_get_max(const FacqStreamData *stmd)
{
	g_return_val_if_fail(FACQ_IS_STREAM_DATA(stmd),NULL);
	return stmd->max;
}

/**
 * facq_stream_data_get_min:
 * @stmd: A #FacqStreamData object.
 *
 * Returns a read-only real array, with the expected (by the source)
 * minimum values of each channel.
 *
 * Returns: a read-only real array, you shouldn't try to free it.
 */
const gdouble *facq_stream_data_get_min(const FacqStreamData *stmd)
{
	g_return_val_if_fail(FACQ_IS_STREAM_DATA(stmd),NULL);
	return stmd->min;
}

/**
 * facq_stream_data_to_socket:
 * @stmd: A #FacqStreamData object.
 * @socket: A connected #GSocket.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Sends the information contained in the #FacqStreamData (minus bps), @stmd, to the
 * receiver at the other side of the socket connection. It can block or not
 * depending on the "blocking" property of the #GSocket, @socket.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_stream_data_to_socket(const FacqStreamData *stmd,GSocket *socket,GError **err)
{
	gdouble period = 0;
	guint32 n_channels = 0, i = 0;
	guint32 *channels = NULL, *units = NULL;
	gssize ret = -1;
	GError *local_err = NULL;
	gdouble *tmp = NULL;

	/* period */
	period = GDOUBLE_TO_BE(stmd->period);
	ret = facq_net_send(socket,(gchar *)&period,sizeof(gdouble),3,&local_err);
	if(ret != sizeof(gdouble) || local_err)
		goto error;

	/* n_channels */
	n_channels = GUINT32_TO_BE(facq_stream_data_get_n_channels(stmd));
	ret = facq_net_send(socket,(gchar *)&n_channels,sizeof(guint32),3,&local_err);
	if(ret != sizeof(guint32) || local_err)
		goto error;

	/* chanlist */
	channels = facq_chanlist_to_comedi_chanlist(stmd->chanlist,NULL);
	for(i = 0;i < stmd->n_channels;i++){
		channels[i] = GUINT32_TO_BE(channels[i]);
	}
	ret = facq_net_send(socket,(gchar *)channels,stmd->n_channels*sizeof(guint32),3,&local_err);
	if(ret != ( sizeof(guint32)*stmd->n_channels) || local_err)
		goto error;
	g_free(channels);

	/* units */
	units = g_malloc0_n(stmd->n_channels,sizeof(guint32));
	for(i = 0;i < stmd->n_channels;i++){
		units[i] = GUINT32_TO_BE(stmd->units[i]);
	}
	ret = facq_net_send(socket,(gchar *)units,sizeof(guint32)*stmd->n_channels,3,&local_err);
	if(ret != (sizeof(guint32)*stmd->n_channels) || local_err)
		goto error;
	g_free(units);

	/* max and min*/
	tmp = g_malloc0_n(stmd->n_channels,sizeof(gdouble));
	for(i = 0;i < stmd->n_channels;i++){
		tmp[i] = GDOUBLE_TO_BE(stmd->max[i]);
	}
	ret = facq_net_send(socket,(gchar *)&tmp,sizeof(gdouble)*stmd->n_channels,3,&local_err);
	if(ret != (sizeof(gdouble)*stmd->n_channels) || local_err)
		goto error;
	for(i = 0;i < stmd->n_channels;i++){
		tmp[i] = GDOUBLE_TO_BE(stmd->min[i]);
	}
	ret = facq_net_send(socket,(gchar *)&tmp,sizeof(gdouble)*stmd->n_channels,3,&local_err);
	if(ret != (sizeof(gdouble)*stmd->n_channels) || local_err)
		goto error;
	g_free(tmp);

	return TRUE;

	error:
	if(units)
		g_free(units);
	if(tmp)
		g_free(tmp);
	if(channels)
		g_free(channels);
	if(local_err)
		g_propagate_error(err,local_err);
	else
		if(err != NULL)
			g_set_error_literal(err,G_IO_ERROR,
						G_IO_ERROR_FAILED,"Unknown error sending the data");
	return FALSE;
}

/**
 * facq_stream_data_from_socket:
 * @socket: A #GSocket object in connected state.
 * @err: A #GError, it will be set in case of error if not %NULL.
 * 
 * Creates a new #FacqStreamData, and puts the received attributes from
 * the @socket on it.
 *
 * Returns: A new #FacqStreamData object, or %NULL in case of error.
 */
FacqStreamData *facq_stream_data_from_socket(GSocket *socket,GError **err)
{
	guint bps = sizeof(gdouble);
	guint32 n_channels = 0, i = 0, chanspec = 0;
	FacqChanlist *chanlist = NULL;
	FacqUnits *units = NULL;
	gdouble period = 0, *max = NULL, *min = NULL;
	gssize ret = 0;
	GError *local_err = NULL;

	ret = facq_net_receive(socket,(gchar *)&period,sizeof(gdouble),3,&local_err);
	if(ret != sizeof(gdouble) || local_err)
		goto error;
	period = GDOUBLE_TO_BE(period);

	ret = facq_net_receive(socket,(gchar *)&n_channels,sizeof(guint32),3,&local_err);
	if(ret != sizeof(guint32) || local_err)
		goto error;
	n_channels = GUINT32_TO_BE(n_channels);

	chanlist = facq_chanlist_new();
	for(i = 0;i < n_channels;i++){
		ret = facq_net_receive(socket,(gchar *)&chanspec,sizeof(guint32),3,&local_err);
		if(ret != sizeof(guint32) || local_err)
			goto error;
		chanspec = GUINT32_TO_BE(chanspec);
		facq_chanlist_add_chan(chanlist,CR_CHAN(chanspec),0,0,0,0);
	}

	units = g_malloc0(sizeof(guint32)*n_channels);
	for(i = 0;i < n_channels;i++){
		ret = facq_net_receive(socket,(gchar *)&units[i],sizeof(guint32),3,&local_err);
		if(ret != sizeof(guint32) || local_err)
			goto error;
		units[i] = GUINT32_TO_BE(units[i]);
	}

	max = g_malloc0(sizeof(gdouble)*n_channels);
	for(i = 0;i < n_channels;i++){
		ret = facq_net_receive(socket,(gchar *)&max[i],sizeof(gdouble),3,&local_err);
		if(ret != sizeof(gdouble) || local_err)
			goto error;
		max[i] = GDOUBLE_TO_BE(max[i]);
	}

	min = g_malloc0(sizeof(gdouble)*n_channels);
	for(i = 0;i < n_channels;i++){
		ret = facq_net_receive(socket,(gchar *)&min[i],sizeof(gdouble),3,&local_err);
		if(ret != sizeof(gdouble) || local_err)
			goto error;
		min[i] = GDOUBLE_TO_BE(min[i]);
	}

	return facq_stream_data_new(bps,n_channels,period,chanlist,units,max,min);

	error:
	if(min)
		g_free(min);
	if(max)
		g_free(max);
	if(units)
		g_free(units);
	if(chanlist)
		facq_chanlist_free(chanlist);
	if(local_err && err != NULL)
		g_propagate_error(err,local_err);
	return NULL;
}

/**
 * facq_stream_data_to_checksum:
 * @stmd: A #FacqStreamData object.
 * @sum: A #GChecksum object.
 *
 * Dumps all the attributes of the #FacqStreamData, trough the #GChecksum, @sum
 * object, using g_checksum_update().
 */
void facq_stream_data_to_checksum(const FacqStreamData *stmd,GChecksum *sum){
	guint32 tmp = 0, i = 0;
	guint *channels = NULL;
	FacqUnits *units = NULL;
	gdouble tmpd = 0, *max = NULL , *min = NULL;

	g_return_if_fail(sum);
	g_return_if_fail(FACQ_IS_STREAM_DATA(stmd));

	tmpd = stmd->period;
	tmpd = GDOUBLE_TO_BE(tmpd);
	g_checksum_update(sum,(guchar *)&tmpd,sizeof(gdouble));
	tmp = stmd->n_channels;
	tmp = GUINT32_TO_BE(tmp);
	g_checksum_update(sum,(guchar *)&tmp,sizeof(guint32));
	channels = facq_chanlist_to_comedi_chanlist(stmd->chanlist,NULL);
	for(i = 0;i < stmd->n_channels;i++){
		tmp = GUINT32_TO_BE(channels[i]);
		g_checksum_update(sum,(guchar *)&tmp,sizeof(guint32));
	}
	g_free(channels);
	units = stmd->units;
	for(i = 0;i < stmd->n_channels;i++){
		tmp = GUINT32_TO_BE(units[i]);
		g_checksum_update(sum,(guchar *)&tmp,sizeof(guint32));
	}
	max = stmd->max;
	for(i = 0;i < stmd->n_channels;i++){
		tmpd = GDOUBLE_TO_BE(max[i]);
		g_checksum_update(sum,(guchar *)&tmpd,sizeof(gdouble));
	}
	min = stmd->min;
	for(i = 0;i < stmd->n_channels;i++){
		tmpd = GDOUBLE_TO_BE(min[i]);
		g_checksum_update(sum,(guchar *)&tmpd,sizeof(gdouble));
	}
}

/**
 * facq_stream_data_free:
 * @stmd: A #FacqStreamData object.
 *
 * Destroys a no longer needed #FacqStreamData object.
 */
void facq_stream_data_free(FacqStreamData *stmd)
{
	g_return_if_fail(FACQ_IS_STREAM_DATA(stmd));
	g_object_unref(G_OBJECT(stmd));
}

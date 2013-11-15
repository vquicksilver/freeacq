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
#if USE_NIDAQ
#include <glib.h>
#include <gio/gio.h>
#include <string.h>
#include "facqresources.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqnidaq.h"
#include "facqchunk.h"
#include "facqsink.h"
#include "facqsinknidaq.h"
#include "facqmisc.h"
#include "facqlog.h"

/**
 * SECTION:facqsinknidaq
 * @short_description: Provides a NIDAQ based data sink
 * @include:facqsinknidaq.h
 *
 * #FacqSinkNidaq provides a NIDAQ based data sink, that is able to write the
 * input data to a National Instruments data acquisition card using the NIDAQ
 * drivers and software.
 *
 * When you create a new #FacqSinkNidaq object, you must provide info about the
 * NIDAQ device that you want to use, for example "Dev1", the number of channels
 * to be written in the device and which channels. If the number of channels
 * is greater than the number of channels in the stream, only the number of
 * channels in the stream will be taken into consideration. NIDAQ software also
 * requires to input the maximum and minimum expected values.
 *
 * Note that this sink is not intended for real time operation, but it should
 * work fine as long as you have enough cpu time.
 *
 * For creating a new #FacqSinkNidaq object you must call facq_sink_nidaq_new(),
 * to use it you must call first facq_sink_start(), and then you must call in an
 * iterative way facq_sink_nidaq_write(), no need for polling in this case, the
 * samples will be written to the card when the samples are available.
 *
 * #FacqSinkNidaq implements all the required operations by #FacqSink class,
 * take a look there if you need more details.
 *
 * facq_sink_nidaq_to_file(), facq_sink_nidaq_key_constructor() and
 * facq_sink_nidaq_constructor() are used by the system to store the config and
 * to recreate #FacqSinkNidaq objects. See facq_sink_to_file(), the
 * #CIConstructor type and the #CIKeyConstructor for more info.
 */

/**
 * FacqSinkNidaq:
 *
 * Contains the private details of the #FacqSinkNidaq.
 */

/**
 * FacqSinkNidaqClass:
 *
 * Class for the #FacqSinkNidaq objects.
 */

G_DEFINE_TYPE(FacqSinkNidaq,facq_sink_nidaq,FACQ_TYPE_SINK);

GQuark facq_sink_nidaq_error_quark(void)
{
	return g_quark_from_static_string("facq-sink-null-error-quark");
}

enum {
	PROP_0,
	PROP_DEVICE,
	PROP_OUT_CHANLIST,
	PROP_MAX,
	PROP_MIN
};

struct _FacqSinkNidaqPrivate {
	/*< private >*/
	gchar *device;
	FacqChanlist *out_chanlist;
	gdouble max;
	gdouble min;
	FacqNIDAQTask *task;
	guint out_n_channels;
	gdouble *out_buffer;
};

/*****--- GObject magic ---*****/
static void facq_sink_nidaq_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqSinkNidaq *sinknidaq = FACQ_SINK_NIDAQ(self);

	switch(property_id){
	case PROP_DEVICE: sinknidaq->priv->device = g_value_dup_string(value);
	break;
	case PROP_OUT_CHANLIST: sinknidaq->priv->out_chanlist = g_value_get_pointer(value);
	break;
	case PROP_MAX: sinknidaq->priv->max = g_value_get_double(value);
	break;
	case PROP_MIN: sinknidaq->priv->min = g_value_get_double(value);
	break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(sinknidaq,property_id,pspec);	
	}
}

static void facq_sink_nidaq_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqSinkNidaq *sinknidaq = FACQ_SINK_NIDAQ(self);

	switch(property_id){
	case PROP_DEVICE: g_value_set_string(value,sinknidaq->priv->device);
	break;
	case PROP_OUT_CHANLIST: g_value_set_pointer(value,sinknidaq->priv->out_chanlist);
	break;
	case PROP_MAX: g_value_set_double(value,sinknidaq->priv->max);
	break;
	case PROP_MIN: g_value_set_double(value,sinknidaq->priv->min);
	break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(sinknidaq,property_id,pspec);	
	}
}
static void facq_sink_nidaq_finalize(GObject *self)
{
	FacqSinkNidaq *sinknidaq = FACQ_SINK_NIDAQ(self);

	if(sinknidaq->priv->device)
		g_free(sinknidaq->priv->device);
	
	if(sinknidaq->priv->out_buffer)
		g_free(sinknidaq->priv->out_buffer);

	if(sinknidaq->priv->out_chanlist)
		facq_chanlist_free(sinknidaq->priv->out_chanlist);

	if(sinknidaq->priv->task)
		facq_nidaq_task_free(sinknidaq->priv->task);

	G_OBJECT_CLASS (facq_sink_nidaq_parent_class)->finalize (self);
}

static void facq_sink_nidaq_class_init(FacqSinkNidaqClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	FacqSinkClass *sink_class = FACQ_SINK_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FacqSinkNidaqPrivate));

	object_class->finalize = facq_sink_nidaq_finalize;
	object_class->set_property = facq_sink_nidaq_set_property;
	object_class->get_property = facq_sink_nidaq_get_property;
	sink_class->sinksave = facq_sink_nidaq_to_file;
	sink_class->sinkstart = facq_sink_nidaq_start;
	sink_class->sinkwrite = facq_sink_nidaq_write;
	sink_class->sinkstop = facq_sink_nidaq_stop;
	sink_class->sinkfree = facq_sink_nidaq_free;

	g_object_class_install_property(object_class,PROP_DEVICE,
					g_param_spec_string("device",
							     "Device",
							     "The NIDAQ device name, for example Dev1",
							     "Dev1",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_OUT_CHANLIST,
					g_param_spec_pointer("out-chanlist",
							     "Out Chanlist",
							     "The output channel list",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_MAX,
					g_param_spec_double("max",
						            "Max",
							    "The expected maximum value for all the samples in all the channels",
							    -G_MAXDOUBLE,
							    G_MAXDOUBLE,
							    10,
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_MIN,
					g_param_spec_double("min",
							    "Min",
							    "The expected minimum value for all the samples in all the channels",
							    -G_MAXDOUBLE,
							    G_MAXDOUBLE,
							    -10,
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));
}

static void facq_sink_nidaq_init(FacqSinkNidaq *sink)
{
	sink->priv = G_TYPE_INSTANCE_GET_PRIVATE(sink,FACQ_TYPE_SINK_NIDAQ,FacqSinkNidaqPrivate);
	sink->priv->device = NULL;
	sink->priv->out_chanlist = NULL;
	sink->priv->max = 0;
	sink->priv->min = 0;
	sink->priv->task = NULL;
	sink->priv->out_buffer = NULL;
}

/*****--- Public methods ---*****/
/**
 * facq_sink_nidaq_key_constructor:
 * @group_name: The group name to use in the #GKeyFile, @key_file.
 * @key_file: A #GKeyFile object.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * It's purpose it's to create a new #FacqSinkNidaq object from a #GKeyFile and
 * a @group_name. This function is used by #FacqCatalog. See #CIKeyConstructor
 * for more details.
 *
 * Returns: A new #FacqSinkNidaq if successful, %NULL in other case.
 */
gpointer facq_sink_nidaq_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err)
{
	GError *local_err = NULL;
	gchar *dev = NULL;
	FacqChanlist *chanlist = NULL;
	gdouble max = 5, min = 0;

	dev = g_key_file_get_string(key_file,group_name,"dev",&local_err);
	if(local_err)
		goto error;
	
	max = g_key_file_get_double(key_file,group_name,"max",&local_err);
	if(local_err)
		goto error;

	min = g_key_file_get_double(key_file,group_name,"min",&local_err);
	if(local_err)
		goto error;

	chanlist = facq_chanlist_from_key_file(key_file,group_name,&local_err);
	if(local_err)
		goto error;

	return facq_sink_nidaq_new(dev,chanlist,max,min,err);

	error:
	if(local_err){
		if(err)
			g_propagate_error(err,local_err);
	}
	return NULL;
}

/**
 * facq_sink_nidaq_constructor:
 * @user_input: A #GPtrArray with the parameters from the user.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqSinkNidaq object from a #GPtrArray, @user_input, with
 * at least four pointer, the first points to the device name, the second points
 * to the maximum expected value, the third to the minimum expected value, and
 * the forth points to the output chanlist. See facq_sink_nidaq_new() for valid
 * values.
 *
 * This function is used by #FacqCatalog, for creating a #FacqSinkNidaq with the
 * parameters provided by the user in a #FacqDynDialog, take a look at these
 * other objects for more details, and to the #CIConstructor type.
 *
 * Returns: A new #FacqSinkNidaq if successful, %NULL in other case.
 */
gpointer facq_sink_nidaq_constructor(const GPtrArray *user_input,GError **err)
{
	gdouble *max = NULL, *min = NULL;
	FacqChanlist *chanlist = NULL;
	gchar *dev = NULL;

	dev = g_ptr_array_index(user_input,0);
	max = g_ptr_array_index(user_input,1);
	min = g_ptr_array_index(user_input,2);
	chanlist = g_ptr_array_index(user_input,3);

	g_object_ref(chanlist);

	return facq_sink_nidaq_new(dev,chanlist,*max,*min,err);
}

/**
 * facq_sink_nidaq_new:
 * @device: The NIDAQ device name, for example "Dev1".
 * @chanlist: A #FacqChanlist object with the output channels information,
 * it should have at least one I/O channel, and the channels should have
 * a CHAN_OUTPUT direction.
 * @max: The maximum expected value.
 * @min: The minimum expected value.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqSinkNidaq object checking before that all the parameters
 * are fine.
 *
 * Returns: A new #FacqSinkNidaq if successful or %NULL in other case.
 */
FacqSinkNidaq *facq_sink_nidaq_new(const gchar *device,FacqChanlist *chanlist,gdouble max,gdouble min,GError **error)
{
	guint n_channels = 0, i = 0;
	
	if(!device){
		if(error != NULL){
			g_set_error_literal(error,FACQ_SINK_NIDAQ_ERROR,
						FACQ_SINK_NIDAQ_ERROR_FAILED,"Invalid device");
		}
		return NULL;
	}

	if(!chanlist){
		if(error != NULL){
			g_set_error_literal(error,FACQ_SINK_NIDAQ_ERROR,
						FACQ_SINK_NIDAQ_ERROR_FAILED,"Invalid chanlist");
		}
		return NULL;
	}

	n_channels = facq_chanlist_get_io_chans_n(chanlist);

	if(max <= min){
		if(error != NULL){
			g_set_error_literal(error,FACQ_SINK_NIDAQ_ERROR,
						FACQ_SINK_NIDAQ_ERROR_FAILED,"Invalid min or max value");
		}
		return NULL;
	}

	if(n_channels == 0){
		if(error != NULL){
			g_set_error_literal(error,FACQ_SINK_NIDAQ_ERROR,
						FACQ_SINK_NIDAQ_ERROR_FAILED,"Invalid chanlist");
		}
		return NULL;
	}

	for(i = 0;i < n_channels;i++){
		if( facq_chanlist_get_io_chan_direction(chanlist,i) != CHAN_OUTPUT){
			if(error != NULL){
				g_set_error_literal(error,FACQ_SINK_NIDAQ_ERROR,
						FACQ_SINK_NIDAQ_ERROR_FAILED,"Invalid chanlist");
			}
			return NULL;
		}
	}

	return FACQ_SINK_NIDAQ(g_object_new(FACQ_TYPE_SINK_NIDAQ,
					     "name",facq_resources_names_sink_nidaq(),
					     "description",facq_resources_descs_sink_nidaq(),
					     "device",device,
					     "out-chanlist",chanlist,
					     "max",max,
					     "min",min,
					     NULL)
				);
}

/*****--- Virtuals ---*****/
/**
 * facq_sink_nidaq_to_file:
 * @sink: A #FacqSinkNidaq casted to #FacqSink.
 * @file: A #GKeyFile object.
 * @group: The group name to use in the #GKeyFile, @file.
 *
 * Implements the facq_sink_to_file() method.
 * Stores the device name, the maximum and minimum expected values and
 * the output chanlist used by the sink in a #GKeyFile.
 * This is used by the facq_stream_save() function, and you shouldn't need to
 * call this.
 */
void facq_sink_nidaq_to_file(FacqSink *sink,GKeyFile *file,const gchar *group)
{
	FacqSinkNidaq *sinknidaq = FACQ_SINK_NIDAQ(sink);

	g_key_file_set_string(file,group,"dev",sinknidaq->priv->device);
	g_key_file_set_double(file,group,"max",sinknidaq->priv->max);
	g_key_file_set_double(file,group,"min",sinknidaq->priv->min);
	facq_chanlist_to_key_file(sinknidaq->priv->out_chanlist,file,group);
}

/**
 * facq_sink_nidaq_start:
 * @sink: A #FacqSinkNidaq object casted to #FacqSink.
 * @stmd: A #FacqStreamData containing the relevant stream properties.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Starts the #FacqSinkNidaq, allowing it to write data to the DAQ card.
 * Internally the list of channels is adjusted to have the same number
 * of channels as the input data, a buffer for storing samples is allocated
 * and the NIDAQ task is started with this function.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_sink_nidaq_start(FacqSink *sink,const FacqStreamData *stmd,GError **err)
{
	GError *local_err = NULL;
	FacqSinkNidaq *sinknidaq = FACQ_SINK_NIDAQ(sink);
	guint out_n_channels, in_n_channels , n_slices = 0;

	sinknidaq->priv->task = facq_nidaq_task_new("Freeacq Sink Task",&local_err);
	if(local_err)
		goto error;

	in_n_channels = facq_stream_data_get_n_channels(stmd);
	out_n_channels = sinknidaq->priv->task->n_channels;

	/* if the output channel list has more channels than the source we must
	 * ignore the excess of channels, so we delete the last channels from
	 * the output chanlist, making the two chanlists of equal length */

	if(in_n_channels < out_n_channels){
		while(out_n_channels != in_n_channels){
			facq_chanlist_del_chan(sinknidaq->priv->out_chanlist);
			out_n_channels = 
				facq_chanlist_get_io_chans_n(sinknidaq->priv->out_chanlist);
		}
	}

	facq_nidaq_task_add_virtual_chan(sinknidaq->priv->task,
						sinknidaq->priv->device,
							sinknidaq->priv->out_chanlist,
								sinknidaq->priv->max,
									sinknidaq->priv->min,
										&local_err);
	if(local_err)
		goto error;

	sinknidaq->priv->out_n_channels = in_n_channels;

	if(!sinknidaq->priv->out_buffer){
		n_slices = ( facq_misc_period_to_chunk_size(stmd->period,sizeof(gdouble),stmd->n_channels) 
				/	(stmd->n_channels*sizeof(gdouble)) );
#if ENABLE_DEBUG
		facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
				 "Allocating buffer of size""%"G_GSIZE_FORMAT,
				 sizeof(gdouble)*in_n_channels*n_slices);
#endif
		sinknidaq->priv->out_buffer = g_malloc0(sizeof(gdouble)*in_n_channels*n_slices);
	}

	facq_nidaq_task_start(sinknidaq->priv->task,&local_err);
        if(local_err)
                goto error;

	return TRUE;

	error:
	if(local_err){
		g_propagate_error(err,local_err);
	}
	return FALSE;
}

/**
 * facq_sink_nidaq_write:
 * @sink: A #FacqSinkNidaq casted to #FacqSink.
 * @stmd: A #FacqStreamData object with the relevant stream information.
 * @chunk: A #FacqChunk containing the data to be written to the sink.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Write the incoming data in the #FacqChunk, @chunk, to the @sink.
 * Note that the samples in the #FacqChunk are in interleaved format
 * and the sink can only write the samples to the correspondent channels.
 *
 * Returns: A #GIOStatus, %G_IO_STATUS_NORMAL if successful, 
 * %G_IO_STATUS_ERROR in other case.
 */
GIOStatus facq_sink_nidaq_write(FacqSink *sink,const FacqStreamData *stmd,FacqChunk *chunk,GError **err)
{
	gssize ret = 0;
	FacqSinkNidaq *sinknidaq = FACQ_SINK_NIDAQ(sink);
	GError *local_err = NULL;
	guint i = 0;
	gsize n_slices = 0;
	const gdouble *src = NULL;
	gdouble *dst = NULL;

	/* chunk->data contains the input, the input is something like the
	 * following figure where each sample is a gdouble (8 bytes):
	 *
	 *   chan 0   chan 1   ...   chan z
	 * |  s0    |   s1   |     |   sz   |  = slice 0
	 *   ...       ...     ...    ...      = slice ...
	 * |  s0    |   s1   | ... |   sz   |  = slice n
	 *
	 * So for each slice we have to take the first out_n_channels samples
	 * and write them in the write_buffer. For example if out_n_channels == 2:
	 * 
	 *    ao0       ao1
	 * |  s0    |   s1   | = write  0
	 *    ...       ...    = write ...
	 * |  s0    |   s1   | = write  n
	 */ 

	n_slices = facq_chunk_get_total_slices(chunk,sizeof(gdouble),stmd->n_channels);

	if(n_slices){
		for(i = 0;i < n_slices;i++){
			dst = (gdouble *)chunk->data;
			src = (gdouble *)facq_chunk_get_n_slice(chunk,sizeof(gdouble),stmd->n_channels,i);
			dst = dst+(i*stmd->n_channels);
			g_memmove(dst,src,8*stmd->n_channels);
		}

		ret = facq_nidaq_task_write(sinknidaq->priv->task,
					    sinknidaq->priv->out_buffer,
					    n_slices,
					    1.0,
					    &local_err);
		if(local_err){
			g_propagate_error(err,local_err);
			return G_IO_STATUS_ERROR;
		}
	}
	return G_IO_STATUS_NORMAL;
}

/**
 * facq_sink_nidaq_stop:
 * @sink: A #FacqSinkNidaq object casted to #FacqSink.
 * @stmd: A #FacqStreamData object with the relevant stream information.
 * @err: A #GError it will be set in case of error if not %NULL.
 *
 * Stops a previously started #FacqSinkNidaq object. The NIDAQ task created
 * is stopped and then destroyed freeing it's resources in this step.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_sink_nidaq_stop(FacqSink *sink,const FacqStreamData *stmd,GError **err)
{
	GError *local_err = NULL;
	FacqSinkNidaq *sinknidaq = FACQ_SINK_NIDAQ(sink);

	/* TODO: we should should probably call here WaitUntilTaskDone before
	 * stopping the task */
	facq_nidaq_task_stop(sinknidaq->priv->task,&local_err);
	facq_nidaq_task_free(sinknidaq->priv->task);

	g_nullify_pointer((gpointer *)&sinknidaq->priv->task);

	if(local_err)
		goto error;

	return TRUE;

	error:
	if(local_err)
		g_propagate_error(err,local_err);
	return FALSE;
}

/**
 * facq_sink_nidaq_free:
 * @sink: A #FacqSinkNidaq object casted to #FacqSink.
 *
 * Destroys a no longer needed #FacqSinkNidaq object.
 */
void facq_sink_nidaq_free(FacqSink *sink)
{
	g_return_if_fail(FACQ_IS_SINK(sink));
	g_object_unref(G_OBJECT(sink));
}

#endif //USE_NIDAQ

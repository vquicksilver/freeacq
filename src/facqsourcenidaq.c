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
#include <math.h>
#include "facqmisc.h"
#include "facqresources.h"
#include "facqlog.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqsource.h"
#include "facqnidaq.h"
#include "facqsourcenidaq.h"

/**
 * SECTION:facqsourcenidaq
 * @title:FacqSourceNidaq
 * @short_description: Provides a NIDAQ based data source
 * @include:facqnidaq.h,facqsourcenidaq.h
 *
 * #FacqSourceNidaq provides a NIDAQ based data source. That is is able
 * to read data from a National Instruments compatible DAQ card using
 * the National Instruments drivers and software.
 *
 * When a new #FacqSourceNidaq object is created, you must provide a 
 * NIDAQ device, for example "Dev1", a list of channels where the acquisition
 * should take place, a buffer size in samples, a sampling period, the maximum
 * and minimum expected values in all the channels, and optionally a poll
 * interval, if you want to poll the driver for new data.
 *
 * #FacqSourceNidaq can read samples from various channels at the same time as
 * long as your card supports it.
 *
 * In windows systems is also possible to simulate a DAQ card with the National
 * Instruments MAX program, ask in the National Instruments homepage how to do
 * this if you are interested, this kind of simulated DAQ card can also be used
 * with this source.
 *
 * For creating a new #FacqSourceNidaq you must call facq_source_nidaq_new(),
 * to use it you must call first facq_source_start(), and then you must call
 * in an iterative way facq_source_nidaq_poll() and facq_source_nidaq_read(),
 * or use a #FacqStream that will do all those things for you.
 * When you don't need more data simply call facq_source_stop() and
 * facq_source_nidaq_free() to destroy the object.
 *
 * #FacqSourceNidaq implements all the needed operations by the #FacqSource
 * class, take a look there if you need more details.
 *
 * facq_source_nidaq_to_file(), facq_source_nidaq_key_constructor() and
 * facq_source_nidaq_constructor() are used by 
 * the system to store the config and to recreate #FacqSourceNidaq objects.
 * See facq_source_to_file(), the #CIConstructor type and the #CIKeyConstructor
 * for more info.
 */

/**
 * FacqSourceNidaq:
 *
 * Contains the private details of the #FacqSourceNidaq objects.
 */

/**
 * FacqSourceNidaqClass:
 *
 * Class for the #FacqSourceNidaq objects.
 */

/**
 * FacqSourceNidaqError:
 * @FACQ_SOURCE_NIDAQ_ERROR_FAILED: Some error happened in the #FacqSourceNidaq.
 *
 * Enum for all the possible error values in #FacqSourceNidaq.
 */

G_DEFINE_TYPE(FacqSourceNidaq,facq_source_nidaq,FACQ_TYPE_SOURCE);

enum {
	PROP_0,
	PROP_DEVICE,
	PROP_BUFSIZE,
	PROP_SLEEP_US
};

struct _FacqSourceNidaqPrivate {
	gchar *device;
	guint32 nibufsize;
	FacqNIDAQTask *task;
	gint32 samp_per_chan_to_read;
	gulong sleep_us;
};

GQuark facq_source_nidaq_error_quark(void)
{
	return g_quark_from_static_string("facq-source-nidaq-error-quark");
}

/*****--- GObject magic ---*****/
static void facq_source_nidaq_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqSourceNidaq *nidaqsrc = FACQ_SOURCE_NIDAQ(self);

	switch(property_id){
	case PROP_DEVICE: g_value_set_string(value,nidaqsrc->priv->device);
	break;
	case PROP_BUFSIZE: g_value_set_uint(value,nidaqsrc->priv->nibufsize);
	break;
	case PROP_SLEEP_US: g_value_set_ulong(value,nidaqsrc->priv->sleep_us);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(nidaqsrc,property_id,pspec);
	}
}

static void facq_source_nidaq_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqSourceNidaq *nidaqsrc = FACQ_SOURCE_NIDAQ(self);
		
	switch(property_id){
	case PROP_DEVICE: nidaqsrc->priv->device = g_value_dup_string(value);
	break;
	case PROP_BUFSIZE: nidaqsrc->priv->nibufsize = g_value_get_uint(value);
	break;
	case PROP_SLEEP_US: nidaqsrc->priv->sleep_us = g_value_get_ulong(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(nidaqsrc,property_id,pspec);
	}
}

static void facq_source_nidaq_finalize(GObject *self)
{
	FacqSourceNidaq *nidaqsrc = FACQ_SOURCE_NIDAQ(self);

	if(nidaqsrc->priv->device)
		g_free(nidaqsrc->priv->device);
	
	if (G_OBJECT_CLASS (facq_source_nidaq_parent_class)->finalize)
    		(*G_OBJECT_CLASS (facq_source_nidaq_parent_class)->finalize) (self);
}

static void facq_source_nidaq_class_init(FacqSourceNidaqClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	FacqSourceClass *source_class = FACQ_SOURCE_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqSourceNidaqPrivate));

	object_class->set_property = facq_source_nidaq_set_property;
	object_class->get_property = facq_source_nidaq_get_property;
	object_class->finalize = facq_source_nidaq_finalize;

	/* override source class virtual methods */
	source_class->srcsave = facq_source_nidaq_to_file;
	source_class->srcstart = facq_source_nidaq_start;
	source_class->srcpoll = facq_source_nidaq_poll;
	source_class->srcread = facq_source_nidaq_read;
	source_class->srcconv = NULL;
	source_class->srcstop = facq_source_nidaq_stop;
	source_class->srcfree = facq_source_nidaq_free;

	g_object_class_install_property(object_class,PROP_DEVICE,
					g_param_spec_string("device",
							    "NIDAQ device",
							    "The NIDAQ device name, for example Dev1",
							    "Dev1",
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));
	
	g_object_class_install_property(object_class,PROP_BUFSIZE,
					g_param_spec_uint("ni-bufsize",
							  "NIDAQ buffer size",
							  "The NIDAQ DMA buffer size, in samples",
							  0,
							  G_MAXUINT,
							  100000,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_SLEEP_US,
					g_param_spec_ulong("sleep-us",
							   "Sleep microseconds",
							   "Sleep time in microseconds (Polling)",
							   0,
							   G_MAXULONG,
							   100000,
							   G_PARAM_READWRITE |
							   G_PARAM_CONSTRUCT_ONLY |
							   G_PARAM_STATIC_STRINGS));
}

static void facq_source_nidaq_init(FacqSourceNidaq *nidaqsrc)
{
	nidaqsrc->priv = G_TYPE_INSTANCE_GET_PRIVATE(nidaqsrc,FACQ_TYPE_SOURCE_NIDAQ,FacqSourceNidaqPrivate);
	nidaqsrc->priv->device = NULL;
	nidaqsrc->priv->nibufsize = 0;
	nidaqsrc->priv->task = NULL;
}

/*****--- Public methods ---*****/
/**
 * facq_source_nidaq_to_file:
 * @src: A #FacqSourceNidaq object casted to #FacqSource.
 * @file: A #GKeyFile object.
 * @group: The group name to use in the #GKeyFile, @file.
 *
 * Implements the facq_source_nidaq_to_file() method.
 * Stores the device, the buffer size, the maximum and minimum
 * expected values, the sampling period, the poll interval
 * and the list of channels inside a #GKeyFile.
 * This is used by facq_stream_save() function, and you shouldn't
 * need to call this.
 */
void facq_source_nidaq_to_file(FacqSource *src,GKeyFile *file,const gchar *group)
{
	const FacqStreamData *stmd = NULL;
	FacqSourceNidaq *source = FACQ_SOURCE_NIDAQ(src);

	stmd = facq_source_get_stream_data(src);
	g_key_file_set_string(file,group,"dev",source->priv->device);
	g_key_file_set_double(file,group,"ni-bufsize",source->priv->nibufsize);
	g_key_file_set_double(file,group,"max",stmd->max[0]);
	g_key_file_set_double(file,group,"min",stmd->min[0]);
	g_key_file_set_double(file,group,"period",stmd->period);
	g_key_file_set_double(file,group,"sleep-us",(gdouble)source->priv->sleep_us);
	facq_chanlist_to_key_file(stmd->chanlist,file,group);
}

/**
 * facq_source_nidaq_key_constructor:
 * @group_name: The group name to use in the #GKeyFile.
 * @key_file: A #GKeyFile object to read parameters from.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * It's purpose it's to create a #FacqSourceNidaq object from a #GKeyFile and a
 * group name. This function is used by #FacqCatalog. See #CIKeyConstructor
 * for more details.
 *
 * Returns: A new #FacqSourceNidaq if successful or %NULL in other case.
 */
gpointer facq_source_nidaq_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err)
{
	GError *local_err = NULL;
	gchar *dev = NULL;
	guint nibufsize = 0;
	gdouble period = 0, max = 5, min = 0;
	gulong sleep_us = 0;
	FacqChanlist *chanlist = NULL;

	dev = g_key_file_get_string(key_file,group_name,"dev",&local_err);
	if(local_err)
		goto error;

	nibufsize = (guint) g_key_file_get_double(key_file,group_name,"ni-bufsize",&local_err);
	if(local_err)
		goto error;

	max = g_key_file_get_double(key_file,group_name,"max",&local_err);
	if(local_err)
		goto error;

	min = g_key_file_get_double(key_file,group_name,"min",&local_err);
	if(local_err)
		goto error;

	period = g_key_file_get_double(key_file,group_name,"period",&local_err);
	if(local_err)
		goto error;

	sleep_us = (gulong) g_key_file_get_double(key_file,group_name,"sleep-us",&local_err);
	if(local_err)
		goto error;

	chanlist = facq_chanlist_from_key_file(key_file,group_name,&local_err);
	if(local_err)
		goto error;

	return facq_source_nidaq_new(dev,chanlist,nibufsize,period,max,min,sleep_us,err);

	error:
	if(dev)
		g_free(dev);
	if(local_err){
		if(err)
			g_propagate_error(err,local_err);
	}
	return NULL;
}

/**
 * facq_source_nidaq_constructor:
 * @user_input: A #GPtrArray with the parameters from the user.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqSourceNidaq object from a #GPtrArray, @user_input, with
 * at least seven pointers, the first should be a pointer the a string with
 * the device name, the second a pointer to a unsigned integer with the buffer
 * size in samples, the third a pointer to the sampling period value in seconds,
 * forth and fifth pointer are for the maximum an minimum value, the sixth
 * pointer should be a pointer to the polling interval value.
 * Finally the last pointer should point to a #FacqChanlist object.
 *
 * Returns: A new #FacqSourceNidaq object if successful or %NULL in case of
 * error.
 */
gpointer facq_source_nidaq_constructor(const GPtrArray *user_input,GError **err)
{
	gchar *dev = NULL;
	guint *buf_size = NULL;
	gdouble *period = NULL, *max = NULL, *min = NULL;
	guint *sleep_us = NULL;
	FacqChanlist *chanlist = NULL;

	dev = g_ptr_array_index(user_input,0);
	buf_size = g_ptr_array_index(user_input,1);
	period = g_ptr_array_index(user_input,2);
	max = g_ptr_array_index(user_input,3);
	min = g_ptr_array_index(user_input,4);
	sleep_us = g_ptr_array_index(user_input,5);
	chanlist = g_ptr_array_index(user_input,6);

	g_object_ref(chanlist);

	return facq_source_nidaq_new(dev,chanlist,*buf_size,*period,*max,*min,*sleep_us,err);
}

/**
 * facq_source_nidaq_new:
 * @dev: The device name, as shown in MAX application, for example "Dev1".
 * @chanlist: A #FacqChanlist object, containing the information of the channels
 * you want to read data from, it should have at least one I/O channel.
 * @nibufsize: The size of the NIDAQ buffer, in samples.
 * @period: The sampling period in seconds.
 * @max: The absolute maximum value you expect to read in all the
 * channels.
 * @min: The absolute minimum value you expect to read in all the channels.
 * @sleep_us: If not 0, it will enable the polling mode, the value will set the
 * poll interval in microseconds.
 * @error: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqSourceNidaq object, that will read data from the DAQ
 * @device according to the other input parameters.
 *
 * Returns: A new #FacqSourceNidaq object or %NULL in case of error.
 */
FacqSourceNidaq *facq_source_nidaq_new(const gchar *dev,FacqChanlist *chanlist,guint32 nibufsize,gdouble period,gdouble max,gdouble min,gulong sleep_us,GError **error)
{
	FacqStreamData *stmd = NULL;
	FacqUnits *units = NULL;
	guint i = 0, n_channels = 0;
	gdouble *Max = NULL;
	gdouble *Min = NULL;

	if(!dev || !chanlist || period == 0 || max <= min){
		if(error != NULL)
			g_set_error_literal(error,FACQ_SOURCE_NIDAQ_ERROR,
					FACQ_SOURCE_NIDAQ_ERROR_FAILED,"Invalid chanlist, period, max or min value");
		return NULL;
	}

	n_channels = facq_chanlist_get_io_chans_n(chanlist);
	if(n_channels == 0){
		if(error != NULL)
			g_set_error_literal(error,FACQ_SOURCE_NIDAQ_ERROR,
					FACQ_SOURCE_NIDAQ_ERROR_FAILED,"Invalid chanlist, it should have at least one I/O channel");
		return NULL;
	}

	//create units, max and min arrays of n_channels length
	units = g_new0(enum chan_unit,n_channels);
	Max = g_new0(gdouble,n_channels);
	Min = g_new0(gdouble,n_channels);

	for(i = 0;i < n_channels;i++){
		units[i] = UNIT_V;
		Max[i] = max;
		Min[i] = min;
	}

	//we are going to read F64 (float64) samples that equals 8 bytes.
	stmd = facq_stream_data_new(8,n_channels,period,chanlist,units,Max,Min);
	return FACQ_SOURCE_NIDAQ(g_object_new(FACQ_TYPE_SOURCE_NIDAQ,
					       "name",
					       facq_resources_names_source_nidaq(), 
					       "description",
					       facq_resources_descs_source_nidaq(),
					       "stream-data",stmd,
					       "device",dev,
					       "ni-bufsize",nibufsize,
					       "sleep-us",sleep_us,
					       NULL));
}

/**
 * facq_source_nidaq_start:
 * @src: A #FacqSourceNidaq casted to #FacqSource.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Starts the #FacqSourceNidaq, creating and starting a new nidaq task.
 * You should call this function before reading data from the #FacqSourceNidaq.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_source_nidaq_start(FacqSource *src,GError **err)
{
	FacqSourceNidaq *srcnidaq = FACQ_SOURCE_NIDAQ(src);
	const FacqStreamData *stmd = NULL;
	gdouble period = 0, read_sleep_time = 0;
	const gdouble *max = NULL, *min = NULL;
	const FacqChanlist *chanlist = NULL;
	GError *local_err = NULL;
	guint32 serial = 0, onbrdbufsize = 0;
	gchar *transfer_mode = NULL , *read_wait_mode = NULL, *reqcond = NULL;
	gboolean read_all_avail_samp = FALSE;

	stmd = facq_source_get_stream_data(FACQ_SOURCE(src));
	period = facq_stream_data_get_period(stmd);
	max = facq_stream_data_get_max(stmd);
	min = facq_stream_data_get_min(stmd);
	chanlist = facq_stream_data_get_chanlist(stmd);

	srcnidaq->priv->samp_per_chan_to_read = 
		facq_misc_period_to_chunk_size(period,stmd->bps,stmd->n_channels)
				/ (stmd->bps*stmd->n_channels);

	srcnidaq->priv->task = facq_nidaq_task_new("Freeacq Task",&local_err);
	if(local_err)
		goto error;

	facq_nidaq_task_add_virtual_chan(srcnidaq->priv->task,
						srcnidaq->priv->device,
							chanlist,max[0],min[0],&local_err);
	if(local_err)
		goto error;

	facq_nidaq_task_setup_timing(srcnidaq->priv->task,
						period,1,&local_err);
	if(local_err)
		goto error;
	
	facq_nidaq_task_setup_input_buffer(srcnidaq->priv->task,srcnidaq->priv->nibufsize,&local_err);
	if(local_err)
		goto error;

	serial = facq_nidaq_device_serial_get(srcnidaq->priv->device,&local_err);
	if(local_err)
		goto error;

	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,
			 "Starting NIDAQmx task on %s with serial 0x%X",
						srcnidaq->priv->device,serial);

#if ENABLE_DEBUG
	facq_log_write("Triying to read onboard buffer size...",FACQ_LOG_MSG_TYPE_INFO);
	onbrdbufsize = facq_nidaq_task_get_onboard_buffer_size(srcnidaq->priv->task,&local_err);
	
	if(local_err)
		goto error;

	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,
			"Board size equals %u samples per channel",
			onbrdbufsize);


	transfer_mode = facq_nidaq_task_get_xfer_mode(srcnidaq->priv->task,&local_err);
	if(local_err)
		goto error;

	if(transfer_mode){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,
					"Using %s",transfer_mode);
		g_free(transfer_mode);
	}

	reqcond = facq_nidaq_task_get_ai_data_xfer_req_cond(srcnidaq->priv->task,&local_err);
	if(local_err)
		goto error;
	if(reqcond)
		facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,"Data transfer: %s",reqcond);

	read_wait_mode = facq_nidaq_task_get_read_wait_mode(srcnidaq->priv->task,NULL,&local_err);
	if(local_err)
		goto error;

	if(read_wait_mode){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,
					"Using %s",read_wait_mode);
		if(g_strcmp0(read_wait_mode,"Sleep Read Wait Mode") == 0){
			read_sleep_time = 
				facq_nidaq_task_get_read_sleep_time(srcnidaq->priv->task,
								    &local_err);
		}
		
		g_free(read_wait_mode);

		if(local_err)
			goto error;
		else
			facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,
						"Read sleep time = %f",read_sleep_time);
	}
#endif

	facq_nidaq_task_start(srcnidaq->priv->task,&local_err);
	if(local_err)
		goto error;

	return TRUE;

	error:
	if(srcnidaq->priv->task)
		facq_nidaq_task_free(srcnidaq->priv->task);
	g_propagate_error(err,local_err);
	return FALSE;
}

/**
 * facq_source_nidaq_poll:
 * @src: A #FacqSourceNidaq casted to #FacqSource.
 * 
 * Implements facq_source_poll() from #FacqSource.
 * Polls the source the check if new data is available, as long as
 * you passed a sleep_us value greater that 0, else this function
 * will do nothing.
 *
 * Returns: 1 if new data is available, 0 in case of timeout, or
 * -1 in case of error.
 */
gint facq_source_nidaq_poll(FacqSource *src)
{
	FacqSourceNidaq *srcnidaq = NULL;
	guint32 availSampPerChan = 0;
	GError *err = NULL;

	srcnidaq = FACQ_SOURCE_NIDAQ(src);

	if(srcnidaq->priv->sleep_us){
		g_usleep(srcnidaq->priv->sleep_us);

		availSampPerChan = 
			facq_nidaq_task_get_read_avail_samples_per_chan(srcnidaq->priv->task,
									&err);
		if(err){
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",err->message);
			g_clear_error(&err);
			return -1;
		}

		if(availSampPerChan >=  srcnidaq->priv->samp_per_chan_to_read){
			return 1;
		}
		else return 0;
	}

	return 1;
}

/**
 * facq_source_nidaq_read:
 * @src: A #FacqSourceNidaq object casted to #FacqSource.
 * @buf: A pointer to a memory area where the data coming from the source will
 * be stored.
 * @count: The size of the memory area where the data will be stored in bytes.
 * @bytes_read: The number of bytes read from the source.
 * @err: A #GError it will be set in case of error if not %NULL.
 *
 * Implements the facq_source_read() operation from #FacqSource.
 * Reads data from the source, (A maximum of @count bytes) putting the data
 * in the memory area pointed by @buf. When the function returns the control the
 * number of bytes read will be written to @bytes_read.
 *
 * Returns: A #GIOStatus, %G_IO_STATUS_NORMAL if successful or
 * %G_IO_STATUS_ERROR in other case.
 */
GIOStatus facq_source_nidaq_read(FacqSource *src,gchar *buf,gsize count,gsize *bytes_read,GError **err)
{
	FacqSourceNidaq *srcnidaq = NULL;
	const FacqStreamData *stmd = NULL;
	gdouble *buffer = NULL, timeout = 0;
	gssize ret = 0;
	guint32 samps_per_chan = 0;
	GError *local_err = NULL;

	srcnidaq = FACQ_SOURCE_NIDAQ(src);
	stmd = facq_source_get_stream_data(src);
	buffer = (gdouble *)buf;
	samps_per_chan = count/(sizeof(gdouble)*stmd->n_channels);
	
	if(stmd->period > 1)
		timeout = stmd->period + 3.0;
	else
		timeout = 3.0;

	ret = facq_nidaq_task_read(srcnidaq->priv->task,
			           buffer,
				   count/sizeof(gdouble),
				   samps_per_chan,
				   timeout,
				   &local_err);
	if(local_err){
		g_propagate_error(err,local_err);
		return G_IO_STATUS_ERROR;
	}

	*bytes_read = ret*sizeof(gdouble)*stmd->n_channels;

	return G_IO_STATUS_NORMAL;
}

/**
 * facq_source_nidaq_stop:
 * @src: A #FacqSourceNidaq object casted to #FacqSource.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Stops the #FacqSourceNidaq, after this you shouldn't read more data
 * from the source. The nidaq task created on the start function is
 * stopped and destroyed.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_source_nidaq_stop(FacqSource *src,GError **err)
{
	GError *local_err = NULL;
	FacqSourceNidaq *srcnidaq = NULL;

	srcnidaq = FACQ_SOURCE_NIDAQ(src);
	facq_nidaq_task_stop(srcnidaq->priv->task,&local_err);
	facq_nidaq_task_free(srcnidaq->priv->task);
	if(local_err){
		g_propagate_error(err,local_err);
		return FALSE;
	}

	return TRUE;
}

/**
 * facq_source_nidaq_free:
 * @src: A #FacqSourceNidaq object casted to #FacqSource.
 *
 * Destroys a no longer needed #FacqSourceNidaq object.
 */
void facq_source_nidaq_free(FacqSource *src)
{
	g_return_if_fail(FACQ_IS_SOURCE_NIDAQ(src));
	g_object_unref(G_OBJECT(src));
}

#endif // USE_NIDAQ

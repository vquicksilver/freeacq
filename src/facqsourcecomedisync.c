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
#if USE_COMEDI
#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#include <string.h>
#include "facqresources.h"
#include "facqlog.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqsource.h"
#include "facqcomedimisc.h"
#include "facqsourcecomedisync.h"

/**
 * SECTION:facqsourcecomedisync
 * @short_description: A synchronous comedi based data source.
 * @include:facqsourcecomedisync.h
 *
 * #FacqSourceComediSync provides a data source that can work with the comedi
 * synchronous primitives, allowing to read data from a single physical channel
 * in a DAQ card. Synchronous mode is supported by almost all the DAQ cards in
 * the market, and it's the most basic mode. According to comedi documentation
 * this mode is called "Single acquisition" mode. This mode is also called 
 * synchronous mode because, the read from the card, will block until the data
 * is read.
 *
 * In a more specific way, this data source should only be used when you don't need
 * a great precision in time intervals between reads, or when your DAQ hardware
 * doesn't support the asynchronous mode, or when you need a period greater or
 * equal than 2^32 nanoseconds between samples, also keep in mind that in this
 * mode you can only read a single channel at the same time. 
 * This is because in this mode only the operating system clock is used to 
 * track the elapsed time between reads, instead of using a dedicated 
 * hardware clock on the DAQ card, so the time between samples is only and 
 * estimation and can vary according to your CPU load, and your clock resolution.
 * (See <function>clock_getres()</function> and the time man page in the 
 * section 7 for more info about how the operating system works on a modern computer).
 *
 * To create a new #FacqSourceComediSync source use
 * facq_source_comedi_sync_new(), to start the source use
 * facq_source_comedi_sync_start(), to stop the source use facq_source_comedi_sync_stop(),
 * to read one or more samples from the source use
 * facq_source_comedi_sync_read(), to convert data to real samples use
 * facq_source_comedi_sync_conv(), and finally to destroy the source you can use
 * facq_source_comedi_sync_free().
 *
 * Anyway you don't need to call these functions directly, minus
 * facq_source_comedi_sync_new(), if you use a #FacqStream to do all the work for
 * you. For more details on the other functions see each function description.
 *
 * facq_source_comedi_sync_to_file(), facq_source_comedi_sync_key_constructor()
 * and facq_source_comedi_sync_constructor() are used by the system to store
 * the config and to recreate the #FacqSourceComediSync objects.
 * See facq_source_to_file(), the #CIConstructor type and the #CIKeyConstructor
 * for more info.
 * 
 */

/**
 * FacqSourceComediSync:
 *
 * Contains the internal details of the implementation.
 */

/**
 * FacqSourceComediSyncClass:
 *
 * Class for the #FacqSourceComediSync objects.
 */

/**
 * FacqSourceComediSyncError:
 * @FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED: Some error happened in the source.
 *
 * Enum containing the error values for #FacqSourceComediSync.
 */

static void facq_source_comedi_sync_initable_iface_init(GInitableIface  *iface);
static gboolean facq_source_comedi_sync_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);

G_DEFINE_TYPE_WITH_CODE(FacqSourceComediSync,facq_source_comedi_sync,FACQ_TYPE_SOURCE,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_source_comedi_sync_initable_iface_init));

enum {
	PROP_0,
	PROP_INDEX,
	PROP_SUBINDEX,
	PROP_CHAN,
	PROP_RANGE,
	PROP_AREF,
	PROP_DEV
};

struct _FacqSourceComediSyncPrivate {
	/*< private >*/
	guint index;
	guint subindex;
	comedi_t *dev;
	comedi_range *rng;
	lsampl_t maxdata;
	GError *construct_error;
};

GQuark facq_source_comedi_sync_error_quark(void)
{
	return g_quark_from_static_string("facq-source-comedi-sync-error-quark");
}

/*--- GObject magic ---*/
static void facq_source_comedi_sync_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqSourceComediSync *source = FACQ_SOURCE_COMEDI_SYNC(self);

	switch(property_id){
	case PROP_INDEX: g_value_set_uint(value,source->priv->index);
	break;
	case PROP_SUBINDEX: g_value_set_uint(value,source->priv->subindex);
	break;
	case PROP_DEV: g_value_set_pointer(value,source->priv->dev);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(source,property_id,pspec);
	}
}

static void facq_source_comedi_sync_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqSourceComediSync *source = FACQ_SOURCE_COMEDI_SYNC(self);

	switch(property_id){
	case PROP_INDEX: source->priv->index = g_value_get_uint(value);
	break;
	case PROP_SUBINDEX: source->priv->subindex = g_value_get_uint(value);
	break;
	case PROP_DEV: source->priv->dev = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(source,property_id,pspec);
	}
}

static void facq_source_comedi_sync_constructed(GObject *self)
{
	FacqSourceComediSync *source = FACQ_SOURCE_COMEDI_SYNC(self);
	GError *local_err = NULL;
	const FacqStreamData *stmd = NULL;
	const FacqChanlist *chanlist = NULL;
	guint chanspec = 0, chan = 0, range = 0;

	stmd = facq_source_get_stream_data(FACQ_SOURCE(source));
	chanlist = stmd->chanlist;
	
	chanspec = facq_chanlist_get_io_chanspec(chanlist,0);
	facq_chanlist_chanspec_to_src_values(chanspec,&chan,&range,NULL,NULL);

	source->priv->rng = comedi_get_range(source->priv->dev,
					     source->priv->subindex,
					     chan,
					     range);
	if(!source->priv->rng){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
			FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		goto error;
	}

	source->priv->maxdata = comedi_get_maxdata(source->priv->dev,
						   source->priv->subindex,
						   chan);
	if(source->priv->maxdata == 0){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
			FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		goto error;
	}

	return;

	error:
	if(source->priv->dev){
		comedi_close(source->priv->dev);
		source->priv->dev = NULL;
	}
	if(local_err)
		g_propagate_error(&source->priv->construct_error,local_err);
}

static void facq_source_comedi_sync_finalize(GObject *self)
{
	FacqSourceComediSync *source = FACQ_SOURCE_COMEDI_SYNC(self);

	g_clear_error(&source->priv->construct_error);

	if(source->priv->dev)
		comedi_close(source->priv->dev);

	/* don't use g_free on source->priv->rng */

	if (G_OBJECT_CLASS (facq_source_comedi_sync_parent_class)->finalize)
    		(*G_OBJECT_CLASS (facq_source_comedi_sync_parent_class)->finalize) (self);
}

static void facq_source_comedi_sync_class_init(FacqSourceComediSyncClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	FacqSourceClass *source_class = FACQ_SOURCE_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqSourceComediSyncPrivate));

	object_class->finalize = facq_source_comedi_sync_finalize;
	object_class->constructed = facq_source_comedi_sync_constructed;
	object_class->set_property = facq_source_comedi_sync_set_property;
	object_class->get_property = facq_source_comedi_sync_get_property;

	/* override source class virtual methods */
	source_class->srcsave = facq_source_comedi_sync_to_file;
	source_class->srcstart = facq_source_comedi_sync_start;
	source_class->srcpoll = NULL;
	source_class->srcread = facq_source_comedi_sync_read;
	source_class->srcconv = facq_source_comedi_sync_conv;
	source_class->srcstop = facq_source_comedi_sync_stop;
	source_class->srcfree = facq_source_comedi_sync_free;

	g_object_class_install_property(object_class,PROP_INDEX,
					g_param_spec_uint("index",
							  "Index",
							  "The device index",
							  0,
							  G_MAXUINT,
							  0,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_SUBINDEX,
					g_param_spec_uint("subindex",
							  "Subindex",
							  "The subdevice index",
							  0,
							  G_MAXUINT,
							  0,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_DEV,
					g_param_spec_pointer("dev",
							     "Dev",
							     "A comedi device pointer",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));
}

static void facq_source_comedi_sync_init(FacqSourceComediSync *source)
{
	source->priv = G_TYPE_INSTANCE_GET_PRIVATE(source,FACQ_TYPE_SOURCE_COMEDI_SYNC,FacqSourceComediSyncPrivate);
	source->priv->subindex = 0;
	source->priv->dev = NULL;
	source->priv->construct_error = NULL;
	source->priv->rng = NULL;
	source->priv->maxdata = 0;
}

/*****--- GInitable implementation ---*****/
static void facq_source_comedi_sync_initable_iface_init(GInitableIface *iface)
{
	iface->init = facq_source_comedi_sync_initable_init;
}

static gboolean facq_source_comedi_sync_initable_init(GInitable *initable,GCancellable *cancellable,GError  **error)
{
	FacqSourceComediSync *source = NULL;

	g_return_val_if_fail(FACQ_IS_SOURCE_COMEDI_SYNC(initable),FALSE);
	source = FACQ_SOURCE_COMEDI_SYNC(initable);
	if(cancellable != NULL){
		g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                           "Cancellable initialization not supported");
      		return FALSE;
    	}
	if(source->priv->construct_error){
		if (error)
        	*error = g_error_copy(source->priv->construct_error);
      		return FALSE;
	}
	return TRUE;
}

/*****--- Public methods ---*****/
/**
 * facq_source_comedi_sync_to_file:
 * @src: A #FacqSourceComediSync object casted to #FacqSource.
 * @file: A #GKeyFile.
 * @group: The group name in the @file, #GKeyFile.
 *
 * Implements the facq_source_to_file() method.
 * Stores the device index, the device subindex, the period and the channel,
 * in the requested group name, inside a #GKeyFile.
 * This is used by facq_stream_save() function, and you shouldn't need to call
 * this.
 */
void facq_source_comedi_sync_to_file(FacqSource *src,GKeyFile *file,const gchar *group)
{
	FacqSourceComediSync *source = FACQ_SOURCE_COMEDI_SYNC(src);
	const FacqStreamData *stmd = NULL;
	
	stmd = facq_source_get_stream_data(src);
	g_key_file_set_double(file,group,"index",source->priv->index);
	g_key_file_set_double(file,group,"subindex",source->priv->subindex);
	g_key_file_set_double(file,group,"period",stmd->period);
	facq_chanlist_to_key_file(stmd->chanlist,file,group);
}

/**
 * facq_source_comedi_sync_key_constructor:
 * @group_name: A string with the group name.
 * @key_file: A #GKeyFile object.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * It's purpose it's to create a new #FacqSourceComediSync object from a
 * #GKeyFile and a group name. This function is used by #FacqCatalog. See
 * #CIKeyConstructor for more details.
 *
 * Returns: %NULL in case of error, or a new #FacqSourceComediSync object if
 * successful.
 */
gpointer facq_source_comedi_sync_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err)
{
	guint index = 0, subindex = 0;
	gdouble period = 0;
	GError *local_err = NULL;
	FacqChanlist *chanlist = NULL;

	index = (guint) g_key_file_get_double(key_file,group_name,"index",&local_err);
	if(local_err)
		goto error;

	subindex = (guint) g_key_file_get_double(key_file,group_name,"subindex",&local_err);
	if(local_err)
		goto error;

	period = g_key_file_get_double(key_file,group_name,"period",&local_err);
	if(local_err)
		goto error;

	chanlist = facq_chanlist_from_key_file(key_file,group_name,&local_err);
	if(local_err)
		goto error;

	return facq_source_comedi_sync_new(index,subindex,period,chanlist,err);

	error:
	if(local_err){
		if(err)
			g_propagate_error(err,local_err);
	}
	return NULL;
}

/**
 * facq_source_comedi_sync_constructor:
 * @user_input: A #GPtrArray with the parameters from the user.
 * @err: A #GError, it will be set in case of error, if not %NULL.
 *
 * Creates a new #FacqSourceComediSync object from a #GPtrArray, @user_input,
 * with at least 4 pointers, the first is a pointer to the device index, the
 * second is a pointer to the device subindex, the third a pointer to the
 * sampling period value, and the forth a pointer to the channel info.
 * See facq_source_comedi_sync_new() for valid values.
 *
 * This function is used by #FacqCatalog, for creating a #FacqSourceComediSync 
 * with the parameters provided by the user in a #FacqDynDialog, take a look 
 * at this other objects for more details, and to the #CIConstructor type.
 *
 * Returns: A new #FacqSourceComediSync object, or %NULL in case of error.
 */
gpointer facq_source_comedi_sync_constructor(const GPtrArray *user_input,GError **err)
{
	guint *index = NULL, *subindex = NULL;
	gdouble *period = NULL;
	FacqChanlist *chanlist = NULL;

	index = g_ptr_array_index(user_input,0);
	subindex = g_ptr_array_index(user_input,1);
	period = g_ptr_array_index(user_input,2);
	chanlist = g_ptr_array_index(user_input,3);

	g_object_ref(chanlist);

	return facq_source_comedi_sync_new(*index,*subindex,*period,chanlist,err);
}

/**
 * facq_source_comedi_sync_new:
 * @index: The device index.
 * @subindex: The subdevice index.
 * @period: The sampling period, in seconds.
 * @chanlist: A #FacqChanlist object.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqSourceComediSync data source object. It requires a valid
 * comedi device index and subindex, also note that chanlist should have one or more IO
 * Channels, but note that only the first channel will be used. This is due to
 * the way comedi synchronous primitives work (Instructions can only be used to read a
 * single channel).
 * The period can be any value between 1e-3 seconds and G_MAXULONG
 * microseconds and should be in seconds.
 *
 * returns: A new #FacqSourceComediSync object, or %NULL in case of error.
 */
FacqSourceComediSync *facq_source_comedi_sync_new(guint index,guint subindex,gdouble period,FacqChanlist *chanlist,GError **err)
{
	FacqStreamData *stmd = NULL;
	FacqSourceComediSync *source = NULL;
	FacqUnits *units = NULL;
	gdouble *max = NULL;
	gdouble *min = NULL;
	gchar *devfilename = NULL;
	comedi_t *dev = NULL;
	GError *local_err = NULL;
	gint subdevice_type = 0, n_subdevices = 0;
	guint chanspec = 0, chan = 0, range = 0;
	comedi_range *rng = NULL;

	//period should be 1 microsecond (1e-3 seconds) or more.
	if(period < (1e-3)){
		if(err != NULL)
			g_set_error_literal(err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
					FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,"Invalid period value");
		return NULL;		
	}
	// g_usleep can sleep up to G_MAXULONG microseconds, that will be the
	// maximum period that this source can have.
	if(period*G_USEC_PER_SEC > G_MAXULONG){
		if(err != NULL)
			g_set_error_literal(err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
					FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,"Invalid period value");
		return NULL;
	}
	if(!chanlist){
		if(err != NULL)
			g_set_error_literal(err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
					FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,"Invalid chanlist");
		return NULL;
	}
	if(facq_chanlist_get_io_chans_n(chanlist) != 1){
		if(err != NULL)
			g_set_error_literal(err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
					FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,"Invalid chanlist");
		return NULL;
	}
	devfilename = g_strdup_printf("%s%u","/dev/comedi",index);
	dev = comedi_open(devfilename);
	g_free(devfilename);
	devfilename = NULL;
	if(!dev){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
			FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		goto error;
	}

	n_subdevices = comedi_get_n_subdevices(dev);
	if(n_subdevices < 0){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
			FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		goto error;
	}
	if(subindex >= n_subdevices){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
			FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,
				"Invalid subdevice");
		goto error;
	}

	subdevice_type = comedi_get_subdevice_type(dev,subindex);
	if(subdevice_type < 0){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
			FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		goto error;
	}
	if(subdevice_type != COMEDI_SUBD_AI && 
		subdevice_type != COMEDI_SUBD_DI &&
			subdevice_type != COMEDI_SUBD_DIO){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
			FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,
				"This kind of subdevice is not supported");
		goto error;
	}
	
	if(!facq_comedi_misc_test_chanlist(dev,subindex,chanlist,&local_err)){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
		g_clear_error(&local_err);
		if(err != NULL){
			g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
					FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,"Invalid chanlist");
			goto error;
		}
	}
	
	chanspec = facq_chanlist_get_io_chanspec(chanlist,0);
	facq_chanlist_chanspec_to_src_values(chanspec,&chan,&range,NULL,NULL);
	rng = comedi_get_range(dev,subindex,chan,range);
	if(!rng){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
			FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		goto error;
	}

	//ensure that we don't get NaN numbers during conversion
	comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);

	//create units
	units = g_new0(FacqUnits,1);
	max = g_new0(gdouble,1);
	min = g_new0(gdouble,1);

	units[0] = rng->unit;
	max[0] = rng->max;
	min[0] = rng->min;
	
	stmd = facq_stream_data_new(sizeof(lsampl_t),1,period,chanlist,units,max,min);
	source = FACQ_SOURCE_COMEDI_SYNC(g_initable_new(FACQ_TYPE_SOURCE_COMEDI_SYNC,NULL,err,
					       "name",
					       facq_resources_names_source_comedi_sync(), 
					       "description",
					       facq_resources_descs_source_comedi_sync(),
					       "stream-data",stmd,
					       "index",index,
					       "subindex",subindex,
					       "dev",dev,
					       NULL));
	if(!source){
		goto error;
	}

	return source;

	error:
	if(stmd)
		facq_stream_data_free(stmd);
	if(local_err)
		g_propagate_error(err,local_err);
	if(devfilename)
		g_free(devfilename);
	if(dev)
		comedi_close(dev);
	return NULL;
}

/**
 * facq_source_comedi_sync_start:
 * @src: A #FacqSourceComediSync casted to #FacqSource.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * It's purpose it's to prepare the source for data acquisition, also locks the
 * comedi device, so no other process can interfere in the acquisition.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_source_comedi_sync_start(FacqSource *src,GError **err)
{
	FacqSourceComediSync *source = FACQ_SOURCE_COMEDI_SYNC(src);
	GError *local_err = NULL;

	if( comedi_lock(source->priv->dev,source->priv->subindex) < 0){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
			FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		goto error;
	}

	return TRUE;

	error:
	if(local_err)
		g_propagate_error(err,local_err);
	return FALSE;
}

/**
 * facq_source_comedi_sync_read:
 * @src: A #FacqSourceComediSync casted to #FacqSource.
 * @buf: A pointer to a free memory area.
 * @count: The size of the memory area pointed by @buf, in bytes.
 * It should be a multiple of the bits per sample supported by the card.
 * @bytes_read: It will store the number of bytes read.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * Reads data from the source, (A maximum of @count bytes) putting it in the
 * memory area pointed by @buf. When the function returns the control the number
 * of bytes read will be written to @bytes_read.
 * Note that the function will block until all the requested bytes are read.
 *
 * Returns: %G_IO_STATUS_NORMAL if successful, %G_IO_STATUS_ERROR in case of
 * error.
 */
GIOStatus facq_source_comedi_sync_read(FacqSource *src,gchar *buf,gsize count,gsize *bytes_read,GError **err)
{
	GError *local_err = NULL;
	guint chanspec = 0, chan = 0, range = 0, aref = 0, i = 0;
	const FacqStreamData *stmd = NULL;
	const FacqChanlist *chanlist = NULL;
	FacqSourceComediSync *source = NULL;
	lsampl_t *data = NULL;

#if ENABLE_DEBUG
	g_return_val_if_fail(FACQ_IS_SOURCE_COMEDI_SYNC(src),G_IO_STATUS_ERROR);
#endif
	source = FACQ_SOURCE_COMEDI_SYNC(src);
	stmd = facq_source_get_stream_data(FACQ_SOURCE(source));
	chanlist = stmd->chanlist;
	chanspec = facq_chanlist_get_io_chanspec(chanlist,0);
	facq_chanlist_chanspec_to_src_values(chanspec,&chan,&range,&aref,NULL);

	data = (lsampl_t *)buf;

	for(i = 0;i < count/stmd->bps;i++){
		/* we must sleep once for each read */
		g_usleep(stmd->period*G_USEC_PER_SEC);

		/* read one sample */
		if( comedi_data_read(source->priv->dev,
				     source->priv->subindex,
				     chan,
				     range,
				     aref,
				     data) < 0 ){
		
			g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
				FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
			goto error;
		}
		/* point to next lsampl_t with data */
		data++;
	}

	*bytes_read = count;
	
	return G_IO_STATUS_NORMAL;

	error:
	if(local_err){
		g_propagate_error(err,local_err);
	}
	return G_IO_STATUS_ERROR;
}

/**
 * facq_source_comedi_sync_conv:
 * @src: A #FacqSourceComediSync object casted to #FacqSource.
 * @ori: A pointer to the memory area where the integer samples are stored.
 * @dst: A pointer to the memory area where the real samples are stored.
 * @samples: The number of samples to convert.
 *
 * Convert the number of samples specified in @samples, to real samples from
 * integer values as returned by the card.
 * See facq_source_conv() for more info.
 *
 */
void facq_source_comedi_sync_conv(FacqSource *src,gpointer ori,gdouble *dst,gsize samples)
{
	FacqSourceComediSync *source = FACQ_SOURCE_COMEDI_SYNC(src);
	lsampl_t *data = (lsampl_t *)ori;
	gsize i = 0;

	for(i = 0;i < samples;i++){
		//analog device
		if(source->priv->maxdata != 1){
			dst[i] = comedi_to_phys(data[i],
					source->priv->rng,
						source->priv->maxdata);
		}
		//digital device
		else {
			if(data[1] == 1)
				dst[i] = source->priv->rng->max;
			else
				dst[i] = source->priv->rng->min;
		}
	}
}

/**
 * facq_source_comedi_sync_stop:
 * @src: A #FacqSourceComediSync casted to #FacqSource.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Stops the source, unlocking the comedi device to other processes.
 * You should call this function when you don't want to read more data
 * from the source.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_source_comedi_sync_stop(FacqSource *src,GError **err)
{
	FacqSourceComediSync *source = FACQ_SOURCE_COMEDI_SYNC(src);
	GError *local_err = NULL;

	if( comedi_unlock(source->priv->dev,source->priv->subindex) < 0){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_SYNC_ERROR,
			FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		goto error;
	}

	return TRUE;

	error:
	if(local_err)
		g_propagate_error(err,local_err);
	return FALSE;
}

/**
 * facq_source_comedi_sync_free:
 * @src: A #FacqSourceComediSync object casted to #FacqSource.
 *
 * Destroys a no longer needed #FacqSourceComediSync object.
 */
void facq_source_comedi_sync_free(FacqSource *src)
{
	g_return_if_fail(FACQ_IS_SOURCE_COMEDI_SYNC(src));
	g_object_unref(G_OBJECT(src));
}
#endif //USE_COMEDI

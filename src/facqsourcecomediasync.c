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
#if GLIB_MINOR_VERSION >= 30
#include <glib-unix.h>
#endif
#include <glib/gprintf.h>
#include <gio/gio.h>
#include <string.h>
#include "facqglibcompat.h"
#include "facqresources.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqsource.h"
#include "facqcomedimisc.h"
#include "facqsourcecomediasync.h"

/**
 * SECTION:facqsourcecomediasync
 * @short_description: The data source for comedi asynchronous devices.
 * @title:FacqSourceComediAsync
 * @see_also: #FacqSource,#GInitable
 * @include: facqsourcecomediasync.h
 *
 * This module provides a data source that can deal with all the <ulink
 * url="http://comedi.org">comedi</ulink> asynchronous input devices.
 * <ulink url="http://comedi.org/doc/commandsstreaming.html">Command</ulink> functionality needs to be
 * supported by the device driver. If your DAQ card doesn't support command
 * functionality you should use #FacqSourceComediSync instead.
 *
 * For creating a new #FacqSourceComediAsync, you should call
 * facq_source_comedi_async_new() and for destroying it you should call
 * facq_source_comedi_async_free(). The other defined functions
 * facq_source_comedi_async_start(), facq_source_comedi_async_stop() and
 * facq_source_comedi_async_conv() are there for implementing all the 
 * needed functions by the #FacqSource class and should not be called in a 
 * direct manner by the user. (The #FacqPipeline will call
 * this functions during it's normal operation).
 *
 * For providing this functionality this module implements the required
 * functions from #FacqSource class.
 *
 * <sect1 id="internal-details">
 *  <title>Internal details</title>
 *  <para>
 *  Comedi library is a complex beast, hence the need for documenting the
 *  internal details of this module, in the following subsections.
 *  </para>
 *  <sect2 id="comedi-async-new">
 *   <title>facq_source_comedi_async_new()</title>
 *   <para>
 *   This function calls the constructor, but before calling the constructor it
 *   does some checks:
 *   </para>
 *   <itemizedlist>
 *    <listitem>
 *     <para>
 *     period > 0
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Open the device, from the index and subindex values.
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Get the subdevice flags, and check for SDF_CMD and SDF_READABLE. This
 *     check ensures that the subdevice supports commands and can be read (Input
 *     subdevice).
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Test the flags parameter with facq_comedi_misc_test_flags(). (This is a TODO).
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Test if the device can be calibrated with
 *     facq_comedi_misc_can_calibrate(). If the device can be calibrated
 *     check if it's calibrated with facq_comedi_misc_test_calibrated().
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Test the chanlist and the period calling facq_comedi_misc_test_chanlist() and
 *     facq_comedi_misc_test_period().
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     If calibration is supported by the device, get the comedi_polynomial_t
 *     for each channel in the chanlist calling
 *     facq_comedi_misc_get_polynomial().
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Get the values for units, maximums and minimums for each channel, and the
 *     value of the bps for the subdevice, and create a new #FacqStreamData
 *     object, that will be owned by the source when created.
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Call the constructor with the required parameters.
 *     </para>
 *    </listitem>
 *   </itemizedlist>
 *  </sect2>
 *  <sect2 id="constructed">
 *   <title>facq_source_comedi_async_constructed()</title>
 *   <para>
 *   This function does the following steps to construct the object.
 *   </para>
 *   <itemizedlist>
 *    <listitem>
 *     <para>
 *     Check if the device driver is blacklisted. This is needed because some
 *     drivers can't be polled without causing a kernel panic. This is done
 *     using the facq_comedi_misc_can_poll().
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Obtain the file descriptor of the device with the function
 *     <function>comedi_fileno()</function>. If the device can be polled (is not
 *     blacklisted) make the file descriptor non-blocking with
 *     g_unix_set_fd_nonblocking() and put the file descriptor on a #GPollFD
 *     object. Finally a #GIOChannel is created with the file descriptor,
 *     setting the encoding to %NULL (Binary data).
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Get the data for doing the conversion of the data from the device to real
 *     values. If the device can't be calibrated, the system needs for each
 *     channel the comedi_range, and maxdata, if the device can be calibrated we
 *     already have all the required data. This data is required by the
 *     <function>comedi_to_phys()</function> and <function>comedi_to_physical()</function> functions.
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Create a new comedi_cmd with facq_comedi_misc_cmd_new(), and setup it
 *     with the function <function>comedi_get_cmd_generic_timed()</function>, add the
 *     chanlist to the command with facq_comedi_misc_cmd_add_chanlist().
 *     Then setup the flags, the stop source and the stop arg of the command.
 *     Finally the command is tested twice with the function
 *     <function>comedi_command_test()</function>.
 *     </para>
 *    </listitem>
 *   </itemizedlist>
 *  </sect2>
 *  <sect2 id="start">
 *   <title>facq_source_comedi_async_start()</title>
 *    <para>
 *    This function starts the acquisition sending the command to the device
 *    driver with <function>comedi_command()</function>, but first locks the subdevice with
 *    <function>comedi_lock()</function> so no other process could interfere with
 *    our acquistion of data.
 *    </para>
 *  </sect2>
 *  <sect2 id="poll">
 *   <title>facq_source_comedi_async_poll()</title>
 *    <para>
 *    When a comedi device is opened, you can obtain a file descriptor with the
 *    <function>comedi_fileno()</function>, this file descriptor is polled in
 *    this function, using the g_poll() function.
 *    </para>
 *  </sect2>
 *  <sect2 id="read">
 *   <title>facq_source_comedi_async_read()</title>
 *    <para>
 *    Reads a chunk from the comedi device, using the g_io_channels_read_chars() function.
 *    </para>
 *  </sect2>
 *  <sect2 id="conv">
 *   <title>facq_source_comedi_async_conv()</title>
 *    <para>
 *    Converts the data read in the facq_source_comedi_async_read() function to
 *    real values, using the <function>comedi_to_physical()</function> if the
 *    device can be calibrated or <function>comedi_to_phys()</function> if the
 *    device can't be calibrated.
 *    </para>
 *  </sect2>
 *  <sect2 id="stop">
 *   <title>facq_source_comedi_async_stop()</title>
 *    <para>
 *    This function stop the acquisition of data, and unlocks the subdevice.
 *    For doing so it checks the subdevice flags for SDF_RUNNING presence if so,
 *    it calls <function>comedi_cancel()</function>. The unlocking is done with
 *    the <function>comedi_unlock()</function> function.
 *    </para>
 *  </sect2>
 *  <sect2 id="finalize">
 *   <title>facq_source_comedi_async_finalize()</title>
 *   <para>
 *   Destroys the private members of the object, and closes the comedi device
 *   using <function>comedi_close()</function> function.
 *   </para>
 *  </sect2>
 * </sect1>
 */ 

 /**
  * FacqSourceComediAsync:
  *
  * Contains the internal details of the #FacqSourceComediAsync objects.
  */

 /**
  * FacqSourceComediAsyncClass:
  *
  * Class for the #FacqSourceComediAsync objects.
  */

 /**
  * FacqSourceComediAsyncError:
  * @FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED: Some error happened in the source.
  *
  * Enum for the #FacqSourceComediAsync error values.
  */

static void facq_source_comedi_async_initable_iface_init(GInitableIface  *iface);
static gboolean facq_source_comedi_async_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);

G_DEFINE_TYPE_WITH_CODE(FacqSourceComediAsync,facq_source_comedi_async,FACQ_TYPE_SOURCE,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_source_comedi_async_initable_iface_init));

#define NEED_SOFT_CALIBRATION "You must run comedi_soft_calibrate first"
#define NEED_HARD_CALIBRATION "You must run comedi_calibrate first"

enum {
	PROP_0,
	PROP_INDEX,
	PROP_SUBINDEX,
	PROP_FLAGS,
	PROP_POLY,
	PROP_DEV,
};

struct _FacqSourceComediAsyncPrivate {
	/*< private >*/
	guint index;
	guint subindex;
	guint flags;
	comedi_polynomial_t *p;
	comedi_t *dev;
	comedi_cmd *cmd;
	comedi_range *rng;
	lsampl_t *maxdata;
	gboolean can_poll;
	GPollFD *pfd;
	GIOChannel *channel;
	GError *construct_error;
};

GQuark facq_source_comedi_async_error_quark(void)
{
	return g_quark_from_static_string("facq-source-comedi-async-error-quark");
}

/*--- GObject magic ---*/
static void facq_source_comedi_async_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqSourceComediAsync *source = FACQ_SOURCE_COMEDI_ASYNC(self);

	switch(property_id){
	case PROP_INDEX: g_value_set_uint(value,source->priv->index);
	break;
	case PROP_SUBINDEX: g_value_set_uint(value,source->priv->subindex);
	break;
	case PROP_FLAGS: g_value_set_uint(value,source->priv->flags);
	break;
	case PROP_DEV: g_value_set_pointer(value,source->priv->dev);
	break;
	case PROP_POLY: g_value_set_pointer(value,source->priv->p);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(source,property_id,pspec);
	}
}

static void facq_source_comedi_async_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqSourceComediAsync *source = FACQ_SOURCE_COMEDI_ASYNC(self);

	switch(property_id){
	case PROP_INDEX: source->priv->index = g_value_get_uint(value);
	break;
	case PROP_SUBINDEX: source->priv->subindex = g_value_get_uint(value);
	break;
	case PROP_FLAGS: source->priv->flags = g_value_get_uint(value);
	break;
	case PROP_DEV: source->priv->dev = g_value_get_pointer(value);
	break;
	case PROP_POLY: source->priv->p = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(source,property_id,pspec);
	}
}


static void facq_source_comedi_async_constructed(GObject *self)
{
	FacqSourceComediAsync *source = FACQ_SOURCE_COMEDI_ASYNC(self);
	FacqSource *src = FACQ_SOURCE(self);
	const FacqChanlist *chanlist = NULL;
	guint period_ns = 0, i = 0, n_channels = 0, chanspec = 0, chan = 0, range = 0;
	gdouble period = 0;
	const FacqStreamData *stmd = NULL;
	GError *local_err = NULL;
	gint fd = -1;
	comedi_range *tmp = NULL;
	
	stmd = facq_source_get_stream_data(src);
	
	//check if we can poll the device driver
	source->priv->can_poll =
		facq_comedi_misc_can_poll(source->priv->dev,&local_err);
	if(local_err)
		goto error;

	fd = comedi_fileno(source->priv->dev);
	if(fd < 0){
		g_set_error_literal(&local_err,
			FACQ_SOURCE_COMEDI_ASYNC_ERROR,
				FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
		goto error;
	}
	//if the device is pollable make the fd non blocking and
	//create a GPollFD.
	if(source->priv->can_poll){
		g_unix_set_fd_nonblocking(fd,TRUE,&local_err);
		if(local_err)
			goto error;

		source->priv->pfd = g_new0(GPollFD,1);
		source->priv->pfd->fd = fd;
		source->priv->pfd->events = G_IO_IN | G_IO_ERR;
	}
	
	//create a new GIOChannel with the fd, set encoding to NULL
	source->priv->channel = g_io_channel_unix_new(fd);
	g_assert(source->priv->channel != NULL);
	if(g_io_channel_set_encoding(source->priv->channel,
                        NULL,&local_err) 
                                != G_IO_STATUS_NORMAL)
                goto error;
	
	//get some data from the stream data object
	period = facq_stream_data_get_period(stmd);
	period_ns = period * 1e9;
	n_channels = facq_stream_data_get_n_channels(stmd);
	chanlist = facq_stream_data_get_chanlist(stmd);
	
	//if p is null the conversion will use
	// comedi_to_phys, so we need each channel comedi_range,
	// and each channel maxdata.
	if(!source->priv->p){
		source->priv->rng = g_new0(comedi_range,n_channels);
		source->priv->maxdata = g_new0(lsampl_t,n_channels);
		for(i = 0;i < n_channels;i++){
			chanspec = facq_chanlist_get_io_chanspec(chanlist,i);
			facq_chanlist_chanspec_to_src_values(chanspec,
							     &chan,
							     &range,
							     NULL,
							     NULL);
			tmp = comedi_get_range(source->priv->dev,
					       source->priv->subindex,
					       chan,range);
			if(!tmp){
				g_set_error_literal(&local_err,
					FACQ_SOURCE_COMEDI_ASYNC_ERROR,
						FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
						comedi_strerror(comedi_errno()));
				goto error;
			}
			source->priv->rng[i].unit = tmp->unit;
			source->priv->rng[i].max = tmp->max;
			source->priv->rng[i].min = tmp->min;
			source->priv->maxdata[i] = 
				comedi_get_maxdata(source->priv->dev,
						   source->priv->subindex,
						   chan);
			if(source->priv->maxdata[i] == 0){
				g_set_error_literal(&local_err,
					FACQ_SOURCE_COMEDI_ASYNC_ERROR,
						FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
						comedi_strerror(comedi_errno()));
				goto error;
			}
		}
	}
	
	/*---- Resume: at this point we have all the needed data, for the
	 * conversions we have the polynomial array p, or rng+maxdata,
	 * if the device is pollable we have a GPollFD pfd. And we can
	 * read data fromt the device using the GIOChannel channel ----*/

	//create a new cmd for the async adquisition
	source->priv->cmd = 
		facq_comedi_misc_cmd_new(source->priv->subindex);

	//prepare the cmd with the period
	if( comedi_get_cmd_generic_timed(source->priv->dev,
				     source->priv->subindex,
				     source->priv->cmd,
				     n_channels,
				     period_ns) < 0){
		g_set_error_literal(&local_err,
			FACQ_SOURCE_COMEDI_ASYNC_ERROR,
				FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
		goto error;
	}
				     
	
	//append the chanlist to the cmd
	facq_comedi_misc_cmd_add_chanlist(source->priv->cmd,
					  chanlist);
	//append the flags to the cmd
	source->priv->cmd->flags |= source->priv->flags;

	//adquisition should be infinite until stop
	source->priv->cmd->stop_src = TRIG_NONE;
	source->priv->cmd->stop_arg = 0;

	//double check the cmd like in the comedi examples
	if(comedi_command_test(source->priv->dev,
			       source->priv->cmd) < 0){
		g_set_error_literal(&local_err,
			FACQ_SOURCE_COMEDI_ASYNC_ERROR,
				FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
	}
	if(comedi_command_test(source->priv->dev,
			       source->priv->cmd) < 0){
		g_set_error_literal(&local_err,
			FACQ_SOURCE_COMEDI_ASYNC_ERROR,
				FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
	}


	return;

	error:
	if(source->priv->rng)
		g_free(source->priv->rng);
	if(source->priv->maxdata)
		g_free(source->priv->maxdata);
	if(source->priv->pfd)
		g_free(source->priv->pfd);
	if(source->priv->channel)
		g_io_channel_unref(source->priv->channel);
	if(source->priv->dev)
		comedi_close(source->priv->dev);
	if(source->priv->p)
		g_free(source->priv->p);
	if(source->priv->cmd){
		if(source->priv->cmd->chanlist)
			g_free(source->priv->cmd->chanlist);
		g_free(source->priv->cmd);
	}
	if(local_err)
		g_propagate_error(&source->priv->construct_error,local_err);
}

static void facq_source_comedi_async_finalize(GObject *self)
{
	FacqSourceComediAsync *source = FACQ_SOURCE_COMEDI_ASYNC(self);

	g_clear_error(&source->priv->construct_error);

	if(source->priv->p)
		g_free(source->priv->p);

	if(source->priv->cmd)
		g_free(source->priv->cmd);

	if(source->priv->rng)
		g_free(source->priv->rng);

	if(source->priv->maxdata)
		g_free(source->priv->maxdata);

	if(source->priv->pfd)
		g_free(source->priv->pfd);

	if(source->priv->dev)
		comedi_close(source->priv->dev);

	if(source->priv->channel)
		g_io_channel_unref(source->priv->channel);

	if (G_OBJECT_CLASS (facq_source_comedi_async_parent_class)->finalize)
    		(*G_OBJECT_CLASS (facq_source_comedi_async_parent_class)->finalize) (self);
}

static void facq_source_comedi_async_class_init(FacqSourceComediAsyncClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	FacqSourceClass *source_class = FACQ_SOURCE_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqSourceComediAsyncPrivate));

	object_class->finalize = facq_source_comedi_async_finalize;
	object_class->constructed = facq_source_comedi_async_constructed;
	object_class->set_property = facq_source_comedi_async_set_property;
	object_class->get_property = facq_source_comedi_async_get_property;

	/* override source class virtual methods */
	source_class->srcsave = facq_source_comedi_async_to_file;
	source_class->srcstart = facq_source_comedi_async_start;
	source_class->srcpoll = facq_source_comedi_async_poll;
	source_class->srcread = facq_source_comedi_async_read;
	source_class->srcconv = facq_source_comedi_async_conv;
	source_class->srcstop = facq_source_comedi_async_stop;
	source_class->srcfree = facq_source_comedi_async_free;

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

	g_object_class_install_property(object_class,PROP_FLAGS,
					g_param_spec_uint("flags",
							  "Flags",
							  "The command flags",
							  0,
							  G_MAXUINT,
							  0,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_POLY,
					g_param_spec_pointer("poly",
							  "Polynomial",
							  "Comedi polynomials for conversion",
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

static void facq_source_comedi_async_init(FacqSourceComediAsync *source)
{
	source->priv = G_TYPE_INSTANCE_GET_PRIVATE(source,FACQ_TYPE_SOURCE_COMEDI_ASYNC,FacqSourceComediAsyncPrivate);
	source->priv->index = 0;
	source->priv->subindex = 0;
	source->priv->flags = 0;
	source->priv->p = NULL;
	source->priv->dev = NULL;
	source->priv->cmd = NULL;
	source->priv->rng = NULL;
	source->priv->maxdata = NULL;
	source->priv->can_poll = FALSE;
	source->priv->pfd = NULL;
	source->priv->channel = NULL;
	source->priv->construct_error = NULL;
}

/*****--- GInitable implementation ---*****/
static void facq_source_comedi_async_initable_iface_init(GInitableIface *iface)
{
	iface->init = facq_source_comedi_async_initable_init;
}

static gboolean facq_source_comedi_async_initable_init(GInitable *initable,GCancellable *cancellable,GError  **error)
{
	FacqSourceComediAsync *source = NULL;

	g_return_val_if_fail(FACQ_IS_SOURCE_COMEDI_ASYNC(initable),FALSE);
	source = FACQ_SOURCE_COMEDI_ASYNC(initable);
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
 * facq_source_comedi_async_to_file:
 * @src: A #FacqSourceComediSync object casted to #FacqSource.
 * @file: A #GKeyFile object.
 * @group: The group name in the @file, #GKeyFile.
 *
 * Implements the facq_source_to_file() method.
 * Stores the device index, subindex, the acquisition flags and the period
 * inside a #GKeyFile.
 * This is used by facq_stream_save() function, and you shouldn't need to call
 * this.
 *
 */
void facq_source_comedi_async_to_file(FacqSource *src,GKeyFile *file,const gchar *group)
{
	FacqSourceComediAsync *source = FACQ_SOURCE_COMEDI_ASYNC(src);
	const FacqStreamData *stmd = NULL;

	stmd = facq_source_get_stream_data(src);
	g_key_file_set_double(file,group,"index",source->priv->index);
	g_key_file_set_double(file,group,"subindex",source->priv->subindex);
	g_key_file_set_double(file,group,"flags",source->priv->flags);
	g_key_file_set_double(file,group,"period",stmd->period);
	facq_chanlist_to_key_file(stmd->chanlist,file,group);
}

/**
 * facq_source_comedi_async_key_constructor:
 * @group_name: A string with the group name.
 * @key_file: A #GKeyFile object.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * It's purpose it's to create a new #FacqSourceComediAsync object from a
 * #GKeyFile and a group name. This function is used by #FacqCatalog. See
 * #CIKeyConstructor for more details.
 *
 * Returns: %NULL in case of error, or a new #FacqSourceComediAsync object if
 * successful.
 */
gpointer facq_source_comedi_async_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err)
{
	GError *local_err = NULL;
	guint index = 0, subindex = 0, flags = 0;
	gdouble period = 0;
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
	
	flags = (guint) g_key_file_get_double(key_file,group_name,"flags",&local_err);
	if(local_err)
		goto error;

	chanlist = facq_chanlist_from_key_file(key_file,group_name,&local_err);
	if(local_err)
		goto error;

	return facq_source_comedi_async_new(index,subindex,flags,chanlist,period,err);

	error:
        if(local_err){
                if(err)
                        g_propagate_error(err,local_err);
        }
        return NULL;
}

/**
 * facq_source_comedi_async_constructor:
 * @user_input: A #GPtrArray with the parameters from the user.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqSourceComediAsync object from a #GPtrArray, @user_input,
 * with at least five pointers, the first is a pointer to the device index, the
 * second is a pointer to the subdevice index, the third is a pointer to the
 * flags, the forth a pointer to the period, and the fifth a pointer to a
 * #FacqChanlist object.
 *
 * Returns: A new #FacqSourceComediAsync object or %NULL in case of error.
 */
gpointer facq_source_comedi_async_constructor(const GPtrArray *user_input,GError **err)
{
	guint *index = NULL, *subindex = NULL, *flags = NULL;
	gdouble *period = NULL;
	FacqChanlist *chanlist = NULL;

	g_return_val_if_fail(user_input->len == 5,NULL);

	index = g_ptr_array_index(user_input,0);
	subindex = g_ptr_array_index(user_input,1);
	flags = g_ptr_array_index(user_input,2);
	period = g_ptr_array_index(user_input,3);

	chanlist = g_ptr_array_index(user_input,4);
	g_object_ref(chanlist);

	return facq_source_comedi_async_new(*index,*subindex,*flags,chanlist,*period,err);
}


/**
 * facq_source_comedi_async_new:
 * @index: The device index, starting at 0.
 * @subindex: The subdevice index, starting at 0.
 * @flags: The <ulink
 * url="http://comedi.org/doc/commandsstreaming.html#comedicmdflags">command flags.</ulink>
 * @chanlist: A #FacqChanlist object.
 * @period: The period in seconds.
 * @err: A #GError it will be set in case of error, if not %NULL.
 *
 * Creates a new #FacqSourceComediAsync data source.
 *
 * Returns: A new #FacqSourceComediAsync data source, or %NULL in case of error.
 */ 
FacqSourceComediAsync *facq_source_comedi_async_new(guint index,guint subindex,guint flags,FacqChanlist *chanlist,gdouble period,GError **err)
{
	FacqStreamData *stmd = NULL;
	FacqSourceComediAsync *source = NULL;
	FacqUnits *units = NULL;
	gdouble *max = NULL;
	gdouble *min = NULL;
	gchar *devfilename = NULL;
	comedi_t *dev = NULL;
	GError *local_err = NULL;
	guint subd_flags = 0, iochans_n = 0;
	gint ret = 0, bps = 0, period_ns = 0;
	comedi_polynomial_t *p = NULL;

	//test period > 0
	if(period == 0){
		if(err != NULL)
			g_set_error_literal(err,FACQ_SOURCE_COMEDI_ASYNC_ERROR,
					FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,"Invalid period value");
		return NULL;		
	}

	period_ns = period * 1e9;

	//Open the device, async devices have a _subd suffix
	devfilename = g_strdup_printf("/dev/comedi%u_subd%u",index,subindex);
	dev = comedi_open(devfilename);
	if(!dev){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_ASYNC_ERROR,
			FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		goto error;
	}
	g_free(devfilename);
	devfilename = NULL;

	//test subdevice flags, SDF_CMD and SDF_READABLE.
	subd_flags = comedi_get_subdevice_flags(dev,
						subindex);
	if(subd_flags < 0){
		g_set_error_literal(&local_err,
			FACQ_SOURCE_COMEDI_ASYNC_ERROR,
				FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
		goto error;
	}
	if(!(subd_flags & SDF_CMD) || !(subd_flags & SDF_READABLE)){
		g_set_error_literal(&local_err,
			FACQ_SOURCE_COMEDI_ASYNC_ERROR,
				FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
					"This subdevice isn't supported");
		goto error;
	}

	//TODO: test flags
	//facq_comedi_misc_test_flags(dev,subindex,flags,&local_err);

	//test if the device can be calibrated, if so test if it's calibrated
	ret = facq_comedi_misc_can_calibrate(dev,subindex,&local_err);
	if(ret < 0)
		goto error;
	if(ret > 0)
		if(!facq_comedi_misc_test_calibrated(dev,&local_err)){
			if(local_err)
				goto error;
			else {
				if(ret == 1)
				g_set_error_literal(&local_err,
					FACQ_SOURCE_COMEDI_ASYNC_ERROR,
						FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
							NEED_SOFT_CALIBRATION);
				if(ret == 2)
				g_set_error_literal(&local_err,
					FACQ_SOURCE_COMEDI_ASYNC_ERROR,
						FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
							NEED_HARD_CALIBRATION);
				goto error;
			}
		}
		
	//test the chanlist
	iochans_n = facq_chanlist_get_io_chans_n(chanlist);
	if(!facq_comedi_misc_test_chanlist(dev,subindex,
					   chanlist,
					  &local_err))
		goto error;
	//test the period again
	if(!facq_comedi_misc_test_period(dev,subindex,
				     	 iochans_n,
				     	 period_ns,
				     	&local_err))
		goto error;
	
	//if the device needs calibration get polynomials for conversion
	if(ret > 0){
		p = facq_comedi_misc_get_polynomial(dev,subindex,
						    chanlist,
						    &local_err);
		if(!p)
			goto error;
	}

	//ensure that we don't get nan on conversion
	comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);

	/*---- create stream data object ----*/
	//create units
	units = facq_comedi_misc_get_units(dev,subindex,chanlist,&local_err);
	if(!units)
		goto error;
	//create max
	max = facq_comedi_misc_get_max(dev,subindex,chanlist,&local_err);
	if(!max)
		goto error;
	//create min
	min = facq_comedi_misc_get_min(dev,subindex,chanlist,&local_err);
	if(!min)
		goto error;

	//get bps
	bps = facq_comedi_misc_get_bps(dev,subindex,&local_err);
	if(bps == 0)
		goto error;

	stmd = facq_stream_data_new(bps,iochans_n,period,chanlist,units,max,min);
	source = FACQ_SOURCE_COMEDI_ASYNC(g_initable_new(FACQ_TYPE_SOURCE_COMEDI_ASYNC,NULL,err,
					       "name",
					       facq_resources_names_source_comedi_async(), 
					       "description",
					       facq_resources_descs_source_comedi_async(),
					       "stream-data",stmd,
					       "subindex",subindex,
					       "flags",flags,
					       "poly",p,
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
 * facq_source_comedi_async_start:
 * @src: A #FacqSourceComediAsync source in the form of a #FacqSource.
 * @err: A #GError, it will be set in case of error, if not %NULL.
 *
 * Starts the acquisition sending the command to the device.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_source_comedi_async_start(FacqSource *src,GError **err)
{
	FacqSourceComediAsync *source = FACQ_SOURCE_COMEDI_ASYNC(src);
	GError *local_err = NULL;

	if( comedi_lock(source->priv->dev,source->priv->subindex) < 0){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_ASYNC_ERROR,
			FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		goto error;
	}

	if( comedi_command(source->priv->dev,source->priv->cmd) != 0){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_ASYNC_ERROR,
			FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
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
 * facq_source_comedi_async_poll:
 * @src: A #FacqSourceComediAsync but in the form of #FacqSource.
 *
 * Polls the comedi device for new data.
 *
 * Returns: -1 in case of error, 1 if new data is available, 0 if there isn't
 * any new data to be read.
 */
gint facq_source_comedi_async_poll(FacqSource *src)
{
	FacqSourceComediAsync *source = NULL;
	gint ret = 0;

	g_return_val_if_fail(FACQ_IS_SOURCE_COMEDI_ASYNC(src),-1);

	source = FACQ_SOURCE_COMEDI_ASYNC(src);

	if(source->priv->can_poll){
		ret = g_poll(source->priv->pfd,1,1000);
		if(ret){
			if(source->priv->pfd->revents & G_IO_ERR)
				return -1;
			if(source->priv->pfd->revents & G_IO_IN)
				return 1;
		}
		return 0; //timeout
	}

	return 1;
}

/**
 * facq_source_comedi_async_read:
 * @src: A #FacqSourceComediAsync source in the form of a #FacqSource.
 * @buf: A pointer to the memory area where the data will be stored.
 * @count: The size of @buf in bytes.
 * @bytes_read: Pointer to a variable that will store the number of bytes read.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Reads the data from the comedi DAQ device, putting at most 
 * @count bytes in the data area pointer in @buf.
 * The function, if successful also puts the number of bytes read in 
 * @bytes_read.
 *
 * Returns: A #GIOStatus according to the status of the read, it will be
 * %G_IO_STATUS_NORMAL if successful or %G_IO_STATUS_ERROR in case of error.
 */
GIOStatus facq_source_comedi_async_read(FacqSource *src,gchar *buf,gsize count,gsize *bytes_read,GError **err)
{
	GError *local_err = NULL;
	FacqSourceComediAsync *source = NULL;
	GIOStatus ret = 0;

#if ENABLE_DEBUG
	g_return_val_if_fail(FACQ_IS_SOURCE_COMEDI_ASYNC(src),G_IO_STATUS_ERROR);
#endif
	source = FACQ_SOURCE_COMEDI_ASYNC(src);

	ret = g_io_channel_read_chars(source->priv->channel,
				     buf,
				     count,
				     bytes_read,
				     &local_err);
		if(local_err)
			goto error;
	
	return ret;

	error:
	if(local_err){
		g_propagate_error(err,local_err);
	}
	return G_IO_STATUS_ERROR;
}

/**
 * facq_source_comedi_async_conv:
 * @src: A #FacqSourceComediAsync source in the form of a #FacqSource.
 * @ori: A pointer to a zone of memory that contains an unconverted chunk.
 * @dst: A pointer to the zone of memory where the function will store the
 * result.
 * @samples: The number of samples to convert.
 *
 * Gets a chunk from the #FacqPipeline in @ori, and converts it to real data
 * putting it in @dst.
 */ 
void facq_source_comedi_async_conv(FacqSource *src,gpointer ori,gdouble *dst,gsize samples)
{
	FacqSourceComediAsync *source = FACQ_SOURCE_COMEDI_ASYNC(src);
	const FacqStreamData *stmd = NULL;
	guint bps = 0, n_channels = 0, i = 0, j = 0;
	lsampl_t sample = 0;
	gsize slices = 0;
	gpointer slice = NULL;
	gdouble *row = NULL;

	stmd = facq_source_get_stream_data(src);
	bps = facq_stream_data_get_bps(stmd);
	n_channels = facq_stream_data_get_n_channels(stmd);
	slices = samples/n_channels;

	for(j = 0;j < slices;j++){
		slice = ori+(j*bps*n_channels);
		row = dst+(j*n_channels);
		for(i = 0; i < n_channels;i++){
			g_memmove(&sample,slice+(i*bps),bps);
			//The device is soft/hard calibrated
			if(source->priv->p){
				row[i] = comedi_to_physical(sample,
						&source->priv->p[i]);
			}
			//The device can't be calibrated
			else {
				row[i] = comedi_to_phys(sample,
						&source->priv->rng[i],
							source->priv->maxdata[i]);
			}
		}
	}
}

/**
 * facq_source_comedi_async_stop:
 * @src: A #FacqSourceComediAsync source in the form of a #FacqSource.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Stops the data acquisition and unlocks the subdevice.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_source_comedi_async_stop(FacqSource *src,GError **err)
{
	FacqSourceComediAsync *source = FACQ_SOURCE_COMEDI_ASYNC(src);
	GError *local_err = NULL;
	guint subd_flags = 0;

	subd_flags = comedi_get_subdevice_flags(source->priv->dev,
						source->priv->subindex);
	if(subd_flags < 0){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_ASYNC_ERROR,
			FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		goto error;
	}

	if(subd_flags & SDF_RUNNING)
		if( comedi_cancel(source->priv->dev,source->priv->subindex) < 0){
			g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_ASYNC_ERROR,
				FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
			goto error;
		}

	if( comedi_unlock(source->priv->dev,source->priv->subindex) < 0){
		g_set_error_literal(&local_err,FACQ_SOURCE_COMEDI_ASYNC_ERROR,
			FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED,
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
 * facq_source_comedi_async_free:
 * @src: A #FacqSourceComediAsync object but in the form of #FacqSource.
 *
 * Destroys the #FacqSourceComediAsync, @src, object, freeing all the associated
 * resources.
 */
void facq_source_comedi_async_free(FacqSource *src)
{
	g_return_if_fail(FACQ_IS_SOURCE_COMEDI_ASYNC(src));
	g_object_unref(G_OBJECT(src));
}
#endif //USE_COMEDI

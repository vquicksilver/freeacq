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
#include <string.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqsource.h"

/**
 * SECTION:facqsource
 * @include: facqsource.h
 * @short_description: parent class for data sources
 * @see_also: #FacqStreamData,#FacqStream,#GIOChannel
 *
 * A #FacqSource is an abstract class that defines the behavior of
 * any data source that can be used. If you want to add a new data 
 * source to the system you must create a new class inheriting it from
 * this one, and override the virtual methods listed on #FacqSourceClass.
 *
 * <sect1 id="source-example">
 * <title>How to implement a new source, example</title>
 * <para>
 * In the following subsections, a source example is showed, you can use it
 * as a reference for implementing your own source.
 * </para>
 * <sect2 id="source-example-h">
 * <title>A source example: facqsourcedumb.h</title>
 * <para>
 * Just for reference here appears a dumb source implementation. More details after the example.
 * </para>
 * <para>
 * <informalexample><programlisting>
 * #ifndef _FREEACQ_SOURCE_DUMB_H
 * #define _FREEACQ_SOURCE_DUMB_H
 * 
 * G_BEGIN_DECLS
 * 
 * #define FACQ_SOURCE_DUMB_ERROR facq_source_dumb_error()
 *
 * #define FACQ_TYPE_SOURCE_DUMB (facq_source_dumb_get_type())
 * #define FACQ_SOURCE_DUMB(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_SOURCE_DUMB,FacqSourceDumb))
 * #define FACQ_SOURCE_DUMB_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_SOURCE_DUMB, FacqSourceDumbClass))
 * #define FACQ_IS_SOURCE_DUMB(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_SOURCE_DUMB))
 * #define FACQ_IS_SOURCE_DUMB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_SOURCE_DUMB))
 * #define FACQ_SOURCE_DUMB_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_SOURCE_DUMB, FacqSourceDumbClass))
 *
 * 
 * typedef enum {
 *        FACQ_SOURCE_DUMB_ERROR_FAILED
 * } FacqSourceDumbError;
 *
 * typedef struct _FacqSourceDumb FacqSourceDumb;
 * typedef struct _FacqSourceDumbClass FacqSourceDumbClass;
 * typedef struct _FacqSourceDumbPrivate FacqSourceDumbPrivate;
 * struct _FacqSourceDumb {
 * 	FacqSource parent_instance;
 * 	FacqSourceDumbPrivate *priv;
 * };
 * struct _FacqSourceDumbClass {
 * 	FacqSourceClass parent_class;
 * };
 *
 * GType facq_source_dumb_get_type(void) G_GNUC_CONST;
 *
 * FacqSourceDumb *facq_source_dumb_new(...,GError **error);
 * gboolean facq_source_dumb_start(FacqSource *src,GError **err);
 * gint facq_source_dumb_poll(FacqSource *src);
 * GIOStatus facq_source_dumb_read(FacqSource *src,gpointer chunk,gsize chunk_size,GError **err);
 * void facq_source_dumb_conv(FacqSource *src,gpointer ori,gpointer dst,guint dst_size);
 * gboolean facq_source_dumb_stop(FacqSource *src,GError **err);
 * void facq_source_dumb_free(FacqSource *src);
 *
 * G_END_DECLS
 *
 * #endif
 * </programlisting></informalexample>
 * </para>
 * </sect2>
 * <sect2 id="source-example-c">
 * <title>A source example: facqsourcedumb.c</title>
 * <para>
 * Here appears the implementation of the previous source.
 * </para>
 * <para>
 * <informalexample><programlisting>
 * #include <glib.h>
 * #include <gio/gio.h>
 * #include "facqunits.h"
 * #include "facqchanlist.h"
 * #include "facqstreamdata.h"
 * #include "facqsource.h"
 * #include "facqsourcedumb.h"
 *
 * static void facq_source_dumb_initable_iface_init(GInitableIface *iface);
 * static gboolean facq_source_dumb_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);
 * G_DEFINE_TYPE_WITH_CODE(FacqSourceDumb,facq_source_dumb,FACQ_TYPE_SOURCE,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_source_dumb_initable_iface_init));
 *
 * #define FACQ_SOURCE_NAME "Dumb source"
 * #define FACQ_SOURCE_DESC "A dumb (Does nothing) source"
 *
 * enum {
 * 	FACQ_SRC_NAME,
 * 	FACQ_SRC_DESC
 * }
 *
 * static const gchar * const src_desc[] =
 * {
 * 	FACQ_SOURCE_NAME,
 * 	FACQ_SOURCE_DESC
 * }
 *
 * enum {
 * 	PROP_0,
 * 	PROP_A,
 * 	PROP_B
 * }
 *
 * struct _FacqSourceDumbPrivate {
 * 	guint a;
 * 	gpointer b;
 * 	GError *construct_error;
 * };
 *
 * GQuark facq_source_dumb_error_quark(void)
 * {
 * 	return g_quark_from_static_string("facq-source-dumb-error-quark");
 * }
 * 
 * static void facq_source_dumb_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *spec)
 * {
 * 	FacqSourceDumb *dumbsrc = FACQ_SOURCE_DUMB(self);
 *
 * 	switch(property_id){
 * 	case PROP_A:
 * 	break;
 * 	case PROP_B:
 * 	break;
 * 	default:
 * 	G_OBJECT_WARN_INVALID_PROPERTY_ID(dumbsrc,property_id,pspec);
 * 	}
 * }
 *
 * static void facq_source_dumb_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *spec)
 * {
 * 	FacqSourceDumb *dumbsrc = FACQ_SOURCE_DUMB(self);
 *
 * 	switch(property_id){
 * 	case PROP_A:
 * 	break;
 * 	case PROP_B:
 * 	break;
 * 	default:
 * 	G_OBJECT_WARN_INVALID_PROPERTY_ID(dumbsrc,property_id,pspec);
 * 	}
 * }
 *
 * static void facq_source_dumb_constructed(GObject *self)
 * {
 * 	FacqSourceDumb *dumbsrc = FACQ_SOURCE_DUMB(self);
 * }
 *
 * static void facq_source_dumb_finalize(GObject *self)
 * {
 * 	FacqSourceDumb *dumbsrc = FACQ_SOURCE_DUMB(self);
 * 	
 * 	if (G_OBJECT_CLASS (facq_source_dumb_parent_class)->finalize)
 *             (*G_OBJECT_CLASS (facq_source_dumb_parent_class)->finalize) (self);
 * }
 *
 * static void facq_source_dumb_class_init(FacqSourceDumbClass *klass)
 * {
 * 	GObjectClass *object_class = G_OBJECT_CLASS(klass);
 * 	FacqSourceClass *source_class = FACQ_SOURCE_CLASS(klass);
 *
 * 	g_type_class_add_private(klass,sizeof(FacqSourceDumbPrivate));
 *
 * 	object_class->set_property = facq_source_dumb_set_property;
 * 	object_class->get_property = facq_source_dumb_get_property;
 * 	object_class->constructed = facq_source_dumb_constructed;
 * 	object_class->finalize = facq_source_dumb_finalize;
 *
 * 	//override source class virtual methods
 * 	source_class->srcstart = facq_source_dumb_start; //or NULL
 * 	source_class->srcpoll = facq_source_dumb_poll; //or NULL
 * 	source_class->srcread = facq_source_dumb_read;
 * 	source_class->srcconv = facq_source_dumb_conv; //or NULL
 * 	source_class->srcstop = facq_source_dumb_stop; //or NULL
 *
 * 	g_object_class_install_property(object_class,PROP_A,
 * 					g_param_spec_*(...
 * 							));
 * 	g_object_class_install_property(object_class,PROP_B,
 * 					g_param_spec_*(...
 * 							));
 *
 * }
 *
 * static void facq_source_dumb_init(FacqSourceDumb *dumbsrc)
 * {
 * 	dumbsrc->priv = G_TYPE_INSTANCE_GET_PRIVATE(dumbsrc,FACQ_TYPE_SOURCE_DUMB,FacqSourceDumbPrivate);
 *
 * }
 *
 * static void facq_source_rand_initable_iface_init(GInitableIface *iface)
 * {
 * 	iface->init = facq_source_rand_initable_init;
 * }
 *
 * static gboolean facq_source_rand_initable_init(GInitable *initable,GCancellable *cancellable,GError  **error)
 * {
 * 	FacqSourceDumb *dumbsource = NULL;
 *
 * 	g_return_val_if_fail(FACQ_IS_SOURCE_DUMB(initable),FALSE);
 * 	dumbsource  = FACQ_SOURCE_DUMB(initable);
 * 	if(cancellable != NULL){
 * 		g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
 * 			"Cancellable initialization not supported");
 * 		return FALSE;
 * 	}
 * 	if(source->priv->construct_error){
 * 		if (error)
 * 			*error = g_error_copy(source->priv->construct_error);
 * 		return FALSE;
 * 	}
 * 	return TRUE;
 * }
 *
 * FacqSourceDumb *facq_source_dumb_new(...,GError **err)
 * {
 * 	FacqStreamData *stmd = NULL;
 * 	FacqSourceDumb *dumbsrc = NULL;
 * 	FacqChanlist *chanlist = NULL;
 * 	FacqUnits *units = NULL;
 * 	gdouble *max = NULL;
 * 	gdouble *min = NULL;
 *
 * 	...
 *
 * 	stmd = facq_stream_data_new(8,n_channels,period,chanlist,units,max,min);
 * 	dumbsrc = FACQ_SOURCE_DUMB(g_initable_new(FACQ_TYPE_SOURCE_DUMB,NULL,err,
 * 							"name",src_desc[FACQ_SRC_NAME],
 *							"description",src_desc[FACQ_SRC_DESC],
 *							"stream-data",stmd,
 *							"...",...,
 *							NULL));
 *
 * 	if(!dumbsrc){
 * 		facq_stream_data_free(stmd);
 * 	}
 *
 * 	return dumbsrc;
 * }
 *
 * gint facq_source_dumb_poll(FacqSource *src)
 * {
 * 	gint ret = -1;
 * 	FacqSourceDumb *dumbsrc = NULL;
 *
 * 	g_return_val_if_fail(FACQ_IS_SOURCE_DUMB(src),-1);
 *
 * 	dumbsrc = FACQ_SOURCE_DUMB(src);
 *
 *  	//Let's imagine that we have a GPollFD ready to be polled
 * 	ret = g_poll(dumbsrc->priv->pollfd,1,200);
 * 	if(ret){
 * 		if(dumbsrc->priv->pfd->revents & G_IO_IN)
 * 			return 1;
 * 		if(dumbsrc->priv->pfd->revents & G_IO_ERR)
 * 			return -1;
 * 	}
 * 	return ret;
 * }
 *
 * GIOStatus facq_source_dumb_read(FacqSource *src,gpointer chunk,gsize chunk_size,GError **err)
 * {
 * 	gsize bytes_read = 0;
 * 	GIOStatus status = 0;
 * 	GError *local_err = NULL;
 * 	FacqSourceDumb *dumbsrc = NULL;
 *
 * 	g_return_val_if_fail(FACQ_IS_SOURCE_RAND(src),G_IO_STATUS_ERROR);
 * 	dumbsrc = FACQ_SOURCE_DUMB(src);
 *      
 * 	//Let's imaginee that we have a GIOChannel ready to be read
 * 	status = g_io_channel_read_chars(dumbsrc->priv->channel,
 * 			(gchar *)chunk,chunk_size,&bytes_read,&local_err);
 *
 * 	if(local_err){
 * 		g_propagate_error(err,local_err);
 * 	}
 *
 * 	return status;
 * }
 *
 * void facq_source_dumb_conv(FacqSource *src,gpointer ori,gpointer dst,guint dst_size)
 * {
 * 	//do your conversion magic
 *
 * 	//put the result on dst
 * 	g_memmove(dst,ori,dst_size);
 * }
 *
 * void facq_source_dumb_free(FacqSource *src)
 * {
 * 	g_return_if_fail(FACQ_IS_SOURCE_DUMB(src));
 * 	g_object_unref(G_OBJECT(src));
 * }
 * </programlisting></informalexample>
 * </para>
 * </sect2>
 * </sect1>
 */

/**
 * FacqSource:
 *
 * Contains all the #FacqSource private data.
 */

/**
 * FacqSourceClass:
 * @srcsave: Virtual method that is called when the stream is saved to
 * a #GKeyFile, when the facq_stream_save() function is called.
 * You must use the #GKeyFile functions to store your source relevant
 * attributes to the #GKeyFile object.
 * @srcstart: Virtual method that is called when the stream is started.
 * In this method you must prepare the source for generating data, making
 * all the required initialization. Be aware that a stream can be started
 * and stopped multiple times, so this function maybe needs to reset the
 * state of the source, depending on the kind of source. Return %TRUE if
 * the initialization is successful %FALSE in any other case.
 * Implementing this method is optional, if you don't provide it an 
 * empty method that returns %TRUE will be used.
 * @srcpoll: Virtual method that gets called before the source is read.
 * In this method you are supposed to use g_poll() or similar function to
 * check for new data without blocking (You can wait for data a short amount
 * of time). You must return 0 in case of timeout, -1 in case of error and 1
 * if the source can be read. Implementing this method is optional, if you
 * don't provide it and empty method that returns 1 will be used.
 * @srcread: Virtual method that is called whenever @srcpoll returns 1. This
 * method is not optional you must provide it. You must put the read data
 * in the memory area and set the number of bytes read. 
 * See #GIOChannel for return values and for generic read/write
 * functions to any kind of file descriptor.
 * @srcconv: Virtual method that is called after reading data from the source.
 * It's optional to provide it, if your source gives data in form of double 
 * precision reals, else you will have to provide it, and do the conversion
 * of the read samples to #gdouble.
 * @srcstop: Virtual method that is called when the stream is stopped. 
 * You must return %TRUE if successful or %FALSE in any other case.
 * It's optional implementing it, if you don't provide it an empty 
 * method that returns %TRUE will be used.
 * @srcfree: Virtual method that is called when the source is no longer needed.
 * You must provide it. In most cases calling g_object_unref() should be enough.
 */

G_DEFINE_TYPE(FacqSource,facq_source,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_NAME,
	PROP_DESC,
	PROP_STARTED,
	PROP_STREAM_DATA
};

struct _FacqSourcePrivate {
	/* Properties */
	gchar *name;
	gchar *desc;
	gboolean started;
	FacqStreamData *stmd;
};

/*****--- GObject magic ---*****/
static void facq_source_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqSource *src = FACQ_SOURCE(self);

	switch(property_id){
	case PROP_NAME: src->priv->name = g_value_dup_string(value);
	break;
	case PROP_DESC: src->priv->desc = g_value_dup_string(value);
	break;
	case PROP_STARTED: src->priv->started = g_value_get_boolean(value);
	break;
	case PROP_STREAM_DATA: src->priv->stmd = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(src,property_id,pspec);
	}
}

static void facq_source_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqSource *src = FACQ_SOURCE(self);
	
	switch(property_id){
	case PROP_NAME: g_value_set_string(value,src->priv->name);
	break;
	case PROP_DESC: g_value_set_string(value,src->priv->desc);
	break;
	case PROP_STARTED: g_value_set_boolean(value,src->priv->started);
	break;
	case PROP_STREAM_DATA: g_value_set_pointer(value,src->priv->stmd);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(src,property_id,pspec);
	}
}

static void facq_source_finalize(GObject *self)
{
	FacqSource *src = FACQ_SOURCE(self);

	if(src->priv->name)
		g_free(src->priv->name);
	if(src->priv->desc)
		g_free(src->priv->desc);
	if(src->priv->stmd)
		facq_stream_data_free(src->priv->stmd);
	
	G_OBJECT_CLASS (facq_source_parent_class)->finalize (self);
}

static void facq_source_class_init(FacqSourceClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	FacqSourceClass *source_class = FACQ_SOURCE_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FacqSourcePrivate));

	object_class->set_property = facq_source_set_property;
	object_class->get_property = facq_source_get_property;
	object_class->finalize = facq_source_finalize;
	source_class->srcsave = NULL;
	source_class->srcstart = NULL;
	source_class->srcpoll = NULL;
	source_class->srcread = NULL;
	source_class->srcstop = NULL;
	source_class->srcfree = NULL;
	source_class->srcconv = NULL;
	
	/* properties */

	/**
	 * FacqSource:name:
	 *
	 * The name of the source, for example "DAQ source".
	 */
	g_object_class_install_property(object_class,PROP_NAME,
					g_param_spec_string("name",
							    "The source name",
							    "The name of the source",
							    "Unknown",
							    G_PARAM_READWRITE | 
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));

	/**
	 * FacqSource:description:
	 *
	 * A detailed source description, for example: "Data adquisition
	 * source".
	 */
	g_object_class_install_property(object_class,PROP_DESC,
					g_param_spec_string("description",
							    "The source description",
							    "The detailed description of the source",
							    "Unknown",
							    G_PARAM_READWRITE | 
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));
	
	/**
	 * FacqSource:started:
	 *
	 * Describes the current status of the source, if the source has been
	 * started it will be %TRUE, else it will be %FALSE.
	 */
	g_object_class_install_property(object_class,PROP_STARTED,
					g_param_spec_boolean("started",
							     "The source status",
							     "The status of the source",
							     FALSE,
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));

	/**
	 * FacqSource:stream-data:
	 *
	 * A #FacqStreamData contained object, that contains details 
	 * about the data that is going to be generated by the source.
	 * See #FacqStreamData for more details.
	 */
	g_object_class_install_property(object_class,PROP_STREAM_DATA,
					g_param_spec_pointer("stream-data",
							     "Stream data",
							     "Stream properties",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));
	
}

static void facq_source_init(FacqSource *src)
{
	src->priv = G_TYPE_INSTANCE_GET_PRIVATE(src,FACQ_TYPE_SOURCE,FacqSourcePrivate);

	src->priv->name = NULL;
	src->priv->desc = NULL;
	src->priv->started = FALSE;
	src->priv->stmd = NULL;
}

/* Public methods */
/**
 * facq_source_get_name:
 * @src: A #FacqSource object, it can be any type of source.
 * 
 * Returns: the name of the source.
 */
const gchar *facq_source_get_name(const FacqSource *src)
{
	g_return_val_if_fail(FACQ_IS_SOURCE(src),NULL);
	return src->priv->name;
}

/**
 * facq_source_get_description:
 * @src: A #FacqSource object, it can be any type of source.
 *
 * Returns: the description of the source.
 */
const gchar *facq_source_get_description(const FacqSource *src)
{
	g_return_val_if_fail(FACQ_IS_SOURCE(src),NULL);
	return src->priv->desc;
}

/**
 * facq_source_get_started:
 * @src: A #FacqSource object, it can be any type of source.
 *
 * Returns the current status of the source object.
 *
 * Returns: %TRUE if started, %FALSE in other case.
 */
gboolean facq_source_get_started(const FacqSource *src)
{
	g_return_val_if_fail(FACQ_IS_SOURCE(src),FALSE);
	return src->priv->started;
}

/**
 * facq_source_get_stream_data:
 * @src: A #FacqSource object, it can be any type of source.
 *
 * Gets the #FacqStreamData attached to the source.
 *
 * Returns: A read only #FacqStreamData object, that you can query
 * with the <function>facq_stream_data_*</function> functions.
 */
const FacqStreamData *facq_source_get_stream_data(const FacqSource *src)
{
	g_return_val_if_fail(FACQ_IS_SOURCE(src),NULL);
	return src->priv->stmd;
}

/**
 * facq_source_needs_conv:
 * @src: A #FacqSource object, it can be any type of source.
 *
 * Checks if the source needs to do a data conversion to real after
 * reading the data.
 *
 * Returns: %TRUE if the data needs to be converted, %FALSE in other case.
 */
gboolean facq_source_needs_conv(FacqSource *src)
{
	g_return_val_if_fail(FACQ_IS_SOURCE(src),FALSE);

	if(FACQ_SOURCE_GET_CLASS(src)->srcconv)
		return TRUE;

	return FALSE;
}

/* Virtuals */
/**
 * facq_source_to_file:
 * @src: A #FacqSource object, it can be any type of source.
 * @file: A #GKeyFile object.
 * @group: The group name for the #GKeyFile, @file.
 *
 * Calls the facq_source_*_to_file() function related to the source type,
 * allowing the source to store important attributes in a #GKeyFile.
 * This functions is used by facq_stream_save().
 */
void facq_source_to_file(FacqSource *src,GKeyFile *file,const gchar *group)
{
	g_return_if_fail(FACQ_IS_SOURCE(src));
	g_return_if_fail(file);
	g_return_if_fail(g_key_file_has_group(file,group));

	if(FACQ_SOURCE_GET_CLASS(src)->srcsave)
		FACQ_SOURCE_GET_CLASS(src)->srcsave(src,file,group);
}

/**
 * facq_source_start:
 * @src: A #FacqSource object, it can be any type of source.
 * @err: #GError for error reporting or %NULL to ignore.
 *
 * This function calls the facq_source_*_start function related to
 * the source type. See #FacqSourceClass for more details.
 * If the start is successful the started property of the source will be set to
 * %TRUE. If the source is already started the function will do nothing and will
 * return %TRUE.
 *
 * Returns: %TRUE if successful or if the source was already started previously, 
 * %FALSE in other case.
 */
gboolean facq_source_start(FacqSource *src,GError **err)
{
	gboolean ret = TRUE;

	g_return_val_if_fail(FACQ_IS_SOURCE(src),FALSE);
	
	if(!src->priv->started){
		if(FACQ_SOURCE_GET_CLASS(src)->srcstart)
			ret = FACQ_SOURCE_GET_CLASS(src)->srcstart(src,err);
		else
			ret = TRUE;

		src->priv->started = TRUE;
	}

	return ret;
}

/**
 * facq_source_poll:
 * @src: A #FacqSource object, it can be any type of source.
 *
 * This function calls the facq_source_*_poll function related to 
 * the source type. See #FacqSourceClass for more details.
 *
 * Returns: 1 if there is data to read, 0 on timeout, -1 on error.
 */
gint facq_source_poll(FacqSource *src)
{
#if ENABLE_DEBUG
	g_return_val_if_fail(FACQ_IS_SOURCE(src),-1);
#endif
	if(FACQ_SOURCE_GET_CLASS(src)->srcpoll)
		return FACQ_SOURCE_GET_CLASS(src)->srcpoll(src);
	else
		return 1;
}

/**
 * facq_source_read:
 * @src: A #FacqSource object,it can be any type of source.
 * @buf: A pointer to a free memory area.
 * @count: The size of the memory area in bytes.
 * @bytes_read: (out caller-allocates): It will be set to the number of
 * bytes read.
 * @err: (allow-none): a #GError for error reporting or %NULL to ignore.
 *
 * This function calls the facq_source_*_read function related to
 * the source type. See #FacqSourceClass for more details.
 *
 * Returns: a #GIOStatus according to the operation result.
 */
GIOStatus facq_source_read(FacqSource *src,gchar *buf,gsize count,gsize *bytes_read,GError **err)
{
#if ENABLE_DEBUG
	g_return_val_if_fail(FACQ_IS_SOURCE(src),-1);
#endif
	return FACQ_SOURCE_GET_CLASS(src)->srcread(src,buf,count,bytes_read,err);
}

/**
 * facq_source_conv:
 * @src: A #FacqSource object, it can be any type of source.
 * @ori: A pointer to a memory area with the original data as read by the source.
 * @dst: A pointer to the memory area where you should put the resulting
 * data of the conversion.
 * @samples: Number of samples in @ori or @dst, it's the same number.
 *
 * Takes the original readed samples on facq_source_read and converts it
 * to double precision reals (IEEE754). If you are reading double 
 * precision reals you don't need to provide this function, else you
 * will have to provide it. 
 */
void facq_source_conv(FacqSource *src,gpointer ori,gdouble *dst,gsize samples)
{
#if ENABLE_DEBUG
	g_return_if_fail(FACQ_IS_SOURCE(src));
#endif
	if( (FACQ_SOURCE_GET_CLASS(src)->srcconv) )
		return FACQ_SOURCE_GET_CLASS(src)->srcconv(src,ori,dst,samples);
}

/**
 * facq_source_stop:
 * @src: A #FacqSource object, it can be any type of source.
 * @err: #GError for error reporting or %NULL to ignore.
 *
 * This function calls the facq_source_*_stop function related to
 * the source type. See #FacqSourceClass for more details.
 *
 * Returns: %TRUE if ok %FALSE in other case. Also if the source is already
 * stopped the return value will be %TRUE.
 */
gboolean facq_source_stop(FacqSource *src,GError **err)
{
	gboolean ret = TRUE;

	g_return_val_if_fail(FACQ_IS_SOURCE(src),FALSE);

	if(src->priv->started){
		if(FACQ_SOURCE_GET_CLASS(src)->srcstop)
			ret = FACQ_SOURCE_GET_CLASS(src)->srcstop(src,err);
		else
			ret = TRUE;

		src->priv->started = FALSE;
	}

	return ret;
}

/**
 * facq_source_free:
 * @src: A #FacqSource object, it can be any type of source.
 *
 * Destroys the source, calling the destructor.
 */
void facq_source_free(FacqSource *src)
{
	g_return_if_fail(FACQ_IS_SOURCE(src));
	FACQ_SOURCE_GET_CLASS(src)->srcfree(src);
}


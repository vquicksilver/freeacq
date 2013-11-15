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
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqchunk.h"
#include "facqstreamdata.h"
#include "facqsink.h"

/**
 * SECTION:facqsink
 * @title:FacqSink
 * @short_description: Base class for the system's data sinks.
 * @include:facqsink.h
 *
 * #FacqSink provides a base class (Or abstract class) for the system's data sinks, 
 * like #FacqSource did for the data sources in the system.
 * All the sinks in the system must implement the virtuals in this class to be
 * able to work with the system, and inherit from this class, so hopefully it's
 * not to hard to add new sinks to the system.
 *
 * <sect1 id="sink-example">
 * <title>How to implement a sink</title>
 * <para>
 * In the following subsections, a sink example is showed, you can use it as a
 * reference for implementing your own sink.
 * </para>
 * <sect2 id="sink-example-h">
 * <title>A sink example: facqsinkdumb.h</title>
 * <para>
 * Just for reference purposes here appears a dumb sink implementation.
 * More details after the header.
 * </para>
 * <para><informalexample><programlisting>
 * This is the header code.
 * </programlisting></informalexample></para>
 * </sect2>
 * <sect2 id="sink-example-c">
 * <title>A sink example: facqsinkdumb.c</title>
 * <para><informalexample><programlisting>
 * This is the .c code.
 * </programlisting></informalexample></para>
 * </sect2>
 * </sect1>
 */

/**
 * FacqSink:
 *
 * Contains the private details of the #FacqSink.
 */

/**
 * FacqSinkClass:
 * @sinksave: Virtual method that is called when the stream is saved to
 * a #GKeyFile, when the facq_stream_save() function is called.
 * You must use the #GKeyFile functions to store your sink relevant
 * attributes to the #GKeyFile object. If you don't have any
 * relevant attributes to store, you don't need to implement this function.
 * @sinkstart: Virtual method that is called when the stream is started.
 * In this method you must prepare the sink for data consumption, making
 * all the required initialization. Be aware that a stream can be started
 * and stopped multiple times, so this function maybe needs to reset the
 * state of the sink, depending on the kind of sink. Return %TRUE if
 * the initialization is successful %FALSE in any other case.
 * Implementing this method is optional, if you don't provide it an 
 * empty method that returns %TRUE will be used.
 * @sinkpoll: Virtual method that gets called before the sink is written.
 * In this method you are supposed to use g_poll() or similar function to
 * check if you can write to the sink without blocking (You can wait for a short amount
 * of time). You must return 0 in case of timeout, -1 in case of error and 1
 * if the sink can be read. Implementing this method is optional, if you
 * don't provide it and empty method that returns 1 will be used.
 * @sinkwrite: Virtual method that is called whenever @srcpoll returns 1. This
 * method is not optional you must provide it. You must write the data in the
 * provided #FacqChunk to the sink, you don't need to free the #FacqChunk the
 * system will do it for you.
 * See #GIOChannel for return values and for generic read/write
 * functions to any kind of file descriptor.
 * @sinkstop: Virtual method that is called when the stream is stopped.
 * You must return %TRUE if successful or %FALSE in any other case.
 * It's optional implementing it, if you don't provide it an empty 
 * method that returns %TRUE will be used. Note that this method can be called
 * before the sink has received any sample.
 * @sinkfree: Virtual method that is called when the sink is no longer needed.
 * You must provide it. Calling g_object_unref() should be enough in most cases.
 */

G_DEFINE_TYPE(FacqSink,facq_sink,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_NAME,
	PROP_DESC,
	PROP_STARTED
};

struct _FacqSinkPrivate {
	/* Properties */
	gchar *name;
	gchar *desc;
	gboolean started;
};

/*****--- GObject magic ---*****/
static void facq_sink_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqSink *sink = FACQ_SINK(self);

	switch(property_id){
	case PROP_NAME: sink->priv->name = g_value_dup_string(value);
	break;
	case PROP_DESC: sink->priv->desc = g_value_dup_string(value);
	break;
	case PROP_STARTED: sink->priv->started = g_value_get_boolean(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(sink,property_id,pspec);
	}
}

static void facq_sink_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqSink *sink = FACQ_SINK(self);

	switch(property_id){
	case PROP_NAME: g_value_set_string(value,sink->priv->name);
	break;
	case PROP_DESC: g_value_set_string(value,sink->priv->desc);
	break;
	case PROP_STARTED: g_value_set_boolean(value,sink->priv->started);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(sink,property_id,pspec);
	}
}

static void facq_sink_finalize(GObject *self)
{
	FacqSink *sink = FACQ_SINK(self);
	
	if(sink->priv->name)
		g_free(sink->priv->name);
	if(sink->priv->desc)
		g_free(sink->priv->desc);

	if (G_OBJECT_CLASS (facq_sink_parent_class)->finalize)
    		(*G_OBJECT_CLASS (facq_sink_parent_class)->finalize) (self);
}

static void facq_sink_class_init(FacqSinkClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	FacqSinkClass *sink_class = FACQ_SINK_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FacqSinkPrivate));

	object_class->set_property = facq_sink_set_property;
	object_class->get_property = facq_sink_get_property;
	object_class->finalize = facq_sink_finalize;
	sink_class->sinksave = NULL;
	sink_class->sinkstart = NULL;
	sink_class->sinkpoll = NULL;
	sink_class->sinkwrite = NULL;
	sink_class->sinkstop = NULL;
	sink_class->sinkfree = NULL;

	/**
	 * FacqSink:name:
	 *
	 * The name of the sink, for example "DAQ sink".
	 */
	g_object_class_install_property(object_class,PROP_NAME,
					g_param_spec_string("name",
							    "The sink name",
							    "The name of the sink",
							    "Unknown",
							    G_PARAM_READWRITE | 
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));

	/**
	 * FacqSink:desc:
	 *
	 * The description of the sink, for example "Dumb sink".
	 */
	g_object_class_install_property(object_class,PROP_DESC,
					g_param_spec_string("description",
							    "The sink description",
							    "The detailed description of the sink",
							    "Unknown",
							    G_PARAM_READWRITE | 
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));

	/**
	 * FacqSink:started:
	 *
	 * Describes the current status of the sink, if the sink has been
	 * started it will be %TRUE, else it will be %FALSE.
	 */
	g_object_class_install_property(object_class,PROP_STARTED,
					g_param_spec_boolean("started",
							     "Started",
							     "The current sink status",
							     FALSE,
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));
}

static void facq_sink_init(FacqSink *sink)
{
	sink->priv = G_TYPE_INSTANCE_GET_PRIVATE(sink,FACQ_TYPE_SINK,FacqSinkPrivate);
	sink->priv->name = NULL;
	sink->priv->desc = NULL;
	sink->priv->started = FALSE;
}

/***** Public methods *****/
/**
 * facq_sink_get_name:
 * @sink: A #FacqSink object, it can be of any type of sink.
 *
 * Gets the name of the sink.
 *
 * Returns: The name of the sink.
 */
const gchar *facq_sink_get_name(const FacqSink *sink)
{
	g_return_val_if_fail(FACQ_IS_SINK(sink),NULL);

	return sink->priv->name;
}

/**
 * facq_sink_get_description:
 * @sink: A #FacqSink object, it can be of any type of sink.
 *
 * Gets the description of the sink.
 *
 * Returns: The description of the sink.
 */
const gchar *facq_sink_get_description(const FacqSink *sink)
{
	g_return_val_if_fail(FACQ_IS_SINK(sink),NULL);

	return sink->priv->desc;
}

/**
 * facq_sink_get_started:
 * @sink: A #FacqSink object, it can be of any type of sink.
 *
 * Gets the current status of the sink, started or stopped,
 * that is %TRUE or %FALSE.
 *
 * Returns: A #gboolean with the current status of the #FacqSink.
 */
gboolean facq_sink_get_started(FacqSink *sink)
{
	g_return_val_if_fail(FACQ_IS_SINK(sink),FALSE);

	return sink->priv->started;
}
/*****--- Virtual methods ---*****/
/**
 * facq_sink_to_file:
 * @sink: A #FacqSink object, it can be of any type of sink.
 * @file: A #GKeyFile object.
 * @group: The group name for the #GKeyFile, @file.
 *
 * Calls the facq_sink_*_to_file() function related to the type of sink,
 * allowing the sink to store important attributes in a #GKeyFile.
 * This function is used by facq_stream_save().
 */
void facq_sink_to_file(FacqSink *sink,GKeyFile *file,const gchar *group)
{
	g_return_if_fail(FACQ_IS_SINK(sink));
	g_return_if_fail(file);
	g_return_if_fail(g_key_file_has_group(file,group));

	if(FACQ_SINK_GET_CLASS(sink)->sinksave)
		FACQ_SINK_GET_CLASS(sink)->sinksave(sink,file,group);
}

/**
 * facq_sink_start:
 * @sink: A #FacqSink object, it can be of any type of sink.
 * @stmd: A #FacqStreamData object.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * This function calls the facq_sink_*_start() function related to the
 * sink type. See #FacqSinkClass for more details.
 * If the start is successful the started property of the sink will be set to
 * %TRUE. If the sink is already started the function will do nothing and will
 * return %TRUE.
 */
gboolean facq_sink_start(FacqSink *sink,const FacqStreamData *stmd,GError **err)
{
	gboolean ret = TRUE;
	g_return_val_if_fail(FACQ_IS_SINK(sink),FALSE);
	
	if(!sink->priv->started){
		if( (FACQ_SINK_GET_CLASS(sink)->sinkstart) )
			ret = FACQ_SINK_GET_CLASS(sink)->sinkstart(sink,stmd,err);
		else
			ret = TRUE;

		sink->priv->started = TRUE;
	}

	return ret;
}

/**
 * facq_sink_poll:
 * @sink: A #FacqSink it can be of any type of sink.
 * @stmd: A #FacqStreamData object.
 *
 * This function calls the facq_sink_*_poll() function related to the
 * sink type. See #FacqSinkClass for more details.
 *
 * Returns: 1 if data can be written without blocking, 0 on timeout, -1 on
 * error.
 */
gint facq_sink_poll(FacqSink *sink,const FacqStreamData *stmd)
{
#if ENABLE_DEBUG
	g_return_val_if_fail(FACQ_IS_SINK(sink),-1);
#endif
	if( (FACQ_SINK_GET_CLASS(sink)->sinkpoll) )
		return FACQ_SINK_GET_CLASS(sink)->sinkpoll(sink,stmd);
	else
		return 1;
}

/**
 * facq_sink_write:
 * @sink: A #FacqSink object, it can be of any type.
 * @stmd: A #FacqStreamData object.
 * @chunk: A #FacqChunk that contains the data to write to the sink.
 * @err: A #GError, you must set it in case of error.
 *
 * This function calls the facq_sink_*_write function related to the sink
 * type. See #FacqSinkClass for more details.
 *
 * Returns: A #GIOStatus according to the operation result.
 */
GIOStatus facq_sink_write(FacqSink *sink,const FacqStreamData *stmd,FacqChunk *chunk,GError **err)
{
#if ENABLE_DEBUG
	g_return_val_if_fail(FACQ_IS_SINK(sink),G_IO_STATUS_ERROR);
#endif
	
	return FACQ_SINK_GET_CLASS(sink)->sinkwrite(sink,stmd,chunk,err);
}

/* facq_sink_stop:
 * @sink: A #FacqSink object, it can be of any type of sink.
 * @stmd: A #FacqStreamData object.
 * @err: A #GError, you must set it in case of error if not %NULL.
 *
 * This function calls the facq_sink_*_stop function related to 
 * the sink type. See #FacqSinkClass for more details.
 *
 * Note that facq_sink_stop could be called before the sink has received any
 * sample.
 */
gboolean facq_sink_stop(FacqSink *sink,const FacqStreamData *stmd,GError **err)
{
	gboolean ret = TRUE;

	g_return_val_if_fail(FACQ_IS_SINK(sink),FALSE);

	if(sink->priv->started){
		if( (FACQ_SINK_GET_CLASS(sink)->sinkstop) )
			ret = FACQ_SINK_GET_CLASS(sink)->sinkstop(sink,stmd,err);
		else
			return TRUE;
	
		sink->priv->started = FALSE;
	}

	return ret;
}

/**
 * facq_sink_free:
 * @sink: A #FacqSink object, it can be any type of sink.
 *
 * Destroys the sink object, calling the destructor.
 */
void facq_sink_free(FacqSink *sink)
{
	g_return_if_fail(FACQ_IS_SINK(sink));

	FACQ_SINK_GET_CLASS(sink)->sinkfree(sink);
}

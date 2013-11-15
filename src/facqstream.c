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
#include "facqmisc.h"
#include "facqcatalog.h"
#include "facqlog.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqchunk.h"
#include "facqstreamdata.h"
#include "facqsource.h"
#include "facqoperation.h"
#include "facqoperationlist.h"
#include "facqsink.h"
#include "facqpipelinemessage.h"
#include "facqpipelinemonitor.h"
#include "facqpipeline.h"
#include "facqstream.h"

/**
 * SECTION:facqstream
 * @include:facqstream.h
 * @short_description: Controls and handles the needed components and operations
 * involved in a stream of data.
 *
 * This high level class, tries to simplify the creation of streams of data
 * with a simple API. When you create a new #FacqStream with facq_stream_new()
 * you must close it first, before you can start moving data. A #FacqStream is
 * closed when it has a #FacqSource and a #FacqSink, optionally it can have also
 * one or more #FacqOperation objects, to check if a stream is closed you can
 * use facq_stream_is_closed().
 *
 * When the stream is closed, you can start moving data from the source to the
 * sink (and trough the operations) with facq_stream_start(), and stop it with
 * facq_stream_stop().
 *
 * A #FacqStream only can have a source and a sink at the same time,
 * and this elements along the #FacqOperation objects, should be added and
 * removed in order, for example if you want to change the #FacqSource object
 * and you have a #FacqSource and a #FacqSink in your stream, you must remove
 * the #FacqSink and then the #FacqSource, before you can add the new
 * #FacqSource, the #FacqSource object is always the first element that is added
 * to the #FacqStream, and the #FacqSink is the last. To add a #FacqSource to a
 * #FacqStream you can use facq_stream_set_source(), to get the #FacqSource
 * object from the #FacqStream facq_stream_get_source() is provided and to remove it
 * you can use facq_stream_remove_source(). Analogous functions does exist for
 * the #FacqSink object, facq_stream_set_sink(), facq_stream_get_sink() and
 * facq_stream_remove_sink(). Before adding the sink you can add or remove
 * operations with facq_stream_append_operation() and
 * facq_stream_remove_operation(), also you can get the number of operations
 * added to the stream with facq_stream_get_operation_num() and get the n-esim
 * #FacqOperation object with facq_stream_get_operation().
 *
 * To remove all the elements (source, operations, and sink) from a stream you
 * can use facq_stream_clear(), to set a name to the stream you can use
 * facq_stream_set_name(), to get the name from the stream you can use
 * facq_stream_get_name().
 *
 * Another way of creating a #FacqStream object is using an ini like file stored
 * on the filesystem. You can use facq_stream_load() for creating a new
 * #FacqStream from one of this files, and facq_stream_save() for creating one
 * of this files from an existing stream.
 *
 * When the stream is no longer needed you can destroy it with
 * facq_stream_free(), note that the #FacqSource and the other elements will be
 * destroyed too, you don't need to destroy them.
 *
 * <sect1 id="frs-files">
 * <title>Freeacq Readable Stream files</title>
 * <para>
 * For saving and loading the different elements that compound a stream, two
 * functions are provided facq_stream_save() and facq_stream_load().
 * Plain text files are used, allowing the user to modify certain values without
 * the need to recreate the stream again from the start. The syntax used by this
 * files is the classic .ini file like format, for more details see #GKeyFile.
 * A valid .frs file should have three or more groups of keys. The first group
 * should be the Stream group, containing the stream name. The second group
 * should contain the source identifier and it's parameters. The third group
 * can contain the sink identifier and it's parameters if no operations are
 * added to the stream, or an operation identifier along with it's parameters.
 * The sink group should be the last group present in the file. Just for more
 * clarity a .frs should follow the following template:
 * <informalexample>
 * <programlisting>
 * [Stream]
 * name=Untitled stream
 *
 * # This is a valid comment.
 * 
 * # Each group identifier an item in the stream, the group name should contain
 * # the item identifier, Software in this example, and the position that the
 * # item has in the stream, 0, because this is the source item.
 * [Software,0]
 * function=0
 * amplitude=5
 * period=1
 * wave-period=1
 * n-channels=1
 *
 * # This is an operation
 * [Plug,1]
 * address=127.0.0.1
 * port=3000
 *
 * # This is the sink description, some items doesn't require parameters.
 * [Null,2]
 * </programlisting>
 * </informalexample>
 * </para>
 * </sect1>
 */

/**
 * FacqStream:
 *
 * Contains the private fields of a #FacqStream.
 */

/**
 * FacqStreamClass:
 *
 * Class for the #FacqStream objects.
 */

/**
 * FacqStreamError:
 * @FACQ_STREAM_ERROR_CLOSED: The stream needs to be closed.
 * @FACQ_STREAM_ERROR_FAILED: There was a failure in some stream operation.
 *
 * Enum values for #FacqStreamError.
 */

G_DEFINE_TYPE(FacqStream,facq_stream,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_STREAM_NAME,
	PROP_RING_SIZE,
	PROP_MONITOR_DATA,
	PROP_MONITOR_ERROR_CB,
	PROP_MONITOR_STOP_CB,
};

struct _FacqStreamPrivate {
	gchar *name;
	FacqSource *src;
	FacqOperationList *oplist;
	FacqSink *sink;
	FacqPipelineMonitor *mon;
	gpointer mon_data;
	FacqPipelineMonitorCb stop_cb;
	FacqPipelineMonitorCb error_cb;
	FacqPipeline *p;
	guint ring_chunks;
};

GQuark facq_stream_error_quark(void)
{
	return g_quark_from_static_string("facq-stream-error-quark");
}

/* Private methods */

static GString *facq_stream_save_get_group_names(const FacqStream *stream)
{
	GString *key_file_content = NULL;
	guint n_items = 0, i = 0;
	const gchar *item_name = NULL;
	gchar *group_name = NULL;

	/* We have a source, and a sink and n operations , n_items */
	n_items = facq_stream_get_operation_num(stream);
	n_items += 2;

	/* Create group names for the key file, note that each group should have
	 * a different name, so we append a number at the end of the name */
	key_file_content = g_string_new("[Stream]\n");

	/* create a group for the source */
	item_name = facq_source_get_name(stream->priv->src);
	group_name = g_strdup_printf("[%s,%u]\n",item_name,i);
	key_file_content = g_string_append(key_file_content,group_name);
	g_free(group_name);
	i++;

	/* create a group for each operation if any */
	if(n_items-2){
		for(i = 1; i < (n_items-1);i++){
			item_name = facq_operation_get_name(facq_operation_list_get(stream->priv->oplist,i-1));
			group_name = g_strdup_printf("[%s,%u]\n",item_name,i);
			key_file_content = g_string_append(key_file_content,group_name);
			g_free(group_name);
		}
	}

	/* create a group for the sink */
	item_name = facq_sink_get_name(stream->priv->sink);
	group_name = g_strdup_printf("[%s,%u]\n",item_name,i);
	key_file_content = g_string_append(key_file_content,group_name);
	g_free(group_name);

	return key_file_content;
}

static void facq_stream_save_item(GKeyFile *file,gpointer item,guint index)
{
	const gchar *name = NULL;
	gchar *group_name = NULL;

	g_return_if_fail(item);

	if(FACQ_IS_SOURCE(item)){
		name = facq_source_get_name(item);
		group_name = g_strdup_printf("%s,%u",name,index);
		facq_source_to_file(FACQ_SOURCE(item),file,group_name);
		g_free(group_name);
		return;
	}
	if(FACQ_IS_OPERATION(item)){
		name = facq_operation_get_name(item);
		group_name = g_strdup_printf("%s,%u",name,index);
		facq_operation_to_file(FACQ_OPERATION(item),file,group_name);
		g_free(group_name);
		return;
	}
	if(FACQ_IS_SINK(item)){
		name = facq_sink_get_name(item);
		group_name = g_strdup_printf("%s,%u",name,index);
		facq_sink_to_file(FACQ_SINK(item),file,group_name);
		g_free(group_name);
		return;
	}
}

static void facq_stream_save_to_file(const gchar *filename,const gchar *key_file_content,GError **err)
{
	GIOChannel *channel = NULL;
	GError *local_err = NULL;

	channel = g_io_channel_new_file(filename,"w",&local_err);
	if(local_err || !channel){
		goto error;
	}

	if( g_io_channel_write_chars(channel,key_file_content,-1,NULL,&local_err) != G_IO_STATUS_NORMAL){
		goto error;
	}

	g_io_channel_unref(channel);

	return;

	error:
	if(channel)
		g_io_channel_unref(channel);
	if(local_err){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
		g_clear_error(&local_err);
	}
	if(err)
		g_set_error_literal(err,FACQ_STREAM_ERROR,
					FACQ_STREAM_ERROR_FAILED,"Can't save to file");
	return;
}

/*****--- GObject magic ---*****/
static void facq_stream_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqStream *stream = FACQ_STREAM(self);

	switch(property_id){
	case PROP_STREAM_NAME: g_value_set_string(value,stream->priv->name);
	break;
	case PROP_MONITOR_DATA: g_value_set_pointer(value,stream->priv->mon_data);
	break;
	case PROP_MONITOR_ERROR_CB: g_value_set_pointer(value,stream->priv->error_cb);
	break;
	case PROP_MONITOR_STOP_CB: g_value_set_pointer(value,stream->priv->stop_cb);
	break;
	case PROP_RING_SIZE: g_value_set_uint(value,stream->priv->ring_chunks);
	break;
	default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(stream,property_id,pspec);
	}
}

static void facq_stream_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqStream *stream = FACQ_STREAM(self);

	switch(property_id){
	case PROP_STREAM_NAME: stream->priv->name = g_value_dup_string(value);
	break;
	case PROP_MONITOR_DATA: stream->priv->mon_data = g_value_get_pointer(value);
	break;
	case PROP_MONITOR_ERROR_CB: stream->priv->error_cb = g_value_get_pointer(value);
	break;
	case PROP_MONITOR_STOP_CB: stream->priv->stop_cb = g_value_get_pointer(value);
	break;
	case PROP_RING_SIZE: stream->priv->ring_chunks = g_value_get_uint(value);
	break;
	default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(stream,property_id,pspec);
	}
}

static void facq_stream_finalize(GObject *self)
{
	FacqStream *stream = FACQ_STREAM(self);

	if(stream->priv->name)
		g_free(stream->priv->name);

	if(FACQ_IS_SOURCE(stream->priv->src))
		facq_source_free(stream->priv->src);
	if(FACQ_IS_SINK(stream->priv->sink))
		facq_sink_free(stream->priv->sink);
	if(stream->priv->oplist)
		facq_operation_list_free(stream->priv->oplist);
	if(stream->priv->p)
		if(FACQ_IS_PIPELINE(stream->priv->p))
			facq_pipeline_free(stream->priv->p);

	if(stream->priv->mon)
		if(FACQ_IS_PIPELINE_MONITOR(stream->priv->mon))
			facq_pipeline_monitor_free(stream->priv->mon);

	if(G_OBJECT_CLASS(facq_stream_parent_class)->finalize)
    		(*G_OBJECT_CLASS(facq_stream_parent_class)->finalize)(self);
}

static void facq_stream_constructed(GObject *self)
{
	FacqStream *stream = FACQ_STREAM(self);
	
	stream->priv->oplist = facq_operation_list_new();
	stream->priv->mon = 
		facq_pipeline_monitor_new(stream->priv->error_cb,
					  stream->priv->stop_cb,
					  stream->priv->mon_data);
}

static void facq_stream_class_init(FacqStreamClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqStreamPrivate));
	
	object_class->set_property = facq_stream_set_property;
	object_class->get_property = facq_stream_get_property;
	object_class->constructed = facq_stream_constructed;
	object_class->finalize = facq_stream_finalize;

	g_object_class_install_property(object_class,PROP_STREAM_NAME,
					g_param_spec_string("stream-name",
							    "Stream name",
							    "The stream name",
							    "Untitled stream",
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT |
							    G_PARAM_STATIC_STRINGS));

		g_object_class_install_property(object_class,PROP_RING_SIZE,
					g_param_spec_uint("ring-size",
							"Ring size",
							"The number of chunks that the ring buffer can store",
							1,
							G_MAXUINT,
							32,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_MONITOR_DATA,
					g_param_spec_pointer("mon-data",
							     "Monitor data",
							     "The data passed to the callback functions",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));
	
	g_object_class_install_property(object_class,PROP_MONITOR_ERROR_CB,
					g_param_spec_pointer("error-cb",
							     "Error callback",
							     "A callback function called on error condition",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_MONITOR_STOP_CB,
					g_param_spec_pointer("stop-cb",
							     "Stop callback",
							     "A callback function called on stop condition",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));

}

static void facq_stream_init(FacqStream *stream)
{
	stream->priv = G_TYPE_INSTANCE_GET_PRIVATE(stream,FACQ_TYPE_STREAM,FacqStreamPrivate);
	stream->priv->name = NULL;
	stream->priv->src = NULL;
	stream->priv->sink = NULL;
	stream->priv->oplist = NULL;
	stream->priv->p = NULL;
	stream->priv->ring_chunks = 0;
}

/*****--- Public methods ---*****/
/**
 * facq_stream_new:
 * @name: (allow-none): The stream's name.
 * @ring_chunks: The maximum number of chunks to have in ram at the same time.
 * @stop_cb: A #FacqPipelineMonitorCb callback function, it will be called by
 * the #FacqPipelineMonitor object if needed on a stop condition.
 * @error_cb: A #FacqPipelineMonitorCb callback function, it will be called by
 * the #FacqPipelineMonitor object if needed in case of error.
 * @data: (allow-none): A pointer to some data that you want to pass to the
 * callback functions.
 *
 * Creates a new #FacqStream object without any #FacqSource or other elements.
 * See #FacqPipelineMonitor for more details on @stop_cb, @error_cb and
 * @data.
 *
 * Returns: A new #FacqStream object.
 */
FacqStream *facq_stream_new(const gchar *name,guint ring_chunks,FacqPipelineMonitorCb stop_cb,
FacqPipelineMonitorCb error_cb,gpointer data)
{
	return g_object_new(FACQ_TYPE_STREAM,
			    "stream-name",name,
			    "ring-size",ring_chunks,
			    "stop-cb",stop_cb,
			    "error-cb",error_cb,
			    "mon-data",data,
			    NULL);
}

/**
 * facq_stream_is_closed:
 * @stream: A #FacqStream object.
 *
 * Checks if the #FacqStream object has been closed or not.
 * A #FacqStream is closed when it has a #FacqSink object.
 *
 * Returns: %TRUE if closed, %FALSE in other case.
 */
gboolean facq_stream_is_closed(const FacqStream *stream)
{
	g_return_val_if_fail(FACQ_IS_STREAM(stream),FALSE);

	if(stream->priv->src && stream->priv->sink)
		if(FACQ_IS_SOURCE(stream->priv->src) 
			&& FACQ_IS_SINK(stream->priv->sink))
				return TRUE;
	return FALSE;
}

/**
 * facq_stream_set_name:
 * @stream: A #FacqStream object.
 * @name: A string with the desired name.
 *
 * Sets the name of the @stream, #FacqStream object.
 */
void facq_stream_set_name(FacqStream *stream,const gchar *name)
{
	g_return_if_fail(FACQ_IS_STREAM(stream));
	g_return_if_fail(name);

	if(stream->priv->name)
		g_free(stream->priv->name);

	stream->priv->name = g_strdup(name);
}

/**
 * facq_stream_get_name:
 * @stream: A #FacqStream object.
 * 
 * Gets the name of the @stream, #FacqStream object.
 *
 * Returns: The current name of the #FacqStream object 
 * or %NULL in case of error. You must free it with g_free() 
 * when no longer needed.
 */
gchar *facq_stream_get_name(const FacqStream *stream)
{
	gchar *ret = NULL;
	
	g_return_val_if_fail(FACQ_IS_STREAM(stream),NULL);

	ret = g_strdup(stream->priv->name);
	return ret;
}

/**
 * facq_stream_set_source:
 * @stream: A #FacqStream object.
 * @src: A #FacqSource object.
 *
 * Adds the #FacqSource object, @src, to the @stream, #FacqStream, object.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_stream_set_source(FacqStream *stream,FacqSource *src)
{
	g_return_val_if_fail(FACQ_IS_STREAM(stream) 
		&& FACQ_IS_SOURCE(src),FALSE);
	
	if(stream->priv->sink)
		return FALSE;

	if(stream->priv->src)
		return FALSE;

	stream->priv->src = src;
	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,"%s","Source added to stream");
	return TRUE;
}

/**
 * facq_stream_get_source:
 * @stream: A #FacqStream object.
 *
 * Gets the #FacqSource object in the stream.
 *
 * Returns: a #FacqSource object (if any) previously added to the stream or %NULL if
 * none.
 */
FacqSource *facq_stream_get_source(const FacqStream *stream)
{
	g_return_val_if_fail(FACQ_IS_STREAM(stream),NULL);

	if(FACQ_IS_SOURCE(stream->priv->src))
		return stream->priv->src;

	return NULL;
}

/**
 * facq_stream_remove_source:
 * @stream: A #FacqStream object.
 *
 * Removes the #FacqSource object from the #FacqStream, @stream, 
 * and destroys it.
 */
void facq_stream_remove_source(FacqStream *stream)
{
	g_return_if_fail(FACQ_IS_STREAM(stream));

	if(FACQ_IS_SOURCE(stream->priv->src)){
		facq_source_free(stream->priv->src);
		stream->priv->src = NULL;
	}
}

/**
 * facq_stream_set_sink:
 * @stream: A #FacqStream object.
 * @sink: A #FacqSink object.
 *
 * Sets the sink in the #FacqStream object. After adding a #FacqSink to the
 * #FacqStream you won't be able to add new elements to the #FacqStream, and it 
 * will be closed.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_stream_set_sink(FacqStream *stream,FacqSink *sink)
{
	g_return_val_if_fail(FACQ_IS_STREAM(stream) && FACQ_IS_SINK(sink),FALSE);
	if(FACQ_IS_SOURCE(stream->priv->src)){
		stream->priv->sink = sink;
		facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,"%s","Sink added to stream");
		return TRUE;
	}
	return FALSE;
}

/**
 * facq_stream_get_sink:
 * @stream: A #FacqStream object.
 *
 * Gets the #FacqSink object (if any) from the #FacqStream object.
 *
 * Returns: A #FacqSink object (if any) or %NULL in other case.
 */
FacqSink *facq_stream_get_sink(const FacqStream *stream)
{
	g_return_val_if_fail(FACQ_IS_STREAM(stream),NULL);
	if(FACQ_IS_SINK(stream->priv->sink))
		return stream->priv->sink;
	return NULL;
}

/**
 * facq_stream_remove_sink:
 * @stream: A #FacqStream object.
 *
 * Removes the #FacqSink object (if any) from the #FacqStream object.
 * The #FacqSink is destroyed after being removed.
 */
void facq_stream_remove_sink(FacqStream *stream)
{
	g_return_if_fail(FACQ_IS_STREAM(stream));
	if(FACQ_IS_SINK(stream->priv->sink)){
		facq_sink_free(stream->priv->sink);
		stream->priv->sink = NULL;
	}
}

/**
 * facq_stream_append_operation:
 * @stream: A #FacqStream object.
 * @op: A #FacqOperation object.
 *
 * Appends an #FacqOperation to the internal #FacqOperationList of the
 * #FacqStream object, @stream.
 *
 * Returns: The number of operations in the #FacqStream, or 0 in case of error.
 */
guint facq_stream_append_operation(FacqStream *stream,const FacqOperation *op)
{
	guint ret = 0;
	g_return_val_if_fail(FACQ_IS_STREAM(stream) && FACQ_IS_OPERATION(op),0);
	if(facq_stream_is_closed(stream) == TRUE) 
		return 0;
	ret = facq_operation_list_add(stream->priv->oplist,op);
	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,"%s","Operation added to stream");
	return ret;
}

/**
 * facq_stream_get_operation:
 * @stream: A #FacqStream object.
 * @index: The index of the operation (0 or greater).
 *
 * Gets the #FacqOperation object corresponding to the index, @index.
 *
 * Returns: The #FacqOperation object or %NULL, if no operation matches the
 * index.
 */
FacqOperation *facq_stream_get_operation(FacqStream *stream,guint index)
{
	g_return_val_if_fail(FACQ_IS_STREAM(stream),NULL);

	return facq_operation_list_get(stream->priv->oplist,index);
}

/**
 * facq_stream_remove_operation:
 * @stream: A #FacqStream object.
 *
 * Removes the latest added operation if any, destroying it.
 *
 * Returns: %G_MAXUINT in case of error, or the number of remaining operations
 * if successful.
 */
guint facq_stream_remove_operation(FacqStream *stream)
{
	g_assert(FACQ_IS_STREAM(stream));
	if(facq_stream_is_closed(stream) == TRUE) 
		return G_MAXUINT;
	return facq_operation_list_del_and_destroy(stream->priv->oplist);
}

/**
 * facq_stream_get_operation_num:
 * @stream: A #FacqStream object.
 *
 * Retrieves the number of operations in the #FacqStream.
 *
 * Returns: The number of operations.
 */
guint facq_stream_get_operation_num(const FacqStream *stream)
{
	g_return_val_if_fail(FACQ_IS_STREAM(stream),0);
	return facq_operation_list_get_length(stream->priv->oplist);
}

/**
 * facq_stream_save:
 * @stream: A closed #FacqStream object, see facq_stream_is_closed() for
 * more details.
 * @filename: filename of the file where you want to store stream details.
 * @err: (allow-none): A #GError, it will be set in case of error.
 *
 * Saves the closed #FacqStream, @stream, to the plain text file pointed by
 * @filename, creating it if necessary. You can create a #FacqStream from this
 * file later using facq_stream_load().
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_stream_save(FacqStream *stream,const gchar *filename,GError **err)
{
	GString *key_file_content = NULL;
	GKeyFile *key_file = NULL;
	gchar *txt_key_file_content = NULL;
	gboolean ret = FALSE;
	GError *local_err = NULL;
	guint i = 0, n_operations = 0;
	gpointer item = NULL;

	g_return_val_if_fail(FACQ_IS_STREAM(stream),FALSE);
	g_return_val_if_fail(filename,FALSE);
	g_return_val_if_fail(facq_stream_is_closed(stream),FALSE);

	key_file_content = facq_stream_save_get_group_names(stream);
	key_file = g_key_file_new();
	ret = g_key_file_load_from_data(key_file,
					key_file_content->str,
					-1,
					G_KEY_FILE_NONE,
					&local_err);
	g_string_free(key_file_content,TRUE);
	if(!ret)
		goto error;

	/* save the stream name */
	g_key_file_set_string(key_file,"Stream","name",stream->priv->name);
	
	/* save the source */
	item = stream->priv->src;
	facq_stream_save_item(key_file,item,i);
	i++;

	/* save the operations */
	n_operations = facq_stream_get_operation_num(stream);
	if(n_operations){
		for(i = 0;i < n_operations;i++){
			item = facq_operation_list_get(stream->priv->oplist,i);
			facq_stream_save_item(key_file,item,i+1);
		}
	}

	/* save the sink */
	i = 1 + n_operations;
	item = stream->priv->sink;
	facq_stream_save_item(key_file,item,i);

	txt_key_file_content = g_key_file_to_data(key_file,NULL,NULL);
	facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,"FILE:\n%s\n",txt_key_file_content);
	facq_stream_save_to_file(filename,txt_key_file_content,&local_err);
	g_free(txt_key_file_content);
	g_key_file_free(key_file);

	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,"%s","Stream saved without errors");
	return TRUE;

	error:
	if(local_err){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",
						local_err->message);
		g_clear_error(&local_err);
	}
	if(err)
		g_set_error_literal(err,FACQ_STREAM_ERROR,
					FACQ_STREAM_ERROR_FAILED,
					"Error saving stream");
	return FALSE;
}

static void facq_stream_load_from_key_file(GKeyFile *key_file,const FacqCatalog *cat,FacqStream *stream,GError **err)
{
	GError *local_err = NULL;
	gchar **groups = NULL, **tokens = NULL;
	guint i = 0, n_items = 0, n_operations = 0;
	gsize n_groups = 0;
	gpointer item = NULL;

	/* we are not ready yet, first check that the number of groups it's >
	 * 3 */
	groups = g_key_file_get_groups(key_file,&n_groups);
	if(n_groups < 3){
		g_set_error_literal(&local_err,FACQ_STREAM_ERROR,
					FACQ_STREAM_ERROR_FAILED,"Invalid file");
		goto error;
	}
	n_items = n_groups - 1; //subtract the stream group
	n_operations = n_items - 2;//subtract the source and the sink

	/* start at group 1, cause group 0 is the [Stream] group */

	/* source */
	tokens = g_strsplit(groups[1],",",2);
	item = facq_catalog_item_from_key_file(key_file,
					       groups[1],
					       tokens[0],
					       cat,
					       FACQ_CATALOG_TYPE_SOURCE,
					       &local_err);
	if(local_err)
		goto error;
	facq_stream_set_source(stream,item);
	g_strfreev(tokens);

	for(i = 0;i < n_operations;i++){
		tokens = g_strsplit(groups[i+2],",",2);
		item = facq_catalog_item_from_key_file(key_file,
						       groups[i+2],
						       tokens[0],
						       cat,
						       FACQ_CATALOG_TYPE_OPERATION,
						       &local_err);
		if(local_err)
			goto error;
		facq_stream_append_operation(stream,item);
		g_strfreev(tokens);
	}

	/* sink */
	tokens = g_strsplit(groups[n_groups-1],",",2);
	item = facq_catalog_item_from_key_file(key_file,
					       groups[n_groups-1],
					       tokens[0],
					       cat,
					       FACQ_CATALOG_TYPE_SINK,
					       &local_err);
	g_strfreev(tokens);
	if(local_err)
		goto error;
	facq_stream_set_sink(stream,item);

	g_strfreev(groups);
	return;

	error:
	if(tokens)
		g_strfreev(tokens);
	if(groups)
		g_strfreev(groups);
	if(local_err){
		if(err)
			g_propagate_error(err,local_err);
	}
	else
		g_set_error_literal(err,FACQ_STREAM_ERROR,
					FACQ_STREAM_ERROR_FAILED,"Unknown error loading stream");
	return;
}

/**
 * facq_stream_load:
 * @filename: A filename pointing to the file you want to read.
 * @cat: A #FacqCatalog object.
 * @ring_chunks: The maximum number of chunks to have in ram at the same time.
 * @stop_cb: A #FacqPipelineMonitorCb callback function, it will be called by
 * the #FacqPipelineMonitor object if needed on a stop condition.
 * @error_cb: A #FacqPipelineMonitorCb callback function, it will be called by
 * the #FacqPipelineMonitor object if needed in case of error.
 * @data: A #FacqPipelineMonitorCb callback function, it will be called by
 * the #FacqPipelineMonitor object if needed in case of error.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqStream object, from a .frs (freeacq readable stream) file.
 *
 * Returns: %NULL in case of failure, or a new #FacqStream object if successful.
 */
FacqStream *facq_stream_load(const gchar *filename,const FacqCatalog *cat,guint ring_chunks,FacqPipelineMonitorCb stop_cb,FacqPipelineMonitorCb error_cb,gpointer data,GError **err)
{
	GKeyFile *key_file = NULL;
	GError *local_err = NULL;
	FacqStream *stream = NULL;
	gchar *group_name = NULL, *stream_name = NULL;

	key_file = g_key_file_new();
	if(!g_key_file_load_from_file(key_file,filename,G_KEY_FILE_NONE,&local_err)){
		goto error;
	}
	/* File validation, it should have at least 3 groups, the first
	 * group is the "Stream" group */
	if(!g_key_file_has_group(key_file,"Stream")){
		goto error;
	}
	group_name = g_key_file_get_start_group(key_file);
	if( g_strcmp0(group_name,"Stream") != 0 )
		goto error;
	if(!g_key_file_has_key(key_file,group_name,"name",&local_err))
		goto error;
	stream_name = g_key_file_get_string(key_file,group_name,"name",&local_err);
	if(local_err || !stream_name)
		goto error;
	g_free(group_name);
	stream = facq_stream_new(stream_name,
				 ring_chunks,
				 stop_cb,
				 error_cb,
				 data);
	/* we have the name, now we must load the rest of items in the stream
	 * we do it in this private function */
	facq_stream_load_from_key_file(key_file,cat,stream,&local_err);
	if(local_err)
		goto error;

	g_key_file_free(key_file);
	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,"%s","Stream loaded without errors");
	return stream;

	error:
	if(key_file)
		g_key_file_free(key_file);
	if(stream)
		facq_stream_free(stream);
	if(group_name)
		g_free(group_name);
	if(local_err){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
		g_clear_error(&local_err);
	}
	if(err){
		g_set_error_literal(err,FACQ_STREAM_ERROR,
					FACQ_STREAM_ERROR_FAILED,"Error loading stream");
	}
	return NULL;
}

/**
 * facq_stream_start:
 * @stream: A #FacqStream object, in closed state.
 * @err: (allow-none): A #GError, it will be set in case of error, if not %NULL.
 *
 * Starts the stream, allowing the data to flow from the source to the
 * operations, if any, and to the sink.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_stream_start(FacqStream *stream,GError **err)
{
	const FacqSource *src = NULL;
	const FacqStreamData *stmd = NULL;
	guint n_channels = 0;
	gsize chunk_size = 0;
	GError *local_err = NULL;

	//check that stream is closed if not set error return false
	if(facq_stream_is_closed(stream) != TRUE){
		if(local_err)
			g_set_error_literal(&local_err,FACQ_STREAM_ERROR,
				FACQ_STREAM_ERROR_CLOSED,
					"The stream should be closed");
		goto error;
	}
	src = stream->priv->src;
	stmd = facq_source_get_stream_data(src);
	n_channels = facq_stream_data_get_n_channels(stmd);
	if(FACQ_IS_PIPELINE(stream->priv->p)){
		facq_pipeline_free(stream->priv->p);
		stream->priv->p = NULL;
	}

	//Empty the monitor from previous messages if any
	facq_pipeline_monitor_clear(stream->priv->mon);
	
	//Calculate a valid chunk size
	if(stmd->period <= 1)
		chunk_size = facq_misc_period_to_chunk_size(stmd->period,
							    sizeof(gdouble),
							    stmd->n_channels);
	else
		chunk_size = 8*n_channels;

	//start the pipeline
	stream->priv->p = facq_pipeline_new(chunk_size,
					    stream->priv->ring_chunks,
					    stream->priv->src,
					    stream->priv->oplist,
					    stream->priv->sink,
					    stream->priv->mon,
					    &local_err);
	if(local_err)
		goto error;

	//attach the monitor to the main thread, it will be polled for new
	//messages one time each second.
	facq_pipeline_monitor_attach(stream->priv->mon);

	if(!facq_pipeline_start(stream->priv->p,&local_err)){
		facq_pipeline_monitor_dettach(stream->priv->mon);
		goto error;
	}

	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,"%s","Stream started");
	return TRUE;

	error:
	if(local_err){
		if(err)
			g_propagate_error(err,local_err);
	}
	return FALSE;
}

/**
 * facq_stream_stop:
 * @stream: A previously started #FacqStream object.
 *
 * Stops a previously started #FacqStream object, it can block
 * some time while the threads are stopped.
 */
void facq_stream_stop(FacqStream *stream)
{	
	//stop the pipeline
	if(stream->priv->p)
		facq_pipeline_stop(stream->priv->p);
	
	facq_log_write("Destroying the pipeline",FACQ_LOG_MSG_TYPE_DEBUG);
	facq_pipeline_free(stream->priv->p);
	stream->priv->p = NULL;

	//detach the monitor
	facq_pipeline_monitor_dettach(stream->priv->mon);

	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,"%s","Stream stopped");
}

/**
 * facq_stream_clear:
 * @stream: A #FacqStream object.
 *
 * Clears the #FacqStream object, that means that the
 * #FacqSource, #FacqOperation objects and the #FacqSink
 * are removed (and destroyed) from the stream, leaving the
 * stream empty again.
 */
void facq_stream_clear(FacqStream *stream)
{
	g_return_if_fail(FACQ_IS_STREAM(stream));
	
	if(stream->priv->sink)
		facq_stream_remove_sink(stream);
	if(stream->priv->oplist)
		while(facq_stream_get_operation_num(stream) != 0)
			facq_stream_remove_operation(stream);
	if(stream->priv->src)
		facq_stream_remove_source(stream);
}

/**
 * facq_stream_free:
 * @stream: A #FacqStream object.
 *
 * Destroys a no longer needed #FacqStream object, including
 * the #FacqSource, the #FacqOperation objects and the #FacqSink
 * if needed.
 */
void facq_stream_free(FacqStream *stream)
{
	g_return_if_fail(FACQ_IS_STREAM(stream));
	g_object_unref(G_OBJECT(stream));
}

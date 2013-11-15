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
#include "facqlog.h"
#include "facqglibcompat.h"
#include "facqchunk.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqbuffer.h"
#include "facqstreamdata.h"
#include "facqsource.h"
#include "facqoperation.h"
#include "facqoperationlist.h"
#include "facqsink.h"
#include "facqpipelinemessage.h"
#include "facqpipelinemonitor.h"
#include "facqpipeline.h"

/**
 * SECTION:facqpipeline
 * @title: FacqPipeline
 * @short_description: A multithreaded pipeline implementation
 * @include: facqpipeline.h
 * @see_also: #FacqChunk,#FacqPipelineMonitor,#FacqBuffer,#FacqSource,#FacqSink,
 * #FacqOperation
 *
 * Note that you shouldn't need to use this object directly, you probably want
 * to use #FacqStream instead, which encapsulates all the internal details of
 * using a #FacqPipeline, hiding complexities to the user.
 *
 * A #FacqPipeline provides a multithreaded pipeline implementation.
 * A pipeline is an abstract concept that deals with the data transmission
 * from a #FacqSource to a #FacqSink, with the possible execution of one or
 * more #FacqOperation that can visualize or manipulate the data before the
 * sink takes care of it.
 * 
 * A #FacqPipeline object should reference a #FacqSource object and a #FacqSink
 * object else the data wouldn't be able to flow from the source to the sink.
 * A #FacqOperationList can optionally be passed to the object at creation time.
 *
 * When the pipeline is started data begins to flow from the source to the operations. 
 * Operation are executed in a sequential way (If any), finally the data is 
 * pushed to the sink.
 *
 * <informalexample>
 * <programlisting>
 * ----------      ---------------             -------------
 * |        |      |             |             |           |
 * | Source | -->  | Operation 0 | --> ... --> |    Sink   |
 * |        |      |             |             |           |
 * ----------      ---------------             -------------
 * </programlisting>
 * </informalexample>
 *
 * This process can go until the pipeline is stopped.
 * A pipeline can be stopped calling facq_pipeline_stop(). In case of error
 * or if the acquisition has finished (For example in EOF condition) a
 * #FacqPipelineMessage will be written to the #FacqPipelineMonitor, but note 
 * that it won't be auto stopped, you should call facq_pipeline_stop(), in
 * your callback functions (This is not needed if you are using #FacqStream
 * instead, but in that case you should call facq_stream_stop() in your
 * callbacks instead). Note that your callback functions will always be called
 * in the main thread.
 *
 * A new #FacqPipeline can be created with facq_pipeline_new(), can be 
 * started with facq_pipeline_start() and can be stopped with
 * facq_pipeline_stop(). When the #FacqPipeline object is no longer needed
 * destroy it with facq_pipeline_free().
 *
 * # Internal details #
 *
 * The following subsection describes implementation details, you can ignore
 * them.
 *
 * A #FacqPipeline object makes use of #FacqSource, #FacqSink,
 * #FacqOperationList, #FacqPipelineMonitor and #FacqBuffer objects to operate.
 *
 * When a pipeline is created with facq_pipeline_new() the new created object
 * contains pointers to a #FacqSource, a #FacqOperationList, a #FacqSink and
 * a #FacqPipelineMonitor.
 * Also a new #FacqBuffer is created at construction time along with a producer
 * thread and a consumer thread. This objects will provide the multithreaded
 * capabilites to the pipeline.
 * 
 * When the pipeline is started with facq_pipeline_start() the first step done
 * by the pipeline is trying to obtain the #FacqStreamData from the source, if
 * this step success, the pipeline will try to start all the operations in the
 * #FacqOperationList (If any). After this steps the sink and the source are 
 * started with facq_sink_start() and facq_source_start(), after this
 * steps 2 new threads are created, a producer thread, consumer thread.
 *
 *
 * <emphasis>Producer thread</emphasis>
 *
 * This thread polls the source for new data and after filling a #FacqChunk,
 * tries to convert the data to #gdouble (If needed, see facq_source_conv() ), 
 * after this the #FacqChunk is pushed to the #FacqBuffer object. 
 * This is repeated until facq_buffer_get_exit() returns TRUE. 
 * After this the source is stopped and the thread destroys itself.
 *
 *
 * <emphasis>Consumer thread</emphasis>
 *
 * This thread pops the data from the #FacqBuffer, this data is obtained in form
 * of #FacqChunk. The thread checks for a new #FacqChunk in each iteration and
 * after receiving it the chunk is dispatched. In case of error the loop is
 * interrupted, the sink is stopped, and the facq_buffer_exit() function is
 * called.
 * When facq_buffer_get_exit() returns TRUE, the thread pops all the remaining 
 * chunks in the #FacqBuffer dispatching them, stopping the sink when finished, 
 * and destroying itself.
 *
 * The process of dispatching a #FacqChunk involves the following steps:
 * - If the operation list is not empty, execute each operation in order. The
 *   data in the chunk can be used in each operation.
 * - The sink is polled, the thread will wait until the sink is ready.
 * - The data is written to the sink.
 * - The array is recycled.
 * 
 */

/** 
 * FacqPipelineClass:
 */

/**
 * FacqPipeline:
 *
 * Contains all the private data needed by the pipeline.
 */

#define EOF_READING_SOURCE   "End of file in source"
#define EOF_WRITING_SINK     "End of file in sink"
#define ERROR_POLLING_SOURCE "Error while polling the source"
#define ERROR_READING_SOURCE "Error while reading the source"
#define ERROR_POLLING_SINK   "Error while polling the sink"
#define ERROR_WRITING_SINK   "Error while writing to the sink"
#define ERROR_OPERATION_DO   "Error in operation"
#define ERROR_PIPELINE_START "Error starting the pipeline"

static void facq_pipeline_initable_iface_init(GInitableIface  *iface);
static gboolean facq_pipeline_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);

G_DEFINE_TYPE_WITH_CODE(FacqPipeline,facq_pipeline,G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_pipeline_initable_iface_init));

enum {
	PROP_0,
	PROP_RING_SIZE,
	PROP_CHUNK_SIZE,
	PROP_MONITOR,
	PROP_SOURCE,
	PROP_OPERATION_LIST,
	PROP_SINK
};

struct _FacqPipelinePrivate {
	GError *construct_error;
	guint ring_chunks;
	guint chunk_size;
	FacqBuffer *buf;
	GThread *producer;
	GThread *consumer;
	FacqPipelineMonitor *mon;
	FacqSource *src;
	FacqOperationList *oplist;
	FacqSink *sink;
};

GQuark facq_pipeline_error_quark(void)
{
	return g_quark_from_static_string("facq-pipeline-error-quark");
}

/*****--- private methods ---*****/
static void facq_pipeline_error_condition(FacqPipeline *p,const gchar *err)
{
	FacqPipelineMessage *msg = NULL;

	msg = facq_pipeline_message_new(FACQ_PIPELINE_MESSAGE_TYPE_ERROR,err);
	facq_pipeline_monitor_push(p->priv->mon,msg);
}

static void facq_pipeline_stop_condition(FacqPipeline *p,const gchar *info)
{
	FacqPipelineMessage *msg = NULL;

	msg = facq_pipeline_message_new(FACQ_PIPELINE_MESSAGE_TYPE_STOP,info);
	facq_pipeline_monitor_push(p->priv->mon,msg);
}

/* facq_pipeline_start_cleanup:
 *
 * @p: A #FacqPipeline Object.
 * @stmd: A #FacqStreamData object.
 *
 * This private function helps stopping the started elements when the
 * facq_pipeline_start() fails before starting the threads, the last element
 * started is the source, so if the source fails, it doesn't need to be stopped.
 * That's the reason cause the source is not stopped in this function.
 */
static void facq_pipeline_start_cleanup(FacqPipeline *p,const FacqStreamData *stmd)
{
	GError *local_err = NULL;

#if ENABLE_DEBUG
	facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,"%s","Pipeline cleanup started");
#endif
	
	if(!facq_source_stop(p->priv->src,&local_err)){
		if(local_err){
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
			g_clear_error(&local_err);
		}
		else {
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",
						"Unknown error stopping the source");
		}
	}
	
	if(!facq_sink_stop(p->priv->sink,stmd,&local_err)){
		if(local_err){
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
			g_clear_error(&local_err);
		}
		else {
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",
					"Unknown error stopping sink");
		}
	}

	if(!facq_operation_list_stop(p->priv->oplist,stmd,&local_err)){
		if(local_err){
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
			g_clear_error(&local_err);
		}
		else {
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",
						"Unknown error stopping operation");
		}
	}
}

static gboolean producer_read_fun(FacqPipeline *p,FacqSource *src,FacqChunk *src_chunk,gsize *bytes_read)
{
	GError *local_err = NULL;
	gsize count = 0;
	gchar *buf = NULL;

	count = facq_chunk_get_free_bytes(src_chunk);
	buf = facq_chunk_write_pos(src_chunk);

	switch(facq_source_read(src,buf,count,bytes_read,&local_err)){
	case G_IO_STATUS_NORMAL:
#if ENABLE_DEBUG
		facq_log_write("P G_IO_STATUS_NORMAL",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
		facq_chunk_add_used_bytes(src_chunk,*bytes_read);
		return TRUE;
	case G_IO_STATUS_AGAIN:
#if ENABLE_DEBUG
		facq_log_write("P G_IO_STATUS_AGAIN",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
		return TRUE;
	case G_IO_STATUS_EOF:
#if ENABLE_DEBUG
		facq_log_write("P G_IO_STATUS_EOF",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
		facq_pipeline_stop_condition(p,EOF_READING_SOURCE);
		facq_log_write(EOF_READING_SOURCE,FACQ_LOG_MSG_TYPE_INFO);
		return FALSE;
	case G_IO_STATUS_ERROR:
#if ENABLE_DEBUG
		facq_log_write("P G_IO_STATUS_ERROR",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
		if(local_err){
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
			g_clear_error(&local_err);
		}
		facq_pipeline_error_condition(p,ERROR_READING_SOURCE);
		return FALSE;
	default:
		return FALSE;
	}
}

static gpointer producer_fun(gpointer pipeline)
{
	FacqPipeline *p = FACQ_PIPELINE(pipeline);
	const FacqStreamData *stmd = NULL;
	FacqSource *src = p->priv->src;
	FacqChunk *dst_chunk = NULL, *src_chunk = NULL;
	gboolean conv = FALSE;
	gint ret = 0;
	GError *src_stop_err = NULL;
	gsize absolute_bytes_read = 0, total_bytes_read = 0, bytes_read = 0;
	GTimer *timer = NULL;
	gdouble total_seconds = 0;

	g_return_val_if_fail(FACQ_IS_PIPELINE(p),NULL);

	timer = g_timer_new();
	stmd = facq_source_get_stream_data(src);
	conv = facq_source_needs_conv(src);
	dst_chunk = facq_buffer_get_recycled(p->priv->buf);

	if(conv)
		src_chunk = facq_chunk_new(stmd->bps*(p->priv->chunk_size/sizeof(gdouble)),NULL);
	else
		src_chunk = dst_chunk;

	while(!facq_buffer_get_exit(p->priv->buf)){
		while(total_bytes_read != src_chunk->len){
#if ENABLE_DEBUG
			facq_log_write("Producer: Polling source",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
			ret = facq_source_poll(src);
			if(ret < 0){
#if ENABLE_DEBUG
				facq_log_write("Producer: Error polling source",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
				facq_pipeline_error_condition(p,ERROR_POLLING_SOURCE);
				goto exit;
			}
			else if(ret > 0){
				bytes_read = 0;
				if(!producer_read_fun(p,src,src_chunk,&bytes_read))
					goto exit;
#if ENABLE_DEBUG
				facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,"Producer: read ""%"G_GSIZE_FORMAT" bytes",bytes_read);
#endif
				total_bytes_read += bytes_read;
				absolute_bytes_read += bytes_read;
			}
			else continue;
		}
		if(conv){
			facq_source_conv(src,
					 src_chunk->data,
					 (gdouble *)dst_chunk->data,
					 dst_chunk->len/sizeof(gdouble));
			facq_chunk_add_used_bytes(dst_chunk,dst_chunk->len);
		}
		facq_buffer_push(p->priv->buf,dst_chunk);
#if ENABLE_DEBUG
		facq_log_write("Producer: going to sleep",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
		dst_chunk = facq_buffer_get_recycled(p->priv->buf);
#if ENABLE_DEBUG
		facq_log_write("Producer: waking up again",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
		total_bytes_read = bytes_read = 0;
		if(!conv)
			src_chunk = dst_chunk;
		else
			facq_chunk_clear(src_chunk);
	}

	exit:
	g_timer_stop(timer);

	facq_buffer_exit(p->priv->buf);
#if ENABLE_DEBUG
	facq_log_write("Producer: Stopping the source",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
	if(!facq_source_stop(src,&src_stop_err)){
		if(src_stop_err){
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",src_stop_err->message);
			g_clear_error(&src_stop_err);
		}
		else
			facq_log_write("Unknown error stopping the source",FACQ_LOG_MSG_TYPE_ERROR);
	}
	
	total_seconds = g_timer_elapsed(timer,NULL);
	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,
			"Read ""%"G_GSIZE_FORMAT" bytes in %f seconds, using %u bytes per sample",
			absolute_bytes_read,
			total_seconds,
			stmd->bps);
	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,
			"Average data input equals %f samples per second",
			(absolute_bytes_read/stmd->bps)/total_seconds);
	g_timer_destroy(timer);

#if ENABLE_DEBUG
	facq_log_write("Producer: exit",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
	if(conv && src_chunk)
		facq_chunk_free(src_chunk);

	return NULL;
}

static gboolean consumer_write_fun(FacqPipeline *p,FacqSink *sink,const FacqStreamData *stmd,FacqChunk *chunk)
{
	GError *err = NULL;

	switch(facq_sink_write(sink,stmd,chunk,&err)){
	case G_IO_STATUS_NORMAL: 
#if ENABLE_DEBUG
		facq_log_write("C G_IO_STATUS_NORMAL",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
		return TRUE;
	case G_IO_STATUS_AGAIN: 
#if ENABLE_DEBUG
		facq_log_write("C G_IO_STATUS_AGAIN",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
		return TRUE;
	case G_IO_STATUS_EOF:
#if ENABLE_DEBUG
		facq_log_write("C G_IO_STATUS_EOF",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
		facq_log_write(EOF_WRITING_SINK,FACQ_LOG_MSG_TYPE_INFO);
		facq_pipeline_stop_condition(p,EOF_WRITING_SINK);
		return FALSE;
	case G_IO_STATUS_ERROR:
#if ENABLE_DEBUG
		facq_log_write("C G_IO_STATUS_ERROR",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
		if(err){
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",err->message);
			g_clear_error(&err);
		}
		else {
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s","Unknown error writing to the sink");
		}
		facq_pipeline_error_condition(p,ERROR_WRITING_SINK);
		return FALSE;
	default:
		return FALSE;
	}
}

static gboolean consumer_oplist_do_fun(FacqPipeline *p,const FacqStreamData *stmd,FacqChunk *chunk,FacqOperationList *oplist)
{
	GError *err = NULL;

	if(!facq_operation_list_do(oplist,chunk,stmd,&err)){
		if(err){
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
					 "Operation error: %s",
					 err->message);
			g_clear_error(&err);
                }
		else {
                        facq_log_write("Unknown error in operation",
                                        FACQ_LOG_MSG_TYPE_ERROR);
                }
		facq_pipeline_error_condition(p,ERROR_OPERATION_DO);
		return FALSE;
        }
        return TRUE;
}

static gboolean consumer_dispatch_chunk(FacqPipeline *p,const FacqStreamData *stmd,FacqOperationList *oplist,FacqSink *sink,FacqChunk *chunk,gsize *absolute_bytes_written)
{
	gint poll_ret = 0;
	guint poll_retries = 0;

#if ENABLE_DEBUG
	facq_log_write("Consumer: processing chunk",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
	if(!consumer_oplist_do_fun(p,stmd,chunk,oplist))
		return TRUE;
#if ENABLE_DEBUG
	facq_log_write("Consumer: polling the sink",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
	while(poll_retries < 3){
		poll_ret = facq_sink_poll(sink,stmd);
#if ENABLE_DEBUG
		facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,"Consumer: poll=%d",poll_ret);
#endif
		if(poll_ret < 0){
#if ENABLE_DEBUG
			facq_log_write("Consumer: error polling the sink",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
			facq_pipeline_error_condition(p,ERROR_POLLING_SINK);
			return TRUE;
		}
		else if(poll_ret > 0){
#if ENABLE_DEBUG
			facq_log_write("Consumer: sink ready",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
			if(!consumer_write_fun(p,sink,stmd,chunk)){
#if ENABLE_DEBUG
				facq_log_write("Consumer: error writing data",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
				facq_chunk_free(chunk);
				return TRUE;
			}
#if ENABLE_DEBUG
			facq_log_write("Consumer: recycling chunk",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
			*absolute_bytes_written += facq_chunk_get_used_bytes(chunk);
			facq_buffer_recycle(p->priv->buf,chunk);
			return FALSE;
		}
		else {
#if ENABLE_DEBUG
			facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
					"Consumer: Increasing retries %u",poll_retries);
#endif
			poll_retries++;
		}
	}
	if(poll_retries == 3){
		facq_pipeline_error_condition(p,ERROR_POLLING_SINK);
		facq_log_write("Error max retries reached while polling sink",FACQ_LOG_MSG_TYPE_ERROR);
		return TRUE;
	}
	return TRUE;
}

static void consumer_end_fun(FacqPipeline *p,const FacqStreamData *stmd,FacqOperationList *oplist,FacqSink *sink,gsize *absolute_bytes_written)
{
	FacqChunk *chunk = NULL;
	gboolean err = FALSE;

	while( (chunk = facq_buffer_try_pop(p->priv->buf)) != NULL){
		err = consumer_dispatch_chunk(p,stmd,oplist,sink,chunk,absolute_bytes_written);
		if(err)
			return;
	}
}

static gpointer consumer_fun(gpointer pipeline)
{
	FacqPipeline *p = FACQ_PIPELINE(pipeline);
	const FacqStreamData *stmd = NULL;
	FacqOperationList *oplist = NULL;
	FacqSink *sink = NULL;
	FacqChunk *chunk = NULL;
	gboolean err = FALSE;
	GError *local_err = NULL;
	GTimer *timer = NULL;
	gsize absolute_bytes_written = 0;
	gdouble total_seconds = 0, timeout = 0;

	g_return_val_if_fail(FACQ_IS_PIPELINE(p),NULL);

	timer = g_timer_new();
	stmd = facq_source_get_stream_data(p->priv->src);
	sink = p->priv->sink;
	oplist = p->priv->oplist;

	if(stmd->period > 1){
		timeout = stmd->period;
	}
	else
		timeout = 1;

	while(!facq_buffer_get_exit(p->priv->buf)){
		chunk = facq_buffer_timeout_pop(p->priv->buf,timeout);
		if(chunk){
#if ENABLE_DEBUG
			facq_log_write("Consumer: Chunk received, processing...",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
			err = consumer_dispatch_chunk(p,stmd,oplist,sink,chunk,&absolute_bytes_written);
			chunk = NULL;
			if(err)
				break;
		}
	}
	if(!err){
#if ENABLE_DEBUG
		facq_log_write("Consumer: processing remaining data...",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
		consumer_end_fun(p,stmd,oplist,sink,&absolute_bytes_written);
	}

	g_timer_stop(timer);
	facq_buffer_exit(p->priv->buf);

#if ENABLE_DEBUG
	facq_log_write("Consumer: Stopping operation list",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
	if(!facq_operation_list_stop(oplist,stmd,&local_err)){
		if(local_err){
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
					"Error stopping operation list: %s",local_err->message);
			g_clear_error(&local_err);
		}
		else
			facq_log_write("Unknown error stopping operation list",
								FACQ_LOG_MSG_TYPE_ERROR);
	}

#if ENABLE_DEBUG
	facq_log_write("Consumer: Stopping sink",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
	if(!facq_sink_stop(sink,stmd,&local_err)){
		if(local_err){
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
						"Error while stopping the sink: %s",
							local_err->message);
			g_clear_error(&local_err);
		}
		else
			facq_log_write("Unknown error while stopping the sink",
						FACQ_LOG_MSG_TYPE_ERROR);
        }

	total_seconds = g_timer_elapsed(timer,NULL);
	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,
			"Wrote ""%"G_GSIZE_FORMAT" bytes in %f seconds, using 8 bytes per sample",
			absolute_bytes_written,
			total_seconds);
	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,
			"Average data output equals %f samples per second",
			(absolute_bytes_written/sizeof(gdouble))/total_seconds);
	g_timer_destroy(timer);
#if ENABLE_DEBUG
	facq_log_write("Consumer: exit",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
	return NULL;
}

/*****---- GObject magic ----*****/
static void facq_pipeline_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqPipeline *p = FACQ_PIPELINE(self);

	switch(property_id){
	case PROP_RING_SIZE: g_value_set_uint(value,p->priv->ring_chunks);
	break;
	case PROP_CHUNK_SIZE: g_value_set_uint(value,p->priv->chunk_size);
	break;
	case PROP_MONITOR: g_value_set_pointer(value,p->priv->mon);
	break;
	case PROP_SOURCE: g_value_set_pointer(value,p->priv->src);
	break;
	case PROP_OPERATION_LIST: g_value_set_pointer(value,p->priv->oplist);
	break;
	case PROP_SINK: g_value_set_pointer(value,p->priv->sink);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (p, property_id, pspec);
	}
}

static void facq_pipeline_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqPipeline *p = FACQ_PIPELINE(self);

	switch(property_id){
	case PROP_RING_SIZE: p->priv->ring_chunks = g_value_get_uint(value);
	break;
	case PROP_CHUNK_SIZE: p->priv->chunk_size = g_value_get_uint(value);
	break;
	case PROP_MONITOR: p->priv->mon = g_value_get_pointer(value);
	break;
	case PROP_SOURCE: p->priv->src = g_value_get_pointer(value);
	break;
	case PROP_OPERATION_LIST: p->priv->oplist = g_value_get_pointer(value);
	break;
	case PROP_SINK: p->priv->sink = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (p, property_id, pspec);
	}
}

static void facq_pipeline_finalize(GObject *self)
{
	FacqPipeline *p = FACQ_PIPELINE(self);

	facq_buffer_free(p->priv->buf);

	G_OBJECT_CLASS(facq_pipeline_parent_class)->finalize(self);
}

static void facq_pipeline_constructed(GObject *self)
{
	FacqPipeline *p = FACQ_PIPELINE(self);

	g_clear_error(&p->priv->construct_error);

	p->priv->buf = 
		facq_buffer_new(p->priv->ring_chunks,
				p->priv->chunk_size,
				&p->priv->construct_error);
}

static void facq_pipeline_class_init(FacqPipelineClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqPipelinePrivate));

	object_class->set_property = facq_pipeline_set_property;
	object_class->get_property = facq_pipeline_get_property;
	object_class->finalize = facq_pipeline_finalize;
	object_class->constructed = facq_pipeline_constructed;

	g_object_class_install_property(object_class,PROP_RING_SIZE,
					g_param_spec_uint("ring-size",
							"Ring size",
							"The number of chunks that the ring buffer can store",
							1,
							G_MAXUINT,
							1,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_CHUNK_SIZE,
					g_param_spec_uint("chunk-size",
							"Chunk size",
							"The size of a chunk",
							1,
							G_MAXUINT,
							1,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_MONITOR,
					g_param_spec_pointer("monitor",
							     "Monitor",
							     "A FacqPipelineMonitor to push error/stop messages",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_SOURCE,
					g_param_spec_pointer("source",
							     "Source",
							     "The source used by the pipeline",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_OPERATION_LIST,
					g_param_spec_pointer("oplist",
							     "Operation list",
							     "The operation list used by the pipeline",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_SINK,
					g_param_spec_pointer("sink",
							     "Sink",
							     "The sink used by the pipeline",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));
}

static void facq_pipeline_init(FacqPipeline *p)
{
	p->priv = G_TYPE_INSTANCE_GET_PRIVATE(p,FACQ_TYPE_PIPELINE,FacqPipelinePrivate);
	p->priv->construct_error = NULL;
	p->priv->buf = NULL;
	p->priv->producer = NULL;
	p->priv->consumer = NULL;
	p->priv->ring_chunks = 0;
	p->priv->chunk_size = 0;
	p->priv->src = NULL;
	p->priv->oplist = NULL;
	p->priv->sink = NULL;
}

static void facq_pipeline_initable_iface_init(GInitableIface *iface)
{
	iface->init = facq_pipeline_initable_init;
}

static gboolean facq_pipeline_initable_init(GInitable *initable,GCancellable *cancellable,GError  **error)
{
	FacqPipeline *p;
	
	g_return_val_if_fail(FACQ_IS_PIPELINE(initable),FALSE);
	p = FACQ_PIPELINE(initable);
	if(cancellable != NULL){
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                           "Cancellable initialization not supported");
      		return FALSE;
    	}
	if(p->priv->construct_error){
      		if(error)
        		*error = g_error_copy(p->priv->construct_error);
      		return FALSE;
    	}
  	return TRUE;
}

/*****--- public methods ---*****/
/**
 * facq_pipeline_new:
 * @chunk_size: The number of bytes of a chunk.
 * @ring_chunks: The size of the ring buffer (in number of chunks).
 * @src: A #FacqSource object.
 * @oplist: A #FacqOperationList object (It can be empty, but not %NULL).
 * @sink: A #FacqSink object.
 * @mon: A #FacqPipelineMonitor object.
 * @err: (allow-none): A #GError.
 *
 * Creates a new #FacqPipeline object.
 *
 * Returns: a new #FacqPipeline object, or %NULL in case of error.
 * In case of error @err is set.
 */
FacqPipeline *facq_pipeline_new(guint chunk_size,guint ring_chunks,FacqSource *src,FacqOperationList *oplist,FacqSink *sink,FacqPipelineMonitor *mon,GError **err)
{
	return FACQ_PIPELINE(
			 g_initable_new(FACQ_TYPE_PIPELINE,
				        NULL,err,
				       "ring-size",ring_chunks,
				       "chunk-size",chunk_size,
				       "monitor",mon,
				       "source",src,
				       "oplist",oplist,
				       "sink",sink,
					NULL)
			 );
}

/**
 * facq_pipeline_start:
 * @p: a #FacqPipeline object.
 * @err: A #GError, it will be set in case of error, if not %NULL.
 *
 * This function starts the pipeline, starting the operation list, the source 
 * and the sink and creating the needed threads for inner working.
 *
 * This function uses #FacqLog to write info about the starting process, you
 * should check the log in case of errors, if you want more detailed info.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */ 
gboolean facq_pipeline_start(FacqPipeline *p,GError **err)
{
	GError *local_err = NULL;
	const FacqStreamData *stmd;

	g_return_val_if_fail(FACQ_IS_PIPELINE(p),FALSE);

	facq_log_write("Pipeline start called, launching!",FACQ_LOG_MSG_TYPE_INFO);

	facq_log_write("Getting the stream data from the source",FACQ_LOG_MSG_TYPE_INFO);
	stmd = facq_source_get_stream_data(p->priv->src);
	if(!stmd){
		facq_log_write("Stream data not available",
					FACQ_LOG_MSG_TYPE_ERROR);
		g_set_error(&local_err,FACQ_PIPELINE_ERROR,
				FACQ_PIPELINE_ERROR_FAILED,"Stream data not available");
		goto error;
	}

#if ENABLE_DEBUG
	facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
			"Stream data: bps=%u period=%.9f n_channels=%u",
			stmd->bps,stmd->period,stmd->n_channels);
#endif

	/* If a operation fails in the start operation, the process will be
	 * interrupted and the previous operations started, if any, will be
	 * stopped by this function */
	facq_log_write("Starting the operation list",FACQ_LOG_MSG_TYPE_INFO);
	if(!facq_operation_list_start(p->priv->oplist,stmd,&local_err))
		goto error;

	/* Start the sink, if the start fails we have to stop the other
	 * operations */
	facq_log_write("Starting the sink",FACQ_LOG_MSG_TYPE_INFO);
	if(!facq_sink_start(p->priv->sink,stmd,&local_err)){
		if(local_err){
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
						"Error starting the sink: %s",
								local_err->message);
		}
		else {
			facq_log_write("Unknown error starting the sink",
							FACQ_LOG_MSG_TYPE_ERROR);
			g_set_error_literal(&local_err,FACQ_PIPELINE_ERROR,
					FACQ_PIPELINE_ERROR_FAILED,"Unknown error starting the sink");
		}
		goto error;
	}

	facq_log_write("Starting the source",FACQ_LOG_MSG_TYPE_INFO);
	if(!facq_source_start(p->priv->src,&local_err)){
		if(local_err){
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
						"Error starting the source: %s",
								local_err->message);
		}
		else {
			facq_log_write("Unknown error starting the source",FACQ_LOG_MSG_TYPE_ERROR);
			g_set_error_literal(&local_err,FACQ_PIPELINE_ERROR,
					FACQ_PIPELINE_ERROR_FAILED,"Unknown error starting the source");
		}
		goto error;
	}
	
	facq_log_write("Launching producer thread",FACQ_LOG_MSG_TYPE_INFO);
	p->priv->producer = g_thread_try_new("prod",
					     (GThreadFunc)producer_fun,
					     (gpointer)p,
					     &local_err);
	if(!(p->priv->producer)){
		if(local_err){
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
						"Error starting the producer thread: %s",
							local_err->message);
		}
		else {
			facq_log_write("Unknown error starting the producer thread",
						FACQ_LOG_MSG_TYPE_ERROR);
			g_set_error_literal(&local_err,FACQ_PIPELINE_ERROR,
					FACQ_PIPELINE_ERROR_FAILED,"Unknown error starting thread");
		}
		goto error;
	}

	facq_log_write("Launching consumer thread",FACQ_LOG_MSG_TYPE_INFO);
	p->priv->consumer = g_thread_try_new("cons",
					     (GThreadFunc)consumer_fun,
					     (gpointer)p,
					     &local_err);
	if(!(p->priv->consumer)){
		if(local_err){
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
						"Error starting the consumer thread: %s",
							local_err->message);
		}
		else {
			facq_log_write("Unknown error starting the consumer thread",
						FACQ_LOG_MSG_TYPE_ERROR);
			g_set_error_literal(&local_err,FACQ_PIPELINE_ERROR,
					FACQ_PIPELINE_ERROR_FAILED,"Unknown error starting thread");
		}	
		goto error;
	}
	return TRUE;

	error:
	/* stop threads from the main thread */
	facq_pipeline_stop(p);

	/* ensure that operations, source and sink are stopped */
	facq_pipeline_start_cleanup(p,stmd);

	facq_log_write(ERROR_PIPELINE_START,FACQ_LOG_MSG_TYPE_ERROR);
	if(local_err){
		if(err)
			g_propagate_error(err,local_err);
	}
	else {
		if(err){
			g_set_error_literal(&local_err,FACQ_PIPELINE_ERROR,
				FACQ_PIPELINE_ERROR_FAILED,ERROR_PIPELINE_START);
			g_propagate_error(err,local_err);
		}
	}
	return FALSE;
}

/**
 * facq_pipeline_stop:
 * @p: A #FacqPipeline object.
 *
 * Stops the pipeline running threads.
 * The function doesn't return the control until all the threads are
 * stopped, so it can take some time to return the control to the caller.
 *
 * Before exiting the consumer thread will stop the #FacqSource and the consumer
 * thread will stop the #FacqOperationList and the #FacqSink.
 */
void facq_pipeline_stop(FacqPipeline *p)
{
	GThread *this_thread = NULL;

	g_return_if_fail(FACQ_IS_PIPELINE(p));

	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,"%s",
			"Stopping pipeline this could take a while");

	this_thread = g_thread_self();

	facq_buffer_exit(p->priv->buf);
	if(p->priv->producer && p->priv->producer != this_thread)
		g_thread_join(p->priv->producer);
	if(p->priv->consumer && p->priv->consumer != this_thread)
		g_thread_join(p->priv->consumer);

	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,"%s","Pipeline stopped");
}

/**
 * facq_pipeline_free:
 * @p: A #FacqPipeline object.
 *
 * Destroys the #FacqPipeline object, @p.
 *
 * <para>
 * <note>
 * Before destroying a #FacqPipeline object you must stop it with
 * facq_pipeline_stop(), else bad things will happen.
 * </note>
 * </para>
 */
void facq_pipeline_free(FacqPipeline *p)
{
	g_return_if_fail(FACQ_IS_PIPELINE(p));
	g_object_unref(G_OBJECT(p));
}

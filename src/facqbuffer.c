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
#include "facqglibcompat.h"
#include "facqchunk.h"
#include "facqbuffer.h"

#define FACQ_BUFFER_DEF_SIZE 32
#define FACQ_BUFFER_DEF_CHUNK_SIZE 1

/**
 * SECTION:facqbuffer
 * @include:facqbuffer.h
 * @short_description: a ring buffer implementation
 * @see_also: #GInitable
 *
 * A #FacqBuffer provides a generic ring buffer that can work with any data type.
 * This buffer tries to implement a multithreaded producer-consumer pattern, 
 * hiding all the complex details with a simple interface.
 *
 * If you want to use it, first you have to create a new buffer with,
 * facq_buffer_new(), after that you can use facq_buffer_push() to
 * put data into the buffer and facq_buffer_pop() to retrieve data from it.
 * To stop using the buffer you should call first facq_buffer_exit().
 * To check if someone of your thread has called facq_buffer_exit(), you
 * can use facq_buffer_get_exit().
 * To free the buffer you should use facq_buffer_free().
 *
 * Also check the more advanced functions facq_buffer_try_pop(),
 * facq_buffer_timeout_pop(), facq_buffer_recycle(), facq_buffer_get_recycled(),
 * and facq_buffer_try_get_recycled().
 *
 * The expected behavior of the user is to create two threads, one of the
 * threads will be the producer and the other the consumer, the producer
 * will put the data into the buffer with facq_buffer_push() and the consumer
 * will retrieve the data from the buffer with facq_buffer_pop().
 *
 */

/**
 * FacqBuffer:
 * 
 * Contains all the #FacqBuffer private data.
 */

/**
 * FacqBufferClass:
 *
 * Class for the #FacqBuffer objects.
 */

/**
 * FacqBufferError:
 * @FACQ_BUFFER_ERROR_FAILED: Some error happened in the #FacqBuffer.
 *
 * Enum containinig all the possible error values for #FacqBuffer.
 */

static void facq_buffer_initable_iface_init(GInitableIface  *iface);
static gboolean facq_buffer_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);

G_DEFINE_TYPE_WITH_CODE(FacqBuffer,facq_buffer,G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_buffer_initable_iface_init));

GQuark facq_buffer_error_quark(void)
{
        return g_quark_from_static_string("facq-buffer-error-quark");
}

enum {
	PROP_0,
	PROP_MAX_CHUNKS,
	PROP_CHUNK_SIZE,
};

struct _FacqBufferPrivate {
	guint max_chunks;
	guint chunk_size;
	gboolean exit;
#if GLIB_MINOR_VERSION >= 32
	GMutex exit_mutex;
#else
	GMutex *exit_mutex;
#endif
	GAsyncQueue *q;
	GAsyncQueue *t;
	GError *construct_error;
};

/* GObject magic */
static void facq_buffer_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqBuffer *buf = FACQ_BUFFER(self);

	switch(property_id){
	case PROP_MAX_CHUNKS: g_value_set_uint(value,buf->priv->max_chunks);
	break;
	case PROP_CHUNK_SIZE: g_value_set_uint(value,buf->priv->chunk_size);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (buf, property_id, pspec);
	}
}

static void facq_buffer_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqBuffer *buf = FACQ_BUFFER(self);

	switch(property_id){
	case PROP_MAX_CHUNKS: buf->priv->max_chunks = g_value_get_uint(value);
	break;
	case PROP_CHUNK_SIZE: buf->priv->chunk_size = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (buf, property_id, pspec);
	}
}

static void facq_buffer_finalize(GObject *self)
{
	FacqBuffer *buf = FACQ_BUFFER(self);

	if(buf->priv->q)
		g_async_queue_unref(buf->priv->q);
	if(buf->priv->t)
		g_async_queue_unref(buf->priv->t);

#if GLIB_MINOR_VERSION >= 32
	g_mutex_clear(&buf->priv->exit_mutex);
#else
	g_mutex_free(buf->priv->exit_mutex);
#endif

	if(G_OBJECT_CLASS(facq_buffer_parent_class)->finalize)
    		(*G_OBJECT_CLASS(facq_buffer_parent_class)->finalize)(self);
}

static void facq_buffer_constructed(GObject *self)
{
	FacqBuffer *buf = FACQ_BUFFER(self);
	FacqChunk *chunk = NULL;
	guint i = 0;

#if GLIB_MINOR_VERSION >= 32
	g_mutex_init(&buf->priv->exit_mutex);
#else
	buf->priv->exit_mutex = g_mutex_new();
#endif

	buf->priv->q = g_async_queue_new_full((GDestroyNotify)facq_chunk_free);
	buf->priv->t = g_async_queue_new_full((GDestroyNotify)facq_chunk_free);

	for(i = 0;i < buf->priv->max_chunks;i++){
		chunk = facq_chunk_new(buf->priv->chunk_size,NULL);
		if(!chunk){
			g_set_error_literal(&buf->priv->construct_error,
					    FACQ_BUFFER_ERROR,
					    FACQ_BUFFER_ERROR_FAILED,
					    "Error allocating memory");
			return;
		}
		g_async_queue_push(buf->priv->t,chunk);
	}

	buf->priv->exit = FALSE;
}

static void facq_buffer_class_init(FacqBufferClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqBufferPrivate));

	object_class->set_property = facq_buffer_set_property;
	object_class->get_property = facq_buffer_get_property;
	object_class->finalize = facq_buffer_finalize;
	object_class->constructed = facq_buffer_constructed;

	/**
	 * FacqBuffer:max-chunks:
	 *
	 * The maximum number of chunks that can be at the same time
	 * in the buffer.
	 */
	g_object_class_install_property(object_class,PROP_MAX_CHUNKS,
					g_param_spec_uint("max-chunks",
							"Maximum chunk number",
							"The maximum number of chunks",
							1,
							G_MAXUINT,
							FACQ_BUFFER_DEF_SIZE,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));

	/**
	 * FacqBuffer:chunk-size:
	 *
	 * The number of bytes per chunk.
	 */
	g_object_class_install_property(object_class,PROP_CHUNK_SIZE,
					g_param_spec_uint("chunk-size",
							"Chunk size",
							"The size of a chunk",
							1,
							G_MAXUINT,
							FACQ_BUFFER_DEF_CHUNK_SIZE,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));
}

static void facq_buffer_init(FacqBuffer *buf)
{
	buf->priv = G_TYPE_INSTANCE_GET_PRIVATE(buf,FACQ_TYPE_BUFFER,FacqBufferPrivate);
	buf->priv->exit = FALSE;
	buf->priv->max_chunks = 0;
	buf->priv->chunk_size = 0;
}

/* GInitable interface */
static void facq_buffer_initable_iface_init(GInitableIface *iface)
{
	iface->init = facq_buffer_initable_init;
}

static gboolean facq_buffer_initable_init(GInitable *initable,GCancellable *cancellable,GError  **error)
{
	FacqBuffer *buf;

	g_return_val_if_fail(FACQ_IS_BUFFER(initable),FALSE);
	buf = FACQ_BUFFER(initable);
	if(cancellable != NULL){
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
			"Cancellable initialization not supported");
		return FALSE;
	}
	if(buf->priv->construct_error){
		if(error)
			*error = g_error_copy(buf->priv->construct_error);
		return FALSE;
	}
	return TRUE;
}
/* Public methods */

/**
 * facq_buffer_new:
 * @max_chunks: Desired maximum chunks.
 * @chunk_size: Desired chunk size in bytes, you can use 
 * facq_misc_period_to_chunk_size() to get a good value according 
 * to the sampling period.
 * @err: #GError for error reporting or %NULL to ignore.
 *
 * Create a new #FacqBuffer that can store up to @max_chunks chunks.
 * The size of each chunk is equal to @chunk_size.
 *
 * Returns: A #FacqBuffer or %NULL on error.
 */
FacqBuffer *facq_buffer_new(guint max_chunks,guint chunk_size,GError **err)
{
	return g_initable_new(FACQ_TYPE_BUFFER,
			    NULL,err,
			    "max-chunks",max_chunks,
			    "chunk-size",chunk_size,
			    NULL);
}

/**
 * facq_buffer_push:
 * @buf: A #FacqBuffer object.
 * @chunk: A pointer to a #FacqChunk containing a data chunk.
 *
 * Pushes @chunk to the @buf #FacqBuffer object.
 */
void facq_buffer_push(FacqBuffer *buf,FacqChunk *chunk)
{
#if ENABLE_DEBUG
	g_return_if_fail(FACQ_IS_BUFFER(buf));
	g_return_if_fail(FACQ_IS_ARRAY(chunk));
#endif
	g_async_queue_push(buf->priv->q,chunk);
}

/**
 * facq_buffer_pop:
 * @buf: A #FacqBuffer object.
 *
 * Retrieves a chunk from the buffer, giving space to one more chunk.
 * It will block until a #FacqChunk is ready to be returned.
 *
 * Return value: a #FacqChunk with data.
 */
FacqChunk *facq_buffer_pop(FacqBuffer *buf)
{
#if ENABLE_DEBUG
	g_return_val_if_fail(FACQ_IS_BUFFER(buf),NULL);
#endif
	return g_async_queue_pop(buf->priv->q);
}

/**
 * facq_buffer_try_pop:
 * @buf: A #FacqBuffer object.
 *
 * Tries to pop a #FacqChunk from the buffer without blocking.
 *
 * Returns: A previously stored #FacqChunk in the buffer, or %NULL
 * if any.
 */
FacqChunk *facq_buffer_try_pop(FacqBuffer *buf)
{
#if ENABLE_DEBUG
	g_return_val_if_fail(FACQ_IS_BUFFER(buf),NULL);
#endif
	return g_async_queue_try_pop(buf->priv->q);
}

/**
 * facq_buffer_timeout_pop:
 * @buf: A #FacqBuffer object.
 * @seconds: The maximum number of seconds to wait for a #FacqChunk.
 *
 * Waits the specified number of seconds to get a #FacqChunk, if the time elapses
 * without receiving any #FacqChunk, the function returns %NULL.
 *
 * Returns: %NULL or the direction of a stored #FacqChunk.
 */
FacqChunk *facq_buffer_timeout_pop(FacqBuffer *buf,gdouble seconds)
{
	guint64 timeout = 0;

#if ENABLE_DEBUG
	g_return_val_if_fail(FACQ_IS_BUFFER(buf),NULL);
#endif
	timeout = seconds*G_USEC_PER_SEC;
	return g_async_queue_timeout_pop(buf->priv->q,timeout);
}

/**
 * facq_buffer_recycle:
 * @buf: A #FacqBuffer object.
 * @chunk: A #FacqChunk object.
 *
 * Stores the #FacqChunk, @chunk, in the buffer, and clears it.
 * Later it can be reused using facq_buffer_get_recycled(), without
 * the need for allocating a new #FacqChunk.
 */
void facq_buffer_recycle(FacqBuffer *buf,FacqChunk *chunk)
{
#if ENABLE_DEBUG
	g_return_if_fail(FACQ_IS_BUFFER(buf));
#endif
	facq_chunk_clear(chunk);
	g_async_queue_push(buf->priv->t,chunk);
}

/**
 * facq_buffer_get_recycled:
 * @buf: A #FacqBuffer object.
 *
 * gets and empty #FacqChunk from the buffer.
 *
 * Returns: An empty #FacqChunk from the buffer, without the
 * need to reallocate a new one.
 */
FacqChunk *facq_buffer_get_recycled(FacqBuffer *buf)
{
#if ENABLE_DEBUG
	g_return_if_fail(FACQ_IS_BUFFER(buf));
#endif
	return g_async_queue_pop(buf->priv->t);
}

/**
 * facq_buffer_try_get_recycled:
 * @buf: A #FacqBuffer object.
 *
 * Tries to get an empty #FacqChunk from the buffer, without the 
 * need to reallocate a new one.
 *
 * Returns: %NULL or an empty #FacqChunk.
 */
FacqChunk *facq_buffer_try_get_recycled(FacqBuffer *buf)
{
#if ENABLE_DEBUG
	g_return_if_fail(FACQ_IS_BUFFER(buf));
#endif
	return g_async_queue_try_pop(buf->priv->t);
}

/**
 * facq_buffer_exit:
 * @buf: A #FacqBuffer Object.
 *
 * Call this function when you have finished using the buffer.
 * It will set an internal property, that you can check with
 * facq_buffer_get_exit(), that can be used by your threads
 * to check the status.
 */
void facq_buffer_exit(FacqBuffer *buf)
{
#if ENABLE_DEBUG
	g_return_if_fail(FACQ_IS_BUFFER(buf));
#endif

#if GLIB_MINOR_VERSION >= 32
	g_mutex_lock(&buf->priv->exit_mutex);
	buf->priv->exit = TRUE;
	g_mutex_unlock(&buf->priv->exit_mutex);
#else
	g_mutex_lock(buf->priv->exit_mutex);
	buf->priv->exit = TRUE;
	g_mutex_unlock(buf->priv->exit_mutex);
#endif
}

/**
 * facq_buffer_get_exit:
 * @buf: A #FacqBuffer object.
 *
 * Call this function from the producer and consumer thread to check
 * when the threads should stop doing his work.
 *
 * Returns: %TRUE if facq_buffer_exit() has been 
 * called on the object, %FALSE in other case.
 */
gboolean facq_buffer_get_exit(FacqBuffer *buf)
{
	gboolean ret = FALSE;

#if ENABLE_DEBUG
	g_return_val_if_fail(FACQ_IS_BUFFER(buf),TRUE);
#endif

#if GLIB_MINOR_VERSION >= 32
	g_mutex_lock(&buf->priv->exit_mutex);
	ret = buf->priv->exit;
	g_mutex_unlock(&buf->priv->exit_mutex);
#else
	g_mutex_lock(buf->priv->exit_mutex);
	ret = buf->priv->exit;
	g_mutex_unlock(buf->priv->exit_mutex);
#endif
	return ret;
}

/**
 * facq_buffer_free:
 * @buf: A #FacqBuffer Object.
 *
 * Destroys the #FacqBuffer Object, @buf.
 */
void facq_buffer_free(FacqBuffer *buf)
{
	g_return_if_fail(FACQ_IS_BUFFER(buf));
	g_object_unref(G_OBJECT(buf));
}

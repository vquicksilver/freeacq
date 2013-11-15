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

#ifdef G_OS_UNIX
#if GLIB_MINOR_VERSION >= 30
#include <glib-unix.h>
#endif
#endif

#include <gio/gio.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqresources.h"
#include "facqglibcompat.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqchunk.h"
#include "facqstreamdata.h"
#include "facqsink.h"
#include "facqsinknull.h"

/**
 * SECTION:facqsinknull
 * @title:FacqSinkNull
 * @short_description: Data sink that destroys incoming data.
 * @include:facqsinknull.h
 *
 * #FacqSinkNull provides a data sink that destroys all the incoming data
 * writting it to the so called null device, that is "NUL" in Windows systems
 * or /dev/null on Unix systems. You should use this type of sink when you
 * don't want to store the samples during an acquisition.
 *
 * To create a new #FacqSinkNull you can use facq_sink_null_new(), and to
 * destroy it you can use facq_sink_null_free(). #FacqSinkNull implements
 * the virtuals in #FacqSink, so if you want to use it without a #FacqStream
 * you must call first facq_sink_start(), and then you must call in an iterative
 * way facq_sink_null_poll() and facq_sink_null_write(). When you don't need
 * to write more data simply call facq_sink_stop() and facq_sink_free() to
 * destroy the object.
 *
 * #FacqSinkNull implements all the needed operations by #FacqSink so take a
 * look there if you need more details.
 *
 * facq_sink_null_key_constructor(), and
 * facq_sink_null_constructor() are used by the system to store the config
 * and to recreate #FacqSinkNull objects.
 * Note that in this case #FacqSinkNull doesn't need to store any property
 * so the facq_sink_null_to_file function is not implemented.
 * See facq_sink_to_file(), the #CIConstructor type and the #CIKeyConstructor
 * for more info.
 */

/**
 * FacqSinkNull:
 *
 * Contains the private details of the #FacqSinkNull objects.
 */

/**
 * FacqSinkNullClass:
 *
 * Class for the #FacqSinkNull objects.
 */

static void facq_sink_null_initable_iface_init(GInitableIface  *iface);
static gboolean facq_sink_null_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);

G_DEFINE_TYPE_WITH_CODE(FacqSinkNull,facq_sink_null,FACQ_TYPE_SINK,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_sink_null_initable_iface_init));

#ifdef G_OS_WIN32 
#define FACQ_SINK_NULL_DPATH "NUL"
#else 
#define FACQ_SINK_NULL_DPATH "/dev/null"
#endif

enum {
	PROP_0,
};

struct _FacqSinkNullPrivate {
	GPollFD *pfd;
	const gchar *path;
	GIOChannel *channel;
	GError *construct_error;
};

/*****--- GObject magic ---*****/
static void facq_sink_null_finalize(GObject *self)
{
	FacqSinkNull *sink = FACQ_SINK_NULL(self);

	g_clear_error(&sink->priv->construct_error);

	if(sink->priv->channel){
		g_io_channel_unref(sink->priv->channel);
	}

	if(sink->priv->pfd)
		g_free(sink->priv->pfd);
	
	G_OBJECT_CLASS (facq_sink_null_parent_class)->finalize (self);
}

static void facq_sink_null_constructed(GObject *self)
{
	FacqSinkNull *sink = FACQ_SINK_NULL(self);
	gint fd = -1;

	//Open the device
	sink->priv->channel = g_io_channel_new_file(FACQ_SINK_NULL_DPATH,
						    "w",
						    &sink->priv->construct_error);
	if(!sink->priv->channel)
		return;

	sink->priv->pfd = g_new0(GPollFD,1);

#ifdef G_OS_WIN32
	g_io_channel_win32_make_pollfd(sink->priv->channel,
				       G_IO_OUT,
				       sink->priv->pfd);
#else
	fd = g_io_channel_unix_get_fd(sink->priv->channel);
	if(!g_unix_set_fd_nonblocking(fd,
				TRUE,&sink->priv->construct_error))
		return;

	sink->priv->pfd->fd = fd;
	sink->priv->pfd->events = G_IO_OUT | G_IO_ERR;
#endif
	//Set io channel encoding to NULL (binary data)
	if(g_io_channel_set_encoding(sink->priv->channel,
				NULL,&sink->priv->construct_error) 
					!= G_IO_STATUS_NORMAL)
		return;

}

static void facq_sink_null_class_init(FacqSinkNullClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	FacqSinkClass *sink_class = FACQ_SINK_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FacqSinkNullPrivate));

	object_class->constructed = facq_sink_null_constructed;
	object_class->finalize = facq_sink_null_finalize;
	sink_class->sinkwrite = facq_sink_null_write;
	sink_class->sinkfree = facq_sink_null_free;
}

static void facq_sink_null_init(FacqSinkNull *sink)
{
	sink->priv = G_TYPE_INSTANCE_GET_PRIVATE(sink,FACQ_TYPE_SINK_NULL,FacqSinkNullPrivate);
	sink->priv->channel = NULL;
	sink->priv->pfd = NULL;
}

/*****--- GInitable implementation ---*****/
static void facq_sink_null_initable_iface_init(GInitableIface *iface)
{
	iface->init = facq_sink_null_initable_init;
}

static gboolean facq_sink_null_initable_init(GInitable *initable,GCancellable *cancellable,GError  **error)
{
	FacqSinkNull *sink = NULL;

	g_return_val_if_fail(FACQ_IS_SINK_NULL(initable),FALSE);
	sink = FACQ_SINK_NULL(initable);
	if(cancellable != NULL){
		g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                           "Cancellable initialization not supported");
      		return FALSE;
    	}
	if(sink->priv->construct_error){
		if (error)
        	*error = g_error_copy(sink->priv->construct_error);
      		return FALSE;
	}
	return TRUE;
}

/*****--- Public methods ---*****/
/**
 * facq_sink_null_key_constructor:
 * @group_name: A string with the group name for the #GKeyFile.
 * @key_file: A #GKeyFile object.
 * @err: (allow-none): A #GError it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqSinkNull object from a #GKeyFile and a @group_name.
 * This function is used by #FacqCatalog. See #CIKeyConstructor for more
 * details.
 *
 * Returns: A new #FacqSinkNull object or %NULL in case of error.
 */
gpointer facq_sink_null_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err)
{
	return facq_sink_null_new(err);
}

/**
 * facq_sink_null_constructor:
 * @user_input: It doesn't matter cause #FacqSinkNull doesn't use parameters.
 * @err: A #GError it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqSinkNull object that will destroy incoming data.
 *
 * This function is used by #FacqCatalog, for creating a #FacqSinkNull
 * with the parameters provided by the user in a #FacqDynDialog, take a look
 * at these other objects for more details, and to the #CIConstructor type.
 *
 * Returns: A new #FacqSinkNull object, or %NULL in case of error.
 */
gpointer facq_sink_null_constructor(const GPtrArray *user_input,GError **err)
{
	return facq_sink_null_new(err);
}

/**
 * facq_sink_null_new:
 * @error: A #GError it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqSinkNull object that will destroy incoming data.
 *
 * Returns: A new #FacqSinkNull object, or %NULL in case of error.
 */
FacqSinkNull *facq_sink_null_new(GError **error)
{
	return FACQ_SINK_NULL(g_initable_new(FACQ_TYPE_SINK_NULL,
					     NULL,
					     error,
					     "name",facq_resources_names_sink_null(),
					     "description",facq_resources_descs_sink_null(),
					     NULL)
				);
}

/*****--- Virtuals ---*****/
/**
 * facq_sink_null_poll:
 * @sink: A #FacqSinkNull object casted to #FacqSink.
 * @stmd: A #FacqStreamData object, with the relevant details of the stream.
 *
 * Polls the null device to check if data can be written without blocking,
 * as far as I know, it's always possible to write to this device without
 * blocking.
 *
 * Returns: %1 if successful, or %-1 in case of error.
 */
gint facq_sink_null_poll(FacqSink *sink,const FacqStreamData *stmd)
{
	gint ret = -1;
	FacqSinkNull *sinknull = NULL;

	g_return_val_if_fail(FACQ_IS_SINK_NULL(sink),-1);

	sinknull = FACQ_SINK_NULL(sink);

	ret = g_poll(sinknull->priv->pfd,1,200);
	if(ret){
		if(sinknull->priv->pfd->revents & G_IO_OUT)
			return 1;
		if(sinknull->priv->pfd->revents & G_IO_ERR)
			return -1;
	}
	return ret;
}
/* return -1 on error, 0 on timeout, 1 if ready to write */

/**
 * facq_sink_null_write:
 * @sink: A #FacqSinkNull casted to #FacqSink.
 * @stmd: A #FacqStreamData object with the relevant stream info.
 * @chunk: A #FacqChunk with the relevant data to be written to the sink.
 * @err: A #GError it will be set in case of error, if not %NULL.
 * 
 * Implements the facq_sink_write() function.
 * Writes data from the #FacqChunk, @chunk, to the sink.
 *
 * Returns: A #GIOStatus, if all goes fine it should be %G_IO_STATUS_NORMAL.
 */
GIOStatus facq_sink_null_write(FacqSink *sink,const FacqStreamData *stmd,FacqChunk *chunk,GError **err)
{
	FacqSinkNull *sinknull = FACQ_SINK_NULL(sink);
	gsize bytes_written = 0;

	return g_io_channel_write_chars(sinknull->priv->channel,
					chunk->data,
					facq_chunk_get_used_bytes(chunk),
					&bytes_written,
					err);
}

/**
 * facq_sink_null_free:
 * @sink: A #FacqSinkNull object casted to #FacqSink.
 *
 * Implements facq_sink_free() from #FacqSink.
 * Destroys a no longer needed #FacqSinkNull.
 */
void facq_sink_null_free(FacqSink *sink)
{
	g_return_if_fail(FACQ_IS_SINK(sink));
	g_object_unref(G_OBJECT(sink));
}

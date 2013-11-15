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
#include <glib.h>
#include <gio/gio.h>
#include <string.h>
#include "facqi18n.h"
#include "gdouble.h"
#include "facqchunk.h"

#define FACQ_CHUNK_DEF_SIZE 8

/**
 * SECTION:facqchunk
 * @include:facqchunk.h
 * @short_description: a fixed size chunk implementation
 * @see_also: #GInitable
 *
 * A #FacqChunk provides a generic chunk that can work with
 * any data type.
 *
 * You can create a new chunk with facq_chunk_new(), you can push
 * data to the chunk with facq_chunk_push(). You can check if the
 * data area of the chunk is full with facq_chunk_is_full().
 * To destroy the chunk facq_chunk_free() is provided, if you want
 * to clear the data area instead so you can reuse it you can use
 * facq_chunk_clear().
 *
 * To get the chunk_size after the chunk has been created you 
 * can use facq_chunk_get_chunk_size(), also is posible to obtain
 * the data area size in bytes using facq_chunk_get_data_size().
 *
 * Special functions for printing and converting to big endian the
 * data area are providad in the case you are using gdouble as 
 * data chunks, facq_chunk_data_double_print() prints the 
 * data to stderr and facq_chunk_data_double_to_be() converts the
 * data to big endian.
 */

/**
 * FacqChunk:
 * @data: a pointer to the data area.
 * @len: the number of chunks pushed to the chunk.
 *
 * Contains the public fields of a #FacqChunk.
 */

/**
 * FacqChunkClass:
 *
 * Class for the #FacqChunk objects.
 */

/**
 * FacqChunkError:
 * @FACQ_CHUNK_ERROR_FAILED: Some error happened in the #FacqChunk object.
 *
 * Enum values for errors in #FacqChunk.
 */

static void facq_chunk_initable_iface_init(GInitableIface  *iface);
static gboolean facq_chunk_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);

G_DEFINE_TYPE_WITH_CODE(FacqChunk,facq_chunk,G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_chunk_initable_iface_init));

GQuark facq_chunk_error_quark(void)
{
        return g_quark_from_static_string("facq-chunk-error-quark");
}

enum {
	PROP_0,
	PROP_CHUNK_SIZE,
};

struct _FacqChunkPrivate {
	GError *construct_error;
	gsize chunk_size;
	gsize used_bytes;
};

/*****--- GObject magic ---*****/
static void facq_chunk_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqChunk *chunk = FACQ_CHUNK(self);

	switch(property_id){
	case PROP_CHUNK_SIZE: g_value_set_uint(value,chunk->priv->chunk_size);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (chunk, property_id, pspec);
	}
}

static void facq_chunk_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqChunk *chunk = FACQ_CHUNK(self);

	switch(property_id){
	case PROP_CHUNK_SIZE: chunk->priv->chunk_size = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (chunk, property_id, pspec);
	}
}

static void facq_chunk_finalize(GObject *self)
{
	FacqChunk *chunk = FACQ_CHUNK(self);

	if(chunk->data)
		g_free(chunk->data);

	if(G_OBJECT_CLASS(facq_chunk_parent_class)->finalize)
    		(*G_OBJECT_CLASS(facq_chunk_parent_class)->finalize)(self);
}

static void facq_chunk_constructed(GObject *self)
{
	FacqChunk *chunk = FACQ_CHUNK(self);

	g_clear_error(&chunk->priv->construct_error);

	if(!chunk->data)
		chunk->data = g_try_malloc0(chunk->priv->chunk_size);
	if(!chunk->data)
		g_set_error_literal(&chunk->priv->construct_error,
			FACQ_CHUNK_ERROR,FACQ_CHUNK_ERROR_FAILED,
			_("Can't allocate the memory area"));
	chunk->len = chunk->priv->chunk_size;
	chunk->priv->used_bytes = 0;
}

static void facq_chunk_class_init(FacqChunkClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqChunkPrivate));

	object_class->set_property = facq_chunk_set_property;
	object_class->get_property = facq_chunk_get_property;
	object_class->finalize = facq_chunk_finalize;
	object_class->constructed = facq_chunk_constructed;

	g_object_class_install_property(object_class,PROP_CHUNK_SIZE,
					g_param_spec_uint("chunk-size",
							"Chunk size",
							"The size of a chunk",
							1,
							G_MAXUINT,
							FACQ_CHUNK_DEF_SIZE,
							G_PARAM_READWRITE |
							G_PARAM_CONSTRUCT_ONLY |
							G_PARAM_STATIC_STRINGS));
}

static void facq_chunk_init(FacqChunk *chunk)
{
	chunk->priv = G_TYPE_INSTANCE_GET_PRIVATE(chunk,FACQ_TYPE_CHUNK,FacqChunkPrivate);
	chunk->priv->construct_error = NULL;
	chunk->data = NULL;
	chunk->priv->chunk_size = 0;
	chunk->len = 0;
}

/*****--- GInitable interface ---*****/
static void facq_chunk_initable_iface_init(GInitableIface *iface)
{
	iface->init = facq_chunk_initable_init;
}

static gboolean facq_chunk_initable_init(GInitable *initable,GCancellable *cancellable,GError  **error)
{
	FacqChunk *chunk;
	
	g_return_val_if_fail(FACQ_IS_CHUNK(initable),FALSE);
	chunk = FACQ_CHUNK(initable);
	if(cancellable != NULL){
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                           "Cancellable initialization not supported");
      		return FALSE;
    	}
	if(chunk->priv->construct_error){
      		if(error)
        		*error = g_error_copy(chunk->priv->construct_error);
      		return FALSE;
    	}
  	return TRUE;
}

/* Public methods */

/**
 * facq_chunk_new:
 * @chunk_size: The size of each chunk.
 * @err: #GError for error reporting or %NULL to ignore.
 *
 * Create a new #FacqChunk that can store up to @chunk_size bytes.
 *
 * Returns: A #FacqChunk or %NULL on error.
 */
FacqChunk *facq_chunk_new(gsize chunk_size,GError **err)
{
	FacqChunk *chunk = FACQ_CHUNK(
			 g_initable_new(FACQ_TYPE_CHUNK,
				       	NULL,err,
				       "chunk-size",chunk_size,
					NULL)
			 ); 
	return chunk;
}

void facq_chunk_add_used_bytes(FacqChunk *chunk,gsize used_bytes)
{
	chunk->priv->used_bytes += used_bytes;
}

gsize facq_chunk_get_free_bytes(const FacqChunk *chunk)
{
	return chunk->priv->chunk_size-chunk->priv->used_bytes;
}

gsize facq_chunk_get_total_slices(const FacqChunk *chunk,gsize bps,guint n_channels)
{
	return chunk->priv->used_bytes/(bps*n_channels);
}

inline gpointer facq_chunk_get_n_slice(const FacqChunk *chunk,gsize bps,guint n_channels,guint n)
{
	if( (n*bps*n_channels) < chunk->priv->used_bytes){
		return &chunk->data[n*bps*n_channels];
	}
	else return NULL;
}

gsize facq_chunk_get_used_bytes(const FacqChunk *chunk)
{
	return chunk->priv->used_bytes;
}

/**
 * facq_chunk_write_pos:
 * @chunk: A #FacqChunk object.
 *
 * Returns: A pointer to the next not written byte in data area.
 */
gchar *facq_chunk_write_pos(const FacqChunk *chunk)
{
#if ENABLE_DEBUG
	g_return_val_if_fail(FACQ_IS_CHUNK(chunk),NULL);
#endif
	return &chunk->data[chunk->priv->used_bytes];
}

/**
 * facq_chunk_get_chunk_size:
 * @chunk: A #FacqChunk object.
 *
 * Returns the chunk_size supported by the chunk, @chunk.
 *
 * Returns: The chunk size.
 */
gsize facq_chunk_get_chunk_size(const FacqChunk *chunk)
{
	return chunk->priv->chunk_size;
}

/**
 * facq_chunk_data_double_to_be:
 * @chunk: A #FacqChunk object.
 *
 * Converts all the gdoubles in all the chunks in the data area
 * to big endian.
 */
void facq_chunk_data_double_to_be(FacqChunk *chunk)
{
	gdouble *data = NULL;
	guint i = 0;

#if ENABLE_DEBUG
	g_return_if_fail(FACQ_IS_CHUNK(chunk));
#endif

	data = (gdouble *)chunk->data;
	for(i = 0;i < (chunk->priv->used_bytes/sizeof(gdouble));i++)
		data[i] = GDOUBLE_TO_BE(data[i]);
	return;
}

void facq_chunk_data_double_print(FacqChunk *chunk)
{
	guint i = 0;
	gdouble *data = NULL;

#if ENABLE_DEBUG
	g_return_if_fail(FACQ_IS_CHUNK(chunk));
#endif

	data = (gdouble *)chunk->data;
	g_print("\n");
	for(i = 0;i < (chunk->priv->used_bytes/sizeof(gdouble));i++)
		g_print("%.9g ",data[i]);
	g_print("%u samples printed\n",i);
	g_print("\n");
}

void facq_chunk_clear(FacqChunk *chunk)
{
#if ENABLE_DEBUG
	g_return_if_fail(FACQ_IS_CHUNK(chunk));
#endif
	chunk->priv->used_bytes = 0;
}

/**
 * facq_chunk_free:
 * @chunk: A #FacqChunk object.
 *
 * Destroys the #FacqChunk, @chunk. 
 */
void facq_chunk_free(FacqChunk *chunk)
{
	g_return_if_fail(FACQ_IS_CHUNK(chunk));
	g_object_unref(G_OBJECT(chunk));
}

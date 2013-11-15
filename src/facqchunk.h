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
#ifndef _FREEACQ_CHUNK_H
#define _FREEACQ_CHUNK_H

G_BEGIN_DECLS

#define FACQ_CHUNK_ERROR facq_chunk_error_quark()

#define FACQ_TYPE_CHUNK (facq_chunk_get_type ())
#define FACQ_CHUNK(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_CHUNK, FacqChunk))
#define FACQ_CHUNK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_CHUNK, FacqChunkClass))
#define FACQ_IS_CHUNK(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_CHUNK))
#define FACQ_IS_CHUNK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_CHUNK))
#define FACQ_CHUNK_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_CHUNK, FacqChunkClass))

typedef struct _FacqChunk FacqChunk;
typedef struct _FacqChunkClass FacqChunkClass;
typedef struct _FacqChunkPrivate FacqChunkPrivate;

typedef enum {
	FACQ_CHUNK_ERROR_FAILED
} FacqChunkError;

struct _FacqChunk {
	/*< private >*/
	GObject parent_instance;
	FacqChunkPrivate *priv;
	/*< public >*/
	gchar *data;
	gsize len;
};

struct _FacqChunkClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_chunk_get_type(void) G_GNUC_CONST;

FacqChunk *facq_chunk_new(gsize chunk_size,GError **err);
gsize facq_chunk_get_used_bytes(const FacqChunk *chunk);
gchar *facq_chunk_write_pos(const FacqChunk *chunk);
gsize facq_chunk_get_total_slices(const FacqChunk *chunk,gsize bps,guint n_channels);
gpointer facq_chunk_get_n_slice(const FacqChunk *chunk,gsize bps,guint n_channels,guint n);
void facq_chunk_add_used_bytes(FacqChunk *chunk,gsize used_bytes);
gsize facq_chunk_get_free_bytes(const FacqChunk *chunk);
gsize facq_chunk_get_chunk_size(const FacqChunk *chunk);
void facq_chunk_data_double_to_be(FacqChunk *chunk);
void facq_chunk_data_double_print(FacqChunk *chunk);
void facq_chunk_clear(FacqChunk *chunk);
void facq_chunk_free(FacqChunk *chunk);

G_END_DECLS

#endif


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
#ifndef _FREEACQ_BUFFER_H_
#define _FREEACQ_BUFFER_H_

G_BEGIN_DECLS

#define FACQ_BUFFER_ERROR facq_buffer_error_quark()

#define FACQ_TYPE_BUFFER (facq_buffer_get_type ())
#define FACQ_BUFFER(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_BUFFER, FacqBuffer))
#define FACQ_BUFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_BUFFER, FacqBufferClass))
#define FACQ_IS_BUFFER(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_BUFFER))
#define FACQ_IS_BUFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_BUFFER))
#define FACQ_BUFFER_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_BUFFER, FacqBufferClass))

typedef struct _FacqBuffer FacqBuffer;
typedef struct _FacqBufferClass FacqBufferClass;
typedef struct _FacqBufferPrivate FacqBufferPrivate;

typedef enum {
	FACQ_BUFFER_ERROR_FAILED
} FacqBufferError;

struct _FacqBuffer {
	/*< private >*/
	GObject parent_instance;
	FacqBufferPrivate *priv;
};

struct _FacqBufferClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_buffer_get_type(void) G_GNUC_CONST;

FacqBuffer *facq_buffer_new(guint max_chunks,guint chunk_size,GError **err);
void facq_buffer_push(FacqBuffer *buf,FacqChunk *chunk);
FacqChunk *facq_buffer_pop(FacqBuffer *buf);
FacqChunk *facq_buffer_try_pop(FacqBuffer *buf);
FacqChunk *facq_buffer_timeout_pop(FacqBuffer *buf,gdouble seconds);
void facq_buffer_recycle(FacqBuffer *buf,FacqChunk *chunk);
FacqChunk *facq_buffer_get_recycled(FacqBuffer *buf);
FacqChunk *facq_buffer_try_get_recycled(FacqBuffer *buf);
void facq_buffer_exit(FacqBuffer *buf);
gboolean facq_buffer_get_exit(FacqBuffer *buf);
void facq_buffer_free(FacqBuffer *buf);

G_END_DECLS

#endif

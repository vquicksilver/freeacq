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
#ifndef _FREEACQ_STREAM_H_
#define _FREEACQ_STREAM_H_

G_BEGIN_DECLS

#define FACQ_STREAM_ERROR facq_stream_error_quark()

#define FACQ_TYPE_STREAM (facq_stream_get_type())
#define FACQ_STREAM(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_STREAM,FacqStream))
#define FACQ_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_STREAM, FacqStreamClass)
#define FACQ_IS_STREAM(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_STREAM))
#define FACQ_IS_STREAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_STREAM))
#define FACQ_STREAM_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_STREAM, FacqStreamClass))

typedef enum {
	FACQ_STREAM_ERROR_CLOSED,
	FACQ_STREAM_ERROR_FAILED
} FacqStreamError;

typedef struct _FacqStream FacqStream;
typedef struct _FacqStreamClass FacqStreamClass;
typedef struct _FacqStreamPrivate FacqStreamPrivate;

struct _FacqStream {
	/*< private >*/
	GObject parent_instance;
	FacqStreamPrivate *priv;
};

struct _FacqStreamClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_stream_get_type(void) G_GNUC_CONST;

FacqStream *facq_stream_new(const gchar *name,guint ring_chunks,FacqPipelineMonitorCb stop_cb,FacqPipelineMonitorCb error_cb,gpointer data);
gboolean facq_stream_is_closed(const FacqStream *stream);
void facq_stream_set_name(FacqStream *stream,const gchar *name);
gchar *facq_stream_get_name(const FacqStream *stream);
gboolean facq_stream_set_source(FacqStream *stream,FacqSource *src);
void facq_stream_remove_source(FacqStream *stream);
FacqSource *facq_stream_get_source(const FacqStream *stream);
gboolean facq_stream_set_sink(FacqStream *stream,FacqSink *sink);
FacqSink *facq_stream_get_sink(const FacqStream *stream);
void facq_stream_remove_sink(FacqStream *stream);
guint facq_stream_append_operation(FacqStream *stream,const FacqOperation *op);
FacqOperation *facq_stream_get_operation(FacqStream *stream,guint index);
guint facq_stream_remove_operation(FacqStream *stream);
guint facq_stream_get_operation_num(const FacqStream *stream);
void facq_stream_clear(FacqStream *stream);
void facq_stream_free(FacqStream *stream);

gboolean facq_stream_start(FacqStream *stream,GError **err);
void facq_stream_stop(FacqStream *stream);
gboolean facq_stream_save(FacqStream *stream,const gchar *filename,GError **err);
FacqStream *facq_stream_load(const gchar *filename,const FacqCatalog *cat,guint ring_chunks,FacqPipelineMonitorCb stop_cb,FacqPipelineMonitorCb error_cb,gpointer data,GError **err);

G_END_DECLS

#endif

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
#ifndef _FREEACQ_SINK_H
#define _FREEACQ_SINK_H

G_BEGIN_DECLS

#define FACQ_TYPE_SINK (facq_sink_get_type ())
#define FACQ_SINK(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_SINK, FacqSink))
#define FACQ_SINK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_SINK, FacqSinkClass))
#define FACQ_IS_SINK(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_SINK))
#define FACQ_IS_SINK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_SINK))
#define FACQ_SINK_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_SINK, FacqSinkClass))

typedef struct _FacqSink FacqSink;
typedef struct _FacqSinkClass FacqSinkClass;
typedef struct _FacqSinkPrivate FacqSinkPrivate;

struct _FacqSink {
	/*< private >*/
	GObject parent_instance;
	FacqSinkPrivate *priv;
};

struct _FacqSinkClass {
	/*< private >*/
	GObjectClass parent_class;
	/*< public >*/
	void (*sinksave)(FacqSink *sink,GKeyFile *file,const gchar *group);
	gboolean (*sinkstart)(FacqSink *sink,const FacqStreamData *stmd,GError **err);
	gint (*sinkpoll)(FacqSink *sink,const FacqStreamData *stmd);
	GIOStatus (*sinkwrite)(FacqSink *sink,const FacqStreamData *stmd,FacqChunk *chunk,GError **err);
	gboolean (*sinkstop)(FacqSink *sink,const FacqStreamData *stmd,GError **err);
	void (*sinkfree)(FacqSink *sink);
};

GType facq_sink_get_type(void) G_GNUC_CONST;

const gchar *facq_sink_get_name(const FacqSink *sink);
const gchar *facq_sink_get_description(const FacqSink *sink);
gboolean facq_sink_get_started(FacqSink *sink);
/* virtuals */
void facq_sink_to_file(FacqSink *sink,GKeyFile *file,const gchar *group);
gboolean facq_sink_start(FacqSink *sink,const FacqStreamData *stmd,GError **err);
gint facq_sink_poll(FacqSink *sink,const FacqStreamData *stmd);
GIOStatus facq_sink_write(FacqSink *sink,const FacqStreamData *stmd,FacqChunk *chunk,GError **err);
gboolean facq_sink_stop(FacqSink *sink,const FacqStreamData *stmd,GError **err);
void facq_sink_free(FacqSink *sink);

G_END_DECLS

#endif

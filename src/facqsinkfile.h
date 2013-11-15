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
#ifndef _FREEACQ_SINK_FILE_H_
#define _FREEACQ_SINK_FILE_H_

G_BEGIN_DECLS

#define FACQ_TYPE_SINK_FILE (facq_sink_file_get_type())
#define FACQ_SINK_FILE(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_SINK_FILE,FacqSinkFile))
#define FACQ_SINK_FILE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_SINK_FILE, FacqSinkFileClass)
#define FACQ_IS_SINK_FILE(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_SINK_FILE))
#define FACQ_IS_SINK_FILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_SINK_FILE))
#define FACQ_SINK_FILE_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_SINK_FILE, FacqSinkFileClass))

typedef struct _FacqSinkFile FacqSinkFile;
typedef struct _FacqSinkFileClass FacqSinkFileClass;
typedef struct _FacqSinkFilePrivate FacqSinkFilePrivate;

struct _FacqSinkFile {
	/*< private >*/
	FacqSink parent_instance;
	FacqSinkFilePrivate *priv;
};

struct _FacqSinkFileClass {
	/*< private >*/
	FacqSinkClass parent_class;
};

GType facq_sink_file_get_type(void) G_GNUC_CONST;

/* Public methods */
gpointer facq_sink_file_constructor(const GPtrArray *user_input,GError **err);
FacqSinkFile *facq_sink_file_new(const gchar *filename,GError **error);
/* virtuals */
void facq_sink_file_to_file(FacqSink *sink,GKeyFile *file,const gchar *group);
gpointer facq_sink_file_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err);
gboolean facq_sink_file_start(FacqSink *sink,const FacqStreamData *stmd,GError **err);
gint facq_sink_file_poll(FacqSink *sink,const FacqStreamData *stmd);
GIOStatus facq_sink_file_write(FacqSink *sink,const FacqStreamData *stmd,FacqChunk *chunk,GError **err);
gboolean facq_sink_file_stop(FacqSink *sink,const FacqStreamData *stmd,GError **err);
void facq_sink_file_free(FacqSink *sink);

G_END_DECLS

#endif

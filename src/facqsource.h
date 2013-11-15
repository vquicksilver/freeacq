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
#ifndef _FACQ_SOURCE_H_
#define _FACQ_SOURCE_H_

G_BEGIN_DECLS

#define FACQ_TYPE_SOURCE (facq_source_get_type ())
#define FACQ_SOURCE(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_SOURCE, FacqSource))
#define FACQ_SOURCE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_SOURCE, FacqSourceClass))
#define FACQ_IS_SOURCE(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_SOURCE))
#define FACQ_IS_SOURCE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_SOURCE))
#define FACQ_SOURCE_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_SOURCE, FacqSourceClass))

typedef struct _FacqSource FacqSource;
typedef struct _FacqSourceClass FacqSourceClass;
typedef struct _FacqSourcePrivate FacqSourcePrivate;

struct _FacqSource {
	/*< private >*/
	GObject parent_instance;
	FacqSourcePrivate *priv;
};

struct _FacqSourceClass {
	/*< private >*/
	GObjectClass parent_class;
	/*< public >*/
	void (*srcsave)(FacqSource *src,GKeyFile *file,const gchar *group);
	gboolean (*srcstart)(FacqSource *src,GError **err);
	gint (*srcpoll)(FacqSource *src);
	GIOStatus (*srcread)(FacqSource *src,gchar *buf,gsize count,gsize *bytes_read,GError **err);
	void (*srcconv)(FacqSource *src,gpointer ori,gdouble *dst,gsize samples);
	gboolean (*srcstop)(FacqSource *src,GError **err);
	void (*srcfree)(FacqSource *src);
};

GType facq_source_get_type(void) G_GNUC_CONST;

const gchar *facq_source_get_name(const FacqSource *src);
const gchar *facq_source_get_description(const FacqSource *src);
gboolean facq_source_get_started(const FacqSource *src);
const FacqStreamData *facq_source_get_stream_data(const FacqSource *src);
gboolean facq_source_needs_conv(FacqSource *src);

/* virtuals */
void facq_source_to_file(FacqSource *src,GKeyFile *file,const gchar *group);
gboolean facq_source_start(FacqSource *src,GError **err);
gint facq_source_poll(FacqSource *src);
GIOStatus facq_source_read(FacqSource *src,gchar *buf,gsize count,gsize *bytes_read,GError **err);
void facq_source_conv(FacqSource *src,gpointer ori,gdouble *dst,gsize samples);
gboolean facq_source_stop(FacqSource *src,GError **err);
void facq_source_free(FacqSource *src);

G_END_DECLS

#endif

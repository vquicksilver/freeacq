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
#ifndef _FREEACQ_SOURCE_NIDAQ_H_
#define _FREEACQ_SOURCE_NIDAQ_H_

#ifndef __GTK_DOC_IGNORE__

G_BEGIN_DECLS

#define FACQ_SOURCE_NIDAQ_ERROR facq_source_nidaq_error_quark()

#define FACQ_TYPE_SOURCE_NIDAQ (facq_source_nidaq_get_type())
#define FACQ_SOURCE_NIDAQ(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_SOURCE_NIDAQ,FacqSourceNidaq))
#define FACQ_SOURCE_NIDAQ_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_SOURCE_NIDAQ, FacqSourceNidaqClass))
#define FACQ_IS_SOURCE_NIDAQ(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_SOURCE_NIDAQ))
#define FACQ_IS_SOURCE_NIDAQ_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_SOURCE_NIDAQ))
#define FACQ_SOURCE_NIDAQ_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_SOURCE_NIDAQ, FacqSourceNidaqClass))

typedef enum {
	FACQ_SOURCE_NIDAQ_ERROR_FAILED
} FacqSourceNidaqError;

typedef struct _FacqSourceNidaq FacqSourceNidaq;
typedef struct _FacqSourceNidaqClass FacqSourceNidaqClass;
typedef struct _FacqSourceNidaqPrivate FacqSourceNidaqPrivate;

struct _FacqSourceNidaq {
	/*< private >*/
	FacqSource parent_instance;
	FacqSourceNidaqPrivate *priv;
};

struct _FacqSourceNidaqClass {
	/*< private >*/
	FacqSourceClass parent_class;
};

GType facq_source_nidaq_get_type(void) G_GNUC_CONST;

void facq_source_nidaq_to_file(FacqSource *src,GKeyFile *file,const gchar *group);
gpointer facq_source_nidaq_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err);
gpointer facq_source_nidaq_constructor(const GPtrArray *user_input,GError **err);
FacqSourceNidaq *facq_source_nidaq_new(const gchar *dev,FacqChanlist *chanlist,guint32 bufsize,gdouble period,gdouble max,gdouble min,gulong sleep_us,GError **error);
/* virtual implementations */
gboolean facq_source_nidaq_start(FacqSource *src,GError **err);
gint facq_source_nidaq_poll(FacqSource *src);
GIOStatus facq_source_nidaq_read(FacqSource *src,gchar *buf,gsize count,gsize *bytes_read,GError **err);
gboolean facq_source_nidaq_stop(FacqSource *src,GError **err);
void facq_source_nidaq_free(FacqSource *src);

G_END_DECLS

#endif

#endif

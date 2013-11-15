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
#if USE_COMEDI

#ifndef _FREEACQ_SOURCE_COMEDI_SYNC_H_
#define _FREEACQ_SOURCE_COMEDI_SYNC_H_

G_BEGIN_DECLS

#define FACQ_SOURCE_COMEDI_SYNC_ERROR facq_source_comedi_sync_error_quark()

#define FACQ_TYPE_SOURCE_COMEDI_SYNC (facq_source_comedi_sync_get_type())
#define FACQ_SOURCE_COMEDI_SYNC(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_SOURCE_COMEDI_SYNC,FacqSourceComediSync))
#define FACQ_SOURCE_COMEDI_SYNC_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_SOURCE_COMEDI_SYNC, FacqSourceComediSyncClass))
#define FACQ_IS_SOURCE_COMEDI_SYNC(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_SOURCE_COMEDI_SYNC))
#define FACQ_IS_SOURCE_COMEDI_SYNC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_SOURCE_COMEDI_SYNC))
#define FACQ_SOURCE_COMEDI_SYNC_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_SOURCE_COMEDI_SYNC, FacqSourceComediSyncClass))

typedef enum {
	FACQ_SOURCE_COMEDI_SYNC_ERROR_FAILED
} FacqSourceComediSyncError;

typedef struct _FacqSourceComediSync FacqSourceComediSync;
typedef struct _FacqSourceComediSyncClass FacqSourceComediSyncClass;
typedef struct _FacqSourceComediSyncPrivate FacqSourceComediSyncPrivate;

struct _FacqSourceComediSync {
	/*< private >*/
	FacqSource parent_instance;
	FacqSourceComediSyncPrivate *priv;
};

struct _FacqSourceComediSyncClass {
	/*< private >*/
	FacqSourceClass parent_class;
};

GType facq_source_comedi_sync_get_type(void) G_GNUC_CONST;

void facq_source_comedi_sync_to_file(FacqSource *src,GKeyFile *file,const gchar *group);
gpointer facq_source_comedi_sync_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err);
gpointer facq_source_comedi_sync_constructor(const GPtrArray *user_input,GError **err);
FacqSourceComediSync *facq_source_comedi_sync_new(guint index,guint subindex,gdouble period,FacqChanlist *chanlist,GError **err);
/* virtual implementations */
gboolean facq_source_comedi_sync_start(FacqSource *src,GError **err);
gint facq_source_comedi_sync_poll(FacqSource *src);
GIOStatus facq_source_comedi_sync_read(FacqSource *src,gchar *buf,gsize count,gsize *bytes_read,GError **err);
void facq_source_comedi_sync_conv(FacqSource *src,gpointer ori,gdouble *dst,gsize samples);
gboolean facq_source_comedi_sync_stop(FacqSource *src,GError **err);
void facq_source_comedi_sync_free(FacqSource *src);

G_END_DECLS

#endif

#endif //USE_COMEDI


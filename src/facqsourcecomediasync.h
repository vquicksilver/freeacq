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

#ifndef _FREEACQ_SOURCE_COMEDI_ASYNC_H_
#define _FREEACQ_SOURCE_COMEDI_ASYNC_H_

G_BEGIN_DECLS

#define FACQ_SOURCE_COMEDI_ASYNC_ERROR facq_source_comedi_async_error_quark()

#define FACQ_TYPE_SOURCE_COMEDI_ASYNC (facq_source_comedi_async_get_type())
#define FACQ_SOURCE_COMEDI_ASYNC(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_SOURCE_COMEDI_ASYNC,FacqSourceComediAsync))
#define FACQ_SOURCE_COMEDI_ASYNC_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_SOURCE_COMEDI_ASYNC, FacqSourceComediAsyncClass))
#define FACQ_IS_SOURCE_COMEDI_ASYNC(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_SOURCE_COMEDI_ASYNC))
#define FACQ_IS_SOURCE_COMEDI_ASYNC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_SOURCE_COMEDI_ASYNC))
#define FACQ_SOURCE_COMEDI_ASYNC_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_SOURCE_COMEDI_ASYNC, FacqSourceComediAsyncClass))

typedef enum {
	FACQ_SOURCE_COMEDI_ASYNC_ERROR_FAILED
} FacqSourceComediAsyncError;

typedef struct _FacqSourceComediAsync FacqSourceComediAsync;
typedef struct _FacqSourceComediAsyncClass FacqSourceComediAsyncClass;
typedef struct _FacqSourceComediAsyncPrivate FacqSourceComediAsyncPrivate;

struct _FacqSourceComediAsync {
	/*< private >*/
	FacqSource parent_instance;
	FacqSourceComediAsyncPrivate *priv;
};

struct _FacqSourceComediAsyncClass {
	/*< private >*/
	FacqSourceClass parent_class;
};

GType facq_source_comedi_async_get_type(void) G_GNUC_CONST;

void facq_source_comedi_async_to_file(FacqSource *src,GKeyFile *file,const gchar *group);
gpointer facq_source_comedi_async_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err);
gpointer facq_source_comedi_async_constructor(const GPtrArray *user_input,GError **err);
FacqSourceComediAsync *facq_source_comedi_async_new(guint index,guint subindex,guint flags,FacqChanlist *chanlist,gdouble period,GError **err);
/* virtual implementations */
gboolean facq_source_comedi_async_start(FacqSource *src,GError **err);
gint facq_source_comedi_async_poll(FacqSource *src);
GIOStatus facq_source_comedi_async_read(FacqSource *src,gchar *buf,gsize count,gsize *bytes_read,GError **err);
void facq_source_comedi_async_conv(FacqSource *src,gpointer ori,gdouble *dst,gsize samples);
gboolean facq_source_comedi_async_stop(FacqSource *src,GError **err);
void facq_source_comedi_async_free(FacqSource *src);

G_END_DECLS

#endif //_FREEACQ_SOURCE_COMEDI_ASYNC_H_

#endif //USE_COMEDI

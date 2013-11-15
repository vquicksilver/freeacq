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
#if USE_NIDAQ

#ifndef _FREEACQ_SINK_NIDAQ_H_
#define _FREEACQ_SINK_NIDAQ_H_

G_BEGIN_DECLS

#define FACQ_SINK_NIDAQ_ERROR facq_sink_nidaq_error_quark()

#define FACQ_TYPE_SINK_NIDAQ (facq_sink_nidaq_get_type())
#define FACQ_SINK_NIDAQ(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_SINK_NIDAQ,FacqSinkNidaq))
#define FACQ_SINK_NIDAQ_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_SINK_NIDAQ, FacqSinkNidaqClass)
#define FACQ_IS_SINK_NIDAQ(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_SINK_NIDAQ))
#define FACQ_IS_SINK_NIDAQ_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_SINK_NIDAQ))
#define FACQ_SINK_NIDAQ_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_SINK_NIDAQ, FacqSinkNidaqClass))

typedef struct _FacqSinkNidaq FacqSinkNidaq;
typedef struct _FacqSinkNidaqClass FacqSinkNidaqClass;
typedef struct _FacqSinkNidaqPrivate FacqSinkNidaqPrivate;

typedef enum {
	FACQ_SINK_NIDAQ_ERROR_FAILED
} FacqSinkNidaqError;

struct _FacqSinkNidaq {
	/*< private >*/
	FacqSink parent_instance;
	FacqSinkNidaqPrivate *priv;
};

struct _FacqSinkNidaqClass {
	/*< private >*/
	FacqSinkClass parent_class;
};

GType facq_sink_nidaq_get_type(void) G_GNUC_CONST;

/* Public methods */
gpointer facq_sink_nidaq_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err);
gpointer facq_sink_nidaq_constructor(const GPtrArray *user_input,GError **err);
FacqSinkNidaq *facq_sink_nidaq_new(const gchar *device,FacqChanlist *chanlist,gdouble max,gdouble min,GError **error);
/* virtuals */
void facq_sink_nidaq_to_file(FacqSink *sink,GKeyFile *file,const gchar *group);
gboolean facq_sink_nidaq_start(FacqSink *sink,const FacqStreamData *stmd,GError **err);
GIOStatus facq_sink_nidaq_write(FacqSink *sink,const FacqStreamData *stmd,FacqChunk *chunk,GError **err);
gboolean facq_sink_nidaq_stop(FacqSink *sink,const FacqStreamData *stmd,GError **err);
void facq_sink_nidaq_free(FacqSink *sink);

G_END_DECLS

#endif

#endif //USE_NIDAQ

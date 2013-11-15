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
#ifndef _FREEACQ_STREAM_DATA_H
#define _FREEACQ_STREAM_DATA_H

G_BEGIN_DECLS

#define FACQ_TYPE_STREAM_DATA (facq_stream_data_get_type ())
#define FACQ_STREAM_DATA(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_STREAM_DATA, FacqStreamData))
#define FACQ_STREAM_DATA_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_STREAM_DATA, FacqStreamDataClass))
#define FACQ_IS_STREAM_DATA(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_STREAM_DATA))
#define FACQ_IS_STREAM_DATA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_STREAM_DATA))
#define FACQ_STREAM_DATA_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_STREAM_DATA, FacqStreamDataClass))

typedef struct _FacqStreamData FacqStreamData;
typedef struct _FacqStreamDataClass FacqStreamDataClass;
typedef struct _FacqStreamDataPrivate FacqStreamDataPrivate;

struct _FacqStreamData {
	/*< private >*/
	GObject parent_instance;
	/*< public >*/
	guint bps;
	guint n_channels;
	gdouble period;
	FacqUnits *units;
	FacqChanlist *chanlist;
	gdouble *max;
	gdouble *min;
};

struct _FacqStreamDataClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_stream_data_get_type(void) G_GNUC_CONST;

FacqStreamData *facq_stream_data_new(guint bps,guint n_channels,gdouble period,FacqChanlist *chanlist,FacqUnits *units,gdouble *max,gdouble *min);
guint facq_stream_data_get_bps(const FacqStreamData *stmd);
guint facq_stream_data_get_n_channels(const FacqStreamData *stmd);
gdouble facq_stream_data_get_period(const FacqStreamData *stmd);
const FacqUnits *facq_stream_data_get_units(const FacqStreamData *stmd);
const FacqChanlist *facq_stream_data_get_chanlist(const FacqStreamData *stmd);
const gdouble *facq_stream_data_get_max(const FacqStreamData *stmd);
const gdouble *facq_stream_data_get_min(const FacqStreamData *stmd);
gboolean facq_stream_data_to_socket(const FacqStreamData *stmd,GSocket *socket,GError **err);
FacqStreamData *facq_stream_data_from_socket(GSocket *socket,GError **err);
void facq_stream_data_to_checksum(const FacqStreamData *stmd,GChecksum *sum);
void facq_stream_data_free(FacqStreamData *stmd);

G_END_DECLS

#endif

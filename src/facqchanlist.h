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
#ifndef _FREEACQ_CHANLIST_H_
#define _FREEACQ_CHANLIST_H_
#if HAVE_COMEDILIB_H
#include <comedilib.h>
#else
#include "facqnocomedi.h"
#endif

G_BEGIN_DECLS

/* Note that CHAN_INPUT = 0 and CHAN_OUTPUT=1 are related to the
 * enum comedi_io_direction, compatibility must be keep with
 * the comedi.h header */
/**
 * FacqChanDir:
 * @CHAN_INPUT: Channel will be read
 * @CHAN_OUTPUT: Channel will be written
 * @CHAN_BASE: Channel will be the base for Digital I/O.
 * @CHAN_START_EXT: Channel will trigger comedi start event.
 * @CHAN_BEGIN_EXT: Channel will trigger comedi begin event.
 * @CHAN_CONVERT_EXT: Channel will trigger comedi convert event.
 *
 * Enum values used for configure the direction of a channel.
 */
typedef enum chan_direction{
	CHAN_INPUT = 0, CHAN_OUTPUT = 1, CHAN_BASE = 3,
	CHAN_START_EXT, CHAN_BEGIN_EXT, CHAN_CONVERT_EXT
} FacqChanDir;

#define FACQ_TYPE_CHANLIST (facq_chanlist_get_type ())
#define FACQ_CHANLIST(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_CHANLIST, FacqChanlist))
#define FACQ_CHANLIST_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_CHANLIST, FacqChanlistClass))
#define FACQ_IS_CHANLIST(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_CHANLIST))
#define FACQ_IS_CHANLIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_CHANLIST))
#define FACQ_CHANLIST_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_CHANLIST, FacqChanlistClass))

typedef struct _FacqChanlist FacqChanlist;
typedef struct _FacqChanlistClass FacqChanlistClass;
typedef struct _FacqChanlistPrivate FacqChanlistPrivate;

struct _FacqChanlist {
	/*< private >*/
	GObject parent_instance;
	FacqChanlistPrivate *priv;
};

struct _FacqChanlistClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_chanlist_get_type(void) G_GNUC_CONST;

FacqChanlist *facq_chanlist_new(void);
void facq_chanlist_add_chan(FacqChanlist *chanlist,guint chan,guint rng,guint aref,guint flags,FacqChanDir dir);
void facq_chanlist_del_chan(FacqChanlist *chanlist);
guint facq_chanlist_get_length(const FacqChanlist *chanlist);
guint facq_chanlist_get_io_chans_n(const FacqChanlist *chanlist);
gboolean facq_chanlist_get_special_chan_index(const FacqChanlist *chanlist,guint *index,FacqChanDir dir);
guint facq_chanlist_get_chanspec(const FacqChanlist *chanlist,guint index);
guint facq_chanlist_get_io_chanspec(const FacqChanlist *chanlist,guint index);
FacqChanDir facq_chanlist_get_chan_direction(const FacqChanlist *chanlist,guint index);
FacqChanDir facq_chanlist_get_io_chan_direction(const FacqChanlist *chanlist,guint index);
guint *facq_chanlist_to_comedi_chanlist(const FacqChanlist *chanlist,guint *length);
gchar *facq_chanlist_to_nidaq_chanlist(const gchar *device,const FacqChanlist *chanlist,guint *length);
enum comedi_conversion_direction *facq_chanlist_get_comedi_conversion_direction_list(const FacqChanlist *chanlist,guint *length);
void facq_chanlist_chanspec_to_src_values(guint chanspec,guint *chan,guint *rng,guint *aref,guint *flags);
gboolean facq_chanlist_search_io_chan(const FacqChanlist *chanlist,guint channel,guint *index);
void facq_chanlist_to_key_file(const FacqChanlist *chanlist,GKeyFile *file,const gchar *group);
FacqChanlist *facq_chanlist_from_key_file(GKeyFile *file,const gchar *group_name,GError **err);
void facq_chanlist_free(FacqChanlist *chanlist);

G_END_DECLS

#endif

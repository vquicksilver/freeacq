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
#ifndef _FREEACQ_BPM_H_
#define _FREEACQ_BPM_H_

G_BEGIN_DECLS

#define FACQ_TYPE_BPM (facq_bpm_get_type ())
#define FACQ_BPM(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_BPM, FacqBPM))
#define FACQ_BPM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_BPM, FacqBPMClass))
#define FACQ_IS_BPM(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_BPM))
#define FACQ_IS_BPM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_BPM))
#define FACQ_BPM_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_BPM,FacqBPMClass))

typedef struct _FacqBPM FacqBPM;
typedef struct _FacqBPMClass FacqBPMClass;
typedef struct _FacqBPMPrivate FacqBPMPrivate;

struct _FacqBPM {
	/*< private >*/
        GObject parent_instance;
        FacqBPMPrivate *priv;
};

struct _FacqBPMClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_bpm_get_type(void) G_GNUC_CONST;

FacqBPM *facq_bpm_new(void);
void facq_bpm_setup(FacqBPM *bpm,guint n_channels,gdouble period);
const gdouble *facq_bpm_compute(FacqBPM *bpm,FacqChunk *chunk);
void facq_bpm_free(FacqBPM *bpm);

G_END_DECLS

#endif

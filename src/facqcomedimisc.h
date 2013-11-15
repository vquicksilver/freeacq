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

#ifndef _FREEACQ_COMEDI_MISC_H
#define _FREEACQ_COMEDI_MISC_H

G_BEGIN_DECLS
#define FACQ_COMEDI_MISC_ERROR facq_comedi_misc_error_quark()

typedef enum {
	FACQ_COMEDI_MISC_ERROR_FAILED
} FacqComediMiscError;

gboolean facq_comedi_misc_test_aref(guint subd_flags,guint aref);
gboolean facq_comedi_misc_test_channel_flags(guint subd_flags,guint flags);
comedi_cmd *facq_comedi_misc_cmd_new(guint subindex);
gboolean facq_comedi_misc_test_period(comedi_t *dev,guint subindex,guint n_channels,guint period,GError **err);
gboolean facq_comedi_misc_test_chanlist(comedi_t *dev,guint subindex,const FacqChanlist *chanlist,GError **err);
gboolean facq_comedi_misc_test_flags(comedi_t *dev,guint subindex,guint flags,GError **err);
gboolean facq_comedi_misc_test_calibrated(comedi_t *dev,GError **err);
void facq_comedi_misc_cmd_add_chanlist(comedi_cmd *cmd,const FacqChanlist *chanlist);
gint facq_comedi_misc_can_calibrate(comedi_t *dev,guint subindex,GError **err);
comedi_polynomial_t *facq_comedi_misc_get_polynomial(comedi_t *dev,guint subindex,const FacqChanlist *chanlist,GError **err);
FacqUnits *facq_comedi_misc_get_units(comedi_t *dev,guint subindex,const FacqChanlist *chanlist,GError **err);
gdouble *facq_comedi_misc_get_max(comedi_t *dev,guint subindex,const FacqChanlist *chanlist,GError **err);
gdouble *facq_comedi_misc_get_min(comedi_t *dev,guint subindex,const FacqChanlist *chanlist,GError **err);
guint facq_comedi_misc_get_bps(comedi_t *dev,guint subindex,GError **err);
gboolean facq_comedi_misc_can_poll(comedi_t *dev,GError **err);
G_END_DECLS

#endif

#endif //USE_COMEDI

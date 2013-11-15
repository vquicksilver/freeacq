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
#ifndef _FREEACQ_NIDAQMXBASE_MISC_H
#define _FREEACQ_NIDAQMXBASE_MISC_H

#if USE_NIDAQ

#if HAVE_NIDAQMXBASE_H

#define _PREFIX_(call) DAQmxBase##call
#include <NIDAQmxBase.h>

#elif HAVE_NIDAQMX_H

#include <NIDAQmx.h>
#define _PREFIX_(call) DAQmx##call

#endif

G_BEGIN_DECLS

#define FACQ_NIDAQ_ERROR facq_nidaq_error_quark()

typedef enum {
	FACQ_NIDAQ_ERROR_FAILED
} FacqNIDAQError;

struct _FacqNIDAQTask {
	TaskHandle taskHandle;
	gchar *phys_channel;
	guint n_channels;
};

typedef struct _FacqNIDAQTask FacqNIDAQTask;

FacqNIDAQTask *facq_nidaq_task_new(const gchar *name,GError **err);
gboolean facq_nidaq_task_start(FacqNIDAQTask *task,GError **err);
gboolean facq_nidaq_task_add_virtual_chan(FacqNIDAQTask *task,const gchar *device,const FacqChanlist *chanlist,gdouble max,gdouble min,GError **err);
gboolean facq_nidaq_task_done(FacqNIDAQTask *task,GError **err);
gboolean facq_nidaq_task_stop(FacqNIDAQTask *task,GError **err);
gboolean facq_nidaq_task_setup_timing(FacqNIDAQTask *task,gdouble period,guint32 samps_per_chan,GError **err);
gboolean facq_nidaq_task_setup_input_buffer(FacqNIDAQTask *task,guint32 samps_per_chan,GError **err);
guint32 facq_nidaq_task_get_read_avail_samples_per_chan(FacqNIDAQTask *task,GError **err);
gchar *facq_nidaq_task_get_read_wait_mode(FacqNIDAQTask *task,gint32 *mode,GError **err);
gdouble facq_nidaq_task_get_read_sleep_time(FacqNIDAQTask *task,GError **err);
gboolean facq_nidaq_task_get_read_all_avail_samples(FacqNIDAQTask *task,GError **err);
void facq_nidaq_task_set_read_all_avail_samples(FacqNIDAQTask *task,gboolean value,GError **err);
gchar *facq_nidaq_task_get_xfer_mode(FacqNIDAQTask *task,GError **err);
void facq_nidaq_task_set_ai_data_xfer_req_cond(FacqNIDAQTask *task,gint32 value,guint32 custhreshold,GError **err);
gchar *facq_nidaq_task_get_ai_data_xfer_req_cond(FacqNIDAQTask *task,GError **err);
guint32 facq_nidaq_task_get_onboard_buffer_size(FacqNIDAQTask *task,GError **err);
void facq_nidaq_task_free(FacqNIDAQTask *task);

gssize facq_nidaq_task_write(FacqNIDAQTask *task,gdouble *buffer,gint32 samps_per_chan,gdouble timeout,GError **err);
gint facq_nidaq_poll_old(FacqNIDAQTask *task,guint32 bufsize,gdouble period,guint64 *totalSampPerChan,GError **err);
gssize facq_nidaq_task_read(FacqNIDAQTask *task,gdouble *buffer,guint32 samples,gint32 samps_per_chan,gdouble timeout,GError **err);

guint32 facq_nidaq_device_serial_get(const gchar *dev,GError **err);
void facq_nidaq_device_reset(const gchar *dev,GError **err);

G_END_DECLS

#endif

#endif //USE_NIDAQ

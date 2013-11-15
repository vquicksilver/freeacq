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
#ifndef _FREEACQ_PIPELINE_MONITOR_H
#define _FREEACQ_PIPELINE_MONITOR_H

G_BEGIN_DECLS

#define FACQ_TYPE_PIPELINE_MONITOR (facq_pipeline_monitor_get_type ())
#define FACQ_PIPELINE_MONITOR(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_PIPELINE_MONITOR, FacqPipelineMonitor))
#define FACQ_PIPELINE_MONITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_PIPELINE_MONITOR, FacqPipelineMonitorClass))
#define FACQ_IS_PIPELINE_MONITOR(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_PIPELINE_MONITOR))
#define FACQ_IS_PIPELINE_MONITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_PIPELINE_MONITOR))
#define FACQ_PIPELINE_MONITOR_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_PIPELINE_MONITOR, FacqPipelineMonitorClass))

typedef struct _FacqPipelineMonitor FacqPipelineMonitor;
typedef struct _FacqPipelineMonitorClass FacqPipelineMonitorClass;
typedef struct _FacqPipelineMonitorPrivate FacqPipelineMonitorPrivate;
typedef void(*FacqPipelineMonitorCb)(FacqPipelineMessage *msg,gpointer data);

struct _FacqPipelineMonitor {
	/*< private >*/
	GObject parent_instance;
	FacqPipelineMonitorPrivate *priv;
};

struct _FacqPipelineMonitorClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_pipeline_monitor_get_type(void) G_GNUC_CONST;

FacqPipelineMonitor *facq_pipeline_monitor_new(FacqPipelineMonitorCb error_cb,FacqPipelineMonitorCb stop_cb,gpointer data);
void facq_pipeline_monitor_push(FacqPipelineMonitor *mon,FacqPipelineMessage *msg);
FacqPipelineMessage *facq_pipeline_monitor_pop(FacqPipelineMonitor *mon);
void facq_pipeline_monitor_clear(FacqPipelineMonitor *mon);
void facq_pipeline_monitor_attach(FacqPipelineMonitor *mon);
void facq_pipeline_monitor_dettach(FacqPipelineMonitor *mon);
void facq_pipeline_monitor_free(FacqPipelineMonitor *mon);

G_END_DECLS

#endif

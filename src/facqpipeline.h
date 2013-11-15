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
#ifndef _FREEACQ_PIPELINE_H
#define _FREEACQ_PIPELINE_H

G_BEGIN_DECLS

#define FACQ_PIPELINE_ERROR facq_pipeline_error_quark()

#define FACQ_TYPE_PIPELINE (facq_pipeline_get_type ())
#define FACQ_PIPELINE(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_PIPELINE, FacqPipeline))
#define FACQ_PIPELINE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_PIPELINE, FacqPipelineClass))
#define FACQ_IS_PIPELINE(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_PIPELINE))
#define FACQ_IS_PIPELINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_PIPELINE))
#define FACQ_PIPELINE_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_PIPELINE, FacqPipelineClass))

typedef enum {
	FACQ_PIPELINE_ERROR_FAILED
} FacqPipelineError;

typedef struct _FacqPipeline FacqPipeline;
typedef struct _FacqPipelineClass FacqPipelineClass;
typedef struct _FacqPipelinePrivate FacqPipelinePrivate;

struct _FacqPipeline {
	/*< private >*/
	GObject parent_instance;
	FacqPipelinePrivate *priv;
};

struct _FacqPipelineClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_pipeline_get_type(void) G_GNUC_CONST;

FacqPipeline *facq_pipeline_new(guint chunk_size,guint ring_chunks,FacqSource *src,FacqOperationList *oplist,FacqSink *sink,FacqPipelineMonitor *mon,GError **err);
gboolean facq_pipeline_start(FacqPipeline *p,GError **err);
void facq_pipeline_stop(FacqPipeline *p);
void facq_pipeline_free(FacqPipeline *p);

G_END_DECLS

#endif

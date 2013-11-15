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
#ifndef _FREEACQ_PIPELINE_MESSAGE_H_
#define _FREEACQ_PIPELINE_MESSAGE_H_

G_BEGIN_DECLS

typedef enum _FacqPipelineMessageType {
	FACQ_PIPELINE_MESSAGE_TYPE_ERROR,
	FACQ_PIPELINE_MESSAGE_TYPE_STOP,
	/*< private >*/
	FACQ_PIPELINE_MESSAGE_TYPE_N
} FacqPipelineMessageType;

#define FACQ_TYPE_PIPELINE_MESSAGE (facq_pipeline_message_get_type ())
#define FACQ_PIPELINE_MESSAGE(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_PIPELINE_MESSAGE, FacqPipelineMessage))
#define FACQ_PIPELINE_MESSAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_PIPELINE_MESSAGE, FacqPipelineMessageClass))
#define FACQ_IS_PIPELINE_MESSAGE(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_PIPELINE_MESSAGE))
#define FACQ_IS_PIPELINE_MESSAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_PIPELINE_MESSAGE))
#define FACQ_PIPELINE_MESSAGE_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_PIPELINE_MESSAGE, FacqPipelineMessageClass))

typedef struct _FacqPipelineMessage FacqPipelineMessage;
typedef struct _FacqPipelineMessageClass FacqPipelineMessageClass;
typedef struct _FacqPipelineMessagePrivate FacqPipelineMessagePrivate;

struct _FacqPipelineMessage {
	/*< private >*/
	GObject parent_instance;
	FacqPipelineMessagePrivate *priv;
};

struct _FacqPipelineMessageClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_pipeline_message_get_type(void) G_GNUC_CONST;

FacqPipelineMessage *facq_pipeline_message_new(FacqPipelineMessageType type,const gchar *info);
FacqPipelineMessageType facq_pipeline_message_get_msg_type(const FacqPipelineMessage *msg);
gchar *facq_pipeline_message_get_info(const FacqPipelineMessage *msg);
void facq_pipeline_message_free(FacqPipelineMessage *msg);

G_END_DECLS

#endif

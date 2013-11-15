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
#include <glib.h>
#include <gio/gio.h>
#include "facqpipelinemessage.h"

/**
 * SECTION:facqpipelinemessage
 * @short_description: Container of pipeline messages.
 * @include: facqpipelinemessage.h
 *
 * Note that this class is only used internally by the #FacqPipeline and you
 * shouldn't need to call any of this functions, except
 * facq_pipeline_message_get_info(). Anyway it's documented here for
 * reference purposes.
 *
 * For creating a new #FacqPipelineMessage just call
 * facq_pipeline_message_new() with the desired type and info.
 * For knowing the type of an existing message call
 * facq_pipeline_message_get_msg_type(), for getting the message info call
 * facq_pipeline_message_get_info(), and for destroying a no longer needed
 * message call facq_pipeline_message_free().
 *
 */

/**
 * FacqPipelineMessage:
 *
 * Contains the private details of the #FacqPipelineMessage objects.
 */

/**
 * FacqPipelineMessageClass:
 *
 * Class for the #FacqPipelineMessage objects.
 */

 /**
  * FacqPipelineMessageType:
  * @FACQ_PIPELINE_MESSAGE_TYPE_ERROR: The pipeline stopped due to an error
  * condition.
  * @FACQ_PIPELINE_MESSAGE_TYPE_STOP: The pipeline stopped due to an stop
  * condition for example, there isn't any more data on the source.
  *
  * Enum values for types of pipeline messages.
  */
G_DEFINE_TYPE(FacqPipelineMessage,facq_pipeline_message,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_MSG_TYPE,
	PROP_MSG_INFO
};

struct _FacqPipelineMessagePrivate {
	FacqPipelineMessageType type;
	gchar *info;
};

/* GObject magic */
static void facq_pipeline_message_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqPipelineMessage *msg = FACQ_PIPELINE_MESSAGE(self);

	switch(property_id){
	case PROP_MSG_INFO: msg->priv->info = g_value_dup_string(value);
	break;
	case PROP_MSG_TYPE: msg->priv->type = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(msg,property_id,pspec);
	}
}

static void facq_pipeline_message_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqPipelineMessage *msg = FACQ_PIPELINE_MESSAGE(self);

	switch(property_id){
	case PROP_MSG_INFO: g_value_set_string(value,msg->priv->info);
	break;
	case PROP_MSG_TYPE: g_value_set_uint(value,msg->priv->type);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(msg,property_id,pspec);
	}
}

static void facq_pipeline_message_finalize(GObject *self)
{
	FacqPipelineMessage *msg = FACQ_PIPELINE_MESSAGE(self);

	if(msg->priv->info)
		g_free(msg->priv->info);

	if(G_OBJECT_CLASS(facq_pipeline_message_parent_class)->finalize)
                (*G_OBJECT_CLASS(facq_pipeline_message_parent_class)->finalize)(self);
}

static void facq_pipeline_message_class_init(FacqPipelineMessageClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqPipelineMessagePrivate));

	object_class->set_property = facq_pipeline_message_set_property;
	object_class->get_property = facq_pipeline_message_get_property;
	object_class->finalize = facq_pipeline_message_finalize;

	g_object_class_install_property(object_class,PROP_MSG_INFO,
					g_param_spec_string("info",
							    "Info",
							    "The message information",
							    NULL,
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_READWRITE |
							    G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_MSG_TYPE,
					g_param_spec_uint("type",
							  "Type",
							  "The message type",
							  FACQ_PIPELINE_MESSAGE_TYPE_ERROR,
							  FACQ_PIPELINE_MESSAGE_TYPE_N-1,
							  FACQ_PIPELINE_MESSAGE_TYPE_ERROR,
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_READWRITE |
							  G_PARAM_STATIC_STRINGS));
}

static void facq_pipeline_message_init(FacqPipelineMessage *msg)
{
	msg->priv = G_TYPE_INSTANCE_GET_PRIVATE(msg,FACQ_TYPE_PIPELINE_MESSAGE,FacqPipelineMessagePrivate);
	msg->priv->info = NULL;
	msg->priv->type = 0;
}

/* Public methods */

/**
 * facq_pipeline_message_new:
 * @type: A value listed in #FacqPipelineMessageType.
 * @info: (allow-none): A pointer to a string or %NULL.
 *
 * Creates a new #FacqPipelineMessage.
 *
 * Returns: A new #FacqPipelineMessage.
 */
FacqPipelineMessage *facq_pipeline_message_new(FacqPipelineMessageType type,const gchar *info)
{
	return g_object_new(FACQ_TYPE_PIPELINE_MESSAGE,"type",type,"info",info,NULL);
}

/**
 * facq_pipeline_message_get_msg_type:
 * @msg: A #FacqPipelineMessage object.
 *
 * Gets the message type from a #FacqPipelineMessage. The type will be one
 * of the values listed in #FacqPipelineMessageType.
 *
 * Returns: The message type, that should be one of the values
 * in #FacqPipelineMessageType.
 */
FacqPipelineMessageType facq_pipeline_message_get_msg_type(const FacqPipelineMessage *msg)
{
	return msg->priv->type;
}

/**
 * facq_pipeline_message_get_info:
 * @msg: A #FacqPipelineMessage object.
 *
 * Gets a copy of the info contained in a #FacqPipelineMessage object.
 * Note that info is a simple text string.
 *
 * Returns: The message information, you must free it with g_free() when
 * no longer needed.
 */
gchar *facq_pipeline_message_get_info(const FacqPipelineMessage *msg)
{
	g_return_val_if_fail(FACQ_IS_PIPELINE_MESSAGE(msg),NULL);
	return g_strdup(msg->priv->info);
}

/**
 * facq_pipeline_message_free:
 * @msg: A #FacqPipelineMessage object.
 *
 * Destroys a no longer needed #FacqPipelineMessage.
 */
void facq_pipeline_message_free(FacqPipelineMessage *msg)
{
	g_return_if_fail(FACQ_IS_PIPELINE_MESSAGE(msg));
	g_object_unref(G_OBJECT(msg));
}

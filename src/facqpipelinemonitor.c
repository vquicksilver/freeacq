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
#include "facqpipelinemonitor.h"

/**
 * SECTION:facqpipelinemonitor
 * @include:facqpipelinemonitor.h
 * @short_description: Provides services for dealing with status changes in the
 * pipeline.
 * @see_also: #FacqPipelineMessage
 *
 * A #FacqPipelineMonitor provides services for dealing with status changes in
 * the pipeline. When a #FacqPipeline object is started, some errors can happen
 * in any of the stream elements, for example in the source, that can stop the
 * pipeline. For dealing with this situations you must create a
 * #FacqPipelineMonitor object and pass it in the facq_pipeline_new() function.
 * When an error or stop condition happens in the pipeline, the correspondent
 * callback function will be called (In the main application thread), 
 * without the need of checking for this conditions manually.
 *
 * For creating a new #FacqPipelineMonitor use facq_pipeline_monitor_new() and
 * pass to it the callback functions, and if you want, a pointer to some data,
 * that will be passed to your callback functions.
 * To destroy the #FacqPipelineMonitor object use facq_pipeline_monitor_free().
 *
 * facq_pipeline_monitor_push(), facq_pipeline_monitor_pop(),
 * facq_pipeline_monitor_clear(), facq_pipeline_monitor_attach(),
 * facq_pipeline_monitor_dettach() shouldn't be used directly by the user,
 * so you can ignore them (#FacqStream is the user of this functions).
 *
 * Note that this object, is not intended for public usage, but anyway is
 * documented here for reference purposes. It's used by #FacqStream in a
 * transparent manner to the user.
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * #FacqPipelineMonitor internals are simple. It's basically a timeout #GSource
 * that it's dispatched each second. In the callback function, the main thread
 * checks for the presence of new #FacqPipelineMessage objects, if any the
 * callback checks the type of the message and calls one of the callback
 * function allowing the user to react to the message. Because the processing is
 * done on the main thread, your callback function can manipulate the Gtk GUI of
 * your program without the need of calling deprecated gdk_lock functions.
 * For storing/retrieving the messages in a thread safe way #FacqPipelineMonitor uses
 * internally a #GAsyncQueue.
 * </para>
 * </sect1>
 */

 /**
  * FacqPipelineMonitorCb:
  * @msg: A #FacqPipelineMessage.
  * @data: A pointer to some relevant data for your callbacks.
  *
  * Type of the callback functions accepted by the #FacqPipelineMonitor object.
  */

G_DEFINE_TYPE(FacqPipelineMonitor,facq_pipeline_monitor,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_ERROR_CALLBACK,
	PROP_STOP_CALLBACK,
	PROP_DATA
};

struct _FacqPipelineMonitorPrivate {
	GAsyncQueue *q;
	guint source_id;
	FacqPipelineMonitorCb error_cb;
	FacqPipelineMonitorCb stop_cb;
	gpointer data;
};

/* Private methods */

/* This function runs on the main thread. It will be called each second.
 * It's purpose it's to check for the presence of a new FacqPipelineMessage
 * coming from the Producer/Consumer thread. Worker threads will push
 * a message to the Queue in case of error, or if stop condition is reach.
 * This function checks the type of the message and calls the corresponding
 * function to the type, that will ran on the main thread, so in case of error
 * or stop condition the pipeline could be stopped from the main thread.
 */
static gboolean facq_pipeline_monitor_timeout_func(gpointer monitor)
{
	FacqPipelineMonitor *mon = FACQ_PIPELINE_MONITOR(monitor);
	FacqPipelineMessage *msg = NULL;

	msg = g_async_queue_try_pop(mon->priv->q);
	if(msg){
		switch(facq_pipeline_message_get_msg_type(msg)){
		case FACQ_PIPELINE_MESSAGE_TYPE_N:
		case FACQ_PIPELINE_MESSAGE_TYPE_ERROR:
			mon->priv->error_cb(msg,mon->priv->data);
		break;
		case FACQ_PIPELINE_MESSAGE_TYPE_STOP:
			mon->priv->stop_cb(msg,mon->priv->data);
		break;
		}
		facq_pipeline_message_free(msg);
		return FALSE;
	}
	return TRUE;
}

/* GObject magic */
static void facq_pipeline_monitor_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqPipelineMonitor *mon = FACQ_PIPELINE_MONITOR(self);

	switch(property_id){
	case PROP_ERROR_CALLBACK: g_value_set_pointer(value,mon->priv->error_cb);
	break;
	case PROP_STOP_CALLBACK: g_value_set_pointer(value,mon->priv->stop_cb);
	break;
	case PROP_DATA: g_value_set_pointer(value,mon->priv->data);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(mon,property_id,pspec);
	}
}

static void facq_pipeline_monitor_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqPipelineMonitor *mon = FACQ_PIPELINE_MONITOR(self);

	switch(property_id){
	case PROP_ERROR_CALLBACK: mon->priv->error_cb = g_value_get_pointer(value);
	break;
	case PROP_STOP_CALLBACK: mon->priv->stop_cb = g_value_get_pointer(value);
	break;
	case PROP_DATA: mon->priv->data = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(mon,property_id,pspec);
	}
}

static void facq_pipeline_monitor_finalize(GObject *self)
{
	FacqPipelineMonitor *mon = FACQ_PIPELINE_MONITOR(self);

	g_async_queue_unref(mon->priv->q);
}

static void facq_pipeline_monitor_constructed(GObject *self)
{
	FacqPipelineMonitor *mon = FACQ_PIPELINE_MONITOR(self);

	mon->priv->q = g_async_queue_new_full((GDestroyNotify)facq_pipeline_message_free);
}

static void facq_pipeline_monitor_class_init(FacqPipelineMonitorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqPipelineMonitorPrivate));

	object_class->set_property = facq_pipeline_monitor_set_property;
	object_class->get_property = facq_pipeline_monitor_get_property;
	object_class->constructed = facq_pipeline_monitor_constructed;
	object_class->finalize = facq_pipeline_monitor_finalize;

	g_object_class_install_property(object_class,PROP_ERROR_CALLBACK,
					g_param_spec_pointer("error-cb",
						             "Error callback",
							     "A callback function for handling errors",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_STOP_CALLBACK,
					g_param_spec_pointer("stop-cb",
							     "Stop callback",
							     "A callback function for handling stop condition",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_DATA,
					g_param_spec_pointer("data",
							     "Data",
							     "A pointer to data for the callbacks",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));
}

static void facq_pipeline_monitor_init(FacqPipelineMonitor *mon)
{
	mon->priv = G_TYPE_INSTANCE_GET_PRIVATE(mon,FACQ_TYPE_PIPELINE_MONITOR,FacqPipelineMonitorPrivate);
	mon->priv->q = NULL;
	mon->priv->source_id = 0;
}

/* Public methods */
/**
 * facq_pipeline_monitor_new:
 * @error_cb: A #FacqPipelineMonitorCb callback.
 * @stop_cb: A #FacqPipelineMonitorCb callback.
 * @data: (allow-none): A pointer to some relevant data.
 *
 * Creates a new #FacqPipelineMonitor object with the specified parameters.
 *
 * Returns: A new #FacqPipelineMonitor object.
 */
FacqPipelineMonitor *facq_pipeline_monitor_new(FacqPipelineMonitorCb error_cb,FacqPipelineMonitorCb stop_cb,gpointer data)
{
	return g_object_new(FACQ_TYPE_PIPELINE_MONITOR,
			    "error-cb",error_cb,
			    "stop-cb",stop_cb,
			    "data",data,
			    NULL);
}

/**
 * facq_pipeline_monitor_push:
 * @mon: A #FacqPipelineMonitor object.
 * @msg: A #FacqPipelineMessage object.
 *
 * Pushes the #FacqPipelineMessage, @msg, into the #FacqPipelineMonitor, @mon.
 * You don't have to use this function, it's called in #FacqPipeline when needed.
 */
void facq_pipeline_monitor_push(FacqPipelineMonitor *mon,FacqPipelineMessage *msg)
{
	g_async_queue_push(mon->priv->q,msg);
}

/**
 * facq_pipeline_monitor_pop:
 * @mon: A #FacqPipelineMonitor object.
 *
 * Pops a #FacqPipelineMessage from the #FacqPipelineMonitor, @mon.
 * You don't have to use this function, it's called in the timeout callback
 * function.
 */
FacqPipelineMessage *facq_pipeline_monitor_pop(FacqPipelineMonitor *mon)
{
	return g_async_queue_pop(mon->priv->q);
}

/**
 * facq_pipeline_monitor_clear:
 * @mon: A #FacqPipelineMonitor object.
 *
 * Clears any pending message in the #FacqPipelineMonitor, @mon.
 * You don't have to use this function, it's called in #FacqStream.
 * <note>
 * <para>
 * This function should be called before facq_pipeline_start()
 * cause length value won't be precise in other case. See
 * g_async_queue_length() for more details.
 * </para>
 * </note>
 */
void facq_pipeline_monitor_clear(FacqPipelineMonitor *mon)
{
	gint length = 0;
	FacqPipelineMessage *msg = NULL;

	g_return_if_fail(FACQ_IS_PIPELINE_MONITOR(mon));
	length = g_async_queue_length(mon->priv->q);
	while(length){
		msg = facq_pipeline_monitor_pop(mon);
		facq_pipeline_message_free(msg);
	}
}

/**
 * facq_pipeline_monitor_attach:
 * @mon: A #FacqPipelineMonitor object.
 *
 * Attaches the timeout source to the main thread, so the internal callback
 * function can start to check for new messages, in the #FacqPipelineMonitor
 * object.
 * You don't need to call this function, this function is called in #FacqStream.
 */
void facq_pipeline_monitor_attach(FacqPipelineMonitor *mon)
{
	g_return_if_fail(FACQ_IS_PIPELINE_MONITOR(mon));
	mon->priv->source_id = 
		g_timeout_add_seconds(1,facq_pipeline_monitor_timeout_func,mon);
}

/**
 * facq_pipeline_monitor_dettach:
 * @mon: A #FacqPipelineMonitor object.
 *
 * Detaches the timeout source from the main thread, stopping the execution of
 * the callback function.
 * You don't need to call this function, this function is called in #FacqStream.
 */ 
void facq_pipeline_monitor_dettach(FacqPipelineMonitor *mon)
{
	g_return_if_fail(FACQ_IS_PIPELINE_MONITOR(mon));
	g_source_remove(mon->priv->source_id);
}

/**
 * facq_pipeline_monitor_free:
 * @mon: A #FacqPipelineMonitor object.
 *
 * Destroys the #FacqPipelineMonitor, @mon.
 */
void facq_pipeline_monitor_free(FacqPipelineMonitor *mon)
{
	g_return_if_fail(FACQ_IS_PIPELINE_MONITOR(mon));
	g_object_unref(G_OBJECT(mon));
}

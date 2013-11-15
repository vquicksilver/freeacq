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
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqplug.h"

/**
 * SECTION:facqplug
 * @include: facqplug.h
 * @short_description: a service for creating network based applications
 * @see_also: #FacqChunk,#FacqStreamData,#GInitable
 *
 * A #FacqPlug provides an application independent way for creating network
 * applications that can receive data from an ongoing acquisition (Using
 * #FacqOperationPlug), easing the process of creating network applications.
 *
 * Internally #FacqPlug handles all the connect/disconnect details, allowing
 * only a single client to be connected simultaneously, and gives and API that
 * allows to disconnect a client, get it's details like address and port, or
 * change the listen address without the need of knowing how TCP/IP works.
 *
 * To make the task of processing received data more easy you only have to
 * provide a function pointer, with optionally a pointer to some extra data that
 * you need, and a timeout. This function will be called each timeout
 * milliseconds and will receive the data in form of a #FacqChunk along with your
 * data.
 *
 * Two signals, "connect" and "disconnect" are provided for easy detection of a 
 * client connections and disconnections, this will allow you to notify the user 
 * of your application of this events if you need to do it.
 *
 * To create a new #FacqPlug object use facq_plug_new(), to change the listen
 * address of an already created #FacqPlug use facq_plug_set_listen_address(),
 * to get the listen address use facq_plug_get_address(), to get the port use
 * facq_plug_get_port(). When a client is connected use
 * facq_plug_get_client_address() to get it's IP address, use
 * facq_plug_disconnect() if you want to disconnect the client, to get details
 * about the stream of data use facq_plug_get_stream_data().
 * Finally to destroy the #FacqPlug object use facq_plug_free().
 *
 * <sect1 id="Internal details">
 *  <title>Internal details</title>
 *  <note>
 *   <para>
 *   <emphasis>
 *   Please ignore this section if you are a user of this module, this is for
 *   developers only.
 *   </emphasis>
 *   </para>
 *  </note>
 *  <sect2 id="used-objects">
 *  <title>Used objects</title>
 *  <para>
 *  To provide the above services #FacqPlug uses the following objects:
 *  </para>
 *  <itemizedlist>
 *   <listitem>
 *    <para>
 *    Two #GSocket objects, one for listening and the other for receiving data
 *    from a connected client.
 *    </para>
 *   </listitem>
 *   <listitem>
 *    <para>
 *    A #GMutex object, for coordinating access to the client socket in the
 *    threads.
 *    </para>
 *   </listitem>
 *   <listitem>
 *    <para>
 *    A #GSource object, this source is attached to the main context and
 *    dispatches client, when a client tries to connect to the system.
 *    </para>
 *   </listitem>
 *   <listitem>
 *    <para>
 *    A #GThread object, for the producer thread.
 *    </para>
 *   </listitem>
 *   <listitem>
 *    <para>
 *    Two #GAsyncQueue objects. This objects are used for thread communication,
 *    for passing messages between the main thread and the producer thread.
 *    </para>
 *   </listitem>
 *   <listitem>
 *    <para>
 *    A #FacqBuffer object, for storing the chunks coming from the client.
 *    </para>
 *   </listitem>
 *   <listitem>
 *    <para>
 *    A #FacqStreamData object, for storing the related stream data.
 *    </para>
 *   </listitem>
 *  </itemizedlist>
 *  </sect2>
 *  <sect2 id="internal-behavior">
 *   <title>Internal behavior</title>
 *   <para>
 *    When the #FacqPlug is created the #GMutex is created, and the listen
 *    socket is created, using the provided address and port by the user, after 
 *    this a #GSource is created, and connected with a callback function, that
 *    is specifically written to deal with client connections. Each time a
 *    client tries to connect to the address and port (If the firewall 
 *    allows it) the callback function will be launched by the main thread.
 *    This function will check if new data can be read from the listen socket
 *    and if this new data corresponds to a new client petition, the client
 *    is accepted, or in other case rejected. This means that the other side
 *    should send a well formed #FacqStreamData object.
 *
 *    When a client is accepted, all the other connection petitions will be
 *    automatically rejected, as expected. Also after a valid #FacqStreamData is
 *    received, a #FacqBuffer, A producer to main #GAsyncQueue, A main to
 *    producer #GAsyncQueue, and a new timeout #GSource is created with the
 *    timeout parameter and function that the user passed at creation time, 
 *    after this the "connected" signal will be emitted telling the user that 
 *    a new client has been successfully connected to the #FacqPlug, finally 
 *    a new thread will be created, this thread is the called "Producer" thread.
 *
 *    After this point, the Main thread and the Producer thread will be running
 *    at the same time, the producer thread will try to get data from the
 *    client's socket and put each #FacqChunk in the #FacqBuffer, in case of
 *    error a message will be send to the main thread. Also the main thread
 *    can send messages to this thread, for example if the user wants to stop
 *    the process.
 *
 *    The main thread will try (When not busy with other things 
 *    like drawing the GUI) to get new chunks of data from the #FacqBuffer 
 *    or new message from the producer thread, (for example, if the client
 *    disconnects the main thread will receive a message from the producer
 *    thread) in the timeout callback function, and in case that new data is present 
 *    it will call the user function with the provided user data (if any). 
 *    The main thread will check for new data each timeout miliseconds, 
 *    so the cpu usage can be keep in a low level. This way the user doesn't
 *    have to worry about data reception or errors in the connection.
 *   </para>
 *  </sect2>
 * </sect1>
 */

 /**
  * FacqPlug:
  *
  * Contains the internal details of the #FacqPlug.
  */

 /**
  * FacqPlugClass:
  *
  * Class for the #FacqPlug objects.
  */

 /**
  * FacqPlugFunc:
  * @chunk: A #FacqChunk object, containing the samples.
  * @data: A pointer to some user data or %NULL.
  *
  * Prototype for the callback function that you must write when using
  * #FacqPlug. The function will be called each timeout time (See
  * facq_plug_new()). You don't have to free the #FacqChunk, the #FacqPlug will
  * do it for you.
  */

 /**
  * FacqPlugError:
  * @FACQ_PLUG_ERROR_FAILED: Some error happened in the plug object.
  *
  * This enum contains the possible error values.
  */

static void facq_plug_initable_iface_init(GInitableIface *iface);
static gboolean facq_plug_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);
static gboolean facq_plug_listen_callback(GSocket *skt,GIOCondition condition,gpointer oplug);
static gboolean facq_plug_timeout_callback(gpointer _plug);

G_DEFINE_TYPE_WITH_CODE(FacqPlug,facq_plug,G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_plug_initable_iface_init));

enum {
	CONNECTED,
	DISCONNECTED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

enum {
	PROP_0,
	PROP_PORT,
	PROP_ADDRESS,
	PROP_TIMEOUT_FUNC,
	PROP_TIMEOUT_DATA,
	PROP_TIMEOUT
};

struct _FacqPlugPrivate {
	gchar *address;
	guint16 port;
	GSocket *lst_skt;
	GSocket *clt_skt;
	/* Mutex for the client socket, it can be used on the main thread and 
	 * on the producer thread at the same time so we must ensure secure
	 * access */
#if GLIB_MINOR_VERSION >= 32
	GMutex client_mutex;
#else
	GMutex *client_mutex;
#endif
	GSource *lst_src; //Handles connection petitions from clients in the Main Thread;
	FacqPlugFunc mts_func; //Main thread timeout func called from our callback
	gpointer mts_data; //timeout source func data
	guint timeout; //timeout for the timeout source
	guint mts_id; //identifier of the source when created;
	FacqStreamData *stmd; //The stream data from the client
	FacqBuffer *buf; //Producer puts data Main pops the data
	GThread *prod; //Producer thread
	GAsyncQueue *ptom; //Producer to Main
	GAsyncQueue *mtop; //Main to Producer
	GError *construct_error;
};

GQuark facq_plug_error_quark(void)
{
	return g_quark_from_static_string("facq-plug-error-quark");
}

/* 
 * Note that FacqPlugMessage and FacqPlugMessageType are helpers for the
 * FacqPlug object, but the user doesn't have any knowing of them.
 * They are used internally only.
 */
typedef enum {
	FACQ_PLUG_MESSAGE_TYPE_DISCONNECT,
	FACQ_PLUG_MESSAGE_TYPE_ERROR,
	FACQ_PLUG_MESSAGE_TYPE_N
} FacqPlugMessageType;

struct _FacqPlugMessage {
	FacqPlugMessageType type;
	gchar *msg;
};

typedef struct _FacqPlugMessage FacqPlugMessage;

/* FacqPlugMessage related operations */
static FacqPlugMessage *facq_plug_message_new(FacqPlugMessageType type,const gchar *msg)
{
	FacqPlugMessage *ret = NULL;

	ret = g_new0(FacqPlugMessage,1);
	ret->type = type;
	if(msg)
		ret->msg = g_strdup(msg);
	
	return ret;
}

static void facq_plug_message_free(FacqPlugMessage *msg)
{
	g_return_if_fail(msg);
	
	if(msg->msg)
		g_free(msg->msg);
	g_free(msg);
}

/* Plug private operations */
static GInetAddress *check_address(const gchar *address,guint16 port,GError **err)
{
	GInetAddress *in_address = NULL;

	if(!address){
		in_address = g_inet_address_new_any(G_SOCKET_FAMILY_IPV4);
	}
	else {
		in_address = g_inet_address_new_from_string(address);
	}

	if(!in_address)
		if(err != NULL)
			g_set_error_literal(err,FACQ_PLUG_ERROR,
					FACQ_PLUG_ERROR_FAILED,"wrong address");

	return in_address;
}

static void facq_plug_lock_client(FacqPlug *plug)
{
	g_return_if_fail(FACQ_IS_PLUG(plug));

#if GLIB_MINOR_VERSION >= 32
	g_mutex_lock(&plug->priv->client_mutex);
#else
	g_mutex_lock(plug->priv->client_mutex);
#endif
}

static void facq_plug_unlock_client(FacqPlug *plug)
{
	g_return_if_fail(FACQ_IS_PLUG(plug));

#if GLIB_MINOR_VERSION >= 32
	g_mutex_unlock(&plug->priv->client_mutex);
#else
	g_mutex_unlock(plug->priv->client_mutex);
#endif
}

static void facq_plug_stop_listening(FacqPlug *plug)
{
	guint id;

	if(plug->priv->lst_src){
		id = g_source_get_id(plug->priv->lst_src);
		g_source_remove(id);
		g_source_destroy(plug->priv->lst_src);
		plug->priv->lst_src = NULL;
	}

	if(G_IS_SOCKET(plug->priv->lst_skt)){
		g_socket_close(plug->priv->lst_skt,NULL);
		g_object_unref(G_OBJECT(plug->priv->lst_skt));
		plug->priv->lst_skt = NULL;
	}
}

static void facq_plug_bind_and_listen(FacqPlug *plug,GError **err)
{
	GInetAddress *in_address = NULL;
	GSocketAddress *sock_addr = NULL;
	GSocketFamily family;
	GError *local_err = NULL;

	in_address = check_address(plug->priv->address,plug->priv->port,&local_err);
	if(local_err || !in_address)
		goto error;

	sock_addr = g_inet_socket_address_new(in_address,plug->priv->port);
	family = g_socket_address_get_family(sock_addr);

	plug->priv->lst_skt = 
				g_socket_new(family,
						G_SOCKET_TYPE_STREAM,
							G_SOCKET_PROTOCOL_TCP,
									&local_err);
	if(!plug->priv->lst_skt)
		goto error;
	
	if(!g_socket_bind(plug->priv->lst_skt,sock_addr,TRUE,&local_err) 
		|| local_err){
			if(local_err)
				goto error;
			else
				g_set_error_literal(&local_err,FACQ_PLUG_ERROR,
							FACQ_PLUG_ERROR_FAILED,"Unknown error binding");
			goto error;
	}

	g_socket_set_listen_backlog(plug->priv->lst_skt,1);
	if(!g_socket_listen(plug->priv->lst_skt,&local_err)
		|| local_err){
			if(local_err)
				goto error;
			else
				g_set_error_literal(&local_err,FACQ_PLUG_ERROR,
							FACQ_PLUG_ERROR_FAILED,"Unknown error listening");
			goto error;
	}

	plug->priv->lst_src = 
		g_socket_create_source(plug->priv->lst_skt,
							G_IO_IN,
								NULL);

	g_source_set_callback(plug->priv->lst_src,
				(GSourceFunc)facq_plug_listen_callback,
									plug,
										NULL);

	g_source_attach(plug->priv->lst_src,NULL);

	return;

	error:
	if(local_err)
		g_propagate_error(err,local_err);
	return;
}

static gpointer prod_fun(gpointer data)
{
	FacqPlug *plug = FACQ_PLUG(data);
	FacqPlugMessage *msg = NULL;
	FacqChunk *chunk = NULL;
	gboolean retctw = FALSE;
	gssize received = 0;
	GError *err = NULL;

	chunk = facq_buffer_get_recycled(plug->priv->buf);

	while(1){
		/* Check the mtop queue for messages */
		msg = g_async_queue_try_pop(plug->priv->mtop);
		if(msg){
			facq_log_write("P message received from main",FACQ_LOG_MSG_TYPE_DEBUG);
			switch(msg->type){
			case FACQ_PLUG_MESSAGE_TYPE_DISCONNECT:
				facq_plug_message_free(msg);
				msg = NULL;
				facq_log_write("P exit",FACQ_LOG_MSG_TYPE_DEBUG);
				return NULL;
			default:
				if(msg)
					facq_plug_message_free(msg);
				facq_log_write("P exit",FACQ_LOG_MSG_TYPE_DEBUG);
				return NULL;
			}
		}
		if(!chunk){
			facq_log_write("P waiting for recycled chunk",FACQ_LOG_MSG_TYPE_DEBUG);
			chunk = facq_buffer_try_get_recycled(plug->priv->buf);
		}
		if(chunk){
			facq_log_write("P empty chunk received, getting data",FACQ_LOG_MSG_TYPE_DEBUG);
			facq_plug_lock_client(plug);

			/* If new data available read the data */
#ifdef G_OS_UNIX
			retctw = g_socket_condition_timed_wait(plug->priv->clt_skt,
							    G_IO_IN | G_IO_ERR | G_IO_HUP,
							    1000000,
							    NULL,
							    &err);
			if(err){
				if(err->code == G_IO_ERROR_TIMED_OUT)
					g_clear_error(&err);
				else
					goto error;
			}
#elif defined(G_OS_WIN32)
			retctw = g_socket_condition_wait(plug->priv->clt_skt,
							 G_IO_IN | G_IO_ERR | G_IO_HUP,
							 NULL,
							 &err);
			if(err){
				goto error;
			}
#endif
			if(!err && retctw){
				retctw = FALSE;

				/* try to read a full chunk */
				facq_log_write("calling facq_net_receive",FACQ_LOG_MSG_TYPE_DEBUG);
				received = facq_net_receive(plug->priv->clt_skt,
							   chunk->data,
							   chunk->len,
							   0,
							   &err);
				facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
						"facq_net_receive returned %"G_GSSIZE_FORMAT,
						received);
				switch(received){
				case -3:
					/* error in receive parameters */
					facq_log_write("Error in receive parameters",FACQ_LOG_MSG_TYPE_ERROR);
					goto error;
				break;
				case -2:
					/* timeout */
					facq_log_write("Timeout receiving data",FACQ_LOG_MSG_TYPE_ERROR);
					goto error;
				break;
				case -1:
					/* error receiving data */
					facq_log_write("Error receiving data",FACQ_LOG_MSG_TYPE_ERROR);
					goto error;
				break;
				case 0:
					/* client disconnected */
					msg = facq_plug_message_new(
								FACQ_PLUG_MESSAGE_TYPE_DISCONNECT,
												NULL);
					g_async_queue_push(plug->priv->ptom,msg);
					goto error;
				break;
				default:
					/* chunk should be full here */
					facq_chunk_add_used_bytes(chunk,received);
					facq_buffer_push(plug->priv->buf,chunk);
					chunk = NULL;
				}
			}
			facq_plug_unlock_client(plug);
			g_thread_yield();
		}
	}

	facq_log_write("P exit",FACQ_LOG_MSG_TYPE_DEBUG);
	return NULL;

	error:
	facq_plug_unlock_client(plug);
	if(chunk)
		facq_chunk_free(chunk);
	if(err){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",err->message);
		msg = facq_plug_message_new(FACQ_PLUG_MESSAGE_TYPE_ERROR,err->message);
		g_async_queue_push(plug->priv->ptom,msg);
		g_clear_error(&err);
	}
	else {
		facq_log_write("Client disconnected or unknown error in producer thread",
				FACQ_LOG_MSG_TYPE_ERROR);
		msg = facq_plug_message_new(FACQ_PLUG_MESSAGE_TYPE_DISCONNECT,NULL);
		g_async_queue_push(plug->priv->ptom,msg);
	}
	facq_log_write("P exit",FACQ_LOG_MSG_TYPE_DEBUG);
	return NULL;
}

static void facq_plug_accept_client(FacqPlug *plug)
{
	gchar *address = NULL;
	guint chunk_size = 0;
	FacqStreamData *stmd = NULL;
	GError *local_err = NULL;

	address = facq_plug_get_client_address(plug,&local_err);
	if(local_err){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
					"Error getting client address: %s",
						local_err->message);
		g_clear_error(&local_err);
		g_socket_shutdown(plug->priv->clt_skt,TRUE,TRUE,NULL);
		g_object_unref(G_SOCKET(plug->priv->clt_skt));
		plug->priv->clt_skt = NULL;
		return;
	}
	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,"%s is connected",address);
	g_free(address);

	stmd = facq_stream_data_from_socket(plug->priv->clt_skt,&local_err);
	if(local_err){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
					"Error getting streamdata: %s",
							local_err->message);
		g_clear_error(&local_err);
		g_socket_shutdown(plug->priv->clt_skt,TRUE,TRUE,NULL);
		g_socket_close(plug->priv->clt_skt,NULL);
		plug->priv->clt_skt = NULL;
		return;
	}
	plug->priv->stmd = stmd;
	facq_log_write("StreamData received, connection accepted",FACQ_LOG_MSG_TYPE_DEBUG);
	
	/* create a FacqBuffer for storing data */
	chunk_size = 
		facq_misc_period_to_chunk_size(plug->priv->stmd->period,
					       sizeof(gdouble),
					       plug->priv->stmd->n_channels);
	plug->priv->buf = facq_buffer_new(5,chunk_size,&local_err);
	/* create async queue for main->producer message passing */
	plug->priv->ptom = g_async_queue_new_full((GDestroyNotify)facq_plug_message_free);
	/* create async queue for producer->main message passing */
	plug->priv->mtop = g_async_queue_new_full((GDestroyNotify)facq_plug_message_free);
	/* attach the timeout source if fun is not NULL. We use here our own
	 * function, and call the user function from our function in it. */
	plug->priv->mts_id = 
			g_timeout_add_full(G_PRIORITY_DEFAULT,
					   plug->priv->timeout,
				           facq_plug_timeout_callback,
					   plug,
					   NULL);
	g_signal_emit(plug,signals[CONNECTED],0);
	/* create a producer thread */
	facq_log_write("Creating producer thread",FACQ_LOG_MSG_TYPE_DEBUG);
	plug->priv->prod = g_thread_try_new("prod",prod_fun,plug,&local_err);
	if(local_err){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
					"Error creating thread: %s",
						local_err->message);
		g_clear_error(&local_err);
	}
}

static void facq_plug_disconnect_client(FacqPlug *plug)
{
	/* destroy the timeout source if any */
	if(plug->priv->mts_id){
		g_source_remove(plug->priv->mts_id);
	}

	/* destroy the buffer */
	facq_buffer_free(plug->priv->buf);

	/* destroy the stream data */
	facq_stream_data_free(plug->priv->stmd);

	/* destroy the queues */
	g_async_queue_unref(plug->priv->ptom);
	g_async_queue_unref(plug->priv->mtop);

	/* emit disconnect signal */
	g_signal_emit(plug,signals[DISCONNECTED],0);
}

static void facq_plug_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqPlug *plug = FACQ_PLUG(self);

	switch(property_id){
	case PROP_ADDRESS: g_value_set_string(value,plug->priv->address);
	break;
	case PROP_PORT: g_value_set_uint(value,plug->priv->port);
	break;
	case PROP_TIMEOUT_FUNC: g_value_set_pointer(value,plug->priv->mts_func);
	break;
	case PROP_TIMEOUT_DATA: g_value_set_pointer(value,plug->priv->mts_data);
	break;
	case PROP_TIMEOUT: g_value_set_uint(value,plug->priv->timeout);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(plug,property_id,pspec);
	}
}

static void facq_plug_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqPlug *plug = FACQ_PLUG(self);

	switch(property_id){
	case PROP_ADDRESS: plug->priv->address = g_value_dup_string(value);
	break;
	case PROP_PORT: plug->priv->port = g_value_get_uint(value);
	break;
	case PROP_TIMEOUT_FUNC: plug->priv->mts_func = g_value_get_pointer(value);
	break;
	case PROP_TIMEOUT_DATA: plug->priv->mts_data = g_value_get_pointer(value);
	break;
	case PROP_TIMEOUT: plug->priv->timeout = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(plug,property_id,pspec);
	}
}

static void facq_plug_finalize(GObject *self)
{
	FacqPlug *plug = FACQ_PLUG(self);
	guint id;

	/* remove the source from the main loop and destroy them */
	if(plug->priv->lst_src){
		id = g_source_get_id(plug->priv->lst_src);
		g_source_remove(id);
		g_source_destroy(plug->priv->lst_src);
	}

	/* destroy the listen socket */
	if(G_IS_SOCKET(plug->priv->lst_skt)){
		g_object_unref(G_OBJECT(plug->priv->lst_skt));
	}

	if(plug->priv->address)
		g_free(plug->priv->address);

#if GLIB_MINOR_VERSION >= 32
        g_mutex_clear(&plug->priv->client_mutex);
#else
        g_mutex_free(plug->priv->client_mutex);
#endif

	if (G_OBJECT_CLASS (facq_plug_parent_class)->finalize)
                (*G_OBJECT_CLASS (facq_plug_parent_class)->finalize) (self);
}

static void facq_plug_constructed(GObject *self)
{
	FacqPlug *plug = FACQ_PLUG(self);
	GError *local_err = NULL;

#if GLIB_MINOR_VERSION >= 32
	g_mutex_init(&plug->priv->client_mutex);
#else
	plug->priv->client_mutex = g_mutex_new();
#endif

	facq_plug_bind_and_listen(plug,&local_err);
	if(local_err)
		g_propagate_error(&plug->priv->construct_error,local_err);
}

static void facq_plug_class_init(FacqPlugClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	
	g_type_class_add_private(klass,sizeof(FacqPlugPrivate));

	object_class->set_property = facq_plug_set_property;
	object_class->get_property = facq_plug_get_property;
	object_class->finalize = facq_plug_finalize;
	object_class->constructed = facq_plug_constructed;

	/**
	 * FacqPlug::connected:
	 * @facqplug: A #FacqPlug object.
	 * @user_data: Some user data or %NULL.
	 *
	 * The ::connected signal is emitted each time a client is accepted by
	 * the #FacqPlug object (Note that only one client can be accepted at
	 * the same time, other petitions are silently rejected). 
	 * An application can use this signal to notify the user of a new connection.
	 */
	signals[CONNECTED] = g_signal_new("connected",
					  G_TYPE_FROM_CLASS(klass),
					  G_SIGNAL_RUN_LAST,
					  0,
					  NULL,
					  NULL,
					  g_cclosure_marshal_VOID__VOID,
					  G_TYPE_NONE,
					  0);
	/**
	 * FacqPlug::disconnected:
	 * @facqplug: A #FacqPlug object.
	 * @user_data: Some user data or %NULL.
	 *
	 * The ::disconnected signal is emitted each time a client disconnects by
	 * the #FacqPlug object. An application can use this signal to notify
	 * the user of the disconnection.
	 */
	signals[DISCONNECTED] = g_signal_new("disconnected",
					      G_TYPE_FROM_CLASS(klass),
					      G_SIGNAL_RUN_LAST,
					      0,
					      NULL,
					      NULL,
					      g_cclosure_marshal_VOID__VOID,
					      G_TYPE_NONE,
					      0);

	g_object_class_install_property(object_class,PROP_ADDRESS,
					g_param_spec_string("address",
							    "The local address",
							    "The listened local address",
							    "127.0.0.1",
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_READWRITE |
							    G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_PORT,
                                        g_param_spec_uint("port",
                                                          "The port",
                                                          "The VI port",
                                                          0,
                                                          65535,
                                                          3000,
                                                          G_PARAM_READWRITE |
                                                          G_PARAM_CONSTRUCT_ONLY |
                                                          G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_TIMEOUT,
					g_param_spec_uint("timeout",
							  "Timeout",
							  "The time between calls in ms",
							  1,
							  G_MAXUINT,
							  500,
							  G_PARAM_READWRITE |
							  G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_TIMEOUT_FUNC,
					g_param_spec_pointer("timeout-func",
							     "Timeout Function",
							     "A function to deal with the data",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_TIMEOUT_DATA,
					g_param_spec_pointer("timeout-data",
							     "Timeout Data",
							     "A pointer to some data for the function",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));
}

static void facq_plug_init(FacqPlug *plug)
{
	plug->priv = G_TYPE_INSTANCE_GET_PRIVATE(plug,FACQ_TYPE_PLUG,FacqPlugPrivate);
	plug->priv->lst_skt = NULL;
	plug->priv->clt_skt = NULL;
	plug->priv->lst_src = NULL;
	plug->priv->address = NULL;
	plug->priv->prod = NULL;
	plug->priv->ptom = NULL;
	plug->priv->mtop = NULL;
	plug->priv->buf = NULL;
	plug->priv->stmd = NULL;
	plug->priv->mts_func = NULL;
	plug->priv->mts_data = NULL;
}

/*****--- Important callbacks ---*****/
static gboolean facq_plug_timeout_callback(gpointer _plug)
{
	FacqPlug *plug = FACQ_PLUG(_plug);
	FacqChunk *chunk = NULL;
	FacqPlugMessage *msg = NULL;
	gboolean ret = TRUE;

	/* Check for messages in the ptom queue */
	facq_log_write("M facq_plug_timeout_callback",FACQ_LOG_MSG_TYPE_DEBUG);
	msg = g_async_queue_try_pop(plug->priv->ptom);
	if(msg){
		/* do actions in function of the type of message if any */
		facq_log_write("M message received",FACQ_LOG_MSG_TYPE_DEBUG);
		switch(msg->type){
		case FACQ_PLUG_MESSAGE_TYPE_DISCONNECT:
		break;
		case FACQ_PLUG_MESSAGE_TYPE_ERROR:
		break;
		default:
		break;
		}

		/* destroy the message */
		facq_plug_message_free(msg);

		/* disconnect */
		facq_plug_disconnect(plug);

		return FALSE;
	}

	/* try to get a chunk else return */
	facq_log_write("M trying to get a chunk of data",FACQ_LOG_MSG_TYPE_DEBUG);
	chunk = facq_buffer_try_pop(plug->priv->buf);
	if(chunk){
		facq_log_write("M I have a chunk of data",FACQ_LOG_MSG_TYPE_DEBUG);
		/* convert the chunk to native */
		facq_chunk_data_double_to_be(chunk);
#if ENABLE_DEBUG
		facq_chunk_data_double_print(chunk);
#endif
		/* call the user function with the chunk and the user data */
		if(plug->priv->mts_func)
			ret = plug->priv->mts_func(chunk,plug->priv->mts_data);

		/* recycle the chunk */
		facq_log_write("M recycling chunk",FACQ_LOG_MSG_TYPE_DEBUG);
		facq_buffer_recycle(plug->priv->buf,chunk);
	}
	else return TRUE;

	/* TODO: if we get an error after processing the data maybe we should
	 * disconnect */
	if(!ret){
		facq_log_write("Error processing data",FACQ_LOG_MSG_TYPE_ERROR);
	}

	/* if we return FALSE the timeout source will not be processed any more */
	facq_log_write("M exiting timeout func",FACQ_LOG_MSG_TYPE_DEBUG);
	return ret;
}

static gboolean facq_plug_listen_callback(GSocket *skt,GIOCondition condition,gpointer oplug)
{
	FacqPlug *plug = FACQ_PLUG(oplug);
	GSocket *rej_skt = NULL;
	GError *local_err = NULL;

	if(condition & G_IO_IN){
		facq_plug_lock_client(plug);
		if(!plug->priv->clt_skt){
			plug->priv->clt_skt = g_socket_accept(skt,NULL,&local_err);
			facq_plug_unlock_client(plug);
			if(!plug->priv->clt_skt || local_err){
				if(local_err)
					facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
								"Error accepting client: %s",local_err->message);
				else
					facq_log_write("Unknown error accepting client",FACQ_LOG_MSG_TYPE_ERROR);
			}
			else {
				facq_plug_accept_client(plug);
				return TRUE;
			}
		}
		else {
			facq_plug_unlock_client(plug);
			facq_log_write("Rejecting connection",FACQ_LOG_MSG_TYPE_INFO);
			rej_skt = g_socket_accept(skt,NULL,&local_err);
			if(local_err){
				facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
							"Error when rejecting client: %s",
								local_err->message);
				g_clear_error(&local_err);
			}
			if(G_IS_SOCKET(rej_skt)){
				g_socket_shutdown(rej_skt,TRUE,TRUE,NULL);
				g_object_unref(G_OBJECT(rej_skt));
			}
		}
	}
	return TRUE;
}

/*****--- GInitable interface ---*****/
static void facq_plug_initable_iface_init(GInitableIface *iface)
{
	iface->init = facq_plug_initable_init;
}

static gboolean facq_plug_initable_init(GInitable *initable,GCancellable *cancellable,GError **error)
{
	FacqPlug *plug = NULL;

	g_return_val_if_fail(FACQ_IS_PLUG(initable),FALSE);

	plug = FACQ_PLUG(initable);
	if(cancellable != NULL){
		g_set_error_literal(error, FACQ_PLUG_ERROR,
					FACQ_PLUG_ERROR_FAILED,"Cancellable initialization not supported");
		return FALSE;
	}
	if(plug->priv->construct_error){
		if(error)
			*error = g_error_copy(plug->priv->construct_error);
		return FALSE;
	}
	return TRUE;
}

/*****--- public methods ---*****/
/**
 * facq_plug_new:
 * @address: The local address to bind.
 * @port: The port used by the service.
 * @fun: A #FacqPlugFunc. It will be called each time that new data arrives
 * from the client.
 * @fun_data: A pointer to some data that you want to pass to @fun function.
 * @timeout_ms: The minimum duration between calls to @fun, in miliseconds.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqPlug object. The #FacqPlug will start attending petitions
 * from clients after this function.
 *
 * Returns: A new #FacqPlug Object or %NULL in case of error.
 */
FacqPlug *facq_plug_new(const gchar *address,guint16 port,FacqPlugFunc fun,gpointer fun_data,guint timeout_ms,GError **err)
{
	return FACQ_PLUG(g_initable_new(FACQ_TYPE_PLUG,NULL,err,
					      "address",address,
					      "port",port,
					      "timeout-func",fun,
					      "timeout-data",fun_data,
					      "timeout",timeout_ms,
					      NULL));
}

/**
 * facq_plug_get_client_address:
 * @plug: A #FacqPlug object.
 * @err: A #GError it will be set in case of error if not %NULL.
 *
 * Gets the client IP address if any, else %NULL will be returned.
 *
 * Returns: The address (free it with g_free() ) or %NULL if no client
 * is connected.
 */
gchar *facq_plug_get_client_address(FacqPlug *plug,GError **err)
{
	GSocketAddress *sock_addr = NULL;
	GInetAddress *in_addr = NULL;
	GError *local_err = NULL;
	gchar *ret = NULL;

	g_return_val_if_fail(FACQ_IS_PLUG(plug),NULL);

	facq_plug_lock_client(plug);

	if(!plug->priv->clt_skt){
		facq_plug_unlock_client(plug);
		return NULL;
	}
	if(!g_socket_is_connected(plug->priv->clt_skt)){
		facq_plug_unlock_client(plug);
		return NULL;
	}
	else {
		sock_addr = 
			g_socket_get_remote_address(plug->priv->clt_skt,
						    &local_err);
		facq_plug_unlock_client(plug);
		if( (local_err && err != NULL) || !sock_addr){
			if(local_err && err != NULL)
				g_propagate_error(err,local_err);
			return NULL;
		}
		in_addr = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(sock_addr));
		ret = g_inet_address_to_string(in_addr);
		if(sock_addr)
			g_object_unref(G_OBJECT(sock_addr));
	}
	return ret;
}

/**
 * facq_plug_set_listen_address:
 * @plug: A #FacqPlug object.
 * @address: (allow-none): The ip address, all (ANY) if %NULL.
 * @port: The port.
 * @err: A #GError it will be set in case of error, if not %NULL.
 *
 * Changes the listen address and port of a #FacqPlug object. If address and
 * port are the same it does nothing and returns %TRUE. If address is %NULL it will take all the
 * possible addresses for IPV4.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_plug_set_listen_address(FacqPlug *plug,const gchar *address,guint16 port,GError **err)
{
	gint retcmp = 0;
	GError *local_err = NULL;
	
	g_return_val_if_fail(FACQ_IS_PLUG(plug),FALSE);

	retcmp = g_strcmp0(plug->priv->address,address);
	if(retcmp == 0 && port == plug->priv->port)
		return TRUE;
	else {
		/* check address */
		check_address(address,port,&local_err);
		if(local_err)
			goto error;
		/* stop listening on previous address */
		facq_plug_stop_listening(plug);
		/* disconnect any client */
		facq_plug_disconnect(plug);
		/* set the new address and port */
		if(plug->priv->address){
			g_free(plug->priv->address);
			plug->priv->address = NULL;
		}
		if(address)
			plug->priv->address = g_strdup(address);
		plug->priv->port = port;
		/* try to bind and listen with the new parameters */
		facq_plug_bind_and_listen(plug,&local_err);
		if(local_err)
			goto error;
	}
	return TRUE;

	error:
	if(err != NULL){
		if(local_err)
			g_propagate_error(err,local_err);
	}
	return FALSE;
}

/**
 * facq_plug_get_address:
 * @plug: A #FacqPlug object.
 *
 * Returns the address (The listen address) associated with the #FacqPlug.
 * If the plug was created with a %NULL address, it will return the string
 * ANY.
 *
 * Returns: The listen address, you must free it with g_free().
 */
gchar *facq_plug_get_address(const FacqPlug *plug)
{
	g_return_val_if_fail(FACQ_IS_PLUG(plug),NULL);

	if(plug->priv->address){
		return g_strdup(plug->priv->address);
	}
	else
		return g_strdup_printf("%s","all");
}

/**
 * facq_plug_get_port:
 * @plug: A #FacqPlug object.
 *
 * Gets the port where the plug is listening for connection petitions.
 *
 * Returns: The port.
 */
guint16 facq_plug_get_port(const FacqPlug *plug)
{
	g_return_val_if_fail(FACQ_IS_PLUG(plug),0);

	return plug->priv->port;
}

/**
 * facq_plug_disconnect:
 * @plug: A #FacqPlug object.
 *
 * If the #FacqPlug is on connected state, it will disconnect the client,
 * closing the connection. If no clients are connected it will do nothing.
 */
void facq_plug_disconnect(FacqPlug *plug)
{
	GError *local_err = NULL;
	FacqPlugMessage *msg = NULL;

	g_return_if_fail(FACQ_IS_PLUG(plug));

	/* check that we are connected else return */
	facq_plug_lock_client(plug);
	if(G_IS_SOCKET(plug->priv->clt_skt)){
		if(!g_socket_is_connected(plug->priv->clt_skt)){
			facq_plug_unlock_client(plug);
			return;
		}
	}
	else {
		facq_plug_unlock_client(plug);
		return;
	}

	/* connected, unlock so the producer can exit later */
	facq_plug_unlock_client(plug);

	/* Put a FacqPlugMessage to the mtop async queue with the disconnect type */
	msg = facq_plug_message_new(FACQ_PLUG_MESSAGE_TYPE_DISCONNECT,NULL);
	g_async_queue_push(plug->priv->mtop,msg);
	/* wait for the producer thread to exit */
	if(plug->priv->prod){
		facq_log_write("M waiting the exit of the P thread",FACQ_LOG_MSG_TYPE_DEBUG);
		g_thread_join(plug->priv->prod);
		plug->priv->prod = NULL;
	}
	facq_log_write("M continuing after exit of P Thread",FACQ_LOG_MSG_TYPE_DEBUG);

	/* destroy the client socket, no need to block the mutex here*/
	if(!g_socket_shutdown(plug->priv->clt_skt,TRUE,TRUE,&local_err)){
		if(local_err){
			facq_log_write(local_err->message,FACQ_LOG_MSG_TYPE_ERROR);
			g_clear_error(&local_err);
		}
		else
			facq_log_write("Error disconnecting",FACQ_LOG_MSG_TYPE_ERROR);
	}
	g_object_unref(G_OBJECT(plug->priv->clt_skt));
	plug->priv->clt_skt = NULL;

	/* destroy created objects when the client was accepted */
	facq_plug_disconnect_client(plug);
}

/**
 * facq_plug_get_stream_data:
 * @plug: A #FacqPlug object.
 *
 * Gets the stream associated #FacqStreamData object.
 *
 * Returns: A #FacqStreamData with details about the stream or %NULL
 * if not connected. You must call g_object_unref() 
 * on it when no longer needed.
 */
FacqStreamData *facq_plug_get_stream_data(FacqPlug *plug)
{
	g_return_val_if_fail(FACQ_IS_PLUG(plug),NULL);

	if(plug->priv->stmd)
		g_object_ref(plug->priv->stmd);

	return plug->priv->stmd;
}

/**
 * facq_plug_free:
 * @plug: A #FacqPlug object.
 *
 * Destroys a #FacqPlug object, disconnecting it if needed.
 */
void facq_plug_free(FacqPlug *plug)
{
	g_return_if_fail(FACQ_IS_PLUG(plug));
	facq_plug_disconnect(plug);
	g_object_unref(G_OBJECT(plug));
}

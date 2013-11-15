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
#include <string.h>
#include "facqlog.h"
#include "gdouble.h"
#include "facqnet.h"
#include "facqresources.h"
#include "facqchunk.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqoperation.h"
#include "facqoperationplug.h"

/*
 * G_SOCKET:
 *
 * ==== Common steps (Client and server) ====
 *
 * 1.- DNS->Address (CLient) | IP4/IP6 -> Address (Server) :
 * 
 * g_resolver_lookup_by_name(IP/NAME) or g_resolver_lookup_by_name_async() | GList
 * of GInetAddress sorted in order of preference and guaranteed to not contain
 * duplicates. If IP it will call g_inet_address_new_from_string() without doing
 * DNS.
 * 
 * 2.- Address+port -> GInetSocketAddress
 * Add the port information to the address information contained in
 * GInetAddress creating a GInetSocketAddress:
 * GSocketAddress *g_inet_socket_address_new(GInetAddress *address,guint16 port);
 *
 * 3.- Obtain FAMILY parameters for socket creation, from GSocketAddress:
 * GSocketFamily family = g_socket_address_get_family(Address);
 *
 * 4.- Create a GSocket attached to an address family but without an address in
 * the family:
 * GSocket *skt =
 *		g_socket_new(family,G_SOCKET_TYPE_STREAM,G_SOCKET_PROTOCOL_TCP);
 *
 * ==== Client steps ====
 * 5.- Connect to the server using the GSocket, if the GSocket "blocking"
 * property is TRUE it will block until finished:
 * gboolean g_socket_connect(skt,Address,NULL,&local_err);
 *
 * 6.- Send FacqStreamData to server, the call will block until there is space
 * for the data in the socket queue. We can wait for G_IO_OUT condition, anyway
 * we may receive G_IO_ERROR_WOULD_BLOCK even if previously notified of
 * G_IO_OUT.
 * gssize ret = g_socket_send(skt,buf,size,NULL,&local_err);
 *
 * ==== Server steps ====
 * We have 2 sockets here, the listen socket, and the socket returned by the
 * accept call. So the best approach would be to make the VI servers, and add a
 * GSource per socket handling all the events.
 *
 * 5.- Bind the address to the socket:
 * gboolean g_socket_bind(skt,Address,TRUE,&local_err);
 *
 * 6.- Mark the socket has a server socket:
 * g_socket_set_listen_backlog(skt,N); //N maximum pending connections 
 * gboolean g_socket_listen(skt,&local_err);
 *
 * 7.- Accept incoming connections on a connection-based socket. If the are no
 * outstanding connections the call will block ("blocking" == TRUE) or return
 * G_IO_ERROR_WOULD_BLOCK:
 * GSocket *g_socket_accept(skt,NULL,&local_err);
 *  
 */

/**
 * SECTION:facqoperationplug
 * @short_description: Provides an operation that allows the connection to VIs.
 * @title:FacqOperationPlug
 * @include:facqoperationplug.h
 *
 * #FacqOperationPlug provides a virtual plug where virtual instruments
 * (Applications that simulate measure instruments) can be connected, using the
 * loopback address of the computer or using a TCP/IP network, like a LAN or
 * internet.
 *
 * #FacqOperationPlug implements the #FacqOperation class.
 *
 * To create a new #FacqOperationPlug use facq_operation_plug_new(), to start
 * it use facq_operation_plug_start(), to stop it use facq_operation_plug_stop()
 * to use the main functionality of the operation (Sending data) use
 * facq_operation_plug_do(), finally to destroy it use
 * facq_operation_plug_free().
 *
 * If you want to write an application that receives data from this object you
 * are looking for #FacqPlug instead.
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * Internally a #GSocket is used to connect to the specified address and port
 * and send the #FacqStreamData and the samples to the other side.
 * </para>
 * </sect1>
 */

/**
 * FacqOperationPlug:
 *
 * Contains the private details of #FacqOperationPlug.
 */

/**
 * FacqOperationPlugClass:
 *
 * Class for the #FacqOperationPlug objects.
 */

/**
 * FacqOperationPlugError:
 * @FACQ_OPERATION_PLUG_ERROR_FAILED: Some error happened in the operation.
 *
 * Enum describing the different error values for #FacqOperationPlug.
 */

G_DEFINE_TYPE(FacqOperationPlug,facq_operation_plug,FACQ_TYPE_OPERATION);

enum {
	PROP_0,
	PROP_INST,
	PROP_PORT,
	PROP_ADDRESS
};

struct _FacqOperationPlugPrivate {
	gchar *address;
	guint16 port;
	GSocket *socket;
};

GQuark facq_operation_plug_error_quark(void)
{
	return g_quark_from_static_string("facq-operation-plug-error-quark");
}

/*****--- Gobject magic ---*****/
static void facq_operation_plug_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqOperationPlug *plug = FACQ_OPERATION_PLUG(self);

	switch(property_id){
	case PROP_PORT: g_value_set_uint(value,plug->priv->port);
	break;
	case PROP_ADDRESS: g_value_set_string(value,plug->priv->address);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(plug,property_id,pspec);
	}
}

static void facq_operation_plug_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqOperationPlug *plug = FACQ_OPERATION_PLUG(self);

	switch(property_id){
	case PROP_PORT: plug->priv->port = g_value_get_uint(value);
	break;
	case PROP_ADDRESS: plug->priv->address = g_value_dup_string(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(plug,property_id,pspec);
	}
}

static void facq_operation_plug_finalize(GObject *self)
{
	FacqOperationPlug *plug = FACQ_OPERATION_PLUG(self);

	if(plug->priv->address)
		g_free(plug->priv->address);

	if(plug->priv->socket)
		g_object_unref(G_OBJECT(plug->priv->socket));

	if (G_OBJECT_CLASS (facq_operation_plug_parent_class)->finalize)
                (*G_OBJECT_CLASS (facq_operation_plug_parent_class)->finalize) (self);
}

static void facq_operation_plug_class_init(FacqOperationPlugClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	FacqOperationClass *operation_class = FACQ_OPERATION_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqOperationPlugPrivate));

	object_class->set_property = facq_operation_plug_set_property;
	object_class->get_property = facq_operation_plug_get_property;
	object_class->finalize = facq_operation_plug_finalize;

	operation_class->opsave = facq_operation_plug_to_file;
	operation_class->opstart = facq_operation_plug_start;
	operation_class->opdo = facq_operation_plug_do;
	operation_class->opstop = facq_operation_plug_stop;
	operation_class->opfree = facq_operation_plug_free;

	g_object_class_install_property(object_class,PROP_ADDRESS,
					g_param_spec_string("address",
							    "The address",
							    "The VI address",
							    "localhost",
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));

	/* remember that port 0 is always reserved and can't be used */
	g_object_class_install_property(object_class,PROP_PORT,
					g_param_spec_uint("port",
							  "The port",
							  "The VI port",
							  1,
							  65536,
							  3000,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));
}

static void facq_operation_plug_init(FacqOperationPlug *plug)
{
	plug->priv = G_TYPE_INSTANCE_GET_PRIVATE(plug,FACQ_TYPE_OPERATION_PLUG,FacqOperationPlugPrivate);
	plug->priv->socket = NULL;
}

/*****--- Public methods ---*****/
/**
 * facq_operation_plug_to_file:
 * @op: A #FacqOperationPlug casted to #FacqOperation.
 * @file: A #GKeyFile object.
 * @group: The group name inside the #GKeyFile, @file.
 *
 * Implements the facq_operation_to_file() method.
 * Stores the address and the port where the operation will try to
 * connect when is started. This allows to recreate the #FacqOperationPlug
 * later.
 * This is used by facq_stream_save() function, and you shouldn't need to calls
 * this.
 */
void facq_operation_plug_to_file(FacqOperation *op,GKeyFile *file,const gchar *group)
{
	FacqOperationPlug *plug = FACQ_OPERATION_PLUG(op);

	g_return_if_fail(g_key_file_has_group(file,group));

	g_key_file_set_string(file,group,"address",plug->priv->address);
	g_key_file_set_double(file,group,"port",plug->priv->port);
}

/**
 * facq_operation_plug_key_constructor:
 * @group_name: A string with the group name.
 * @key_file: A #GKeyFile object.
 * @err: (allow-none): A #GError it will be set in case of error if not %NULL.
 *
 * It's purpose it's to create a new #FacqOperationPlug object from a #GKeyFile,
 * @key_file, and a @group_name. This function is used by #FacqCatalog. See
 * #CIKeyConstructor for more details.
 *
 * Returns: %NULL in case of error, or a new #FacqOperationPlug object if
 * successful.
 */
gpointer facq_operation_plug_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err)
{
	GError *local_err = NULL;
	gchar *address = NULL;
	guint16 port = 3000;
	gpointer op = NULL;

	address = g_key_file_get_string(key_file,group_name,"address",&local_err);
	if(local_err)
		goto error;

	port = (guint16) g_key_file_get_double(key_file,group_name,"port",&local_err);
	if(local_err)
		goto error;

	op = facq_operation_plug_new(address,port);
	
	g_free(address);

	return op;

	error:
	if(local_err){
		if(err)
			g_propagate_error(err,local_err);
	}
	return NULL;
}

/**
 * facq_operation_plug_constructor:
 * @user_input: A #GPtrArray with the parameters from the user.
 * @err: A #GError, it will be used in case of error if not %NULL.
 *
 * Creates a new #FacqOperationPlug object from a #GPtrArray, @user_input,
 * with at least 2 pointers, the first a pointer to the address, the second a 
 * pointer to a guint with the port number. See facq_operation_plug_new() for
 * valid values.
 *
 * This function is used by #FacqCatalog, for creating a #FacqOperationPlug
 * object with the parameters provided by the user in a #FacqDynDialog, take a
 * look to these other objects for more details, and to the #CIConstructor type.
 *
 * Returns: A new #FacqOperationPlug object, or %NULL in case of error.
 */
gpointer facq_operation_plug_constructor(const GPtrArray *user_input,GError **err)
{
	gchar *address = NULL;
	guint *port = NULL;

	address = g_ptr_array_index(user_input,0);
	port = g_ptr_array_index(user_input,1);

	return facq_operation_plug_new(address,*port);
}

/**
 * facq_operation_plug_new:
 * @address: An IP address or hostname.
 * @port: The port value.
 *
 * Creates a new #FacqOperationPlug with the requested address, @address and
 * port, @port.
 *
 * Returns: A new #FacqOperationPlug object.
 */
FacqOperationPlug *facq_operation_plug_new(const gchar *address,guint16 port)
{
	return FACQ_OPERATION_PLUG(g_object_new(FACQ_TYPE_OPERATION_PLUG,
						"name",facq_resources_names_operation_plug(),
						"description",facq_resources_descs_operation_plug(),
						"address",address,
						"port",port,
						NULL) );
}

/**
 * facq_operation_plug_start:
 * @op: A #FacqOperationPlug casted to #FacqOperation.
 * @stmd: A #FacqStreamData with the stream relevant information.
 * @err: A #GError it will be set in case of error if not %NULL.
 *
 * Starts the #FacqOperationPlug. That means that #FacqOperationPlug
 * will try to establish a connection with the requested address and port,
 * if successful, a #FacqStreamData object will be send to the other
 * side using facq_stream_data_to_socket() function.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_operation_plug_start(FacqOperation *op,const FacqStreamData *stmd,GError **err)
{
	FacqOperationPlug *plug = NULL;
	GResolver *def = NULL;
	GSocketAddress *address = NULL;
	GList *address_list = NULL;
	GError *local_err = NULL;
	gboolean try = TRUE;
	GSocketFamily family;

	plug = FACQ_OPERATION_PLUG(op);

	if(plug->priv->socket)
		g_object_unref(G_OBJECT(plug->priv->socket));
	plug->priv->socket = NULL;

	def = g_resolver_get_default();
	address_list = g_resolver_lookup_by_name(def,plug->priv->address,NULL,&local_err);
	g_object_unref(G_OBJECT(def));
	if(local_err)
		goto error;
	
	while(address_list && try){
		address =
			g_inet_socket_address_new(
					G_INET_ADDRESS(address_list->data),
									plug->priv->port);
		family = g_socket_address_get_family(address);
		plug->priv->socket = 
			g_socket_new(family,G_SOCKET_TYPE_STREAM,G_SOCKET_PROTOCOL_TCP,&local_err);
			if(local_err){
				facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
							"Error creating socket: %s",
								local_err->message);
				g_clear_error(&local_err);
				break;
			}
		if(!g_socket_connect(plug->priv->socket,address,NULL,&local_err)){
			if(local_err){
				facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
							"Error connecting: %s",local_err->message);
				g_clear_error(&local_err);
			}
			address_list = g_list_next(address_list);
			g_object_unref(G_OBJECT(plug->priv->socket));
			plug->priv->socket = NULL;
		}
		else try=FALSE;
	}
	g_resolver_free_addresses(address_list);
	if(!plug->priv->socket){
		g_set_error_literal(&local_err,FACQ_OPERATION_PLUG_ERROR,
					FACQ_OPERATION_PLUG_ERROR_FAILED,"Error connecting to VI");
		goto error;
	}

	if(!facq_stream_data_to_socket(stmd,plug->priv->socket,&local_err)){
		if(local_err)
			facq_log_write(local_err->message,FACQ_LOG_MSG_TYPE_ERROR);
		goto error;
	}

	return TRUE;

	error:
	if(plug->priv->socket){
		g_object_unref(G_OBJECT(plug->priv->socket));
		plug->priv->socket = NULL;
	}
	if(local_err){
		if(err)
			g_propagate_error(err,local_err);
	}
	else {
		g_set_error_literal(&local_err,FACQ_OPERATION_PLUG_ERROR,
					FACQ_OPERATION_PLUG_ERROR_FAILED,
						"Unknown error while starting the plug");
		g_propagate_error(err,local_err);
	}
	return FALSE;
}

/**
 * facq_operation_plug_do:
 * @op: A #FacqOperationPlug casted to #FacqOperation.
 * @chunk: A #FacqChunk containing the samples.
 * @stmd: A #FacqStreamData with the relevant stream information.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Sends the data contained in the #FacqChunk, in big endian format, to the
 * other side of the connection, using the facq_net_send() function.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_operation_plug_do(FacqOperation *op,FacqChunk *chunk,const FacqStreamData *stmd,GError **err)
{
	FacqOperationPlug *plug = FACQ_OPERATION_PLUG(op);
	gssize ret = 0;
	gsize used_bytes = 0;
	GError *local_err = NULL;

	used_bytes = facq_chunk_get_used_bytes(chunk);

#if ENABLE_DEBUG
	facq_chunk_data_double_print(chunk);
#endif

	facq_chunk_data_double_to_be(chunk);
	ret = facq_net_send(plug->priv->socket,
			    chunk->data,
			    used_bytes,
			    3,&local_err);
	facq_chunk_data_double_to_be(chunk);

	/* in case of error ignore it, cause is not critial, the stream
	 * can continue in case the VI is closed */
	if(ret != used_bytes || local_err){
		if(local_err){
			facq_log_write(local_err->message,FACQ_LOG_MSG_TYPE_ERROR);
			g_clear_error(&local_err);
		}
	}

	return TRUE;
}

/**
 * facq_operation_plug_stop:
 * @op: A #FacqOperationPlug object casted to #FacqOperation.
 * @stmd: A #FacqStreamData containing the relevant properties of the stream.
 * @err: A #GError it will be set in case of error if not %NULL.
 *
 * Stops a previously started #FacqOperationPlug operation, the socket is
 * shutdown and destroyed.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_operation_plug_stop(FacqOperation *op,const FacqStreamData *stmd,GError **err)
{
	FacqOperationPlug *plug = FACQ_OPERATION_PLUG(op);

	if(G_IS_SOCKET(plug->priv->socket)){
		g_socket_shutdown(plug->priv->socket,TRUE,TRUE,NULL);
		g_object_unref(G_OBJECT(plug->priv->socket));
	}
	plug->priv->socket = NULL;

	return TRUE;
}

/**
 * facq_operation_plug_free:
 * @op: A #FacqOperationPlug object.
 *
 * Destroys a no longer needed #FacqOperationPlug object.
 */
void facq_operation_plug_free(FacqOperation *op)
{
	g_return_if_fail(FACQ_IS_OPERATION_PLUG(op));
	g_object_unref(G_OBJECT(op));
}

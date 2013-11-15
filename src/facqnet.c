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
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <glib.h>
#include <gio/gio.h>
#include "facqlog.h"
#include "facqnet.h"

/**
 * SECTION:facqnet
 * @short_description:Net related functions.
 * @title:FacqNet
 * @include:facqnet.h
 *
 * This module contains functions related with the network functions, for
 * example for sending and receiving data.
 *
 * Only two functions are provided for the moment facq_net_send() and 
 * facq_net_receive(), check the description of each function for more details.
 *
 */
static gboolean check_values(GSocket *skt,gchar *buf,gsize size)
{
	if(!G_IS_SOCKET(skt))
		goto error;
	if(!buf)
		goto error;
	if(size >= G_MAXSSIZE)
		goto error;
	return TRUE;

	error:
	facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
				"Invalid parameters in send or receive function");
	return FALSE;
}

/**
 * facq_net_send:
 * @skt: A connected #GSocket object.
 * @buf: A pointer to the memory area that contains the data that you want to
 * send.
 * @size: The size of the memory area pointed by @buf, in bytes.
 * @retry: The number of retries or 0 for infinite retries.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Tries to send the number of bytes, @size, in the memory area pointed by @buf,
 * using at most @retry retries.
 *
 * returns: The number of bytes sent if successful, -1 in case of error, 
 * -2 in case of timeout, -3 if wrong parameters are passed to the function.
 */
gssize facq_net_send(GSocket *skt,gchar *buf,gsize size,guint retry,GError **err)
{
	guint retries = 0;
	GError *local_err = NULL;
	gsize to_send = 0, remaining = 0;
	gssize ret = 0, total = 0;

	if(!check_values(skt,buf,size)){
		if(err != NULL)
			g_set_error_literal(err,G_IO_ERROR,
						G_IO_ERROR_FAILED,"Can't send any data");
		return -3;
	}

	if(retry == 0)
		retry = G_MAXUINT;

	to_send = 0;
	remaining = size;

	/* Try to send all the data in one call, retrying at most retry times */
	for(retries = 0; retries < retry; retries++){
		ret = g_socket_send(skt,&buf[to_send],remaining,NULL,&local_err);
		if(ret <= 0){
			if(local_err){
				facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
							"%s",local_err->message);
				if(err != NULL)
					g_propagate_error(err,local_err);
				return -1;
			}
			else {
				if(err != NULL)
					g_set_error_literal(err,G_IO_ERROR,
								G_IO_ERROR_FAILED,"Error sending data");
				return -1;
			}
		}
		else {
			total += ret;
			remaining = size - total;
			to_send += ret;
			if(total == size)
				break;
#if ENABLE_DEBUG
			facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,"%s",
					"Retrying to send all the data");
#endif
		}
	}

	if(total != size){
		if(err != NULL)
			g_set_error_literal(err,G_IO_ERROR,
						G_IO_ERROR_TIMED_OUT,"Operation timed out");
		return -2;
	}

	return total;
}

/**
 * facq_net_receive:
 * @skt: A connected #GSocket object.
 * @buf: A pointer to a free memory area.
 * @size: The size of the memory area pointed by @buf.
 * @retry: The number of times to retry receiving data until @buf is full, or 0.
 * if 0 the function will use an automatic number of retries, and won't return
 * until size bytes are read.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Receives @size bytes, in the memory area pointed by @buf, from the connect
 * socket @skt, using at most @retry retries.
 *
 * Returns: The number of bytes received if successful, 0 if disconnected, -1 in case of error, 
 * -2 in case of timeout, -3 if wrong parameters are passed to the function.
 */
gssize facq_net_receive(GSocket *skt,gchar *buf,gsize size,guint retry,GError **err)
{
	guint retries = 0;
	gsize to_read = 0, remaining = 0;
	gssize ret = 0, total = 0;
	GError *local_err = NULL;

	if(!check_values(skt,buf,size)){
		if(err != NULL)
			g_set_error_literal(err,G_IO_ERROR,
						G_IO_ERROR_FAILED,"Can't receive any data");
		return -3;
	}

	if(retry == 0)
		retry = G_MAXUINT;

	to_read = 0;
	remaining = size;

	for(retries = 0; retries < retry && total < size; retries++){
#if ENABLE_DEBUG
		facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,"Receiving data retry=%u",retries);
#endif
		ret = g_socket_receive(skt,&buf[to_read],remaining,NULL,&local_err);
#if ENABLE_DEBUG
		facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
				"g_socket_receive ret=%"G_GSSIZE_FORMAT,
				ret);
#endif
		switch(ret){
		case 0:
			facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,
							"Disconnected");
			return 0;
		break;
		case -1:
			if(local_err){
				facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
							"%s",local_err->message);
				if(err != NULL)
					g_propagate_error(err,local_err);
				return -1;
			}
			else {
				if(err != NULL)
					g_set_error_literal(err,G_IO_ERROR,
								G_IO_ERROR_FAILED,"Error receiving data");
				facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
								"Unknown error receiving data");
				return -1;
			}
		break;
		default:
			total += ret;
			to_read += ret;
			remaining = size - total;
			if(total == size)
				break;
		}
	}

	if(total != size){
		if(err != NULL)
			g_set_error_literal(err,G_IO_ERROR,
						G_IO_ERROR_TIMED_OUT,"Operation timed out");
		return -2;
	}

	return total;
}

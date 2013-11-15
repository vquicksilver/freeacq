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
#ifndef _FREEACQ_GLIB_COMPAT_H
#define _FREEACQ_GLIB_COMPAT_H

G_BEGIN_DECLS

#if GLIB_MINOR_VERSION < 32

gboolean g_socket_condition_timed_wait(GSocket *socket,GIOCondition condition,gint64 timeout,GCancellable *cancellable,GError **error);
GThread *g_thread_try_new(const gchar *name,GThreadFunc func,gpointer data,GError **error);
gboolean g_cond_wait_until(GCond *cond,GMutex *mutex,gint64 end_time);
void g_queue_free_full(GQueue *queue,GDestroyNotify free_func);
gpointer g_async_queue_timeout_pop(GAsyncQueue *queue,guint64 timeout);

#if GLIB_MINOR_VERSION < 30

#define G_VALUE_INIT  { 0, { { 0 } } }

#ifdef G_OS_UNIX

#define G_UNIX_ERROR (g_unix_error_quark())

GQuark g_unix_error_quark (void);

gboolean g_unix_set_fd_nonblocking(gint fd,gboolean nonblock,GError **error);
#endif  /*G_OS_UNIX*/

#if GLIB_MINOR_VERSION < 28

void g_list_free_full(GList *list,GDestroyNotify free_func);
gint64 g_get_monotonic_time(void);

#if GLIB_MINOR_VERSION < 26

#if GLIB_MINOR_VERSION < 24

gpointer g_malloc0_n(gsize n_blocks,gsize n_block_bytes);

#endif // < 24

#define G_TIME_SPAN_SECOND (G_GINT64_CONSTANT(1000000)) 

#endif // < 26

#endif // < 28

#endif // < 30

#endif // < 32

G_END_DECLS

#endif

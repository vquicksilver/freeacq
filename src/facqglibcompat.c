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
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "facqglibcompat.h"


#ifdef G_OS_WIN32
#include <fcntl.h>
#include <unistd.h>
#endif

#if GLIB_MINOR_VERSION < 32

/*
 * g_socket_condition_timed_wait:
 *
 * Provides g_socket_condition_timed_wait function for older 
 * glib versions. It's a simplified version of the glib one
 * that should work with all glib version from glib-2.22.
 */
gboolean g_socket_condition_timed_wait(GSocket *socket,GIOCondition condition,gint64 timeout,GCancellable *cancellable,GError **error)
{
	gint64 start_time;
	GPollFD poll_fd[2];
	gint result;
	gint num;

	g_return_val_if_fail(G_IS_SOCKET(socket),FALSE);

	if(g_cancellable_set_error_if_cancelled(cancellable,error))
		return FALSE;

	if(timeout != -1)
		timeout /= 1000;

	start_time = g_get_monotonic_time();

	poll_fd[0].fd = g_socket_get_fd(socket);
	poll_fd[0].events = condition;
	num = 1;

	if(g_cancellable_make_pollfd(cancellable,&poll_fd[1]))
		num++;

	while(TRUE){
		result = g_poll(poll_fd,num,timeout);
		if(result != -1 || errno != EINTR)
			break;

		if(timeout != -1){
			timeout -= (g_get_monotonic_time () - start_time) * 1000;
			if(timeout < 0)
				timeout = 0;
		}
	}
    
	if(num > 1)
		g_cancellable_release_fd(cancellable);

	if(result == 0){
		g_set_error_literal(error,G_IO_ERROR,G_IO_ERROR_TIMED_OUT,
								"Socket I/O timed out");
		return FALSE;
	}

	return !g_cancellable_set_error_if_cancelled(cancellable,error);
}

/*
 * g_thread_try_new:
 *
 * Just wrap g_thread_create, name is ignored.
 */
GThread *g_thread_try_new(const gchar *name,GThreadFunc func,gpointer data,GError **error)
{
	GThread *new_thread = NULL;

	new_thread = g_thread_create(func,data,TRUE,error);
	return new_thread;
}

/*
 * g_cond_wait_until:
 *
 * Just wrap g_cond_timed_wait.
 */
gboolean g_cond_wait_until(GCond *cond,GMutex *mutex,gint64 end_time)
{
	GTimeVal time;

	g_get_current_time(&time);
	g_time_val_add(&time,G_USEC_PER_SEC);
	return g_cond_timed_wait(cond,mutex,&time);
}

void g_queue_free_full(GQueue *queue,GDestroyNotify free_func)
{
	g_queue_foreach(queue,(GFunc)free_func, NULL);
  	g_queue_free(queue);
}

gpointer g_async_queue_timeout_pop(GAsyncQueue *queue,guint64 timeout)
{
	GTimeVal time;

	g_get_current_time(&time);
	g_time_val_add(&time,(glong)timeout);
	return g_async_queue_timed_pop(queue,&time);
}

#if GLIB_MINOR_VERSION < 30

#ifdef G_OS_UNIX
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

GQuark g_unix_error_quark (void)
{  
	return g_quark_from_static_string ("g-unix-error-quark");
}

static 
gboolean g_unix_set_error_from_errno(GError **error,gint saved_errno)
{
	g_set_error_literal(error,
                       	    G_UNIX_ERROR,
                       	    0,
                       	    g_strerror(saved_errno));
  	errno = saved_errno;
  	return FALSE;
}

gboolean g_unix_set_fd_nonblocking(gint fd,gboolean nonblock,GError **error)
{
#ifdef F_GETFL
	glong fcntl_flags;
	fcntl_flags = fcntl(fd,F_GETFL);
	
	if(fcntl_flags == -1)
    		return g_unix_set_error_from_errno(error,errno);

  	if(nonblock){
#ifdef O_NONBLOCK
      		fcntl_flags |= O_NONBLOCK;
#else
      		fcntl_flags |= O_NDELAY;
#endif
    	}
  	else {
#ifdef O_NONBLOCK
      		fcntl_flags &= ~O_NONBLOCK;
#else
      		fcntl_flags &= ~O_NDELAY;
#endif
    	}
  	if(fcntl (fd, F_SETFL, fcntl_flags) == -1)
    		return g_unix_set_error_from_errno (error, errno);
  	return TRUE;
#else
  	return g_unix_set_error_from_errno (error, EINVAL);
#endif
}
#endif //G_OS_UNIX

#if GLIB_MINOR_VERSION < 28

void g_list_free_full(GList *list,GDestroyNotify free_func)
{
	g_list_foreach (list, (GFunc) free_func, NULL);
	g_list_free (list);
}

#ifdef G_OS_WIN32
#define STRICT
#include <windows.h>
static ULONGLONG (*g_GetTickCount64) (void) = NULL;
static guint32 g_win32_tick_epoch = 0;

G_GNUC_INTERNAL void
g_clock_win32_init (void)
{
  HMODULE kernel32;

  g_GetTickCount64 = NULL;
  kernel32 = GetModuleHandle ("KERNEL32.DLL");
  if (kernel32 != NULL)
    g_GetTickCount64 = (void *) GetProcAddress (kernel32, "GetTickCount64");
  g_win32_tick_epoch = ((guint32)GetTickCount()) >> 31;
}
#endif /* G_OS_WIN32 */

gint64 g_get_monotonic_time(void){
#ifdef HAVE_CLOCK_GETTIME
  /* librt clock_gettime() is our first choice */
	struct timespec ts;
#ifdef CLOCK_MONOTONIC
	clock_gettime (CLOCK_MONOTONIC, &ts);
#else
  	clock_gettime (CLOCK_REALTIME, &ts);
#endif
	g_assert(G_GINT64_CONSTANT(-315569520000000000) < ts.tv_sec &&
		ts.tv_sec < G_GINT64_CONSTANT (315569520000000000));
	return (((gint64) ts.tv_sec) * 1000000) + (ts.tv_nsec / 1000);

#elif defined (G_OS_WIN32)
	guint64 ticks;
  	guint32 ticks32;
      	guint32 ticks_as_32bit;
      	guint32 epoch;
	
	if(g_GetTickCount64 != NULL){
	ticks = g_GetTickCount64 ();
      	ticks32 = timeGetTime();

      
      	ticks_as_32bit = (guint32)ticks;
	if(ticks32 - ticks_as_32bit <= G_MAXINT32)
		ticks += ticks32 - ticks_as_32bit;
      	else
		ticks -= ticks_as_32bit - ticks32;
    	}
  	else {
		epoch = g_atomic_int_get(&g_win32_tick_epoch);
      		ticks32 = timeGetTime();
		if((ticks32 >> 31) != (epoch & 1)){
	  		epoch++;
	  		g_atomic_int_set (&g_win32_tick_epoch, epoch);
		}
		ticks = (guint64)ticks32 | ((guint64)epoch) << 31;
    	}

  	return ticks * 1000;
#else /* !HAVE_CLOCK_GETTIME && ! G_OS_WIN32*/
	GTimeVal tv;

  	g_get_current_time (&tv);

  	return (((gint64) tv.tv_sec) * 1000000) + tv.tv_usec;
#endif
}

#if GLIB_MINOR_VERSION < 26

#if GLIB_MINOR_VERSION < 24

#define SIZE_OVERFLOWS(a,b) (G_UNLIKELY ((b) > 0 && (a) > G_MAXSIZE / (b)))

gpointer g_malloc0_n(gsize n_blocks,gsize n_block_bytes)
{ 
	if(SIZE_OVERFLOWS(n_blocks,n_block_bytes)){
		g_error ("%s: overflow allocating %"G_GSIZE_FORMAT"*%"G_GSIZE_FORMAT" bytes",
               G_STRLOC,n_blocks,n_block_bytes);
    	}
   	return g_malloc0(n_blocks * n_block_bytes);
}

#endif // < 24

#endif // < 26

#endif // < 28

#endif // < 30

#endif // < 32

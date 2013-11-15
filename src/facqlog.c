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
#ifdef G_OS_UNIX
#include <sys/stat.h>
#include <syslog.h>
#endif
#include "facqlog.h"

/**
 * SECTION:facqlog
 * @title: FacqLog
 * @short_description: A logging service provider.
 * @include: facqlog.h
 *
 * FacqLog provides a way to handle and control messages.
 * A message is a string of #FacqLogMsgType type that usually
 * contains debug information, general information, a warning, 
 * or an error message.
 *
 * Using FacqLog messages can be redirected to various kind of
 * outputs, as seen in #FacqLogOut, note that the syslog output it's 
 * only available on UNIX systems.
 *
 * When the output is written to a file a new file is created if it doesn't
 * exist. The name of this file depends on the name of the program, for
 * example for a program called fuu it will be called fuu.log.
 *
 * The path where this file is created depends on the platform where the
 * code is running. If the code is running on unix a new folder with
 * the program name will be created in the user home directory, for
 * example on Linux for a user called john, and a program called Foo 
 * the path will be /home/john/.Foo/log/. If the code is running
 * on a Windows platform the path is determined by the 
 * CSIDL_LOCAL_APPDATA variable, so for a user called john it will be
 * C:\Documents and Settings\username\Local Settings\Application Data\Foo\log\ .
 * Specifications defining XDG variables and CSIDL variables can be found
 * in the <ulink url="http://standards.freedesktop.org/basedir-spec/latest/ar01s03.html">Freedesktop site</ulink>
 * and in <ulink url="http://msdn.microsoft.com/en-us/library/bb762494.aspx">Microsoft site</ulink> , anyway
 * it shouldn't be needed as long as glib handles these internal details.
 *
 * All operations in #FacqLog are thread safe, and can be called from multiple
 * threads at the same time.
 *
 * For using FacqLog first you need to initialize it using facq_log_enable(),
 * after this you can setup the log mask using facq_log_set_mask(). This mask
 * help you filter what kind of message are allowed to reach the enabled
 * outputs and can be used for filter debugging messages that shouldn't be seen
 * by the final users. 
 *
 * By default all the outputs are disabled. You can enable or disable (Depending on the previous state) 
 * one or more outputs using facq_log_toggle_out().
 *
 * To write a message to all the enabled outputs you can use facq_log_write()
 * or facq_log_write_v().
 *
 * When FacqLog is no longer needed you should call facq_log_disable().
 *
 * <example>
 *  <title>Using FacqLog to write to stdout</title>
 *  <programlisting>
 *  #include <glib.h>
 *  #include <gio/gio.h>
 *  #include "facqlog.h"
 *  ...
 *
 *  int main(int argc,char **argv)
 *  {
 *     ...
 *     facq_log_enable();
 *     facq_log_set_mask(FACQ_LOG_MSG_TYPE_WARNING);
 *     facq_log_toggle_out(FACQ_LOG_OUT_STDOUT);
 *     ...
 *     //this can be called from any thread without worries
 *     facq_log_write("A debug message that will be ignored",FACQ_LOG_MSG_TYPE_DEBUG);
 *     facq_log_write("An info message",FACQ_LOG_MSG_TYPE_INFO);
 *     ...
 *     facq_log_disable();
 *     return 0;
 *  }
 *  </programlisting>
 * </example>
 */

/**
 * FacqLogError:
 * @FACQ_LOG_ERROR_FAILED: Some error happened in the #FacqLog.
 *
 * Enum containing all the error values for #FacqLog.
 */

static FacqLog *FACQ_LOG_PRIV = NULL;

#if GLIB_MINOR_VERSION > 24
static gint LOCK = 0;
#define FACQ_LOG_LOCK g_bit_lock(&LOCK,0);
#define FACQ_LOG_UNLOCK g_bit_unlock(&LOCK,0);
#else
G_LOCK_DEFINE(lock);
#define FACQ_LOG_LOCK G_LOCK(lock);
#define FACQ_LOG_UNLOCK G_UNLOCK(lock);
#endif

G_DEFINE_TYPE(FacqLog,facq_log,G_TYPE_OBJECT);

GQuark facq_log_error_quark(void)
{
	return g_quark_from_static_string("facq-log-error-quark");
}

enum {
	PROP_0
};

struct _FacqLogPrivate {
	FacqLogMsgType mask;
	GIOChannel *log_channel;
	gboolean out_stdout;
	gboolean out_stderr;
	gboolean out_file;
#ifdef G_OS_UNIX
	gboolean out_syslog;
#endif
};

/* GObject magic */
static void facq_log_finalize(GObject *self)
{
	FacqLog *log = FACQ_LOG(self);

	if(log->priv->log_channel){
		g_io_channel_shutdown(log->priv->log_channel,TRUE,NULL);
		g_io_channel_unref(log->priv->log_channel);
	}

#ifdef G_OS_UNIX
	if(log->priv->out_syslog)
		closelog();
#endif
		
	if(G_OBJECT_CLASS(facq_log_parent_class)->finalize)
    		(*G_OBJECT_CLASS(facq_log_parent_class)->finalize)(self);
}

static void facq_log_class_init(FacqLogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqLogPrivate));

	object_class->finalize = facq_log_finalize;
}

static void facq_log_init(FacqLog *log)
{
	log->priv = G_TYPE_INSTANCE_GET_PRIVATE(log,FACQ_TYPE_LOG,FacqLogPrivate);
	log->priv->mask = FACQ_LOG_MSG_TYPE_WARNING;
	log->priv->log_channel = NULL;
	log->priv->out_stdout = FALSE;
	log->priv->out_stderr = FALSE;
	log->priv->out_file = FALSE;
#ifdef G_OS_UNIX
	log->priv->out_syslog = FALSE;
#endif
}

/* private methods */
static void facq_log_create_log_file(const gchar *filename)
{
	gchar *path = NULL;
	gint ret = 0;

	path = g_path_get_dirname(filename);
#ifdef G_OS_UNIX
	ret = g_mkdir_with_parents(path,S_IRWXU|S_IRGRP|S_IXGRP);
#else
	//on windows systems glib calls _wmkdir, mode is ignored
	ret = g_mkdir_with_parents(path,0);
#endif
	g_assert(ret == 0);
}

gchar *facq_log_get_date_time(void)
{
	gchar *ret = NULL;
#if GLIB_MINOR_VERSION >= 26
	GDateTime *time;

	time = g_date_time_new_now_local();
	ret = g_date_time_format(time,"%a %b %d %H:%M:%S %Y");
	g_date_time_unref(time);
#else
	GTimeVal time_val;

	g_get_current_time(&time_val);
	//discard micro seconds
	time_val.tv_usec = 0;
	ret = g_time_val_to_iso8601(&time_val);
#endif
	return ret;
}

static void facq_log_toggle_stdout(FacqLog *log)
{
	log->priv->out_stdout = !log->priv->out_stdout;
}

static void facq_log_toggle_stderr(FacqLog *log)
{
	log->priv->out_stderr = !log->priv->out_stderr;
}

static void facq_log_toggle_file(FacqLog *log,GError **err)
{
	GIOStatus ret = G_IO_STATUS_NORMAL;
	gchar *filename = NULL;

	if(log->priv->out_file)
		ret = g_io_channel_flush(log->priv->log_channel,err);

	log->priv->out_file = !log->priv->out_file;
	
	if(ret != G_IO_STATUS_NORMAL)
		return;

	if(log->priv->out_file && !log->priv->log_channel){
		filename = facq_log_get_filename();
		facq_log_create_log_file(filename);
		log->priv->log_channel = 
			g_io_channel_new_file(filename,"a",err);
		g_free(filename);
		if(!log->priv->log_channel)
			log->priv->out_file = FALSE;
	}
}

#ifdef G_OS_UNIX
static void facq_log_toggle_syslog(FacqLog *log)
{
	if(log->priv->out_syslog)
		closelog();

	log->priv->out_syslog = !log->priv->out_syslog;

	if(log->priv->out_syslog)
		openlog(NULL,LOG_NDELAY | LOG_PID,LOG_DAEMON);
}
#endif

static const gchar *facq_log_msg_type_to_human(FacqLogMsgType type)
{
	const gchar * const msg_types[] =
	{
		"DEBUG" , "INFO", "WARNING", "ERROR"
	};

	if(type < FACQ_LOG_MSG_TYPE_N){
		return msg_types[type];
	}
	else 
		return NULL;
}

static FacqLog *facq_log_new(void)
{
	return g_object_new(FACQ_TYPE_LOG,NULL);
}

static void facq_log_free(FacqLog *log)
{
	g_return_if_fail(FACQ_IS_LOG(log));
	g_object_unref(G_OBJECT(log));
}

/* public methods */
/**
 * facq_log_enable:
 *
 * You should call this function first before using any of the other logging
 * functions. It's purpose it's to initialize the internal logging mechanism.
 *
 * After calling this function you should enable one or more logging outputs
 * with facq_log_toggle_out().
 */
void facq_log_enable(void)
{
	FACQ_LOG_LOCK
	
	if(!FACQ_IS_LOG(FACQ_LOG_PRIV))
		FACQ_LOG_PRIV = facq_log_new();
	
	FACQ_LOG_UNLOCK
}

/**
 * facq_log_set_mask:
 * @mask: The minimum type of message that is allowed to reach the logging
 * outputs. %FACQ_LOG_MSG_TYPE_DEBUG allows all the types. See #FacqLogMsgType
 * for possible values and order.
 *
 * Set's the logging mask. The default mask allows only warning and error
 * messages to reach the logging outputs, so the default value is
 * %FACQ_LOG_MSG_TYPE_WARNING.
 */
void facq_log_set_mask(FacqLogMsgType mask)
{
	FACQ_LOG_LOCK

	if(FACQ_IS_LOG(FACQ_LOG_PRIV)){
		if(mask < FACQ_LOG_MSG_TYPE_N)
			FACQ_LOG_PRIV->priv->mask = mask;
	}

	FACQ_LOG_UNLOCK
}

/**
 * facq_log_toggle_out:
 * @out: The output you want to toggle. See #FacqLogOut for valid values.
 * @err: (allow-none): A #GError, or %NULL. If not %NULL will be set in case of
 * error.
 * 
 * Toggles the output, @out, enabling or disabling it depending on the previous
 * state of the output. 
 */
void facq_log_toggle_out(FacqLogOut out,GError **err)
{
	FACQ_LOG_LOCK
	
	if(!FACQ_IS_LOG(FACQ_LOG_PRIV))
		return;
	switch(out){
	case FACQ_LOG_OUT_STDOUT:
		facq_log_toggle_stdout(FACQ_LOG_PRIV);
	break;
	case FACQ_LOG_OUT_STDERR:
		facq_log_toggle_stderr(FACQ_LOG_PRIV);
	break;
	case FACQ_LOG_OUT_FILE:
		facq_log_toggle_file(FACQ_LOG_PRIV,err);
	break;
#ifdef G_OS_UNIX
	case FACQ_LOG_OUT_SYSLOG:
		facq_log_toggle_syslog(FACQ_LOG_PRIV);
	break;
#endif
	default:
	break;
	}
	
	FACQ_LOG_UNLOCK
}

/**
 * facq_log_write:
 * @msg: A message.
 * @type: The type of the message. See #FacqLogMsgType for valid values.
 *
 * Writes the message, @msg, to all the previously enabled outputs.
 * Before writting the message to the outputs, @type will be compared against an
 * internal mask to check if it should be written, if the value of type is minor
 * than the value of the internal mask, the message will be silently dropped.
 *
 */
void facq_log_write(const gchar *msg,FacqLogMsgType type)
{
	gchar *out_buffer = NULL;
	gchar *date_time = NULL;
#ifdef G_OS_UNIX
	gint syslog_priority = LOG_NOTICE;
#endif

	if(!msg)
		return;

	FACQ_LOG_LOCK
	
	if(FACQ_IS_LOG(FACQ_LOG_PRIV)){
		if(type >= FACQ_LOG_PRIV->priv->mask){
			
			date_time = facq_log_get_date_time();
			out_buffer = g_strdup_printf("%s <%s>: %s\n",
				     date_time,
				     facq_log_msg_type_to_human(type),
				     msg);
			g_free(date_time);
			
			if(FACQ_LOG_PRIV->priv->out_stdout)
				g_print("%s",out_buffer);
			if(FACQ_LOG_PRIV->priv->out_stderr)
				g_printerr("%s",out_buffer);
			if(FACQ_LOG_PRIV->priv->out_file){
				g_io_channel_write_chars(
					FACQ_LOG_PRIV->priv->log_channel,
					out_buffer,
					-1,
					NULL,
					NULL);
				g_io_channel_flush(FACQ_LOG_PRIV->priv->log_channel,NULL);
			}
#ifdef G_OS_UNIX
			if(FACQ_LOG_PRIV->priv->out_syslog){
				switch(type){
				case FACQ_LOG_MSG_TYPE_DEBUG:
					syslog_priority = LOG_DEBUG;
				break;
				case FACQ_LOG_MSG_TYPE_INFO:
					syslog_priority = LOG_INFO;
				break;
				case FACQ_LOG_MSG_TYPE_WARNING:
					syslog_priority = LOG_WARNING;
				break;
				case FACQ_LOG_MSG_TYPE_ERROR:
					syslog_priority = LOG_ERR;
				break;
				default:
					syslog_priority = LOG_NOTICE;
				break;
				}
				syslog(syslog_priority,"%s",msg);
			}
#endif
		}
	}
	g_free(out_buffer);

	FACQ_LOG_UNLOCK
}

/**
 * facq_log_write_v:
 * @type: a #FacqLogMsgType valid value.
 * @format: a standard printf() format string. See g_strdup_vprintf() for
 * details.
 * @...: the parameters to insert into the format string
 *
 * It's like facq_log_write but it can accept parameters like on
 * <function>printf()</function> function.
 */ 
void facq_log_write_v(FacqLogMsgType type,const gchar *format,...) 
{
	va_list args;
	gchar *msg = NULL;

	va_start(args,format);
	msg = g_strdup_vprintf(format,args);
	facq_log_write(msg,type);
	g_free(msg);
	va_end(args);
}

/**
 * facq_log_get_filename:
 *
 * Returns the filename of the log file. This function will always return the
 * filename, even in the case that the #FACQ_LOG_OUT_FILE it's not enabled.
 *
 * Returns: The absolute filename of the log file (Absolute path).
 */
gchar *facq_log_get_filename(void)
{
	gchar *filename = NULL, *app_log = NULL, *app_folder = NULL;
	const gchar *home_dir = NULL, *app_name = NULL;

	app_name = g_get_application_name();
	if(!app_name){
		g_set_application_name("Unknown");
		app_name = g_get_application_name();
	}
	g_assert(app_name);

#ifdef G_OS_UNIX
	home_dir = g_get_home_dir();
	app_folder = g_strdup_printf(".%s",app_name);
#else
	home_dir = g_get_user_data_dir();
	app_folder = g_strdup_printf("%s",app_name);
#endif	
	app_log = g_strdup_printf("%s.log",app_name);

	filename = g_build_filename(home_dir,
				    app_folder,
				    "log",
				    app_log,
				    NULL);
	g_free(app_log);
	g_free(app_folder);
	return filename;
}

/**
 * facq_log_disable:
 *
 * Disables the logging mechanism. If you want to use it again you must call
 * facq_log_enable() and configure the mask and the outputs again.
 *
 * This function should get called at the end of your program.
 */
void facq_log_disable(void)
{
	FACQ_LOG_LOCK
	
	if(FACQ_IS_LOG(FACQ_LOG_PRIV))
		facq_log_free(FACQ_LOG_PRIV);
	FACQ_LOG_PRIV = NULL;
	
	FACQ_LOG_UNLOCK
}

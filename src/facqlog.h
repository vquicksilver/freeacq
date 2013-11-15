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
#ifndef _FREEACQ_LOGGER_H
#define _FREEACQ_LOGGER_H

#include <stdarg.h>

G_BEGIN_DECLS

#define FACQ_LOG_ERROR facq_log_error_quark()

#define FACQ_TYPE_LOG (facq_log_get_type ())
#define FACQ_LOG(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_LOG, FacqLog))
#define FACQ_LOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_LOG, FacqLogClass))
#define FACQ_IS_LOG(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_LOG))
#define FACQ_IS_LOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_LOG))
#define FACQ_LOG_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_LOG, FacqLogClass))

typedef struct _FacqLog FacqLog;
typedef struct _FacqLogClass FacqLogClass;
typedef struct _FacqLogPrivate FacqLogPrivate;

typedef enum {
	FACQ_LOG_ERROR_FAILED
} FacqLogError;

/**
 * FacqLogMsgType:
 * @FACQ_LOG_MSG_TYPE_DEBUG: A debug message.
 * @FACQ_LOG_MSG_TYPE_INFO: An info message.
 * @FACQ_LOG_MSG_TYPE_WARNING: A warning message.
 * @FACQ_LOG_MSG_TYPE_ERROR: An error message.
 *
 * Enum values for each type of message that the logging system can log.
 */
typedef enum {
	FACQ_LOG_MSG_TYPE_DEBUG,
	FACQ_LOG_MSG_TYPE_INFO,
	FACQ_LOG_MSG_TYPE_WARNING,
	FACQ_LOG_MSG_TYPE_ERROR,
	/*< private >*/
	FACQ_LOG_MSG_TYPE_N
} FacqLogMsgType;

/**
 * FacqLogOut:
 * @FACQ_LOG_OUT_STDOUT: Log to stdout.
 * @FACQ_LOG_OUT_STDERR: Log to stderr.
 * @FACQ_LOG_OUT_FILE: Log to a file.
 * @FACQ_LOG_OUT_SYSLOG: Log to syslog.
 *
 * Enum values for supported outputs of the logging system.
 *
 * <note>
 *  <para>
 *   Please note that FACQ_LOG_OUT_SYSLOG is only available on UNIX platforms.
 *  </para>
 * </note>
 */
typedef enum {
	FACQ_LOG_OUT_STDOUT,
	FACQ_LOG_OUT_STDERR,
	FACQ_LOG_OUT_FILE,
#ifdef G_OS_UNIX
	FACQ_LOG_OUT_SYSLOG,
#endif
	/*< private >*/
	FACQ_LOG_OUT_N
} FacqLogOut;

struct _FacqLog {
	/*< private >*/
	GObject parent_instance;
	FacqLogPrivate *priv;
};

struct _FacqLogClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_log_get_type(void) G_GNUC_CONST;

void facq_log_enable(void);
void facq_log_set_mask(FacqLogMsgType mask);
void facq_log_toggle_out(FacqLogOut out,GError **err);
void facq_log_write(const gchar *msg,FacqLogMsgType type);
void facq_log_write_v(FacqLogMsgType type,const gchar *format,...) G_GNUC_PRINTF(2,3);
gchar *facq_log_get_filename(void);
void facq_log_disable(void);

G_END_DECLS

#endif

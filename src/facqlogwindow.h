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
#ifndef _FREEACQ_LOG_WINDOW_H
#define _FREEACQ_LOG_WINDOW_H

G_BEGIN_DECLS

#define FACQ_TYPE_LOG_WINDOW (facq_log_window_get_type())
#define FACQ_LOG_WINDOW(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_LOG_WINDOW,FacqLogWindow))
#define FACQ_LOG_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_LOG_WINDOW, FacqLogWindowClass))
#define FACQ_IS_LOG_WINDOW(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_LOG_WINDOW))
#define FACQ_IS_LOG_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_LOG_WINDOW))
#define FACQ_LOG_WINDOW_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_LOG_WINDOW, FacqLogWindowClass))

typedef struct _FacqLogWindow FacqLogWindow;
typedef struct _FacqLogWindowClass FacqLogWindowClass;
typedef struct _FacqLogWindowPrivate FacqLogWindowPrivate;

struct _FacqLogWindow {
	/*< private >*/
        GObject parent_instance;
        FacqLogWindowPrivate *priv;
};

struct _FacqLogWindowClass {
	/*< private >*/
        GObjectClass parent_class;
};

GType facq_log_window_get_type(void) G_GNUC_CONST;

FacqLogWindow *facq_log_window_new(GtkWidget *top_window,const gchar *filename,guint lines,GError **err);
void facq_log_window_free(FacqLogWindow *log_window);

G_END_DECLS

#endif

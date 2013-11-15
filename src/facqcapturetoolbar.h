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
#ifndef _FREEACQ_CAPTURE_TOOLBAR_H
#define _FREEACQ_CAPTURE_TOOLBAR_H

G_BEGIN_DECLS

#define FACQ_TYPE_CAPTURE_TOOLBAR (facq_capture_toolbar_get_type ())
#define FACQ_CAPTURE_TOOLBAR(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_CAPTURE_TOOLBAR, FacqCaptureToolbar))
#define FACQ_CAPTURE_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_CAPTURE_TOOLBAR, FacqCaptureToolbarClass))
#define FACQ_IS_CAPTURE_TOOLBAR(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_CAPTURE_TOOLBAR))
#define FACQ_IS_CAPTURE_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_CAPTURE_TOOLBAR))
#define FACQ_CAPTURE_TOOLBAR_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_CAPTURE_TOOLBAR, FacqCaptureToolbarClass))

typedef	struct _FacqCaptureToolbar FacqCaptureToolbar;
typedef struct _FacqCaptureToolbarClass FacqCaptureToolbarClass;
typedef struct _FacqCaptureToolbarPrivate FacqCaptureToolbarPrivate;

struct _FacqCaptureToolbar {
	/*< private >*/
	GObject parent_instance;
	FacqCaptureToolbarPrivate *priv;
};

struct _FacqCaptureToolbarClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_capture_toolbar_get_type(void) G_GNUC_CONST;

FacqCaptureToolbar *facq_capture_toolbar_new(gpointer data);
GtkWidget *facq_capture_toolbar_get_widget(FacqCaptureToolbar *toolbar);
void facq_capture_toolbar_enable_add(FacqCaptureToolbar *toolbar);
void facq_capture_toolbar_enable_remove(FacqCaptureToolbar *toolbar);
void facq_capture_toolbar_enable_clear(FacqCaptureToolbar *toolbar);
void facq_capture_toolbar_enable_play(FacqCaptureToolbar *toolbar);
void facq_capture_toolbar_disable_stop(FacqCaptureToolbar *toolbar);
void facq_capture_toolbar_disable_add(FacqCaptureToolbar *toolbar);
void facq_capture_toolbar_disable_remove(FacqCaptureToolbar *toolbar);
void facq_capture_toolbar_disable_clear(FacqCaptureToolbar *toolbar);
void facq_capture_toolbar_disable_play(FacqCaptureToolbar *toolbar);
void facq_capture_toolbar_disable_stop(FacqCaptureToolbar *toolbar);
void facq_capture_toolbar_free(FacqCaptureToolbar *toolbar);

G_END_DECLS

#endif

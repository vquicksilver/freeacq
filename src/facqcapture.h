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
#include "facqcatalog.h"

#ifndef _FREEACQ_CAPTURE_H
#define _FREEACQ_CAPTURE_H

G_BEGIN_DECLS

#define FACQ_TYPE_CAPTURE (facq_capture_get_type ())
#define FACQ_CAPTURE(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_CAPTURE, FacqCapture))
#define FACQ_CAPTURE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_CAPTURE, FacqCaptureClass))
#define FACQ_IS_CAPTURE(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_CAPTURE))
#define FACQ_IS_CAPTURE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_CAPTURE))
#define FACQ_CAPTURE_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_CAPTURE, FacqCaptureClass))

typedef struct _FacqCapture FacqCapture;
typedef struct _FacqCaptureClass FacqCaptureClass;
typedef struct _FacqCapturePrivate FacqCapturePrivate;

struct _FacqCapture {
	/*< private >*/
	GObject parent_instance;
	FacqCapturePrivate *priv;
};

struct _FacqCaptureClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_capture_get_type(void) G_GNUC_CONST;

FacqCapture *facq_capture_new(const FacqCatalog *catalog);
GtkWidget *facq_capture_get_widget(const FacqCapture *cap);
void facq_capture_stream_preferences(FacqCapture *cap);
void facq_capture_stream_new(FacqCapture *cap);
void facq_capture_stream_open(FacqCapture *cap);
void facq_capture_stream_save_as(FacqCapture *cap);
void facq_capture_stream_close(FacqCapture *cap);
void facq_capture_control_add(FacqCapture *cap);
void facq_capture_control_remove(FacqCapture *cap);
void facq_capture_control_clear(FacqCapture *cap);
void facq_capture_control_play(FacqCapture *cap);
void facq_capture_control_stop(FacqCapture *cap);
void facq_capture_log(FacqCapture *cap);
void facq_capture_free(FacqCapture *cap);

G_END_DECLS

#endif

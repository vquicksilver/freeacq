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
#ifndef _FREEACQ_STREAM_DIALOG_H
#define _FREEACQ_STREAM_DIALOG_H

G_BEGIN_DECLS

#define FACQ_TYPE_STREAM_DIALOG (facq_stream_dialog_get_type ())
#define FACQ_STREAM_DIALOG(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_STREAM_DIALOG, FacqStreamDialog))
#define FACQ_STREAM_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_STREAM_DIALOG, FacqStreamDialogClass))
#define FACQ_IS_STREAM_DIALOG(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_STREAM_DIALOG))
#define FACQ_IS_STREAM_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_STREAM_DIALOG))
#define FACQ_STREAM_DIALOG_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_STREAM_DIALOG, FacqStreamDialogClass))

typedef struct _FacqStreamDialog FacqStreamDialog;
typedef struct _FacqStreamDialogClass FacqStreamDialogClass;
typedef struct _FacqStreamDialogPrivate FacqStreamDialogPrivate;

struct _FacqStreamDialog {
	/*< private >*/
	GObject parent_instance;
	FacqStreamDialogPrivate *priv;
};

struct _FacqStreamDialogClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_stream_dialog_get_type(void) G_GNUC_CONST;

FacqStreamDialog *facq_stream_dialog_new(GtkWidget *top_window,const gchar *name);
gint facq_stream_dialog_run(FacqStreamDialog *dialog);
gchar *facq_stream_dialog_get_input(const FacqStreamDialog *dialog);
void facq_stream_dialog_free(FacqStreamDialog *dialog);

G_END_DECLS

#endif

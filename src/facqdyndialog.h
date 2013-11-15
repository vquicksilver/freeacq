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
#ifndef _FREEACQ_DYN_DIALOG_H_
#define _FREEACQ_DYN_DIALOG_H_

G_BEGIN_DECLS

#define FACQ_DYN_DIALOG_ERROR facq_dyn_dialog_error_quark()

#define FACQ_TYPE_DYN_DIALOG (facq_dyn_dialog_get_type ())
#define FACQ_DYN_DIALOG(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_DYN_DIALOG, FacqDynDialog))
#define FACQ_DYN_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_DYN_DIALOG, FacqDynDialogClass))
#define FACQ_IS_DYN_DIALOG(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_DYN_DIALOG))
#define FACQ_IS_DYN_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_DYN_DIALOG))
#define FACQ_DYN_DIALOG_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_DYN_DIALOG, FacqDynDialogClass))

typedef struct _FacqDynDialog FacqDynDialog;
typedef struct _FacqDynDialogClass FacqDynDialogClass;
typedef struct _FacqDynDialogPrivate FacqDynDialogPrivate;

typedef enum {
	FACQ_DYN_DIALOG_ERROR_FAILED
} FacqDynDialogError;

struct _FacqDynDialog {
	/*< private >*/
	GObject parent_instance;
	FacqDynDialogPrivate *priv;
};

struct _FacqDynDialogClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_dyn_dialog_get_type(void) G_GNUC_CONST;

FacqDynDialog *facq_dyn_dialog_new(GtkWidget *top_window,const gchar *description,GError **err);
gint facq_dyn_dialog_run(FacqDynDialog *dialog);
const GPtrArray *facq_dyn_dialog_get_input(FacqDynDialog *dialog);
void facq_dyn_dialog_free(FacqDynDialog *dialog);

G_END_DECLS

#endif

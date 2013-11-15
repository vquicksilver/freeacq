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
#ifndef _FREEACQ_BAF_VIEW_DIALOG_H
#define _FREEACQ_BAF_VIEW_DIALOG_H

G_BEGIN_DECLS

#define FACQ_TYPE_BAF_VIEW_DIALOG (facq_baf_view_dialog_get_type ())
#define FACQ_BAF_VIEW_DIALOG(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_BAF_VIEW_DIALOG, FacqBAFViewDialog))
#define FACQ_BAF_VIEW_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_BAF_VIEW_DIALOG, FacqBAFViewDialogClass))
#define FACQ_IS_BAF_VIEW_DIALOG(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_BAF_VIEW_DIALOG))
#define FACQ_IS_BAF_VIEW_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_BAF_VIEW_DIALOG))
#define FACQ_BAF_VIEW_DIALOG_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_BAF_VIEW_DIALOG, FacqBAFViewDialogClass))

typedef struct _FacqBAFViewDialog FacqBAFViewDialog;
typedef struct _FacqBAFViewDialogClass FacqBAFViewDialogClass;
typedef struct _FacqBAFViewDialogPrivate FacqBAFViewDialogPrivate;

struct _FacqBAFViewDialog {
	/*< private >*/
	GObject parent_instance;
	FacqBAFViewDialogPrivate *priv;
};

struct _FacqBAFViewDialogClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_baf_view_dialog_get_type(void) G_GNUC_CONST;

FacqBAFViewDialog *facq_baf_view_dialog_new(GtkWidget *top_window);
gint facq_baf_view_dialog_run(FacqBAFViewDialog *dialog);
gdouble facq_baf_view_dialog_get_input(const FacqBAFViewDialog *dialog);
void facq_baf_view_dialog_free(FacqBAFViewDialog *dialog);

G_END_DECLS

#endif

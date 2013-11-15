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
#ifndef _FREEACQ_CATALOG_DIALOG_H
#define _FREEACQ_CATALOG_DIALOG_H

G_BEGIN_DECLS

#define FACQ_TYPE_CATALOG_DIALOG (facq_catalog_dialog_get_type ())
#define FACQ_CATALOG_DIALOG(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_CATALOG_DIALOG, FacqCatalogDialog))
#define FACQ_CATALOG_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_CATALOG_DIALOG, FacqCatalogDialogClass))
#define FACQ_IS_CATALOG_DIALOG(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_CATALOG_DIALOG))
#define FACQ_IS_CATALOG_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_CATALOG_DIALOG))
#define FACQ_CATALOG_DIALOG_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_CATALOG_DIALOG, FacqCatalogDialogClass))

typedef struct _FacqCatalogDialog FacqCatalogDialog;
typedef struct _FacqCatalogDialogClass FacqCatalogDialogClass;
typedef struct _FacqCatalogDialogPrivate FacqCatalogDialogPrivate;

struct _FacqCatalogDialog {
	/*< private >*/
	GObject parent_instance;
	FacqCatalogDialogPrivate *priv;
};

struct _FacqCatalogDialogClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_catalog_dialog_get_type(void) G_GNUC_CONST;

FacqCatalogDialog *facq_catalog_dialog_new(GtkWidget *top_window,const FacqCatalog *cat,FacqCatalogType type);
gint facq_catalog_dialog_run(FacqCatalogDialog *dialog);
guint facq_catalog_dialog_get_input(const FacqCatalogDialog *dialog,gboolean *selected,FacqCatalogType *type);
void facq_catalog_dialog_free(FacqCatalogDialog *dialog);

G_END_DECLS

#endif

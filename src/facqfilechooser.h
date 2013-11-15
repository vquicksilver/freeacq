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
#ifndef _FREEACQ_FILE_CHOOSER_H
#define _FREEACQ_FILE_CHOOSER_H

G_BEGIN_DECLS

typedef enum _FacqFileChooserDialogType {
	FACQ_FILE_CHOOSER_DIALOG_TYPE_SAVE,
	FACQ_FILE_CHOOSER_DIALOG_TYPE_LOAD,
	/* < private > */
	FACQ_FILE_CHOOSER_DIALOG_TYPE_N
} FacqFileChooserDialogType;

#define FACQ_TYPE_FILE_CHOOSER (facq_file_chooser_get_type())
#define FACQ_FILE_CHOOSER(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_FILE_CHOOSER,FacqFileChooser))
#define FACQ_FILE_CHOOSER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_FILE_CHOOSER, FacqFileChooserClass)
#define FACQ_IS_FILE_CHOOSER(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_FILE_CHOOSER))
#define FACQ_IS_FILE_CHOOSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_FILE_CHOOSER))
#define FACQ_FILE_CHOOSER_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_FILE_CHOOSER, FacqFileChooserClass))

typedef struct _FacqFileChooser FacqFileChooser;
typedef struct _FacqFileChooserClass FacqFileChooserClass;
typedef struct _FacqFileChooserPrivate FacqFileChooserPrivate;

struct _FacqFileChooser {
	/*< private >*/
	GObject parent_instance;
	FacqFileChooserPrivate *priv;
};

struct _FacqFileChooserClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_file_chooser_get_type(void) G_GNUC_CONST;

FacqFileChooser *facq_file_chooser_new(GtkWidget *topwindow,FacqFileChooserDialogType type,const gchar *ext,const gchar *description);
int facq_file_chooser_run_dialog(FacqFileChooser *chooser);
gchar *facq_file_chooser_get_filename_for_system(const FacqFileChooser *chooser);
gchar *facq_file_chooser_get_filename_for_display(const FacqFileChooser *chooser);
void facq_file_chooser_free(FacqFileChooser *chooser);

G_END_DECLS

#endif

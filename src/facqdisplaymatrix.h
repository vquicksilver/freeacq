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
#ifndef _FREEACQ_DISPLAY_MATRIX_H
#define _FREEACQ_DISPLAY_MATRIX_H

G_BEGIN_DECLS

#define FACQ_DISPLAY_MATRIX_ERROR facq_display_matrix_error_quark()

#define FACQ_TYPE_DISPLAY_MATRIX (facq_display_matrix_get_type ())
#define FACQ_DISPLAY_MATRIX(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_DISPLAY_MATRIX, FacqDisplayMatrix))
#define FACQ_DISPLAY_MATRIX_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_DISPLAY_MATRIX, FacqDisplayMatrixClass))
#define FACQ_IS_DISPLAY_MATRIX(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_DISPLAY_MATRIX))
#define FACQ_IS_DISPLAY_MATRIX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_DISPLAY_MATRIX))
#define FACQ_DISPLAY_MATRIX_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_DISPLAY_MATRIX, FacqDisplayMatrixClass))

typedef struct _FacqDisplayMatrix FacqDisplayMatrix;
typedef struct _FacqDisplayMatrixClass FacqDisplayMatrixClass;
typedef struct _FacqDisplayMatrixPrivate FacqDisplayMatrixPrivate;

typedef enum {
        FACQ_DISPLAY_MATRIX_ERROR_FAILED
} FacqDisplayMatrixError;

struct _FacqDisplayMatrix {
	/*< private >*/
	GObject parent_instance;
	FacqDisplayMatrixPrivate *priv;
};

struct _FacqDisplayMatrixClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_display_matrix_get_type(void) G_GNUC_CONST;

FacqDisplayMatrix *facq_display_matrix_new(guint rows,guint cols);
GtkWidget *facq_display_matrix_get_widget(const FacqDisplayMatrix *dis);
gboolean facq_display_matrix_setup(FacqDisplayMatrix *mat,guint *channels,guint n_channels,GError **err);
void facq_display_matrix_set_values(FacqDisplayMatrix *mat,const gdouble *val);
void facq_display_matrix_free(FacqDisplayMatrix *mat);

G_END_DECLS

#endif

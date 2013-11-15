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
#include <gtk/gtk.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqi18n.h"
#include "facqcolor.h"
#include "facqdisplay.h"
#include "facqdisplaymatrix.h"

/**
 * SECTION:facqdisplaymatrix
 * @short_description: A matrix of FacqDisplay elements.
 * @title:FacqDisplayMatrix
 * @include:facqdisplaymatrix.h
 *
 * Provides a matrix for displaying 1, 4, 16, and so on #FacqDisplay elements
 * at the same time, allowing to view various numeric values at the same time.
 * It's used by the plethysmograph application to show the beats per minute of
 * one or more patients.
 *
 * When you create the matrix for the first time with facq_display_matrix_new()
 * you can specify the number of rows and columns, that the matrix should have, 
 * instead of using the square multiples of each integer between 1 and 16.
 *
 * If you use the facq_display_matrix_setup() function the number of
 * #FacqDisplay widgets showed in the matrix will be adjusted to the nearest
 * square number to the number of channels, and each #FacqDisplay footer
 * will show the channel that is currently monitored, if used.
 *
 * To set the values in each #FacqDisplay use the
 * facq_display_matrix_set_values() function. Finally to destroy the
 * #FacqDisplayMatrix with the contained #FacqDisplay objects just call
 * facq_display_matrix_free().
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * Internally a #FacqDisplayMatrix uses a #GtkScrolledWindow, #GtkTable and a
 * #GPtrArray. The top level widget is the #GtkScrolledWindow.
 * Take a look also at #FacqDisplay.
 * </para>
 * </sect1>
 * 
 */

/**
 * FacqDisplayMatrix:
 *
 * Contains the private details of the #FacqDisplayMatrix.
 */

/**
 * FacqDisplayMatrixClass:
 *
 * Class for all the #FacqDisplayMatrix objects.
 */

/**
 * FacqDisplayMatrixError:
 * @FACQ_DISPLAY_MATRIX_ERROR_FAILED: some error happened in the
 * #FacqDisplayMatrix.
 *
 * Enum for all the possible error values in the #FacqDisplayMatrix.
 */

G_DEFINE_TYPE(FacqDisplayMatrix,facq_display_matrix,G_TYPE_OBJECT);

GQuark facq_display_matrix_error_quark(void)
{
        return g_quark_from_static_string("facq-display-matrix-error-quark");
}

enum {
	PROP_0,
	PROP_ROWS,
	PROP_COLS
};

struct _FacqDisplayMatrixPrivate {
	GPtrArray *dis;
	GtkWidget *table;
	GtkWidget *scrolled_window;
	guint rows;
	guint cols;
	guint n_channels;
};

/*****--- GObject magic ---*****/
static void facq_display_matrix_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqDisplayMatrix *mat = FACQ_DISPLAY_MATRIX(self);

	switch(property_id){
	case PROP_ROWS: mat->priv->rows = g_value_get_uint(value);
	break;
	case PROP_COLS: mat->priv->cols = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(mat,property_id,pspec);
	}
}

static void facq_display_matrix_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqDisplayMatrix *mat = FACQ_DISPLAY_MATRIX(self);

	switch(property_id){
	case PROP_ROWS: g_value_set_uint(value,mat->priv->rows);
	break;
	case PROP_COLS: g_value_set_uint(value,mat->priv->cols);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(mat,property_id,pspec);
	}
}

static void facq_display_matrix_constructed(GObject *self)
{
	FacqDisplayMatrix *mat = FACQ_DISPLAY_MATRIX(self);
	GtkWidget *table = NULL, *display = NULL, *scrolled_window = NULL;
	FacqDisplay *dis = NULL;
	guint i = 0, j = 0;

	table = gtk_table_new(mat->priv->rows,
			      mat->priv->cols,
			      TRUE);

	mat->priv->dis = 
		g_ptr_array_new_with_free_func((GDestroyNotify)facq_display_free);

	scrolled_window = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);

	for(j = 0;j < mat->priv->rows;j++){
		for(i = 0;i < mat->priv->cols;i++){
			dis = facq_display_new("Patient BPM",
					       "Unknown patient",
					       "Not connected",
					       j*mat->priv->cols+i);
			g_ptr_array_add(mat->priv->dis,dis);
			display = facq_display_get_widget(dis);
			gtk_table_attach_defaults(GTK_TABLE(table),display,i,i+1,j,j+1);
		}
	}

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window),
						table);

	gtk_widget_show_all(scrolled_window);

	mat->priv->table = table;
	mat->priv->scrolled_window = scrolled_window;
}

static void facq_display_matrix_finalize(GObject *self)
{
	FacqDisplayMatrix *mat = FACQ_DISPLAY_MATRIX(self);

	if(mat->priv->dis)
		g_ptr_array_free(mat->priv->dis,TRUE);

	if(GTK_IS_WIDGET(mat->priv->table))
		gtk_widget_destroy(mat->priv->table);

	G_OBJECT_CLASS(facq_display_matrix_parent_class)->finalize(self);
}

static void facq_display_matrix_class_init(FacqDisplayMatrixClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqDisplayMatrixPrivate));

	object_class->set_property = facq_display_matrix_set_property;
	object_class->get_property = facq_display_matrix_get_property;
        object_class->constructed = facq_display_matrix_constructed;
        object_class->finalize = facq_display_matrix_finalize;

	g_object_class_install_property(object_class,PROP_ROWS,
					g_param_spec_uint("rows",
							  "Rows",
							  "The number of rows",
							  1,
							  16,
							  1,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_COLS,
					g_param_spec_uint("cols",
							  "Columns",
							  "The number of columns",
							  1,
							  16,
							  1,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));
}

static void facq_display_matrix_init(FacqDisplayMatrix *mat)
{
	mat->priv = G_TYPE_INSTANCE_GET_PRIVATE(mat,FACQ_TYPE_DISPLAY_MATRIX,FacqDisplayMatrixPrivate);
	mat->priv->rows = 1;
	mat->priv->cols = 1;
	mat->priv->dis = NULL;
	mat->priv->n_channels = 1;
}

/*****--- private methods ---*****/
/* facq_display_matrix_get_y:
 * @n_channels: The number of channels to display, 256
 * is the maximum accepted value.
 *
 * Searches the nearest number that multiplied by itself 
 * approaches @n_channels, that is:
 * y / y*y >= n_channels.
 */
static guint facq_display_matrix_get_y(guint n_channels)
{
	guint y = 1, squarey = 0;

	for(y = 1;y <= 16;y++){
		squarey = y*y;
		if(n_channels <= squarey)
			return y;
	}

	return 1;
}


/****--- public methods ---*****/
/**
 * facq_display_matrix_new:
 * @rows: The initial number of rows (minor or equal than 16).
 * @cols: The initial number of columns (minor or equal than 16).
 *
 * Creates a new #FacqDisplayMatrix object, with rows x cols
 * #FacqDisplay objects inside.
 *
 * Returns: A new #FacqDisplayMatrix object, or %NULL in case of wrong
 * parameters.
 */
FacqDisplayMatrix *facq_display_matrix_new(guint rows,guint cols)
{
	g_return_val_if_fail(rows >= 1 && rows <= 16, NULL);
	g_return_val_if_fail(cols >= 1 && cols <= 16, NULL);

	return g_object_new(FACQ_TYPE_DISPLAY_MATRIX,
			    "rows",rows,
			    "cols",cols,
			    NULL);
}

/**
 * facq_display_matrix_get_widget:
 * @mat: A #FacqDisplayMatrix object.
 *
 * Gets the top level widget of the #FacqDisplayMatrix object, after this
 * you can add the widget to your application to show it.
 *
 * Returns: The top level widget.
 */
GtkWidget *facq_display_matrix_get_widget(const FacqDisplayMatrix *mat)
{
	g_return_val_if_fail(FACQ_IS_DISPLAY_MATRIX(mat),NULL);
	return mat->priv->scrolled_window;
}

/**
 * facq_display_matrix_setup:
 * @mat: A #FacqDisplayMatrix object.
 * @channels: An array of @n_channels length, with the number of each channel,
 * ordered by position.
 * @n_channels: The number of channels to display.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Setups the #FacqDisplayMatrix to show an optimum number of #FacqDisplay
 * objects, the nearest y / y*y >= n_channels.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_display_matrix_setup(FacqDisplayMatrix *mat,guint *channels,guint n_channels,GError **err)
{
	guint y = 1 , i = 0, j = 0;
	GtkWidget *table = NULL;
	FacqDisplay *dis = NULL;
	GtkWidget *display = NULL;
	GPtrArray *displays = NULL;
	gchar *entrytext = NULL;
	gchar *footer = NULL;

	g_return_val_if_fail(FACQ_IS_DISPLAY_MATRIX(mat),FALSE);
	g_return_val_if_fail(n_channels != 0,FALSE);
	g_return_val_if_fail(channels,FALSE);

	/* check n_channels is <= 256 */
	if(n_channels > 256){
		if(err)
			g_set_error(err,FACQ_DISPLAY_MATRIX_ERROR,
					FACQ_DISPLAY_MATRIX_ERROR_FAILED,"Channel number not supported");
		return FALSE;
	}

	/* get the number of rows and columns for the n_channels 
	 * we search smaller y where y in [1,..,16] / n_channels <= y*y */
	y = facq_display_matrix_get_y(n_channels);

	/* check if we need to resize the table, that it's y != [rows||cols]
	 * in that case we must resize the table, trying to keep the patient
	 * names */
	if(y != mat->priv->rows || y != mat->priv->cols){
		/* create a new y*y table and a new Ptr Array for storing the
		 * display objects */
		table = gtk_table_new(y,y,TRUE);
		displays = g_ptr_array_new_with_free_func((GDestroyNotify)facq_display_free);

		/* create y*y displays and put them in the new table trying
		 * to obtain the old patient name from the previous table */
		for(j = 0;j < y;j++){
			for(i = 0;i < y;i++){
				if(j*y+i < n_channels){
					footer = g_strdup_printf("Channel %u",channels[j*y+i]);
					if(j*y+i < mat->priv->dis->len){
						dis = (FacqDisplay *) g_ptr_array_index(mat->priv->dis,j*y+i);
						if(FACQ_IS_DISPLAY(dis))
							entrytext = facq_display_get_entry_text(dis);
					}
				}
				dis = facq_display_new("Patient's BPM",
						       (entrytext) ? entrytext : "Unknown Patient",
						       (footer) ? footer : "Not connected",
						       j*y+i);
				g_ptr_array_add(displays,dis);
				display = facq_display_get_widget(dis);
				gtk_table_attach_defaults(GTK_TABLE(table),display,i,i+1,j,j+1);
				if(entrytext)
					g_free(entrytext);
				if(footer)
					g_free(footer);
				entrytext = NULL;
				footer = NULL;
			}
		}

		/* destroy the old table */
		gtk_widget_destroy(mat->priv->table);
		g_ptr_array_free(mat->priv->dis,TRUE);

		/* put the new table in the scrolled window */
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(mat->priv->scrolled_window),
						      table);
		mat->priv->table = table;
		mat->priv->dis = displays;
		gtk_widget_show_all(table);

		/* set rows and cols */
		mat->priv->rows = y;
		mat->priv->cols = y;
	}

	/* set n_channels */
	mat->priv->n_channels = n_channels;

	return TRUE;
}

/**
 * facq_display_matrix_set_values:
 * @mat: A #FacqDisplayMatrix object.
 * @val: An array with n_channels values.
 *
 * Calls facq_display_set_value() for each #FacqDisplay in the
 * #FacqDisplayMatrix, @mat.
 * The length of val should be equal to cols*rows or equal to n_channels
 * depending on if you used facq_display_matrix_setup() or not.
 */
void facq_display_matrix_set_values(FacqDisplayMatrix *mat,const gdouble *val)
{
	guint i = 0;
	FacqDisplay *dis = NULL;

	g_return_if_fail(FACQ_IS_DISPLAY_MATRIX(mat));
	g_return_if_fail(val != NULL);

	for(i = 0;i < mat->priv->n_channels;i++){
		dis = (FacqDisplay *) g_ptr_array_index(mat->priv->dis,i);
		facq_display_set_value(dis,val[i]);
	}
}

/**
 * facq_display_matrix_free:
 * @mat: A #FacqDisplayMatrix object.
 *
 * Destroys the #FacqDisplayMatrix object along with all the
 * contained #FacqDisplay objects.
 */
void facq_display_matrix_free(FacqDisplayMatrix *mat)
{
	g_return_if_fail(FACQ_IS_DISPLAY_MATRIX(mat));
	g_object_unref(G_OBJECT(mat));
}

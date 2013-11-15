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
#ifndef _FREEACQ_OSCOPE_PLOT_H
#define _FREEACQ_OSCOPE_PLOT_H

G_BEGIN_DECLS

#define FACQ_OSCOPE_PLOT_ERROR facq_oscope_plot_error_quark()

#define FACQ_TYPE_OSCOPE_PLOT (facq_oscope_plot_get_type())
#define FACQ_OSCOPE_PLOT(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_OSCOPE_PLOT,FacqOscopePlot))
#define FACQ_OSCOPE_PLOT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_OSCOPE_PLOT, FacqOscopePlotClass))
#define FACQ_IS_OSCOPE_PLOT(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_OSCOPE_PLOT))
#define FACQ_IS_OSCOPE_PLOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_OSCOPE_PLOT))
#define FACQ_OSCOPE_PLOT_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_OSCOPE_PLOT, FacqOscopePlotClass))

typedef struct _FacqOscopePlot FacqOscopePlot;
typedef struct _FacqOscopePlotClass FacqOscopePlotClass;
typedef struct _FacqOscopePlotPrivate FacqOscopePlotPrivate;

typedef enum {
        FACQ_OSCOPE_PLOT_ERROR_FAILED
} FacqOscopePlotError;

struct _FacqOscopePlot {
	GObject parent_instance;
	FacqOscopePlotPrivate *priv;
};

struct _FacqOscopePlotClass {
	GObjectClass parent_class;
};

GType facq_oscope_plot_get_type(void) G_GNUC_CONST;

FacqOscopePlot *facq_oscope_plot_new(void);
gboolean facq_oscope_plot_setup(FacqOscopePlot *plot,gdouble period,guint n_channels,GError **err);
void facq_oscope_plot_process_chunk(FacqOscopePlot *plot,FacqChunk *chunk);
GtkWidget *facq_oscope_plot_get_widget(const FacqOscopePlot *plot);
void facq_oscope_plot_set_zoom(FacqOscopePlot *plot,gboolean enable);
void facq_oscope_plot_zoom_in(FacqOscopePlot *plot);
void facq_oscope_plot_zoom_out(FacqOscopePlot *plot);
void facq_oscope_plot_zoom_home(FacqOscopePlot *plot);
void facq_oscope_plot_free(FacqOscopePlot *plot);

G_END_DECLS

#endif

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
#ifndef _FREEACQ_BAF_VIEW_PLOT_H
#define _FREEACQ_BAF_VIEW_PLOT_H

G_BEGIN_DECLS

#define FACQ_BAF_VIEW_PLOT_ERROR facq_baf_view_plot_error_quark()

#define FACQ_TYPE_BAF_VIEW_PLOT (facq_baf_view_plot_get_type())
#define FACQ_BAF_VIEW_PLOT(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_BAF_VIEW_PLOT,FacqBAFViewPlot))
#define FACQ_BAF_VIEW_PLOT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_BAF_VIEW_PLOT, FacqBAFViewPlotClass))
#define FACQ_IS_BAF_VIEW_PLOT(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_BAF_VIEW_PLOT))
#define FACQ_IS_BAF_VIEW_PLOT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_BAF_VIEW_PLOT))
#define FACQ_BAF_VIEW_PLOT_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_BAF_VIEW_PLOT, FacqBAFViewPlotClass))

typedef struct _FacqBAFViewPlot FacqBAFViewPlot;
typedef struct _FacqBAFViewPlotClass FacqBAFViewPlotClass;
typedef struct _FacqBAFViewPlotPrivate FacqBAFViewPlotPrivate;

struct _FacqBAFViewPlot {
	/*< private >*/
	GObject parent_instance;
	FacqBAFViewPlotPrivate *priv;
};

struct _FacqBAFViewPlotClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_baf_view_plot_get_type(void) G_GNUC_CONST;

FacqBAFViewPlot *facq_baf_view_plot_new(void);
void facq_baf_view_plot_setup(FacqBAFViewPlot *plot,guint samples_per_page,gdouble period,guint n_channels);
void facq_baf_view_plot_push_chunk(FacqBAFViewPlot *plot,gdouble *chunk);
void facq_baf_view_plot_draw_page(FacqBAFViewPlot *plot,gdouble n_page);
GtkWidget *facq_baf_view_plot_get_widget(const FacqBAFViewPlot *plot);
void facq_baf_view_plot_clear(FacqBAFViewPlot *plot);
void facq_baf_view_plot_zoom_in(FacqBAFViewPlot *plot);
void facq_baf_view_plot_zoom_out(FacqBAFViewPlot *plot);
void facq_baf_view_plot_zoom_home(FacqBAFViewPlot *plot);
void facq_baf_view_plot_free(FacqBAFViewPlot *plot);

G_END_DECLS

#endif

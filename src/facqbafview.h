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
#ifndef _FREEACQ_BAF_VIEW_H
#define _FREEACQ_BAF_VIEW_H

G_BEGIN_DECLS

#define FACQ_TYPE_BAF_VIEW (facq_baf_view_get_type ())
#define FACQ_BAF_VIEW(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_BAF_VIEW, FacqBAFView))
#define FACQ_BAF_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_BAF_VIEW, FacqBAFViewClass))
#define FACQ_IS_BAF_VIEW(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_BAF_VIEW))
#define FACQ_IS_BAF_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_BAF_VIEW))
#define FACQ_BAF_VIEW_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_BAF_VIEW, FacqBAFViewClass))

typedef struct _FacqBAFView FacqBAFView;
typedef struct _FacqBAFViewClass FacqBAFViewClass;
typedef struct _FacqBAFViewPrivate FacqBAFViewPrivate;

struct _FacqBAFView {
	/*< private >*/
	GObject parent_instance;
	FacqBAFViewPrivate *priv;
};

struct _FacqBAFViewClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_baf_view_get_type(void) G_GNUC_CONST;

FacqBAFView *facq_baf_view_new(gdouble time_per_page);
GtkWidget *facq_baf_view_get_widget(const FacqBAFView *view);
void facq_baf_view_setup_page_time(FacqBAFView *view);
void facq_baf_view_open_file(FacqBAFView *view);
void facq_baf_view_export_file(FacqBAFView *view);
void facq_baf_view_close_file(FacqBAFView *view);
void facq_baf_view_plot_page(FacqBAFView *view,gdouble page);
void facq_baf_view_plot_page_spin(FacqBAFView *view);
void facq_baf_view_plot_first_page(FacqBAFView *view);
void facq_baf_view_plot_prev_page(FacqBAFView *view);
void facq_baf_view_plot_next_page(FacqBAFView *view);
void facq_baf_view_plot_last_page(FacqBAFView *view);
void facq_baf_view_zoom_in(FacqBAFView *view);
void facq_baf_view_zoom_out(FacqBAFView *view);
void facq_baf_view_zoom_fit(FacqBAFView *view);
void facq_baf_view_free(FacqBAFView *view);

G_END_DECLS

#endif

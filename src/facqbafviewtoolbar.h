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
#ifndef _FREEACQ_BAF_VIEW_TOOLBAR_H
#define _FREEACQ_BAF_VIEW_TOOLBAR_H

G_BEGIN_DECLS

#define FACQ_TYPE_BAF_VIEW_TOOLBAR (facq_baf_view_toolbar_get_type ())
#define FACQ_BAF_VIEW_TOOLBAR(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_BAF_VIEW_TOOLBAR, FacqBAFViewToolbar))
#define FACQ_BAF_VIEW_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_BAF_VIEW_TOOLBAR, FacqBAFViewToolbarClass))
#define FACQ_IS_BAF_VIEW_TOOLBAR(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_BAF_VIEW_TOOLBAR))
#define FACQ_IS_BAF_VIEW_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_BAF_VIEW_TOOLBAR))
#define FACQ_BAF_VIEW_TOOLBAR_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_BAF_VIEW_TOOLBAR, FacqBAFViewToolbarClass))

typedef struct _FacqBAFViewToolbar FacqBAFViewToolbar;
typedef struct _FacqBAFViewToolbarClass FacqBAFViewToolbarClass;
typedef struct _FacqBAFViewToolbarPrivate FacqBAFViewToolbarPrivate;

struct _FacqBAFViewToolbar {
	/*< private >*/
	GObject parent_instance;
	FacqBAFViewToolbarPrivate *priv;
};

struct _FacqBAFViewToolbarClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_baf_view_toolbar_get_type(void) G_GNUC_CONST;

FacqBAFViewToolbar *facq_baf_view_toolbar_new(gpointer data);
GtkWidget *facq_baf_view_toolbar_get_widget(FacqBAFViewToolbar *toolbar);
void facq_baf_view_toolbar_set_total_pages(FacqBAFViewToolbar *toolbar,gdouble pages);
gdouble facq_baf_view_toolbar_read_spin_button(FacqBAFViewToolbar *toolbar);
void facq_baf_view_toolbar_goto_page(FacqBAFViewToolbar *toolbar,gdouble page_n);
void facq_baf_view_toolbar_disable_navigation(FacqBAFViewToolbar *toolbar);
void facq_baf_view_toolbar_free(FacqBAFViewToolbar *toolbar);

G_END_DECLS

#endif

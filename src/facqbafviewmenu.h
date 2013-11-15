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
#ifndef _FREEACQ_BAF_VIEW_MENU_H
#define _FREEACQ_BAF_VIEW_MENU_H

G_BEGIN_DECLS

#define FACQ_TYPE_BAF_VIEW_MENU (facq_baf_view_menu_get_type ())
#define FACQ_BAF_VIEW_MENU(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_BAF_VIEW_MENU, FacqBAFViewMenu))
#define FACQ_BAF_VIEW_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_BAF_VIEW_MENU, FacqBAFViewMenuClass))
#define FACQ_IS_BAF_VIEW_MENU(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_BAF_VIEW_MENU))
#define FACQ_IS_BAF_VIEW_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_BAF_VIEW_MENU))
#define FACQ_BAF_VIEW_MENU_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_BAF_VIEW_MENU, FacqBAFViewMenuClass))

typedef struct _FacqBAFViewMenu FacqBAFViewMenu;
typedef struct _FacqBAFViewMenuClass FacqBAFViewMenuClass;
typedef struct _FacqBAFViewMenuPrivate FacqBAFViewMenuPrivate;

struct _FacqBAFViewMenu {
	/*< private >*/
	GObject parent_instance;
	FacqBAFViewMenuPrivate *priv;
};

struct _FacqBAFViewMenuClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_baf_view_menu_get_type(void) G_GNUC_CONST;

FacqBAFViewMenu *facq_baf_view_menu_new(gpointer data);
GtkWidget *facq_baf_view_menu_get_widget(const FacqBAFViewMenu *menu);
void facq_baf_view_menu_set_total_pages(FacqBAFViewMenu *menu,gdouble total_pages);
void facq_baf_view_menu_disable_navigation(FacqBAFViewMenu *menu);
void facq_baf_view_menu_goto_page(FacqBAFViewMenu *menu,gdouble page_n);
void facq_baf_view_menu_enable_close(FacqBAFViewMenu *menu);
void facq_baf_view_menu_disable_close(FacqBAFViewMenu *menu);
void facq_baf_view_menu_disable_save_as(FacqBAFViewMenu *menu);
void facq_baf_view_menu_enable_save_as(FacqBAFViewMenu *menu);
void facq_baf_view_menu_free(FacqBAFViewMenu *menu);

G_END_DECLS

#endif

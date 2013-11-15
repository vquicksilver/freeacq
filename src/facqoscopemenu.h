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
#ifndef _FREEACQ_OSCOPE_MENU_H
#define _FREEACQ_OSCOPE_MENU_H

G_BEGIN_DECLS

#define FACQ_TYPE_OSCOPE_MENU (facq_oscope_menu_get_type ())
#define FACQ_OSCOPE_MENU(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_OSCOPE_MENU, FacqOscopeMenu))
#define FACQ_OSCOPE_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_OSCOPE_MENU,FacqOscopeMenuClass))
#define FACQ_IS_OSCOPE_MENU(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_OSCOPE_MENU))
#define FACQ_IS_OSCOPE_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_OSCOPE_MENU))
#define FACQ_OSCOPE_MENU_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_OSCOPE_MENU, FacqOscopeMenuClass))

typedef struct _FacqOscopeMenu FacqOscopeMenu;
typedef struct _FacqOscopeMenuClass FacqOscopeMenuClass;
typedef struct _FacqOscopeMenuPrivate FacqOscopeMenuPrivate;

struct _FacqOscopeMenu {
	/*< private >*/
	GObject parent_instance;
	FacqOscopeMenuPrivate *priv;
};

struct _FacqOscopeMenuClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_oscope_menu_get_type(void) G_GNUC_CONST;

FacqOscopeMenu *facq_oscope_menu_new(gpointer data);
GtkWidget *facq_oscope_menu_get_widget(const FacqOscopeMenu *menu);
void facq_oscope_menu_disable_preferences(FacqOscopeMenu *menu);
void facq_oscope_menu_disable_disconnect(FacqOscopeMenu *menu);
void facq_oscope_menu_disable_zoom_in(FacqOscopeMenu *menu);
void facq_oscope_menu_disable_zoom_out(FacqOscopeMenu *menu);
void facq_oscope_menu_disable_zoom_home(FacqOscopeMenu *menu);
void facq_oscope_menu_enable_preferences(FacqOscopeMenu *menu);
void facq_oscope_menu_enable_disconnect(FacqOscopeMenu *menu);
void facq_oscope_menu_enable_zoom_in(FacqOscopeMenu *menu);
void facq_oscope_menu_enable_zoom_out(FacqOscopeMenu *menu);
void facq_oscope_menu_enable_zoom_home(FacqOscopeMenu *menu);
void facq_oscope_menu_free(FacqOscopeMenu *menu);

G_END_DECLS

#endif 

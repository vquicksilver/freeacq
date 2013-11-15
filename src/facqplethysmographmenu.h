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
#ifndef _FREEACQ_PLETHYSMOGRAPH_MENU_H
#define _FREEACQ_PLETHYSMOGRAPH_MENU_H

G_BEGIN_DECLS

#define FACQ_TYPE_PLETHYSMOGRAPH_MENU (facq_plethysmograph_menu_get_type ())
#define FACQ_PLETHYSMOGRAPH_MENU(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_PLETHYSMOGRAPH_MENU, FacqPlethysmographMenu))
#define FACQ_PLETHYSMOGRAPH_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_PLETHYSMOGRAPH_MENU,FacqPlethysmographMenuClass))
#define FACQ_IS_PLETHYSMOGRAPH_MENU(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_PLETHYSMOGRAPH_MENU))
#define FACQ_IS_PLETHYSMOGRAPH_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_PLETHYSMOGRAPH_MENU))
#define FACQ_PLETHYSMOGRAPH_MENU_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_PLETHYSMOGRAPH_MENU, FacqPlethysmographMenuClass))

typedef struct _FacqPlethysmographMenu FacqPlethysmographMenu;
typedef struct _FacqPlethysmographMenuClass FacqPlethysmographMenuClass;
typedef struct _FacqPlethysmographMenuPrivate FacqPlethysmographMenuPrivate;

struct _FacqPlethysmographMenu {
	/*< private >*/
	GObject parent_instance;
	FacqPlethysmographMenuPrivate *priv;
};

struct _FacqPlethysmographMenuClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_plethysmograph_menu_get_type(void) G_GNUC_CONST;

FacqPlethysmographMenu *facq_plethysmograph_menu_new(gpointer data);
GtkWidget *facq_plethysmograph_menu_get_widget(const FacqPlethysmographMenu *menu);
void facq_plethysmograph_menu_disable_plug_preferences(FacqPlethysmographMenu *menu);
void facq_plethysmograph_menu_disable_disconnect(FacqPlethysmographMenu *menu);
void facq_plethysmograph_menu_enable_plug_preferences(FacqPlethysmographMenu *menu);
void facq_plethysmograph_menu_enable_disconnect(FacqPlethysmographMenu *menu);
void facq_plethysmograph_menu_free(FacqPlethysmographMenu *menu);

G_END_DECLS

#endif 

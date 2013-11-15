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
#ifndef _FREEACQ_CAPTURE_MENU_H
#define _FREEACQ_CAPTURE_MENU_H

G_BEGIN_DECLS

#define FACQ_TYPE_CAPTURE_MENU (facq_capture_menu_get_type ())
#define FACQ_CAPTURE_MENU(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_CAPTURE_MENU, FacqCaptureMenu))
#define FACQ_CAPTURE_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_CAPTURE_MENU, FacqCaptureMenuClass))
#define FACQ_IS_CAPTURE_MENU(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_CAPTURE_MENU))
#define FACQ_IS_CAPTURE_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_CAPTURE_MENU))
#define FACQ_CAPTURE_MENU_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_CAPTURE_MENU, FacqCaptureMenuClass))

typedef struct _FacqCaptureMenu FacqCaptureMenu;
typedef struct _FacqCaptureMenuClass FacqCaptureMenuClass;
typedef struct _FacqCaptureMenuPrivate FacqCaptureMenuPrivate;

struct _FacqCaptureMenu {
	/*< private >*/
	GObject parent_instance;
	FacqCaptureMenuPrivate *priv;
};

struct _FacqCaptureMenuClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_capture_menu_get_type(void) G_GNUC_CONST;

FacqCaptureMenu *facq_capture_menu_new(gpointer data);
GtkWidget *facq_capture_menu_get_widget(const FacqCaptureMenu *menu);
void facq_capture_menu_enable_add(FacqCaptureMenu *menu);
void facq_capture_menu_enable_remove(FacqCaptureMenu *menu);
void facq_capture_menu_enable_clear(FacqCaptureMenu *menu);
void facq_capture_menu_enable_play(FacqCaptureMenu *menu);
void facq_capture_menu_enable_stop(FacqCaptureMenu *menu);
void facq_capture_menu_enable_preferences(FacqCaptureMenu *menu);
void facq_capture_menu_enable_new(FacqCaptureMenu *menu);
void facq_capture_menu_enable_open(FacqCaptureMenu *menu);
void facq_capture_menu_enable_save_as(FacqCaptureMenu *menu);
void facq_capture_menu_enable_close(FacqCaptureMenu *menu);
void facq_capture_menu_disable_add(FacqCaptureMenu *menu);
void facq_capture_menu_disable_remove(FacqCaptureMenu *menu);
void facq_capture_menu_disable_clear(FacqCaptureMenu *menu);
void facq_capture_menu_disable_play(FacqCaptureMenu *menu);
void facq_capture_menu_disable_stop(FacqCaptureMenu *menu);
void facq_capture_menu_disable_preferences(FacqCaptureMenu *menu);
void facq_capture_menu_disable_new(FacqCaptureMenu *menu);
void facq_capture_menu_disable_open(FacqCaptureMenu *menu);
void facq_capture_menu_disable_save_as(FacqCaptureMenu *menu);
void facq_capture_menu_disable_close(FacqCaptureMenu *menu);
void facq_capture_menu_free(FacqCaptureMenu *menu);

G_END_DECLS

#endif

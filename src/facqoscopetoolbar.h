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
#ifndef _FREEACQ_OSCOPE_TOOLBAR_H
#define _FREEACQ_OSCOPE_TOOLBAR_H

G_BEGIN_DECLS

#define FACQ_TYPE_OSCOPE_TOOLBAR (facq_oscope_toolbar_get_type ())
#define FACQ_OSCOPE_TOOLBAR(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_OSCOPE_TOOLBAR, FacqOscopeToolbar))
#define FACQ_OSCOPE_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_OSCOPE_TOOLBAR, FacqOscopeToolbarClass))
#define FACQ_IS_OSCOPE_TOOLBAR(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_OSCOPE_TOOLBAR))
#define FACQ_IS_OSCOPE_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_OSCOPE_TOOLBAR))
#define FACQ_OSCOPE_TOOLBAR_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_OSCOPE_TOOLBAR, FacqOscopeToolbarClass))

typedef struct _FacqOscopeToolbar FacqOscopeToolbar;
typedef struct _FacqOscopeToolbarClass FacqOscopeToolbarClass;
typedef struct _FacqOscopeToolbarPrivate FacqOscopeToolbarPrivate;

struct _FacqOscopeToolbar {
	/*< private >*/
	GObject parent_instance;
	FacqOscopeToolbarPrivate *priv;
};

struct _FacqOscopeToolbarClass {
	/*< private  >*/
	GObjectClass parent_class;
};

GType facq_oscope_toolbar_get_type(void) G_GNUC_CONST;

FacqOscopeToolbar *facq_oscope_toolbar_new(gpointer data);
GtkWidget *facq_oscope_toolbar_get_widget(FacqOscopeToolbar *toolbar);
void facq_oscope_toolbar_disable_disconnect(FacqOscopeToolbar *toolbar);
void facq_oscope_toolbar_disable_preferences(FacqOscopeToolbar *toolbar);
void facq_oscope_toolbar_disable_zoom_in(FacqOscopeToolbar *toolbar);
void facq_oscope_toolbar_disable_zoom_out(FacqOscopeToolbar *toolbar);
void facq_oscope_toolbar_disable_zoom_home(FacqOscopeToolbar *toolbar);
void facq_oscope_toolbar_enable_disconnect(FacqOscopeToolbar *toolbar);
void facq_oscope_toolbar_enable_preferences(FacqOscopeToolbar *toolbar);
void facq_oscope_toolbar_enable_zoom_in(FacqOscopeToolbar *toolbar);
void facq_oscope_toolbar_enable_zoom_out(FacqOscopeToolbar *toolbar);
void facq_oscope_toolbar_enable_zoom_home(FacqOscopeToolbar *toolbar);
void facq_oscope_toolbar_free(FacqOscopeToolbar *toolbar);

G_END_DECLS

#endif

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
#ifndef _FREEACQ_PLETHYSMOGRAPH_TOOLBAR_H
#define _FREEACQ_PLETHYSMOGRAPH_TOOLBAR_H

G_BEGIN_DECLS

#define FACQ_TYPE_PLETHYSMOGRAPH_TOOLBAR (facq_plethysmograph_toolbar_get_type ())
#define FACQ_PLETHYSMOGRAPH_TOOLBAR(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_PLETHYSMOGRAPH_TOOLBAR, FacqPlethysmographToolbar))
#define FACQ_PLETHYSMOGRAPH_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_PLETHYSMOGRAPH_TOOLBAR, FacqPlethysmographToolbarClass))
#define FACQ_IS_PLETHYSMOGRAPH_TOOLBAR(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_PLETHYSMOGRAPH_TOOLBAR))
#define FACQ_IS_PLETHYSMOGRAPH_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_PLETHYSMOGRAPH_TOOLBAR))
#define FACQ_PLETHYSMOGRAPH_TOOLBAR_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_PLETHYSMOGRAPH_TOOLBAR, FacqPlethysmographToolbarClass))

typedef struct _FacqPlethysmographToolbar FacqPlethysmographToolbar;
typedef struct _FacqPlethysmographToolbarClass FacqPlethysmographToolbarClass;
typedef struct _FacqPlethysmographToolbarPrivate FacqPlethysmographToolbarPrivate;

struct _FacqPlethysmographToolbar {
	/*< private >*/
	GObject parent_instance;
	FacqPlethysmographToolbarPrivate *priv;
};

struct _FacqPlethysmographToolbarClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_plethysmograph_toolbar_get_type(void) G_GNUC_CONST;

FacqPlethysmographToolbar *facq_plethysmograph_toolbar_new(gpointer data);
GtkWidget *facq_plethysmograph_toolbar_get_widget(FacqPlethysmographToolbar *toolbar);
void facq_plethysmograph_toolbar_disable_disconnect(FacqPlethysmographToolbar *toolbar);
void facq_plethysmograph_toolbar_disable_plug_preferences(FacqPlethysmographToolbar *toolbar);
void facq_plethysmograph_toolbar_enable_disconnect(FacqPlethysmographToolbar *toolbar);
void facq_plethysmograph_toolbar_enable_plug_preferences(FacqPlethysmographToolbar *toolbar);
void facq_plethysmograph_toolbar_free(FacqPlethysmographToolbar *toolbar);

G_END_DECLS

#endif

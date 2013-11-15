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
#ifndef _FREEACQ_PLETHYSMOGRAPH_H
#define _FREEACQ_PLETHYSMOGRAPH_H

G_BEGIN_DECLS

#define FACQ_TYPE_PLETHYSMOGRAPH (facq_plethysmograph_get_type ())
#define FACQ_PLETHYSMOGRAPH(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_PLETHYSMOGRAPH, FacqPlethysmograph))
#define FACQ_PLETHYSMOGRAPH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_PLETHYSMOGRAPH, FacqPlethysmographClass))
#define FACQ_IS_PLETHYSMOGRAPH(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_PLETHYSMOGRAPH))
#define FACQ_IS_PLETHYSMOGRAPH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_PLETHYSMOGRAPH))
#define FACQ_PLETHYSMOGRAPH_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_PLETHYSMOGRAPH, FacqPlethysmographClass))

typedef struct _FacqPlethysmograph FacqPlethysmograph;
typedef struct _FacqPlethysmographClass FacqPlethysmographClass;
typedef struct _FacqPlethysmographPrivate FacqPlethysmographPrivate;

struct _FacqPlethysmograph {
	/*< private >*/
	GObject parent_instance;
	FacqPlethysmographPrivate *priv;
};

struct _FacqPlethysmographClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_plethysmograph_get_type(void) G_GNUC_CONST;

FacqPlethysmograph *facq_plethysmograph_new(const gchar *address,guint16 port,GError **err);
GtkWidget *facq_plethysmograph_get_widget(const FacqPlethysmograph *plethysmograph);
void facq_plethysmograph_disconnect(FacqPlethysmograph *plethysmograph);
void facq_plethysmograph_set_listen_address(FacqPlethysmograph *plethysmograph);
void facq_plethysmograph_free(FacqPlethysmograph *plethysmograph);

G_END_DECLS

#endif

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
#ifndef _FREEACQ_OSCOPE_H
#define _FREEACQ_OSCOPE_H

G_BEGIN_DECLS

#define FACQ_TYPE_OSCOPE (facq_oscope_get_type ())
#define FACQ_OSCOPE(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_OSCOPE, FacqOscope))
#define FACQ_OSCOPE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_OSCOPE, FacqOscopeClass))
#define FACQ_IS_OSCOPE(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_OSCOPE))
#define FACQ_IS_OSCOPE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_OSCOPE))
#define FACQ_OSCOPE_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_OSCOPE, FacqOscopeClass))

typedef struct _FacqOscope FacqOscope;
typedef struct _FacqOscopeClass FacqOscopeClass;
typedef struct _FacqOscopePrivate FacqOscopePrivate;

struct _FacqOscope {
	GObject parent_instance;
	FacqOscopePrivate *priv;
};

struct _FacqOscopeClass {
	GObjectClass parent_class;
};

GType facq_oscope_get_type(void) G_GNUC_CONST;

FacqOscope *facq_oscope_new(const gchar *address,guint16 port,GError **err);
GtkWidget *facq_oscope_get_widget(const FacqOscope *oscope);
void facq_oscope_disconnect(FacqOscope *oscope);
void facq_oscope_set_listen_address(FacqOscope *oscope);
void facq_oscope_zoom_in(FacqOscope *oscope);
void facq_oscope_zoom_out(FacqOscope *oscope);
void facq_oscope_zoom_100(FacqOscope *oscope);
void facq_oscope_free(FacqOscope *oscope);

G_END_DECLS

#endif

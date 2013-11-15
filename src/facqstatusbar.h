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
#ifndef _FREEACQ_STATUSBAR_H
#define _FREEACQ_STATUSBAR_H

G_BEGIN_DECLS

#define FACQ_TYPE_STATUSBAR (facq_statusbar_get_type ())
#define FACQ_STATUSBAR(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_STATUSBAR, FacqStatusbar))
#define FACQ_STATUSBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_STATUSBAR, FacqStatusbarClass))
#define FACQ_IS_STATUSBAR(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_STATUSBAR))
#define FACQ_IS_STATUSBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_STATUSBAR))
#define FACQ_STATUSBAR_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_STATUSBAR, FacqStatusbarClass))

typedef struct _FacqStatusbar FacqStatusbar;
typedef struct _FacqStatusbarClass FacqStatusbarClass;
typedef struct _FacqStatusbarPrivate FacqStatusbarPrivate;

struct _FacqStatusbar {
	/*< private >*/
	GObject parent_instance;
	FacqStatusbarPrivate *priv;
};

struct _FacqStatusbarClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_statusbar_get_type(void) G_GNUC_CONST;

FacqStatusbar *facq_statusbar_new(void);
GtkWidget *facq_statusbar_get_widget(const FacqStatusbar *statusbar);
void facq_statusbar_write_msg(FacqStatusbar *statusbar,const gchar *format,...) G_GNUC_PRINTF(2,3);
void facq_statusbar_free(FacqStatusbar *statusbar);

G_END_DECLS

#endif

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
#ifndef _FREEACQ_DISPLAY_H
#define _FREEACQ_DISPLAY_H

G_BEGIN_DECLS

#define FACQ_TYPE_DISPLAY (facq_display_get_type ())
#define FACQ_DISPLAY(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_DISPLAY, FacqDisplay))
#define FACQ_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_DISPLAY, FacqDisplayClass))
#define FACQ_IS_DISPLAY(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_DISPLAY))
#define FACQ_IS_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_DISPLAY))
#define FACQ_DISPLAY_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_DISPLAY, FacqDisplayClass))

typedef struct _FacqDisplay FacqDisplay;
typedef struct _FacqDisplayClass FacqDisplayClass;
typedef struct _FacqDisplayPrivate FacqDisplayPrivate;

struct _FacqDisplay {
	/*< private >*/
	GObject parent_instance;
	FacqDisplayPrivate *priv;
};

struct _FacqDisplayClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_display_get_type(void) G_GNUC_CONST;

FacqDisplay *facq_display_new(const gchar *title,const gchar *entrytext,const gchar *footer,guint channel);
GtkWidget *facq_display_get_widget(const FacqDisplay *dis);
gdouble facq_display_get_value(const FacqDisplay *dis);
void facq_display_set_value(FacqDisplay *dis,gdouble value);
void facq_display_set_title(FacqDisplay *dis,const gchar *title);
gchar *facq_display_get_entry_text(FacqDisplay *dis);
void facq_display_set_entry_text(FacqDisplay *dis,const gchar *entrytext);
void facq_display_set_footer(FacqDisplay *dis,const gchar *footer);
void facq_display_free(FacqDisplay *dis);

G_END_DECLS

#endif

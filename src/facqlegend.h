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
#ifndef _FREEACQ_LEGEND_H
#define _FREEACQ_LEGEND_H

G_BEGIN_DECLS

#define FACQ_TYPE_LEGEND (facq_legend_get_type ())
#define FACQ_LEGEND(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_LEGEND, FacqLegend))
#define FACQ_LEGEND_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_LEGEND, FacqLegendClass))
#define FACQ_IS_LEGEND(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_LEGEND))
#define FACQ_IS_LEGEND_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_LEGEND))
#define FACQ_LEGEND_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_LEGEND, FacqLegendClass))

typedef struct _FacqLegend FacqLegend;
typedef struct _FacqLegendClass FacqLegendClass;
typedef struct _FacqLegendPrivate FacqLegendPrivate;

struct _FacqLegend {
	/*< private >*/
	GObject parent_instance;
	FacqLegendPrivate *priv;
};

struct _FacqLegendClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_legend_get_type(void) G_GNUC_CONST;

FacqLegend *facq_legend_new(void);
GtkWidget *facq_legend_get_widget(const FacqLegend *leg);
void facq_legend_set_data(FacqLegend *leg,const FacqStreamData *stmd);
void facq_legend_clear_data(FacqLegend *leg);
void facq_legend_free(FacqLegend *leg);

G_END_DECLS

#endif

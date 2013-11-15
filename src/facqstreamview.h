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
#ifndef _FREEACQ_STREAM_VIEW_H
#define _FREEACQ_STREAM_VIEW_H

G_BEGIN_DECLS

#define FACQ_TYPE_STREAM_VIEW (facq_stream_view_get_type ())
#define FACQ_STREAM_VIEW(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_STREAM_VIEW, FacqStreamView))
#define FACQ_STREAM_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_STREAM_VIEW, FacqStreamViewClass))
#define FACQ_IS_STREAM_VIEW(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_STREAM_VIEW))
#define FACQ_IS_STREAM_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_STREAM_VIEW))
#define FACQ_STREAM_VIEW_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_STREAM_VIEW, FacqStreamViewClass))

typedef enum facq_stream_view_status {
	FACQ_STREAM_VIEW_STATUS_NO_STREAM,
	FACQ_STREAM_VIEW_STATUS_NEW_STREAM,
	FACQ_STREAM_VIEW_STATUS_WITH_SOURCE,
	FACQ_STREAM_VIEW_STATUS_WITH_SINK,
	FACQ_STREAM_VIEW_STATUS_PLAY,
	FACQ_STREAM_VIEW_STATUS_STOP,
	FACQ_STREAM_VIEW_STATUS_ERROR
} FacqStreamViewStatus;

#define FACQ_ITEM_TYPE_SOURCE "Source"
#define FACQ_ITEM_TYPE_OPERATION "Operation"
#define FACQ_ITEM_TYPE_SINK "Sink"

typedef enum item_types {
	FACQ_STREAM_VIEW_ITEM_TYPE_SOURCE,
	FACQ_STREAM_VIEW_ITEM_TYPE_OPERATION,
	FACQ_STREAM_VIEW_ITEM_TYPE_SINK,
	/*< private >*/
	FACQ_STREAM_VIEW_ITEM_TYPE_N
} FacqStreamViewItemType;

typedef struct _FacqStreamView FacqStreamView;
typedef struct _FacqStreamViewClass FacqStreamViewClass;
typedef struct _FacqStreamViewPrivate FacqStreamViewPrivate;

struct _FacqStreamView {
	/*< private >*/
	GObject parent_instance;
	FacqStreamViewPrivate *priv;
};

struct _FacqStreamViewClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_stream_view_get_type(void) G_GNUC_CONST;

FacqStreamView *facq_stream_view_new(void);
GtkWidget *facq_stream_view_get_widget(const FacqStreamView *view);
void facq_stream_view_set_status(FacqStreamView *view,FacqStreamViewStatus status);
void facq_stream_view_push_item(FacqStreamView *view,FacqStreamViewItemType type,const gchar *name,const gchar *desc);
void facq_stream_view_pop_item(FacqStreamView *view);
void facq_stream_view_clear_data(FacqStreamView *view);
void facq_stream_view_free(FacqStreamView *view);

G_END_DECLS

#endif

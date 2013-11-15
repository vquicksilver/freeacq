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
#ifndef _FREEACQ_CHANLIST_EDITOR_H
#define _FREEACQ_CHANLIST_EDITOR_H

G_BEGIN_DECLS

#define FACQ_TYPE_CHANLIST_EDITOR (facq_chanlist_editor_get_type ())
#define FACQ_CHANLIST_EDITOR(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_CHANLIST_EDITOR, FacqChanlistEditor))
#define FACQ_CHANLIST_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_CHANLIST_EDITOR, FacqChanlistEditorClass))
#define FACQ_IS_CHANLIST_EDITOR(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_CHANLIST_EDITOR))
#define FACQ_IS_CHANLIST_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_CHANLIST_EDITOR))
#define FACQ_CHANLIST_EDITOR_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_CHANLIST_EDITOR, FacqChanlistEditorClass))

typedef struct _FacqChanlistEditor FacqChanlistEditor;
typedef struct _FacqChanlistEditorClass FacqChanlistEditorClass;
typedef struct _FacqChanlistEditorPrivate FacqChanlistEditorPrivate;

struct _FacqChanlistEditor {
	/*< private >*/
	GObject parent_instance;
	FacqChanlistEditorPrivate *priv;
};

struct _FacqChanlistEditorClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_chanlist_editor_get_type(void) G_GNUC_CONST;

FacqChanlistEditor *facq_chanlist_editor_new(gboolean input,gboolean advanced,guint max_channels,gboolean extra_aref);
GtkWidget *facq_chanlist_editor_get_widget(const FacqChanlistEditor *ed);
FacqChanlist *facq_chanlist_editor_get_chanlist(const FacqChanlistEditor *ed);
void facq_chanlist_editor_free(FacqChanlistEditor *ed);

G_END_DECLS

#endif

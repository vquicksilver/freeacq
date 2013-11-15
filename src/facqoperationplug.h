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
#ifndef _FREEACQ_OPERATION_PLUG_H
#define _FREEACQ_OPERATION_PLUG_H

G_BEGIN_DECLS

#define FACQ_OPERATION_PLUG_ERROR facq_operation_plug_error_quark()

#define FACQ_TYPE_OPERATION_PLUG (facq_operation_plug_get_type ())
#define FACQ_OPERATION_PLUG(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_OPERATION_PLUG, FacqOperationPlug))
#define FACQ_OPERATION_PLUG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_OPERATION_PLUG, FacqOperationPlugClass))
#define FACQ_IS_OPERATION_PLUG(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_OPERATION_PLUG))
#define FACQ_IS_OPERATION_PLUG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_OPERATION_PLUG))
#define FACQ_OPERATION_PLUG_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_OPERATION_PLUG,FacqOperationPlugClass))

typedef struct _FacqOperationPlug FacqOperationPlug;
typedef struct _FacqOperationPlugClass FacqOperationPlugClass;
typedef struct _FacqOperationPlugPrivate FacqOperationPlugPrivate;

typedef enum {
	FACQ_OPERATION_PLUG_ERROR_FAILED
} FacqOperationPlugError;

struct _FacqOperationPlug {
	/*< private >*/
        FacqOperation parent_instance;
        FacqOperationPlugPrivate *priv;
};

struct _FacqOperationPlugClass {
	/*< private >*/
        FacqOperationClass parent_class;
};

GType facq_operation_plug_get_type(void) G_GNUC_CONST;

gpointer facq_operation_plug_constructor(const GPtrArray *user_input,GError **err);
FacqOperationPlug *facq_operation_plug_new(const gchar *address,guint16 port);

/* virtual implementations */
void facq_operation_plug_to_file(FacqOperation *op,GKeyFile *file,const gchar *group);
gpointer facq_operation_plug_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err);
gboolean facq_operation_plug_start(FacqOperation *op,const FacqStreamData *stmd,GError **err);
gboolean facq_operation_plug_do(FacqOperation *op,FacqChunk *chunk,const FacqStreamData *stmd,GError **err);
gboolean facq_operation_plug_stop(FacqOperation *op,const FacqStreamData *stmd,GError **err);
void facq_operation_plug_free(FacqOperation *op);

G_END_DECLS

#endif

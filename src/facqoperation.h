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
#ifndef _FREEACQ_OPERATION_H_
#define _FREEACQ_OPERATION_H_

G_BEGIN_DECLS

#define FACQ_TYPE_OPERATION (facq_operation_get_type ())
#define FACQ_OPERATION(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_OPERATION, FacqOperation))
#define FACQ_OPERATION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_OPERATION, FacqOperationClass))
#define FACQ_IS_OPERATION(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_OPERATION))
#define FACQ_IS_OPERATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_OPERATION))
#define FACQ_OPERATION_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_OPERATION, FacqOperationClass))

typedef struct _FacqOperation FacqOperation;
typedef struct _FacqOperationClass FacqOperationClass;
typedef struct _FacqOperationPrivate FacqOperationPrivate;

struct _FacqOperation {
	/*< private >*/
	GObject parent_instance;
	FacqOperationPrivate *priv;
};

struct _FacqOperationClass {
	/*< private >*/
	GObjectClass parent_class;
	/*< public >*/
	void (*opsave)(FacqOperation *op,GKeyFile *file,const gchar *group);
	gboolean (*opstart)(FacqOperation *op,const FacqStreamData *stmd,GError **err);
	gboolean (*opdo)(FacqOperation *op,FacqChunk *chunk,const FacqStreamData *stmd,GError **err);
	gboolean (*opstop)(FacqOperation *op,const FacqStreamData *stmd,GError **err);
	void (*opfree)(FacqOperation *op);
};

GType facq_operation_get_type(void) G_GNUC_CONST;

const gchar *facq_operation_get_name(FacqOperation *op);
const gchar *facq_operation_get_description(FacqOperation *op);
gboolean facq_operation_get_started(FacqOperation *op);
void facq_operation_to_file(FacqOperation *op,GKeyFile *file,const gchar *group);
gboolean facq_operation_start(FacqOperation *op,const FacqStreamData *stmd,GError **err);
gboolean facq_operation_do(FacqOperation *op,FacqChunk *chunk,const FacqStreamData *stmd,GError **err);
gboolean facq_operation_stop(FacqOperation *op,const FacqStreamData *stmd,GError **err);
void facq_operation_free(FacqOperation *op);

G_END_DECLS

#endif

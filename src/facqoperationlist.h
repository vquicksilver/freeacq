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
#ifndef _FACQ_OPERATION_LIST_H_
#define _FACQ_OPERATION_LIST_H_

G_BEGIN_DECLS

#define FACQ_OPERATION_LIST_ERROR facq_operation_list_error_quark()

#define FACQ_TYPE_OPERATION_LIST (facq_operation_list_get_type ())
#define FACQ_OPERATION_LIST(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_OPERATION_LIST, FacqOperationList))
#define FACQ_OPERATION_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_OPERATION_LIST, FacqOperationListClass))
#define FACQ_IS_OPERATION_LIST(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_OPERATION_LIST))
#define FACQ_IS_OPERATION_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_OPERATION_LIST))
#define FACQ_OPERATION_LIST_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_OPERATION_LIST, FacqOperationListClass))

typedef struct _FacqOperationList FacqOperationList;
typedef struct _FacqOperationListClass FacqOperationListClass;
typedef struct _FacqOperationListPrivate FacqOperationListPrivate;

typedef enum {
	FACQ_OPERATION_LIST_ERROR_FAILED
} FacqOperationListError;

struct _FacqOperationList {
	/*< private >*/
	GObject parent_instance;
	FacqOperationListPrivate *priv;
};

struct _FacqOperationListClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_operation_list_get_type(void) G_GNUC_CONST;

FacqOperationList *facq_operation_list_new(void);
guint facq_operation_list_get_length(const FacqOperationList *oplist);
guint facq_operation_list_add(FacqOperationList *oplist,const FacqOperation *op);
FacqOperation *facq_operation_list_get(const FacqOperationList *oplist,guint i);
guint facq_operation_list_del_and_destroy(FacqOperationList *oplist);
gboolean facq_operation_list_start(FacqOperationList *oplist,const FacqStreamData *stmd,GError **err);
gboolean facq_operation_list_do(FacqOperationList *oplist,FacqChunk *chunk,const FacqStreamData *stmd,GError **err);
gboolean facq_operation_list_stop(FacqOperationList *oplist,const FacqStreamData *stmd,GError **err);
void facq_operation_list_free(FacqOperationList *oplist);

G_END_DECLS

#endif

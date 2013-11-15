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
#include <glib.h>
#include <gio/gio.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqunits.h"
#include "facqlog.h"
#include "facqchanlist.h"
#include "facqchunk.h"
#include "facqstreamdata.h"
#include "facqoperation.h"
#include "facqoperationlist.h"

/**
 * SECTION:facqoperationlist
 * @short_description: Handles one or more operations.
 * @title:FacqOperationList
 * @include:facqoperationlist.h
 * @see_also: #FacqOperation
 *
 * #FacqOperationList provides the operations needed to handle one or more
 * #FacqOperation objects. 
 *
 * To create a new #FacqOperationList you can use facq_operation_list_new(),
 * to get the number of operations in the list use
 * facq_operation_list_get_length(), to add an existing #FacqOperation use
 * facq_operation_list_add(), to remove and destroy the latest added operation use
 * facq_operation_list_del_and_destroy(), to get an operation from the list knowing it's
 * index use facq_operation_list_get(), to start all the operations in the list
 * use facq_operation_list_start(), to execute the main functionality of each
 * operation in order use facq_operation_list_do(), to stop all the operations
 * use facq_operation_list_stop(), finally to destroy the #FacqOperationList 
 * and all the operations added to it use facq_operation_list_free().
 */

/**
 * FacqOperationList:
 *
 * Contains the private details of the #FacqOperationList.
 */

/**
 * FacqOperationListClass:
 *
 * Class for all the #FacqOperationList objects.
 */

/**
 * FacqOperationListError:
 * @FACQ_OPERATION_LIST_ERROR_FAILED: Some error happened in the list.
 *
 * Enum for all the possible error values in a #FacqOperationList.
 */

G_DEFINE_TYPE(FacqOperationList,facq_operation_list,G_TYPE_OBJECT);

#define DEFAULT_SIZE 10

enum {
	PROP_0,
};

struct _FacqOperationListPrivate {
	GPtrArray *list;
};

GQuark facq_operation_list_error_quark(void)
{
	return g_quark_from_static_string("facq-operation-list-error-quark");
}

static void facq_operation_list_free_op(gpointer data);

/*****--- GObject magic ---*****/
static void facq_operation_list_constructed(GObject *self)
{
	FacqOperationList *oplist = FACQ_OPERATION_LIST(self);

	oplist->priv->list = g_ptr_array_sized_new(DEFAULT_SIZE);
	g_ptr_array_set_free_func(oplist->priv->list,facq_operation_list_free_op);
	g_assert(oplist->priv->list != NULL);
}

static void facq_operation_list_finalize(GObject *self)
{
	FacqOperationList *oplist = FACQ_OPERATION_LIST(self);

	if(oplist->priv->list){
		g_ptr_array_free(oplist->priv->list,TRUE);
	}

	G_OBJECT_CLASS(facq_operation_list_parent_class)->finalize(self);
}

static void facq_operation_list_class_init(FacqOperationListClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FacqOperationListPrivate));

	object_class->finalize = facq_operation_list_finalize;
	object_class->constructed = facq_operation_list_constructed;
}

static void facq_operation_list_init(FacqOperationList *oplist)
{
	oplist->priv = G_TYPE_INSTANCE_GET_PRIVATE(oplist,FACQ_TYPE_OPERATION_LIST,FacqOperationListPrivate);
	oplist->priv->list = NULL;
}
/*****--- Private methods ---*****/
static void facq_operation_list_free_op(gpointer data)
{
	FacqOperation *op = NULL;

	g_return_if_fail(FACQ_IS_OPERATION(data));
	op = FACQ_OPERATION(data);
	if(FACQ_IS_OPERATION(op))
		facq_operation_free(op);
}

/*****--- Public methods ---*****/
/**
 * facq_operation_list_new:
 *
 * Creates a new (and empty) #FacqOperationList.
 *
 * Returns: A new #FacqOperationList.
 */
FacqOperationList *facq_operation_list_new(void)
{
	return g_object_new(FACQ_TYPE_OPERATION_LIST,NULL);
}

/**
 * facq_operation_list_get_length:
 * @oplist: A #FacqOperationList object.
 *
 * Gets the number of added operations to the #FacqOperationList.
 *
 * Returns: 0 or greater.
 */
guint facq_operation_list_get_length(const FacqOperationList *oplist)
{
	g_return_val_if_fail(FACQ_IS_OPERATION_LIST(oplist),0);
	return oplist->priv->list->len;
}

/**
 * facq_operation_list_add:
 * @oplist: A #FacqOperationList.
 * @op: A #FacqOperation of any type.
 *
 * Adds a #FacqOperation object to the list, returning the index of the
 * operation in the list, starting at 0.
 * 
 * <note><para>
 * You shouldn't add two times the same operation to the list,
 * the list doesn't have any mechanism to check if an added operation is a
 * duplicate. For example you can add two different instances of the
 * #FacqOperationPlug operation, but not two times the same instance.
 * </para></note>
 *
 * Returns: The position (index) of the operation in the list.
 */
guint facq_operation_list_add(FacqOperationList *oplist,const FacqOperation *op)
{
	g_return_val_if_fail(FACQ_IS_OPERATION(op),0);
	
	g_ptr_array_add(oplist->priv->list,(gpointer)op);
	return facq_operation_list_get_length(oplist);
}

/**
 * facq_operation_list_get:
 * @oplist: A #FacqOperationList object.
 * @i: The index of the operation in the list, starting at 0.
 *
 * Retrieves the #FacqOperation that is in position @i in the list,
 * or %NULL if the position doesn't exist.
 *
 * Returns: A #FacqOperation in the list, or %NULL if @i is out of range.
 */
FacqOperation *facq_operation_list_get(const FacqOperationList *oplist,guint i)
{
	g_return_val_if_fail(FACQ_IS_OPERATION_LIST(oplist),NULL);
	FacqOperation *op = g_ptr_array_index(oplist->priv->list,i);
	return op;
}

/**
 * facq_operation_list_del_and_destroy:
 * @oplist: A #FacqOperationList object (Can be an empty one).
 *
 * Deletes the last operation added to the #FacqOperationList, @oplist,
 * freeing all the resources associated with the operation.
 * The operation will be destroyed.
 *
 * Returns: The number of operations left in the list.
 */
guint facq_operation_list_del_and_destroy(FacqOperationList *oplist)
{ 
	g_return_val_if_fail(FACQ_IS_OPERATION_LIST(oplist),0);

	if(oplist->priv->list->len){
		g_ptr_array_remove_index(oplist->priv->list,oplist->priv->list->len-1);
		return oplist->priv->list->len;
	}
	else return 0;
}

/**
 * facq_operation_list_start:
 * @oplist: A #FacqOperationList (It can be empty).
 * @stmd: A #FacqStreamData.
 * @err: (allow-none): A #GError it will be set in case of error if not %NULL.
 *
 * Starts all the #FacqOperation elements in the #FacqOperationList, @oplist.
 * In case of error the started operations will be stopped again.
 * Detailed errors are output trough #FacqLog.
 *
 * Returns: %TRUE if all the operations had been started without errors
 * or if the list doesn't have any operations, %FALSE in other case.
 */ 
gboolean facq_operation_list_start(FacqOperationList *oplist,const FacqStreamData *stmd,GError **err)
{
	gboolean ret = TRUE;
	guint i = 0, j = 0;
	FacqOperation *op = NULL;
	GError *local_error = NULL;

	g_return_val_if_fail(FACQ_IS_OPERATION_LIST(oplist),FALSE);
	g_return_val_if_fail(FACQ_IS_STREAM_DATA(stmd),FALSE);

	for(i = 0;i < oplist->priv->list->len;i++){
		op = facq_operation_list_get(oplist,i);
		if(!facq_operation_start(op,stmd,&local_error)){
			if(local_error){
				facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
						"Error starting operation: %s",
								local_error->message);
				g_clear_error(&local_error);
			}
			ret = FALSE;
			break;
		}
	}
	/* stop started i operations if any */
	if(!ret){
		facq_log_write("Stopping previous started operations if any",FACQ_LOG_MSG_TYPE_ERROR);
		for(j = 0;j < i;j++){
			op = facq_operation_list_get(oplist,j);
			if(!facq_operation_stop(op,stmd,&local_error)){
				if(local_error){
					facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
								"Error stopping operation: %s",
									local_error->message);
					g_clear_error(&local_error);
				}
			}
		}
		/* Set a human readable error for the user */
		if(err != NULL){
			g_set_error_literal(&local_error,
						FACQ_OPERATION_LIST_ERROR,
							FACQ_OPERATION_LIST_ERROR_FAILED,
								"Unable to start all the operations");
			g_propagate_error(err,local_error);
		}
	}
	return ret;
}

/**
 * facq_operation_list_do:
 * @oplist: A #FacqOperationList object.
 * @chunk: A #FacqChunk object, containing the samples from the stream of data.
 * @stmd: A #FacqStreamData object, containing the relevant information of the
 * stream.
 * @err: A #GError it will be set in case of error if not %NULL.
 *
 * Calls the facq_operation_do() function for each #FacqOperation in the list,
 * making the operations process the data in the #FacqChunk, in the order that
 * the operation were added to the list.
 *
 * Returns: %TRUE if all operations completed the operation successfully or
 * %FALSE in other case.
 */
gboolean facq_operation_list_do(FacqOperationList *oplist,FacqChunk *chunk,const FacqStreamData *stmd,GError **err)
{
	guint i = 0;
	FacqOperation *op = NULL;
	GError *local_error = NULL;

	for(i = 0;i < oplist->priv->list->len;i++){
		op = facq_operation_list_get(oplist,i);
		if(!facq_operation_do(op,chunk,stmd,&local_error)){
			if(err != NULL && local_error)
				g_propagate_error(err,local_error);
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * facq_operation_list_stop:
 * @oplist: A #FacqOperationList (it can be an empty one).
 * @stmd: A #FacqStreamData object.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Stops all the #FacqOperation elements in the list. In case of error the
 * detailed error message for each operation is passed trough #FacqLog.
 * All the operations are stopped, even in case of error, this means that if a
 * stop function fails, the function continues with the next operation in the
 * list until all operations are processed.
 *
 * Returns: %TRUE if all the operations had been stopped without errors, %FALSE in other case.
 */
gboolean facq_operation_list_stop(FacqOperationList *oplist,const FacqStreamData *stmd,GError **err)
{
	gboolean ret = TRUE;
	guint i = 0;
	FacqOperation *op = NULL;
	GError *local_error = NULL;

	g_return_val_if_fail(FACQ_IS_OPERATION_LIST(oplist),FALSE);
	g_return_val_if_fail(FACQ_IS_STREAM_DATA(stmd),FALSE);

	for(i = 0;i < oplist->priv->list->len;i++){
		op = facq_operation_list_get(oplist,i);
		if(!facq_operation_stop(op,stmd,&local_error)){
			if(local_error){
				facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
							"Error while stopping operation: %s",
								local_error->message);
				g_clear_error(&local_error);
			}
			ret = FALSE;
		}
	}
	if(!ret){
		if(err != NULL){
			g_set_error_literal(&local_error,
						FACQ_OPERATION_LIST_ERROR,
							FACQ_OPERATION_LIST_ERROR_FAILED,
								"Unable to stop all the operations");
			g_propagate_error(err,local_error);
		}
	}
	return ret;
}

/**
 * facq_operation_list_free:
 * @oplist: A #FacqOperationList object.
 *
 * Destroys all the contained #FacqOperation and the #FacqOperationList object,
 * freeing all the resources.
 */
void facq_operation_list_free(FacqOperationList *oplist)
{
	g_return_if_fail(FACQ_IS_OPERATION_LIST(oplist));
	g_object_unref(oplist);
}


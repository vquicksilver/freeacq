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
#include "facqchanlist.h"
#include "facqchunk.h"
#include "facqstreamdata.h"
#include "facqoperation.h"

/**
 * SECTION:facqoperation
 * @include:facqoperation.h
 * @short_description: Parent class for operations
 *
 * A #FacqOperation is an abstract class that defines the behavior of 
 * any operation in the system. If you want to add a new operation to the
 * system you must create a new class inheriting for this one, and implement
 * the needed virtual methods overriding them.
 *
 * Note that an operation can modify or read the stream of data but can't add
 * more samples to the stream of data. An operation can work on one or more
 * channels at the same time.
 *
 * An example of operation is #FacqOperationPlug.
 */

/**
 * FacqOperation:
 *
 * Contains all the private details of a #FacqOperation.
 */

/**
 * FacqOperationClass:
 * @opsave: Virtual method that is called when the stream is saved to a
 * #GKeyFile, when the facq_stream_save() function is invoked.
 * To write this method use the relevant #GKeyFile functions to store the
 * operation relevant attributes to the #GKeyFile.
 * @opstart: Virtual method that is invoked when the stream is started.
 * It's optional to implement this method.
 * In this method you can prepare the operation to process the data, making
 * all the required initialization steps. You must return %TRUE if successful
 * %FALSE in other case.
 * @opdo: Virtual method where the operation does it's main functionality.
 * You receive a pointer to your operation, to a #FacqStreamData object
 * containing detailed info about the stream data, and a #FacqChunk with the
 * data samples. This function will be called in an iterative way while new
 * #FacqChunk objects are ready to be processed. In case of error you must
 * set the #GError, and return %FALSE. Note that you are allowed to modify
 * the data in the #FacqChunk, but you shouldn't change the number of samples.
 * @opstop: Virtual method that is called when the stream is stopped.
 * It's optional to implement this method.
 * In this method you are supposed to do whatever things you need to do not process
 * any more data with your operation, if needed. You must return %TRUE if
 * successful, %FALSE in other case.
 * @opfree: Virtual method that is called when the operation is no longer
 * needed. You must provide it. In most cases calling g_object_unref() should be
 * enough.
 *
 */

G_DEFINE_TYPE(FacqOperation,facq_operation,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_NAME,
	PROP_DESC,
	PROP_STARTED
};

struct _FacqOperationPrivate {
	/* Common properties of all operations */
	gchar *name;
	gchar *desc;
	gboolean started;
};

/*****--- GObject magic ---*****/
static void facq_operation_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqOperation *op = FACQ_OPERATION(self);

	switch(property_id){
	case PROP_NAME: op->priv->name = g_value_dup_string(value);
	break;
	case PROP_DESC: op->priv->desc = g_value_dup_string(value);
	break;
	case PROP_STARTED: op->priv->started = g_value_get_boolean(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(op,property_id,pspec);
	}
}

static void facq_operation_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqOperation *op = FACQ_OPERATION(self);

	switch(property_id){
	case PROP_NAME: g_value_set_string(value,op->priv->name);
	break;
	case PROP_DESC: g_value_set_string(value,op->priv->desc);
	break;
	case PROP_STARTED: g_value_set_boolean(value,op->priv->started);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(op,property_id,pspec);
	}
}

static void facq_operation_finalize(GObject *self)
{
	FacqOperation *op = FACQ_OPERATION(self);
	if(op->priv->name)
		g_free(op->priv->name);
	if(op->priv->desc)
		g_free(op->priv->desc);

	if (G_OBJECT_CLASS (facq_operation_parent_class)->finalize)
    		(*G_OBJECT_CLASS (facq_operation_parent_class)->finalize) (self);
}

static void facq_operation_class_init(FacqOperationClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	FacqOperationClass *operation_class = FACQ_OPERATION_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FacqOperationPrivate));

	object_class->set_property = facq_operation_set_property;
	object_class->get_property = facq_operation_get_property;
	object_class->finalize = facq_operation_finalize;
	operation_class->opdo = NULL;
	operation_class->opfree = NULL;
	operation_class->opstart = NULL;
	operation_class->opstop = NULL;
	operation_class->opsave = NULL;

	/* properties */

	g_object_class_install_property(object_class,PROP_NAME,
					g_param_spec_string("name",
							    "The operation name",
							    "The name of the operation",
							    "Unknown",
							    G_PARAM_READWRITE | 
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_DESC,
					g_param_spec_string("description",
							    "The operation description",
							    "The detailed description of the operation",
							    "Unknown",
							    G_PARAM_READWRITE | 
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_STARTED,
					g_param_spec_boolean("started",
						             "Started",
							     "The operation current status",
							     FALSE,
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));
}

static void facq_operation_init(FacqOperation *operation)
{
	operation->priv = G_TYPE_INSTANCE_GET_PRIVATE(operation,FACQ_TYPE_OPERATION,FacqOperationPrivate);

	operation->priv->name = NULL;
	operation->priv->desc = NULL;
	operation->priv->started = FALSE;
}

/*****--- Public methods ---*****/
/**
 * facq_operation_get_name:
 * @op: A #FacqOperation object of any type.
 *
 * Gets the operation name.
 *
 * Returns: The name of the operation.
 *
 */
const gchar *facq_operation_get_name(FacqOperation *op)
{
	g_return_val_if_fail(FACQ_IS_OPERATION(op),NULL);
	return op->priv->name;
}

/**
 * facq_operation_get_description:
 * @op: A #FacqOperation object of any type.
 *
 * Gets the operation description.
 *
 * Returns: The description of the operation.
 *
 */
const gchar *facq_operation_get_description(FacqOperation *op)
{
	g_return_val_if_fail(FACQ_IS_OPERATION(op),NULL);
	return op->priv->desc;
}

/**
 * facq_operation_get_started:
 * @op: A #FacqOperation object.
 *
 * Gets the current status of the #FacqOperation.
 *
 * Returns: %TRUE if the operation it's in started state, %FALSE in other case.
 */
gboolean facq_operation_get_started(FacqOperation *op)
{
	g_return_val_if_fail(FACQ_IS_OPERATION(op),FALSE);
	return op->priv->started;
}

/**
 * facq_operation_to_file:
 * @op: A #FacqOperation object, it can be any type of operation.
 * @file: A #GKeyFile object.
 * @group: The group name for the #GKeyFile, @file.
 *
 * Calls the facq_operation_*_to_file() function related to the operation type,
 * allowing the operation to store important attributes in a #GKeyFile.
 * This function is used by facq_stream_save().
 */
void facq_operation_to_file(FacqOperation *op,GKeyFile *file,const gchar *group)
{
	g_return_if_fail(FACQ_IS_OPERATION(op));
        g_return_if_fail(file);
	g_return_if_fail(g_key_file_has_group(file,group));

        if(FACQ_OPERATION_GET_CLASS(op)->opsave)
                FACQ_OPERATION_GET_CLASS(op)->opsave(op,file,group);
}

/**
 * facq_operation_start:
 * @op: A #FacqOperation object.
 * @stmd: A #FacqStreamData object, containing the relevant properties of the
 * stream.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * This function calls the facq_operation_*_start() function related to the kind
 * of operation. If successful the operation state will change to the started
 * state. If the operation is already started the function will do nothing and
 * will return %TRUE.
 *
 * See #FacqOperationClass for more details.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_operation_start(FacqOperation *op,const FacqStreamData *stmd,GError **err)
{
	gboolean ret = TRUE;

	g_return_val_if_fail(FACQ_IS_OPERATION(op),FALSE);

	if(!op->priv->started){
		if(FACQ_OPERATION_GET_CLASS(op)->opstart)
			ret = FACQ_OPERATION_GET_CLASS(op)->opstart(op,stmd,err);
		else
			ret = TRUE;
		
		op->priv->started = TRUE;
	}

	return ret;
}

/**
 * facq_operation_do:
 * @op: A #FacqOperation object of any type.
 * @chunk: A #FacqChunk containing samples.
 * @stmd: A #FacqStreamData object, containing the relevant stream information.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Executes the main functionality of the operation over the data contained in
 * the #FacqChunk passed to the function. You can use @stmd to know the number
 * of channels or other properties needed to your operation.
 * Note that an operation is allowed to do whatever the operation wants with the
 * data, but can't change the #FacqChunk, only the contained data.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_operation_do(FacqOperation *op,FacqChunk *chunk,const FacqStreamData *stmd,GError **err)
{
	g_return_val_if_fail(FACQ_IS_OPERATION(op),FALSE);
	return FACQ_OPERATION_GET_CLASS(op)->opdo(op,chunk,stmd,err);
}

/**
 * facq_operation_stop:
 * @op: A #FacqOperation object.
 * @stmd: A #FacqStreamData object, containing the relevant stream information.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * This function calls the facq_operation_*_stop() function related to the
 * operation type. See #FacqOperationClass for more details.
 *
 * Returns: %TRUE if successful, %FALSE in other case. If the operation is
 * already stopped the return value will be %TRUE.
 */
gboolean facq_operation_stop(FacqOperation *op,const FacqStreamData *stmd,GError **err)
{
	gboolean ret = TRUE;

	g_return_val_if_fail(FACQ_IS_OPERATION(op),FALSE);
	
	if(op->priv->started){
		if(FACQ_OPERATION_GET_CLASS(op)->opstop)
			ret = FACQ_OPERATION_GET_CLASS(op)->opstop(op,stmd,err);
		else
			ret = TRUE;

		op->priv->started = FALSE;
	}
	return ret;
}

/**
 * facq_operation_free:
 * @op: A #FacqOperation object.
 *
 * Destroys a no longer needed #FacqOperation object, or any
 * object which class it's a child of #FacqOperation.
 */
void facq_operation_free(FacqOperation *op)
{
	g_return_if_fail(FACQ_IS_OPERATION(op));
	FACQ_OPERATION_GET_CLASS(op)->opfree(op);
}

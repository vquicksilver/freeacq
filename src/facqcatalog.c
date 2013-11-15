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
#include "facqi18n.h"
#include "facqcatalog.h"

/**
 * SECTION:facqcatalog
 * @include:facqcatalog.h
 * @short_description: Provides a catalog of stream elements available to the
 * system.
 * @title:FacqCatalog
 * @see_also: #FacqDynDialog
 *
 * A #FacqCatalog, provides a way to store the available types of elements
 * available to use in a #FacqStream by the system. This is needed because
 * depending on compilation options, operating system and other factors some
 * elements like sources, or sink, may not be available. The system needs to
 * know which elements is capable of use at runtime.
 *
 * To create a new #FacqCatalog use facq_catalog_new().
 *
 * Different types of #FacqSource , #FacqOperation and #FacqSink elements can
 * be registered in the #FacqCatalog. You can use facq_catalog_append_source(),
 * facq_catalog_append_operation() and facq_catalog_append_sink() to add
 * supported stream elements to the catalog.
 *
 * When you add an item to the catalog (A source, an operation or a sink) you
 * make an association between the item and certain properties. Each item should
 * have a name, a description, a #FacqDynDialog string, a pointer to an icon
 * (This is optional), and 2 function pointers of #CIConstructor and
 * #CIKeyConstructor types. 
 *
 * The #CIConstructor type function it's needed for
 * creating new objects of that type of element from a #FacqDynDialog along with
 * #FacqDynDialog string, this allows to create graphical gtk dialogs from
 * strings, and parse the user input without the need of coding new dialogs.
 * When the user interacts with a #FacqDynDialog a #GArray with the parameters
 * from the user will be created, your #CIConstructor function should parse 
 * and check that this parameters are correct, and call the object constructor
 * with them, returning the new created object, or %NULL in case of error.
 *
 * The #CIKeyConstructor type function it's needed for
 * creating new object of that type of element from a #GKeyFile object.
 * This allows the system to store the element parameters on a #GKeyFile,
 * allowing the system to store the parameters of all the elements in a
 * #FacqStream. Later the user can reload that stream from a file without the
 * need of recreating all the objects manually. When the stream is loaded from
 * the file, the system will call the correspondent #CIKeyConstructor for each
 * type, your function should take the group name received and the #GKeyFile
 * object and read the parameters from the #GKeyFile, and use them to call the
 * real constructor of the object.
 *
 * The CI stands for Catalog Item if you are wondering what is the meaning.
 *
 * To retrieve certain properties from an item, you can use the following
 * functions, and now which kind of item is (A source, an operation, or a sink)
 * and the order that the item has in that list of items, you can use
 * facq_catalog_get_dyn_diag_string() to retrieve the #FacqDynDialog string,
 * facq_catalog_get_name() to retrieve the name of the item,
 * facq_catalog_get_description() to retrieve the description of the item, and
 * facq_catalog_get_icon() to retrieve the icon if any.
 *
 * To create an object of any registered type in the #FacqCatalog you can use
 * facq_catalog_constructor_call() and to do the same from a #GKeyFile and a
 * group name you can use facq_catalog_item_from_key_file().
 *
 * Finally to destroy a #FacqCatalog use facq_catalog_free().
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * Internally a #FacqCatalog uses 3 #GArray objects to keep the registered
 * item types. One array is used for keeping track of sources, another one stores 
 * operations and the third sinks.
 * </para>
 * </sect1>
 */

/**
 * FacqCatalog:
 *
 * Contains the private details of the #FacqCatalog.
 */

/**
 * FacqCatalogClass:
 *
 * Class for the #FacqCatalog objects.
 */

/**
 * FacqCatalogType:
 * @FACQ_CATALOG_TYPE_SOURCE: You want to operate over a source type.
 * @FACQ_CATALOG_TYPE_OPERATION: You want to operate over an operation type.
 * @FACQ_CATALOG_TYPE_SINK: You want to operate over a sink type.
 *
 * Enum that stores the types of items that can be added to the #FacqCatalog.
 */

 /**
  * FacqCatalogError:
  * @FACQ_CATALOG_ERROR_FAILED: Some error happened in the catalog.
  *
  * Enum values for the possible errors in a #FacqCatalog.
  */
G_DEFINE_TYPE(FacqCatalog,facq_catalog,G_TYPE_OBJECT);

struct _FacqCatalogPrivate {
	GArray *SourceItems;
	GArray *OperationItems;
	GArray *SinkItems;
};

GQuark facq_catalog_error_quark(void)
{
	return g_quark_from_static_string("facq-catalog-error-quark");
}

/*****--- Private methods ---*****/
static GArray *facq_catalog_array_from_type(const FacqCatalog *cat,FacqCatalogType type)
{
	switch(type){
	case FACQ_CATALOG_TYPE_SOURCE:
		return cat->priv->SourceItems;
	break;
	case FACQ_CATALOG_TYPE_OPERATION:
		return cat->priv->OperationItems;
	break;
	case FACQ_CATALOG_TYPE_SINK:
		return cat->priv->SinkItems;
	break;
	default:
		return NULL;
	}
}

static CatalogItem *facq_catalog_item_new(const gchar *name,const gchar *desc,const gchar *ddstring,gpointer icon,CIConstructor cons,CIKeyConstructor kcons)
{
	CatalogItem *ret = g_malloc0(sizeof(CatalogItem));

	ret->name = g_strdup(name);
	ret->desc = g_strdup(desc);
	ret->dyn_dialog_string = g_strdup(ddstring);
	ret->icon = icon;
	ret->constructor = cons;
	ret->keyconstructor = kcons;

	return ret;
}

static void facq_catalog_item_free(gpointer _item)
{
	CatalogItem *item = (CatalogItem *)_item;

	g_free(item->name);
	g_free(item->desc);
	g_free(item->dyn_dialog_string);
	if(item->icon)
		g_object_unref(G_OBJECT(item->icon));
}

static void facq_catalog_append_item(GArray *Items,CatalogItem *item)
{
	g_array_append_vals(Items,item,1);
}

/*****--- GObject magic ---*****/
static void facq_catalog_finalize(GObject *self)
{
	FacqCatalog *catalog = FACQ_CATALOG(self);
#if GLIB_MINOR_VERSION < 32
	guint i = 0;
	CatalogItem *item = NULL;
#endif
	
	if(catalog->priv->SourceItems){
#if GLIB_MINOR_VERSION < 32
		for(i = 0;i < catalog->priv->SourceItems->len;i++){
			item = &g_array_index(catalog->priv->SourceItems,CatalogItem,i);
			g_array_remove_index(catalog->priv->SourceItems,i);
			facq_catalog_item_free(item);
		}
#endif
		g_array_free(catalog->priv->SourceItems,TRUE);
	}
	
	if(catalog->priv->OperationItems){
#if GLIB_MINOR_VERSION < 32
		for(i = 0;i < catalog->priv->OperationItems->len;i++){
			item = &g_array_index(catalog->priv->OperationItems,CatalogItem,i);
			g_array_remove_index(catalog->priv->OperationItems,i);
			facq_catalog_item_free(item);
		}
#endif
		g_array_free(catalog->priv->OperationItems,TRUE);
	}

	if(catalog->priv->SinkItems){
#if GLIB_MINOR_VERSION < 32
		for(i = 0;i < catalog->priv->SinkItems->len;i++){
			item = &g_array_index(catalog->priv->SinkItems,CatalogItem,i);
			g_array_remove_index(catalog->priv->SinkItems,i);
			facq_catalog_item_free(item);
		}
#endif
		g_array_free(catalog->priv->SinkItems,TRUE);
	}

	if (G_OBJECT_CLASS (facq_catalog_parent_class)->finalize)
    		(*G_OBJECT_CLASS (facq_catalog_parent_class)->finalize) (self);
}

static void facq_catalog_constructed(GObject *self)
{
	FacqCatalog *catalog = FACQ_CATALOG(self);

	catalog->priv->SourceItems = g_array_new(FALSE,TRUE,sizeof(CatalogItem));
	catalog->priv->OperationItems = g_array_new(FALSE,TRUE,sizeof(CatalogItem));
	catalog->priv->SinkItems = g_array_new(FALSE,TRUE,sizeof(CatalogItem));
#if GLIB_MINOR_VERSION >= 32
	g_array_set_clear_func(catalog->priv->SourceItems,(GDestroyNotify)facq_catalog_item_free);
	g_array_set_clear_func(catalog->priv->OperationItems,(GDestroyNotify)facq_catalog_item_free);
	g_array_set_clear_func(catalog->priv->SinkItems,(GDestroyNotify)facq_catalog_item_free);
#endif
}

static void facq_catalog_class_init(FacqCatalogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FacqCatalogPrivate));

	object_class->finalize = facq_catalog_finalize;
	object_class->constructed = facq_catalog_constructed;
}

static void facq_catalog_init(FacqCatalog *catalog)
{
	catalog->priv = G_TYPE_INSTANCE_GET_PRIVATE(catalog,FACQ_TYPE_CATALOG,FacqCatalogPrivate);

	catalog->priv->SourceItems = NULL;
	catalog->priv->OperationItems = NULL;
	catalog->priv->SinkItems = NULL;
}

/* public methods */

/**
 * facq_catalog_new:
 *
 * Creates a new #FacqCatalog object.
 *
 * Returns: a new #FacqCatalog object.
 */
FacqCatalog *facq_catalog_new(void)
{
	return g_object_new(FACQ_TYPE_CATALOG,NULL);
}

/**
 * facq_catalog_append_source:
 * @cat: A #FacqCatalog object.
 * @name: The name of the type, for example "Special source".
 * @desc: A brief description of the type, for example "A special source that
 * does foo".
 * @ddstring: A #FacqDynDialog string, see #FacqDynDialog for more details.
 * @icon: (allow-none): A pointer to an icon in any format, or %NULL.
 * @cons: A pointer to a #CIConstructor type function.
 * @kcons: A pointer to a #CIKeyConstructor type function.
 *
 * Appends a new type of source to the #FacqCatalog, allowing the system to use
 * it.
 */
void facq_catalog_append_source(FacqCatalog *cat,const gchar *name,const gchar *desc,const gchar *ddstring,gpointer icon,CIConstructor cons,CIKeyConstructor kcons)
{
	CatalogItem *item = NULL;

	g_return_if_fail(FACQ_IS_CATALOG(cat));

	item = facq_catalog_item_new(name,desc,ddstring,icon,cons,kcons);
	facq_catalog_append_item(cat->priv->SourceItems,item);
}

/**
 * facq_catalog_append_operation:
 * @cat: A #FacqCatalog object.
 * @name: The name of the type, for example "Special operation".
 * @desc: A brief description of the type, for example "A special operation that
 * does foo".
 * @ddstring: A #FacqDynDialog string, see #FacqDynDialog for more details.
 * @icon: (allow-none): A pointer to an icon in any format, or %NULL.
 * @cons: A pointer to a #CIConstructor type function.
 * @kcons: A pointer to a #CIKeyConstructor type function.
 *
 * Appends a new type of operation to the #FacqCatalog, allowing the system to
 * use it.
 */
void facq_catalog_append_operation(FacqCatalog *cat,const gchar *name,const gchar *desc,const gchar *ddstring,gpointer icon,CIConstructor cons,CIKeyConstructor kcons)
{
	CatalogItem *item = NULL;

	g_return_if_fail(FACQ_IS_CATALOG(cat));
		
	item = facq_catalog_item_new(name,desc,ddstring,icon,cons,kcons);
	facq_catalog_append_item(cat->priv->OperationItems,item);
}

/**
 * facq_catalog_append_sink:
 * @cat: A #FacqCatalog object.
 * @name: The name of the type, for example "Special sink".
 * @desc: A brief description of the type, for example "A special sink that
 * does foo".
 * @ddstring: A #FacqDynDialog string, see #FacqDynDialog for more details.
 * @icon: (allow-none): A pointer to an icon in any format, or %NULL.
 * @cons: A pointer to a #CIConstructor type function.
 * @kcons: A pointer to a #CIKeyConstructor type function.
 *
 * Appends a new type of sink to the #FacqCatalog, allowing the system to use
 * it.
 */
void facq_catalog_append_sink(FacqCatalog *cat,const gchar *name,const gchar *desc,const gchar *ddstring,gpointer icon,CIConstructor cons,CIKeyConstructor kcons)
{
	CatalogItem *item = NULL;

	g_return_if_fail(FACQ_IS_CATALOG(cat));

	item = facq_catalog_item_new(name,desc,ddstring,icon,cons,kcons);
	facq_catalog_append_item(cat->priv->SinkItems,item);
}

/**
 * facq_catalog_get_sources:
 * @cat: A #FacqCatalog object.
 *
 * Gets the private #GArray that contains all the kinds of source registered in
 * the system. This function is used by #FacqCatalogDialog you shouldn't use
 * this if you don't know what you are doing.
 *
 * Returns: A #GArray.
 */
GArray *facq_catalog_get_sources(const FacqCatalog *cat)
{
	g_return_val_if_fail(FACQ_IS_CATALOG(cat),NULL);
	return cat->priv->SourceItems;
}

/**
 * facq_catalog_get_operations:
 * @cat: A #FacqCatalog object.
 *
 * Gets the private #GArray that contains all the kinds of operations registered
 * in the system. This function is used by #FacqCatalogDialog you shouldn't use
 * this if you don't know what you are doing.
 */
GArray *facq_catalog_get_operations(const FacqCatalog *cat)
{
	g_return_val_if_fail(FACQ_IS_CATALOG(cat),NULL);
	return cat->priv->OperationItems;
}

/**
 * facq_catalog_get_sinks:
 * @cat: A #FacqCatalog object.
 *
 * Gets the private #GArray that contains all the kinds of sinks registered
 * in the system. This function is used by #FacqCatalogDialog you shouldn't use
 * this if you don't know what you are doing.
 */
GArray *facq_catalog_get_sinks(const FacqCatalog *cat)
{
	g_return_val_if_fail(FACQ_IS_CATALOG(cat),NULL);
	return cat->priv->SinkItems;
}

/**
 * facq_catalog_get_dyn_diag_string:
 * @cat: A #FacqCatalog object.
 * @type: A #FacqCatalogType valid value.
 * @index: The index of the type, that is if you want the first kind of source
 * added put 0, and so on.
 *
 * Gets the #FacqDynDialog string for the specified type. This string can be
 * used to build a gtk dialog to ask to the user the desired parameters for 
 * the creation of an object.
 *
 * Returns: The #FacqDynDialog string. Free it with g_free() when no longer
 * needed.
 */
gchar *facq_catalog_get_dyn_diag_string(const FacqCatalog *cat,FacqCatalogType type,guint index)
{
	GArray *array = facq_catalog_array_from_type(cat,type);
	CatalogItem *item = NULL;
	gchar *ret = NULL;

	item = &g_array_index(array,CatalogItem,index);
	ret = g_strdup(item->dyn_dialog_string);
	return ret;
}

/**
 * facq_catalog_get_name:
 * @cat: A #FacqCatalog object.
 * @type: A #FacqCatalogType valid value.
 * @index: The index of the type, that is if you want the first kind of source
 * added put 0, and so on.
 *
 * Gets the name for the specified type.
 *
 * Returns: The name string. Free it with g_free() when no longer needed.
 */
gchar *facq_catalog_get_name(const FacqCatalog *cat,FacqCatalogType type,guint index)
{
	GArray *array = facq_catalog_array_from_type(cat,type);
	CatalogItem *item = NULL;
	gchar *ret = NULL;

	item = &g_array_index(array,CatalogItem,index);
	ret = g_strdup(item->name);

	return ret;
}

/**
 * facq_catalog_get_description:
 * @cat: A #FacqCatalog object.
 * @type: A #FacqCatalogType valid value.
 * @index: The index of the type, that is if you want the first kind of source
 * added put 0, and so on.
 *
 * Gets the description for the specified type.
 *
 * Returns: The description string. Free it with g_free() when no longer needed.
 */
gchar *facq_catalog_get_description(const FacqCatalog *cat,FacqCatalogType type,guint index)
{
	GArray *array = facq_catalog_array_from_type(cat,type);
	CatalogItem *item = NULL;
	gchar *ret = NULL;

	item = &g_array_index(array,CatalogItem,index);
	ret = g_strdup(item->desc);
	return ret;
}

/**
 * facq_catalog_constructor_call:
 * @cat: A #FacqCatalog object.
 * @type: A #FacqCatalogType valid value.
 * @index: The index of the type, that is if you want the first kind of source
 * added put 0, and so on.
 * @params: A #GArray with the input parameters for the creation of the new
 * object.
 * @error: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new objects from the #FacqCatalog. The object will be a source, an
 * operation or a sink.
 *
 * Returns: The new created object if successful or %NULL in case of error.
 */
gpointer facq_catalog_constructor_call(const FacqCatalog *cat,FacqCatalogType type,guint index,const GPtrArray *params,GError **error)
{
	GArray *array = facq_catalog_array_from_type(cat,type);
	CatalogItem *item = NULL;

	item = &g_array_index(array,CatalogItem,index);
	if(item->constructor)
		return item->constructor(params,error);
	else
		return NULL;
}

/**
 * facq_catalog_get_icon:
 * @cat: A #FacqCatalog object.
 * @type: A #FacqCatalogType valid value.
 * @index: The index of the type, that is if you want the first kind of source
 * added put 0, and so on.
 *
 * Gets the icon for the specified type if any.
 *
 * Returns: The icon if any or %NULL.
 */
gpointer facq_catalog_get_icon(const FacqCatalog *cat,FacqCatalogType type,guint index)
{
	GArray *array = facq_catalog_array_from_type(cat,type);
	CatalogItem *item = NULL;

	item = &g_array_index(array,CatalogItem,index);
	return item->icon;
}

/**
 * facq_catalog_item_from_key_file:
 * @key_file: A #GKeyFile object.
 * @group_name: The group name to use in the #GKeyFile object.
 * @name: The name of the type of object you want to create.
 * @cat: A #FacqCatalog object.
 * @type: A #FacqCatalogType valid value.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new object of the requested type, taking the parameters from a
 * #GKeyFile object. It also checks that the requested object type is registered
 * in the #FacqCatalog else the function will fail.
 *
 * Returns: The new requested object, or %NULL in case of error.
 */
gpointer facq_catalog_item_from_key_file(GKeyFile *key_file,const gchar *group_name,const gchar *name,const FacqCatalog *cat,FacqCatalogType type,GError **err)
{
	GError *local_err = NULL;
	GArray *items = NULL;
	CatalogItem *item = NULL;
	guint i = 0;
	gpointer ret = NULL;

	/* check that name is in the catalog */
	items = facq_catalog_array_from_type(cat,type);
	for(i = 0; i < items->len; i++){
		item = &g_array_index(items,CatalogItem,i);
		if(g_strcmp0(item->name,name) == 0){
			if(item->keyconstructor)
				ret = item->keyconstructor(group_name,key_file,&local_err);
			break;
		}
	}

	if(local_err)
		goto error;
	
	if(ret == NULL && local_err == NULL){
		g_set_error_literal(&local_err,FACQ_CATALOG_ERROR,
					FACQ_CATALOG_ERROR_FAILED,_("Item not supported"));
		goto error;
	}

	return ret;

	error:
	if(local_err){
		if(err)
		g_propagate_error(err,local_err);
	}
	return NULL;
}

/**
 * facq_catalog_free:
 * @catalog: a #FacqCatalog object.
 *
 * Destroys a #FacqCatalog object.
 */
void facq_catalog_free(FacqCatalog *catalog)
{
	g_assert(FACQ_IS_CATALOG(catalog));
	g_object_unref(catalog);
}

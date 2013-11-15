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
#include <strings.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqresources.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqchunk.h"
#include "facqstreamdata.h"
#include "facqfile.h"
#include "facqsink.h"
#include "facqsinkfile.h"

/**
 * SECTION:facqsinkfile
 * @title:FacqSinkFile
 * @short_description: Provides a data sink to store an acquisition on a file.
 * @include:facqsinkfile.h
 *
 * #FacqSinkFile provides a data sink to store an acquisition on a file.
 * For doing this #FacqSinkFile uses the #FacqFile functions.
 * The samples acquired will be stored on the file along with some other
 * relevant data, like the sampling period... see #FacqFile for more details.
 *
 * #FacqSinkFile implements the #FacqSink class take a look there if you need
 * more details.
 *
 * For creating a new #FacqSinkFile you must call facq_sink_file_new(),
 * to use it you must call first facq_sink_start(), and then you must call
 * in an iterative way facq_sink_file_poll() and facq_sink_file_write(), or
 * use a #FacqStream that will do all those things for you.
 * When you don't need to write more data simply call facq_sink_stop() and
 * facq_sink_file_free() or facq_sink_free() to destroy the object.
 *
 * facq_sink_file_to_file(), facq_sink_file_key_constructor() and
 * facq_sink_file_constructor() are used by the system to store the config
 * and to recreate #FacqSinkFile objects. See facq_sink_to_file() 
 * the #CIConstructor type and the #CIKeyConstructor for more info.
 */

/**
 * FacqSinkFile:
 *
 * Contains the private details of the #FacqSinkFile.
 */

/**
 * FacqSinkFileClass:
 *
 * Class for the #FacqSinkFile objects.
 */

static void facq_sink_file_initable_iface_init(GInitableIface  *iface);
static gboolean facq_sink_file_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);

G_DEFINE_TYPE_WITH_CODE(FacqSinkFile,facq_sink_file,FACQ_TYPE_SINK,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_sink_file_initable_iface_init));

enum {
	PROP_0,
	PROP_FILENAME,
};

struct _FacqSinkFilePrivate {
	FacqFile *file;
	gchar *filename;
	GError *construct_error;
};

/*****--- GObject magic ---*****/
static void facq_sink_file_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqSinkFile *sinkfile = FACQ_SINK_FILE(self);

	switch(property_id){
	case PROP_FILENAME: sinkfile->priv->filename = g_value_dup_string(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(sinkfile,property_id,pspec);
	}
}

static void facq_sink_file_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqSinkFile *sinkfile = FACQ_SINK_FILE(self);

	switch(property_id){
	case PROP_FILENAME: g_value_set_string(value,sinkfile->priv->filename);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(sinkfile,property_id,pspec);
	}
}
static void facq_sink_file_finalize(GObject *self)
{
	FacqSinkFile *sinkfile = FACQ_SINK_FILE(self);

	g_clear_error(&sinkfile->priv->construct_error);

	if(sinkfile->priv->file)
		facq_file_free(sinkfile->priv->file);

	g_free(sinkfile->priv->filename);
	G_OBJECT_CLASS (facq_sink_file_parent_class)->finalize (self);
}

static void facq_sink_file_constructed(GObject *self)
{
	FacqSinkFile *sinkfile = FACQ_SINK_FILE(self);

	g_assert(sinkfile->priv->filename != NULL);
	sinkfile->priv->file = 
		facq_file_new(sinkfile->priv->filename,
				&sinkfile->priv->construct_error);
}

static void facq_sink_file_class_init(FacqSinkFileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	FacqSinkClass *sink_class = FACQ_SINK_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FacqSinkFilePrivate));

	object_class->constructed = facq_sink_file_constructed;
	object_class->finalize = facq_sink_file_finalize;
	object_class->set_property = facq_sink_file_set_property;
	object_class->get_property = facq_sink_file_get_property;
	sink_class->sinksave = facq_sink_file_to_file;
	sink_class->sinkstart = facq_sink_file_start;
	sink_class->sinkpoll = facq_sink_file_poll;
	sink_class->sinkwrite = facq_sink_file_write;
	sink_class->sinkstop = facq_sink_file_stop;
	sink_class->sinkfree = facq_sink_file_free;

	g_object_class_install_property(object_class,PROP_FILENAME,
					g_param_spec_string("filename",
							     "Filename",
							     "The sink filename as given by the user",
							     "Unknown",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));
}

static void facq_sink_file_init(FacqSinkFile *sink)
{
	sink->priv = G_TYPE_INSTANCE_GET_PRIVATE(sink,FACQ_TYPE_SINK_FILE,FacqSinkFilePrivate);
	sink->priv->filename = NULL;
	sink->priv->file = NULL;
}

/*****--- GInitable implementation ---*****/
static void facq_sink_file_initable_iface_init(GInitableIface *iface)
{
	iface->init = facq_sink_file_initable_init;
}

static gboolean facq_sink_file_initable_init(GInitable *initable,GCancellable *cancellable,GError  **error)
{
	FacqSinkFile *sink = NULL;

	g_return_val_if_fail(FACQ_IS_SINK_FILE(initable),FALSE);
	sink = FACQ_SINK_FILE(initable);
	if(cancellable != NULL){
		g_set_error_literal(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                           "Cancellable initialization not supported");
      		return FALSE;
    	}
	if(sink->priv->construct_error){
		if (error)
        	*error = g_error_copy(sink->priv->construct_error);
      		return FALSE;
	}
	return TRUE;
}

/*****--- Public methods ---*****/
/**
 * facq_sink_file_constructor:
 * @user_input: A #GPtrArray with the parameters from the user.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqSinkFile object from a #GPtrArray, @user_input,
 * with a pointer to the filename.
 *
 * This function is used by #FacqCatalog, for creating a new #FacqSinkFile with
 * the parameters provided by the user in a #FacqDynDialog, take a look at this
 * other objects for more details, and to the #CIConstructor type.
 *
 * Returns: A new #FacqSinkFile, or %NULL in case of error.
 */
gpointer facq_sink_file_constructor(const GPtrArray *user_input,GError **err)
{
	gchar *filename = NULL;

	filename = g_ptr_array_index(user_input,0);
	return facq_sink_file_new(filename,err);
}

/**
 * facq_sink_file_key_constructor:
 * @group_name: A string with the group name for the #GKeyFile.
 * @key_file: A #GKeyFile object.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * It's purpose it's to create a new #FacqSinkFile object from a #GKeyFile and
 * a @group_name. This function is used by #FacqCatalog. See #CIKeyConstructor
 * for more details.
 *
 * Returns: %NULL in case of error, or a new #FacqSinkFile object if successful.
 *
 */
gpointer facq_sink_file_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err)
{
	GError *local_err = NULL;
	gchar *filename = NULL;

	filename = g_key_file_get_string(key_file,group_name,"filename",&local_err);
	if(local_err)
		goto error;

	return facq_sink_file_new(filename,err);

	error:
	if(local_err){
		if(err)
			g_propagate_error(err,local_err);
	}
	return NULL;
}

/**
 * facq_sink_file_new:
 * @filename: The desired filename where the sink will store the data.
 * @error: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqSinkFile object that will store the data to the file
 * pointer by @filename. The file will be created/overwritten if needed.
 *
 * Returns: A new #FacqSinkFile if successful or %NULL in case of error.
 */
FacqSinkFile *facq_sink_file_new(const gchar *filename,GError **error)
{
	return FACQ_SINK_FILE(g_initable_new(FACQ_TYPE_SINK_FILE,
					     NULL,
					     error,
					     "name",
					     facq_resources_names_sink_file(),
					     "description",facq_resources_descs_sink_file(),
					     "filename",filename,
					     NULL)
				);
}

/*****--- Virtuals ---*****/
/**
 * facq_sink_file_to_file:
 * @sink: A #FacqSinkFile casted to #FacqSink.
 * @file: A #GKeyFile object where the sink properties will be stored.
 * @group: A string with the group name.
 *
 * Implements the facq_sink_to_file() method.
 * Saves the filename property of a #FacqSinkFile, @sink,
 * to a #GKeyFile, @file, using the group
 * name @group.
 * This is used by the facq_stream_save() function, and you shouldn't need
 * to call this.
 */
void facq_sink_file_to_file(FacqSink *sink,GKeyFile *file,const gchar *group)
{
	FacqSinkFile *sinkfile = FACQ_SINK_FILE(sink);

	g_key_file_set_string(file,group,"filename",sinkfile->priv->filename);
}

/**
 * facq_sink_file_start:
 * @sink: A #FacqSinkFile casted to #FacqSink.
 * @stmd: A #FacqStreamData object with the relevant stream information.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Starts the #FacqSinkFile, allowing it to prepare for the data writing.
 * The header of the #FacqFile will be written in this step.
 *
 * Returns: %TRUE if successful or %FALSE in other case.
 */
gboolean facq_sink_file_start(FacqSink *sink,const FacqStreamData *stmd,GError **err)
{
	GError *local_err = NULL;
	FacqSinkFile *sinkfile = FACQ_SINK_FILE(sink);
	FacqFile *file = NULL;

	file = sinkfile->priv->file;

	facq_file_reset(file,&local_err);
	if(local_err)
		goto error;

	facq_file_write_header(file,stmd,&local_err);
	if(local_err)
		goto error;

	return TRUE;

	error:
	if(local_err){
		g_propagate_error(err,local_err);
	}
	return FALSE;
}

/**
 * facq_sink_file_poll:
 * @sink: A #FacqSinkFile object casted to #FacqSink.
 * @stmd: A #FacqStreamData object with the relevant information of the stream.
 *
 * Polls the file to check if it can be written without blocking.
 * Internally facq_file_poll() is used, so the return values that
 * can be provided by this function are the same as in facq_file_poll().
 *
 * Returns: See facq_file_poll() return values.
 */
gint facq_sink_file_poll(FacqSink *sink,const FacqStreamData *stmd)
{
	FacqSinkFile *sinkfile = NULL;
	FacqFile *file = NULL;

	g_return_val_if_fail(FACQ_IS_SINK_FILE(sink),-1);

	sinkfile = FACQ_SINK_FILE(sink);
	file = sinkfile->priv->file;

	return facq_file_poll(file);
}
/* return -1 on error, 0 on timeout, 1 if ready to write */

/**
 * facq_sink_file_write:
 * @sink: A #FacqSinkFile object casted to #FacqSink.
 * @stmd: A #FacqStreamData with the relevant information of the stream.
 * @chunk: A #FacqChunk with the data to be written.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Writes the samples contained in the #FacqChunk, @chunk to the #FacqFile
 * managed by the @sink.
 *
 * Returns: %G_IO_STATUS_NORMAL if successful, any other #GIOStatus in other
 * case.
 */
GIOStatus facq_sink_file_write(FacqSink *sink,const FacqStreamData *stmd,FacqChunk *chunk,GError **err)
{
	GIOStatus ret = 0;
	FacqSinkFile *sinkfile = FACQ_SINK_FILE(sink);
	FacqFile *file = sinkfile->priv->file;
	GError *local_err = NULL;
	
	ret = facq_file_write_samples(file,chunk,&local_err);
	if(local_err)
		g_propagate_error(err,local_err);
	return ret;
}

/**
 * facq_sink_file_stop:
 * @sink: A #FacqSinkFile object casted to #FacqSink.
 * @stmd: A #FacqStreamData containing the relevant stream information.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Stops the #FacqSinkFile object. After calling this function you shouldn't
 * write data to the sink, until facq_sink_file_start() is called again.
 * It writes the so called tail to the file, see #FacqFile for more info.
 *
 * Returns: %TRUE if sucessful, %FALSE in other case.
 */
gboolean facq_sink_file_stop(FacqSink *sink,const FacqStreamData *stmd,GError **err)
{
	GError *local_err = NULL;
	FacqSinkFile *sinkfile = FACQ_SINK_FILE(sink);
	FacqFile *file = NULL;

	file = sinkfile->priv->file;
	if(!facq_file_write_tail(file,&local_err)){
		if(local_err)
			goto error;
	}

	if( !facq_file_stop(file,&local_err) ){
		if(local_err)
			goto error;
		return FALSE;
	}

	return TRUE;

	error:
	if(local_err)
		g_propagate_error(err,local_err);
	return FALSE;
}

/**
 * facq_sink_file_free:
 * @sink: A #FacqSinkFile casted to #FacqSink.
 *
 * Destroys a no longer needed #FacqSinkFile.
 */
void facq_sink_file_free(FacqSink *sink)
{
	g_return_if_fail(FACQ_IS_SINK(sink));
	g_object_unref(G_OBJECT(sink));
}

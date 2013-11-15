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
#include "facqlog.h"
#include <unistd.h>
#if GLIB_MINOR_VERSION >= 30
#ifdef G_OS_UNIX
#include <glib-unix.h>
#endif /* G_OS_UNIX */
#endif /* 30 */
#include <glib/gstdio.h>
#include <string.h>
#include <strings.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqglibcompat.h"
#include "gdouble.h"
#include "facqunits.h"
#include "facqchunk.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqfile.h"

#define FIRST_LINE "Sampling period %.9g seconds\n"
#define SECOND_LINE_ATOM "channel %u (%s)\t"

/**
 * SECTION:facqfile
 * @short_description: Implementation of the FacqFile format and related
 * operations.
 * @title:FacqFile
 * @include: facqfile.h
 *
 * This module provides a way to store acquisition data on a binary file. The
 * class that provides this service along with other operations is called
 * #FacqFile.
 *
 * When writing data to a file in the filesystem #FacqFile follows a strict
 * format convention. This format ensures the integrity of the file after the
 * acquisition has finished using a SHA256 algorithm, the digest is stored in
 * the file so the integrity of the file can be verified later.
 *
 * To create a file see <link linkend="facqfile-steps">Steps to create a
 * FacqFile</link>, to read data from a file see 
 * <link linkend="facqfile-reading">Reading a FacqFile</link>.
 *
 * To convert a binary file to human readable format, so it can be processed
 * with other software, use facq_file_to_human().
 *
 * <sect1 id="facqfile-steps">
 *  <title>Steps to create a FacqFile</title>
 *   <para>
 *   You can use this "recipe" to create a #FacqFile:
 *   <orderedlist>
 *    <listitem>
 *     <para>
 *     Call facq_file_new().
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Call facq_file_reset().
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Call facq_file_write_header().
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Call facq_file_poll() if ready call facq_file_write_samples().
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Call facq_file_write_tail().
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Call facq_file_stop().
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     Call facq_file_free().
 *     </para>
 *    </listitem>
 *   </orderedlist>
 *   </para>
 * </sect1>
 * 
 * <sect1 id="facqfile-reading">
 *  <title>Reading a FacqFile</title>
 *  <para>
 *  <orderedlist>
 *  <listitem>
 *   <para>
 *   Call facq_file_open().
 *   </para>
 *  </listitem>
 *  <listitem>
 *   <para>
 *   Call facq_file_read_header(), this will create a #FacqStreamData object.
 *   </para>
 *  </listitem>
 *  <listitem>
 *   <para>
 *   Call facq_file_read_tail().
 *   </para>
 *  </listitem>
 *  </orderedlist>
 *  </para>
 * </sect1>
 *
 * <sect1 id="facqfile-format">
 *  <title>File format</title>
 *   <para>
 *   This section explains the format that is used by #FacqFile. #FacqFile uses
 *   binary data (In big endian) for storing the acquired samples and some 
 *   important details of the acquisition in binary files. When you examine a
 *   #FacqFile stored on disk you can view three sections as described on the
 *   following figure:
 *   <informalexample>
 *    <programlisting>
 *     0        x                       y              EOF
 *     --------------------------------------------------
 *     |        |                       |               |
 *     | Header | Samples (Interleaved) | Tail (Footer) |
 *     |        |                       |               |
 *     --------------------------------------------------
 *
 *     x >= 16 bytes + 2 * (n_channels * 4 bytes) + 2 * (n_channels * 8 bytes) 
 *     y = EOF - 40 bytes
 *    </programlisting>
 *   </informalexample>
 *   </para>
 *  <sect2 id="facqfile-header">
 *   <title>Header information</title>
 *   <para>
 *   This section contains info about the fields that form the header.
 *
 *   The header is the first section that can be found on a #FacqFile. 
 *   All the data in the header is written on big endian data format.
 *   The following fields are part of the header:
 *   
 *    <orderedlist>
 *     <listitem>
 *      <para>
 *      <emphasis>magic</emphasis> &mdash; A 32 bit word with a magic number that identifies that the file
 *      is a valid file.
 *      </para>
 *     </listitem>
 *     <listitem>
 *      <para>
 *      <emphasis>period</emphasis> &mdash; A 64 bit double with the
 *      sampling period length in seconds.
 *      </para>
 *     </listitem>
 *     <listitem>
 *      <para>
 *      <emphasis>number of channels</emphasis> &mdash; A 32 bit unsigned
 *      integer representing the number of channels used in the acquisition.
 *      </para>
 *     </listitem>
 *     <listitem>
 *      <para>
 *      <emphasis>channels</emphasis> &mdash; Number of channels per 32 bit unsigned
 *      integer entries with the index of the physical channel. So if the
 *      number of channels equals two, the header should contain two entries,
 *      each entry will be a number (The index).
 *      </para>
 *     </listitem>
 *     <listitem>
 *      <para>
 *      <emphasis>units</emphasis> &mdash;
 *      </para>
 *     </listitem>
 *     <listitem>
 *      <para>
 *      <emphasis>maximums</emphasis> &mdash;
 *      </para>
 *     </listitem>
 *     <listitem>
 *      <para>
 *      <emphasis>minimums</emphasis> &mdash;
 *      </para>
 *     </listitem>
 *    </orderedlist>
 *   </para>
 *  </sect2>
 *  <sect2 id="facqfile-tail">
 *  <title>Tail information</title>
 *   <para>
 *   This section contains info about the fields that form the tail (footer).
 *
 *   The tail is the last section that can be found on a #FacqFile. 
 *   All the data in the tail is written on big endian data format.
 *   The following fields are part of the tail:
 *   <orderedlist>
 *    <listitem>
 *     <para>
 *     <emphasis>number of samples</emphasis> &mdash; The total number of
 *     samples that have been written to the file. (8 bytes in guint64 format).
 *     </para>
 *    </listitem>
 *    <listitem>
 *     <para>
 *     <emphasis>digest</emphasis> &mdash; The SHA256 Hash that results from
 *     computing all the bytes in the file minus the digest. (32 bytes).
 *     </para>
 *    </listitem>
 *   </orderedlist>
 *   </para>
 *  </sect2>
 * </sect1>
 */

/**
 * FacqFile:
 *
 * Contains the private details of the #FacqFile objects.
 */

/**
 * FacqFileClass:
 *
 * Class for the #FacqFile objects.
 */

/**
 * FacqFileError:
 * @FACQ_FILE_ERROR_FAILED: Some error happened in the #FacqFile.
 *
 * Enum containing all the possible error values for #FacqFile.
 */

static void facq_file_initable_iface_init(GInitableIface  *iface);
static gboolean facq_file_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);

G_DEFINE_TYPE_WITH_CODE(FacqFile,facq_file,G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_file_initable_iface_init));

GQuark facq_file_error_quark(void)
{
        return g_quark_from_static_string("facq-file-error-quark");
}

enum {
	PROP_0,
	PROP_FILENAME,
};

struct _FacqFilePrivate {
	GError *construct_error;
	GPollFD *pfd;
	GIOChannel *channel;
	gchar *filename;
	gchar *tmp_filename;
	guint64 written_samples;
	guint8 digest[32];
	GChecksum *sum;
};

/* GObject magic */
static void facq_file_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqFile *file = FACQ_FILE(self);

	switch(property_id){
	case PROP_FILENAME: g_value_set_string(value,file->priv->filename);
	break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(file,property_id,pspec);
	}
}

static void facq_file_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqFile *file = FACQ_FILE(self);

	switch(property_id){
	case PROP_FILENAME: file->priv->filename = g_value_dup_string(value);
	break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(file,property_id,pspec);
	}
}

static void facq_file_finalize(GObject *self)
{
	FacqFile *file = FACQ_FILE(self);
	GError *local_err = NULL;

	g_clear_error(&file->priv->construct_error);

	if(file->priv->channel){
		g_io_channel_shutdown(file->priv->channel,TRUE,&local_err);
		if(local_err){
			facq_log_write(local_err->message,FACQ_LOG_MSG_TYPE_ERROR);
			g_clear_error(&local_err);
		}
		g_io_channel_unref(file->priv->channel);
	}

	if(file->priv->pfd)
		g_free(file->priv->pfd);
	
	if(file->priv->filename)
		g_free(file->priv->filename);

	if(file->priv->tmp_filename)
		g_free(file->priv->tmp_filename);
	
	if(file->priv->sum)
		g_checksum_free(file->priv->sum);

	G_OBJECT_CLASS (facq_file_parent_class)->finalize (self);
}

static void facq_file_constructed(GObject *self)
{
	FacqFile *file = FACQ_FILE(self);

	g_assert(file->priv->filename != NULL);
	
	file->priv->pfd = g_new0(GPollFD,1);
	file->priv->pfd->events = G_IO_OUT | G_IO_ERR;
	file->priv->sum = g_checksum_new(G_CHECKSUM_SHA256);
	g_assert(file->priv->sum);
}

static void facq_file_class_init(FacqFileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqFilePrivate));

	object_class->set_property = facq_file_set_property;
	object_class->get_property = facq_file_get_property;
	object_class->constructed = facq_file_constructed;
	object_class->finalize = facq_file_finalize;

	g_object_class_install_property(object_class,PROP_FILENAME,
					g_param_spec_string("filename",
							     "Filename",
							     "The filename",
							     "Unknown",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));
}

static void facq_file_init(FacqFile *file)
{
	file->priv = G_TYPE_INSTANCE_GET_PRIVATE(file,FACQ_TYPE_FILE,FacqFilePrivate);
	file->priv->construct_error = NULL;
	file->priv->pfd = NULL;
	file->priv->channel = NULL;
	file->priv->filename = NULL;
	file->priv->tmp_filename = NULL;
	file->priv->written_samples = 0;
	memset(file->priv->digest,0,32);
	file->priv->sum = NULL;
}

/* GInitable interface */
static void facq_file_initable_iface_init(GInitableIface *iface)
{
        iface->init = facq_file_initable_init;
}

static gboolean facq_file_initable_init(GInitable *initable,GCancellable *cancellable,GError  **error)
{
        FacqFile *file;
        
        g_return_val_if_fail(FACQ_IS_FILE(initable),FALSE);
        file = FACQ_FILE(initable);
        if(cancellable != NULL){
                g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                           "Cancellable initialization not supported");
                return FALSE;
        }
        if(file->priv->construct_error){
                if(error)
                        *error = g_error_copy(file->priv->construct_error);
                return FALSE;
        }
        return TRUE;
}
/* private write procedures */
static void facq_file_write_magic(GIOChannel *channel,GError **err)
{
	guint32 tmp = 0;
	gsize bytes_written = 0;
	GError *local_error = NULL;

	tmp = MAGIC_NUMBER;
	tmp = GUINT32_TO_BE(tmp);
	g_io_channel_write_chars(channel,(const gchar *)&tmp,sizeof(guint32),&bytes_written,&local_error);
        if(local_error || bytes_written != sizeof(guint32)){
                if(local_error){
                        g_propagate_error(err,local_error);
                        return;
                }
                g_set_error_literal(err,FACQ_FILE_ERROR,
                                FACQ_FILE_ERROR_FAILED,"Error when writting magic");
                return;
        }
}

static void facq_file_write_period(GIOChannel *channel,gdouble period,GError **err)
{
	gdouble tmp = 0;
	gsize bytes_written = 0;
	GError *local_error = NULL;

	tmp = GDOUBLE_TO_BE(period);
	g_io_channel_write_chars(channel,(const gchar *)&tmp,sizeof(gdouble),&bytes_written,&local_error);
        if(local_error || bytes_written != sizeof(gdouble)){
                if(local_error){
                        g_propagate_error(err,local_error);
                        return;
                }
                g_set_error_literal(err,FACQ_FILE_ERROR,
                                FACQ_FILE_ERROR_FAILED,"Error when writting period");
                return;
        }
}

static void facq_file_write_n_channels(GIOChannel *channel,guint32 n_channels,GError **err)
{
	guint32 tmp = 0;
	gsize bytes_written = 0;
	GError *local_error = NULL;

	tmp = GUINT32_TO_BE(n_channels);
	g_io_channel_write_chars(channel,(const gchar *)&tmp,sizeof(guint32),&bytes_written,&local_error);
        if(local_error || bytes_written != sizeof(guint32)){
                if(local_error){
                        g_propagate_error(err,local_error);
                        return;
                }
                g_set_error_literal(err,FACQ_FILE_ERROR,
                                FACQ_FILE_ERROR_FAILED,"Error when writting number of channels");
                return;
        }
}

static void facq_file_write_channels(GIOChannel *channel,const guint *channels,guint32 n_channels,GError **err)
{
	guint32 i = 0, tmp = 0;
	gsize bytes_written = 0;
	GError *local_error = NULL;

	for(i = 0;i < n_channels;i++){
		tmp = GUINT32_TO_BE(channels[i]);
                g_io_channel_write_chars(channel,(const gchar *)&tmp,sizeof(guint32),&bytes_written,&local_error);
                if(local_error || bytes_written != (sizeof(guint32))){
                        if(local_error){
                                g_propagate_error(err,local_error);
				return;
                        }
                        g_set_error_literal(err,FACQ_FILE_ERROR,
                                        FACQ_FILE_ERROR_FAILED,"Error when writting channels");
			return;
                }
	}
}

static void facq_file_write_units(GIOChannel *channel,const FacqUnits *units,guint32 n_channels,GError **err)
{
	GError *local_error = NULL;
	guint32 i = 0, tmp = 0;
	gsize bytes_written = 0;

	for(i = 0;i < n_channels;i++){
                tmp = GUINT32_TO_BE(units[i]);
                g_io_channel_write_chars(channel,(const gchar *)&tmp,sizeof(FacqUnits),&bytes_written,&local_error);
                if(local_error || bytes_written != sizeof(enum chan_unit)){
                        if(local_error){
                                g_propagate_error(err,local_error);
                                return;
                        }
                        g_set_error_literal(err,FACQ_FILE_ERROR,
                                        FACQ_FILE_ERROR_FAILED,"Error when writting units");
                        return;
		}
	}              
}

static void facq_file_write_max(GIOChannel *channel,const gdouble *max,guint32 n_channels,GError **err)
{
	gdouble mtmp = 0;
	GError *local_error = NULL;
	guint32 i = 0;
	gsize bytes_written = 0;

	for(i = 0;i < n_channels;i++){
                mtmp = GDOUBLE_TO_BE(max[i]);
                g_io_channel_write_chars(channel,(const gchar *)&mtmp,sizeof(gdouble),&bytes_written,&local_error);
                if(local_error || bytes_written != sizeof(gdouble)){
                        if(local_error){
                                g_propagate_error(err,local_error);
                                return;
                        }
                        g_set_error_literal(err,FACQ_FILE_ERROR,
                                        FACQ_FILE_ERROR_FAILED,"Error when writting max or min values");
                        return;
                }
        }
}

static void facq_file_write_min(GIOChannel *channel,const gdouble *min,guint32 n_channels,GError **err)
{
	facq_file_write_max(channel,min,n_channels,err);
}

static void facq_file_write_written_samples(GIOChannel *channel,guint64 written_samples,GError **err)
{
	gsize bytes_written = 0;
	GError *local_error = NULL;
	
	written_samples = GUINT64_TO_BE(written_samples);
	g_io_channel_write_chars(channel,(const gchar *)&written_samples,sizeof(guint64),&bytes_written,&local_error);
        if(local_error || bytes_written != (sizeof(guint64))){
                if(local_error && err != NULL){
                        g_propagate_error(err,local_error);
                        return;
                }
                if(err != NULL){
                        g_set_error_literal(err,FACQ_FILE_ERROR,
                                        FACQ_FILE_ERROR_FAILED,"Error when writting n_samples");
                        return;
                }
        }

}

static void facq_file_write_digest(GIOChannel *channel,const guint8 *digest,GError **err)
{
	GError *local_error = NULL;
	gsize bytes_written = 0;
	guint8 *rdigest = NULL;
	guint32 i = 0, j = 0;

        rdigest = g_new0(guint8,32);
	g_assert(rdigest != NULL);

	for(i = 32, j = 0;i > 0;i--,j++){
                rdigest[j] = digest[i-1];
        }

	g_io_channel_write_chars(channel,(const gchar *)rdigest,sizeof(guint8)*32,&bytes_written,&local_error);
        if(local_error || bytes_written != (32*sizeof(guint8))){
                if(local_error && err != NULL){
                        g_propagate_error(err,local_error);
                        g_free(rdigest);
                        return;
                }
                if(err != NULL){
                        g_set_error_literal(err,FACQ_FILE_ERROR,
                                        FACQ_FILE_ERROR_FAILED,"Error when writting digest");
                        g_free(rdigest);
                        return;
                }
        }
	g_free(rdigest);
}

/* private read procedures */
static gboolean facq_file_goto_area(GIOChannel *channel,guint32 n_channels,enum file_area area,GError **err)
{
	GIOStatus status;
	GError *local_error = NULL;
	guint offset = 0;
	GSeekType seek_type = 0;

	/*
	 * |M|P|N| C . C |U . U |M . M |M . M |S . S |W |D |
	 * |A|E|_| H . H |N . N |A . A |I . I |A . A |R |I |
	 * |G|R|C| A . A |I . I |X . X |N . N |M . M |I |G |
	 * |I|I|H| N   N |T   T |0   N |0   N |P   P |T |E |
	 * |C|O|A| N   N |0   N |      |      |L   L |T |S |
	 * | |D|N| E   E |      |      |      |E   E |E |T |
	 * | | |N| L   L |      |      |      |0   N |N |  |
	 * | | |E| 0   N |      |      |      |      |_ |  |
	 * | | |L|       |      |      |      |      |S |  |
	 * | | |S|       |      |      |      |      |A |  |
	 * | | | |       |      |      |      |      |M |  |
	 * | | | |       |      |      |      |      |P |  |
	 * | | | |       |      |      |      |      |L |  |
	 * | | | |       |      |      |      |      |E |  |
	 * | | | |       |      |      |      |      |S |  |
	 * |4|8|4|4xN    |4xN   |8xN   |8xN   |8xW_S |8 |32|
	 */     

	switch(area){
	case START:
		offset = 0;
		seek_type = G_SEEK_SET;
	case FIRST_CHANNEL: 
		// sizeof(magic) + sizeof(n_channels) + sizeof(period)
		offset = 16;
		seek_type = G_SEEK_SET;
	break;
	case FIRST_UNIT:
		
		offset = 16 + (n_channels*sizeof(guint32));
		seek_type = G_SEEK_SET;
	break;
	case FIRST_MAX:
		offset = 16 + (2*n_channels*sizeof(guint32));
		seek_type = G_SEEK_SET;
	break;
	case FIRST_MIN:
		offset = 16 + (4*n_channels*sizeof(guint32));
		seek_type = G_SEEK_SET;
	break;
	case FIRST_SAMPLE: 
		offset = 16 + (6*n_channels*sizeof(guint32));
		seek_type = G_SEEK_SET;
	break;
	case END_OF_FILE:
		offset = 0;
		seek_type = G_SEEK_END;
	default:
		g_assert(TRUE);
	}

	status = g_io_channel_seek_position(channel,offset,seek_type,&local_error);
	if(local_error != NULL || status == G_IO_STATUS_ERROR){
		if(local_error)
			goto error;
		g_set_error_literal(err,FACQ_FILE_ERROR,
				FACQ_FILE_ERROR_FAILED,"Unknown error while seeking position");
		goto error;
	}
	return TRUE;

	error:
	if(local_error && err != NULL)
		g_propagate_error(err,local_error);
	return FALSE;
}

static guint32 facq_file_read_magic(GIOChannel *channel,GError **err)
{
	GError *local_error = NULL;
	guint32 magic = 0;

	g_io_channel_seek_position(channel,0,G_SEEK_SET,&local_error);
	if(local_error)
		goto error;
	
	g_io_channel_read_chars(channel,(gchar *)&magic,sizeof(guint32),NULL,&local_error);
	if(local_error)
		goto error;

	magic = GUINT32_TO_BE(magic);

	return magic;

	error:
	if(local_error && err != NULL)
		g_propagate_error(err,local_error);
	return 0;
}

static gdouble facq_file_read_period(GIOChannel *channel,GError **err)
{
	GError *local_error = NULL;
	gdouble period = 0;

	g_io_channel_seek_position(channel,sizeof(guint32),G_SEEK_SET,&local_error);
	if(local_error)
		goto error;
	
	g_io_channel_read_chars(channel,(gchar *)&period,sizeof(gdouble),NULL,&local_error);
	if(local_error)
		goto error;

	period = GDOUBLE_TO_BE(period);

	return period;

	error:
	if(local_error && err != NULL)
		g_propagate_error(err,local_error);
	return 0;
}

static guint32 facq_file_read_n_channels(GIOChannel *channel,GError **err)
{
	GError *local_error = NULL;
	guint32 n_channels = 0;

	g_io_channel_seek_position(channel,3*sizeof(guint32),G_SEEK_SET,&local_error);
	if(local_error)
		goto error;
	
	g_io_channel_read_chars(channel,(gchar *)&n_channels,sizeof(guint32),NULL,&local_error);
	if(local_error)
		goto error;

	n_channels = GUINT32_TO_BE(n_channels);

	return n_channels;

	error:
	if(local_error && err != NULL)
		g_propagate_error(err,local_error);
	return 0;
}

static guint32 *facq_file_read_channels(GIOChannel *channel,guint32 n_channels,GError **err)
{
	guint32 *channels = g_new0(guint32,n_channels);
	GError *local_error = NULL;
	gsize readed_bytes = 0;
	guint32 i = 0;

	if(!channels){
		if(err != NULL)
			g_set_error_literal(err,FACQ_FILE_ERROR,
				FACQ_FILE_ERROR_FAILED,"Can't read channels, wrong n_channels number");
		return NULL;
	}

	facq_file_goto_area(channel,n_channels,FIRST_CHANNEL,&local_error);
	if(local_error)
		goto error;

	for(i = 0;i < n_channels;i++){
		g_io_channel_read_chars(channel,(gchar *)&channels[i],sizeof(guint32),&readed_bytes,&local_error);
		if(local_error)
			goto error;
		if(readed_bytes != (sizeof(guint32)) ){
			if(err != NULL)
				g_set_error_literal(err,FACQ_FILE_ERROR,
					FACQ_FILE_ERROR_FAILED,"Can't read channels, wrong n_channels number");
			goto error;
		}
		channels[i] = GUINT32_TO_BE(channels[i]);
	}

	return channels;

	error:
	if(channels)
		g_free(channels);
	if(local_error && err != NULL)
		g_propagate_error(err,local_error);
	return NULL;
}

static FacqUnits *facq_file_read_units(GIOChannel *channel,guint32 n_channels,GError **err)
{
	GError *local_error = NULL;
	FacqUnits *units = NULL;
	gsize readed_bytes = 0;
	guint i = 0;

	units = g_new0(enum chan_unit,n_channels);
	if(!units){
		if(err != NULL)
			g_set_error_literal(err,FACQ_FILE_ERROR,
				FACQ_FILE_ERROR_FAILED,"Can't read units, wrong n_channels number");
		return NULL;
	}


	facq_file_goto_area(channel,n_channels,FIRST_UNIT,&local_error);
	if(local_error)
		goto error;

	g_io_channel_read_chars(channel,(gchar *)units,sizeof(enum chan_unit)*n_channels,&readed_bytes,&local_error);
	if(local_error)
		goto error;
	if(readed_bytes != (sizeof(enum chan_unit)*n_channels) ){
		if(err != NULL)
			g_set_error_literal(err,FACQ_FILE_ERROR,
				FACQ_FILE_ERROR_FAILED,"Can't read units, wrong n_channels number");
		goto error;
	}

	for(i = 0; i < n_channels;i++){
		units[i] = GUINT32_TO_BE(units[i]);
	}

	return units;

	error:
	if(units)
		g_free(units);
	if(local_error && err != NULL)
		g_propagate_error(err,local_error);
	return NULL;
}

static gdouble *facq_file_read_max(GIOChannel *channel,guint32 n_channels,GError **err)
{
	GError *local_error = NULL;
	gdouble *max = NULL;
	gsize readed_bytes = 0;
	guint i = 0;

	max = g_new0(gdouble,n_channels);
	if(!max){
		if(err != NULL)
			g_set_error_literal(err,FACQ_FILE_ERROR,FACQ_FILE_ERROR_FAILED,
					"Can't read max, wrong n_channels number");
		return NULL;
	}

	facq_file_goto_area(channel,n_channels,FIRST_MAX,&local_error);
	if(local_error)
		goto error;

	g_io_channel_read_chars(channel,(gchar *)max,sizeof(gdouble)*n_channels,&readed_bytes,&local_error);
	if(local_error)
		goto error;
	if(readed_bytes != (sizeof(gdouble)*n_channels) ){
		if(err != NULL)
			g_set_error_literal(err,FACQ_FILE_ERROR,FACQ_FILE_ERROR_FAILED,
					"Can't read max, wrong n_channels number");
		goto error;
	}

	for(i = 0;i < n_channels;i++){
		max[i] = GDOUBLE_TO_BE(max[i]);
	}

	return max;

	error:
	if(max)
		g_free(max);
	if(local_error && err != NULL)
		g_propagate_error(err,local_error);
	return NULL;
}

static gdouble *facq_file_read_min(GIOChannel *channel,guint32 n_channels,GError **err)
{
	GError *local_error = NULL;
	gdouble *min = NULL;
	gsize readed_bytes = 0;
	guint i = 0;

	min = g_new0(gdouble,n_channels);
	if(!min){
		if(err != NULL)
			g_set_error_literal(err,FACQ_FILE_ERROR,FACQ_FILE_ERROR_FAILED,
					"Can't read min, wrong n_channels number");
		return NULL;
	}

	facq_file_goto_area(channel,n_channels,FIRST_MIN,&local_error);
	if(local_error)
		goto error;

	g_io_channel_read_chars(channel,(gchar *)min,sizeof(gdouble)*n_channels,&readed_bytes,&local_error);
	if(local_error)
		goto error;
	if(readed_bytes != (sizeof(gdouble)*n_channels) ){
		if(err != NULL)
			g_set_error_literal(err,FACQ_FILE_ERROR,FACQ_FILE_ERROR_FAILED,
					"Can't read min, wrong n_channels number");
		goto error;
	}

	for(i = 0;i < n_channels;i++){
		min[i] = GDOUBLE_TO_BE(min[i]);
	}

	return min;

	error:
	if(min)
		g_free(min);
	if(local_error && err != NULL)
		g_propagate_error(err,local_error);
	return NULL;	
}

static guint64 facq_file_read_written_samples(GIOChannel *channel,GError **err)
{
	GError *local_error = NULL;
	guint64 n_samples = 0;
	gsize bytes_readed = 0;
	GIOStatus ret = 0;

	ret = g_io_channel_seek_position(channel,0,G_SEEK_END,&local_error);
	if(local_error || ret != G_IO_STATUS_NORMAL)
		goto error;

	ret = g_io_channel_seek_position(channel,-40,G_SEEK_CUR,&local_error);
	if(local_error || ret != G_IO_STATUS_NORMAL)
		goto error;

	ret = g_io_channel_read_chars(channel,(gchar *)&n_samples,sizeof(guint64),&bytes_readed,&local_error);
	if(local_error || bytes_readed 
		!= sizeof(guint64) || ret != G_IO_STATUS_NORMAL)
			goto error;

	return GUINT64_TO_BE(n_samples);

	error:
	if(local_error && err != NULL)
		g_propagate_error(err,local_error);
	else
		g_set_error(err,FACQ_FILE_ERROR,
			FACQ_FILE_ERROR_FAILED,
				"Error reading written samples");
	return 0;
}

static guint8 *facq_file_read_digest(GIOChannel *channel,GError **err)
{
	GError *local_error = NULL;
	guint8 *rdigest = g_new0(guint8,32);
	guint8 *digest = g_new0(guint8,32);
	guint i = 0, j = 0;

	g_io_channel_seek_position(channel,0,G_SEEK_END,&local_error);
	if(local_error)
		goto error;

	g_io_channel_seek_position(channel,-32,G_SEEK_CUR,&local_error);
	if(local_error)
		goto error;

	g_io_channel_read_chars(channel,(gchar *)digest,sizeof(guint8)*32,NULL,&local_error);
	if(local_error)
		goto error;

	//reverse digest
	for(i = 32, j = 0;i > 0;i--,j++){
		rdigest[j] = digest[i-1];
	}

	g_free(digest);
	return rdigest;

	error:
	if(local_error && err != NULL)
		g_propagate_error(err,local_error);
	if(digest)
		g_free(digest);
	if(rdigest)
		g_free(rdigest);
	return NULL;
}
/* private conversion helpers */
static GIOChannel *facq_file_bin_read_open(const gchar *filename,GError **err)
{
	GIOChannel *channel = NULL;
	GIOStatus status;
	GError *local_error = NULL;

	channel = g_io_channel_new_file(filename,"r",&local_error);
        if(local_error || !channel){
                if(local_error == NULL && err != NULL && channel == NULL)
                        g_set_error_literal(err,FACQ_FILE_ERROR,
                                FACQ_FILE_ERROR_FAILED,"Can't open file");
                goto error;
        }
	status = g_io_channel_set_encoding(channel,NULL,&local_error);
        if(local_error || status != G_IO_STATUS_NORMAL)
                goto error;

        return channel;

	error:
        if(channel)
                g_io_channel_unref(channel);
        if(local_error && err != NULL)
                g_propagate_error(err,local_error);
        return NULL;
}

static GIOChannel *facq_file_txt_write_open(const gchar *filename,GError **err)
{
	GIOChannel *channel = NULL;
	GError *local_error = NULL;

	channel = g_io_channel_new_file(filename,"w",&local_error);
        if(local_error || !channel){
                if(local_error == NULL && err != NULL && channel == NULL)
                        g_set_error_literal(err,FACQ_FILE_ERROR,
                                FACQ_FILE_ERROR_FAILED,"Can't open file");
                goto error;
        }

        return channel;

	error:
        if(channel)
                g_io_channel_unref(channel);
        if(local_error && err != NULL)
                g_propagate_error(err,local_error);
        return NULL;
}

static gboolean facq_file_txt_write_header(GIOChannel *channel,const FacqStreamData *stmd,GError **err)
{
	gchar *first_line = NULL, *channels_line = NULL;
	GError *local_err = NULL;
	gsize bytes_written = 0;
	guint i = 0, *channels = NULL, chan = 0;
	const FacqUnits *units = NULL;
	const FacqChanlist *chanlist = NULL;

	first_line = g_strdup_printf(FIRST_LINE,stmd->period);
	if( g_io_channel_write_chars(channel,
		first_line,-1,&bytes_written,&local_err) 
			!= G_IO_STATUS_NORMAL )
			goto error;
	g_free(first_line);

	chanlist = stmd->chanlist;
	channels = facq_chanlist_to_comedi_chanlist(chanlist,NULL);
	units = stmd->units;

	for(i = 0;i < stmd->n_channels;i++){
		facq_chanlist_chanspec_to_src_values(channels[i],
						     &chan,
						     NULL,
						     NULL,
						     NULL);
		channels_line = g_strdup_printf(SECOND_LINE_ATOM,
					chan,
					facq_units_type_to_human(units[i]));
		if( g_io_channel_write_chars(channel,
			channels_line,-1,&bytes_written,&local_err)
				!= G_IO_STATUS_NORMAL )
					goto error;
		g_free(channels_line);
	}
	g_free(channels);

	channels_line = g_strdup_printf("\n");
	if( g_io_channel_write_chars(channel,
			channels_line,-1,&bytes_written,&local_err)
				!= G_IO_STATUS_NORMAL )
					goto error;
	g_free(channels_line);

	return TRUE;

	error:
	if(channels_line)
		g_free(channels_line);
	if(channels)
		g_free(channels);
	if(first_line)
		g_free(first_line);
	if(local_err){
		g_propagate_error(err,local_err);
		return FALSE;
	}
	else
		g_set_error_literal(err,FACQ_FILE_ERROR,
                                FACQ_FILE_ERROR_FAILED,"Can't write header");
	return FALSE;
}

static gboolean facq_file_txt_write_samples(GIOChannel *src,GIOChannel *dst,guint32 n_channels,guint64 written_samples,GError **err)
{
	GError *local_err = NULL;
	guint32 j = 0;
	guint64 i = 0;
	gdouble sample = 0;
	gchar *text = NULL;
	gsize bytes = 0;

	facq_file_goto_area(src,n_channels,FIRST_SAMPLE,&local_err);
	if(local_err)
		goto error;

	for(i = 0;i < written_samples;){
		for(j = 0;j < n_channels;j++){
			if( g_io_channel_read_chars(src,(gchar *)&sample,
				sizeof(gdouble),&bytes,&local_err)
					!= G_IO_STATUS_NORMAL)
						goto error;
			sample = GDOUBLE_TO_BE(sample);
			text = g_strdup_printf("%.6g    ",sample);
			if( g_io_channel_write_chars(dst,text,-1,
				&bytes,&local_err)
					!= G_IO_STATUS_NORMAL )
						goto error;
			g_free(text);
			if(j == (n_channels-1)){
				text = g_strdup_printf("\n");
				if( g_io_channel_write_chars(dst,text,-1,
					&bytes,&local_err)
						!= G_IO_STATUS_NORMAL )
							goto error;
				g_free(text);
			}
			i++;
		}
	}

	return TRUE;

	error:
	if(text)
		g_free(text);
	if(local_err)
		g_propagate_error(err,local_err);
	return FALSE;
}

/* public procedures */
/**
 * facq_file_new:
 * @filename: The desired filename, can be an absolute or relative path, plus
 * the name of the file.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqFile object, this will translate later in a file in the
 * filesystem with @filename as is filename.
 *
 * Returns: A new #FacqFile or %NULL in case of error.
 */
FacqFile *facq_file_new(const gchar *filename,GError **err)
{
	return FACQ_FILE(g_initable_new(FACQ_TYPE_FILE,
					NULL,err,
					"filename",filename,
					NULL)
			);
}

/**
 * facq_file_reset:
 * @file: a #FacqFile object.
 * @err: a #GError, if not %NULL it will be set in case of error.
 * 
 * Resets the state of a #FacqFile object, restoring it to it's initial state.
 * 
 * Internally the checksum, the number of samples, the temporal filename and
 * the #GIOChannel of the temporal file are reset.
 */
void facq_file_reset(FacqFile *file,GError **err)
{
	gint fd = -1;
	GError *local_err = NULL;

	file->priv->written_samples = 0;
	g_checksum_reset(file->priv->sum);
	memset(file->priv->digest,0,32);
	if(file->priv->tmp_filename){
		g_free(file->priv->tmp_filename);
		file->priv->tmp_filename = NULL;
	}
	if(file->priv->channel)
		g_io_channel_unref(file->priv->channel);

	file->priv->tmp_filename = g_strdup_printf("%s.XXXXXX",file->priv->filename);
	fd = g_mkstemp(file->priv->tmp_filename);
	if(fd < 0){
		g_set_error_literal(&local_err,FACQ_FILE_ERROR,
					FACQ_FILE_ERROR_FAILED,"Error creating temporal file");
		goto error;
	}

#ifdef G_OS_UNIX
	g_unix_set_fd_nonblocking(fd,TRUE,&local_err);
	if(local_err)
		goto error;

	file->priv->pfd->fd = fd;

	file->priv->channel = g_io_channel_unix_new(fd);
#endif

#ifdef G_OS_WIN32
	file->priv->channel = g_io_channel_win32_new_fd(fd);

	g_io_channel_win32_make_pollfd(file->priv->channel,
					G_IO_OUT,
					file->priv->pfd);
#endif

	g_io_channel_set_encoding(file->priv->channel,NULL,&local_err);
	if(local_err)
		goto error;
	return;

	error:
	if(local_err){
		g_propagate_error(err,local_err);
	}
	return;
}

/**
 * facq_file_write_header:
 * @file: A #FacqFile object, in initial state.
 * @stmd: A #FacqStreamData object.
 * @err: A #GError, if not %NULL it will be set in case of error.
 *
 * Writes the header information to the file @file, updating the checksum in the
 * process (magic and the rest of the fields are written in big endian, so they
 * are passed in this form trough the checksum). 
 * See <link linkend="facqfile-header">Header information</link> for details on the
 * header fields.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_file_write_header(FacqFile *file,const FacqStreamData *stmd,GError **err)
{
	GError *local_err = NULL;
	guint *channels = NULL;
	GIOChannel *channel = NULL;
	guint32 magic = MAGIC_NUMBER;
	
	g_return_val_if_fail(FACQ_IS_FILE(file),FALSE);
	channel = file->priv->channel;

	facq_file_write_magic(channel,&local_err);
	if(local_err)
		goto error;
	facq_file_write_period(channel,stmd->period,&local_err);
	if(local_err)
		goto error;
	facq_file_write_n_channels(channel,stmd->n_channels,&local_err);
	if(local_err)
		goto error;
	channels = facq_chanlist_to_comedi_chanlist(stmd->chanlist,NULL);
	facq_file_write_channels(channel,channels,stmd->n_channels,&local_err);
	g_free(channels);
	if(local_err)
		goto error;
	facq_file_write_units(channel,stmd->units,stmd->n_channels,&local_err);
	if(local_err)
		goto error;
	facq_file_write_max(channel,stmd->max,stmd->n_channels,&local_err);
	if(local_err)
		goto error;
	facq_file_write_min(channel,stmd->min,stmd->n_channels,&local_err);
	if(local_err)
		goto error;

	g_io_channel_flush(channel,&local_err);
        if(local_err)
		goto error;

	magic = GUINT32_TO_BE(magic);
	g_checksum_update(file->priv->sum,
			(guchar *)&magic,sizeof(guint32));
	facq_stream_data_to_checksum(stmd,file->priv->sum);
	
	return TRUE;

	error:
	if(local_err){
		g_propagate_error(err,local_err);
	}
	return FALSE;
}

/**
 * facq_file_poll:
 * @file: A #FacqFile object.
 *
 * Polls the file descriptor of the temporal file to check if it can be written
 * without blocking or errors.
 *
 * Returns: 1 if the file can be written, -1 in other case.
 */
gint facq_file_poll(FacqFile *file)
{
	gint ret = -1;
	
	g_return_val_if_fail(FACQ_IS_FILE(file),-1);
	if(!file->priv->pfd)
		return -1;

	ret = g_poll(file->priv->pfd,1,500);
	if(ret){
		if(file->priv->pfd->revents & G_IO_OUT)
			return 1;
		if(file->priv->pfd->revents & G_IO_ERR)
			return -1;
	}
	return ret;
}

/**
 * facq_file_write_samples:
 * @file: A #FacqFile object.
 * @chunk: A #FacqChunk containing the samples to write.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * Writes the samples contained in the #FacqChunk, @chunk, to the #FacqFile @file,
 * in big endian format, updating the checksum and increasing the internal
 * counter of written samples.
 *
 * Returns: A #GIOStatus value as returned by g_io_channel_write_chars().
 */
GIOStatus facq_file_write_samples(FacqFile *file,FacqChunk *chunk,GError **err)
{
	GIOStatus ret = 0;
	GError *local_err = NULL;
	gsize bytes_written = 0;
	gsize used_bytes = 0;

#if ENABLE_DEBUG
	g_return_val_if_fail(FACQ_IS_FILE(file),G_IO_STATUS_ERROR);
	g_return_val_if_fail(FACQ_IS_CHUNK(chunk),G_IO_STATUS_ERROR);
#endif

	used_bytes = facq_chunk_get_used_bytes(chunk);
	facq_chunk_data_double_to_be(chunk);
	g_checksum_update(file->priv->sum,
		(guchar *)chunk->data,used_bytes);

	ret = g_io_channel_write_chars(file->priv->channel,
				       chunk->data,
				       used_bytes,
				       &bytes_written,
				       &local_err);
	if(local_err){
		g_propagate_error(err,local_err);
		return ret;
	}
	if(bytes_written != used_bytes){
		g_set_error_literal(&local_err,FACQ_FILE_ERROR,
					       FACQ_FILE_ERROR_FAILED,"Can't write all bytes to file");
	}
	file->priv->written_samples += (bytes_written/sizeof(gdouble));

	return ret;
}

/**
 * facq_file_write_tail:
 * @file: A #FacqFile object.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * Writes the tail information to the file (The footer). To see what information
 * is contained in the tail see <link linkend="facqfile-tail">Tail information</link>.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_file_write_tail(FacqFile *file,GError **err)
{
	GError *local_err = NULL;
	GIOChannel *channel = NULL;
	guint64 written_samples = 0;
	guint8 *digest = NULL;
	gsize digestlen = 32;

	g_return_val_if_fail(FACQ_IS_FILE(file),FALSE);
	channel = file->priv->channel;
	written_samples = file->priv->written_samples;
	digest = file->priv->digest;

	written_samples = GUINT64_TO_BE(written_samples);
	g_checksum_update(file->priv->sum,
			  (guchar *)&written_samples,
			  sizeof(guint64));
	written_samples = GUINT64_TO_BE(written_samples);

	g_checksum_get_digest(file->priv->sum,
			      digest,
			      &digestlen);

	facq_file_write_written_samples(channel,written_samples,&local_err);
	if(local_err)
		goto error;
	facq_file_write_digest(channel,digest,&local_err);
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
 * facq_file_stop:
 * @file: A #FacqFile object.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * This function shutdowns the #GIOChannel for the temporal file, and renames
 * the temporal file, to the filename requested by the user.
 *
 * You won't be able to write to the file after calling this function, until you
 * call facq_file_reset().
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */ 
gboolean facq_file_stop(FacqFile *file,GError **err)
{
	GError *local_err = NULL;

	g_return_val_if_fail(FACQ_IS_FILE(file),FALSE);

	g_io_channel_shutdown(file->priv->channel,TRUE,&local_err);
	if(local_err)
		goto error;

	g_io_channel_unref(file->priv->channel);
	file->priv->channel = NULL;

	close(file->priv->pfd->fd);

	/* Try to remove the destination file, else g_rename will fail to rename
	 * the temporal file to the destination if the destination already
	 * exists. This is caused because g_rename in windows is a wrapper to MoveFileExW
	 * and this function will fail if the destination exists, so probably
	 * the glib g_remove function should do this on it's own.*/

	if( g_remove(file->priv->filename) != 0){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,"Can't remove the destination file");
	}

	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,
			"Renaming %s to %s",file->priv->tmp_filename,file->priv->filename);
	if( g_rename(file->priv->tmp_filename,file->priv->filename) != 0){
		g_set_error_literal(&local_err,FACQ_FILE_ERROR,
				FACQ_FILE_ERROR_FAILED,
					"Error renaming temporal file");
		goto error;
	}
	return TRUE;

	error:
	if(local_err)
		g_propagate_error(err,local_err);
	return FALSE;
}

/**
 * facq_file_open:
 * @filename: The filename of the #FacqFile.
 * @err: (allow-none): A #GError it will be set in case of error if not %NULL.
 *
 * Opens for reading a previously created #FacqFile stored on disk with filename, @filename,
 * checking that the magic number is fine.
 *
 * Returns: A new created #FacqFile object if successful or %NULL in other case.
 */
FacqFile *facq_file_open(const gchar *filename,GError **err)
{
	FacqFile *file = NULL;
	GError *local_err = NULL;

	file = facq_file_new(filename,&local_err);
	if(local_err)
		goto error;

	file->priv->channel = facq_file_bin_read_open(filename,&local_err);
	if(local_err)
		goto error;

	facq_file_check_magic(file,&local_err);
	if(local_err)
		goto error;

	return file;

	error:
	if(file)
		facq_file_free(file);
	if(local_err)
		g_propagate_error(err,local_err);
	return NULL;
}

/**
 * facq_file_read_header:
 * @file: A previously opened #FacqFile.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Reads the header from a previously opened #FacqFile, creating a new
 * #FacqStreamData object with the read data.
 *
 * Returns: A new #FacqStreamData object, or %NULL in case of error.
 */
FacqStreamData *facq_file_read_header(FacqFile *file,GError **err)
{
	FacqStreamData *stmd = NULL;
	GError *local_err = NULL;
	guint bps = sizeof(gdouble), n_channels = 0, i = 0;
	FacqUnits *units = NULL;
	FacqChanlist *chanlist = NULL;
	guint *channels = NULL;
	gdouble period = 0, *max = NULL, *min = NULL;
	GIOChannel *channel = NULL;

	g_return_val_if_fail(FACQ_IS_FILE(file),NULL);
	channel = file->priv->channel;

	n_channels = facq_file_read_n_channels(channel,&local_err);
	if(local_err)
		goto error;

	period = facq_file_read_period(channel,&local_err);
	if(local_err)
		goto error;

	channels = facq_file_read_channels(channel,n_channels,&local_err);
	if(local_err)
		goto error;
	
	chanlist = facq_chanlist_new();
	for(i = 0;i < n_channels;i++)
		facq_chanlist_add_chan(chanlist,channels[i],
					0,0,0,CHAN_INPUT);

	units = facq_file_read_units(channel,n_channels,&local_err);
	if(local_err)
		goto error;
	max = facq_file_read_max(channel,n_channels,&local_err);
	if(local_err)
		goto error;
	min = facq_file_read_min(channel,n_channels,&local_err);
	if(local_err)
		goto error;

	stmd = facq_stream_data_new(bps,n_channels,period,chanlist,units,max,min);
	return stmd;

	error:
	if(local_err)
		g_propagate_error(err,local_err);
	return NULL;
}

/**
 * facq_file_read_tail:
 * @file: A #FacqFile object.
 * @written_samples: (out caller-allocates):
 * @err: (allow-none):A #GError, it will be set in case of error if not %NULL.
 *
 * Reads the number of samples and the digest from a previously opened
 * #FacqFile.
 *
 * Returns: A 32 bytes array with the digest that you must free with g_free()
 * or %NULL in case of error.
 */
guint8 *facq_file_read_tail(FacqFile *file,guint64 *written_samples,GError **err)
{
	GError *local_err = NULL;
	guint8 *digest = NULL;
	GIOChannel *channel = NULL;

	g_return_val_if_fail(FACQ_IS_FILE(file),NULL);
	channel = file->priv->channel;

	*written_samples = facq_file_read_written_samples(channel,&local_err);
	if(local_err)
		goto error;
	
	digest = facq_file_read_digest(channel,&local_err);
	if(local_err)
		goto error;

	return digest;

	error:
	if(local_err)
		g_propagate_error(err,local_err);
	return NULL;
}

/**
 * facq_file_check_magic:
 * @file: A #FacqFile object.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * Checks if the first 4 bytes, of the #FacqFile @file, equals to the magic
 * number in big endian.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_file_check_magic(FacqFile *file,GError **err)
{
	guint32 magic = 0;
	GError *local_err = NULL;
	GIOChannel *channel = NULL;

	g_return_val_if_fail(FACQ_IS_FILE(file),FALSE);
	channel = file->priv->channel;

	magic = facq_file_read_magic(channel,&local_err);
	if(local_err){
		g_propagate_error(err,local_err);
		return FALSE;
	}
	if(magic != MAGIC_NUMBER){
		g_set_error_literal(err,
			FACQ_FILE_ERROR,FACQ_FILE_ERROR_FAILED,
				"Wrong magic");
		return FALSE;
	}
	return TRUE;
}

/**
 * facq_file_to_human:
 * @binfilename: The filename of the previously created #FacqFile.
 * @txtfilename: The filename of the plain text file that is going to be
 * created.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * This function handles all the dirty details of converting a #FacqFile binary
 * file stored on disk, pointed by @binfilename, to plain text format in a new
 * file pointed by @txtfilename.
 *
 * Returns: %TRUE if successful, %FALSE in case of error.
 */
gboolean facq_file_to_human(const gchar *binfilename,const gchar *txtfilename,GError **err)
{
	GIOChannel *dst = NULL;
	GError *local_err = NULL;
	FacqStreamData *stmd = NULL;
	guint64 written_samples = 0;
	guint8 *digest = NULL;
	FacqFile *srcfile = NULL;

	srcfile = facq_file_open(binfilename,&local_err);
	if(local_err)
		goto error;

	dst = facq_file_txt_write_open(txtfilename,&local_err);
	if(local_err)
		goto error;

	stmd = facq_file_read_header(srcfile,&local_err);
	if(local_err)
		goto error;

	digest = facq_file_read_tail(srcfile,&written_samples,&local_err);
	if(local_err)
		goto error;
	
	facq_file_txt_write_header(dst,stmd,&local_err);
	if(local_err)
		goto error;

	facq_file_txt_write_samples(srcfile->priv->channel,
				    dst,stmd->n_channels,written_samples,
				    &local_err);
	if(local_err)
		goto error;

	facq_file_free(srcfile);

	g_io_channel_shutdown(dst,TRUE,&local_err);
	if(local_err)
		goto error;
	
	g_io_channel_unref(dst);

	g_free(digest);
	facq_stream_data_free(stmd);
	return TRUE;

	error:
	if(srcfile)
		facq_file_free(srcfile);
	if(digest)
		g_free(digest);
	if(stmd)
		facq_stream_data_free(stmd);
	if(local_err)
		g_propagate_error(err,local_err);
	return FALSE;
}

/**
 * facq_file_verify:
 * @filename: The filename of the file.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * This function verifies that the file pointed by @filename is a valid
 * #FacqFile. For verifying the functions checks first the magic number.
 * After this step, the header and the tail of the file are read, if successful
 * the function does a third step where samples in the file are read and
 * counted, the total number of samples read should equal the number of samples
 * in the tail, and should be a multiple of n_channels in the header.
 * Finally the checksum should equal the checksum in the tail.
 *
 * Returns: %TRUE if the file can be verified, %FALSE in other case.
 */
gboolean facq_file_verify(const gchar *filename,GError **err)
{
	FacqFile *file = NULL;
	GIOStatus rret = 0;
	guint32 magic = MAGIC_NUMBER;
	guint64 total_samples = 0, written_samples = 0, i = 0;
	guint8 *tail_digest = NULL, digest[32];
	GChecksum *sum = NULL;
	FacqStreamData *stmd_header = NULL;
	GError *local_err = NULL;
	gdouble sample = 0;
	gsize bytes = 0;

	file = facq_file_open(filename,&local_err);
	if(local_err){
		g_propagate_error(err,local_err);
		return FALSE;
	}
	
	stmd_header = facq_file_read_header(file,&local_err);
	if(local_err)
		goto error;
	
	tail_digest = facq_file_read_tail(file,&written_samples,&local_err);
	if(local_err)
		goto error;

	facq_file_goto_area(file->priv->channel,stmd_header->n_channels,FIRST_SAMPLE,&local_err);
	if(local_err)
		goto error;

	/* Create a GChecksum, and pass to it the header in big endian */
	sum = g_checksum_new(G_CHECKSUM_SHA256);

	magic = GUINT32_TO_BE(magic);
	g_checksum_update(sum,(guchar *)&magic,sizeof(guint32));
	facq_stream_data_to_checksum(stmd_header,sum);

	/* for each sample read it from file (already in big endian) and pass
	 * the sample to the checksum, updating total_samples in each sample*/
	for(i = 0;i < written_samples;i++){
		rret = g_io_channel_read_chars(file->priv->channel,
						(gchar *)&sample,
							sizeof(gdouble),
								&bytes,
									&local_err);
		if(local_err)
			goto error;
		if(bytes != sizeof(gdouble))
			goto error;
		if(rret != G_IO_STATUS_NORMAL)
			goto error;
		g_checksum_update(sum,(guchar *)&sample,sizeof(gdouble));
		total_samples++;
	}

	/* check that total_samples == written_samples */
	if(total_samples != written_samples){
		g_set_error_literal(&local_err,FACQ_FILE_ERROR,
					      FACQ_FILE_ERROR_FAILED,"Number of samples differs");
		goto error;
	}

	/* Pass the total_samples in big endian to the checksum */
	total_samples = GUINT64_TO_BE(total_samples);
	g_checksum_update(sum,(guchar *)&total_samples,sizeof(guint64));

	/* Get the digest */
	bytes = 32;
	g_checksum_get_digest(sum,digest,&bytes);

	/* Compare the obtained digest with the digest in tail_digest */
	for(i = 0;i < 32;i++){
		if(tail_digest[i] != digest[i]){
			g_set_error_literal(&local_err,FACQ_FILE_ERROR,
						FACQ_FILE_ERROR_FAILED,"digest differs");
			goto error;
		}
	}

	facq_file_free(file);
	g_checksum_free(sum);
	facq_stream_data_free(stmd_header);
	g_free(tail_digest);

	return TRUE;

	error:
	if(file)
		facq_file_free(file);
	if(sum)
		g_checksum_free(sum);
	if(tail_digest)
		g_free(tail_digest);
	if(stmd_header)
		facq_stream_data_free(stmd_header);
	if(local_err)
		g_propagate_error(err,local_err);
	return FALSE;
}

/**
 * facq_file_get_filename:
 * @file: A #FacqFile Object.
 *
 * Returns the filename of the specified #FacqFile, @file.
 *
 * Returns: The filename, free it with g_free().
 */
gchar *facq_file_get_filename(FacqFile *file)
{
	g_return_val_if_fail(FACQ_IS_FILE(file),NULL);
	return g_strdup(file->priv->filename);
}

/**
 * facq_file_chunk_iterator:
 * @file: A #FacqFile Object, created with facq_file_open().
 * @start: The number of the first chunk, starting at 0, and up to
 * (written_samples/n_channels)-1.
 * @chunks: The number of chunks to process, from 1 to written_samples/n_channels.
 * If @chunks is greater than the number of chunks in the file the value will be auto
 * adjusted and a warning will be printed to the #FacqLog system.
 * @itercb: A function of type #FacqFileIterCb, it will be called for each chunk
 * between start and end.
 * @data: A pointer to some data that can be passed to @itercb function.
 * @err: A #GError it will be set in case of error if not %NULL.
 *
 * Reads all the chunks in the file from the start position to the end and calls
 * the function @itercb for each chunk, passing the @data parameter to it.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_file_chunk_iterator(FacqFile *file,guint64 start,guint64 chunks,FacqFileIterCb itercb,gpointer data,GError **err)
{
	FacqStreamData *stmd = NULL;
	gdouble *chunk = NULL;
	GError *local_err = NULL;
	guint64 n_chunks = 0, written_samples = 0, i = 0;
	guint n_channels = 0, j = 0;
	guint8 *digest = NULL;
	GIOStatus rret = 0;
	gsize bytes = 0;

	g_return_val_if_fail(FACQ_IS_FILE(file),FALSE);

	if(chunks == 0 || itercb == NULL){
		g_set_error_literal(&local_err,FACQ_FILE_ERROR,
					       FACQ_FILE_ERROR_FAILED,"Iterator: Invalid input parameters");
		goto error;
	}

	stmd = facq_file_read_header(file,&local_err);
	if(local_err)
		goto error;

	n_channels = stmd->n_channels;
	facq_stream_data_free(stmd);

	digest = facq_file_read_tail(file,&written_samples,&local_err);
	if(digest)
		g_free(digest);
	if(local_err)
		goto error;
	
	n_chunks = written_samples/n_channels;

	facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
			"File has a total of %"G_GUINT64_FORMAT" chunks",n_chunks);

	if(start >= n_chunks){
		g_set_error_literal(&local_err,FACQ_FILE_ERROR,
					       FACQ_FILE_ERROR_FAILED,"Iterator: start out of range");
		goto error;
	}
	if(chunks > n_chunks){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_WARNING,
				"Iterator: chunks %"
				G_GUINT64_FORMAT
				" is out of range correcting value to maximum %"
				G_GUINT64_FORMAT,
				chunks,n_chunks);
		chunks = n_chunks;
	}

	facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
			"Iterating from %"
			G_GUINT64_FORMAT
			" chunk up to %"
			G_GUINT64_FORMAT
			" of %"
			G_GUINT64_FORMAT
			" chunks",
			start,chunks,n_chunks);

	/* go to first sample in the file */
	facq_file_goto_area(file->priv->channel,n_channels,FIRST_SAMPLE,&local_err);
	if(local_err)
		goto error;

	/* go to first byte of start chunk */
	if( g_io_channel_seek_position(file->priv->channel,
				start*n_channels*sizeof(gdouble),
					G_SEEK_CUR,&local_err) != G_IO_STATUS_NORMAL ){
		goto error;
	}
	chunk = g_malloc0(sizeof(gdouble)*n_channels);

	for(i = start;i < chunks && i < n_chunks;i++){
		rret = g_io_channel_read_chars(file->priv->channel,
						(gchar *)chunk,
							sizeof(gdouble)*n_channels,
								&bytes,
									&local_err);
		if(local_err)
			goto error;
		if(bytes != (sizeof(gdouble)*n_channels) )
			goto error;
		if(rret != G_IO_STATUS_NORMAL)
			goto error;
		for(j = 0;j < n_channels;j++)
			chunk[j] = GDOUBLE_TO_BE(chunk[j]);
		itercb(data,chunk);
	}

	g_free(chunk);
	return TRUE;

	error:
	if(local_err){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
		g_clear_error(&local_err);
	}
	if(err != NULL)
		g_set_error(err,FACQ_FILE_ERROR,
				FACQ_FILE_ERROR_FAILED,"Error processing data from file");
	if(chunk)
		g_free(chunk);
	if(stmd)
		facq_stream_data_free(stmd);
	return FALSE;
}

/**
 * facq_file_free:
 * @file: A #FacqFile object.
 *
 * Destroys the #FacqFile object, @file, freeing it's resources.
 */
void facq_file_free(FacqFile *file)
{
	g_return_if_fail(FACQ_IS_FILE(file));
	g_object_unref(G_OBJECT(file));
}

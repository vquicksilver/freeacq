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
#ifndef _FREEACQ_FILE_H
#define _FREEACQ_FILE_H

G_BEGIN_DECLS

#define MAGIC_NUMBER 123581321
#define FACQ_FILE_ERROR facq_file_error_quark()

#define FACQ_TYPE_FILE (facq_file_get_type ())
#define FACQ_FILE(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_FILE, FacqFile))
#define FACQ_FILE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_FILE, FacqFileClass))
#define FACQ_IS_FILE(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_FILE))
#define FACQ_IS_FILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_FILE))
#define FACQ_FILE_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_FILE, FacqFileClass))

typedef struct _FacqFile FacqFile;
typedef struct _FacqFileClass FacqFileClass;
typedef struct _FacqFilePrivate FacqFilePrivate;
typedef void(*FacqFileIterCb)(gpointer,gdouble *);

typedef enum {
        FACQ_FILE_ERROR_FAILED
} FacqFileError;

enum file_area {
        START, FIRST_CHANNEL, FIRST_UNIT, FIRST_MAX, FIRST_MIN, FIRST_SAMPLE, END_OF_FILE
};

struct _FacqFile {
	/*< private >*/
        GObject parent_instance;
        FacqFilePrivate *priv;
};

struct _FacqFileClass {
	/*< private >*/
        GObjectClass parent_class;
};

GType facq_file_get_type(void) G_GNUC_CONST;

FacqFile *facq_file_new(const gchar *filename,GError **err);
void facq_file_reset(FacqFile *file,GError **err);
gboolean facq_file_write_header(FacqFile *file,const FacqStreamData *stmd,GError **err);
gint facq_file_poll(FacqFile *file);
GIOStatus facq_file_write_samples(FacqFile *file,FacqChunk *chunk,GError **err);
gboolean facq_file_write_tail(FacqFile *file,GError **err);
gboolean facq_file_stop(FacqFile *file,GError **err);

FacqFile *facq_file_open(const gchar *filename,GError **err);
FacqStreamData *facq_file_read_header(FacqFile *file,GError **err);
guint8 *facq_file_read_tail(FacqFile *file,guint64 *written_samples,GError **err);
gboolean facq_file_check_magic(FacqFile *file,GError **err);
gboolean facq_file_to_human(const gchar *binfilename,const gchar *txtfilename,GError **err);
gboolean facq_file_verify(const gchar *filename,GError **err);
gchar *facq_file_get_filename(FacqFile *file);
gboolean facq_file_chunk_iterator(FacqFile *file,guint64 start,guint64 chunks,FacqFileIterCb itercb,gpointer data,GError **err);
void facq_file_free(FacqFile *file);

G_END_DECLS

#endif

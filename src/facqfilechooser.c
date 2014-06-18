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
#include <gtk/gtk.h>
#include "facqi18n.h"
#include "facqlog.h"
#include "facqfilechooser.h"

/**
 * SECTION:facqfilechooser
 * @include:facqfilechooser.h
 * @title:FacqFileChooser
 * @short_description: Provides a gtk based dialog for choosing files.
 *
 * #FacqFileChooser provides a gtk based dialog for choosing files.
 *
 * You can create a new #FacqFileChooser dialog with facq_file_chooser_new(),
 * you can display the dialog to the user with facq_file_chooser_run_dialog(),
 * you can get the user input with facq_file_chooser_get_filename_for_system()
 * or facq_file_chooser_get_filename_for_display(). Finally to destroy the
 * object use facq_file_chooser_free().
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * #FacqFileChooser uses internally the following objects: #GtkFileChooser.
 * </para>
 * </sect1>
 */

/**
 * FacqFileChooser:
 *
 * Contains all the internal details of the #FacqFileChooser objects.
 */

/**
 * FacqFileChooserClass:
 *
 * Class for all the #FacqFileChooser objects.
 */

/**
 * FacqFileChooserDialogType:
 * @FACQ_FILE_CHOOSER_DIALOG_TYPE_SAVE: You want a dialog for saving a file.
 * @FACQ_FILE_CHOOSER_DIALOG_TYPE_LOAD: You want a dialog for loading a file.
 *
 * Enum for different kinds of dialog types that can be showed to the user.
 */
G_DEFINE_TYPE(FacqFileChooser,facq_file_chooser,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_TOPWINDOW,
	PROP_TYPE,
	PROP_EXTENSION,
	PROP_DESCRIPTION
};

struct _FacqFileChooserPrivate {
	GtkWidget *topwindow;
	FacqFileChooserDialogType type;
	gchar *extension;
	gchar *description;
	GtkWidget *dialog;
	gchar *filename;
};

/*****--- GObject magic ---*****/
void facq_file_chooser_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqFileChooser *chooser = FACQ_FILE_CHOOSER(self);

	switch(property_id){
	case PROP_TOPWINDOW: g_value_set_pointer(value,chooser->priv->topwindow);
	break;
	case PROP_TYPE: g_value_set_uint(value,chooser->priv->type);
	break;
	case PROP_EXTENSION: g_value_set_string(value,chooser->priv->extension);
	break;
	case PROP_DESCRIPTION: g_value_set_string(value,chooser->priv->description);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(chooser,property_id,pspec);
	}
}

void facq_file_chooser_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqFileChooser *chooser = FACQ_FILE_CHOOSER(self);

	switch(property_id){
	case PROP_TOPWINDOW: chooser->priv->topwindow = g_value_get_pointer(value);
	break;
	case PROP_TYPE: chooser->priv->type = g_value_get_uint(value);
	break;
	case PROP_EXTENSION: chooser->priv->extension = g_value_dup_string(value);
	break;
	case PROP_DESCRIPTION: chooser->priv->description = g_value_dup_string(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(chooser,property_id,pspec);
	}
}

void facq_file_chooser_constructed(GObject *self)
{
	FacqFileChooser *chooser = FACQ_FILE_CHOOSER(self);
	GtkFileFilter *file_filter = NULL;
	gchar *pattern = NULL;
	gchar *default_filename = NULL;

	if(chooser->priv->type == FACQ_FILE_CHOOSER_DIALOG_TYPE_SAVE){
		chooser->priv->dialog =
#if GTK_MAJOR_VERSION > 2
			gtk_file_chooser_dialog_new("Select or create a file",
						    GTK_WINDOW(chooser->priv->topwindow),
						    GTK_FILE_CHOOSER_ACTION_SAVE,
						    _("_Cancel"),GTK_RESPONSE_CANCEL,
						    _("_OK"),GTK_RESPONSE_ACCEPT,
						    NULL);
#else
			gtk_file_chooser_dialog_new("Select or create a file",
						    GTK_WINDOW(chooser->priv->topwindow),
						    GTK_FILE_CHOOSER_ACTION_SAVE,
						    GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
						    GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT,
						    NULL);
#endif
		
		//set overwrite confirmation
		gtk_file_chooser_set_do_overwrite_confirmation(
				GTK_FILE_CHOOSER(chooser->priv->dialog),TRUE);
		
		//Let the user create new folders if desired
		gtk_file_chooser_set_create_folders(
				GTK_FILE_CHOOSER(chooser->priv->dialog),TRUE);
		
		//Suggest a default filename
		default_filename = g_strdup_printf("Untitled %s.%s",
						   chooser->priv->description,
						   chooser->priv->extension);
							
		gtk_file_chooser_set_current_name(
				GTK_FILE_CHOOSER(chooser->priv->dialog),
						default_filename);
		g_free(default_filename);

	} else {

		file_filter = gtk_file_filter_new();
		
		if(chooser->priv->description)
		gtk_file_filter_set_name(file_filter,
				         chooser->priv->description);
		if(chooser->priv->extension){
			pattern = g_strdup_printf("*.%s",chooser->priv->extension);
			gtk_file_filter_add_pattern(file_filter,pattern);
			g_free(pattern);
		}

		chooser->priv->dialog = 
#if GTK_MAJOR_VERSION > 2
			gtk_file_chooser_dialog_new("Select a file",
						    GTK_WINDOW(chooser->priv->topwindow),
						    GTK_FILE_CHOOSER_ACTION_OPEN,
						    _("_Cancel"), GTK_RESPONSE_CANCEL,
						    _("_Open"), GTK_RESPONSE_ACCEPT,
						    NULL);
#else
			gtk_file_chooser_dialog_new("Select a file",
						    GTK_WINDOW(chooser->priv->topwindow),
						    GTK_FILE_CHOOSER_ACTION_OPEN,
						    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
						    NULL);
#endif


		//Don't let the user create new folders in an open dialog
		gtk_file_chooser_set_create_folders(
				GTK_FILE_CHOOSER(chooser->priv->dialog),FALSE);
		//Add filter
		gtk_file_chooser_add_filter(
				GTK_FILE_CHOOSER(chooser->priv->dialog),file_filter);
	}

	//for the moment only local files are supported
	gtk_file_chooser_set_local_only(
			GTK_FILE_CHOOSER(chooser->priv->dialog),TRUE);
	//Don't show hidden files
	gtk_file_chooser_set_show_hidden(
			GTK_FILE_CHOOSER(chooser->priv->dialog),FALSE);
	gtk_widget_show_all(chooser->priv->dialog);
}

void facq_file_chooser_finalize(GObject *self)
{
	FacqFileChooser *chooser = FACQ_FILE_CHOOSER(self);

	if(chooser->priv->filename)
		g_free(chooser->priv->filename);

	if(chooser->priv->extension)
		g_free(chooser->priv->extension);

	if(chooser->priv->description)
		g_free(chooser->priv->description);

	if(GTK_IS_WIDGET(chooser->priv->dialog))
		gtk_widget_destroy(chooser->priv->dialog);

	if (G_OBJECT_CLASS (facq_file_chooser_parent_class)->finalize)
    		(*G_OBJECT_CLASS (facq_file_chooser_parent_class)->finalize) (self);
}

void facq_file_chooser_class_init(FacqFileChooserClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqFileChooserPrivate));

	object_class->get_property = facq_file_chooser_get_property;
	object_class->set_property = facq_file_chooser_set_property;
	object_class->finalize = facq_file_chooser_finalize;
	object_class->constructed = facq_file_chooser_constructed;

	g_object_class_install_property(object_class,PROP_TOPWINDOW,
					g_param_spec_pointer("topwindow",
							     "Topwindow",
							     "The dialog parent window",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_TYPE,
					g_param_spec_uint("type",
							  "Type",
							  "The dialog purpose",
							   0,
							   FACQ_FILE_CHOOSER_DIALOG_TYPE_N-1,
							   0,
							   G_PARAM_READWRITE |
							   G_PARAM_CONSTRUCT_ONLY |
							   G_PARAM_STATIC_STRINGS));


	g_object_class_install_property(object_class,PROP_EXTENSION,
					g_param_spec_string("extension",
							    "Extension",
							    "The file extension for example txt",
							    "txt",
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));
	
	g_object_class_install_property(object_class,PROP_DESCRIPTION,
					g_param_spec_string("description",
						            "Description",
							    "The file type description, for example Plain text file",
							    "Plain text file",
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));
}

void facq_file_chooser_init(FacqFileChooser *chooser)
{
	chooser->priv = G_TYPE_INSTANCE_GET_PRIVATE(chooser,FACQ_TYPE_FILE_CHOOSER,FacqFileChooserPrivate);
	chooser->priv->dialog = NULL;
	chooser->priv->description = NULL;
	chooser->priv->extension = NULL;
	chooser->priv->filename = NULL;
}

/*****--- Public methods ---*****/
/**
 * facq_file_chooser_new:
 * @topwindow: The toplevel application window.
 * @type: The dialog type, see #FacqFileChooserDialogType for valid values.
 * @ext: The file extension, for example "txt".
 * @description: A description for the kind of file, for example "Plain text
 * file".
 *
 * Creates a new #FacqFileChooser object, that is able to display a
 * #GtkFileChooser dialog to the user, but this object takes care of all the
 * complicated details for you. You have to specify the @type of the dialog that
 * you want to use, the extension of the files you want to manipulate and
 * provide a description for that kind of files.
 *
 * Returns: A new #FacqFileChooser object.
 */
FacqFileChooser *facq_file_chooser_new(GtkWidget *topwindow,FacqFileChooserDialogType type,const gchar *ext,const gchar *description)
{
	g_return_val_if_fail(GTK_IS_WINDOW(topwindow),NULL);

	return g_object_new(FACQ_TYPE_FILE_CHOOSER,"topwindow",topwindow,
						   "type",type,
						   "extension",ext,
						   "description",description,
						   NULL);
}

/**
 * facq_file_chooser_run_dialog:
 * @chooser: A #FacqFileChooser object.
 *
 * Runs the dialog for selecting or saving a file, the return value varies
 * according to the user actions. If the user choose a file you can obtain the
 * filename after calling this function with
 * facq_file_chooser_get_filename_for_system() and with
 * facq_file_chooser_get_filename_for_display().
 *
 * Returns: %GTK_RESPONSE_ACCEPT if the user pressed the accept button, -1 in any
 * other case.
 */
gint facq_file_chooser_run_dialog(FacqFileChooser *chooser)
{
	g_return_val_if_fail(FACQ_IS_FILE_CHOOSER(chooser),-1);
	
	if( gtk_dialog_run(GTK_DIALOG(chooser->priv->dialog)) != GTK_RESPONSE_ACCEPT){
		return -1;
	}
	chooser->priv->filename = 
		gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser->priv->dialog));
	return GTK_RESPONSE_ACCEPT;
}

/**
 * facq_file_chooser_get_filename_for_system:
 * @chooser: A #FacqFileChooser object.
 *
 * Returns the filename of the file choose by the user, in the filesystem
 * encoding (native), this format can be used with functions like open() but it
 * should not be used for displaying on screen.
 *
 * Returns: The filename in native encoding, use g_free() when finished. If the
 * user doesn't choose any file it will return %NULL.
 */
gchar *facq_file_chooser_get_filename_for_system(const FacqFileChooser *chooser)
{
	g_return_val_if_fail(FACQ_IS_FILE_CHOOSER(chooser),NULL);
	
	if(chooser->priv->filename)
		return g_strdup(chooser->priv->filename);
	else
		return NULL;
}

/**
 * facq_file_chooser_get_filename_for_display:
 * @chooser: A #FacqFileChooser object.
 *
 * Returns the filename of the file choose by the user in utf8 format so it can
 * be used in the gui. Note that if the filename can't be converted from native
 * to utf8 the filename returned will be "Unknown file name" but the native
 * filename is still valid and can be used. To obtain the native filename use
 * facq_file_chooser_get_filename_for_system() instead.
 *
 * Returns: The filename in utf8 format, you must free it with g_free().
 * If the user doesn't choose any file it will return %NULL.
 */
gchar *facq_file_chooser_get_filename_for_display(const FacqFileChooser *chooser)
{
	gchar *utf8_filename = NULL;
	GError *local_error = NULL;

	g_return_val_if_fail(FACQ_IS_FILE_CHOOSER(chooser),NULL);
	g_return_val_if_fail(chooser->priv->filename,NULL);
	
	utf8_filename = g_filename_to_utf8(chooser->priv->filename,
						-1,NULL,NULL,&local_error);
	if(local_error){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_error->message);
		g_clear_error(&local_error);
		utf8_filename = g_strdup_printf("%s","Unknown file name");
	}
	return utf8_filename;
}

/**
 * facq_file_chooser_free:
 * @chooser: A #FacqFileChooser object.
 *
 * Destroys the #FacqFileChooser , @chooser, freeing it's resources.
 */
void facq_file_chooser_free(FacqFileChooser *chooser)
{
	g_return_if_fail(FACQ_IS_FILE_CHOOSER(chooser));
	g_object_unref(G_OBJECT(chooser));
}

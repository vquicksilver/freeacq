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
#include "facqlogwindow.h"

/**
 * SECTION:facqlogwindow
 * @title:FacqLogWindow
 * @short_description: Provides a GtkWindow to see the facqlog messages.
 * @include:facqlogwindow.h
 *
 * #FacqLogWindow provides a #GtkWindow that is able to display to the user
 * messages logged to a file using #FacqLog.
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * Internally a #FacqLogWindow uses the following objects, a #GtkWindow,
 * a #GtkScrolledWindow, a #GtkTextView, a #GFileMonitor and a #GIOChannel.
 * </para>
 * </sect1>
 */

/**
 * FacqLogWindow:
 *
 * Contains the internal details of the #FacqLogWindow.
 */

/**
 * FacqLogWindowClass:
 *
 * Class for the #FacqLogWindow objects.
 */
static void facq_log_window_initable_iface_init(GInitableIface *iface);
static gboolean facq_log_window_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);

G_DEFINE_TYPE_WITH_CODE(FacqLogWindow,facq_log_window,G_TYPE_OBJECT,
G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_log_window_initable_iface_init));

enum {
	PROP_0,
	PROP_TOP_WINDOW,
	PROP_FILENAME,
	PROP_LINES
};

struct _FacqLogWindowPrivate {
	GtkWidget *top_window;
	gchar *filename;
	guint lines;
	GtkWidget *window;
	GtkWidget *scrolled_window;
	GtkWidget *text_view;
	GIOChannel *log;
	GFile *file;
	GFileMonitor *mon;
	gulong handler_id;
	GError *construct_error;
};

/*****--- Private methods ---*****/
/* facq_log_window_from_log_to_text_buffer:
 * @log_window: A #FacqLogWindow object.
 * @log_content: The log content.
 *
 * This private method is called on the constructor method of the @log_window
 * object. It's purpose it's to split the @log_content in lines, count the
 * number of lines, and count how many lines are in the @log_content.
 * After this the buffer of the #GtkTextView will be updated, showing 
 * all the lines that can be seen according to the lines parameter used
 * in facq_log_window_new().
 */
static void facq_log_window_from_log_to_text_buffer(FacqLogWindow *log_window,gchar *log_content)
{
	guint n_lines = 0, i = 0;
	gchar **lines = NULL;
	GString *buffer = NULL;
	GtkTextBuffer *text_buffer = NULL;

	g_return_if_fail(log_content);

	/* split the log content in lines */
	lines = g_strsplit(log_content,"\n",0);

	/* count how many lines are there in the log file */
	n_lines = g_strv_length(lines);
	if(!n_lines){
		g_strfreev(lines);
		return;
	}

	text_buffer = 
		gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_window->priv->text_view));

	if(n_lines <= log_window->priv->lines){
		/* The user has requested a number of lines higher than the actual
		 * number of lines in the log file.
		 * Write all the file content to the text buffer of the text
		 * view widget */
		gtk_text_buffer_set_text(text_buffer,log_content,-1);
	}
	else {
		/* write only the last priv->lines lines in the log, that is
		 * from line (n_lines - priv->lines)-1 to n_lines.
		 * So we go line by line and append the lines to a #GString
		 * after this we put the contents of the string to the buffer
		 * and destroy the string. */
		buffer = g_string_new(NULL);
		for(i = (n_lines - log_window->priv->lines)-1 ; i < n_lines; i++){
			g_string_append(buffer,lines[i]);
			g_string_append(buffer,"\n");
		}
		gtk_text_buffer_set_text(text_buffer,buffer->str,-1);
		g_string_free(buffer,TRUE);
	}

	g_strfreev(lines);
}

/*
 * facq_log_file_append_to_text_buffer:
 * @buffer: A #GtkTextBuffer.
 * @log_content: The log content.
 * 
 * Appends the text in @log_content to the #GtkTextBuffer, @buffer.
 */
static void facq_log_file_append_to_text_buffer(GtkTextBuffer *buffer,const gchar *log_content)
{
	GtkTextIter startIter , endIter;
	gchar *text_buffer_content = NULL;
	GString *string_buffer = NULL;
	
	gtk_text_buffer_get_start_iter(buffer,&startIter);
	gtk_text_buffer_get_end_iter(buffer,&endIter);
	text_buffer_content = 
		gtk_text_buffer_get_text(buffer,&startIter,&endIter,FALSE);
	string_buffer = g_string_new(text_buffer_content);
	g_free(text_buffer_content);
	g_string_append(string_buffer,log_content);
	gtk_text_buffer_set_text(buffer,string_buffer->str,-1);
	g_string_free(string_buffer,TRUE);
}

/*
 * facq_log_file_replace_text_buffer:
 * @buffer: A #GtkTextBuffer.
 * @lines: A pointer to an array of strings with each line.
 * @read_lines: The number of lines in the array.
 * @max_lines: The maximum number of lines the user wants to display.
 *
 * This function is called when all the content in the text buffer needs to be
 * replaced, that it's when you want to write a number of lines higher than
 * the current number of free lines in the buffer, for example say that
 * the user wants to show 30 lines and the buffer has 200 new lines, we want
 * to show to the user only the last 30 lines in that 200 new lines.
 * In this case there is no need to mix old lines in the buffer with new lines.
 */
static void facq_log_file_replace_text_buffer(GtkTextBuffer *buffer,gchar **lines,guint read_lines,guint max_lines)
{
	GString *string_buffer = NULL;
	guint i = 0;

	string_buffer = g_string_new(NULL);

	for(i = (read_lines-max_lines)-1;i < read_lines;i++){
		g_string_append(string_buffer,lines[i]);
		g_string_append(string_buffer,"\n");
	}
	gtk_text_buffer_set_text(buffer,string_buffer->str,-1);

	g_string_free(string_buffer,TRUE);
}

/*
 * facq_log_file_update_text_buffer:
 * @buffer: A #GtkTextBuffer.
 * @log_content: The log content.
 * @n_prev_lines: Number of previous lines to keep in the buffer.
 *
 * This function is called, when the number of new lines available in the log
 * is minor than the number of lines requested by the user to be displayed, but
 * this time the buffer has enough free lines to show all the new lines,
 * and also can show @n_prev_lines of the previous iteration, and the number
 * of empty lines in the buffer is minor than the total number of lines read.
 */
static void facq_log_file_update_text_buffer(GtkTextBuffer *buffer,gchar *log_content,guint n_prev_lines)
{
	GtkTextIter startIter, endIter;
	gchar *text_buffer_content = NULL;
	gchar **prev_lines = NULL;
	guint n_lines_in_buffer = 0;
	GString *string_buffer = NULL;
	guint i = 0;

	gtk_text_buffer_get_start_iter(buffer,&startIter);
	gtk_text_buffer_get_end_iter(buffer,&endIter);
	text_buffer_content = 
		gtk_text_buffer_get_text(buffer,&startIter,&endIter,FALSE);
	prev_lines = g_strsplit(text_buffer_content,"\n",-1);
	n_lines_in_buffer = g_strv_length(prev_lines);
	string_buffer = g_string_new(NULL);
	for(i = (n_lines_in_buffer - n_prev_lines -1);i < n_lines_in_buffer;i++){
		g_string_append(string_buffer,prev_lines[i]);
		g_string_append(string_buffer,"\n");
	}
	g_string_append(string_buffer,log_content);

	gtk_text_buffer_set_text(buffer,string_buffer->str,-1);

	g_string_free(string_buffer,TRUE);
	g_free(text_buffer_content);
	g_strfreev(prev_lines);
}

/*****--- Callbacks ---*****/
static void log_file_changed_callback(GFileMonitor *mon,GFile *file,GFile *other,GFileMonitorEvent event,gpointer data)
{
	FacqLogWindow *log_window = FACQ_LOG_WINDOW(data);
	gchar *log_content = NULL, **read_lines = NULL;
	gsize log_size;
	GError *local_err = NULL;
	guint n_written_lines = 0, n_read_lines = 0, n_prev_lines = 0, n_empty_lines = 0;
	GtkTextBuffer *text_buffer = NULL;
	GtkTextIter endIter;

	if( g_io_channel_read_to_end(log_window->priv->log,
				     &log_content,
				     &log_size,&local_err) != G_IO_STATUS_NORMAL){
		if(local_err){
			g_printerr("%s\n",local_err->message);
			g_clear_error(&local_err);
		}
		if(log_content)
			g_free(log_content);
		return;
	}
	
	text_buffer = 
		gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_window->priv->text_view));

	n_written_lines = gtk_text_buffer_get_line_count(text_buffer);
	read_lines = g_strsplit(log_content,"\n",0);
	n_read_lines = g_strv_length(read_lines);
	n_empty_lines = log_window->priv->lines - n_written_lines;

	if(n_empty_lines >= n_read_lines){
		/* append the read_lines lines to the end */
		facq_log_file_append_to_text_buffer(text_buffer,log_content);
	}
	else {
		if(n_read_lines >= log_window->priv->lines){
			/* update the buffer with the last priv->lines lines no
			 * need to store previous lines. */
			facq_log_file_replace_text_buffer(text_buffer,
							  read_lines,
							  n_read_lines,
							  log_window->priv->lines);
		}
		else {
			/* store the last previous lines from the buffer */
			n_prev_lines = log_window->priv->lines - n_read_lines;

			/* mix the last lines with the new lines and update the
			 * buffer */
			facq_log_file_update_text_buffer(text_buffer,log_content,n_prev_lines);
		}
	}

	/* scroll to end */
	gtk_text_buffer_get_end_iter(text_buffer,&endIter);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(log_window->priv->text_view),
				     &endIter,
				     0.0,FALSE,0.0,0.0);

	g_free(log_content);
	g_strfreev(read_lines);
}

static gboolean delete_event(GtkWidget *widget,GdkEvent *event,gpointer data)
{
	facq_log_window_free(data);
	return FALSE;
}

/*****--- GObject magic ---*****/
static void facq_log_window_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqLogWindow *log_window = FACQ_LOG_WINDOW(self);

	switch(property_id){
	case PROP_TOP_WINDOW: g_value_set_pointer(value,log_window->priv->top_window);
	break;
	case PROP_FILENAME: g_value_set_string(value,log_window->priv->filename);
	break;
	case PROP_LINES: g_value_set_uint(value,log_window->priv->lines);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(log_window,property_id,pspec);
	}
}

static void facq_log_window_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqLogWindow *log_window = FACQ_LOG_WINDOW(self);

	switch(property_id){
	case PROP_TOP_WINDOW: log_window->priv->top_window = g_value_get_pointer(value);
	break;
	case PROP_FILENAME: log_window->priv->filename = g_value_dup_string(value);
	break;
	case PROP_LINES: log_window->priv->lines = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(log_window,property_id,pspec);
	}
}

static void facq_log_window_finalize(GObject *self)
{
	FacqLogWindow *log_window = FACQ_LOG_WINDOW(self);

	if(log_window->priv->construct_error)
		g_clear_error(&log_window->priv->construct_error);

	if(log_window->priv->handler_id)
		g_signal_handler_disconnect(log_window->priv->mon,
					    log_window->priv->handler_id);

	if(log_window->priv->mon)
		g_object_unref(log_window->priv->mon);

	if(log_window->priv->file)
		g_object_unref(G_OBJECT(log_window->priv->file));

	if(log_window->priv->log)
		g_io_channel_unref(log_window->priv->log);
	
	if(log_window->priv->filename)
		g_free(log_window->priv->filename);

	if(log_window->priv->window)
		gtk_widget_destroy(log_window->priv->window);

	if(G_OBJECT_CLASS(facq_log_window_parent_class)->finalize)
		(*G_OBJECT_CLASS(facq_log_window_parent_class)->finalize)(self);
}

static void facq_log_window_constructed(GObject *self)
{
	gchar *window_title = NULL;
	GtkWidget *window = NULL;
	GError *local_err = NULL;
	gchar *log_content = NULL;
	gsize log_size = 0;
	FacqLogWindow *log_window = FACQ_LOG_WINDOW(self);

	/* create a new #GIOChannel for reading the log file */
	log_window->priv->log = 
		g_io_channel_new_file(log_window->priv->filename,"r",&local_err);
	if(local_err)
		goto error;

	/* read the file to the end */
	if( g_io_channel_read_to_end(log_window->priv->log,
				     &log_content,
				     &log_size,
				     &local_err) != G_IO_STATUS_NORMAL ){
		g_io_channel_unref(log_window->priv->log);
		log_window->priv->log = NULL;
		goto error;
	}
	/* set \n as line term */
	g_io_channel_set_line_term(log_window->priv->log,"\n",-1);

	/* create the file monitor, first we create a #GFile. */
	log_window->priv->file = g_file_new_for_path(log_window->priv->filename);
	log_window->priv->mon = 
		g_file_monitor(log_window->priv->file,
			       G_FILE_MONITOR_NONE,
			       NULL,
			       &local_err);

	/* create the text view and put the last lines, lines, in buffer */
	log_window->priv->text_view = gtk_text_view_new();
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(log_window->priv->text_view),
					 FALSE);
	gtk_text_view_set_justification(GTK_TEXT_VIEW(log_window->priv->text_view),
					GTK_JUSTIFY_LEFT);
	gtk_text_view_set_editable(GTK_TEXT_VIEW(log_window->priv->text_view),
				   FALSE);
	
	facq_log_window_from_log_to_text_buffer(log_window,log_content);

	/* connect the file monitor with the callback function, every time the
	 * file changes the callback will be called*/
	g_signal_connect(log_window->priv->mon,"changed",
				G_CALLBACK(log_file_changed_callback),log_window);

	/* create a scrolled window and append the text view to it*/
	window = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(window),
				       GTK_POLICY_AUTOMATIC,
				       GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(window),GTK_SHADOW_NONE);
	log_window->priv->scrolled_window = window;
	gtk_container_add(GTK_CONTAINER(window),log_window->priv->text_view);

	/* Main log window, this window contains all the other components
	 * and will have a title according to your main application window title */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	if(gtk_window_get_title(GTK_WINDOW(log_window->priv->top_window)))
		window_title = 
			g_strdup_printf("%s Log",
				gtk_window_get_title(GTK_WINDOW(log_window->priv->top_window)));
	else
		window_title = g_strdup_printf("Log Window");

	gtk_window_set_title(GTK_WINDOW(window),window_title);
	g_free(window_title);
	gtk_window_set_destroy_with_parent(GTK_WINDOW(window),TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(log_window->priv->top_window));
	gtk_container_add(GTK_CONTAINER(window),log_window->priv->scrolled_window);
	g_signal_connect(window,"delete-event",
			G_CALLBACK(delete_event),log_window);

	/* ready to go */
	gtk_widget_show_all(window);
	log_window->priv->window = window;
	
	return;

	error:
	if(local_err)
		g_propagate_error(&log_window->priv->construct_error,local_err);
	return;
}

static void facq_log_window_class_init(FacqLogWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqLogWindowPrivate));

	object_class->set_property = facq_log_window_set_property;
	object_class->get_property = facq_log_window_get_property;
	object_class->constructed = facq_log_window_constructed;
	object_class->finalize = facq_log_window_finalize;

	g_object_class_install_property(object_class,PROP_TOP_WINDOW,
					g_param_spec_pointer("top-window",
							     "Top window",
							     "The app top window",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_FILENAME,
					g_param_spec_string("filename",
							    "Filename",
							    "The log filename",
							    "Unknown",
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_LINES,
					g_param_spec_uint("lines",
							  "Lines",
							  "The number of lines to display",
							  1,
							  10000,
							  30,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));
}

static void facq_log_window_init(FacqLogWindow *log_window)
{
	log_window->priv = G_TYPE_INSTANCE_GET_PRIVATE(log_window,FACQ_TYPE_LOG_WINDOW,FacqLogWindowPrivate);
	log_window->priv->top_window = NULL;
	log_window->priv->filename = NULL;
}

/*****--- GInitable iface ---*****/
static void facq_log_window_initable_iface_init(GInitableIface *iface)
{
	iface->init = facq_log_window_initable_init;
}

static gboolean facq_log_window_initable_init(GInitable *initable,GCancellable *cancellable,GError **error)
{
	FacqLogWindow *log_window;

	g_return_val_if_fail(FACQ_IS_LOG_WINDOW(initable),FALSE);
	log_window = FACQ_LOG_WINDOW(initable);
	if(cancellable != NULL){
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                           "Cancellable initialization not supported");
                return FALSE;
	}
	if(log_window->priv->construct_error){
		if(error)
			*error = g_error_copy(log_window->priv->construct_error);
		return FALSE;
	}
	return TRUE;
}

/*****--- Public methods ---*****/
/**
 * facq_log_window_new:
 * @top_window: The toplevel #GtkWindow of your application.
 * @filename: The name of the file where #FacqLog is logging messages.
 * @lines: The number of lines you want to show to the user, between 1 and
 * 10000, if you don't know what value to use here use 30.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqLogWindow object, and shows the log window.
 * Note that the window content will auto-update as long as new text arrives
 * the log.
 *
 * Returns: A new #FacqLogWindow object.
 */
FacqLogWindow *facq_log_window_new(GtkWidget *top_window,const gchar *filename,guint lines,GError **err)
{
	return g_initable_new(FACQ_TYPE_LOG_WINDOW,
			    NULL,err,
			    "top-window",top_window,
			    "filename",filename,
			    "lines",lines,
			    NULL);
}

/**
 * facq_log_window_free:
 * @log_window: A #FacqLogWindow object.
 *
 * Destroys a no longer needed #FacqLogWindow object, including the
 * window showed to the user.
 */
void facq_log_window_free(FacqLogWindow *log_window)
{
	g_return_if_fail(FACQ_IS_LOG_WINDOW(log_window));
	g_object_unref(G_OBJECT(log_window));
}


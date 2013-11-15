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
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqi18n.h"
#include "facqlog.h"
#include "facqcatalog.h"
#include "gdouble.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqchunk.h"
#include "facqsource.h"
#include "facqoperation.h"
#include "facqoperationlist.h"
#include "facqsink.h"
#include "facqpipelinemessage.h"
#include "facqpipelinemonitor.h"
#include "facqpipeline.h"
#include "facqstream.h"
#include "facqstatusbar.h"
#include "facqcapturemenu.h"
#include "facqcapturetoolbar.h"
#include "facqstreamview.h"
#include "facqstreamdialog.h"
#include "facqcatalogdialog.h"
#include "facqdyndialog.h"
#include "facqlogwindow.h"
#include "facqresourcesicons.h"
#include "facqcapture.h"

/**
 * SECTION:facqcapture
 * @short_description: Main object in the capture application
 * @include:facqcapture.h
 * @title:FacqCapture
 * @see_also:#FacqCatalog
 *
 * Implements the control class and creates the graphical user interface for
 * the capture application.
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * The capture interface uses the following #GtkWidget objects, a toplevel
 * #GtkWindow, a #FacqCaptureMenu object, a #FaqCaptureToolbar object,
 * a #FacqStatusbar, and a #FacqStreamView for showing the status of the
 * stream and help the user with the manipulation of the stream.
 *
 * For showing the capture log a #FacqLogWindow is used.
 *
 * For implementing the operations a #FacqStream and a #FacqCatalog are used.
 * Before creating a #FacqCapture object, you need to create a #FacqCatalog.
 * </para>
 * </sect1>
 */

G_DEFINE_TYPE(FacqCapture,facq_capture,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_CATALOG
};

struct _FacqCapturePrivate {
	GtkWidget *window;
	FacqCaptureMenu *menu;
	FacqCaptureToolbar *toolbar;
	FacqStatusbar *statusbar;
	FacqStreamView *view;
	FacqLogWindow *log_window;
	const FacqCatalog *catalog;
	FacqStream *stream;
	guint ring_chunks;
};

/*****--- callbacks ---*****/
/*
 * delete_event:
 *
 * This callback gets called when the user presses the quit button,
 * it brakes the main loop allowing the application to exit.
 */
static gboolean delete_event(GtkWidget *widget,GdkEvent *event,gpointer data)
{
	gtk_main_quit();
	return FALSE;
}

/*
 * capture_stop_callback:
 *
 * This callback is used by the #FacqPipelineMonitor object that is created
 * when the stream is started, if the #FacqPipelineMonitor object gets a 
 * stop condition from any of the objects in the #FacqPipeline, this callback gets
 * exec. The callback calls facq_capture_control_stop() stopping the stream
 * and updates the interface state. Also if the #FacqPipelineMessage has
 * some info of the stop condition it will be showed to the user in the statusbar.
 */
static void capture_stop_callback(FacqPipelineMessage *msg,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);
	gchar *info = NULL;

	facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,"%s", "On stop callback");
	facq_capture_control_stop(cap);
	info = facq_pipeline_message_get_info(msg);
	if(info)
		facq_statusbar_write_msg(cap->priv->statusbar,"%s",info);
	g_free(info);
}


/*
 * capture_error_callback:
 *
 * This callback is used by the #FacqPipelineMonitor object that is created
 * when the stream is started, if the #FacqPipelineMonitor object gets a
 * error condition from any of the objects in the #FacqPipeline, this callback
 * gets exec. The callback calls facq_capture_control_stop() stopping the stream
 * and updating the graphical user interface. If there is some textual info in
 * the @msg, it will be showed to the user in the statusbar
 */
static void capture_error_callback(FacqPipelineMessage *msg,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);
	gchar *info = NULL;

	facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,"%s", "On error callback");
	facq_capture_control_stop(cap);
	facq_stream_view_set_status(cap->priv->view,FACQ_STREAM_VIEW_STATUS_ERROR);
	info = facq_pipeline_message_get_info(msg);
	if(info)
		facq_statusbar_write_msg(cap->priv->statusbar,_("Error: %s"),info);
	g_free(info);
}

static void facq_capture_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqCapture *cap = FACQ_CAPTURE(self);

	switch(property_id){
	case PROP_CATALOG: cap->priv->catalog = g_value_get_pointer(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(cap,property_id,pspec);
	}
}

static void facq_capture_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqCapture *cap = FACQ_CAPTURE(self);

	switch(property_id){
	case PROP_CATALOG:
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(cap,property_id,pspec);
	}
}

static void facq_capture_constructed(GObject *self)
{
	FacqCapture *cap = FACQ_CAPTURE(self);
	GtkWidget *vbox = NULL;

	cap->priv->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(cap->priv->window),_("Capture"));
	gtk_window_set_icon(GTK_WINDOW(cap->priv->window),
				facq_resources_icons_capture());
	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(cap->priv->window),vbox);

	cap->priv->statusbar = facq_statusbar_new();
	cap->priv->menu = facq_capture_menu_new(cap);
	cap->priv->toolbar = facq_capture_toolbar_new(cap);
	cap->priv->view = facq_stream_view_new();

	gtk_box_pack_start(GTK_BOX(vbox),
			facq_capture_menu_get_widget(cap->priv->menu),FALSE,FALSE,0);

	gtk_box_pack_start(GTK_BOX(vbox),
			facq_capture_toolbar_get_widget(cap->priv->toolbar),FALSE,FALSE,0);

	gtk_box_pack_start(GTK_BOX(vbox),
			facq_stream_view_get_widget(cap->priv->view),TRUE,TRUE,0);

	gtk_box_pack_end(GTK_BOX(vbox),
			facq_statusbar_get_widget(cap->priv->statusbar),FALSE,FALSE,0);

	g_signal_connect(cap->priv->window,"delete-event",
				G_CALLBACK(delete_event),cap);

	gtk_widget_show_all(cap->priv->window);
}

static void facq_capture_finalize(GObject *self)
{
	FacqCapture *cap = FACQ_CAPTURE(self);

	if(FACQ_IS_LOG_WINDOW(cap->priv->log_window)){
		facq_log_window_free(cap->priv->log_window);
	}

	if(FACQ_IS_CAPTURE_MENU(cap->priv->menu)){
		facq_capture_menu_free(cap->priv->menu);
	}

	if(FACQ_IS_CAPTURE_TOOLBAR(cap->priv->toolbar)){
		facq_capture_toolbar_free(cap->priv->toolbar);
	}

	if(FACQ_IS_STREAM_VIEW(cap->priv->view)){
		facq_stream_view_free(cap->priv->view);
	}

	if(FACQ_IS_STREAM(cap->priv->stream)){
		facq_stream_free(cap->priv->stream);
	}

	if(FACQ_IS_STATUSBAR(cap->priv->statusbar)){
		facq_statusbar_free(cap->priv->statusbar);
	}

	if(GTK_IS_WIDGET(cap->priv->window))
		gtk_widget_destroy(cap->priv->window);

	G_OBJECT_CLASS(facq_capture_parent_class)->finalize(self);
}

static void facq_capture_class_init(FacqCaptureClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqCapturePrivate));

	object_class->set_property = facq_capture_set_property;
	object_class->get_property = facq_capture_get_property;
	object_class->constructed = facq_capture_constructed;
	object_class->finalize = facq_capture_finalize;

	g_object_class_install_property(object_class,PROP_CATALOG,
						g_param_spec_pointer("catalog",
								     "Catalog",
								     "A Catalog with the supported elements",
								     G_PARAM_CONSTRUCT_ONLY |
								     G_PARAM_READWRITE |
								     G_PARAM_STATIC_STRINGS));
}

static void facq_capture_init(FacqCapture *cap)
{
	cap->priv = G_TYPE_INSTANCE_GET_PRIVATE(cap,FACQ_TYPE_CAPTURE,FacqCapturePrivate);
	cap->priv->ring_chunks = 32;
}

/**
 * facq_capture_new:
 * @catalog: A #FacqCatalog object with the registered items.
 *
 * Creates a new #FacqCapture object, that creates all the needed
 * graphical user interface objects allowing the user to use the
 * capture application.
 *
 * Returns: A new #FacqCapture object.
 */
FacqCapture *facq_capture_new(const FacqCatalog *catalog)
{
	return g_object_new(FACQ_TYPE_CAPTURE,"catalog",catalog,NULL);
}

/**
 * facq_capture_get_widget:
 * @cap: A #FacqCapture object.
 *
 * Gets the top level #GtkWindow of the #FacqCapture object, which in turn
 * it's the capture application top window.
 *
 * Returns: A #GtkWidget pointing to the top level #GtkWindow.
 */
GtkWidget *facq_capture_get_widget(const FacqCapture *cap)
{
	g_return_val_if_fail(FACQ_IS_CAPTURE(cap),NULL);
	return cap->priv->window;
}

/**
 * facq_capture_stream_preferences:
 * @cap: A #FacqCapture object.
 *
 * Allows the user to configure the number of chunks that
 * can be used at the same time in the #FacqBuffer that is
 * created by the #FacqPipeline, that is created when 
 * the stream is started. The number of chunks provided by the
 * user will be stored in the @cap object, and will be used in
 * the next stream created.
 * This function is called when the user presses the preferences button
 * in the Stream menu of the GUI.
 */
void facq_capture_stream_preferences(FacqCapture *cap)
{
	FacqDynDialog *dyn_diag = NULL;
	const GPtrArray *user_input = NULL;
	guint *ring_chunks = NULL;
	GError *local_err = NULL;

	g_return_if_fail(FACQ_IS_CAPTURE(cap));

	dyn_diag = facq_dyn_dialog_new(cap->priv->window,
				       "UINT,Ring size,1024,32,32,1",
				       &local_err);
	if(local_err){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
		g_clear_error(&local_err);
		return;
	}
	if( facq_dyn_dialog_run(dyn_diag) == GTK_RESPONSE_OK){
		user_input = facq_dyn_dialog_get_input(dyn_diag);
		ring_chunks = g_ptr_array_index(user_input,0);
		cap->priv->ring_chunks = *ring_chunks;
		facq_statusbar_write_msg(cap->priv->statusbar,"%s",_("Settings applied"));
	}
	facq_dyn_dialog_free(dyn_diag);
}

/**
 * facq_capture_stream_new:
 * @cap: A #FacqCapture object.
 *
 * Creates a new #FacqStream so new items can be added to the
 * stream. First the function creates a new #FacqStreamDialog
 * for asking the user a name for the stream. If the user doesn't
 * cancel the dialog, the name is retrieved from the #FacqStreamDialog
 * and an emptry #FacqStream is created with the name, the number of
 * chunks in the #FacqBuffer stored in @cap, and the private
 * capture_stop_callback and capture_error_callback provided in this file.
 * After this the #GtkWindow title is set using the stream name, the
 * preferences, the save as, the new, and open entries in the #FacqCaptureMenu
 * are disabled and the add button and entry is enabled. Finally a message is
 * shown in the statusbar to the user, and the #FacqStreamView is set to
 * the %FACQ_STREAM_VIEW_STATUS_NEW_STREAM state.
 * This function is called when the user presses the Stream->New entry in
 * the #FacqCaptureMenu.
 */
void facq_capture_stream_new(FacqCapture *cap)
{
	gchar *window_title = NULL;
	gchar *name = NULL;
	FacqStreamDialog *dialog = NULL;

	g_return_if_fail(FACQ_IS_CAPTURE(cap));

	if(cap->priv->stream){
		facq_statusbar_write_msg(cap->priv->statusbar,"%s",
				_("Stream exists, close the stream first"));
		return;
	}

	dialog = facq_stream_dialog_new(cap->priv->window,NULL);
	if( facq_stream_dialog_run(dialog) == GTK_RESPONSE_OK){
		name = facq_stream_dialog_get_input(dialog);
		if(name){
			cap->priv->stream = facq_stream_new(name,
						cap->priv->ring_chunks,
							capture_stop_callback,
								capture_error_callback,cap);
			window_title = g_strdup_printf("%s - %s",_("Capture"),name);
			g_free(name);
			gtk_window_set_title(GTK_WINDOW(cap->priv->window),window_title);
			g_free(window_title);
			facq_capture_menu_disable_preferences(cap->priv->menu);
			facq_capture_menu_disable_save_as(cap->priv->menu);
			facq_capture_menu_enable_add(cap->priv->menu);
			facq_capture_menu_enable_close(cap->priv->menu);
			facq_capture_toolbar_enable_add(cap->priv->toolbar);
			facq_capture_menu_disable_new(cap->priv->menu);
			facq_capture_menu_disable_open(cap->priv->menu);
			facq_stream_view_set_status(cap->priv->view,FACQ_STREAM_VIEW_STATUS_NEW_STREAM);
			facq_statusbar_write_msg(cap->priv->statusbar,"%s",_("New stream created"));
		}
	}
	facq_stream_dialog_free(dialog);
}

/**
 * facq_capture_stream_open:
 * @cap: A #FacqCapture object.
 * 
 * This function allows the user to tell the system a filename, and
 * the system will parse the .frs (Freeacq Readable Stream, see #FacqStream
 * for more details) and if successful will create a new #FacqStream, with
 * the stored objects and parameters in the .frs file.
 *
 * The function first creates a #FacqDynDialog which in turn uses a
 * #FacqFileChooser for asking the user a filename and a path of .frs file.
 * If the user doesn't cancel the operation in the dialog, the
 * facq_stream_load() is called for trying to create a new #FacqStream.
 * If the function fails and error is displayed to the user in the
 * #FacqStatusbar. If the function it's successful a message is shown to 
 * the user in #FacqStatusbar allowing the user to know that the operation is
 * successful, the window title is updated with the stream name,
 * the #FacqStreamView is updated to show the items in the new
 * created #FacqStream (For showing the name and description of each item),
 * and the GUI is updated to disable the Open,Preferences,New,Add,Stop entries
 * in the #FacqCaptureMenu, and the Save as, Play,Clear,Close and Remove are
 * enabled. In the #FacqCaptureToolbar the Add, and Stop buttons are disabled and the
 * Play,Clear and Remove buttons are enabled.
 * Finally the #FacqStreamView status is set to
 * %FACQ_STREAM_VIEW_STATUS_WITH_SINK.
 *
 * This function it's called when the user presses the Open entry in
 * the Stream submenu in the #FacqCaptureMenu.
 */
void facq_capture_stream_open(FacqCapture *cap)
{
	GError *local_err = NULL;
	const gchar *filename = NULL;
	FacqDynDialog *dyn_dialog = NULL;
	const GPtrArray *user_input = NULL;
	FacqStream *stream = NULL;
	gchar *window_title = NULL, *name = NULL;
	gpointer item = NULL;
	const gchar *item_name = NULL , *item_desc = NULL;
	guint i = 0, n_operations = 0;

	g_return_if_fail(FACQ_IS_CAPTURE(cap));
	g_return_if_fail(cap->priv->stream == NULL);
	dyn_dialog = facq_dyn_dialog_new(cap->priv->window,"FILENAME,1,frs,Freeacq Readable Stream",NULL);

	if( facq_dyn_dialog_run(dyn_dialog) == GTK_RESPONSE_OK){
		user_input = facq_dyn_dialog_get_input(dyn_dialog);
		filename = g_ptr_array_index(user_input,0);
		stream = facq_stream_load(filename,cap->priv->catalog,
					  cap->priv->ring_chunks,
					  capture_stop_callback,
					  capture_error_callback,cap,
					  &local_err);
		if(!stream){
			if(local_err){
				facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
				facq_statusbar_write_msg(cap->priv->statusbar,
							 "%s",local_err->message);
				g_clear_error(&local_err);
			}
		}
		else {
			facq_statusbar_write_msg(cap->priv->statusbar,"%s",_("Stream loaded"));
			cap->priv->stream = stream;
			name = facq_stream_get_name(stream);
			window_title = g_strdup_printf("%s - %s",_("Capture"),name);
			gtk_window_set_title(GTK_WINDOW(cap->priv->window),window_title);
			g_free(window_title);
			g_free(name);
			if(facq_stream_is_closed(cap->priv->stream)){
				/* update the stream view with each item */
				/* source */
				item = facq_stream_get_source(stream);
				item_name = facq_source_get_name(item);
				item_desc = facq_source_get_description(item);
				facq_stream_view_push_item(cap->priv->view,
							   FACQ_STREAM_VIEW_ITEM_TYPE_SOURCE,
							   item_name,
							   item_desc);

				n_operations = facq_stream_get_operation_num(stream);
				for(i = 0;i < n_operations;i++){
					item = facq_stream_get_operation(stream,i);
					item_name = facq_operation_get_name(item);
					item_desc = facq_operation_get_description(item);
					facq_stream_view_push_item(cap->priv->view,
								   FACQ_STREAM_VIEW_ITEM_TYPE_OPERATION,
								   item_name,
								   item_desc);
				}

				item = facq_stream_get_sink(stream);
				item_name = facq_sink_get_name(item);
				item_desc = facq_sink_get_description(item);
				facq_stream_view_push_item(cap->priv->view,
							   FACQ_STREAM_VIEW_ITEM_TYPE_SINK,
							   item_name,
							   item_desc);
				/* update the gui */
				facq_capture_menu_disable_open(cap->priv->menu);
				facq_capture_menu_disable_preferences(cap->priv->menu);
				facq_capture_menu_disable_new(cap->priv->menu);
				facq_capture_menu_enable_save_as(cap->priv->menu);
				facq_capture_toolbar_disable_add(cap->priv->toolbar);
				facq_capture_menu_disable_add(cap->priv->menu);
				facq_capture_toolbar_disable_stop(cap->priv->toolbar);
				facq_capture_menu_disable_stop(cap->priv->menu);
				facq_capture_toolbar_enable_play(cap->priv->toolbar);
				facq_capture_menu_enable_play(cap->priv->menu);
				facq_capture_toolbar_enable_clear(cap->priv->toolbar);
				facq_capture_menu_enable_clear(cap->priv->menu);
				facq_capture_menu_enable_close(cap->priv->menu);
				facq_capture_menu_enable_remove(cap->priv->menu);
				facq_capture_toolbar_enable_remove(cap->priv->toolbar);
				facq_stream_view_set_status(cap->priv->view,FACQ_STREAM_VIEW_STATUS_WITH_SINK);
			}
		}
	}
	facq_dyn_dialog_free(dyn_dialog);
}

/**
 * facq_capture_stream_save_as:
 * @cap: A #FacqCapture object.
 *
 */
void facq_capture_stream_save_as(FacqCapture *cap)
{
	GError *local_err = NULL;
	FacqDynDialog *dyn_dialog = NULL;
	const GPtrArray *user_input = NULL;
	gchar *filename = NULL;

	g_return_if_fail(FACQ_IS_CAPTURE(cap));
	g_return_if_fail(FACQ_IS_STREAM(cap->priv->stream));
	g_return_if_fail(facq_stream_is_closed(cap->priv->stream));

	facq_capture_menu_disable_clear(cap->priv->menu);
	facq_capture_menu_disable_remove(cap->priv->menu);
	
	dyn_dialog = facq_dyn_dialog_new(cap->priv->window,
					 "FILENAME,0,frs,Freeacq Readable Stream",
					 NULL);

	if( facq_dyn_dialog_run(dyn_dialog) == GTK_RESPONSE_OK){
		user_input = facq_dyn_dialog_get_input(dyn_dialog);
		filename = g_ptr_array_index(user_input,0);
		if(!facq_stream_save(cap->priv->stream,filename,&local_err)){
			if(local_err){
				facq_statusbar_write_msg(cap->priv->statusbar,"%s",local_err->message);
				facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
				g_clear_error(&local_err);
			}
		}
		else 
			facq_statusbar_write_msg(cap->priv->statusbar,"%s",_("Stream saved"));
	}

	facq_dyn_dialog_free(dyn_dialog);
	facq_capture_menu_enable_clear(cap->priv->menu);
	facq_capture_menu_enable_remove(cap->priv->menu);
}

/**
 * facq_capture_stream_close:
 * @cap: A #FacqCapture object.
 *
 * Closes the stream, destroying it, the user won't be able to
 * add new elements to the stream until a new stream has been created.
 *
 */
void facq_capture_stream_close(FacqCapture *cap)
{
	g_return_if_fail(FACQ_IS_CAPTURE(cap));
	
	if(!cap->priv->stream)
		return;

	facq_stream_free(cap->priv->stream);
	cap->priv->stream = NULL;

	facq_capture_menu_disable_close(cap->priv->menu);
	facq_capture_menu_disable_play(cap->priv->menu);
	facq_capture_toolbar_disable_play(cap->priv->toolbar);
	facq_capture_menu_disable_stop(cap->priv->menu);
	facq_capture_toolbar_disable_stop(cap->priv->toolbar);
	facq_capture_menu_disable_add(cap->priv->menu);
	facq_capture_toolbar_disable_add(cap->priv->toolbar);
	facq_capture_menu_disable_remove(cap->priv->menu);
	facq_capture_toolbar_disable_remove(cap->priv->toolbar);
	facq_capture_menu_disable_clear(cap->priv->menu);
	facq_capture_toolbar_disable_clear(cap->priv->toolbar);
	facq_stream_view_set_status(cap->priv->view,FACQ_STREAM_VIEW_STATUS_NO_STREAM);
	gtk_window_set_title(GTK_WINDOW(cap->priv->window),_("Capture"));
	facq_stream_view_clear_data(cap->priv->view);
	facq_statusbar_write_msg(cap->priv->statusbar,_("Stream closed"));
	facq_capture_menu_disable_save_as(cap->priv->menu);
	facq_capture_menu_enable_preferences(cap->priv->menu);
	facq_capture_menu_enable_new(cap->priv->menu);
	facq_capture_menu_enable_open(cap->priv->menu);
}

/*
 * facq_capture_control_add_show_catalog:
 * @cap: A #FacqCapture object.
 * @type: The type of catalog to show to the user, (Sources, operations or
 * sinks).
 * @selected: This variable will store the index of the selected item in
 * the catalog if any.
 *
 * Creates and display the #FacqCatalogDialog to the user, showing the 
 * requested type of items, and setting the @selected variable to the
 * index of the chosen element.
 *
 * Returns: %TRUE if the user selected some item in the catalog,
 * %FALSE in any other case.
 */
static gboolean facq_capture_control_add_show_catalog(FacqCapture *cap,FacqCatalogType *type,guint *selected)
{
	FacqCatalogDialog *dialog = NULL;
	gboolean really_selected = FALSE;

	dialog = facq_catalog_dialog_new(cap->priv->window,
					 cap->priv->catalog,
					 *type);

	if( facq_catalog_dialog_run(dialog) != GTK_RESPONSE_OK){
		facq_catalog_dialog_free(dialog);
		return FALSE;
	}
	else {
		*selected = facq_catalog_dialog_get_input(dialog,&really_selected,type);
		facq_catalog_dialog_free(dialog);
		if(!really_selected)
			return FALSE;
	}

	return TRUE;
}

/*
 * facq_capture_control_show_dyn_dialog:
 * @cap: A #FacqCapture object.
 * @type: The type of elements in the #FacqCatalog to show, a valid
 * #FacqCatalogType value.
 * @selected: The index of the selected item inside the @type catalog.
 *
 * Displays the chosen element type dialog to the user, so the user is
 * able to select the properties of the item for configuring it.
 *
 * The function uses the #FacqCatalog referenced in the #FacqCapture object
 * to get a dyn dialog string of the @type and @selected item. Then
 * uses the facq_dyn_dialog_new() function to create a new #FacqDynDialog
 * and displays it to the user, with facq_dyn_dialog_run().
 *
 * Returns: a #FacqDynDialog if the user pressed OK in the dialog, or
 * %NULL in other case. You can get the input parameters from the
 * #FacqDynDialog later.
 */
static FacqDynDialog *facq_capture_control_show_dyn_dialog(FacqCapture *cap,FacqCatalogType type,guint selected)
{
	gchar *dyn_desc = NULL;
	FacqDynDialog *dialog = NULL;
	GError *local_err = NULL;

	dyn_desc = facq_catalog_get_dyn_diag_string(cap->priv->catalog,
			                            type,
						    selected);
	if(!dyn_desc)
		return NULL;
	dialog = facq_dyn_dialog_new(cap->priv->window,
				     dyn_desc,
				     &local_err);
	g_free(dyn_desc);
	if(local_err){
		g_clear_error(&local_err);
		return NULL;
	}
	if(facq_dyn_dialog_run(dialog) == GTK_RESPONSE_OK){
		return dialog;
	}
	facq_dyn_dialog_free(dialog);
	return NULL;
}

/**
 * facq_capture_control_add:
 * @cap: A #FacqCapture object.
 *
 * Adds a new item to the #FacqStream.
 *
 * This function is called when the user presses the add button in
 * the #FacqCaptureToolbar or when the user presses the add entry in
 * the control submenu, in the #FacqCaptureMenu.
 */
void facq_capture_control_add(FacqCapture *cap)
{
	FacqCatalogType type;
	FacqDynDialog *dyn_dialog = NULL;
	const GPtrArray *user_input = NULL;
	GError *local_err = NULL;
	guint selected = 0;
	gpointer element = NULL;
	gchar *name = NULL, *desc = NULL;

	g_return_if_fail(FACQ_IS_CAPTURE(cap));
	g_return_if_fail(FACQ_IS_STREAM(cap->priv->stream));

	if(facq_stream_is_closed(cap->priv->stream))
		return;
	
	if(!facq_stream_get_source(cap->priv->stream)){
		type = FACQ_CATALOG_TYPE_SOURCE;
	}
	else {
		type = FACQ_CATALOG_TYPE_OPERATION;
	}

	if(facq_capture_control_add_show_catalog(cap,&type,&selected)){
		dyn_dialog = facq_capture_control_show_dyn_dialog(cap,type,selected);
		if(dyn_dialog){
			user_input = facq_dyn_dialog_get_input(dyn_dialog);
			element = 
				facq_catalog_constructor_call(cap->priv->catalog,
							      type,
							      selected,
							      user_input,
							      &local_err);
			facq_dyn_dialog_free(dyn_dialog);
		}
	}

	if(local_err){
		facq_statusbar_write_msg(cap->priv->statusbar,"%s",local_err->message);
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
		g_clear_error(&local_err);
		return;
	}

	if(element){
		name = facq_catalog_get_name(cap->priv->catalog,type,selected);
		desc = facq_catalog_get_description(cap->priv->catalog,type,selected);
		switch(type){
		case FACQ_CATALOG_TYPE_SOURCE:
			facq_stream_set_source(cap->priv->stream,FACQ_SOURCE(element));
			facq_stream_view_set_status(cap->priv->view,
						    FACQ_STREAM_VIEW_STATUS_WITH_SOURCE);
			facq_capture_menu_enable_remove(cap->priv->menu);
			facq_capture_toolbar_enable_remove(cap->priv->toolbar);
			facq_capture_toolbar_enable_clear(cap->priv->toolbar);
			facq_capture_menu_enable_clear(cap->priv->menu);
			facq_statusbar_write_msg(cap->priv->statusbar,"%s",_("Source added to stream"));
			facq_stream_view_push_item(cap->priv->view,
					           FACQ_STREAM_VIEW_ITEM_TYPE_SOURCE,
						   name,desc);
		break;
		case FACQ_CATALOG_TYPE_OPERATION:
			facq_stream_append_operation(cap->priv->stream,FACQ_OPERATION(element));
			facq_statusbar_write_msg(cap->priv->statusbar,"%s",_("Operation added to stream"));
			facq_stream_view_push_item(cap->priv->view,
						   FACQ_STREAM_VIEW_ITEM_TYPE_OPERATION,
						   name,desc);
		break;
		case FACQ_CATALOG_TYPE_SINK:
			facq_stream_set_sink(cap->priv->stream,FACQ_SINK(element));
			facq_statusbar_write_msg(cap->priv->statusbar,"%s",_("Sink added, stream ready"));
			facq_stream_view_set_status(cap->priv->view,FACQ_STREAM_VIEW_STATUS_WITH_SINK);
			facq_stream_view_push_item(cap->priv->view,
						   FACQ_STREAM_VIEW_ITEM_TYPE_SINK,
						   name,desc);
		break;
		}
		if(name)
			g_free(name);
		if(desc)
			g_free(desc);
	}

	if(facq_stream_is_closed(cap->priv->stream)){
		facq_capture_menu_enable_save_as(cap->priv->menu);
		facq_capture_toolbar_disable_add(cap->priv->toolbar);
		facq_capture_menu_disable_add(cap->priv->menu);
		facq_capture_toolbar_disable_stop(cap->priv->toolbar);
		facq_capture_menu_disable_stop(cap->priv->menu);
		facq_capture_toolbar_enable_play(cap->priv->toolbar);
		facq_capture_menu_enable_play(cap->priv->menu);
		facq_capture_toolbar_enable_clear(cap->priv->toolbar);
		facq_capture_menu_enable_clear(cap->priv->menu);
		facq_stream_view_set_status(cap->priv->view,FACQ_STREAM_VIEW_STATUS_WITH_SINK);
	}
}

/**
 * facq_capture_control_remove:
 * @cap: A #FacqCapture object.
 *
 * Removes the last element added to the stream.
 *
 * The function first pops the item from the #FacqStreamView, calling the
 * function facq_stream_view_pop_item(), after this the add button and entry
 * are enabled. Finally the function checks what kind of element is the latest
 * element added to the stream using the facq_stream_get_sink(),
 * facq_stream_get_operation_num() and facq_stream_get_source() functions
 * removes the element from the stream using the facq_stream_remove_sink(),
 * facq_stream_remove_operation() and facq_stream_remove_source() functions
 * and updates the GUI (The #FacqStreamView item) according to the kind of element.
 *
 * A status message is written to the #FacqStatusbar when finished.
 *
 * This function is called when the user presses the remove button
 * in the toolbar, or the remove menu entry.
 */
void facq_capture_control_remove(FacqCapture *cap)
{
	g_return_if_fail(FACQ_IS_CAPTURE(cap));
	g_return_if_fail(FACQ_IS_STREAM(cap->priv->stream));

	facq_stream_view_pop_item(cap->priv->view);
	facq_capture_menu_enable_add(cap->priv->menu);
	facq_capture_toolbar_enable_add(cap->priv->toolbar);

	if(facq_stream_get_sink(cap->priv->stream)){
		facq_stream_remove_sink(cap->priv->stream);
		facq_stream_view_set_status(cap->priv->view,FACQ_STREAM_VIEW_STATUS_WITH_SOURCE);
		facq_statusbar_write_msg(cap->priv->statusbar,"%s",_("Sink removed"));
		facq_capture_menu_disable_play(cap->priv->menu);
		facq_capture_toolbar_disable_play(cap->priv->toolbar);
		facq_capture_menu_disable_save_as(cap->priv->menu);
		return;
	}

	if(facq_stream_get_operation_num(cap->priv->stream)){
		facq_stream_remove_operation(cap->priv->stream);
		facq_stream_view_set_status(cap->priv->view,FACQ_STREAM_VIEW_STATUS_WITH_SOURCE);
		facq_statusbar_write_msg(cap->priv->statusbar,"%s",_("Operation removed"));
		return;
	}

	if(facq_stream_get_source(cap->priv->stream)){
		facq_stream_remove_source(cap->priv->stream);
		facq_stream_view_set_status(cap->priv->view,FACQ_STREAM_VIEW_STATUS_NEW_STREAM);
		facq_statusbar_write_msg(cap->priv->statusbar,"%s",_("Source removed"));
		facq_capture_menu_disable_remove(cap->priv->menu);
		facq_capture_toolbar_disable_remove(cap->priv->toolbar);
		facq_capture_toolbar_disable_clear(cap->priv->toolbar);
		facq_capture_menu_disable_clear(cap->priv->menu);
		facq_capture_menu_enable_add(cap->priv->menu);
		facq_capture_toolbar_enable_add(cap->priv->toolbar);
		return;
	}
}

/**
 * facq_capture_control_clear:
 * @cap: A #FacqCapture object.
 *
 * Clears all the elements that have been added to the stream, including
 * sources, operations and sinks, but the stream is not destroyed, the user
 * can add more items to the stream after it's has been cleared.
 *
 * The function first call facq_stream_clear() clearing the stream, and after
 * this the user interface is modified to reflect the new stream state, 
 * the preferences, remove, clear, play, stop and save as entries are disabled in
 * the #FacqCaptureMenu, the remove, clear, add, play and stop buttons are
 * disabled in the #FacqCaptureToolbar. The add entry in the menu and the
 * add button in the toolbar are enabled, and the #FacqStreamView state is set
 * to %FACQ_STREAM_VIEW_STATUS_NEW_STREAM.
 *
 * This function is called when the user presses the Clear button
 * in the #FacqCaptureToolbar or the Clear menu entry in the #FacqCaptureMenu.
 */
void facq_capture_control_clear(FacqCapture *cap)
{
	g_return_if_fail(FACQ_IS_CAPTURE(cap));
	g_return_if_fail(FACQ_IS_STREAM(cap->priv->stream));

	facq_stream_clear(cap->priv->stream);

	facq_capture_menu_disable_preferences(cap->priv->menu);
	facq_capture_menu_enable_add(cap->priv->menu);
	facq_capture_menu_disable_remove(cap->priv->menu);
	facq_capture_toolbar_disable_remove(cap->priv->toolbar);
	facq_capture_menu_disable_clear(cap->priv->menu);
	facq_capture_toolbar_disable_clear(cap->priv->toolbar);
	facq_capture_menu_enable_close(cap->priv->menu);
	facq_capture_toolbar_enable_add(cap->priv->toolbar);
	facq_capture_menu_disable_play(cap->priv->menu);
	facq_capture_toolbar_disable_play(cap->priv->toolbar);
	facq_capture_menu_disable_stop(cap->priv->menu);
	facq_capture_toolbar_disable_stop(cap->priv->toolbar);
	facq_capture_menu_disable_save_as(cap->priv->menu);
	facq_stream_view_set_status(cap->priv->view,FACQ_STREAM_VIEW_STATUS_NEW_STREAM);
	facq_stream_view_clear_data(cap->priv->view);
	facq_statusbar_write_msg(cap->priv->statusbar,"%s",_("Stream cleared"));
}

/**
 * facq_capture_control_play:
 * @cap: A #FacqCapture object.
 *
 * Starts the stream, allowing the data to be captured in the source
 * processed by the operations (If any), and consumed by the sink.
 *
 * The @cap object, should contain a #FacqStream that is closed.
 *
 * The function disables the Play,Add,Remove,Clear buttons in the
 * #FacqCaptureToolbar, and the Play,Add,Remove,Clear and Close entries
 * in the #FacqCaptureMenu and enabled the stop button. 
 * After this calls the facq_stream_start()
 * function, if the function is successful the #FacqStreamView status is
 * set to %FACQ_STREAM_VIEW_STATUS_PLAY and the message "Stream started"
 * is pushed to the #FacqStatusbar, if the function fails the Close,Play,
 * Remove,Clear buttons in the toolbar are enabled and the Stop button
 * is disabled, the #FacqStreamView status is set to
 * %FACQ_STREAM_VIEW_STATUS_ERROR, and the error is pushed to the
 * #FacqStatusbar.
 *
 * This function is called when the user presses the Play button in
 * the #FacqCaptureToolbar or the Play entry in the #FacqCaptureMenu.
 */
void facq_capture_control_play(FacqCapture *cap)
{
	GError *local_err = NULL;

	g_return_if_fail(FACQ_IS_CAPTURE(cap));
	g_return_if_fail(FACQ_IS_STREAM(cap->priv->stream));
	g_return_if_fail(facq_stream_is_closed(cap->priv->stream));
	
	facq_capture_menu_disable_play(cap->priv->menu);
	facq_capture_toolbar_disable_play(cap->priv->toolbar);
	facq_capture_menu_enable_stop(cap->priv->menu);
	facq_capture_toolbar_enable_stop(cap->priv->toolbar);
	facq_capture_menu_disable_add(cap->priv->menu);
	facq_capture_toolbar_disable_add(cap->priv->toolbar);
	facq_capture_menu_disable_remove(cap->priv->menu);
	facq_capture_toolbar_disable_remove(cap->priv->toolbar);
	facq_capture_menu_disable_clear(cap->priv->menu);
	facq_capture_toolbar_disable_clear(cap->priv->toolbar);
	facq_capture_menu_disable_close(cap->priv->menu);

	if(!facq_stream_start(cap->priv->stream,&local_err)){
		facq_capture_menu_enable_close(cap->priv->menu);
		facq_capture_menu_enable_play(cap->priv->menu);
		facq_capture_toolbar_enable_play(cap->priv->toolbar);
		facq_capture_menu_disable_stop(cap->priv->menu);
		facq_capture_toolbar_disable_stop(cap->priv->toolbar);
		facq_capture_menu_disable_add(cap->priv->menu);
		facq_capture_toolbar_disable_add(cap->priv->toolbar);
		facq_capture_menu_enable_remove(cap->priv->menu);
		facq_capture_toolbar_enable_remove(cap->priv->toolbar);
		facq_capture_menu_enable_clear(cap->priv->menu);
		facq_capture_toolbar_enable_clear(cap->priv->toolbar);
		facq_stream_view_set_status(cap->priv->view,
					FACQ_STREAM_VIEW_STATUS_ERROR);
		facq_statusbar_write_msg(cap->priv->statusbar,"%s",
					local_err->message);
		g_clear_error(&local_err);
	}
	else {
		facq_stream_view_set_status(cap->priv->view,
					FACQ_STREAM_VIEW_STATUS_PLAY);
		facq_statusbar_write_msg(cap->priv->statusbar,"%s",
					_("Stream started"));
	}
}

/**
 * facq_capture_control_stop:
 * @cap: A #FacqCapture object.
 *
 * Stops the stream, stopping the data process.
 *
 * Disables the stop button in the #FacqCaptureToolbar and the
 * stop entry in the Control submenu, displays a message to the user
 * in the #FacqStatusbar telling the user that stream  is being stopped
 * and calls facq_stream_stop() stopping the #FacqStream.
 * After this the #FacqStreamView status is set to
 * %FACQ_STREAM_VIEW_STATUS_STOP, and the Play,Remove,Clear buttons
 * in the #FacqCaptureToolbar and the Play,Remove,Clear entries in the
 * #FacqCaptureMenu are enabled. Finally a message is pushed to the
 * #FacqStatusbar telling the user that the stream is stopped.
 *
 * This function is called when the user presses the stop button
 * in the #FacqCaptureToolbar or the stop entry in the #FacqCaptureMenu.
 */
void facq_capture_control_stop(FacqCapture *cap)
{
	g_return_if_fail(FACQ_IS_CAPTURE(cap));
	g_return_if_fail(FACQ_IS_STREAM(cap->priv->stream));

	facq_capture_menu_disable_stop(cap->priv->menu);
	facq_capture_toolbar_disable_stop(cap->priv->toolbar);
	facq_statusbar_write_msg(cap->priv->statusbar,"%s",
				_("Stopping, this can take a while..."));

	facq_stream_stop(cap->priv->stream);
	facq_stream_view_set_status(cap->priv->view,
					FACQ_STREAM_VIEW_STATUS_STOP);
	facq_statusbar_write_msg(cap->priv->statusbar,_("Stream stopped"));
	facq_capture_toolbar_enable_play(cap->priv->toolbar);
	facq_capture_menu_enable_play(cap->priv->menu);
	facq_capture_toolbar_enable_remove(cap->priv->toolbar);
	facq_capture_toolbar_enable_clear(cap->priv->toolbar);
	facq_capture_menu_enable_remove(cap->priv->menu);
	facq_capture_menu_enable_clear(cap->priv->menu);
	facq_capture_menu_enable_close(cap->priv->menu);
}

/**
 * facq_capture_log:
 * @cap: A #FacqCapture object.
 *
 * Display the #FacqLogWindow to the user, allowing the user to
 * view the system log.
 *
 * This function is called when the user presses the Log->Read
 * entry in the #FacqCaptureMenu.
 */
void facq_capture_log(FacqCapture *cap)
{
	GError *local_err = NULL;

	cap->priv->log_window = 
		facq_log_window_new(cap->priv->window,
				    facq_log_get_filename(),
				    1024,
				    &local_err);
	if(local_err){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
		facq_statusbar_write_msg(cap->priv->statusbar,"%s",_("Error opening log"));
		g_clear_error(&local_err);
	}
}

/**
 * facq_capture_free:
 * @cap: A #FacqCapture object.
 *
 * Destroys a no longer needed #FacqCapture object, and
 * all the contained objects, minus the referenced #FacqCatalog
 * object passed in the creation.
 *
 * This function is called after the gtk_main() function, when the
 * user presses the quit button, or closes the windows.
 */
void facq_capture_free(FacqCapture *cap)
{
	g_return_if_fail(FACQ_IS_CAPTURE(cap));
	g_object_unref(G_OBJECT(cap));
}

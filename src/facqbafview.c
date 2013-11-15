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
#include <math.h>
#include "facqi18n.h"
#include "facqlog.h"
#include "gdouble.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqchunk.h"
#include "facqfile.h"
#include "facqcolor.h"
#include "facqstatusbar.h"
#include "facqresourcesicons.h"
#include "facqlegend.h"
#include "facqbafviewmenu.h"
#include "facqbafviewtoolbar.h"
#include "facqfilechooser.h"
#include "facqbafviewplot.h"
#include "facqbafviewdialog.h"
#include "facqbafview.h"

/**
 * SECTION:facqbafview
 * @title:FacqBAFView
 * @short_description: Control class for the viewer application.
 * @include:facqbafview.h
 *
 * Implements the control class and creates the graphical user interface for the
 * capture application.
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * The viewer interface uses the following #GtkWidget objects, a toplevel #GtkWindow,
 * a #FacqBAFViewMenu, a #FacqBAFViewToolbar, a #FacqBAFViewPlot, and a #FacqLegend.
 *
 * Also internally a #FacqFile object is created when a file is opened, to
 * manage the file. Also a #FacqStreamData will be created from the file.
 * </para>
 * </sect1>
 *
 */

/**
 * FacqBAFView:
 *
 * Contains the internal details of a #FacqBAFView object.
 */

/**
 * FacqBAFViewClass:
 *
 * The class for the #FacqBAFView objects.
 */

G_DEFINE_TYPE(FacqBAFView,facq_baf_view,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_PAGE_TIME
};

struct _FacqBAFViewPrivate {
	GtkWidget *window;
	FacqBAFViewMenu *menu;
	FacqBAFViewToolbar *toolbar;
	FacqBAFViewPlot *plot;
	FacqStatusbar *statusbar;
	FacqLegend *legend;
	FacqFile *file;
	FacqStreamData *stmd;
	guint64 written_samples;
	guint samples_per_page;
	gdouble page_time;
	gdouble total_pages;
	gdouble current_page;
};

static gboolean delete_event(GtkWidget *widget,GdkEvent *event,gpointer data)
{
	gtk_main_quit();
	return FALSE;
}

static void facq_baf_view_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqBAFView *view = FACQ_BAF_VIEW(self);

	switch(property_id){
	case PROP_PAGE_TIME: view->priv->page_time = g_value_get_double(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(view,property_id,pspec);
	}
}

static void facq_baf_view_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqBAFView *view = FACQ_BAF_VIEW(self);

	switch(property_id){
	case PROP_PAGE_TIME: g_value_set_double(value,view->priv->page_time);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(view,property_id,pspec);
	}
}

static void facq_baf_view_constructed(GObject *self)
{
	FacqBAFView *view = FACQ_BAF_VIEW(self);
	GtkWidget *vbox = NULL, *vpaned = NULL, *frame = NULL;

	view->priv->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(view->priv->window),
				_("Binary Acquisition File Viewer"));
	gtk_window_set_icon(GTK_WINDOW(view->priv->window),
				facq_resources_icons_viewer());
	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(view->priv->window),vbox);

	view->priv->statusbar = facq_statusbar_new();
	view->priv->legend = facq_legend_new();
	view->priv->menu = facq_baf_view_menu_new(view);
	view->priv->toolbar = facq_baf_view_toolbar_new(view);
	view->priv->plot = facq_baf_view_plot_new();

	gtk_box_pack_start(GTK_BOX(vbox),
			facq_baf_view_menu_get_widget(view->priv->menu),FALSE,FALSE,0);

	gtk_box_pack_start(GTK_BOX(vbox),
			facq_baf_view_toolbar_get_widget(view->priv->toolbar),FALSE,FALSE,0);

	vpaned = gtk_vpaned_new();
	gtk_paned_pack1(GTK_PANED(vpaned),
			facq_baf_view_plot_get_widget(view->priv->plot),TRUE,FALSE);

	frame = gtk_frame_new(_("Color legend"));
	gtk_frame_set_label_align(GTK_FRAME(frame),0.5,0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_NONE);
	gtk_container_add(GTK_CONTAINER(frame),facq_legend_get_widget(view->priv->legend));
	gtk_paned_pack2(GTK_PANED(vpaned),frame,FALSE,TRUE);

	gtk_box_pack_start(GTK_BOX(vbox),vpaned,TRUE,TRUE,0);

	gtk_box_pack_end(GTK_BOX(vbox),
			facq_statusbar_get_widget(view->priv->statusbar),FALSE,FALSE,0);

	g_signal_connect(view->priv->window,"delete-event",
				G_CALLBACK(delete_event),NULL);

	gtk_widget_show_all(view->priv->window);
}

static void facq_baf_view_finalize(GObject *self)
{
	FacqBAFView *view = FACQ_BAF_VIEW(self);

	if(FACQ_IS_FILE(view->priv->file)){
		facq_file_free(view->priv->file);
	}

	if(FACQ_IS_STREAM_DATA(view->priv->stmd)){
		facq_stream_data_free(view->priv->stmd);
	}

	if(FACQ_IS_BAF_VIEW_MENU(view->priv->menu)){
		facq_baf_view_menu_free(view->priv->menu);
	}

	if(FACQ_IS_BAF_VIEW_TOOLBAR(view->priv->toolbar)){
		facq_baf_view_toolbar_free(view->priv->toolbar);
	}

	if(FACQ_IS_BAF_VIEW_PLOT(view->priv->plot)){
		facq_baf_view_plot_free(view->priv->plot);
	}

	if(FACQ_IS_STATUSBAR(view->priv->statusbar)){
		facq_statusbar_free(view->priv->statusbar);
	}

	if(FACQ_IS_LEGEND(view->priv->legend)){
		facq_legend_free(view->priv->legend);
	}

	if(GTK_IS_WIDGET(view->priv->window))
		gtk_widget_destroy(view->priv->window);

	G_OBJECT_CLASS(facq_baf_view_parent_class)->finalize(self);
}

static void facq_baf_view_class_init(FacqBAFViewClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqBAFViewPrivate));

	object_class->set_property = facq_baf_view_set_property;
	object_class->get_property = facq_baf_view_get_property;
	object_class->constructed = facq_baf_view_constructed;
	object_class->finalize = facq_baf_view_finalize;

	g_object_class_install_property(object_class,PROP_PAGE_TIME,
					g_param_spec_double("page-time",
						            "Page time",
							    "The time plotted in each page in seconds",
							    5.0,
							    86400, //seconds in a day
							    30.0,
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_READWRITE |
							    G_PARAM_STATIC_STRINGS));
}

static void facq_baf_view_init(FacqBAFView *view)
{
	view->priv = G_TYPE_INSTANCE_GET_PRIVATE(view,FACQ_TYPE_BAF_VIEW,FacqBAFViewPrivate);
	view->priv->written_samples = 0;
}

/**
 * facq_baf_view_new:
 * @time_per_page: The default time per page.
 *
 * Creates a new control #FacqBAFView object. This function is used
 * when the viewer application is started.
 *
 * Returns: A new created #FacqBAFView object.
 */
FacqBAFView *facq_baf_view_new(gdouble time_per_page)
{
	return g_object_new(FACQ_TYPE_BAF_VIEW,"page-time",time_per_page,NULL);
}

/**
 * facq_baf_view_get_widget:
 * @view: A #FacqBAFView object.
 *
 * Gets the top level widget of the user interface of the viewer application,
 * so you can add it to your application.
 *
 * Returns: A #GtkWidget pointing to the top level widget of the
 * user interface, that is the top level #GtkWindow.
 */
GtkWidget *facq_baf_view_get_widget(const FacqBAFView *view)
{
	g_return_val_if_fail(FACQ_IS_BAF_VIEW(view),NULL);
	return view->priv->window;
}

/**
 * facq_baf_view_setup_page_time:
 * @view: A #FacqBAFView object.
 *
 * Setups the time per page, showing to the user a #FacqBAFViewDialog,
 * if the user presses the OK button, the time per page value is extracted
 * from the #FacqBAFViewDialog, and the page_time attribute is set.
 * Finally the #FacqBAFViewDialog is destroyed.
 *
 * This function it's called when the user presses the page setup button in
 * the #FacqBAFViewToolbar, or the user presses the Page->Page setup entry in
 * the #FacqBAFViewMenu.
 */
void facq_baf_view_setup_page_time(FacqBAFView *view)
{
	FacqBAFViewDialog *dialog = NULL;

	dialog = facq_baf_view_dialog_new(view->priv->window);
	if( facq_baf_view_dialog_run(dialog) == GTK_RESPONSE_OK){
		view->priv->page_time = facq_baf_view_dialog_get_input(dialog);
		facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
				"setting page time to %f seconds",view->priv->page_time);
	}
	facq_baf_view_dialog_free(dialog);
}

/**
 * facq_baf_view_open_file:
 * @view: A #FacqBAFView object.
 *
 * Opens an existing binary acquisition file.
 *
 * The application first creates a new #FacqFileChooser with
 * facq_file_chooser_new(), and the dialog is showed to the user
 * with facq_file_chooser_run_dialog() and the user can choose 
 * a binary acquisition file or cancel the operation.
 * If the user presses the Open button in the dialog, the function
 * tries to retrieve the filename from the dialog, if successful
 * a message is written to the #FacqStatusbar with the name of the file.
 * In the next step the file is verified with the facq_file_verify() 
 * procedure, if the function fails the operation is canceled, else 
 * a new #FacqFile is created with the facq_file_open() function.
 * The #FacqFile object is used for creating a new #FacqStreamData with
 * the facq_file_read_header() function, and then the facq_file_read_tail()
 * function is called for obtaining the number of total samples written to the
 * file. Then the #FacqStreamData object is used to fill the information in
 * the #FacqLegend object with the facq_legend_set_data() function.
 *
 * After this step the number of samples per page is calculated using
 * and stored in the #FacqBAFView:
 * samples_per_page = page_time/period;
 * 
 * The number of total pages is calculated with:
 * total_pages = floor( (written_samples/n_channels) / samples_per_page)
 * if(fmod(written_samples,total_samples*samples_per_page))
 *	total_pages++;
 * 
 * Finally the plot is setup, with the facq_baf_view_plot_setup() function.
 * The number of pages in the #FacqBAFViewMenu is set with the
 * facq_baf_view_menu_set_total_pages() and the same for the #FacqBAFViewToolbar
 * with the facq_baf_view_toolbar_set_total_pages().
 * The current page is set to page 0, the first page is displayed using the
 * facq_baf_view_plot_page(), and the close and export entries in the
 * #FacqBAFViewMenu are enabled.
 * 
 *
 * This function it's called when the user presses the File->Open entry in the
 * #FacqBAFViewMenu.
 */
void facq_baf_view_open_file(FacqBAFView *view)
{
	FacqFileChooser *chooser = NULL;
	gchar *utf8_filename = NULL, *local_filename = NULL;
	guint samples_per_page = 0;
	gdouble total_pages = 0;
	GError *local_err = NULL;
	guint8 *digest = NULL;

	g_return_if_fail(FACQ_IS_BAF_VIEW(view));

	chooser = facq_file_chooser_new(view->priv->window,
					FACQ_FILE_CHOOSER_DIALOG_TYPE_LOAD,
					"baf",
					_("Binary Adquisition File"));
	if( facq_file_chooser_run_dialog(chooser) == GTK_RESPONSE_ACCEPT){
		utf8_filename = facq_file_chooser_get_filename_for_display(chooser);
		if(!utf8_filename){
			goto exit;
		}
		else {
			facq_statusbar_write_msg(view->priv->statusbar,
							_("Opening %s"),utf8_filename);
			g_free(utf8_filename);
			local_filename = facq_file_chooser_get_filename_for_system(chooser);
			if(!facq_file_verify(local_filename,&local_err)){
				if(local_err){
					facq_statusbar_write_msg(view->priv->statusbar,
							_("Error verifying file: %s"),local_err->message);
					g_clear_error(&local_err);
					goto exit;
				}
				else {
					facq_statusbar_write_msg(view->priv->statusbar,"%s",
							_("The file isn't valid"));
					goto exit;
				}
			}
			view->priv->file = facq_file_open(local_filename,&local_err);
			g_free(local_filename);
			if(local_err){
				view->priv->file = NULL;
				facq_statusbar_write_msg(view->priv->statusbar,
							_("Error opening file: %s"),local_err->message);
				g_clear_error(&local_err);
				goto exit;
			}
			view->priv->stmd = facq_file_read_header(view->priv->file,&local_err);
			if(!view->priv->stmd){
				if(local_err){
					facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
							_("Error reading header: %s"),local_err->message);
					g_clear_error(&local_err);
				}
				facq_statusbar_write_msg(view->priv->statusbar,"%s",_("Error reading file"));
				facq_file_free(view->priv->file);
				view->priv->file = NULL;
				goto exit;
			}
			digest = facq_file_read_tail(view->priv->file,&view->priv->written_samples,&local_err);
			g_free(digest);
			if(local_err){
				facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
						_("Error reading tail: %s"),local_err->message);
				g_clear_error(&local_err);
				facq_statusbar_write_msg(view->priv->statusbar,_("Error reading file"));
				facq_file_free(view->priv->file);
				view->priv->file = NULL;
				facq_stream_data_free(view->priv->stmd);
				view->priv->stmd = NULL;
				goto exit;
			}
			facq_legend_set_data(view->priv->legend,view->priv->stmd);
			facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
					"period: %.9g written_samples: %lu n_channels: %u",
						view->priv->stmd->period,
							view->priv->written_samples,
								view->priv->stmd->n_channels);
			samples_per_page = view->priv->page_time/view->priv->stmd->period;
			total_pages = floor( (view->priv->written_samples/view->priv->stmd->n_channels) 
									/ samples_per_page );
			if(fmod(view->priv->written_samples,((total_pages)*samples_per_page)))
				total_pages++;

			facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
					"Total pages %f and %u samples per page",
								total_pages,samples_per_page);
			view->priv->samples_per_page = samples_per_page;
			facq_baf_view_plot_setup(view->priv->plot,
							samples_per_page,
								view->priv->stmd->period,
									view->priv->stmd->n_channels);
			facq_baf_view_menu_set_total_pages(view->priv->menu,total_pages);
			facq_baf_view_toolbar_set_total_pages(view->priv->toolbar,total_pages);
			view->priv->total_pages = total_pages;
			view->priv->current_page = 0;
			facq_baf_view_plot_page(view,1);
			facq_baf_view_menu_enable_close(view->priv->menu);
			facq_baf_view_menu_enable_save_as(view->priv->menu);
		}
	}
	exit:
	facq_file_chooser_free(chooser);
}

/**
 * facq_baf_view_export_file:
 * @view: A #FacqBAFView object.
 *
 * This function exports a binary adquisition file to a plain text file,
 * where each sample appears in a column, according to the number of channels
 * (The samples are interleaved in the file). Keep in mind that a binary
 * acquisition file has been previously loaded.
 *
 * The function first creates a #FacqFileChooser object with the
 * facq_file_chooser_new() function for saving a plain text file.
 * Then the facq_file_chooser_run_dialog() function is called.
 * The #FacqFileChooser dialog is shown to the user, that will have the option
 * to choose a filename to save the export, or cancel the operation. If the
 * user accepts the operation the export filename is retrieved from the dialog,
 * the filename is retrieved from the #FacqFile with facq_file_get_filename(),
 * and the funcion facq_file_to_human() is called for exporting the file.
 * The user is informed trough a message in the #FacqStatusbar of the status
 * of the operation.
 *
 * This function is called when the user presses the File->Export entry in the
 * #FacqBAFViewMenu.
 */
void facq_baf_view_export_file(FacqBAFView *view)
{
	FacqFileChooser *chooser = NULL;
	gchar *utf8_dst_filename = NULL;
	gchar *local_dst_filename = NULL;
	gchar *local_src_filename = NULL;
	GError *local_err = NULL;
	gboolean ret = FALSE;

	g_return_if_fail(FACQ_IS_BAF_VIEW(view));
	g_return_if_fail(FACQ_IS_FILE(view->priv->file));

	chooser = facq_file_chooser_new(view->priv->window,
					FACQ_FILE_CHOOSER_DIALOG_TYPE_SAVE,
					"txt",
					_("Plain Text File"));
	if( facq_file_chooser_run_dialog(chooser) == GTK_RESPONSE_ACCEPT){
		local_dst_filename = facq_file_chooser_get_filename_for_system(chooser);
		if(!local_dst_filename)
			goto exit;
		local_src_filename = facq_file_get_filename(view->priv->file);
		if(!local_src_filename){
			g_free(local_dst_filename);
			goto exit;
		}
		ret = facq_file_to_human(local_src_filename,local_dst_filename,&local_err);
		g_free(local_src_filename);
		g_free(local_dst_filename);
		if(!ret){
			if(local_err){
				facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,
						_("Error exporting file: %s"),local_err->message);
				g_clear_error(&local_err);
			}
			facq_statusbar_write_msg(view->priv->statusbar,_("Error exporting file"));
		}
		else {
			utf8_dst_filename = facq_file_chooser_get_filename_for_display(chooser);
			facq_statusbar_write_msg(view->priv->statusbar,
					_("Successful export to %s"),utf8_dst_filename);
			g_free(utf8_dst_filename);
		}
	}
	exit:
	facq_file_chooser_free(chooser);
}

/**
 * facq_baf_view_close_file:
 * @view: A #FacqBAFView object.
 *
 * Closes a previously opened binary acquisition file.
 *
 * The function frees the #FacqFile and the #FacqStreamData, created
 * when the file is opened, set the number of written_samples to 0
 * disables the navigation buttons in the toolbar with
 * facq_baf_view_toolbar_disable_navigation(), and the navigation entries
 * in the menu with the facq_baf_view_menu_disable_navigation(). Finally
 * the close and save as entries are disabled, the legend is cleared with
 * facq_legend_clear_data(), the number of total pages in the toolbar
 * is set to 1 with facq_baf_view_toolbar_set_total_pages(), and the
 * plot is cleared with facq_baf_view_plot_clear().
 * The functions ends writing a message to the #FacqStatusbar telling
 * the user that the file is closed.
 *
 * This function is called when the user presses the File->Close
 * menu entry.
 */
void facq_baf_view_close_file(FacqBAFView *view)
{
	g_return_if_fail(FACQ_IS_BAF_VIEW(view));

	if(FACQ_IS_FILE(view->priv->file)){
		facq_file_free(view->priv->file);
		view->priv->file = NULL;
	}
	if(FACQ_IS_STREAM_DATA(view->priv->stmd)){
		facq_stream_data_free(view->priv->stmd);
		view->priv->stmd = NULL;
	}
	view->priv->written_samples = 0;
	facq_baf_view_toolbar_disable_navigation(view->priv->toolbar);
	facq_baf_view_menu_disable_navigation(view->priv->menu);
	facq_baf_view_menu_disable_close(view->priv->menu);
	facq_baf_view_menu_disable_save_as(view->priv->menu);
	facq_legend_clear_data(view->priv->legend);
	facq_baf_view_toolbar_set_total_pages(view->priv->toolbar,1);
	facq_baf_view_plot_clear(view->priv->plot);
	facq_statusbar_write_msg(view->priv->statusbar,"%s",_("File closed"));
}

static void iter_caller(gpointer data,gdouble *chunk)
{
	FacqBAFViewPlot *plot = FACQ_BAF_VIEW_PLOT(data);

	facq_baf_view_plot_push_chunk(plot,chunk);
}

/**
 * facq_baf_view_plot_page:
 * @view: A #FacqBAFView object.
 * @page: The number of page between 1 and total pages.
 *
 * Plots the page, @page, in the #FacqBAFViewPlot.
 *
 * The function first set the current_page to the page specified in @page.
 * Then the navigation is disabled in the toolbar and in the menu with the
 * facq_baf_view_menu_disable_navigation() and
 * facq_baf_view_toolbar_disable_navigation().
 *
 * After this the function calculates the start slice, and the number of slices
 * for this page, and the function facq_baf_view_plot_push_chunk() is called
 * for each slice. Finally the facq_baf_view_plot_draw_page() function is
 * called, and the number of page is set into the toolbar and into the menu with
 * facq_baf_view_menu_goto_page() and facq_baf_view_toolbar_goto_page().
 *
 * This function is called by the facq_baf_view_plot_page_spin(),
 * facq_baf_view_plot_first_page(), facq_baf_view_plot_prev_page(),
 * facq_baf_view_plot_next_page(), facq_baf_view_plot_last_page().
 */
void facq_baf_view_plot_page(FacqBAFView *view,gdouble page)
{
	guint64 start = 0;
	guint64 chunks = 0;
	GError *local_err = NULL;

	g_return_if_fail(FACQ_IS_BAF_VIEW(view));
	g_return_if_fail(FACQ_IS_FILE(view->priv->file));
	g_return_if_fail(page > 0 && page <= view->priv->total_pages);
	
	if(view->priv->current_page == page)
		return;
	
	view->priv->current_page = page;
	facq_baf_view_menu_disable_navigation(view->priv->menu);
	facq_baf_view_toolbar_disable_navigation(view->priv->toolbar);

	start = (page-1)*view->priv->samples_per_page;
	chunks = page*view->priv->samples_per_page;

	facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
			"Loading chunks from %lu to %lu",start,chunks);

	facq_file_chunk_iterator(view->priv->file,
					start,chunks,
						iter_caller,
							view->priv->plot,&local_err);
	
	facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
				"%s","Redrawing page\n");

	facq_baf_view_plot_draw_page(view->priv->plot,page);
	facq_baf_view_menu_goto_page(view->priv->menu,page);
	facq_baf_view_toolbar_goto_page(view->priv->toolbar,page);

	if(local_err){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"Error drawing page: %s",local_err->message);
		g_clear_error(&local_err);
	}
}

/**
 * facq_baf_view_plot_page_spin:
 * @view: A #FacqBAFView object.
 *
 * Reads the value of the spin button, and plots the page, referenced
 * by the page number.
 *
 * This function is called by the facq_baf_view_toolbar_callback_intro()
 * when the user presses the intro button in the spin button that shows the
 * number of page plotted.
 */
void facq_baf_view_plot_page_spin(FacqBAFView *view)
{
	gdouble page_n = 1;

	g_return_if_fail(FACQ_IS_BAF_VIEW(view));
	page_n = facq_baf_view_toolbar_read_spin_button(view->priv->toolbar);
	facq_baf_view_plot_page(view,page_n);
}

/**
 * facq_baf_view_plot_first_page:
 * @view: A #FacqBAFView object.
 *
 * Plots the first page in the #FacqBAFViewPlot, calling the
 * facq_baf_view_plot_page().
 *
 * This function is called when the user presses the First page button in the
 * toolbar, or the Page->First in the menu. The callback functions 
 * facq_baf_view_toolbar_callback_goto_first() and
 * facq_baf_view_menu_callback_goto_first()
 * are called when the button are pressed, and the callbacks call this function.
 */
void facq_baf_view_plot_first_page(FacqBAFView *view)
{
	g_return_if_fail(FACQ_IS_BAF_VIEW(view));

	facq_baf_view_plot_page(view,1);
}

/**
 * facq_baf_view_plot_prev_page:
 * @view: A #FacqBAFView object.
 * 
 * Plots the previous page in the #FacqBAFViewPlot, calling the
 * facq_baf_view_plot_page().
 *
 * This function is called when the user presses the Prev page button in the
 * toolbar, or the Page->Prev in the menu. The callback functions
 * facq_baf_view_menu_callback_go_back() and facq_baf_view_toolbar_callback_go_back()
 * are called when the buttons are pressed, and this callbacks call this
 * function.
 */
void facq_baf_view_plot_prev_page(FacqBAFView *view)
{
	gdouble page = 1;

	g_return_if_fail(FACQ_IS_BAF_VIEW(view));
	g_return_if_fail(view->priv->current_page != 1);
	
	page = view->priv->current_page-1;
	facq_baf_view_plot_page(view,page);
}

/**
 * facq_baf_view_plot_next_page:
 * @view: A #FacqBAFView object.
 *
 * Plots the next page in the #FacqBAFViewPlot, calling the
 * facq_baf_view_plot_page() function, after computing the page number.
 *
 * This function is called by the
 * facq_baf_view_toolbar_callback_go_forward() and 
 * facq_baf_view_menu_callback_go_forward() callbacks when the next
 * page button in the toolbar, or the next entry in the menu is pressed.
 */
void facq_baf_view_plot_next_page(FacqBAFView *view)
{
	gdouble page = 1;

	g_return_if_fail(FACQ_IS_BAF_VIEW(view));
	g_return_if_fail(view->priv->current_page != view->priv->total_pages);

	page = view->priv->current_page+1;
	facq_baf_view_plot_page(view,page);
}

/**
 * facq_baf_view_plot_last_page:
 * @view: A #FacqBAFView object.
 *
 * Plots the last page in the #FacqBAFViewPlot, calling the
 * facq_baf_view_plot_page().
 *
 * This function is called when the user presses the last page button
 * or the last entry in the menu. The callback functions
 * facq_baf_view_menu_callback_goto_last() and
 * facq_baf_view_toolbar_callback_goto_last() are called when the buttons
 * are pressed, and the callbacks call this function.
 */
void facq_baf_view_plot_last_page(FacqBAFView *view)
{
	gdouble page = 1;

	g_return_if_fail(FACQ_IS_BAF_VIEW(view));

	page = view->priv->total_pages;
	facq_baf_view_plot_page(view,page);
}

/**
 * facq_baf_view_zoom_in:
 * @view: A #FacqBAFView object.
 *
 * Zooms the view in the #FacqBAFViewPlot, calling the 
 * facq_baf_view_plot_zoom_in().
 *
 * facq_baf_view_menu_callback_zoom_in() and
 * facq_baf_view_toolbar_callback_zoom_in() are called when the buttons
 * are pressed, and the callbacks call this function.
 */
void facq_baf_view_zoom_in(FacqBAFView *view)
{
	facq_baf_view_plot_zoom_in(view->priv->plot);
}

/**
 * facq_baf_view_zoom_out:
 * @view: A #FacqBAFView object.
 *
 * Zooms the view out the #FacqBAFViewPlot, calling the 
 * facq_baf_view_plot_zoom_out().
 *
 * facq_baf_view_menu_callback_zoom_out() and
 * facq_baf_view_toolbar_callback_zoom_out() are called when the buttons
 * are pressed, and the callbacks call this function.
 */
void facq_baf_view_zoom_out(FacqBAFView *view)
{
	facq_baf_view_plot_zoom_out(view->priv->plot);
}

/**
 * facq_baf_view_zoom_fit:
 * @view: A #FacqBAFView object.
 *
 * Returns to the normal view the #FacqBAFViewPlot, calling the 
 * facq_baf_view_plot_zoom_home().
 *
 * facq_baf_view_menu_callback_zoom_100() and
 * facq_baf_view_toolbar_callback_zoom_100() are called when the buttons
 * are pressed, and the callbacks call this function.
 */
void facq_baf_view_zoom_fit(FacqBAFView *view)
{
	facq_baf_view_plot_zoom_home(view->priv->plot);
}

/**
 * facq_baf_view_free:
 * @view: A #FacqBAFView object.
 *
 * Destroys a no longer needed #FacqBAFView object and all
 * the contained elements.
 */
void facq_baf_view_free(FacqBAFView *view)
{
	g_return_if_fail(FACQ_IS_BAF_VIEW(view));
	g_object_unref(G_OBJECT(view));
}

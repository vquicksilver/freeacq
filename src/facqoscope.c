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
#include "facqresourcesicons.h"
#include "facqmisc.h"
#include "facqchunk.h"
#include "facqlog.h"
#include "gdouble.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqnet.h"
#include "facqplug.h"
#include "facqplugdialog.h"
#include "facqcolor.h"
#include "facqstatusbar.h"
#include "facqoscopetoolbar.h"
#include "facqoscopemenu.h"
#include "facqoscopeplot.h"
#include "facqlegend.h"
#include "facqoscope.h"

/**
 * SECTION:facqoscope
 * @title:FacqOscope
 * @short_description: Control class for the oscilloscope application
 * @include:facqoscope.h
 *
 * Provides the control class for the oscilloscope application.
 * This class controls all the other objects involved in the operation
 * of the oscilloscope.
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * This class stores the default address and port, and create the
 * user interface with the following objects, a #FacqOscopeMenu,
 * a #FacqOscopeToolbar, a #FacqOscopePlot, a #FacqLegend and
 * a #FacqStatusbar.
 *
 * For receiving the data from the capture application, a #FacqPlug object is
 * used, this class implement the connected and disconnected callbacks of this
 * class.
 * </para>
 * </sect1>
 */
static void facq_oscope_initable_iface_init(GInitableIface  *iface);
static gboolean facq_oscope_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);

G_DEFINE_TYPE_WITH_CODE(FacqOscope,facq_oscope,G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_oscope_initable_iface_init));

enum {
	PROP_0,
	PROP_ADDRESS,
	PROP_PORT
};

struct _FacqOscopePrivate {
	gchar *address;
	guint16 port;
	GtkWidget *window;
	FacqOscopeMenu *menu;
	FacqOscopeToolbar *toolbar;
	FacqOscopePlot *plot;
	FacqLegend *legend;
	FacqStatusbar *statusbar;
	FacqPlug *plug;
	GError *construct_error;
};

/*****--- callbacks ---*****/
static gboolean delete_event(GtkWidget *widget,GdkEvent *event,gpointer data)
{
	gtk_main_quit();
	return FALSE;
}

/* this callback is called each time a client connects, and the #FacqPlug
 * object emits the connected signal */
static void connected_callback(FacqPlug *plug,gpointer _oscope)
{
	FacqOscope *oscope = FACQ_OSCOPE(_oscope);
	FacqStreamData *stmd = NULL;
	gchar *address = NULL;
        GError *local_err = NULL;

        address = facq_plug_get_client_address(plug,&local_err);
        if(local_err){
                facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
		g_clear_error(&local_err);
		return;
        }
	else if(address){
		/* get the stream data */
		stmd = facq_plug_get_stream_data(plug);
		if(!stmd)
			return;

		/* setup the plot */
		facq_oscope_plot_setup(oscope->priv->plot,
				       stmd->period,
				       stmd->n_channels,
				       &local_err);
		if(local_err){
			g_free(address);
			g_object_unref(G_OBJECT(stmd));
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
			g_clear_error(&local_err);
			facq_statusbar_write_msg(oscope->priv->statusbar,"%s","Client sent wrong data");
			facq_plug_disconnect(plug);
			return;
		}

		/* update the legend widget */
		facq_legend_set_data(oscope->priv->legend,stmd);

		/* unref the stream data */
		g_object_unref(G_OBJECT(stmd));

		/* set the gui widgets according to the new connected status */
		facq_oscope_toolbar_disable_preferences(oscope->priv->toolbar);
		facq_oscope_menu_disable_preferences(oscope->priv->menu);
		facq_oscope_toolbar_enable_disconnect(oscope->priv->toolbar);
		facq_oscope_menu_enable_disconnect(oscope->priv->menu);
		facq_oscope_plot_set_zoom(oscope->priv->plot,FALSE);
		facq_oscope_toolbar_disable_zoom_in(oscope->priv->toolbar);
		facq_oscope_menu_disable_zoom_in(oscope->priv->menu);
		facq_oscope_toolbar_disable_zoom_out(oscope->priv->toolbar);
		facq_oscope_menu_disable_zoom_out(oscope->priv->menu);
		facq_oscope_toolbar_disable_zoom_home(oscope->priv->toolbar);
		facq_oscope_menu_disable_zoom_home(oscope->priv->menu);
		facq_statusbar_write_msg(oscope->priv->statusbar,
					_("New client connected from %s"),address);
		g_free(address);
	}
}

/* this callback is called each time a client disconnects */
static void disconnected_callback(FacqPlug *plug,gpointer _oscope)
{
	FacqOscope *oscope = FACQ_OSCOPE(_oscope);

	facq_oscope_toolbar_disable_disconnect(oscope->priv->toolbar);
	facq_oscope_menu_disable_disconnect(oscope->priv->menu);
	facq_oscope_toolbar_enable_preferences(oscope->priv->toolbar);
	facq_oscope_menu_enable_preferences(oscope->priv->menu);
	facq_oscope_plot_set_zoom(oscope->priv->plot,TRUE);
	facq_oscope_toolbar_enable_zoom_in(oscope->priv->toolbar);
	facq_oscope_menu_enable_zoom_in(oscope->priv->menu);
	facq_oscope_toolbar_enable_zoom_out(oscope->priv->toolbar);
	facq_oscope_menu_enable_zoom_out(oscope->priv->menu);
	facq_oscope_toolbar_enable_zoom_home(oscope->priv->toolbar);
	facq_oscope_menu_enable_zoom_home(oscope->priv->menu);
	facq_statusbar_write_msg(oscope->priv->statusbar,_("Client disconnected"));
}

/* this is the data callback for the #FacqPlug, this function is 
 * called by the main thread, from time to time, when the main thread is idle
 * allowing us to process the data here */
static gboolean data_callback(FacqChunk *chunk,gpointer _oscope)
{
	FacqOscope *oscope = FACQ_OSCOPE(_oscope);

	g_return_val_if_fail(FACQ_IS_CHUNK(chunk),FALSE);
	g_return_val_if_fail(FACQ_IS_OSCOPE(oscope),FALSE);

	/* just plot the chunk */
#if ENABLE_DEBUG
	facq_log_write("Oscope processing chunk",FACQ_LOG_MSG_TYPE_DEBUG);
#endif

	facq_oscope_plot_process_chunk(oscope->priv->plot,chunk);

	return TRUE;
}

/*****--- GObject magic ---*****/
static void facq_oscope_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqOscope *oscope = FACQ_OSCOPE(self);

	switch(property_id){
	case PROP_ADDRESS: oscope->priv->address = g_value_dup_string(value);
	break;
	case PROP_PORT: oscope->priv->port = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(oscope,property_id,pspec);
	}
}

static void facq_oscope_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqOscope *oscope = FACQ_OSCOPE(self);

	switch(property_id){
	case PROP_ADDRESS: g_value_set_string(value,oscope->priv->address);
	break;
	case PROP_PORT: g_value_set_uint(value,oscope->priv->port);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(oscope,property_id,pspec);
	}
}

static void facq_oscope_finalize(GObject *self)
{
	FacqOscope *oscope = FACQ_OSCOPE(self);

	if(oscope->priv->construct_error)
		g_clear_error(&oscope->priv->construct_error);

	if(oscope->priv->address)
		g_free(oscope->priv->address);

	if(FACQ_IS_PLUG(oscope->priv->plug)){
		facq_plug_free(oscope->priv->plug);
	}

	if(FACQ_IS_OSCOPE_MENU(oscope->priv->menu))
		facq_oscope_menu_free(oscope->priv->menu);

	if(FACQ_IS_OSCOPE_TOOLBAR(oscope->priv->toolbar))
		facq_oscope_toolbar_free(oscope->priv->toolbar);

	if(FACQ_IS_OSCOPE_PLOT(oscope->priv->plot))
		facq_oscope_plot_free(oscope->priv->plot);
	
	if(FACQ_IS_LEGEND(oscope->priv->legend))
		facq_legend_free(oscope->priv->legend);

	if(FACQ_IS_STATUSBAR(oscope->priv->statusbar));
		facq_statusbar_free(oscope->priv->statusbar);

	if(GTK_IS_WIDGET(oscope->priv->window))
		gtk_widget_destroy(oscope->priv->window);

	G_OBJECT_CLASS(facq_oscope_parent_class)->finalize(self);
}

static void facq_oscope_constructed(GObject *self)
{
	FacqOscope *oscope = FACQ_OSCOPE(self);
	GdkPixbuf *icon = NULL;
	GtkWidget *vbox = NULL;
	GtkWidget *vpaned = NULL;
	GtkWidget *frame = NULL;
	GError *local_err = NULL;

	oscope->priv->plug = 
		facq_plug_new(oscope->priv->address,
			      oscope->priv->port,
			      data_callback,
			      oscope,
			      100,
			      &local_err);

	if(local_err){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",
					local_err->message);
		g_propagate_error(&oscope->priv->construct_error,local_err);
		return;
	}

	oscope->priv->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(oscope->priv->window),_("Oscilloscope"));
	icon = facq_resources_icons_oscope();
	gtk_window_set_icon(GTK_WINDOW(oscope->priv->window),icon);
	g_object_unref(icon);
	oscope->priv->menu = facq_oscope_menu_new(oscope);
	oscope->priv->toolbar = facq_oscope_toolbar_new(oscope);
	oscope->priv->plot = facq_oscope_plot_new();
	oscope->priv->legend = facq_legend_new();
	oscope->priv->statusbar = facq_statusbar_new();

	/* connect the signals to the callbacks */
	g_signal_connect(oscope->priv->plug,"connected",
			G_CALLBACK(connected_callback),oscope);

	g_signal_connect(oscope->priv->plug,"disconnected",
			G_CALLBACK(disconnected_callback),oscope);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(oscope->priv->window),vbox);

	gtk_box_pack_start(GTK_BOX(vbox),
				facq_oscope_menu_get_widget(oscope->priv->menu),
									FALSE,FALSE,0);

	gtk_box_pack_start(GTK_BOX(vbox),
				facq_oscope_toolbar_get_widget(oscope->priv->toolbar),
								        FALSE,FALSE,0);
	vpaned = gtk_vpaned_new();
	gtk_widget_set_size_request(vpaned,256,-1);
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_NONE);
	gtk_container_add(GTK_CONTAINER(frame),facq_oscope_plot_get_widget(oscope->priv->plot));
	gtk_paned_pack1(GTK_PANED(vpaned),frame,TRUE,FALSE);
	gtk_widget_set_size_request(frame,200,-1);

	frame = gtk_frame_new("Color legend");
	gtk_frame_set_label_align(GTK_FRAME(frame),0.5,0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_NONE);
	gtk_widget_set_size_request(frame,50,-1);
	gtk_container_add(GTK_CONTAINER(frame),facq_legend_get_widget(oscope->priv->legend));
	gtk_paned_pack2(GTK_PANED(vpaned),frame,FALSE,TRUE);
	
	gtk_box_pack_start(GTK_BOX(vbox),vpaned,TRUE,TRUE,0);

	gtk_box_pack_end(GTK_BOX(vbox),
				facq_statusbar_get_widget(oscope->priv->statusbar),
									FALSE,FALSE,0);

	g_signal_connect(oscope->priv->window,"delete-event",
			G_CALLBACK(delete_event),NULL);
	
	if(oscope->priv->address)
		facq_statusbar_write_msg(oscope->priv->statusbar,
						_("Listening on %s:%u"),
						oscope->priv->address,oscope->priv->port);
	else
		facq_statusbar_write_msg(oscope->priv->statusbar,
						_("Listening on all:%u"),
							oscope->priv->port);
	gtk_widget_show(oscope->priv->window);
}

static void facq_oscope_class_init(FacqOscopeClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqOscopePrivate));

	object_class->set_property = facq_oscope_set_property;
	object_class->get_property = facq_oscope_get_property;
	object_class->constructed = facq_oscope_constructed;
	object_class->finalize = facq_oscope_finalize;

	g_object_class_install_property(object_class,PROP_ADDRESS,
					g_param_spec_string("address",
                                                            "The local address",
                                                            "The listened local address",
                                                            "127.0.0.1",
                                                            G_PARAM_CONSTRUCT_ONLY |
                                                            G_PARAM_READWRITE |
                                                            G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_PORT,
                                        g_param_spec_uint("port",
                                                          "The port",
                                                          "The VI port",
                                                          0,
                                                          65535,
                                                          3000,
                                                          G_PARAM_READWRITE |
                                                          G_PARAM_CONSTRUCT_ONLY |
                                                          G_PARAM_STATIC_STRINGS));
}

static void facq_oscope_init(FacqOscope *oscope)
{
	oscope->priv = G_TYPE_INSTANCE_GET_PRIVATE(oscope,FACQ_TYPE_OSCOPE,FacqOscopePrivate);
	oscope->priv->address = NULL;
	oscope->priv->port = 3000;
	oscope->priv->window = NULL;
	oscope->priv->menu = NULL;
	oscope->priv->toolbar = NULL;
	oscope->priv->plot = NULL;
	oscope->priv->statusbar = NULL;
	oscope->priv->plug = NULL;
}

/*****--- GInitable interface ---*****/
static void facq_oscope_initable_iface_init(GInitableIface *iface)
{
	iface->init = facq_oscope_initable_init;
}

static gboolean facq_oscope_initable_init(GInitable *initable,GCancellable *cancellable,GError **error)
{
	FacqOscope *oscope;

	g_return_val_if_fail(FACQ_IS_OSCOPE(initable),FALSE);
	oscope = FACQ_OSCOPE(initable);
	if(cancellable != NULL){
                g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                           "Cancellable initialization not supported");
                return FALSE;
        }
        if(oscope->priv->construct_error){
                if(error)
                        *error = g_error_copy(oscope->priv->construct_error);
                return FALSE;
        }
        return TRUE;
}

/**
 * facq_oscope_new:
 * @address: The default ip address.
 * @port: The default port to listen on.
 * @err: A #GError it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqOscope object that will listen for connections
 * on @address:@port.
 *
 * Returns: A new #FacqOscope object.
 */
FacqOscope *facq_oscope_new(const gchar *address,guint16 port,GError **err)
{
	return FACQ_OSCOPE(g_initable_new(FACQ_TYPE_OSCOPE,
					  NULL,err,
					  "address",address,
					  "port",port,
					  NULL));
}

/**
 * facq_oscope_get_widget:
 * @oscope: A #FacqOscope object.
 *
 * Gets the toplevel widget of the oscilloscope application.
 *
 * Returns: A #GtkWidget pointing to the top level window of the oscilloscope
 * application.
 */
GtkWidget *facq_oscope_get_widget(const FacqOscope *oscope)
{
	g_return_val_if_fail(FACQ_IS_OSCOPE(oscope),NULL);
	return oscope->priv->window;
}

/**
 * facq_oscope_disconnect:
 * @oscope: A #FacqOscope object.
 *
 * Calls the facq_plug_disconnect() function on the internal #FacqPlug object
 * contained in the #FacqOscope, disconnecting any connected client if any.
 *
 * This function is called when the user presses the disconnect button in the
 * #FacqOscopeToolbar or the Disconnect entry in the #FacqOscopeMenu.
 */
void facq_oscope_disconnect(FacqOscope *oscope)
{
	g_return_if_fail(FACQ_IS_OSCOPE(oscope));

	facq_plug_disconnect(oscope->priv->plug);
}

/**
 * facq_oscope_set_listen_address:
 * @oscope: A #FacqOscope object.
 *
 * Allows the user to change the default listen address and port of the
 * oscilloscope. For doing this the function creates a #FacqPlugDialog
 * with the facq_plug_dialog_new() function and displays it to the
 * user with the facq_plug_dialog_run() function, if the user accepts the
 * changes in the dialog, the function facq_plug_set_listen_address()
 * is called when the new parameters, and the status of the operation is
 * displayed in the #FacqStatusbar.
 *
 * This function is called when the user presses the preferences button
 * in the #FacqOscopeToolbar, or the preferences entry in the #FacqOscopeMenu.
 */
void facq_oscope_set_listen_address(FacqOscope *oscope)
{
	FacqPlug *plug = NULL;
	FacqStatusbar *statusbar = NULL;
	gchar *address = NULL;
	guint16 port = 0;
	GError *local_err = NULL;
	FacqPlugDialog *dialog = NULL;

	plug = oscope->priv->plug;
	address = facq_plug_get_address(plug);
	port = facq_plug_get_port(plug);
	statusbar = oscope->priv->statusbar;
	dialog = facq_plug_dialog_new(facq_oscope_get_widget(oscope),address,port);
	g_free(address);
	
	if(facq_plug_dialog_run(dialog) == GTK_RESPONSE_OK){
		address = facq_plug_dialog_get_input(dialog,&port);
		facq_plug_set_listen_address(plug,address,port,&local_err);
		if(address)
			g_free(address);
		if(local_err){
			facq_statusbar_write_msg(statusbar,
					_("Error %s"),local_err->message);
			g_clear_error(&local_err);
		}
		else {
			address = facq_plug_get_address(plug);
			port = facq_plug_get_port(plug);
			facq_statusbar_write_msg(statusbar,
					_("Listening on %s:%u"),address,port);
			g_free(address);
		}
	}
	facq_plug_dialog_free(dialog);
}

/**
 * facq_oscope_zoom_in:
 * @oscope: A #FacqOscope object.
 *
 * Zooms in the view on the #FacqOscopePlot object.
 *
 * This function is called each time the user presses the zoom
 * in button in the toolbar or the entry in the menu.
 */
void facq_oscope_zoom_in(FacqOscope *oscope)
{
	facq_oscope_plot_zoom_in(oscope->priv->plot);
}

/**
 * facq_oscope_zoom_out:
 * @oscope: A #FacqOscope object.
 *
 * Zooms out the view on the #FacqOscopePlot object.
 *
 * This function is called each time the user presses the zoom
 * out button in the toolbar or the entry in the menu.
 */
void facq_oscope_zoom_out(FacqOscope *oscope)
{
	facq_oscope_plot_zoom_out(oscope->priv->plot);
}

/**
 * facq_oscope_zoom_100:
 * @oscope: A #FacqOscope object.
 *
 * Restores the default view on the #FacqOscopePlot object.
 *
 * This function is called each time the user presses the zoom
 * fit button in the toolbar or the entry in the menu.
 */
void facq_oscope_zoom_100(FacqOscope *oscope)
{
	facq_oscope_plot_zoom_home(oscope->priv->plot);
}

/**
 * facq_oscope_free:
 * @oscope: A #FacqOscope object.
 *
 * Destroys a no longer needed #FacqOscope object.
 */
void facq_oscope_free(FacqOscope *oscope)
{
	g_return_if_fail(FACQ_IS_OSCOPE(oscope));
	g_object_unref(G_OBJECT(oscope));
}

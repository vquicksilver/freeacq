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
#include "facqbpm.h"
#include "facqstatusbar.h"
#include "facqplethysmographtoolbar.h"
#include "facqplethysmographmenu.h"
#include "facqplethysmograph.h"
#include "facqdisplay.h"
#include "facqdisplaymatrix.h"

/**
 * SECTION:facqplethysmograph
 * @short_description: Class for the main object of the plethysmograph application.
 * @include:facqplethysmograph
 * @title:FacqPlethysmograph
 *
 * #FacqPlethysmograph is the main object for the plethysmograph application.
 * It tries to handle and encapsulate all the complexities and the other objects
 * used by it.
 *
 * To create a new #FacqPlethysmograph object use facq_plethysmograph_new(),
 * to get the top level window of the application use facq_plethysmograph_get_widget(),
 * to disconnect the #FacqPlethysmograph when it's in connected state call
 * facq_plethysmograph_disconnect(), to change the listen address where the 
 * #FacqPlethysmograph is operating call
 * facq_plethysmograph_set_listen_address(), finally when no longer needed you
 * can destroy the object with facq_plethysmograph_free().
 *
 * By the default the #FacqPlethysmograph will show the pulse for up to 16
 * patients, but the #FacqDisplayMatrix will grow or shrink to have an optimum
 * square size depending on the number of channels, for example for one channel
 * only 1 #FacqDisplay will be showed, for 3 channels a 2x2 #FacqDisplay array
 * will be showed, only 3 would be used, and so on.
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * Internally the #FacqPlethysmograph uses the following objects to operate,
 * A #GtkWindow, a #FacqBPM object, a #FacqPlethysmographMenu, a
 * #FacqPlethysmographToolbar, a #FacqStatusbar, a #FacqPlug and a
 * #FacqDisplayMatrix.
 *
 * The #FacqPlethysmograph implements the callbacks needed by #FacqPlug for
 * the connected and disconnected signals (See the connected_callback function
 * and the disconnected_callback function) and also provides a #FacqPlugFunc
 * that is called by the #FacqPlug each time that new chunk of data is ready
 * to be processed (See the data_callback function).
 *
 * When a new client connects to the #FacqPlug the data starts flowing from the
 * client to the #FacqPlug and each time a new chunk is ready to be processed the
 * #FacqPlugFunc is called, during this call the #FacqBPM object is used to
 * calculate the actual pulse value in each channel using the facq_bpm_compute()
 * function, and after computing this values the displayed values in the
 * #FacqDisplayMatrix are updated using the facq_display_matrix_set_values()
 * function.
 *
 * The #FacqDisplayMatrix is composed of #FacqDisplay objects that also allow to
 * set a patient name while the program is running.
 * </para>
 * </sect1>
 */

/**
 * FacqPlethysmograph:
 *
 * Contains the private details of the #FacqPlethysmograph
 */

/**
 * FacqPlethysmographClass:
 *
 * Class for the #FacqPlethysmograph objects.
 */
static void facq_plethysmograph_initable_iface_init(GInitableIface  *iface);
static gboolean facq_plethysmograph_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);

G_DEFINE_TYPE_WITH_CODE(FacqPlethysmograph,facq_plethysmograph,G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_plethysmograph_initable_iface_init));

enum {
	PROP_0,
	PROP_ADDRESS,
	PROP_PORT
};

struct _FacqPlethysmographPrivate {
	gchar *address;
	guint16 port;
	GtkWidget *window;
	FacqBPM *bpm;
	FacqPlethysmographMenu *menu;
	FacqPlethysmographToolbar *toolbar;
	FacqStatusbar *statusbar;
	FacqPlug *plug;
	FacqDisplayMatrix *mat;
	GError *construct_error;
};

/*****--- callbacks ---*****/
static gboolean delete_event(GtkWidget *widget,GdkEvent *event,gpointer data)
{
	gtk_main_quit();
	return FALSE;
}

static void connected_callback(FacqPlug *plug,gpointer _plethysmograph)
{
	FacqPlethysmograph *plethysmograph = FACQ_PLETHYSMOGRAPH(_plethysmograph);
	FacqStreamData *stmd = NULL;
	gchar *address = NULL;
	guint *channels = NULL;
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

		/* setup the bpm */
		facq_bpm_setup(plethysmograph->priv->bpm,
			       stmd->n_channels,
			       stmd->period);

		/* get the channels array */
		channels = facq_chanlist_to_comedi_chanlist(stmd->chanlist,NULL);

		/* setup the display matrix */
		facq_display_matrix_setup(plethysmograph->priv->mat,
					  channels,
					  stmd->n_channels,
					  &local_err);
		g_free(channels);

		if(local_err){
			g_free(address);
			g_object_unref(G_OBJECT(stmd));
			facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",local_err->message);
			g_clear_error(&local_err);
			facq_statusbar_write_msg(plethysmograph->priv->statusbar,"%s","Client sent wrong data");
			facq_plug_disconnect(plug);
			return;
		}

		/* unref the stream data */
		g_object_unref(G_OBJECT(stmd));

		/* set the gui widgets according to the new connected status */
		facq_plethysmograph_toolbar_disable_plug_preferences(plethysmograph->priv->toolbar);
		facq_plethysmograph_menu_disable_plug_preferences(plethysmograph->priv->menu);
		facq_plethysmograph_toolbar_enable_disconnect(plethysmograph->priv->toolbar);
		facq_plethysmograph_menu_enable_disconnect(plethysmograph->priv->menu);
		facq_statusbar_write_msg(plethysmograph->priv->statusbar,
					_("New client connected from %s"),address);
		g_free(address);
	}
}

static void disconnected_callback(FacqPlug *plug,gpointer _plethysmograph)
{
	FacqPlethysmograph *plethysmograph = FACQ_PLETHYSMOGRAPH(_plethysmograph);

	facq_plethysmograph_toolbar_disable_disconnect(plethysmograph->priv->toolbar);
	facq_plethysmograph_menu_disable_disconnect(plethysmograph->priv->menu);
	facq_plethysmograph_toolbar_enable_plug_preferences(plethysmograph->priv->toolbar);
	facq_plethysmograph_menu_enable_plug_preferences(plethysmograph->priv->menu);
	facq_statusbar_write_msg(plethysmograph->priv->statusbar,_("Client disconnected"));
}

static gboolean data_callback(FacqChunk *chunk,gpointer _plethysmograph)
{
	FacqPlethysmograph *plethysmograph = FACQ_PLETHYSMOGRAPH(_plethysmograph);
	const gdouble *bpm = NULL;

	g_return_val_if_fail(FACQ_IS_CHUNK(chunk),FALSE);
	g_return_val_if_fail(FACQ_IS_PLETHYSMOGRAPH(plethysmograph),FALSE);

#if ENABLE_DEBUG
	facq_log_write("Plethysmograph processing chunk",FACQ_LOG_MSG_TYPE_DEBUG);
#endif
	bpm = facq_bpm_compute(plethysmograph->priv->bpm,chunk);
	facq_display_matrix_set_values(plethysmograph->priv->mat,bpm);

	return TRUE;
}

/*****--- GObject magic ---*****/
static void facq_plethysmograph_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqPlethysmograph *plethysmograph = FACQ_PLETHYSMOGRAPH(self);

	switch(property_id){
	case PROP_ADDRESS: plethysmograph->priv->address = g_value_dup_string(value);
	break;
	case PROP_PORT: plethysmograph->priv->port = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(plethysmograph,property_id,pspec);
	}
}

static void facq_plethysmograph_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqPlethysmograph *plethysmograph = FACQ_PLETHYSMOGRAPH(self);

	switch(property_id){
	case PROP_ADDRESS: g_value_set_string(value,plethysmograph->priv->address);
	break;
	case PROP_PORT: g_value_set_uint(value,plethysmograph->priv->port);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(plethysmograph,property_id,pspec);
	}
}

static void facq_plethysmograph_finalize(GObject *self)
{
	FacqPlethysmograph *plethysmograph = FACQ_PLETHYSMOGRAPH(self);

	if(plethysmograph->priv->construct_error)
		g_clear_error(&plethysmograph->priv->construct_error);

	if(plethysmograph->priv->address)
		g_free(plethysmograph->priv->address);

	if(FACQ_IS_PLUG(plethysmograph->priv->plug))
		facq_plug_free(plethysmograph->priv->plug);

	if(FACQ_IS_BPM(plethysmograph->priv->bpm))
		facq_bpm_free(plethysmograph->priv->bpm);

	if(FACQ_IS_PLETHYSMOGRAPH_MENU(plethysmograph->priv->menu))
		facq_plethysmograph_menu_free(plethysmograph->priv->menu);

	if(FACQ_IS_DISPLAY_MATRIX(plethysmograph->priv->mat))
		facq_display_matrix_free(plethysmograph->priv->mat);

	if(FACQ_IS_PLETHYSMOGRAPH_TOOLBAR(plethysmograph->priv->toolbar))
		facq_plethysmograph_toolbar_free(plethysmograph->priv->toolbar);

	if(FACQ_IS_STATUSBAR(plethysmograph->priv->statusbar));
		facq_statusbar_free(plethysmograph->priv->statusbar);

	if(GTK_IS_WIDGET(plethysmograph->priv->window))
		gtk_widget_destroy(plethysmograph->priv->window);

	G_OBJECT_CLASS(facq_plethysmograph_parent_class)->finalize(self);
}

static void facq_plethysmograph_constructed(GObject *self)
{
	FacqPlethysmograph *plethysmograph = FACQ_PLETHYSMOGRAPH(self);
	GdkPixbuf *icon = NULL;
	GtkWidget *vbox = NULL;
	GError *local_err = NULL;

	plethysmograph->priv->plug = 
		facq_plug_new(plethysmograph->priv->address,
			      plethysmograph->priv->port,
			      data_callback,
			      plethysmograph,
			      100,
			      &local_err);

	if(local_err){
		facq_log_write_v(FACQ_LOG_MSG_TYPE_ERROR,"%s",
					local_err->message);
		g_propagate_error(&plethysmograph->priv->construct_error,local_err);
		return;
	}

	plethysmograph->priv->bpm = facq_bpm_new();
	plethysmograph->priv->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(plethysmograph->priv->window),_("Plethysmograph"));
	icon = facq_resources_icons_plethysmograph();
	gtk_window_set_icon(GTK_WINDOW(plethysmograph->priv->window),icon);
	g_object_unref(icon);
	plethysmograph->priv->menu = facq_plethysmograph_menu_new(plethysmograph);
	plethysmograph->priv->toolbar = facq_plethysmograph_toolbar_new(plethysmograph);
	plethysmograph->priv->mat = facq_display_matrix_new(4,4);
	plethysmograph->priv->statusbar = facq_statusbar_new();

	/* connect the signals to the callbacks */
	g_signal_connect(plethysmograph->priv->plug,"connected",
			G_CALLBACK(connected_callback),plethysmograph);

	g_signal_connect(plethysmograph->priv->plug,"disconnected",
			G_CALLBACK(disconnected_callback),plethysmograph);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(plethysmograph->priv->window),vbox);

	gtk_box_pack_start(GTK_BOX(vbox),
				facq_plethysmograph_menu_get_widget(plethysmograph->priv->menu),
									FALSE,FALSE,0);

	gtk_box_pack_start(GTK_BOX(vbox),
				facq_plethysmograph_toolbar_get_widget(plethysmograph->priv->toolbar),
								        FALSE,FALSE,0);

	gtk_box_pack_start(GTK_BOX(vbox),
				facq_display_matrix_get_widget(plethysmograph->priv->mat),
									TRUE,TRUE,0);

	gtk_box_pack_end(GTK_BOX(vbox),
				facq_statusbar_get_widget(plethysmograph->priv->statusbar),
									FALSE,FALSE,0);

	g_signal_connect(plethysmograph->priv->window,"delete-event",
			G_CALLBACK(delete_event),NULL);
	
	if(plethysmograph->priv->address)
		facq_statusbar_write_msg(plethysmograph->priv->statusbar,
						_("Listening on %s:%u"),
						plethysmograph->priv->address,plethysmograph->priv->port);
	else
		facq_statusbar_write_msg(plethysmograph->priv->statusbar,
						_("Listening on all:%u"),
							plethysmograph->priv->port);

	gtk_widget_show_all(plethysmograph->priv->window);
}

static void facq_plethysmograph_class_init(FacqPlethysmographClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqPlethysmographPrivate));

	object_class->set_property = facq_plethysmograph_set_property;
	object_class->get_property = facq_plethysmograph_get_property;
	object_class->constructed = facq_plethysmograph_constructed;
	object_class->finalize = facq_plethysmograph_finalize;

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

static void facq_plethysmograph_init(FacqPlethysmograph *plethysmograph)
{
	plethysmograph->priv = G_TYPE_INSTANCE_GET_PRIVATE(plethysmograph,FACQ_TYPE_PLETHYSMOGRAPH,FacqPlethysmographPrivate);
	plethysmograph->priv->address = NULL;
	plethysmograph->priv->port = 3000;
	plethysmograph->priv->window = NULL;
	plethysmograph->priv->menu = NULL;
	plethysmograph->priv->toolbar = NULL;
	plethysmograph->priv->statusbar = NULL;
	plethysmograph->priv->plug = NULL;
}

/*****--- GInitable interface ---*****/
static void facq_plethysmograph_initable_iface_init(GInitableIface *iface)
{
	iface->init = facq_plethysmograph_initable_init;
}

static gboolean facq_plethysmograph_initable_init(GInitable *initable,GCancellable *cancellable,GError **error)
{
	FacqPlethysmograph *plethysmograph;

	g_return_val_if_fail(FACQ_IS_PLETHYSMOGRAPH(initable),FALSE);
	plethysmograph = FACQ_PLETHYSMOGRAPH(initable);
	if(cancellable != NULL){
                g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                           "Cancellable initialization not supported");
                return FALSE;
        }
        if(plethysmograph->priv->construct_error){
                if(error)
                        *error = g_error_copy(plethysmograph->priv->construct_error);
                return FALSE;
        }
        return TRUE;
}

/**
 * facq_plethysmograph_new:
 * @address: The listen address.
 * @port: The port.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqPlethysmograph object, that listens for connection
 * petitions on the address, @address, and port, @port. (See #FacqPlug for more
 * details about valid values).
 *
 * Returns: A new #FacqPlethysmograph object or %NULL in case of error.
 */
FacqPlethysmograph *facq_plethysmograph_new(const gchar *address,guint16 port,GError **err)
{
	return FACQ_PLETHYSMOGRAPH(g_initable_new(FACQ_TYPE_PLETHYSMOGRAPH,
					  NULL,err,
					  "address",address,
					  "port",port,
					  NULL));
}

/**
 * facq_plethysmograph_get_widget:
 * @plethysmograph: A #FacqPlethysmograph object.
 *
 * Gets the top level application widget, so you can show it to the user.
 *
 * Returns: A #GtkWidget pointing to the top level window.
 */
GtkWidget *facq_plethysmograph_get_widget(const FacqPlethysmograph *plethysmograph)
{
	g_return_val_if_fail(FACQ_IS_PLETHYSMOGRAPH(plethysmograph),NULL);
	return plethysmograph->priv->window;
}

/**
 * facq_plethysmograph_disconnect:
 * @plethysmograph: A #FacqPlethysmograph object.
 *
 * Wrapper for facq_plug_disconnect() on the internal #FacqPlug object
 * used by the #FacqPlethysmograph.
 */
void facq_plethysmograph_disconnect(FacqPlethysmograph *plethysmograph)
{
	g_return_if_fail(FACQ_IS_PLETHYSMOGRAPH(plethysmograph));
	facq_plug_disconnect(plethysmograph->priv->plug);
}

/**
 * facq_plethysmograph_set_listen_address:
 * @plethysmograph: A #FacqPlethysmograph object.
 *
 * Sets the listen address on a disconnect #FacqPlethysmograph object.
 * It shows a #FacqPlugDialog to the user, and in the case the user
 * accepts the changes, facq_plug_set_listen_address() is called on
 * the internal #FacqPlug object, with the new parameters.
 *
 * The status of the operation will be printed to the #FacqStatusbar.
 */
void facq_plethysmograph_set_listen_address(FacqPlethysmograph *plethysmograph)
{
	FacqPlug *plug = NULL;
	FacqStatusbar *statusbar = NULL;
	gchar *address = NULL;
	guint16 port = 0;
	GError *local_err = NULL;
	FacqPlugDialog *dialog = NULL;

	plug = plethysmograph->priv->plug;
	address = facq_plug_get_address(plug);
	port = facq_plug_get_port(plug);
	statusbar = plethysmograph->priv->statusbar;
	dialog = facq_plug_dialog_new(facq_plethysmograph_get_widget(plethysmograph),address,port);
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
 * facq_plethysmograph_free:
 * @plethysmograph: A #FacqPlethysmograph object.
 *
 * Destroys a no longer needed #FacqPlethysmograph object.
 */
void facq_plethysmograph_free(FacqPlethysmograph *plethysmograph)
{
	g_return_if_fail(FACQ_IS_PLETHYSMOGRAPH(plethysmograph));
	g_object_unref(G_OBJECT(plethysmograph));
}

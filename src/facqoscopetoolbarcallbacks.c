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
#include <gtkdatabox.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqlog.h"
#include "gdouble.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqplug.h"
#include "facqstatusbar.h"
#include "facqoscopetoolbar.h"
#include "facqoscopetoolbar.h"
#include "facqoscopeplot.h"
#include "facqoscope.h"
#include "facqoscopetoolbarcallbacks.h"

/**
 * SECTION:facqoscopetoolbarcallbacks
 * @short_description: Contains the toolbar callbacks for the oscilloscope.
 * @include:facqoscopetoolbarcallbacks.h
 *
 * Contains the toolbar callbacks for the oscilloscope, for the plug
 * preferences, for the disconnect, for the zoom in, zoom out, and normal size
 * buttons.
 *
 * When you press the preferences button the
 * facq_oscope_toolbar_callback_preferences() is called, when you press the
 * disconnect button facq_oscope_toolbar_callback_disconnect() is called.
 *
 * The zoom in, zoom out and normal size corresponds to the
 * facq_oscope_toolbar_callback_zoom_in(),
 * facq_oscope_toolbar_callback_zoom_out() and
 * facq_oscope_toolbar_callback_zoom_100().
 */

/**
 * facq_oscope_toolbar_callback_preferences:
 * @toolbutton: The toolbar's preferences button.
 * @data: A pointer to a #FacqOscope object.
 *
 * Calls the facq_oscope_set_listen_address() on @data.
 *
 */
void facq_oscope_toolbar_callback_preferences(GtkToolButton *toolbutton,gpointer data)
{
	FacqOscope *oscope = FACQ_OSCOPE(data);
	
	facq_oscope_set_listen_address(oscope);
}

/**
 * facq_oscope_toolbar_callback_disconnect:
 * @toolbutton: The toolbar's disconnect button.
 * @data: A pointer to a #FacqOscope object.
 *
 * Calls the facq_oscope_disconnect() function on @data.
 */
void facq_oscope_toolbar_callback_disconnect(GtkToolButton *toolbutton,gpointer data)
{
	FacqOscope *oscope = FACQ_OSCOPE(data);

	facq_oscope_disconnect(oscope);
}

/**
 * facq_oscope_toolbar_callback_zoom_in:
 * @toolbutton: The toolbar's Zoom in button.
 * @data: A pointer to a #FacqOscope object.
 *
 * Calls the facq_oscope_zoom_in() function on @data.
 */
void facq_oscope_toolbar_callback_zoom_in(GtkToolButton *toolbutton,gpointer data)
{
	FacqOscope *oscope = FACQ_OSCOPE(data);

	facq_oscope_zoom_in(oscope);
}

/**
 * facq_oscope_toolbar_callback_zoom_out:
 * @toolbutton: The toolbar's Zoom out button.
 * @data: A pointer to a #FacqOscope object.
 *
 * Calls the facq_oscope_zoom_out() function on @data.
 */
void facq_oscope_toolbar_callback_zoom_out(GtkToolButton *toolbutton,gpointer data)
{
	FacqOscope *oscope = FACQ_OSCOPE(data);

	facq_oscope_zoom_out(oscope);
}

/**
 * facq_oscope_toolbar_callback_zoom_100:
 * @toolbutton: The toolbar's Normal size button.
 * @data: A pointer to a #FacqOscope object.
 *
 * Calls the facq_oscope_zoom_100() function on @data.
 */
void facq_oscope_toolbar_callback_zoom_100(GtkToolButton *toolbutton,gpointer data)
{
	FacqOscope *oscope = FACQ_OSCOPE(data);

	facq_oscope_zoom_100(oscope);
}

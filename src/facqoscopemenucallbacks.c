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
#include "facqi18n.h"
#include "facqoscope.h"
#include "facqresourcesicons.h"
#include "facqoscopemenucallbacks.h"

/**
 * SECTION:facqoscopemenucallbacks
 * @short_description: Provides the callbacks for the menu in the oscilloscope
 * application.
 * @include:facqoscopemenucallbacks.h
 *
 * Provides the menu callbacks for the oscilloscope application, for the entries 
 * in the Plug, Zoom and Help submenus. That is callbacks for the Preferences,
 * Disconnect, Zoom In, Zoom Out, Normal Size and About menu entries.
 *
 * When you press one of the entries in the menu one of the following callback
 * functions will be called, facq_oscope_menu_callback_preferences() is called
 * when you press the entry Plug->Preferences, facq_oscope_menu_callback_disconnect()
 * is called when you press the entry Plug->Disconnect, 
 * facq_oscope_menu_callback_zoom_in() is called when you press the entry
 * Zoom->Zoom in , facq_oscope_menu_callback_zoom_out() is called when you press
 * the entry Zoom->Zoom out , facq_oscope_menu_callback_zoom_100() is called
 * when you press the Zoom->Normal Size entry and finally
 * facq_oscope_menu_callback_about() is called when you press the Help->About
 * entry.
 */

/**
 * facq_oscope_menu_callback_preferences:
 * @item: A #GtkMenuItem in this case it's the Preferences entry.
 * @data: A pointer to a #FacqOscope object.
 *
 * Calls the facq_oscope_set_listen_address() function on @data.
 */
void facq_oscope_menu_callback_preferences(GtkMenuItem *item,gpointer data)
{
	FacqOscope *oscope = FACQ_OSCOPE(data);
	
	facq_oscope_set_listen_address(oscope);
}

/**
 * facq_oscope_menu_callback_disconnect:
 * @item: A #GtkMenuItem in this case it's the Disconnect entry.
 * @data: A pointer to a #FacqOscope object.
 *
 * Calls the facq_oscope_disconnect() function on @data.
 */
void facq_oscope_menu_callback_disconnect(GtkMenuItem *item,gpointer data)
{
	FacqOscope *oscope = FACQ_OSCOPE(data);

	facq_oscope_disconnect(oscope);
}

/**
 * facq_oscope_menu_callback_zoom_in:
 * @item: A #GtkMenuItem in this case it's the Zoom In entry.
 * @data: A pointer to a #FacqOscope object.
 *
 * Calls the facq_oscope_zoom_in() function on @data.
 */
void facq_oscope_menu_callback_zoom_in(GtkMenuItem *item,gpointer data)
{
	FacqOscope *oscope = FACQ_OSCOPE(data);

	facq_oscope_zoom_in(oscope);
}

/**
 * facq_oscope_menu_callback_zoom_out:
 * @item: A #GtkMenuItem in this case it's the Zoom Out entry.
 * @data: A pointer to a #FacqOscope object.
 *
 * Calls the facq_oscope_zoom_out() function on @data.
 */
void facq_oscope_menu_callback_zoom_out(GtkMenuItem *item,gpointer data)
{
	FacqOscope *oscope = FACQ_OSCOPE(data);

	facq_oscope_zoom_out(oscope);
}

/**
 * facq_oscope_menu_callback_preferences:
 * @item: A #GtkMenuItem in this case it's the Normal Size entry.
 * @data: A pointer to a #FacqOscope object.
 *
 * Calls the facq_oscope_zoom_100() function on @data.
 */
void facq_oscope_menu_callback_zoom_100(GtkMenuItem *item,gpointer data)
{
	FacqOscope *oscope = FACQ_OSCOPE(data);

	facq_oscope_zoom_100(oscope);
}

/**
 * facq_oscope_menu_callback_about:
 * @item: A #GtkMenuItem in this case it's the About entry.
 * @data: A pointer to a #FacqOscope object.
 *
 * Shows the about dialog of the oscilloscope application.
 */
void facq_oscope_menu_callback_about(GtkMenuItem *item,gpointer data)
{
	FacqOscope *oscope = FACQ_OSCOPE(data);

	gtk_show_about_dialog(GTK_WINDOW(facq_oscope_get_widget(oscope)),
			"program-name", _("Freeacq Oscilloscope"),
			"version","0.0.1",
			"comments", _("A simple oscilloscope tool"),
			"logo",facq_resources_icons_oscope(),
			NULL);
}

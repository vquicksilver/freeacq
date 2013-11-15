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
#include "facqplethysmograph.h"
#include "facqresourcesicons.h"
#include "facqplethysmographmenucallbacks.h"

/**
 * SECTION:facqplethysmographmenucallbacks
 * @short_description: Contains the menu callbacks for the plethysmograph
 * @include:facqplethysmographmenucallbacks.h
 *
 * Contains the menu callbacks for the plethysmograph, for the plug preferences
 * button, for the disconnect button and for the about button.
 *
 * When you press one of the entries (or buttons) in the menu one of the
 * following callback functions will be called, 
 * facq_plethysmograph_menu_callback_plug_preferences() is called when you
 * press the entry Plug->Preferences,
 * facq_plethysmograph_menu_callback_disconnect() is called when you press the
 * entry Plug->Disconnect, and facq_plethysmograph_menu_callback_about()
 * is called when you press the entry Help->About.
 *
 */

/**
 * facq_plethysmograph_menu_callback_plug_preferences:
 * @item: A #GtkMenuItem in this case it's the preferences entry.
 * @data: A pointer to a #FacqPlethysmograph object.
 *
 * Calls facq_plethysmograph_set_listen_address() on the #FacqPlethysmograph
 * pointed by @data, after the user pressed the preferences entry.
 *
 */
void facq_plethysmograph_menu_callback_plug_preferences(GtkMenuItem *item,gpointer data)
{
	FacqPlethysmograph *plethysmograph = FACQ_PLETHYSMOGRAPH(data);
	facq_plethysmograph_set_listen_address(plethysmograph);
}

/**
 * facq_plethysmograph_menu_callback_disconnect:
 * @item: A #GtkMenuItem in this case it's the disconnect entry.
 * @data: A pointer to a #FacqPlethysmograph object.
 *
 * Calls the facq_plethysmograph_disconnect() on the #FacqPlethysmograph
 * pointed by @data, after the user pressed the disconnect entry.
 *
 */
void facq_plethysmograph_menu_callback_disconnect(GtkMenuItem *item,gpointer data)
{
	FacqPlethysmograph *plethysmograph = FACQ_PLETHYSMOGRAPH(data);
	facq_plethysmograph_disconnect(plethysmograph);
}

/**
 * facq_plethysmograph_menu_callback_about:
 * @item: A #GtkMenuItem in this case it's the about entry.
 * @data: A pointer to a #FacqPlethysmograph object.
 *
 * Shows a #GtkAboutDialog with information about the plethysmograph
 * application, after the user has pressed the about entry in the
 * help submenu.
 *
 */
void facq_plethysmograph_menu_callback_about(GtkMenuItem *item,gpointer data)
{
	FacqPlethysmograph *plethysmograph = FACQ_PLETHYSMOGRAPH(data);

	gtk_show_about_dialog(GTK_WINDOW(facq_plethysmograph_get_widget(plethysmograph)),
			"program-name", _("Freeacq Plethysmograph"),
			"version","0.0.1",
			"comments", _("A Plethysmograph tool"),
			"logo",facq_resources_icons_plethysmograph(),
			NULL);
}

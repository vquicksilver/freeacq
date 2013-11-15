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
#include "facqplethysmograph.h"
#include "facqplethysmographtoolbarcallbacks.h"

/**
 * SECTION:facqplethysmographtoolbarcallbacks
 * @short_description: Contains the toolbar callbacks for the plethysmograph
 * @include:facqplethysmographtoolbarcallbacks.h
 *
 * Contains the toolbar callbacks for the plethysmograph, for the plug
 * preferences button and for the disconnect button.
 *
 * When you press the preferences button the
 * facq_plethysmograph_toolbar_callback_plug_preferences() function is called
 * and when you press the disconnect button the
 * facq_plethysmograph_toolbar_callback_disconnect() function is called.
 */

/**
 * facq_plethysmograph_toolbar_callback_plug_preferences:
 * @toolbutton: The toolbar's preferences button.
 * @data: A pointer to a #FacqPlethysmograph object.
 *
 * Calls the facq_plethysmograph_set_listen_address() on @data.
 */
void facq_plethysmograph_toolbar_callback_plug_preferences(GtkToolButton *toolbutton,gpointer data)
{
	FacqPlethysmograph *plethysmograph = FACQ_PLETHYSMOGRAPH(data);
	facq_plethysmograph_set_listen_address(plethysmograph);
}

/**
 * facq_plethysmograph_toolbar_callback_disconnect:
 * @toolbutton: The toolbar's disconnect button.
 * @data: A pointer to a #FacqPlethysmograph object.
 *
 * Calls the facq_plethysmograph_disconnect() on @data.
 */
void facq_plethysmograph_toolbar_callback_disconnect(GtkToolButton *toolbutton,gpointer data)
{
	FacqPlethysmograph *plethysmograph = FACQ_PLETHYSMOGRAPH(data);
	facq_plethysmograph_disconnect(plethysmograph);
}

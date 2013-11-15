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
#include "facqcapture.h"
#include "facqresourcesicons.h"
#include "facqcapturemenucallbacks.h"

/**
 * SECTION:facqcapturemenucallbacks
 * @short_description: Provides the callback functions for the menu in the
 * capture application.
 * @include:facqcapturemenucallbacks.h
 *
 * Provides the callback functions for the menu in the capture application,
 * including a callback for each of this entries, Stream->Preferences, 
 * Stream->New, Stream->Open, Stream->Save as, Stream->Close, Control->Play,
 * Control->Stop, Control->Clear, Control->Add, Control->Remove, Log->Read,
 * Help->About.
 */

/**
 * facq_capture_menu_callback_new:
 * @item: A #GtkMenuItem in this case it's the New entry.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_stream_new() on the #FacqCapture object pointed by
 * @data, after the user pressed the New entry.
 */
void facq_capture_menu_callback_new(GtkMenuItem *item,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_stream_new(cap);
}

/**
 * facq_capture_menu_callback_open:
 * @item: A #GtkMenuItem in this case it's the Open entry.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_stream_open() on the #FacqCapture object pointed by
 * @data, after the user pressed the Open entry.
 */
void facq_capture_menu_callback_open(GtkMenuItem *item,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_stream_open(cap);
}

/**
 * facq_capture_menu_callback_save_as:
 * @item: A #GtkMenuItem in this case it's the Save as entry.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_stream_save_as() on the #FacqCapture object pointed by
 * @data, after the user pressed the Save as entry.
 */
void facq_capture_menu_callback_save_as(GtkMenuItem *item,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_stream_save_as(cap);
}

/**
 * facq_capture_menu_callback_close:
 * @item: A #GtkMenuItem in this case it's the Close entry.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_stream_close() on the #FacqCapture object pointed by
 * @data, after the user pressed the Close entry.
 */
void facq_capture_menu_callback_close(GtkMenuItem *item,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_stream_close(cap);
}

/**
 * facq_capture_menu_callback_preferences:
 * @item: A #GtkMenuItem in this case it's the Preferences entry.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_stream_preferences() on the #FacqCapture object pointed by
 * @data, after the user pressed the Preferences entry.
 */
void facq_capture_menu_callback_preferences(GtkMenuItem *item,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_stream_preferences(cap);
}

/**
 * facq_capture_menu_callback_play:
 * @item: A #GtkMenuItem in this case it's the Play entry.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_control_play() on the #FacqCapture object pointed by
 * @data, after the user pressed the Play entry.
 */
void facq_capture_menu_callback_play(GtkMenuItem *item,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_control_play(cap);
}

/**
 * facq_capture_menu_callback_stop:
 * @item: A #GtkMenuItem in this case it's the Stop entry.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_control_stop() on the #FacqCapture object pointed by
 * @data, after the user pressed the Stop entry.
 */
void facq_capture_menu_callback_stop(GtkMenuItem *item,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_control_stop(cap);
}

/**
 * facq_capture_menu_callback_add:
 * @item: A #GtkMenuItem in this case it's the Add entry.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_control_add() on the #FacqCapture object pointed by
 * @data, after the user pressed the Add entry.
 */
void facq_capture_menu_callback_add(GtkMenuItem *item,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_control_add(cap);
}

/**
 * facq_capture_menu_callback_remove:
 * @item: A #GtkMenuItem in this case it's the Add entry.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_control_remove() on the #FacqCapture object pointed by
 * @data, after the user pressed the Add entry.
 */
void facq_capture_menu_callback_remove(GtkMenuItem *item,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

        facq_capture_control_remove(cap);
}

/**
 * facq_capture_menu_callback_clear:
 * @item: A #GtkMenuItem in this case it's the Clear entry.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_control_clear() on the #FacqCapture object pointed by
 * @data, after the user pressed the Clear entry.
 */
void facq_capture_menu_callback_clear(GtkMenuItem *item,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_control_clear(cap);
}

/**
 * facq_capture_menu_callback_log:
 * @item: A #GtkMenuItem in this case it's the Log entry.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_log() on the #FacqCapture object pointed by
 * @data, after the user pressed the Log entry.
 */
void facq_capture_menu_callback_log(GtkMenuItem *item,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_log(cap);
}

/**
 * facq_capture_menu_callback_about:
 * @item: A #GtkMenuItem in this case it's the About entry.
 * @data: A pointer to a #FacqCapture object.
 *
 * Shows a #GtkAboutDialog about the capture application 
 * after the user pressed the About entry.
 */
void facq_capture_menu_callback_about(GtkMenuItem *item,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

        gtk_show_about_dialog(GTK_WINDOW(facq_capture_get_widget(cap)),
                        "program-name", _("Freeacq Capture"),
                        "version","0.0.1",
                        "comments", _("A Stream manager and data capturer"),
			"logo",facq_resources_icons_capture(),
                        NULL);
}

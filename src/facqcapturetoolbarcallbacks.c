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
#include "facqcapture.h"
#include "facqcapturetoolbarcallbacks.h"

/**
 * SECTION:facqcapturetoolbarcallbacks
 * @short_description: Provides the callbacks for the capture's toolbar.
 * @include:facqcapturetoolbarcallbacks.h
 *
 * Provides the callback function for capture's toolbar, for the add button,
 * for the remove button, for the clear button, for the play button, 
 * and for the stop button.
 *
 * When you press the one of the buttons in the toolbar one of the following
 * callback functions will be called in response.
 * When you press the add button facq_capture_toolbar_callback_add() will be
 * called, when you press the remove button
 * facq_capture_toolbar_callback_remove() will be called, when you press the
 * clear button facq_capture_toolbar_callback_clear() will be called, when you
 * press the play button facq_capture_toolbar_callback_play() will be called,
 * when you press the stop button facq_capture_toolbar_callback_stop() will
 * be called.
 */

/**
 * facq_capture_toolbar_callback_add:
 * @toolbutton: A #GtkToolButton in this case it's the add button.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_control_add() function on @data.
 */
void facq_capture_toolbar_callback_add(GtkToolButton *toolbutton,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_control_add(cap);
}

/**
 * facq_capture_toolbar_callback_remove:
 * @entry: A #GtkToolButton in this case it's the remove button.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_control_remove() function on @data.
 */
void facq_capture_toolbar_callback_remove(GtkToolButton *toolbutton,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_control_remove(cap);
}

/**
 * facq_capture_toolbar_callback_clear:
 * @toolbutton: A #GtkToolButton in this case it's the clear button.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_control_clear() function on @data.
 */
void facq_capture_toolbar_callback_clear(GtkToolButton *toolbutton,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_control_clear(cap);
}

/**
 * facq_capture_toolbar_callback_play:
 * @toolbutton: A #GtkToolButton in this case it's the play button.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_control_play() function on @data.
 */
void facq_capture_toolbar_callback_play(GtkToolButton *toolbutton,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_control_play(cap);
}

/**
 * facq_capture_toolbar_callback_stop:
 * @toolbutton: A #GtkToolButton in this case it's the stop button.
 * @data: A pointer to a #FacqCapture object.
 *
 * Calls the facq_capture_control_stop() function on @data.
 */
void facq_capture_toolbar_callback_stop(GtkToolButton *toolbutton,gpointer data)
{
	FacqCapture *cap = FACQ_CAPTURE(data);

	facq_capture_control_stop(cap);
}

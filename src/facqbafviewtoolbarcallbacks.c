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
#include "facqbafview.h"
#include "facqbafviewtoolbarcallbacks.h"

/**
 * SECTION:facqbafviewtoolbarcallbacks
 * @short_description: Provides the callback functions for the toolbar in the
 * binary acquisition file viewer application.
 * @include:facqbafviewtoolbarballbacks.h
 *
 * Provides the callback function for the toolbar in the binary acquisition file
 * viewer application, including callbacks for the Page setup, First page,
 * Previous page, the #GtkSpinButton, Next page, Last page, Zoom in, Zoom out, 
 * and Normal Size buttons.
 *
 * The following callbacks function will be called in response to the following
 * events:
 *
 * - Press Page setup button -> facq_baf_view_toolbar_callback_page_setup()
 * - Press First page button -> facq_baf_view_toolbar_callback_goto_first()
 * - Press Prev page button  -> facq_baf_view_toolbar_callback_go_back()
 * - Press Next page button  -> facq_baf_view_toolbar_callback_go_forward()
 * - Press Last page button  -> facq_baf_view_toolbar_callback_goto_last()
 * - Press Zoom in button    -> facq_baf_view_toolbar_callback_zoom_in()
 * - Press Zoom out button   -> facq_baf_view_toolbar_callback_zoom_out()
 * - Press Normal size button-> facq_baf_view_toolbar_callback_zoom_100()
 *
 * The facq_baf_view_toolbar_callback_intro() is called each time the
 * #GtkSpinButton in the toolbar emits the "value-changed" signal.
 */

/**
 * facq_baf_view_toolbar_callback_page_setup:
 * @toolbutton: A #GtkToolButton in this case a pointer to the page setup
 * button.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_setup_page_time() function on @data.
 */
void facq_baf_view_toolbar_callback_page_setup(GtkToolButton *toolbutton,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_setup_page_time(view);
}

/**
 * facq_baf_view_toolbar_callback_intro:
 * @spin: A pointer to the #GtkSpinButton in the toolbar.
 * @data: A pointer to a #FacqBAFView object.
 *
 * This function gets called each time the #GtkSpinButton in the toolbar
 * emits the "value-changed" signal. It calls the facq_baf_view_plot_page_spin()
 * function. Note that in the object hierarchy #GtkEntry is a father of the
 * #GtkSpinButton.
 */
void facq_baf_view_toolbar_callback_intro(GtkSpinButton *spin,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);
	
	facq_baf_view_plot_page_spin(view);
}

/**
 * facq_baf_view_toolbar_callback_goto_first:
 * @toolbutton: A #GtkToolButton in this case a pointer to the first page
 * button.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_plot_first_page() function on @data.
 */
void facq_baf_view_toolbar_callback_goto_first(GtkToolButton *toolbutton,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_plot_first_page(view);
}

/**
 * facq_baf_view_toolbar_callback_go_back:
 * @toolbutton: A #GtkToolButton in this case a pointer to the prev page
 * button.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_plot_prev_page() function on @data.
 */
void facq_baf_view_toolbar_callback_go_back(GtkToolButton *toolbutton,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_plot_prev_page(view);
}

/**
 * facq_baf_view_toolbar_callback_page_setup:
 * @toolbutton: A #GtkToolButton in this case a pointer to the next page button.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_plot_next_page() function on @data.
 */
void facq_baf_view_toolbar_callback_go_forward(GtkToolButton *toolbutton,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_plot_next_page(view);
}

/**
 * facq_baf_view_toolbar_callback_goto_last:
 * @toolbutton: A #GtkToolButton in this case a pointer to the last page
 * button.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_plot_last_page() function on @data.
 */
void facq_baf_view_toolbar_callback_goto_last(GtkToolButton *toolbutton,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_plot_last_page(view);
}

/**
 * facq_baf_view_toolbar_callback_zoom_in:
 * @toolbutton: A #GtkToolButton in this case a pointer to the Zoom in
 * button.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_zoom_in() function on @data.
 */
void facq_baf_view_toolbar_callback_zoom_in(GtkToolButton *toolbutton,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_zoom_in(view);
}

/**
 * facq_baf_view_toolbar_callback_page_setup:
 * @toolbutton: A #GtkToolButton in this case a pointer to the Zoom out button.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_zoom_out() function on @data.
 */
void facq_baf_view_toolbar_callback_zoom_out(GtkToolButton *toolbutton,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_zoom_out(view);
}

/**
 * facq_baf_view_toolbar_callback_page_setup:
 * @toolbutton: A #GtkToolButton in this case a pointer to the Normal size
 * button.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_zoom_fit() function on @data.
 */
void facq_baf_view_toolbar_callback_zoom_100(GtkToolButton *toolbutton,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_zoom_fit(view);
}

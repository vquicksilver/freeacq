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
#include "facqbafview.h"
#include "facqresourcesicons.h"
#include "facqbafviewmenucallbacks.h"

/**
 * SECTION:facqbafviewmenucallbacks
 * @short_description: Provides the callback functions for binary acquistion
 * file viewer menu.
 * @include:facqbafviewmenucallbacks.h
 *
 * Provides the callback functions for binary acquisition file viewer menu.
 * Each time you press one of the entries in the menu one of this callback
 * will be launched in response:
 *
 * - File->Open        - facq_baf_view_menu_callback_open()
 * - File->Close       - facq_baf_view_menu_callback_close()
 * - File->Export      - facq_baf_view_menu_callback_save_as()
 * - Page->Page setup  - facq_baf_view_menu_callback_page_setup()
 * - Page->First       - facq_baf_view_menu_callback_goto_first()
 * - Page->Back        - facq_baf_view_menu_callback_go_back()
 * - Page->Next        - facq_baf_view_menu_callback_go_forward()
 * - Page->Last        - facq_baf_view_menu_callback_goto_last()
 * - Zoom->Zoom In     - facq_baf_view_menu_callback_zoom_in()
 * - Zoom->Zoom Out    - facq_baf_view_menu_callback_zoom_out()
 * - Zoom->Normal size - facq_baf_view_menu_callback_zoom_100()
 * - Help->About       - facq_baf_view_menu_callback_about()
 */

/**
 * facq_baf_view_menu_callback_open:
 * @item: A #GtkMenuItem in this case a pointer to the Open entry.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_open_file() function on @data.
 */
void facq_baf_view_menu_callback_open(GtkMenuItem *item,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_open_file(view);
}

/**
 * facq_baf_view_menu_callback_save_as:
 * @item: A #GtkMenuItem in this case a pointer to the Save as entry.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_export_file() function on @data.
 */
void facq_baf_view_menu_callback_save_as(GtkMenuItem *item,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_export_file(view);
}

/**
 * facq_baf_view_menu_callback_close:
 * @item: A #GtkMenuItem in this case a pointer to the Close entry.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_close_file() function on @data.
 */
void facq_baf_view_menu_callback_close(GtkMenuItem *item,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_close_file(view);
}

/**
 * facq_baf_view_menu_callback_page_setup:
 * @item: A #GtkMenuItem in this case a pointer to the Page setup entry.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_setup_page_time() function on @data.
 */
void facq_baf_view_menu_callback_page_setup(GtkMenuItem *item,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_setup_page_time(view);
}

/**
 * facq_baf_view_menu_callback_goto_first:
 * @item: A #GtkMenuItem in this case a pointer to the First (page) entry.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_plot_first_page() function on @data.
 */
void facq_baf_view_menu_callback_goto_first(GtkMenuItem *item,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_plot_first_page(view);
}

/**
 * facq_baf_view_menu_callback_go_back:
 * @item: A #GtkMenuItem in this case a pointer to the Back (page) entry.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_plot_prev_page() function on @data.
 */
void facq_baf_view_menu_callback_go_back(GtkMenuItem *item,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_plot_prev_page(view);
}

/**
 * facq_baf_view_menu_callback_go_forward:
 * @item: A #GtkMenuItem in this case a pointer to the Next (page) entry.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_plot_next_page() function on @data.
 */
void facq_baf_view_menu_callback_go_forward(GtkMenuItem *item,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_plot_next_page(view);
}

/**
 * facq_baf_view_menu_callback_goto_last:
 * @item: A #GtkMenuItem in this case a pointer to the Last (page) entry.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_plot_last_page() function on @data.
 */
void facq_baf_view_menu_callback_goto_last(GtkMenuItem *item,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

        facq_baf_view_plot_last_page(view);
}

/**
 * facq_baf_view_menu_callback_zoom_in:
 * @item: A #GtkMenuItem in this case a pointer to the Zoom In entry.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_zoom_in() function on @data.
 */
void facq_baf_view_menu_callback_zoom_in(GtkMenuItem *item,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

	facq_baf_view_zoom_in(view);
}

/**
 * facq_baf_view_menu_callback_zoom_out:
 * @item: A #GtkMenuItem in this case a pointer to the Zoom out entry.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_zoom_out() function on @data.
 */
void facq_baf_view_menu_callback_zoom_out(GtkMenuItem *item,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

        facq_baf_view_zoom_out(view);
}

/**
 * facq_baf_view_menu_callback_zoom_100:
 * @item: A #GtkMenuItem in this case a pointer to the Normal size entry.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Calls the facq_baf_view_zoom_fit() function on @data.
 */
void facq_baf_view_menu_callback_zoom_100(GtkMenuItem *item,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

        facq_baf_view_zoom_fit(view);
}

/**
 * facq_baf_view_menu_callback_about:
 * @item: A #GtkMenuItem in this case a pointer to the About (help) entry.
 * @data: A pointer to a #FacqBAFView object.
 *
 * Shows a #GtkAboutDialog with information about the binary acquisition file
 * viewer application.
 */
void facq_baf_view_menu_callback_about(GtkMenuItem *item,gpointer data)
{
	FacqBAFView *view = FACQ_BAF_VIEW(data);

        gtk_show_about_dialog(GTK_WINDOW(facq_baf_view_get_widget(view)),
                        "program-name", _("Freeacq BAF Viewer"),
                        "version","0.0.1",
                        "comments", _("A Binary Acquisition File (.baf) viewer"),
			"logo",facq_resources_icons_viewer(),
                        NULL);
}

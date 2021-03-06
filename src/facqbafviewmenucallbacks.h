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
#ifndef _FREEACQ_BAF_VIEW_MENU_CALLBACKS_H
#define _FREEACQ_BAF_VIEW_MENU_CALLBACKS_H

G_BEGIN_DECLS

void facq_baf_view_menu_callback_open(GtkMenuItem *item,gpointer data);
void facq_baf_view_menu_callback_save_as(GtkMenuItem *item,gpointer data);
void facq_baf_view_menu_callback_close(GtkMenuItem *item,gpointer data);
void facq_baf_view_menu_callback_page_setup(GtkMenuItem *item,gpointer data);
void facq_baf_view_menu_callback_goto_first(GtkMenuItem *item,gpointer data);
void facq_baf_view_menu_callback_go_back(GtkMenuItem *item,gpointer data);
void facq_baf_view_menu_callback_go_forward(GtkMenuItem *item,gpointer data);
void facq_baf_view_menu_callback_goto_last(GtkMenuItem *item,gpointer data);
void facq_baf_view_menu_callback_zoom_in(GtkMenuItem *item,gpointer data);
void facq_baf_view_menu_callback_zoom_out(GtkMenuItem *item,gpointer data);
void facq_baf_view_menu_callback_zoom_100(GtkMenuItem *item,gpointer data);
void facq_baf_view_menu_callback_about(GtkMenuItem *item,gpointer data);

G_END_DECLS

#endif

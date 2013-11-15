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
#ifndef _FREEACQ_CAPTURE_TOOLBAR_CALLBACKS_H
#define _FREEACQ_CAPTURE_TOOLBAR_CALLBACKS_H

G_BEGIN_DECLS

void facq_capture_toolbar_callback_add(GtkToolButton *toolbutton,gpointer data);
void facq_capture_toolbar_callback_remove(GtkToolButton *entry,gpointer data);
void facq_capture_toolbar_callback_clear(GtkToolButton *toolbutton,gpointer data);
void facq_capture_toolbar_callback_play(GtkToolButton *toolbutton,gpointer data);
void facq_capture_toolbar_callback_stop(GtkToolButton *toolbutton,gpointer data);

G_END_DECLS

#endif

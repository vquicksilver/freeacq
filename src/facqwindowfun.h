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
#ifndef _FREEACQ_WINDOW_FUN_H
#define _FREEACQ_WINDOW_FUN_H

G_BEGIN_DECLS

typedef enum win_func {
	FACQ_WF_TYPE_REC,
	FACQ_WF_TYPE_TRI,
	FACQ_WF_TYPE_BAR,
	FACQ_WF_TYPE_WEL,
	FACQ_WF_TYPE_HAN,
	FACQ_WF_TYPE_HAM,
	FACQ_WF_TYPE_FLA,
	FACQ_WF_TYPE_BLA,
	/*< private >*/
	FACQ_WF_TYPE_N
} FacqWindowFunType;

gdouble *facq_window_fun(gsize n_samples,FacqWindowFunType type);

G_END_DECLS

#endif

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
#ifndef _FACQ_UNITS_H_
#define _FACQ_UNITS_H_

G_BEGIN_DECLS

#ifndef __GTK_DOC_IGNORE__
#define FACQ_UNIT_VOLT "Volts (V)"
#define FACQ_UNIT_MA   "MiliAmperes (mA)"
#define FACQ_UNIT_NONE "Unitless (None)"
#endif

typedef enum chan_unit {
	UNIT_V = 0, 
	UNIT_MA = 1, 
	UNIT_U = 2, 
	/*< private >*/
	UNIT_NUMBER
} FacqUnits;

const gchar *facq_units_type_to_human(FacqUnits unit);

G_END_DECLS

#endif

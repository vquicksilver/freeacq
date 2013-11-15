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
#include <glib.h>
#include "facqi18n.h"
#include "facqunits.h"

/**
 * SECTION:facqunits
 * @title:FacqUnits
 * @include:facqunits.h
 * @short_description:handles different kind of units.
 *
 * This module handles different kind of units.
 * To see available units see #FacqUnits enumeration.
 * The value of each kind of unit is not arbitrary, and should be keep as is to
 * keep compatibility with the comedi library.
 *
 * You can obtain a valid printable text for each kind of unit with
 * facq_units_type_to_human().
 */

/**
 * FacqUnits:
 * @UNIT_V: Identifier used for Volts.
 * @UNIT_MA: Identifier used for mA (mili Amperes).
 * @UNIT_U: Identifier used for a Unknown unit.
 *
 * This enumeration contains unit values.
 */

/**
 * facq_units_type_to_human:
 * @unit: A valid #FacqUnits enumerated value.
 *
 * Convert from numeric identifier for a unit to a printable text string.
 *
 * Returns: A human readable text corresponding to the unit, @unit.
 */
const gchar *facq_units_type_to_human(FacqUnits unit)
{
	const gchar * const unit_types[]=
	{
		N_(FACQ_UNIT_VOLT), N_(FACQ_UNIT_MA), N_(FACQ_UNIT_NONE)
	};
	const gchar *ret = NULL;
	
	if(unit < UNIT_NUMBER)
		ret = _(unit_types[unit]);
	return ret;
}

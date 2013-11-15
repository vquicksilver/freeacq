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
#include "gdouble.h"
#ifndef GDOUBLE_TO_BE
/**
 * SECTION:gdouble
 * @short_description: Provides double real precision methods.
 * @title:gdouble
 * @include:gdouble.h
 *
 * This module provides missing operations for the glib's data type #gdouble.
 * This type provides a way to use double real numbers following the IEEE754
 * standard.
 */

/**
 * GDOUBLE_TO_BE:
 * @data: The input @gdouble.
 *
 * Translates to big endian a , @data, #gdouble or if @data it's already 
 * in big endian format it converts it to little endian.
 *
 * Returns: A #gdouble in big endian or little endian format.
 */
gdouble GDOUBLE_TO_BE(gdouble data)
{
	/* This snippet of code is taken from GDOUBLE_TO_BE
	 * implementation of GStreamer. */
        union {
		guint64 input;
		gdouble output;
	} uswap;

	uswap.output = data;
	uswap.input = GUINT64_TO_BE(uswap.input);
	return uswap.output;
/*
	guint8 *pd = NULL,*pb = NULL,i = 0;
        gdouble back = 0;

        if(G_BYTE_ORDER == G_LITTLE_ENDIAN){
                pd = (guint8 *)&data;
                pb = (guint8 *)&back;
                for(i = 0;i < 8;i++){
                        pb[i] = pd[7-i];
                }
                return back;
        }
        return data;
*/
}
#endif

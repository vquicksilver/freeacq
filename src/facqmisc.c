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
#include <math.h>
#include "facqmisc.h"

/**
 * SECTION:facqmisc
 * @title:FacqMisc
 * @short_description: Miscellaneous, but not less important, functions.
 *
 * This module includes some miscellaneous functions, see each function
 * description for more details.
 */

/** 
 * facq_misc_period_to_chunk_size:
 * @period: The period between samples in seconds.
 * @bps: The number of bytes per sample.
 * @n_channels: The number of channels.
 *
 * Returns a number of bytes divisible by bps*n_channels product, that
 * corresponds to the nearest period value. This value will be passed
 * to the pipeline to determine the size of each chunk.
 * As a result the period value could be modified if needed.
 * Use this function to get a correlation between the chunk size and the
 * period.
 *
 * Returns: The number of bytes per chunk corresponding to @period.
 */
gsize facq_misc_period_to_chunk_size(gdouble period,guint bps,guint n_channels)
{
	gdouble slice_size = bps*n_channels;
	gdouble tmp_size = 0, n_times = 0;

	if(period >= 1){
		return slice_size;
	}
	else {
		tmp_size = ((1/period) * slice_size);
		if( fmod(tmp_size,slice_size) == 0){
			return (gsize)tmp_size;
		}
		else {
			/* round tmp_size to the nearest upper integer that can
			 * be divided by slice_size with a remainder of 0 */
			tmp_size = round( (1/period) * slice_size);
			n_times = (tmp_size/slice_size);
			n_times = round(n_times);
			return (gsize)(n_times*slice_size);
		}
	}
}

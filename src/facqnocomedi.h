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
#ifndef _FREEACQ_NO_COMEDI_H
#define _FREEACQ_NO_COMEDI_H

G_BEGIN_DECLS

#ifndef __GTK_DOC_IGNORE__
/* definitions taken from comedi.h and comedilib.h */
#define AREF_GROUND     0x00
#define AREF_COMMON     0x01
#define AREF_DIFF       0x02
#define AREF_OTHER      0x03    
#define CR_ALT_FILTER   (1<<26)
#define CR_DITHER       CR_ALT_FILTER
#define CR_DEGLITCH     CR_ALT_FILTER
#define CR_ALT_SOURCE   (1<<27)
#define CR_EDGE 	(1<<30)
#define CR_INVERT       (1<<31)
#define CR_CHAN(a)      ((a)&0xffff)
#define CR_RANGE(a)     (((a)>>16)&0xff)
#define CR_AREF(a)      (((a)>>24)&0x03)
#define CR_FLAGS_MASK   0xfc000000
#define CR_PACK(chan,rng,aref)  ( (((aref)&0x3)<<24) | (((rng)&0xff)<<16) | (chan) )
#define CR_PACK_FLAGS(chan, range, aref, flags) (CR_PACK(chan, range, aref) | ((flags) & CR_FLAGS_MASK))

enum comedi_conversion_direction
{
        COMEDI_TO_PHYSICAL,
        COMEDI_FROM_PHYSICAL
};
#endif

G_END_DECLS

#endif

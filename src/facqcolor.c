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
#include "facqcolor.h"

/**
 * SECTION:facqcolor
 * @title:FacqColor
 * @short_description:Manages a uniform set of colors
 * @include:facqcolor.h
 *
 * This module provides color management. Color are used in different programs
 * when more than one signal can be seen at the same time, to make more easy
 * to the eye distinguish different channels.
 *
 * Usually you get a list of channels, and you want to get a color from each
 * channel, for doing this, the more easy way is to get the channel index,
 * that is the position that the channel has in the list, and assign a color to
 * it. For doing this you can use the facq_gdk_color_from_index() function.
 *
 * The other function, facq_gdk_pixbuf_from_index() created a small pixbuf
 * with a square filled with the color that corresponds to index, it's used
 * by the #FacqLegend module.
 *
 */

static const gchar * const white[] = {
	"snow", "GhostWhite", "WhiteSmoke", "gainsboro", "FloralWhite", "OldLace", "linen", "AntiqueWhite",
	"PapayaWhip", "BlanchedAlmond", "bisque", "PeachPuff", "NavajoWhite", "moccasin", "cornsilk", "ivory",
	"LemonChiffon", "seashell", "honeydew", "MintCream", "azure", "AliceBlue", "lavender", "LavenderBlush",
	"MistyRose", "white", "ivory2", "ivory3", "ivory4", "honeydew2", "honeydew3", "honeydew4"
};

static const gchar * const blue[] = {
	"MidnightBlue", "navy", "NavyBlue", "CornflowerBlue", "DarkSlateBlue", "SlateBlue",
	"MediumSlateBlue", "LightSlateBlue", "MediumBlue", "RoyalBlue", "blue", "DodgerBlue",
	"DeepSkyBlue", "SkyBlue", "LightSkyBlue", "SteelBlue", "LightSteelBlue", "LightBlue",
	"PowderBlue", "PaleTurquoise", "DarkTurquoise", "MediumTurquoise", "turquoise",
	"cyan", "LightCyan", "CadetBlue", "MediumAquamarine", "aquamarine", "SlateBlue1",
	"DarkBlue", "DarkCyan", "SlateBlue2"
};

static const gchar * const green[] = {
	"DarkGreen", "DarkOliveGreen", "DarkSeaGreen", "SeaGreen", "MediumSeaGreen",
	"LightSeaGreen", "PaleGreen", "SpringGreen", "LawnGreen", "green", "chartreuse",
	"MediumSpringGreen", "GreenYellow", "LimeGreen", "YellowGreen", "ForestGreen",
	"OliveDrab", "DarkSeaGreen1", "DarkSeaGreen2", "DarkSeaGreen3", "DarkSeaGreen4",
	"SeaGreen1", "SeaGreen2", "SeaGreen3", "SeaGreen4", "PaleGreen1", "PaleGreen2",
	"PaleGreen3", "PaleGreen4", "SpringGreen2", "SpringGreen3", "SpringGreen4"
};

static const gchar * const yellow[] = {
	"DarkKhaki", "khaki", "PaleGoldenrod", "LightGoldenrodYellow", "LightYellow", "yellow",
	"gold", "LightGoldenrod", "goldenrod", "DarkGoldenrod", "LightGoldenrod1",
	"LightGoldenrod2", "LightGoldenrod3", "LightGoldenrod4", "LightYellow2", "LightYellow3",
	"LightYellow4", "yellow2", "yellow3", "yellow4", "gold2", "gold3", "gold4", "goldenrod1",
	"goldenrod2", "goldenrod3", "goldenrod4", "DarkGoldenrod1", "DarkGoldenrod2",
	"DarkGoldenrod3", "DarkGoldenrod4", "khaki1"
};

static const gchar * const brown[] = {
	"RosyBrown", "IndianRed", "SaddleBrown", "sienna", "peru", "burlywood", "beige", "wheat",
	"SandyBrown", "tan", "chocolate", "firebrick", "brown", "firebrick1", "firebrick2", "firebrick3",
	"firebrick4", "brown1", "brown2", "brown3", "brown4", "chocolate1", "chocolate2", "chocolate3",
	"chocolate4", "tan1", "tan2", "tan3", "tan4", "wheat1", "wheat2", "wheat3"
};

static const gchar * const red[] = {
	"red", "red2", "red3", "red4", "DarkRed", "IndianRed1", "IndianRed2", "IndianRed3", "IndianRed4",
	"orange", "DarkOrange", "coral", "LightCoral", "tomato", "OrangeRed", "orange2", "orange3",
	"orange4", "DarkOrange1", "DarkOrange2", "DarkOrange3", "DarkOrange4", "coral1", "coral2",
	"coral3", "coral4", "tomato2", "tomato3", "tomato4", "OrangeRed2", "OrangeRed3", "OrangeRed4"
};

static const gchar *const pink[] = {
	"DarkSalmon", "salmon", "LightSalmon", "HotPink", "DeepPink", "pink", "LightPink", "DeepPink2",
	"DeepPink3", "DeepPink4", "HotPink2", "HotPink3", "HotPink4", "pink1", "pink2", "pink3",
	"pink4", "LightPink1", "LightPink2", "LightPink3", "LightPink4", "salmon1", "salmon2", "salmon3",
	"salmon4", "LightSalmon2", "LightSalmon3", "LightSalmon4", "PaleVioletRed", "PaleVioletRed1", "PaleVioletRed2"
};

static const gchar *const violet[] = {
	"MediumVioletRed", "VioletRed", "magenta", "violet", "plum", "orchid", "MediumOrchid",
	"DarkOrchid", "DarkViolet", "BlueViolet", "purple", "MediumPurple", "thistle",
	"PaleVioletRed3", "PaleVioletRed4", "VioletRed1", "VioletRed2", "VioletRed3",
	"VioletRed4", "magenta2", "magenta3", "magenta4", "orchid1", "orchid2", "orchid3",
	"orchid4", "plum1", "plum2", "plum3", "plum4", "MediumOrchid1", "MediumOrchid2"
};

/**
 * facq_gdk_color_from_index:
 * @chan_index: The channel index, from 0 to 255.
 * @color: A #GdkColor pointer, that will store the result.
 *
 * Makes a 1-1 correspondence between a number and a color,
 * storing the result in @color. So for each number you will get
 * a different color, as long as you use a @chan_index between 0
 * and 255.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 *
 */
gboolean facq_gdk_color_from_index(guint chan_index,GdkColor *color)
{
	guint group = 0, index = 0;
	const gchar *named_color = NULL;

	if(!color)
		return FALSE;

	if(chan_index >= 0 && chan_index <= 255){
		group = chan_index % 8;
		index = chan_index % 32;
		switch(group){
		case 0:
			named_color = white[index];
		break;
		case 1:
			named_color = blue[index];
		break;
		case 2:
			named_color = green[index];
		break;
		case 3:
			named_color = yellow[index];
		break;
		case 4:
			named_color = brown[index];
		break;
		case 5:
			named_color = red[index];
		break;
		case 6:
			named_color = pink[index];
		break;
		case 7:
			named_color = violet[index];
		break;
		}
	}
	if(named_color){
		return gdk_color_parse(named_color,color);
	}
	return FALSE;
}

/**
 * facq_gdk_pixbuf_from_index:
 * @chan_index: The channel index, a number between 0 and 255.
 *
 * Creates a #GdkPixbuf object, with a black square (black border)
 * filled with the color that corresponds to @index.
 *
 * Returns: A #GdkPixbuf with a filled square. You must free it with
 * g_object_unref() when no longer needed.
 */
GdkPixbuf *facq_gdk_pixbuf_from_index(guint chan_index)
{
	GdkColor color;
	GdkPixbuf *ret = NULL;
	guchar *data = NULL, *pix = NULL;
	gint rowstride = 0;
	gchar i = 0, j = 0;

	g_return_val_if_fail(chan_index >= 0 && chan_index <= 255,NULL);

	/* 24x24 pixels x 4 bytes per pixel equals 24x24x4 bytes for a RGB pixbuf */
	ret = gdk_pixbuf_new(GDK_COLORSPACE_RGB,TRUE,8,24,24);
	data = gdk_pixbuf_get_pixels(ret);
	rowstride = gdk_pixbuf_get_rowstride(ret);
	facq_gdk_color_from_index(chan_index,&color);

	for(i = 0;i < 24;i++){
		for(j = 0; j < 24;j++){
			pix = data + j*rowstride + i*4;
			if(i == 0 || j == 0 || i == 23 || j == 23){
				pix[0] = 0;
				pix[1] = 0;
				pix[2] = 0;
				pix[3] = 255;
			}
			else {
				pix[0] = color.red;
				pix[1] = color.green;
				pix[2] = color.blue;
				pix[3] = 255;
			}
		}
	}

	return ret;
}

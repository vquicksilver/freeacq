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
#include <string.h>
#include <math.h>
#include <glib.h>
#include "facqwindowfun.h"

/** 
 *  SECTION:facqwindowfun
 *  @title:FacqWindowFun
 *  @short_description:Get various window functions as 1d real vectors.
 *  @include:facqwindowfun.h
 *  
 *  This module hides all the internal details needed for dealing with window
 *  functions. In signal processing a window function is a mathematical function
 *  that is zero valued outside of some chosen interval.
 *
 *  Use facq_window_fun() to obtain a real vector with the desired function. To see
 *  supported types see #FacqWindowFunType.
 *
 *  <sect1>
 *    <title>Window Functions</title>
 *    <para>
 *     Applications of window functions include spectral analisys, filter design (FIR filters),
 *     and beamforming. In typical applications, the window functions used are
 *     non-negative smooth "bell-shaped" curves, though rectangle, triangle, and
 *     other functions can be used.
 *
 *     A more general definition of window functions does not require them to be
 *     identically zero outside an interval, as long as the product of the window
 *     multiplied by its argument is square integrable, that is, that the function
 *     goes sufficiently rapidly toward zero.
 *    </para>
 *   </sect1>
 */

/**
 * FacqWindowFunType:
 * @FACQ_WF_TYPE_REC: Rectangular window.
 * @FACQ_WF_TYPE_TRI: Triangular window.
 * @FACQ_WF_TYPE_BAR: Bartlett window.
 * @FACQ_WF_TYPE_WEL: Welch window.
 * @FACQ_WF_TYPE_HAN: Hanning window.
 * @FACQ_WF_TYPE_HAM: Hamming window.
 * @FACQ_WF_TYPE_FLA: Flat-Top window.
 * @FACQ_WF_TYPE_BLA: Blackman window.
 *
 * Enumeration describing the various types of window functions supported.
 */

#define SQUARE(x) x*x

#define REC(n,N) 1
#define TRI(n,N) 1-ABS( (n-((N-1)/2.0))/((N+1)/2.0) )
#define BAR(n,N) 1-ABS( (n-((N-1)/2.0))/((N-1)/2.0) )
#define WEL(n,N) 1-SQUARE( (n-(   (  (N-1) / 2.0 )))/( (N+1) / 2.0 ) )
#define HAN(n,N) 0.5*( 1 - cos(   (2*G_PI*n)/(N-1)   ) )
#define HAM(n,N) 0.54-( 0.46*cos(   (2*G_PI*n)/(N-1)   ) )

#define FLA(n,N) 1                    \
- (  1.93*cos( (2*G_PI*n)/(N-1) )  )  \
+ (  1.29*cos( (4*G_PI*n)/(N-1) )  )  \
- (  0.388*cos( (6*G_PI*n)/(N-1) )  ) \
+ (  0.028*cos( (8*G_PI*n)/(N-1) )  ) 

#define BLA_0 (7938/18608.0)
#define BLA_1 (9240/18608.0)
#define BLA_2 (1430/18608.0)
#define BLA(n,N) BLA_0                    \
- ( BLA_1*cos( (2*G_PI*n)/(N-1) ) )       \
+ ( BLA_2*cos( (4*G_PI*n)/(N-1) ) )       \

#define DISPATCHER(y,x,N,type)    \
switch(type){                     \
	case FACQ_WF_TYPE_REC:    \
		y = REC(x,N);     \
	break;                    \
	case FACQ_WF_TYPE_BAR:    \
		y = BAR(x,N);     \
	break;                    \
	case FACQ_WF_TYPE_TRI:    \
		y = TRI(x,N);     \
	break;                    \
	case FACQ_WF_TYPE_WEL:    \
		y = WEL(x,N);     \
	break;                    \
	case FACQ_WF_TYPE_HAN:    \
		y = HAN(x,N);     \
	break;                    \
	case FACQ_WF_TYPE_HAM:    \
		y = HAM(x,N);     \
	break;                    \
	case FACQ_WF_TYPE_FLA:    \
		y = FLA(x,N);     \
	break;                    \
	case FACQ_WF_TYPE_BLA:    \
		y = BLA(x,N);     \
	break;                    \
}

/**
 * facq_window_fun:
 * @n_samples: The desired number of points in the returned vector.
 * @type: The type of window function to return, see #FacqWindowFunType for
 * available types.
 *
 * Computes @n_samples of the window function determined by @type, and creates a
 * vector containing the computed samples.
 *
 * Returns: A real vector, you must free it with g_free().
 */
gdouble *facq_window_fun(gsize n_samples,FacqWindowFunType type)
{
	gdouble *ret = NULL;
	guint i = 0;

	ret = g_malloc0_n(n_samples,sizeof(gdouble));

	for(i = 0;i < n_samples;i++)
		DISPATCHER(ret[i],i,n_samples,type);

	return ret;
}

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
#include "facqcomplex.h"

/**
 * SECTION:facqcomplex
 * @short_description:provides complex number support
 * @title:FacqComplex
 * @include:facqcomplex.h
 *
 * This module provides a way to deal with complex numbers.
 *
 *  Complex  numbers  are  numbers  of the form z = a+b*i, where a and b are real
 *  numbers and i = sqrt(-1), so that i*i = -1.
 *  There are other ways to represent that number.  The pair (a,b) of  real  numbers  
 *  may be viewed as a point in the plane, given by X- and Y-coordinates.
 *  This same point may also be described by giving  the  pair  of  real  numbers
 *  (r,phi),  where  r is the distance to the origin O, and phi the angle between
 *  the X-axis and the line Oz.  Now z = r*exp(i*phi) = r*(cos(phi)+i*sin(phi)).
 *
 * You can use facq_complex_abs() to obtain the module of a complex number,
 * facq_complex_angle() to obtain the phase.
 * To set the values of a complex number use facq_complex_set_r() for setting
 * the real part and facq_complex_set_i() for setting the imaginary part.
 */

GQuark facq_complex_error_quark(void)
{
        return g_quark_from_static_string("facq-complex-error-quark");
}

static void math_error_first(void)
{
	errno = 0;
	feclearexcept(FE_ALL_EXCEPT);
}

/**
 * facq_complex_abs:
 * @c: A #FacqComplex number.
 * @err: (allow-none): A #GError.
 *
 * Returns the "modulus" or "radius" of a complex number.
 *
 * Returns: On success the length of right-angled triangle with sides of length
 * x and y (@c = x+I*y). In case of error 0 is returned and @err is set.
 */
gdouble facq_complex_abs(FacqComplex c,GError **err)
{
	GError *local_err = NULL;
	gdouble ret = 0, x = 0,y = 0;

	x = creal(c);
	y = cimag(c);

	math_error_first();

	ret = hypot(x,y);

	if(fetestexcept(FE_OVERFLOW)){
		if(err)
			g_set_error_literal(err,FACQ_COMPLEX_ERROR,
					FACQ_COMPLEX_ERROR_OVERFLOW,"Overflow Error");
		ret = 0;
	}
	if(fetestexcept(FE_UNDERFLOW)){
		if(err)
			g_set_error(err,FACQ_COMPLEX_ERROR,
					FACQ_COMPLEX_ERROR_UNDERFLOW,"Underflow Error");
		ret = 0;
	}

	return ret;
}

/**
 * facq_complex_angle:
 * @c: A FacqComplex number.
 *
 * Returns the "phase angle" of a complex number.
 *
 * Returns: A value between [-pi,pi].
 */
gdouble facq_complex_angle(FacqComplex c)
{
	/* Note that carg can't fail because atan() can't fail, so there isn't
	 * any need for error checking */
	return carg(c);
}

/**
 * facq_complex_set_r:
 * @c: A pointer to a #FacqComplex.
 * @real: A real number.
 *
 * Sets the real part of the #FacqComplex, @c, to the value @real.
 */
void facq_complex_set_r(FacqComplex *c,gdouble real)
{
	g_memmove(c,&real,sizeof(gdouble));
}

/**
 * facq_complex_set_i:
 * @c: A pointer to a #FacqComplex.
 * @imag: A real number.
 *
 * Sets the imaginary part of the #FacqComplex, @c, to the value @imag.
 */
void facq_complex_set_i(FacqComplex *c,gdouble imag)
{
	gchar *p = (gchar *)c;

	g_memmove(p+8,&imag,sizeof(gdouble));
}

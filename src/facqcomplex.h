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
#ifndef _FREEACQ_COMPLEX_H_
#define _FREEACQ_COMPLEX_H_

#include <string.h>
#include <math.h>
#include <errno.h>
#include <fenv.h>
#include <complex.h>

/* See man math_error for details of error handling of math funcs. */

G_BEGIN_DECLS

#ifndef __GTK_DOC_IGNORE__
#define FACQ_COMPLEX_ERROR facq_complex_error_quark()
#endif

/**
 * FacqComplexError:
 * @FACQ_COMPLEX_ERROR_FAILED: Generic error identifier.
 * @FACQ_COMPLEX_ERROR_UNDERFLOW: Error caused by underflow.
 * @FACQ_COMPLEX_ERROR_OVERFLOW: Error caused by overflow.
 */
typedef enum {
	FACQ_COMPLEX_ERROR_FAILED,
	FACQ_COMPLEX_ERROR_UNDERFLOW,
	FACQ_COMPLEX_ERROR_OVERFLOW
} FacqComplexError;

/**
 * FacqComplex:
 *
 * A type for operating with complex numbers.
 */
typedef double complex FacqComplex;

gdouble facq_complex_abs(FacqComplex c,GError **err);
gdouble facq_complex_angle(FacqComplex c);
void facq_complex_set_r(FacqComplex *c,gdouble real);
void facq_complex_set_i(FacqComplex *c,gdouble imag);

G_END_DECLS

#endif

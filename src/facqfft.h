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
#ifndef _FREEACQ_FFT_H_
#define _FREEACQ_FFT_H_

#include "facqlog.h"
#include "facqcomplex.h"

#ifndef __GTK_DOC_IGNORE__
#define FACQ_FFT_F 0
#define FACQ_FFT_B 1
#endif

#if USE_FFTW3
#include <fftw3.h>
#endif

G_BEGIN_DECLS

#define FACQ_FFT_ERROR facq_fft_error_quark()

/**
 * FacqFFTError:
 * @FACQ_FFT_ERROR_FAILED: Some error was detected.
 *
 * Enum values used for error reporting.
 */
typedef enum {
	FACQ_FFT_ERROR_FAILED
} FacqFFTError;

/**
 * FacqFFTDir:
 * @FACQ_FFT_DIR_FORWARD: Compute a forward transform (Time to Frequency
 * domain).
 * @FACQ_FFT_DIR_BACKWARD: Compute a backward transform (Frequency to time
 * domain).
 *
 * Enum values for valid DFT directions.
 */
typedef enum facq_fft_direction {
	FACQ_FFT_DIR_FORWARD = FACQ_FFT_F,
	FACQ_FFT_DIR_BACKWARD = FACQ_FFT_B
} FacqFFTDir;

/**
 * FacqFFTType:
 * @FACQ_FFT_TYPE_C2C: Make a Complex to Complex transform.
 * @FACQ_FFT_TYPE_R2C: Make a Real to Complex transform.
 * @FACQ_FFT_TYPE_C2R: Make a Complex to Real transform.
 *
 * Enum values for the various types of supported transforms.
 */
typedef enum facq_fft_type {
	FACQ_FFT_TYPE_C2C,
	FACQ_FFT_TYPE_R2C,
	FACQ_FFT_TYPE_C2R
} FacqFFTType;

#define FACQ_TYPE_FFT_CONFIG (facq_fft_config_get_type ())
#define FACQ_FFT_CONFIG(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_FFT_CONFIG, FacqFFTConfig))
#define FACQ_FFT_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_FFT_CONFIG, FacqFFTConfigClass))
#define FACQ_IS_FFT_CONFIG(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_FFT_CONFIG))
#define FACQ_IS_FFT_CONFIG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_FFT_CONFIG))
#define FACQ_FFT_CONFIG_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_FFT_CONFIG, FacqFFTConfigClass))

typedef struct _FacqFFTConfig FacqFFTConfig;
typedef struct _FacqFFTConfigClass FacqFFTConfigClass;
typedef struct _FacqFFTConfigPrivate FacqFFTConfigPrivate;

struct _FacqFFTConfig {
	/*< private >*/
	GObject parent_instance;
	/*< public >*/
	gpointer out;
	gsize len;
	/*< private >*/
	FacqFFTConfigPrivate *priv;
};

struct _FacqFFTConfigClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_fft_config_get_type(void) G_GNUC_CONST;

FacqFFTConfig *facq_fft_config_new(gpointer input,gsize n,FacqFFTDir dir,FacqFFTType type,GError **error);
void facq_fft_config_free(FacqFFTConfig *config);
gpointer facq_fft_compute(FacqFFTConfig *config);

gpointer facq_fft_malloc(gsize size);
void facq_fft_free(gpointer ptr);

G_END_DECLS

#endif

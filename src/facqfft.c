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
#include <gio/gio.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqfft.h"

/**
 * SECTION:facqfft
 * @title:FacqFFT
 * @include:facqfft.h
 * @short_description: Computes DFT using FFTW3 library or a Cooley-Tukey based
 * algorithm.
 * @see_also: #GInitable
 * 
 * This module provides a way to compute DFT (Discrete Fourier Transforms), or
 * DTFT (Discrete Time Fourier Transform).
 *
 * For using it you have to create a #FacqFFTConfig object with
 * facq_fft_config_new(). This object contains all the needed parameters for the
 * module to take care of all the internal details.
 *
 * The module behaviour depends on the compilation time options. If the module is
 * compiled with support for the <ulink url="http://www.fftw.org">FFTW3</ulink> library, the module will be able to
 * compute complex and real inputs, and will be able to handle complex and real
 * outputs. See the #FacqFFTType enumeration for supported inputs and outputs.
 *
 * If <ulink url="http://www.fftw.org">FFTW3</ulink> isn't available or is disabled at compilation time, a Cooley-Tukey
 * algorithm will be used to compute the DFT.
 *
 * In both cases you can do forward (Time to Frequency) and backward (Frequency
 * to time) transforms. (See #FacqFFTDir).
 *
 * To compute the transform you should call facq_fft_compute().
 *
 * To finish destroy the #FacqFFTConfig object with facq_fft_config_free().
 *
 * # Usage example #
 * <para>
 * <informalexample>
 * <programlisting>
 * #include <glib.h>
 * #include <gio/gio.h>
 * #include "facqfft.h"
 * ...
 * FacqFFTConfig *config = NULL;
 * FacqComplex *input = NULL, out = NULL;
 * GError *err = NULL;
 *
 * input = facq_fft_malloc(sizeof(FacqComplex)*N);
 * ...
 * config = facq_fft_config_new(input,N,FACQ_FFT_DIR_FORWARD,FACQ_FFT_TYPE_C2C,&err);
 * ...
 * out = facq_fft_compute(config);
 * ...
 * g_print("Result length %u\n",config->len);
 * for(i = 0;i < config->len;i++)
 *	g_print("%f %fi\n",creal(config->out[i],cimag(config->out[i])));
 * ...
 * facq_fft_config_free(config);
 * facq_fft_free(input);
 * ...
 * </programlisting>
 * </informalexample>
 * </para>
 */

/** 
 * FacqFFTConfig:
 * @out: A pointer to the zone of memory where the compute result will be
 * stored. This zone can contain #FacqComplex samples or #gdouble samples.
 * @len: The number of samples contained in @out.
 *
 * Contains the public fields of #FacqFFTConfig.
 */

static void facq_fft_config_initable_iface_init(GInitableIface *iface);
static gboolean facq_fft_config_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);

G_DEFINE_TYPE_WITH_CODE(FacqFFTConfig,facq_fft_config,G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_fft_config_initable_iface_init));

GQuark facq_fft_error_quark(void)
{
        return g_quark_from_static_string("facq-fft-error-quark");
}

enum {
	PROP_0,
	PROP_INPUT,
	PROP_SIZE,
	PROP_DIR,
	PROP_TYPE
};

struct _FacqFFTConfigPrivate {
	GError *construct_error;
	gpointer input;
	gsize n;
	FacqFFTDir dir;
	FacqFFTType type;
#if USE_FFTW3
	fftw_plan plan;
	guint flags;
#endif
};

/* GObject magic */
static void facq_fft_config_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqFFTConfig *config = FACQ_FFT_CONFIG(self);

	switch(property_id){
	case PROP_SIZE: g_value_set_uint(value,config->priv->n);
	break;
	case PROP_INPUT: g_value_set_pointer(value,config->priv->input);
	break;
	case PROP_DIR: g_value_set_uint(value,config->priv->dir);
	break;
	case PROP_TYPE: g_value_set_uint(value,config->priv->type);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(config,property_id,pspec);
	}
}

static void facq_fft_config_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqFFTConfig *config = FACQ_FFT_CONFIG(self);

	switch(property_id){
	case PROP_SIZE: config->priv->n = g_value_get_uint(value);
	break;
	case PROP_INPUT: config->priv->input = g_value_get_pointer(value);
	break;
	case PROP_DIR: config->priv->dir = g_value_get_uint(value);
	break;
	case PROP_TYPE: config->priv->type = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(config,property_id,pspec);
	}
}

static void facq_fft_config_finalize(GObject *self)
{
	FacqFFTConfig *config = FACQ_FFT_CONFIG(self);

	g_clear_error(&config->priv->construct_error);

#if USE_FFTW3
	if(config->priv->plan)
		fftw_destroy_plan(config->priv->plan);
#endif
	facq_fft_free(config->out);
}

static void facq_fft_config_constructed(GObject *self)
{
	FacqFFTConfig *config = FACQ_FFT_CONFIG(self);
	gsize N = config->priv->n;
	gint sign = config->priv->dir ? -1 : 1;

#if USE_FFTW3
	switch(config->priv->type){
	case FACQ_FFT_TYPE_C2C:
		config->out = facq_fft_malloc(N*sizeof(FacqComplex));
	break;
	case FACQ_FFT_TYPE_R2C:
		if(N % 2 != 0)
			N = N-1;
		N = N/2+1;
		config->out = facq_fft_malloc(N*sizeof(FacqComplex));
	break;
	case FACQ_FFT_TYPE_C2R:
		config->out = facq_fft_malloc(N*sizeof(gdouble));
	break;
	}
#else
	//TODO: Do something similar to the above code for our own FFT.
	config->out = facq_fft_malloc(N*sizeof(FacqComplex));
#endif

	config->len = N;

#if USE_FFTW3
	switch(config->priv->type){
	case FACQ_FFT_TYPE_C2C:
	config->priv->plan = 
		fftw_plan_dft_1d(N,config->priv->input,
					config->out,sign,
						config->priv->flags);
	break;
	case FACQ_FFT_TYPE_R2C:
	config->priv->plan =
		fftw_plan_dft_r2c_1d(config->priv->n
					,config->priv->input,
						config->out,
							config->priv->flags);
	break;
	case FACQ_FFT_TYPE_C2R:
	config->priv->plan =
		fftw_plan_dft_c2r_1d(config->priv->n,config->priv->input,
					config->out,
						config->priv->flags);
	break;
	default: 
	return;
	}

	if(!config->priv->plan){
		g_set_error_literal(&config->priv->construct_error,
				FACQ_FFT_ERROR,FACQ_FFT_ERROR_FAILED,
					"Input settings not supported");
	}
#endif
}

static void facq_fft_config_class_init(FacqFFTConfigClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqFFTConfigPrivate));

	object_class->set_property = facq_fft_config_set_property;
	object_class->get_property = facq_fft_config_get_property;
	object_class->finalize = facq_fft_config_finalize;
	object_class->constructed = facq_fft_config_constructed;

	g_object_class_install_property(object_class,PROP_INPUT,
					g_param_spec_pointer("input",
						"The input data",
						"A pointer to the input data samples",
						G_PARAM_READWRITE |
						G_PARAM_STATIC_STRINGS |
						G_PARAM_CONSTRUCT_ONLY
						));

	g_object_class_install_property(object_class,PROP_SIZE,
					g_param_spec_uint("size",
						"The input size",
						"The input size (The number of samples)",
						1,
						G_MAXUINT,
						1,
						G_PARAM_READWRITE |
						G_PARAM_STATIC_STRINGS |
						G_PARAM_CONSTRUCT_ONLY
						));

	g_object_class_install_property(object_class,PROP_DIR,
					g_param_spec_uint("dir",
						"The FFT direction",
						"The FFT direction forward or backward",
						FACQ_FFT_DIR_FORWARD,
						FACQ_FFT_DIR_BACKWARD,
						FACQ_FFT_DIR_FORWARD,
						G_PARAM_READWRITE |
						G_PARAM_CONSTRUCT_ONLY |
						G_PARAM_STATIC_STRINGS
						));

	g_object_class_install_property(object_class,PROP_TYPE,
					g_param_spec_uint("type",
						"The FFT transform type",
						"The FFT transform type, eg:C2C",
						FACQ_FFT_TYPE_C2C,
						FACQ_FFT_TYPE_C2R,
						FACQ_FFT_TYPE_C2C,
						G_PARAM_READWRITE |
						G_PARAM_CONSTRUCT_ONLY |
						G_PARAM_STATIC_STRINGS
						));
}

static void facq_fft_config_init(FacqFFTConfig *config)
{
	config->priv = G_TYPE_INSTANCE_GET_PRIVATE(config,FACQ_TYPE_FFT_CONFIG,FacqFFTConfigPrivate);
	config->priv->construct_error = NULL;
	config->priv->n = 0;
	config->priv->input = config->out = NULL;
	config->priv->dir = FACQ_FFT_DIR_FORWARD;
	config->priv->type = FACQ_FFT_TYPE_C2C;
#if USE_FFTW3
	config->priv->plan = NULL;
	config->priv->flags = FFTW_ESTIMATE;
#endif
}

/* GInitable interface */
static void facq_fft_config_initable_iface_init(GInitableIface *iface)
{
	iface->init = facq_fft_config_initable_init;
}

static gboolean facq_fft_config_initable_init(GInitable *initable,GCancellable *cancellable,GError **error)
{
	FacqFFTConfig *config;

	g_return_val_if_fail(FACQ_IS_FFT_CONFIG(initable),FALSE);
	config = FACQ_FFT_CONFIG(initable);
	if(cancellable != NULL){
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                           "Cancellable initialization not supported");
      		return FALSE;
	}
	if(config->priv->construct_error){
		if(error)
			*error = g_error_copy(config->priv->construct_error);
		return FALSE;
	}
	return TRUE;
}

static void facq_fft(FacqFFTConfig *config)
{
	/* TODO: for the moment use FFTW3
	 * We can implement later a function based on Cooley-Tukey algorithm.
	 * Also it would be better to use 2^m samples both when using FFTW3 and
	 * when using this function. FFTW3 supports arbitrary sizes but it's
	 * hard to do that in a custom implementation. Interesting links for
	 * doing this function are:
	 *
	 * http://en.literateprograms.org/Cooley-Tukey_FFT_algorithm_%28C%29
	 * http://en.wikipedia.org/wiki/Cooley-Tukey_FFT_algorithm
	 * http://jeremykun.com/2012/07/18/the-fast-fourier-transform/
	 * http://cnx.org/content/m16334/1.13/
	 * http://www.cmlab.csie.ntu.edu.tw/cml/dsp/training/coding/transform/fft.html
	 *
	 * Also we can use other libraries like GSL (GNU Scientific library) or
	 * FFTPACK.
	 */
}

/**
 * facq_fft_config_new:
 * @input: A pointer to the input data. It can be real data (gdouble) or complex
 * data #FacqComplex, depending on @dir value.
 * @n: The number of samples (Real or complex) in the input.
 * @dir: The direction of the Fourier transform, see #FacqFFTDir for valid
 * values. Note that R2C transforms are always forward transforms and C2R
 * transforms are always backward transforms.
 * @type: The type of transform that you want to compute. See #FacqFFTType for
 * valid values. Note that the type will condition the expected kind of input and
 * output.
 * @error: (allow-none): A #GError, for detailed info. If provided, it will be set in case of error.
 *
 * Creates a new #FacqFFTConfig object, that contains all the required info to
 * compute a DFT.
 *
 * Returns: A new #FacqFFTConfig object or %NULL in case of error.
 */
FacqFFTConfig *facq_fft_config_new(gpointer input,gsize n,FacqFFTDir dir,FacqFFTType type,GError **error)
{
	return FACQ_FFT_CONFIG(g_initable_new(FACQ_TYPE_FFT_CONFIG,
					      NULL,error,
					      "input",input,
					      "size",n,
					      "dir",dir,
					      "type",type,
					      NULL)
			      );
}

/**
 * facq_fft_compute:
 * @config: A #FacqFFTConfig object, created with facq_fft_config_new().
 * 
 * This function computes the DFT according to @config values.
 *
 * <note>
 *  <para>
 *  When using FFTW3 library the output follows the following rules:
 *
 *
 *  
 *  - The DFT results are stored in-order in the array out, with the 
 *  zero-frequency (DC) component in out[0].
 *
 *
 *
 *  - We use the standard “in-order” output ordering—the k-th 
 *  output corresponds to the frequency k/n (or k/T, where T is your total 
 *  sampling period). For those who like to think in terms of positive and 
 *  negative frequencies, this means that the positive frequencies are stored 
 *  in the first half of the output and the negative frequencies are stored 
 *  in backwards order in the second half of the output. (The frequency -k/n 
 *  is the same as the frequency (n-k)/n.)
 *
 *  See <ulink url="http://www.fftw.org/fftw3_doc/What-FFTW-Really-Computes.html#What-FFTW-Really-Computes">FFTW3
 *  reference </ulink> for more details.
 *  </para>
 * </note>
 *
 * Returns: A pointer to the computed data (It can be obtained also with
 * config->out pointer). To obtain the number of complex or
 * real samples in this memory area, you can check config->len.
 * When transforming real data, the FFT will be hermitian, in this case N/2
 * coefficients represent frecuencies from 0 to fs/2 (Nyquist), two consecutive
 * coefficients are spaced apart by fs/N Hz.
 */
gpointer facq_fft_compute(FacqFFTConfig *config)
{
#if USE_FFTW3
	gdouble *rout = NULL;
	FacqComplex *cout = NULL;
	guint i = 0;

	/* A plan created with the basic interface is only for doing one
	 * fftw_execute(plan). In other cases we would use the Advanced interface to
	 * say howmany times we want to execute a plan, but we can't know it
	 * cause we are going to do it in real time so we need to use here the 
	 * 4.6 New-array Execute Functions */

	switch(config->priv->type){
	case FACQ_FFT_TYPE_C2C:
		fftw_execute_dft(config->priv->plan,config->priv->input,config->out);
	break;
	case FACQ_FFT_TYPE_R2C:
		fftw_execute_dft_r2c(config->priv->plan,config->priv->input,config->out);
	break;
	case FACQ_FFT_TYPE_C2R:
		fftw_execute_dft_c2r(config->priv->plan,config->priv->input,config->out);
	break;
	default:
	break;
	}

	/* FFTW computes an unnormalized DFT. 
	 * Thus, computing a forward followed by a backward transform (or vice versa) 
	 * results in the original array scaled by n.
	 *
	 * So we solve this here for each sample do sample/n */

	if(config->priv->type == FACQ_FFT_TYPE_C2R){
		rout = config->out;
		for(i = 0;i < config->len;i++)
			rout[i]/=config->priv->n;
	}
	if(config->priv->dir == FACQ_FFT_DIR_BACKWARD &&
		config->priv->type == FACQ_FFT_TYPE_C2C){
		cout = config->out;
		for(i = 0;i < config->len;i++)
			facq_complex_set_r(&cout[i],creal(cout[i])/config->priv->n);
	}
#else
	facq_fft(config);
#endif
	return config->out;
}

/**
 * facq_fft_config_free:
 * @config: A #FacqFFTConfig object.
 *
 * Destroys a #FacqFFTConfig.
 * <para>
 *  <note>
 *   Note that you are responsible of freeing the input data.
 *  </note>
 * </para>
 */
void facq_fft_config_free(FacqFFTConfig *config)
{
	g_return_if_fail(FACQ_IS_FFT_CONFIG(config));
	g_object_unref(G_OBJECT(config));
}

/**
 * facq_fft_malloc:
 * @size: A the number of bytes you want to allocate.
 *
 * Allocates a memory area that can be used to store the input samples. The
 * returned memory area is aligned so it can be used with SIMD instructions.
 * This is required by the <ulink url="http://www.fftw.org">FFTW3</ulink> library.
 *
 * Returns: A pointer to the allocated memory area.
 * 
 * <para>
 *  <note>
 *   The allocated memory must be freed with facq_fft_free() and never with free(), g_free(), or delete().
 *  </note>
 * </para>
 */
gpointer facq_fft_malloc(gsize size)
{
#if USE_FFTW3
	return fftw_malloc(size);
#else
	return g_malloc0(size);
#endif
}

/**
 * facq_fft_free:
 * @ptr: A pointer to a memory area allocated with facq_fft_malloc().
 *
 * Frees the memory area pointed by @ptr.
 */
void facq_fft_free(gpointer ptr)
{
#if USE_FFTW3
	fftw_free(ptr);
#else
	g_free(ptr);
#endif
}

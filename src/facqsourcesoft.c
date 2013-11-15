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
#include <string.h>
#include <math.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqlog.h"
#include "facqmisc.h"
#include "facqresources.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqsource.h"
#include "facqsourcesoft.h"

/**
 * SECTION:facqsourcesoft
 * @short_description: A software based real data source.
 * @include:facqsourcesoft.h
 *
 * #FacqSourceSoft provides a software based real data source. It can operate
 * like a real data acquisition card, generating various kinds of waveforms.
 * It can be used also to test the system when no DAQ hardware is available.
 * 
 * When a new #FacqSourceSoft object is created, you must provide a period value
 * for the waveform, a sampling period value, and the kind of waveform you wish
 * to generate, remember that you must follow the Shanon theorem if you want to
 * retrieve a full waveform.
 * 
 * #FacqSourceSoft has a high dependency on the clock used by the operating
 * system, to correctly simulate the period of the waveform. Anyway due to the
 * nature of non real time operating systems you can't fully trust that the
 * period is respected. In any case if you put the right period value and your
 * combination of hardware and software supports it (That is, if the period is a
 * multiple of the so called quantum in operating systems theory) you will get
 * a more or less proper result. To let things more clear the result depends on
 * your hardware, the precision of your operating system sleep system call and
 * available cpu time.
 *
 * #FacqSourceSoft is able to generate various periodic waveforms, in one or more
 * channels including sines, cosines, flat waves, sawtooth and square waves. In
 * the case of sine,cosine,sawtooth and square waves if you choose to use
 * various channels a different phase will be added to each channel.
 * There is also the option to generate a random waveform in this case the
 * signal will vary in each channel using a software pseudorandom number
 * generator and won't be a periodic signal (It shouldn't be if you are
 * lucky enough).
 *
 * For creating a new #FacqSourceSoft you must call facq_source_soft_new(), 
 * to use it you must call first facq_source_start(), and then you must call 
 * in an iterative way facq_source_soft_poll() and facq_source_soft_read(), 
 * or use a #FacqStream that will do all those thing for you.
 * When you don't need more data simply call facq_source_stop() and 
 * facq_source_soft_free() to destroy the object.
 *
 * #FacqSourceSoft implements all the needed operations by the #FacqSource class
 * take a look there if you need more details.
 *
 * facq_source_soft_to_file(), facq_source_soft_key_constructor() and
 * facq_source_soft_constructor() are used by
 * the system to store the config and to recreate #FacqSourceSoft objects.
 * See facq_source_to_file(), the #CIConstructor type and the #CIKeyConstructor
 * for more info.
 */

/**
 * FacqSourceSoft:
 *
 * Contains all the private details of the #FacqSourceSoft.
 */

/**
 * FacqSourceSoftClass:
 *
 * Class for the #FacqSourceSoft objects.
 */

/**
 * FacqSourceSoftError:
 * @FACQ_SOURCE_SOFT_ERROR_FAILED: Some error happened in the source.
 *
 * Enum that contains all the possible errors for the #FacqSourceSoft.
 */

/**
 * FacqFuncType:
 * @FACQ_FUNC_TYPE_RAN: Random
 * @FACQ_FUNC_TYPE_SIN: Sine
 * @FACQ_FUNC_TYPE_COS: Cosine
 * @FACQ_FUNC_TYPE_FLA: Flat wave.
 * @FACQ_FUNC_TYPE_SAW: Sawtooth.
 * @FACQ_FUNC_TYPE_SQU: Square wave.
 *
 * Enum identifying all the possible types of waves that #FacqSourceSoft can
 * generate.
 */

G_DEFINE_TYPE(FacqSourceSoft,facq_source_soft,FACQ_TYPE_SOURCE);

enum {
	PROP_0,
	PROP_AMPLITUDE,
	PROP_FUNC,
	PROP_FUNC_PERIOD
};

struct _FacqSourceSoftPrivate {
	/* private */
	GRand *rand;
	gdouble amplitude;
	guint func;
	guint64 iter;
	gdouble func_period;
	gsize multiplier;
};

GQuark facq_source_soft_error_quark(void)
{
	return g_quark_from_static_string("facq-source-soft-error-quark");
}

/*****--- GObject magic ---*****/
static void facq_source_soft_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqSourceSoft *softsrc = FACQ_SOURCE_SOFT(self);

	switch(property_id){
	case PROP_AMPLITUDE: g_value_set_double(value,softsrc->priv->amplitude);
	break;
	case PROP_FUNC: g_value_set_uint(value,softsrc->priv->func);
	break;
	case PROP_FUNC_PERIOD: g_value_set_double(value,softsrc->priv->func_period);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(softsrc,property_id,pspec);
	}
}

static void facq_source_soft_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqSourceSoft *softsrc = FACQ_SOURCE_SOFT(self);
		
	switch(property_id){
	case PROP_AMPLITUDE: softsrc->priv->amplitude = g_value_get_double(value);
	break;
	case PROP_FUNC: softsrc->priv->func = g_value_get_uint(value);
	break;
	case PROP_FUNC_PERIOD: softsrc->priv->func_period = g_value_get_double(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(softsrc,property_id,pspec);
	}
}

static void facq_source_soft_constructed(GObject *self)
{
	FacqSourceSoft *softsrc = FACQ_SOURCE_SOFT(self);
	const FacqStreamData *stmd = NULL;

	if(softsrc->priv->func == FACQ_FUNC_TYPE_RAN)
		softsrc->priv->rand = g_rand_new();

	stmd = facq_source_get_stream_data(FACQ_SOURCE(softsrc));
	if(stmd->period < 1){
		/* we set the multiplier here, the purpose of multiplier it's to
		 * count the number of slices that needs to be generated by the
		 * read function, this number will multiply the period in the
		 * poll function, simulating a real read */
		softsrc->priv->multiplier = 
			facq_misc_period_to_chunk_size(stmd->period,
						       sizeof(gdouble),
						       stmd->n_channels);
		softsrc->priv->multiplier /= (8*stmd->n_channels);
	}
}

static void facq_source_soft_finalize(GObject *self)
{
	FacqSourceSoft *softsrc = FACQ_SOURCE_SOFT(self);

	if(softsrc->priv->rand)
		g_rand_free(softsrc->priv->rand);
	
	if (G_OBJECT_CLASS (facq_source_soft_parent_class)->finalize)
    		(*G_OBJECT_CLASS (facq_source_soft_parent_class)->finalize) (self);
}

static void facq_source_soft_class_init(FacqSourceSoftClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	FacqSourceClass *source_class = FACQ_SOURCE_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqSourceSoftPrivate));

	object_class->set_property = facq_source_soft_set_property;
	object_class->get_property = facq_source_soft_get_property;
	object_class->finalize = facq_source_soft_finalize;
	object_class->constructed = facq_source_soft_constructed;

	/* override source class virtual methods */
	source_class->srcsave = facq_source_soft_to_file;
	source_class->srcstart = NULL;
	source_class->srcpoll = facq_source_soft_poll;
	source_class->srcread = facq_source_soft_read;
	source_class->srcconv = NULL;
	source_class->srcstop = NULL;
	source_class->srcfree = facq_source_soft_free;

	g_object_class_install_property(object_class,PROP_AMPLITUDE,
					g_param_spec_double("amplitude",
							    "Signal amplitude",
							    "The signal amplitude",
							    1,
							    G_MAXDOUBLE,
							    1,
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_FUNC,
					g_param_spec_uint("func",
							  "Signal function",
							  "The function used to generate the signal",
							  0,
							  FACQ_FUNC_TYPE_N-1,
							  0,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_FUNC_PERIOD,
					g_param_spec_double("func-period",
							  "The period of the signal",
							  "The signal period, it's the time it takes to draw an entire waveform",
							  0.001,
							  (G_MAXULONG/1e6),
							  1,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));
}

static void facq_source_soft_init(FacqSourceSoft *softsrc)
{
	softsrc->priv = G_TYPE_INSTANCE_GET_PRIVATE(softsrc,FACQ_TYPE_SOURCE_SOFT,FacqSourceSoftPrivate);
	softsrc->priv->amplitude = 1;
	softsrc->priv->rand = NULL;
	softsrc->priv->func = 0;
	softsrc->priv->func_period = 0;
	softsrc->priv->multiplier = 1;
}

/*****--- Public methods ---*****/
/**
 * facq_source_soft_to_file:
 * @src: A #FacqSourceSoft object casted to #FacqSource.
 * @file: A #GKeyFile.
 * @group: The group name in the @file, #GKeyFile.
 *
 * Implements the facq_source_to_file() method.
 * Stores the function type, the amplitude, the period, the wave period, and the
 * number of channels in the requested group name, inside a #GKeyFile.
 * This is used by facq_stream_save() function, and you shouldn't need to call
 * this.
 */
void facq_source_soft_to_file(FacqSource *src,GKeyFile *file,const gchar *group)
{
	const FacqStreamData *stmd = facq_source_get_stream_data(src);

	FacqSourceSoft *srcsoft = FACQ_SOURCE_SOFT(src);
	
	g_key_file_set_integer(file,group,"function",srcsoft->priv->func);
	g_key_file_set_double(file,group,"amplitude",srcsoft->priv->amplitude);
	g_key_file_set_double(file,group,"period",stmd->period);
	g_key_file_set_double(file,group,"wave-period",srcsoft->priv->func_period);
	g_key_file_set_double(file,group,"n-channels",stmd->n_channels);
}

/**
 * facq_source_soft_key_constructor:
 * @group_name: A string with the group name.
 * @key_file: A #GKeyFile object.
 * @err: (allow-none): A #GError it will be set in case of error if not %NULL.
 *
 * It's purpose it's to create a new #FacqSourceSoft object from a #GKeyFile and
 * a group name. This function is used by #FacqCatalog. See #CIKeyConstructor
 * for more details.
 *
 * Returns: %NULL in case of error, or a new #FacqSourceSoft object if
 * successful.
 */
gpointer facq_source_soft_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err)
{
	FacqFuncType fun = 0;
	gdouble amplitude = 0, period = 0, wave_period = 0;
	guint32 n_channels = 0;
	GError *local_err = NULL;

	fun = g_key_file_get_integer(key_file,group_name,"function",&local_err);
	if(local_err)
		goto error;

	amplitude = g_key_file_get_double(key_file,group_name,"amplitude",&local_err);
	if(local_err)
		goto error;

	period = g_key_file_get_double(key_file,group_name,"period",&local_err);
	if(local_err)
		goto error;

	wave_period = g_key_file_get_double(key_file,group_name,"wave-period",&local_err);
	if(local_err)
		goto error;

	n_channels = (guint32) g_key_file_get_double(key_file,group_name,"n-channels",&local_err);
	if(local_err)
		goto error;

	return facq_source_soft_new(fun,amplitude,wave_period,period,n_channels,err);

	error:
	if(local_err){
		if(err)
			g_propagate_error(err,local_err);
	}
	return NULL;
}

/**
 * facq_source_soft_constructor:
 * @user_input: A #GPtrArray with the parameters from the user.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqSourceSoft object from a #GPtrArray, @user_input, with
 * at least 4 pointers, the first a pointer to the kind of function, the second
 * a pointer to the desired amplitude, the third a pointer to the wave period,
 * the forth a pointer to the period value, and the fifth a pointer to the
 * desired number of channels. See facq_source_soft_new() for valid values.
 *
 * This function is used by #FacqCatalog, for creating a #FacqSourceSoft with
 * the parameters provided by the user in a #FacqDynDialog, take a look at this
 * other objects for more details, and to the #CIConstructor type.
 *
 * Returns: A new #FacqSourceSoft object, or %NULL in case of error.
 */
gpointer facq_source_soft_constructor(const GPtrArray *user_input,GError **err)
{
	guint *fun = NULL, *n_channels = NULL;
	gdouble *amplitude = NULL, *period = NULL, *wave_period = NULL;

	fun = g_ptr_array_index(user_input,0);
	amplitude = g_ptr_array_index(user_input,1);
	wave_period = g_ptr_array_index(user_input,2);
	period = g_ptr_array_index(user_input,3);
	n_channels = g_ptr_array_index(user_input,4);

	return facq_source_soft_new(*fun,*amplitude,
			            *wave_period,
				    *period,
				    *n_channels,
				    err);
}

/**
 * facq_source_soft_new:
 * @fun: The kind of function to simulate. See #FacqFuncType for valid values.
 * @amplitude: A real value that will set the maximum amplitude for the
 * generated functions. It should be 1 or higher.
 * @wave_period: The wave period in seconds. (In case of a random wave it will
 * be ignored).
 * @period: The sampling period in seconds.
 * @n_channels: The number of channels. If it's greater than one it will
 * generate a wave in each channel. Starting on channel 0, up to channel n-1.
 * @error: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqSourceSoft object, that will create data according to the
 * input parameters.
 *
 * Returns: A new #FacqSourceSoft object, or %NULL in case of error.
 */
FacqSourceSoft *facq_source_soft_new(FacqFuncType fun,gdouble amplitude,gdouble wave_period,gdouble period,guint n_channels,GError **error)
{
	FacqStreamData *stmd = NULL;
	FacqChanlist *chanlist = NULL;
	FacqUnits *units = NULL;
	guint i = 0;
	gdouble *max = NULL;
	gdouble *min = NULL;

	if(n_channels == 0 || amplitude < 1 || 
		period < 0.001 || period > (G_MAXULONG/1e6) ||
			wave_period < 0.001 || wave_period > (G_MAXULONG/1e6)){
		
		if(error != NULL)
			g_set_error_literal(error,FACQ_SOURCE_SOFT_ERROR,
					FACQ_SOURCE_SOFT_ERROR_FAILED,
						"Invalid n_channels, sampling period, wave period,"
						" or amplitude value");
		return NULL;
	}

	//create chanlist and units, of n_channels length
	chanlist = facq_chanlist_new();
	units = g_new0(enum chan_unit,n_channels);
	max = g_new0(gdouble,n_channels);
	min = g_new0(gdouble,n_channels);

	for(i = 0;i < n_channels;i++){
		facq_chanlist_add_chan(chanlist,i,0,0,0,CHAN_INPUT);
		units[i] = UNIT_U;
		max[i] = amplitude;
		min[i] = -amplitude;
	}

	stmd = facq_stream_data_new(8,n_channels,period,chanlist,units,max,min);
	return FACQ_SOURCE_SOFT(g_object_new(FACQ_TYPE_SOURCE_SOFT,
					       "name",facq_resources_names_source_soft(), 
					       "description",facq_resources_descs_source_soft(),
					       "func",fun,
					       "func-period",wave_period,
					       "amplitude",amplitude,
					       "stream-data",stmd,
					       NULL));
}

/**
 * facq_source_soft_poll:
 * @src: A #FacqSourceSoft casted to @FacqSource.
 *
 * Implements facq_source_poll() from #FacqSource.
 * Polls the source to check if new data it's ready to be read from the source.
 * Because this kind of source doesn't use real hardware, it's simple an elegant
 * way of calling g_usleep().
 *
 * Returns: This type of source always returns 1.
 */
gint facq_source_soft_poll(FacqSource *src)
{
	const FacqStreamData *stmd = NULL;
	FacqSourceSoft *srcsoft = NULL;
	gdouble period = 0;

	g_return_val_if_fail(FACQ_IS_SOURCE_SOFT(src),-1);

	stmd = facq_source_get_stream_data(src);
	period = stmd->period;
	srcsoft = FACQ_SOURCE_SOFT(src);

	g_usleep(period*G_USEC_PER_SEC*srcsoft->priv->multiplier);

	return 1;
}
/* A #FacqSource always 
 * returns 1 if it's ready to read, 0 in case of timeout 
 * and -1 in case of error */

/* This is used by the square wave */
static gdouble facq_func_sign(gdouble input)
{
	if(input == 0)
		return 0;
	if(input < 0)
		return -1;
	if(input > 0)
		return 1;
}

/* see http://mathworld.wolfram.com/FractionalPart.html 
 * This is used by the sawtooth wave */
static inline gdouble facq_func_frac(gdouble input)
{
	return input-floor(input);
}

static gdouble facq_func_get_sample(FacqFuncType fun,GRand *rand,gdouble period,gdouble sampling_period,gdouble amplitude,guint64 iter,guint channel)
{
	gdouble sample = 0, phase = 0;

	switch(fun){
	case FACQ_FUNC_TYPE_RAN:
		sample = g_rand_double_range(rand,-amplitude,amplitude);
	break;
	case FACQ_FUNC_TYPE_SIN:
		phase = channel;
		sample = amplitude * 
				sin(  ( (2*G_PI/period) * (sampling_period * iter) + phase )  );
	break;
	case FACQ_FUNC_TYPE_COS:
		phase = channel;
		sample = amplitude * 
				cos(  ( (2*G_PI/period) * (sampling_period * iter) + phase )  );
	break;
	case FACQ_FUNC_TYPE_FLA:
		sample = amplitude;
	break;
	case FACQ_FUNC_TYPE_SAW:
		phase = channel;
		sample = amplitude * 
				facq_func_frac(  ( (iter*sampling_period)/period + phase )  );
	break;
	case FACQ_FUNC_TYPE_SQU:
		sample = amplitude * 
				facq_func_sign(sin(  ( (2*G_PI/period) 
							* (sampling_period *iter) + phase )  ));
	break;
	default:
		sample = 0;
	}

	return sample;
}

/**
 * facq_source_soft_read:
 * @src: A #FacqSourceSoft casted to #FacqSource.
 * @buf: A pointer to a free memory area.
 * @count: The number of available bytes in the memory area.
 * @bytes_read: It will store the number of bytes read.
 * @err: (allow-none): A #GError, It will be set in case of error if not %NULL.
 *
 * Reads data from the source, (A maximum of @count bytes) putting 
 * it in the memory area pointed by @buf. When the function returns the control
 * the number of bytes read will be written to @bytes_read.
 *
 * Returns: This kind of source always returns %G_IO_STATUS_NORMAL:
 */
GIOStatus facq_source_soft_read(FacqSource *src,gchar *buf,gsize count,gsize *bytes_read,GError **err)
{
	FacqSourceSoft *srcsoft = NULL;
	const FacqStreamData *stmd = NULL;
	gdouble *sample = NULL;
	gsize slices = 0;
	guint i = 0,j = 0;

#if ENABLE_DEBUG
	g_return_val_if_fail(FACQ_IS_SOURCE_SOFT(src),G_IO_STATUS_ERROR);
#endif
	srcsoft = FACQ_SOURCE_SOFT(src);
	stmd = facq_source_get_stream_data(src);

	sample = (gdouble *)buf;
	slices = count/(sizeof(gdouble)*stmd->n_channels);

	for(i = 0;i < slices;i++){
		sample = (gdouble *)(buf + (i * sizeof(gdouble) * stmd->n_channels) );
		for(j = 0;j < stmd->n_channels;j++){
			sample[j] = facq_func_get_sample(srcsoft->priv->func,
							 srcsoft->priv->rand,
							 srcsoft->priv->func_period,
							 facq_stream_data_get_period(stmd),
							 srcsoft->priv->amplitude,
							 srcsoft->priv->iter,
							 j);
		}
		srcsoft->priv->iter++;
	}

	*bytes_read = count;

	return G_IO_STATUS_NORMAL;
}

/**
 * facq_source_soft_free:
 * @src: A #FacqSourceSoft casted to #FacqSource.
 *
 * Implements facq_source_free() from #FacqSource.
 * Destroys a no longer needed #FacqSourceSoft.
 */
void facq_source_soft_free(FacqSource *src)
{
	g_return_if_fail(FACQ_IS_SOURCE_SOFT(src));
	g_object_unref(G_OBJECT(src));
}

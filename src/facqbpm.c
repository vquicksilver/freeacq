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
#include "facqlog.h"
#include "facqglibcompat.h"
#include "facqchunk.h"
#include "facqbpm.h"

/**
 * SECTION:facqbpm
 * @title:FacqBPM
 * @include:facqbpm.h
 * @short_description: A multichannel BPM (Beats per minute) counter.
 *
 * This module provides a multichannel BPM (Beats per minute) counter.
 * 
 * For using it you have to create a #FacqBPM object first with the
 * facq_bpm_new() function. Then you must call facq_bpm_setup() to 
 * let the object do some internal initialization. After this to obtain the
 * number of beats per minute you must call facq_bpm_compute(). 
 * When the #FacqBPM is no longer needed destroy it with facq_bpm_free().
 *
 * <sect1 id="bpm-algo">
 *  <title>BPM algorithm</title>
 *  <para>
 *  Before the BPM can be used, you must setup it. The algorithm searches for
 *  maximum values sample by sample for each channel. When a maximum is found
 *  a counter starts counting the number of samples until a new maximum arrives,
 *  at this point, we calculate the time between this two consecutive maximums,
 *  and if for one pulse it takes that time, x pulses will happen in 60 seconds.
 *  </para>
 * </sect1>
 */

/**
 * FacqBPMClass:
 *
 * Class for the FacqBPM objects.
 */

/**
 * FacqBPM:
 *
 * Contains all the private values required by the BPM operation.
 */

G_DEFINE_TYPE(FacqBPM,facq_bpm,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_PERIOD,
	PROP_N_CHANNELS
};

typedef enum {
	BPM_0,
	BPM_G1,
	BPM_D1,
	BPM_G2,
	BPM_D2
} BPMState;

struct _FacqBPMPrivate {
	/*< private >*/
	gdouble period;
	guint n_channels;
	gdouble *bpm;
	gdouble *prev;
	guint64 *counter;
	BPMState *state;
};

static void facq_bpm_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqBPM *bpm = FACQ_BPM(self);

	switch(property_id){
	case PROP_PERIOD: g_value_set_double(value,bpm->priv->period);
	break;
	case PROP_N_CHANNELS: g_value_set_uint(value,bpm->priv->n_channels);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(bpm,property_id,pspec);
	}
}

static void facq_bpm_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqBPM *bpm = FACQ_BPM(self);

	switch(property_id){
	case PROP_PERIOD: bpm->priv->period = g_value_get_double(value);
	break;
	case PROP_N_CHANNELS: bpm->priv->n_channels = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(bpm,property_id,pspec);
	}
}

static void facq_bpm_finalize(GObject *self)
{
	FacqBPM *bpm = FACQ_BPM(self);

	if(bpm->priv->state)
		g_free(bpm->priv->state);
	if(bpm->priv->bpm)
		g_free(bpm->priv->bpm);
	if(bpm->priv->prev)
		g_free(bpm->priv->prev);
	if(bpm->priv->counter)
		g_free(bpm->priv->counter);
}

static void facq_bpm_class_init(FacqBPMClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqBPMPrivate));

	object_class->set_property = facq_bpm_set_property;
	object_class->get_property = facq_bpm_get_property;
	object_class->finalize = facq_bpm_finalize;

	g_object_class_install_property(object_class,PROP_N_CHANNELS,
					g_param_spec_uint("n-channels",
							  "Number of channels",
							  "The number of channels",
							  1,
							  G_MAXUINT,
							  1,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT |
							  G_PARAM_STATIC_STRINGS ));

	g_object_class_install_property(object_class,PROP_PERIOD,
					g_param_spec_double("period",
							  "The period",
							  "The sampling period of the input samples in seconds",
							  1e-9,
							  G_MAXDOUBLE,
							  1,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT |
							  G_PARAM_STATIC_STRINGS ));
}

static void facq_bpm_init(FacqBPM *bpm)
{
	bpm->priv = G_TYPE_INSTANCE_GET_PRIVATE(bpm,FACQ_TYPE_BPM,FacqBPMPrivate);
	bpm->priv->period = 0;
	bpm->priv->n_channels = 1;
	bpm->priv->bpm = NULL;
	bpm->priv->prev = NULL;
	bpm->priv->counter = NULL;
	bpm->priv->state = NULL;
}

/**
 * facq_bpm_new:
 *
 * Creates a new #FacqBPM object. You must setup it with facq_bpm_setup().
 *
 * Returns: A new #FacqBPM object.
 */
FacqBPM *facq_bpm_new(void)
{
	return g_object_new(FACQ_TYPE_BPM,NULL);
}

/**
 * facq_bpm_setup:
 * @bpm: A #FacqBPM object.
 * @n_channels: The number of channels in the input data.
 * @period: The period between samples, in seconds.
 *
 * Setups the #FacqBPM object, after calling this function
 * the object can start processing #FacqChunk objects with
 * facq_bpm_compute().
 */ 
void facq_bpm_setup(FacqBPM *bpm,guint n_channels,gdouble period)
{
	GValue params[2] = { G_VALUE_INIT, G_VALUE_INIT };
	guint i = 0;

	g_value_init(&params[0],G_TYPE_UINT);
	g_value_set_uint(&params[0],n_channels);
	g_object_set_property(G_OBJECT(bpm),"n-channels",&params[0]);
	g_value_init(&params[1],G_TYPE_DOUBLE);
	g_value_set_double(&params[1],period);
	g_object_set_property(G_OBJECT(bpm),"period",&params[1]);

	bpm->priv->state =
		g_realloc(bpm->priv->state,
			  sizeof(BPMState)*bpm->priv->n_channels);
	bpm->priv->bpm = 
		g_realloc(bpm->priv->bpm,
			  sizeof(gdouble)*bpm->priv->n_channels);
	bpm->priv->prev =
		g_realloc(bpm->priv->prev,
			  sizeof(gdouble)*bpm->priv->n_channels);
	bpm->priv->counter =
		g_realloc(bpm->priv->counter,
			  sizeof(guint64)*bpm->priv->n_channels);

	for(i = 0;i < bpm->priv->n_channels;i++){
		bpm->priv->state[i] = BPM_0;
		bpm->priv->bpm[i] = 0;
		bpm->priv->prev[i] = 0;
		bpm->priv->counter[i] = 0;
	}

	g_value_unset(&params[0]);
	g_value_unset(&params[1]);
}

/**
 * facq_bpm_compute:
 * @bpm: A #FacqBPM object.
 * @chunk: A #FacqChunk object, containing the samples of one or
 * more signals, and one or more slices per signal.
 *
 * Computes the beats per minute value per channel, from a #FacqChunk.
 * You must call this function for each #FacqChunk.
 *
 * Returns: A real array, with the same number of values, as the number
 * of channels you passed in the setup function. Each value corresponds to
 * the number of beats per minute per channel.
 */
const gdouble *facq_bpm_compute(FacqBPM *bpm,FacqChunk *chunk)
{
	gdouble *slice = NULL;
	gsize total_slices = 0, i = 0, j = 0;

	total_slices = 
		facq_chunk_get_total_slices(chunk,
					    sizeof(gdouble),
					    bpm->priv->n_channels);
	slice = (gdouble *)chunk->data;

	for(i = 0;i < total_slices;i++){
		for(j = 0;j < bpm->priv->n_channels;j++){
			switch(bpm->priv->state[j]){
			case BPM_0:
#if ENABLE_DEBUG
				facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,"channel %u, BPM_0",j);
#endif
				if(slice[j] < bpm->priv->prev[j])
					bpm->priv->state[j] = BPM_D2;
				else if(slice[j] > bpm->priv->prev[j])
					bpm->priv->state[j] = BPM_G1;
				else
					bpm->priv->state[j] = BPM_0;
				continue;
			break;
			case BPM_G1:
#if ENABLE_DEBUG
				facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,"channel %u, BPM_G1",j);
#endif
				if(slice[j] < bpm->priv->prev[j]){
					bpm->priv->state[j] = BPM_D1;
					bpm->priv->counter[j] = 0;
					continue;
				}
			break;
			case BPM_D1:
#if ENABLE_DEBUG
				facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,"channel %u, BPM_D1",j);
#endif
				bpm->priv->counter[j] += 1;
				if(slice[j] > bpm->priv->prev[j]){
					bpm->priv->state[j] = BPM_G2;
					continue;
				}
			break;
			case BPM_G2:
#if ENABLE_DEBUG
				facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,"channel %u, BPM_G2",j);
#endif
				bpm->priv->counter[j] += 1;
				if(slice[j] < bpm->priv->prev[j]){
					bpm->priv->state[j] = BPM_D1;
					bpm->priv->bpm[j] = 
						60.0 / (bpm->priv->counter[j] * bpm->priv->period);
					bpm->priv->counter[j] = 0;
					continue;
				}
			break;
			case BPM_D2:
#if ENABLE_DEBUG
				facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,"channel %u, BPM_G1",j);
#endif
				if(slice[j] > bpm->priv->prev[j]){
					bpm->priv->state[j] = BPM_G1;
					continue;
				}
			break;
			}
			bpm->priv->prev[j] = slice[j];
		}
		slice += bpm->priv->n_channels;
	}

	return bpm->priv->bpm;
}

/**
 * facq_bpm_free:
 * @bpm: A #FacqBPM object.
 *
 * Destroys the #FacqBPM, @bpm, object.
 */
void facq_bpm_free(FacqBPM *bpm)
{
	g_return_if_fail(FACQ_IS_BPM(bpm));
	g_object_unref(G_OBJECT(bpm));
}

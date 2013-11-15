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
#include "facqchanlist.h"

/**
 * SECTION:facqchanlist
 * @include:facqchanlist.h
 * @short_description: a list of channels
 * @see_also: #FacqUnits
 *
 * A #FacqChanlist provides a way to store, retrieve and update a channel
 * list. A channel it's an analog or digital I/O line that can be readed 
 * or written, and it's identified by an unsigned integer, starting at 0. 
 * The same channel can be repeated on the same list.
 * A channel can also be used for special purposes, depending on the kind
 * of data adquisition. For example when using an async comedi subdevice
 * a channel can be used to trigger events (scan_begin, convert or scan_end).
 * A channel also can be used as a base channel for a DIO adquisition.
 *
 * These special purposes are covered by the #FacqChanDir data type along
 * with the standard input or output purposes.
 *
 * A channel can have other properties, these properties are range, 
 * analog reference, and flags. 
 *
 * The range property tells the hardware
 * what values should be associated with the maximum and minimum value
 * that the channel can read, for example for the same channel a subdevice
 * can support diferent ranges like [-5,5] Volts and [-2.5,2.5] Volts, if you
 * know that the signal you are going to measure it's contained in the second
 * range you will get better precission when measuring the signal if you use
 * the second range. 
 *
 * The analog reference tells the subdevice which analog
 * reference should be used when measuring, it can be ground (Referenced 
 * from earth ground), common (Same reference for all the channel without
 * using earth ground), differential (Use 2 channels for doing the measure)
 * the value of the samples will be the substract between the 2 values,
 * or other (For other references that weren't already listed).
 * 
 * The flags are other options that can be enabled for a channel.
 * For the moment the flags allow to enable 
 * <ulink url="https://en.wikipedia.org/wiki/Dither">dithering</ulink> 
 * or 
 * <ulink url="https://en.wikipedia.org/wiki/Glitch#Electronics_glitch">deglitching</ulink>.
 *
 * If you put together the chan number, range, reference and flags you
 * obtain what is called a "chanspec", the chanlist stores all the info
 * in form of chanspecs. A chanspec it's an unsigned integer which has 
 * encoded all the info about a channel.
 *
 * For more details on data adquisition hardware and channels you can
 * go to the <ulink url="http://www.comedi.org">Comedi</ulink> site.
 *
 * You can create an empty #FacqChanlist with facq_chanlist_new(), add 
 * a channel to the list with facq_chanlist_add_chan(), remove the last
 * added channel with facq_chanlist_del_chan(), get the number of stored
 * channels (Including special channels) with facq_chanlist_get_length() 
 * or get the number of stored I/O channels (Without special channels) with 
 * facq_chanlist_get_io_chans_n().
 *
 * To obtain a chanspec of a previously stored channel you can use 
 * facq_chanlist_get_chanspec() and facq_chanlist_get_io_chanspec().
 * To translate a chanspec to the original values you can use 
 * facq_chanlist_chanspec_to_src_values().
 *
 * Searching for special channels is possible using the 
 * facq_chanlist_get_special_chan_index() function.
 *
 * If you know the index (The position in the list) of a channel you 
 * can query it's direction using facq_chanlist_get_chan_direction() or
 * facq_chanlist_get_io_chan_direction().
 *
 * To check if a channel had already been added to the list use 
 * facq_chanlist_search_io_chan().
 *
 * To get an array with the channel numbers use 
 * facq_chanlist_to_comedi_chanlist().
 *
 * To get an string that can be used with NIDAQmx software when
 * creating virtual channels for a task use facq_chanlist_to_nidaq_chanlist().
 *
 * facq_chanlist_get_comedi_conversion_direction_list() gives a 
 * comedi compatible direction list.
 *
 * Finally to destroy a #FacqChanlist use facq_chanlist_free().
 */

/**
 * FacqChanlist:
 */

/**
 * FacqChanlistClass:
 */

G_DEFINE_TYPE(FacqChanlist,facq_chanlist,G_TYPE_OBJECT);

#define FACQ_CHANLIST_TEST_AREF(x) g_assert(x == AREF_GROUND || x == AREF_COMMON || x == AREF_DIFF || x == AREF_OTHER)
#define FACQ_CHANLIST_TEST_CHAN_DIR(x) g_assert(x == CHAN_INPUT || x == CHAN_OUTPUT || x == CHAN_BASE || \
x == CHAN_START_EXT || x == CHAN_BEGIN_EXT || x == CHAN_CONVERT_EXT)

enum {
	PROP_0,
	PROP_LENGTH
};

struct _FacqChanlistPrivate {
	GArray *uintlist;
	GArray *dirlist;
	gboolean base_chan;//for SYNC DIO/DI/DO
	gboolean start_ext;//for ASYNC AI/AO/DI/DO/DIO
	gboolean begin_ext;//for ASYNC AI/AO/DI/DO/DIO
	gboolean convert_ext;//for ASYNC AI/AO/DI/DO/DIO
	guint length;
};

/*****--- GObject magic ---*****/
static void facq_chanlist_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqChanlist *chanlist = FACQ_CHANLIST(self);

	switch(property_id){
	case PROP_LENGTH: g_value_set_uint(value,chanlist->priv->length);
	break;
	default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (chanlist, property_id, pspec);
	}
}

static void facq_chanlist_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqChanlist *chanlist = FACQ_CHANLIST(self);

	switch(property_id){
	case PROP_LENGTH: chanlist->priv->length = g_value_get_uint(value);
	break;
	default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (chanlist, property_id, pspec);
	}
}

static void facq_chanlist_finalize(GObject *self)
{
	FacqChanlist *chanlist = FACQ_CHANLIST(self);
	guint *aux = NULL;
	FacqChanDir *dir = NULL;

	if(chanlist->priv->uintlist)
		aux = (guint *)g_array_free(chanlist->priv->uintlist,FALSE);
		if(aux)
			g_free(aux);
	if(chanlist->priv->dirlist)
		dir = (FacqChanDir *)g_array_free(chanlist->priv->dirlist,FALSE);
		if(dir)
			g_free(dir);

	if (G_OBJECT_CLASS (facq_chanlist_parent_class)->finalize)
    		(*G_OBJECT_CLASS (facq_chanlist_parent_class)->finalize) (self);
}

static void facq_chanlist_constructed(GObject *self)
{
	FacqChanlist *chanlist = FACQ_CHANLIST(self);

	chanlist->priv->uintlist = g_array_new(FALSE,FALSE,sizeof(guint));
	chanlist->priv->dirlist = g_array_new(FALSE,FALSE,sizeof(FacqChanDir));
	chanlist->priv->length = 0;
	chanlist->priv->base_chan = FALSE;
	chanlist->priv->start_ext = FALSE;
	chanlist->priv->begin_ext = FALSE;
	chanlist->priv->convert_ext = FALSE;
}

static void facq_chanlist_class_init(FacqChanlistClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass, sizeof(FacqChanlistPrivate));

	object_class->set_property = facq_chanlist_set_property;
	object_class->get_property = facq_chanlist_get_property;
	object_class->finalize = facq_chanlist_finalize;
	object_class->constructed = facq_chanlist_constructed;
	
	/**
	 * facqchanlist:length:
	 */
	g_object_class_install_property(object_class,PROP_LENGTH,
					g_param_spec_uint("length",
							  "Chanlist length",
							  "The number of channels in the Chanlist",
							  0,
							  G_MAXUINT,
							  0,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));
}

static void facq_chanlist_init(FacqChanlist *chanlist)
{
	chanlist->priv = G_TYPE_INSTANCE_GET_PRIVATE(chanlist,FACQ_TYPE_CHANLIST,FacqChanlistPrivate);

	chanlist->priv->uintlist = NULL;
	chanlist->priv->dirlist = NULL;
	chanlist->priv->length = 0;
	chanlist->priv->base_chan = FALSE;
	chanlist->priv->start_ext = FALSE;
	chanlist->priv->begin_ext = FALSE;
	chanlist->priv->convert_ext = FALSE;
}

/*****--- private methods ---*****/
/* facq_chanlist_get_chan_base_index:
 * 
 * Given a chanlist and a optional index allocated by the caller
 * a channel with dir == CHAN_BASE is "searched", and index is set
 * to the position where it's found. If it's found TRUE is returned. 
 *
 * Note that it only can be on the first position on the array
 */
static gboolean facq_chanlist_get_chan_base_index(const FacqChanlist *chanlist,guint *index)
{
	FacqChanDir dir;

	if(chanlist->priv->base_chan){
		dir = facq_chanlist_get_chan_direction(chanlist,0);
		if(dir == CHAN_BASE){
			*index = 0;
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * facq_chanlist_get_chan_ext_index:
 *
 * Search for a channel with dir equal to CHAN_START_EXT, CHAN_BEGIN_EXT,
 * or CHAN_CONVERT_EXT. If found, return TRUE and update index with the
 * position of the channel in the list (this last step is optional).
 *
 * Note that is possible to have 3 special channels as long as a channel
 * with DIR = CHAN_BASE is not present. Special channel will always be found
 * at the start of the list so it will be at 0, 1 or 2 position.
 */
static gboolean facq_chanlist_get_chan_ext_index(const FacqChanlist *chanlist,guint *index,FacqChanDir dir)
{
	guint i = 0;
	guint len = facq_chanlist_get_length(chanlist);
	gboolean ret = FALSE;

	if(chanlist->priv->base_chan)
		return FALSE;
	switch(dir){
	case CHAN_START_EXT:
		if(chanlist->priv->start_ext == FALSE)
			return FALSE;
	break;
	case CHAN_BEGIN_EXT:
		if(chanlist->priv->begin_ext == FALSE)
			return FALSE;
	break;
	case CHAN_CONVERT_EXT:
		if(chanlist->priv->convert_ext == FALSE)
			return FALSE;
	break;
	default: return FALSE;
	}
	
	for(i = 0;i < len;i++){
		if(i == 3) break;
		if(facq_chanlist_get_chan_direction(chanlist,i) == dir){
			ret = TRUE;
			if(index)
				*index = i;
			break;
		}
	}
	return ret;
}

/* public methods */

/**
 * facq_chanlist_new:
 *
 * Creates a new #FacqChanlist object.
 *
 * Returns: a new #FacqChanlist object.
 */
FacqChanlist *facq_chanlist_new(void)
{
	return g_object_new(FACQ_TYPE_CHANLIST,NULL);
}

/**
 * facq_chanlist_add_chan:
 * @chanlist: a #FacqChanlist object.
 * @chan: The channel number.
 * @rng: The range number.
 * @aref: %AREF_GROUND, %AREF_COMMON, %AREF_DIFF or %AREF_OTHER.
 * @flags: %CR_DITHER or %CR_DEGLITCH, %CR_ALT_SOURCE, %CR_EDGE, %CR_INVERT.
 * @dir: %CHAN_INPUT, %CHAN_OUTPUT, %CHAN_BASE, %CHAN_START_EXT,
 * %CHAN_BEGIN_EXT, or %CHAN_CONVERT_EXT.
 * 
 * Adds a new channel to a #FacqChanlist with the given range,flags 
 * and dir parameters.
 * <note>
 * You can add only a special channel if direction is equal to CHAN_BASE,
 * and 3 special channels when using CHAN_*_EXT directions.
 * </note>
 */
void facq_chanlist_add_chan(FacqChanlist *chanlist,guint chan,guint rng,guint aref,guint flags,FacqChanDir dir)
{
	guint chanspec = 0;

	g_assert(FACQ_IS_CHANLIST(chanlist));
	FACQ_CHANLIST_TEST_AREF(aref);
	FACQ_CHANLIST_TEST_CHAN_DIR(dir);
	g_assert(chanlist->priv->uintlist != NULL && chanlist->priv->dirlist != NULL);
	g_assert(chanlist->priv->length == chanlist->priv->uintlist->len && chanlist->priv->length == chanlist->priv->dirlist->len);
	
	chanspec = CR_PACK_FLAGS(chan,rng,aref,flags);
	
	if(dir == CHAN_BASE && !chanlist->priv->base_chan){
		if(!chanlist->priv->start_ext && !chanlist->priv->begin_ext && !chanlist->priv->convert_ext){
			chanlist->priv->base_chan = TRUE;
			g_array_prepend_vals(chanlist->priv->uintlist,&chanspec,1);
			g_array_prepend_vals(chanlist->priv->dirlist,&dir,1);
			chanlist->priv->length+=1;
		}
		return;
	}
	if(dir == CHAN_START_EXT && !chanlist->priv->start_ext){
		if(chanlist->priv->base_chan == FALSE){
			chanlist->priv->start_ext = TRUE;
			g_array_prepend_vals(chanlist->priv->uintlist,&chanspec,1);
			g_array_prepend_vals(chanlist->priv->dirlist,&dir,1);
			chanlist->priv->length+=1;
		}
		return;
	}
	if(dir == CHAN_BEGIN_EXT && !chanlist->priv->begin_ext){
		if(chanlist->priv->base_chan == FALSE){
			chanlist->priv->begin_ext = TRUE;
			g_array_prepend_vals(chanlist->priv->uintlist,&chanspec,1);
			g_array_prepend_vals(chanlist->priv->dirlist,&dir,1);
			chanlist->priv->length+=1;
		}
		return;
	}
	if(dir == CHAN_CONVERT_EXT && !chanlist->priv->convert_ext){
		if(chanlist->priv->base_chan == FALSE){
			chanlist->priv->convert_ext = TRUE;
			g_array_prepend_vals(chanlist->priv->uintlist,&chanspec,1);
			g_array_prepend_vals(chanlist->priv->dirlist,&dir,1);
			chanlist->priv->length+=1;
		}
		return;
	}
	if(dir == CHAN_INPUT || dir == CHAN_OUTPUT){
		g_array_append_vals(chanlist->priv->uintlist,&chanspec,1);
		g_array_append_vals(chanlist->priv->dirlist,&dir,1);
		chanlist->priv->length+=1;
	}
	return;
}

/**
 * facq_chanlist_del_chan:
 * @chanlist: A #FacqChanlist object.
 * 
 * Removes the latest added channel on the @chanlist, if any.
 *
 * <note>
 * If you are going to add and remove channel, and are going to use
 * special channels, add the special channels first.
 * </note>
 */
void facq_chanlist_del_chan(FacqChanlist *chanlist)
{
	guint len = 0;
	FacqChanDir dir;

	g_return_if_fail(FACQ_IS_CHANLIST(chanlist));
	len = facq_chanlist_get_length(chanlist);
	if(len){
		g_array_remove_index(chanlist->priv->uintlist,len-1);
		dir = facq_chanlist_get_chan_direction(chanlist,len-1);
		switch(dir){
		case CHAN_BASE: chanlist->priv->base_chan = FALSE;
		break;
		case CHAN_START_EXT: chanlist->priv->start_ext = FALSE;
		break;
		case CHAN_BEGIN_EXT: chanlist->priv->begin_ext = FALSE;
		break;
		case CHAN_CONVERT_EXT: chanlist->priv->convert_ext = FALSE;
		break;
		default:
		break;
		}
		g_array_remove_index(chanlist->priv->dirlist,len-1);
		chanlist->priv->length-=1;
	}
	g_assert(chanlist->priv->length == chanlist->priv->uintlist->len && chanlist->priv->length == chanlist->priv->dirlist->len);
	return;
}

/**
 * facq_chanlist_get_length:
 * @chanlist: A #FacqChanlist object.
 *
 * Gives the total number of channels added to the list.
 *
 * Returns: The total number of channels added to the list, including
 * special channels.
 */
guint facq_chanlist_get_length(const FacqChanlist *chanlist)
{
	g_assert(FACQ_IS_CHANLIST(chanlist));
	return chanlist->priv->length;
}

/**
 * facq_chanlist_get_io_chans_n:
 * @chanlist: A #FacqChanlist object.
 *
 * Gives the total number of I/O channels added to the list.
 * (Without taking into account the special channels).
 *
 * Returns: The number of I/O channels in the list.
 */
guint facq_chanlist_get_io_chans_n(const FacqChanlist *chanlist)
{
	guint n_ext = 0;

	g_return_val_if_fail(FACQ_IS_CHANLIST(chanlist),0);

	if( facq_chanlist_get_length(chanlist) == 0 ) 
		return 0;
	else {
		if(chanlist->priv->base_chan){
			return facq_chanlist_get_length(chanlist)-1;
		}

		if(chanlist->priv->start_ext) n_ext++;
		if(chanlist->priv->begin_ext) n_ext++;
		if(chanlist->priv->convert_ext) n_ext++;
	}
	return facq_chanlist_get_length(chanlist)-n_ext;
}

/**
 * facq_chanlist_get_special_chan_index:
 * @chanlist: A #FacqChanlist object.
 * @index:(out caller-allocates)(allow-none): An output parameter to store the
 * position that the channel is using in the list, if any.
 * @dir: %CHAN_BASE, %CHAN_START_EXT, %CHAN_BEGIN_EXT, or %CHAN_CONVERT_EXT.
 *
 * Queries the #FacqChanlist object, @chanlist, for a channel with a direction
 * @dir, if found returns %TRUE and *index is set to the position, else
 * %FALSE is returned.
 *
 * Returns: %TRUE if the channel is found or %FALSE in the other case.
 */
gboolean facq_chanlist_get_special_chan_index(const FacqChanlist *chanlist,guint *index,FacqChanDir dir)
{
	gboolean ret = FALSE;

	if(dir == CHAN_BASE){
		ret = facq_chanlist_get_chan_base_index(chanlist,index);
	}
	if(dir == CHAN_START_EXT || dir == CHAN_BEGIN_EXT || dir == CHAN_CONVERT_EXT){
		ret = facq_chanlist_get_chan_ext_index(chanlist,index,dir);
	}
	return ret;
}

/**
 * facq_chanlist_get_chanspec:
 * @chanlist: A #FacqChanlist object.
 * @index: The position of the channel in the @chanlist.
 *
 * Given a #FacqChanlist, @chanlist, and a position, @index, in the chanlist
 * the function returns the corresponding chanspec.
 *
 * Returns: A chanspec.
 * <para>
 * <note>
 * @index can be any value in [0,facq_chanlist_get_length()].
 *
 * A chanspec it's an unsigned integer which has encoded the channel number,
 * the range, the analog reference and the flags. To obtain this values from
 * a chanspec see facq_chanlist_chanspec_to_src_values().
 * </note>
 * </para>
 */
guint facq_chanlist_get_chanspec(const FacqChanlist *chanlist,guint index)
{
	guint chanspec = 0;
	
	g_assert(FACQ_IS_CHANLIST(chanlist));

	g_assert(index < facq_chanlist_get_length(chanlist));
	chanspec = g_array_index(chanlist->priv->uintlist,guint,index);
	return chanspec;
}

/**
 * facq_chanlist_get_io_chanspec:
 * @chanlist: a #FacqChanlist object.
 * @index: The position of the channel in the @chanlist.
 *
 * Given a #FacqChanlist, @chanlist and a index this function 
 * returns the chanspec of a I/O channel contained in the list.
 *
 * Returns: A chanspec.
 * <para>
 * <note>
 * @index can be any value in [0,facq_chanlist_get_io_chans_n()].
 *
 * A chanspec it's an unsigned integer which has encoded the channel number,
 * the range, the analog reference and the flags. To obtain this values from
 * a chanspec see facq_chanlist_chanspec_to_src_values().
 * </note>
 * </para>
 */
guint facq_chanlist_get_io_chanspec(const FacqChanlist *chanlist,guint index)
{
	guint chanspec = 0 ,iochans_n = 0,first_io_idx = 0;
	
	iochans_n = facq_chanlist_get_io_chans_n(chanlist);
	g_assert(FACQ_IS_CHANLIST(chanlist));
	g_assert(index < iochans_n);

	first_io_idx = facq_chanlist_get_length(chanlist) - iochans_n;

	chanspec = g_array_index(chanlist->priv->uintlist,guint,index+first_io_idx);
	return chanspec;
}

/**
 * facq_chanlist_get_chan_direction:
 * @chanlist: A #FacqChanlist object.
 * @index: The channel position in the list.
 *
 * Returns the direction stored in the list for a particular channel that
 * is at position, @index.
 *
 * Returns: A #FacqChanDir.
 *
 * <para>
 * <note>
 * @index can be any value in [0,facq_chanlist_get_length()].
 * </note>
 * </para>
 */
FacqChanDir facq_chanlist_get_chan_direction(const FacqChanlist *chanlist,guint index)
{
	FacqChanDir dir = 0;

	g_assert(FACQ_IS_CHANLIST(chanlist));

	g_assert(index < facq_chanlist_get_length(chanlist));
	dir = g_array_index(chanlist->priv->dirlist,FacqChanDir,index);
	return dir;
}

/**
 * facq_chanlist_get_io_chan_direction:
 * @chanlist: A #FacqChanlist Object.
 * @index: The position of the channel in the list.
 *
 * Returns the direction stored in the list for a particular channel that
 * is at position, @index, without taking into account special channels.
 *
 * Returns: A #FacqChanDir.
 *
 * <para>
 * <note>
 * @index can be any value in [0,facq_chanlist_get_io_chans_n()].
 * </note>
 * </para>
 */
FacqChanDir facq_chanlist_get_io_chan_direction(const FacqChanlist *chanlist,guint index)
{
	guint iochans_n = 0,first_io_idx = 0;
	FacqChanDir dir = 0;
	
	iochans_n = facq_chanlist_get_io_chans_n(chanlist);
	g_assert(FACQ_IS_CHANLIST(chanlist));
	g_assert(index < iochans_n);

	first_io_idx = facq_chanlist_get_length(chanlist) - iochans_n;

	dir = g_array_index(chanlist->priv->dirlist,guint,index+first_io_idx);
	return dir;
}

/**
 * facq_chanlist_to_comedi_chanlist:
 * @chanlist: A #FacqChanlist object.
 * @length:(out caller-allocates)(allow-none): Placeholder for the returned
 * array lenght. (It can be %NULL if you don't want this value).
 *
 * Given a #FacqChanlist, @chanlist, returns an array with all the contained
 * channels (Not chanspec) in the list. If you provide @length the length of
 * the array is set.
 *
 * Returns: An (guint) array. You have to free it with g_free(). %NULL will
 * be returned in case of an empty chanlist.
 */
guint *facq_chanlist_to_comedi_chanlist(const FacqChanlist *chanlist,guint *length)
{
	guint *ret = NULL, i = 0, j = 0,len = 0;
	FacqChanDir dir;
	g_return_val_if_fail(FACQ_IS_CHANLIST(chanlist),NULL);
	len = facq_chanlist_get_io_chans_n(chanlist);
	
	if(len == 0){
		if(length)
			*length = 0;
		return NULL;
	}
	
	ret = g_new0(guint,len);
	for(i = 0, j = 0;i < chanlist->priv->length && j < len;i++){
		dir = g_array_index(chanlist->priv->dirlist,FacqChanDir,i);
		if(dir == CHAN_INPUT || dir == CHAN_OUTPUT){
			ret[j] = g_array_index(chanlist->priv->uintlist,guint,i);
			j++;
		}
	}
	if(length)
		*length = len;
	return ret;
}

/**
 * facq_chanlist_to_nidaq_chanlist:
 * @device: The device name.
 * @chanlist: A #FacqChanlist. Note that this function only takes into
 * consideration channel numbers and directions. It should have one I/O channel
 * or more.
 * @length: (allow-none): If not %NULL it will be set to the number of I/O
 * channels in the chanlist.
 *
 * Creates a valid string describing a NIDAQmx virtual channel. 
 * For each I/O Channel in the chanlist a new string is appended to the final
 * string with the form: device/aiN so for example if you have channels 0,1,2
 * and the device name is Dev1 the function will return a string with the
 * following form: Dev1/ai0,Dev1/ai1,Dev1/ai2 . This string can be passed to the
 * NIDAQmx software.
 *
 * Returns: A valid NIDAQmx chanlist, you should free it when no longer
 * needed.
 */
gchar *facq_chanlist_to_nidaq_chanlist(const gchar *device,const FacqChanlist *chanlist,guint *length)
{
	gchar *ret = NULL, *channel = NULL;
	guint io_chans_n = 0, i = 0, chan_number = 0;
	GString *phys_channel = NULL;
	FacqChanDir dir = 0;

	g_return_val_if_fail(FACQ_IS_CHANLIST(chanlist),NULL);
	io_chans_n = facq_chanlist_get_io_chans_n(chanlist);
	g_return_val_if_fail( io_chans_n != 0,NULL);
	if(length)
		*length = io_chans_n;


	for(i = 0;i < io_chans_n;i++){
		chan_number = facq_chanlist_get_io_chanspec(chanlist,i);
		facq_chanlist_chanspec_to_src_values(chan_number,&chan_number,NULL,NULL,NULL);
		dir = facq_chanlist_get_io_chan_direction(chanlist,i);
		
		if(i == 0){
			phys_channel = g_string_new(NULL);

			if(dir == CHAN_INPUT) 
				channel = g_strdup_printf("%s/ai%u",device,chan_number);
			if(dir == CHAN_OUTPUT)
				channel = g_strdup_printf("%s/ao%u",device,chan_number);
		}
		else {
			if(dir == CHAN_INPUT)
				channel = g_strdup_printf(",%s/ai%u",device,chan_number);
			if(dir == CHAN_OUTPUT)
				channel = g_strdup_printf(",%s/ao%u",device,chan_number);
		}
		g_string_append(phys_channel,channel);
		g_free(channel);
	}
	ret = g_string_free(phys_channel,FALSE);
	return ret;
}

/**
 * facq_chanlist_get_comedi_conversion_direction_list:
 * @chanlist: A #FacqChanlist object.
 * @length:(out caller-allocates)(allow-none): Placeholder for the length of
 * the returned array. (Can be NULL if you don't want this value).
 *
 * Converts a provided #FacqChanlist, @chanlist, to a direction list that
 * can be understood by Comedi. This list can be used with the comedi
 * functions <function>comedi_get_hardcal_converter</function> and 
 * <function>comedi_get_softcal_converter</function>.
 *
 * Returns: A (enum comedi_conversion_direction) array, you must free it 
 * using g_free(). If the chanlist is empty %NULL will be returned.
 */
enum comedi_conversion_direction *facq_chanlist_get_comedi_conversion_direction_list(const FacqChanlist *chanlist,guint *length)
{
	enum comedi_conversion_direction *ret = NULL;
	guint i = 0,j = 0,len = 0;
	FacqChanDir dir;
	g_return_val_if_fail(FACQ_IS_CHANLIST(chanlist),NULL);
	len = facq_chanlist_get_io_chans_n(chanlist);

	if(len == 0) {
		if(length)
			*length = 0;
		return NULL;
	}
	
	ret = g_new0(enum comedi_conversion_direction,len);
	for(i = 0, j = 0;i < facq_chanlist_get_length(chanlist);i++){
		dir = g_array_index(chanlist->priv->dirlist,FacqChanDir,i);
		if(dir == CHAN_INPUT){
			ret[j] = COMEDI_TO_PHYSICAL;
			j++;
		}
		if(dir == CHAN_OUTPUT){
			ret[j] = COMEDI_FROM_PHYSICAL;
			j++;
		}
	}
	if(length)
		*length = len;
	return ret;
}

/**
 * facq_chanlist_chanspec_to_src_values:
 * @chanspec: A chanspec provided by the user.
 * @chan:(out caller-allocates)(allow-none): The channel in the chanspec.
 * @rng:(out caller-allocates)(allow-none): The range in the chanspec.
 * @aref:(out caller-allocates)(allow-none): The aref in the chanspec.
 * @flags:(out caller-allocates)(allow-none): The flags in the chanspec.
 * 
 * Given a chanspec extracts the contained fields in it according to the
 * parameters you want to obtain.
 */
void facq_chanlist_chanspec_to_src_values(guint chanspec,guint *chan,guint *rng,guint *aref,guint *flags)
{
	if(chan) *chan = CR_CHAN(chanspec);
	if(rng) *rng = CR_RANGE(chanspec);
	if(aref) *aref = CR_AREF(chanspec);
	if(flags) *flags = chanspec & CR_FLAGS_MASK;
}

/**
 * facq_chanlist_search_io_chan:
 * @chanlist: A #FacqChanlist object.
 * @channel: A channel number.
 * @index: (out caller-allocates) (allow-none): location to store actual index, or %NULL.
 *
 * Given a #FacqChanlist and a channel, @channel, the function searchs for
 * the channel in the list. If found @index is set to the first ocurrence and
 * the function returns %TRUE. (Note that @index can be %NULL, this can be
 * desired if you only want to test for the presence of @channel in the list).
 *
 * Returns: %TRUE if the channel is found, %FALSE in other case.
 */
gboolean facq_chanlist_search_io_chan(const FacqChanlist *chanlist,guint channel,guint *index)
{
	guint i = 0, io_chan_n = 0, chanspec = 0, chan = 0;

	io_chan_n = facq_chanlist_get_io_chans_n(chanlist);
	if(io_chan_n == 0)
		return FALSE;
	for(i = 0; i < io_chan_n;i++){
		chanspec = facq_chanlist_get_io_chanspec(chanlist,i);
		facq_chanlist_chanspec_to_src_values(chanspec,&chan,NULL,NULL,NULL);
		if(chan == channel){
			if(index)
				*index = i;
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * facq_chanlist_to_key_file:
 * @chanlist: A #FacqChanlist object.
 * @file: A #GKeyFile object.
 * @group: The group name inside the #GKeyFile, to write the chanlist.
 *
 * Stores the data contained in the #FacqChanlist in a human readable string, 
 * with a key called "chanlist" in a #GKeyFile. The format used to store the
 * data is the following:
 * channel number-range-analog reference-flags-direction, parameters of next
 * channel,...
 */
void facq_chanlist_to_key_file(const FacqChanlist *chanlist,GKeyFile *file,const gchar *group)
{
	guint chanspec = 0, chan = 0, range = 0, aref = 0, flags = 0;
	FacqChanDir dir = 0;
	guint i = 0, n_channels = 0;
	GString *cstr = NULL;
	gchar *chunk = NULL;

	g_return_if_fail(g_key_file_has_group(file,group));

	cstr = g_string_new(NULL);
	n_channels = facq_chanlist_get_io_chans_n(chanlist);
	for(i = 0;i < n_channels;i++){
		chanspec = facq_chanlist_get_io_chanspec(chanlist,i);
		facq_chanlist_chanspec_to_src_values(chanspec,&chan,&range,&aref,&flags);
		dir = facq_chanlist_get_io_chan_direction(chanlist,i);
		if(i < n_channels-1)
			chunk = g_strdup_printf("%u-%u-%u-%u-%u,",chan,range,aref,flags,(guint)dir);
		if(i == n_channels-1)
			chunk = g_strdup_printf("%u-%u-%u-%u-%u",chan,range,aref,flags,(guint)dir);
		g_string_append(cstr,chunk);
		g_free(chunk);
	}
	g_key_file_set_string(file,group,"chanlist",cstr->str);
	g_string_free(cstr,TRUE);
}

/**
 * facq_chanlist_from_key_file:
 * @file: A #GKeyFile.
 * @group_name: The group name in the #GKeyFile
 * @err: A #GError it will be set in case of error if not %NULL
 *
 * Creates a new #FacqChanlist object from an input human readable text
 * contained in a #GKeyFile object, under some group_name. The tag searched
 * in the group will be "chanlist". The format of the channels in the chanlist
 * will have the following format:
 * channel number-range-aref-flags-direction,... and so on. Each channel
 * specification will be separated by a ",".
 *
 * Returns: A new #FacqChanlist object, or %NULL in case of error.
 */
FacqChanlist *facq_chanlist_from_key_file(GKeyFile *file,const gchar *group_name,GError **err)
{
	FacqChanlist *chanlist = NULL;
	guint chan = 0, range = 0, aref = 0, flags = 0, n_channels = 0, i = 0;
	FacqChanDir dir = 0;
	gchar *chanlist_string = NULL;
	gchar **channels = NULL;
	gchar **chan_info = NULL;
	GError *local_err = NULL;

	g_return_val_if_fail(g_key_file_has_group(file,group_name),NULL);

	chanlist_string = g_key_file_get_string(file,group_name,"chanlist",&local_err);
	if(local_err)
		goto error;
	
	channels = g_strsplit(chanlist_string,",",0);

	i = 0;
	n_channels = 0;
	while(channels[i] != NULL){
		if(channels[i] != NULL)
			n_channels++;
		i++;
	}
	if(n_channels){
		chanlist = facq_chanlist_new();
		for(i = 0;i < n_channels;i++){
			chan_info = g_strsplit(channels[i],"-",0);
			
			if(chan_info[0])
				chan = (guint) g_ascii_strtod(chan_info[0],NULL);
			if(chan_info[1])
				range = (guint) g_ascii_strtod(chan_info[1],NULL);
			if(chan_info[2])
				aref = (guint) g_ascii_strtod(chan_info[2],NULL);
			if(chan_info[3])
				flags = (guint) g_ascii_strtod(chan_info[3],NULL);
			if(chan_info[4])
				dir = (FacqChanDir) g_ascii_strtod(chan_info[4],NULL);
			
			facq_chanlist_add_chan(chanlist,chan,range,aref,flags,dir);
			g_strfreev(chan_info);
		}
	}
	g_strfreev(channels);
	g_free(chanlist_string);
	return chanlist;

	error:
	if(chan_info)
		g_strfreev(chan_info);
	if(channels)
		g_strfreev(channels);
	if(chanlist_string)
		g_free(chanlist_string);
	if(chanlist)
		facq_chanlist_free(chanlist);
	if(err && local_err){
		g_propagate_error(err,local_err);
	}
	return NULL;
}

/**
 * facq_chanlist_free:
 * @chanlist: a #FacqChanlist object.
 *
 * Destroys a #FacqChanlist object.
 */
void facq_chanlist_free(FacqChanlist *chanlist)
{
	g_assert(FACQ_IS_CHANLIST(chanlist));
	g_object_unref(chanlist);
}

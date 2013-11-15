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
#if HAVE_CONFIG_H
#include <config.h>
#endif
#if USE_COMEDI
#include <glib.h>
#include <gio/gio.h>
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqcomedimisc.h"

/**
 * SECTION:facqcomedimisc
 * @title:FacqComediMisc
 * @short_description: miscellaneous functions for dealing with comedi.
 *
 * This module contains miscellaneous functions, that are not provided
 * by comedi, but are directly coupled with the comedi library.
 *
 * Much of the functionality provided by this module could be providad
 * by the comedi library, so maybe we can evaluate if we can put some
 * of this functions in the comedi library source later.
 */

/**
 * FacqComediMiscError:
 * @FACQ_COMEDI_MISC_ERROR_FAILED: Some operation failed.
 *
 * Enum values for errors in the facqcomedimisc module.
 */

GQuark facq_comedi_misc_error_quark(void)
{
        return g_quark_from_static_string("facq-comedi-misc-error-quark");
}

/*
 * facq_comedi_misc_test_chanspec_ai:
 *
 * See facq_comedi_misc_test_chanspec for details.
 */
static gboolean facq_comedi_misc_test_chanspec_ai(guint subd_flags,guint aref,FacqChanDir dir,GError **err)
{
	if(!facq_comedi_misc_test_aref(subd_flags,aref)){
		g_set_error_literal(err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				"Invalid analog reference");
		return FALSE;
	}
	if(subd_flags & SDF_CMD){
		if(dir != CHAN_INPUT && dir != CHAN_BEGIN_EXT
			&& dir != CHAN_CONVERT_EXT 
				&& dir != CHAN_CONVERT_EXT){
		g_set_error_literal(err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				"Invalid direction value");
		return FALSE;
		}
	}
	else{
		if(dir != CHAN_INPUT){
		g_set_error_literal(err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				"Invalid direction value");
		return FALSE;
		}
	}
	return TRUE;
}

/*
 * facq_comedi_misc_test_chanspec_di:
 *
 * See facq_comedi_misc_test_chanspec for details.
 */
static gboolean facq_comedi_misc_test_chanspec_di(guint subd_flags,guint aref,FacqChanDir dir,GError **err)
{
	if(aref != 0){
		g_set_error_literal(err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				"Invalid analog reference");
		return FALSE;
	}
	if(subd_flags & SDF_CMD){
		if(dir == CHAN_OUTPUT){
			g_set_error_literal(err,FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					"Invalid direction value");
			return FALSE;
		}
	}
	else {
		if(dir != CHAN_INPUT && dir != CHAN_BASE){
			g_set_error_literal(err,FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					"Invalid direction value");
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * facq_comedi_misc_test_chanspec_dio:
 *
 * See facq_comedi_misc_test_chanspec for details.
 */
static gboolean facq_comedi_misc_test_chanspec_dio(guint subd_flags,guint aref,FacqChanDir dir,GError **err)
{
	if(aref != 0){
		g_set_error_literal(err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				"Invalid analog reference");
		return FALSE;
	}
	if(subd_flags & SDF_CMD){
		return TRUE;
	}
	else {
		if(dir != CHAN_INPUT 
			&& dir != CHAN_OUTPUT && dir != CHAN_BASE){
			g_set_error_literal(err,FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					"Invalid direction value");
			return FALSE;
		}
	}
	
	return TRUE;
}

/*
 * facq_comedi_misc_test_chanspec:
 *
 * The functions checks if a chanspec is correct for a subdevice,
 * using the data read from the subdevice and the chanspec values.
 * Ensures:
 * - chan value is fine. chan € [0,comedi_get_n_channels()]
 * - range value is fine. chan € [0,comedi_get_n_ranges()]
 * - aref value is fine. (For digital devices it should be 0).
 * - flags is fine. (Checks CR_ALT_FILTER in subd_flags)
 * - dir is fine. (Direction values are adequate to the subdev type).
 *
 * For digital subdevices aref should always be equal to 0, for all
 * the channels.
 * For AI (SYNC ONLY) subdevices, dir should be CHAN_INPUT.
 * For DI (SYNC ONLY) subdevices, dir € (CHAN_INPUT,CHAN_BASE).
 * For DIO (SYNC_ONLY) subdevices, dir €(CHAN_INPUT,CHAN_OUTPUT,CHAN_BASE)
 * For AI (A+S) subdevices, dir € (CHAN_INPUT, CHAN_*_EXT).
 * For DI (A+S) subdevices, dir € (CHAN_INPUT, CHAN_*_EXT, CHAN_BASE).
 * For DIO (A+S) subdevices, dir € (*).
 *
 * The logic for dir and aref is encapsulated in the 3 functions above 
 * this one.
 */
static gboolean facq_comedi_misc_test_chanspec(guint n_channels,guint n_ranges,guint subd_flags,guint chan,guint range,guint aref,guint flags,FacqChanDir dir,gint subdevice_type,GError **err)
{
	gboolean ret = TRUE;
	GError *local_err = NULL;

	if(chan >= n_channels){
		g_set_error_literal(&local_err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				"Invalid channel number");
		goto error;
	}
	if(range >= n_ranges){
		g_set_error_literal(&local_err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				"Invalid range number");
		goto error;
	}
	if(!facq_comedi_misc_test_channel_flags(subd_flags,flags)){
		g_set_error_literal(&local_err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				"Invalid channel flags");
		goto error;
	}

	switch(subdevice_type){
	case COMEDI_SUBD_AI:
	ret = facq_comedi_misc_test_chanspec_ai(subd_flags,
					  	aref,
					  	dir,
					  	err);
	break;
	case COMEDI_SUBD_DI:
	ret = facq_comedi_misc_test_chanspec_di(subd_flags,
						aref,
						dir,
						err);
	break;
	case COMEDI_SUBD_DIO:
	ret = facq_comedi_misc_test_chanspec_dio(subd_flags,
						 aref,
						 dir,
						 err);
	break;
	}
	
	return ret;

	error:
	if(local_err)
		g_propagate_error(err,local_err);
	return FALSE;
}

static comedi_polynomial_t *facq_comedi_misc_get_polynomial_soft(comedi_t *dev,guint subindex,const FacqChanlist *chanlist,comedi_calibration_t *cc,GError **err)
{
	comedi_polynomial_t *p = NULL;
	guint iochans_n = 0, i = 0;
	guint chanspec = 0, chan = 0, range = 0;
	enum comedi_conversion_direction *dir = NULL;

	iochans_n = facq_chanlist_get_io_chans_n(chanlist);
	p = g_new0(comedi_polynomial_t,iochans_n);
	dir = facq_chanlist_get_comedi_conversion_direction_list(chanlist,NULL);
	for(i = 0;i < iochans_n;i++){
		chanspec = facq_chanlist_get_io_chanspec(chanlist,i);
		facq_chanlist_chanspec_to_src_values(chanspec,
					&chan,&range,
						NULL,NULL);
		if( comedi_get_softcal_converter(subindex,
						 chan,
						 range,
						 dir[i],
						 cc,
						 &p[i]) < 0){
			g_set_error_literal(err,FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
			g_free(dir);
			return NULL;
		}
	}
	g_free(dir);
	return p;
}

static comedi_polynomial_t *facq_comedi_misc_get_polynomial_hard(comedi_t *dev,guint subindex,const FacqChanlist *chanlist,GError **err)
{
	comedi_polynomial_t *p = NULL;
	guint iochans_n = 0, i = 0;
	guint chanspec = 0, chan = 0, range = 0;
	enum comedi_conversion_direction *dir = NULL;

	iochans_n = facq_chanlist_get_io_chans_n(chanlist);
	p = g_new0(comedi_polynomial_t,iochans_n);
	dir = facq_chanlist_get_comedi_conversion_direction_list(chanlist,NULL);
	for(i = 0;i < iochans_n;i++)
		chanspec = facq_chanlist_get_io_chanspec(chanlist,i);
		facq_chanlist_chanspec_to_src_values(chanspec,
				&chan,&range,
					NULL,NULL);
		if( comedi_get_hardcal_converter(dev,
		 			         subindex,
					         chan,
					         range,
					         dir[i],
					         &p[i]) < 0 ){
			g_set_error_literal(err,FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
			g_free(dir);
			return NULL;
		}

	g_free(dir);
	return p;
}

/*---- public methods ----*/

/**
 * facq_comedi_misc_test_aref:
 * @subd_flags: The subdevice flags, see 
 * <function>comedi_get_subdevice_flags()</function>
 * @aref: An analog reference.
 *
 * Checks if the analog reference @aref is contained in the 
 * subdevice flags @subd_flags. 
 *
 * Returns: %TRUE if supported, %FALSE in the other case.
 */
gboolean facq_comedi_misc_test_aref(guint subd_flags,guint aref)
{
	switch(aref){
	case AREF_GROUND: 
		return (subd_flags & SDF_GROUND) ? TRUE : FALSE;
	case AREF_COMMON: 
		return (subd_flags & SDF_COMMON) ? TRUE : FALSE;
	case AREF_DIFF: 
		return (subd_flags & SDF_DIFF) ? TRUE : FALSE;
	case AREF_OTHER: 
		return (subd_flags & SDF_OTHER) ? TRUE : FALSE;
	default: 
		return FALSE;
	}

	return TRUE;
}

/**
 * facq_comedi_misc_test_channel_flags:
 * @subd_flags: The subdevice flags, see
 * <function>comedi_get_subdevice_flags()</function>.
 * @flags: The flags to check. At the moment only %CR_ALT_FILTER is
 * supported.
 *
 * Checks if the channel flags, @flags, are supported by the subdevice.
 *
 * Returns: %TRUE if supported %FALSE in other case.
 */
gboolean facq_comedi_misc_test_channel_flags(guint subd_flags,guint flags)
{
	if(flags != 0){
		if(flags & CR_ALT_FILTER){
			if(subd_flags & SDF_DITHER || subd_flags & SDF_DEGLITCH)
				return TRUE;
		}
		return FALSE;
	}
	return TRUE;
}

/**
 *  facq_comedi_misc_cmd_new:
 *  @subindex: The subdevice index.
 *
 *  Creates a new comedi_cmd.
 *
 *  Returns: A new comedi_cmd structure, you must free it with g_free().
 */
comedi_cmd *facq_comedi_misc_cmd_new(guint subindex)
{
	comedi_cmd *ret = NULL;

	ret = g_new0(comedi_cmd,1);
	ret->subdev = subindex;
	return ret;
}

/**
 * facq_comedi_misc_test_period:
 * @dev: A comedi_t device.
 * @subindex: The subdevice index.
 * @n_channels: The number of channels in the chanlist.
 * @period: The sampling period in nanoseconds.
 * @err: a #GError.
 *
 * Checks if the sampling period is supported by the subdevice.
 *
 * Returns: %TRUE if the period is supported %FALSE in other case.
 */
gboolean facq_comedi_misc_test_period(comedi_t *dev,guint subindex,guint n_channels,guint period,GError **err)
{
	comedi_cmd *cmd = facq_comedi_misc_cmd_new(subindex);

	if( comedi_get_cmd_generic_timed(dev,
					 subindex,
					 cmd,
					 n_channels,
					 period) < 0){
		g_set_error_literal(err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		g_free(cmd);
		return FALSE;
	}
	if(cmd->scan_begin_arg != period){
		g_set_error(err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				"Period not supported try %u instead",
					cmd->scan_begin_arg);
		g_free(cmd);
		return FALSE;
	}
	g_free(cmd);
	return TRUE;
}

/**
 * facq_comedi_misc_test_chanlist:
 * @dev: A comedi_t device.
 * @subindex: A subdevice index.
 * @chanlist: A #FacqChanlist object.
 * @err: A #GError.
 *
 * Checks if the provided #FacqChanlist, @chanlist, can be used with
 * the subdevice @subindex, in the device @dev.
 *
 * Returns: %TRUE if supported %FALSE in other case.
 */
gboolean facq_comedi_misc_test_chanlist(comedi_t *dev,guint subindex,const FacqChanlist *chanlist,GError **err)
{
	GError *local_err = NULL;
	guint i = 0, len = 0, iochans_n = 0;
	guint n_channels = 0, n_ranges = 0, subd_flags = 0;
	guint chanspec = 0, chan = 0, range = 0, aref = 0, flags = 0;
	FacqChanDir dir = 0;
	gint subdevice_type = 0;

	g_return_val_if_fail(FACQ_IS_CHANLIST(chanlist),FALSE);
	len = facq_chanlist_get_length(chanlist);
	iochans_n = facq_chanlist_get_io_chans_n(chanlist);
	if(len < 1 || iochans_n == 0){
		g_set_error(&local_err,FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					"Chanlist needs at least one I/O channel");
		goto error;
	}
	subdevice_type = comedi_get_subdevice_type(dev,subindex);
	if(subdevice_type < 0){
		g_set_error_literal(&local_err,FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
		goto error;
	}
	subd_flags = comedi_get_subdevice_flags(dev,subindex);
	if(subd_flags < 0){
		g_set_error_literal(&local_err,FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
		goto error;
	}
	n_channels = comedi_get_n_channels(dev,subindex);
	if(n_channels < 0){
		g_set_error_literal(&local_err,FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
		goto error;
	}
	for(i = 0;i < len;i++){
		chanspec = facq_chanlist_get_io_chanspec(chanlist,i);
		facq_chanlist_chanspec_to_src_values(chanspec,
							&chan,&range,
								&aref,&flags);
		dir = facq_chanlist_get_io_chan_direction(chanlist,i);
		n_ranges = comedi_get_n_ranges(dev,subindex,chan);
		if(n_ranges < 0){
			g_set_error_literal(&local_err,FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
			goto error;	
		}
		if(!facq_comedi_misc_test_chanspec(n_channels,n_ranges,
						subd_flags,chan,range,
						aref,flags,dir,
						subdevice_type,
						&local_err))
			goto error;
	}
	return TRUE;

	error:
	if(local_err)
		g_propagate_error(err,local_err);
	return FALSE;
}

/**
 * facq_comedi_misc_test_flags:
 * @dev: A comedi_t device.
 * @subindex: The subdevice index.
 * @flags: The flags to check.
 * @err: A #GError.
 *
 * Checks if the cmd flags @flags, are supported by the subdevice.
 *
 * Returns: %TRUE if supported %FALSE in other case.
 */
gboolean facq_comedi_misc_test_flags(comedi_t *dev,guint subindex,guint flags,GError **err)
{
	return TRUE;
}

/**
 * facq_comedi_misc_test_calibrated:
 * @dev: A comedi_t device.
 * @err: A #GError.
 *
 * Checks if the comedi device @dev, has been calibrated or not.
 * If a device was calibrated, a file with the calibration data
 * should exist. This function checks for this file.
 *
 * Returns: %TRUE if calibrated, %FALSE in other case.
 *
 * <para>
 * <note>
 * A malicious user can delete the file after the check is done,
 * but the comedi functions can deal with that. This is a simple
 * check.
 * </note>
 * </para>
 */
gboolean facq_comedi_misc_test_calibrated(comedi_t *dev,GError **err)
{
	gchar *cal_filename = NULL;
	gboolean ret = FALSE;

	cal_filename = comedi_get_default_calibration_path(dev);
	if(!cal_filename){
		g_set_error_literal(err,FACQ_COMEDI_MISC_ERROR,
					FACQ_COMEDI_MISC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		g_free(cal_filename);
		return FALSE;
	}
	ret = g_file_test(cal_filename,G_FILE_TEST_EXISTS);
	g_free(cal_filename);
	return ret;
}

/**
 * facq_comedi_misc_cmd_add_chanlist:
 * @cmd: A comedi_cmd struct.
 * @chanlist: A #FacqChanlist object.
 *
 * Reads the @chanlist object, and configures @cmd according to it.
 * Translates special directions to src_events and args.
 */
void facq_comedi_misc_cmd_add_chanlist(comedi_cmd *cmd,const FacqChanlist *chanlist)
{
	guint i = 0, n_channels = 0, chanspec = 0, channel = 0;
	FacqChanDir dir = 0;

	g_return_if_fail(FACQ_IS_CHANLIST(chanlist));

	n_channels = facq_chanlist_get_length(chanlist);

	//search the list for CHAN_*_EXT and change
	//the cmd args and srcs.
	for(i = 0;i < n_channels;i++){
		chanspec = facq_chanlist_get_chanspec(chanlist,i);
		facq_chanlist_chanspec_to_src_values(chanspec,&channel,
							NULL,NULL,NULL);
		dir = facq_chanlist_get_chan_direction(chanlist,i);
		switch(dir){
		case CHAN_START_EXT:
			cmd->start_src = TRIG_EXT;
			cmd->start_arg = channel;
		continue;
		case CHAN_BEGIN_EXT:
			cmd->scan_begin_src = TRIG_EXT;
			cmd->scan_begin_arg = channel;
		continue;
		case CHAN_CONVERT_EXT:
			cmd->scan_begin_src = TRIG_EXT;
			cmd->scan_begin_arg = channel;
		continue;
		default:
		continue;
		}
	}

	cmd->chanlist =	
		facq_chanlist_to_comedi_chanlist(chanlist,
						&cmd->chanlist_len);
	
	return;
}

/**
 * facq_comedi_misc_can_calibrate
 * @dev: A comedi_t device.
 * @subindex: The subdevice index.
 * @err: A #GError.
 *
 * Checks if a subdevice can be calibrated, in affirmative case returns
 * the supported calibration type.
 *
 * Returns: -1 in case of error, 0 if device can't be calibrated, 1 if
 * device can be soft-calibrated, 2 if device is hard-calibrated.
 */
gint facq_comedi_misc_can_calibrate(comedi_t *dev,guint subindex,GError **err)
{
	guint subd_flags = 0;
	GError *local_err = NULL;
	const gchar *driver_name = NULL;
	guint i = 0, drivers_len = 6;
	const gchar * const drivers[] =
        {
                "ni_pcimio", "ni_atmio",
                "ni_mio_cs", "cb_pcidas",
                "cb_pcidas64", "ni_labpc"
        };

	subd_flags = comedi_get_subdevice_flags(dev,subindex);
	if(subd_flags < 0){
		g_set_error_literal(&local_err,FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
		goto error;
	}
	if(subd_flags & SDF_SOFT_CALIBRATED)
		return 1;
	else {
		driver_name = comedi_get_driver_name(dev);
		if(!driver_name){
			g_set_error_literal(&local_err,FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
			goto error;
		}
		for(i = 0;i < drivers_len;i++)
			if(g_strcmp0(driver_name,drivers[i]) == 0)
				return 2;
	}
	return 0;

	error:
	if(local_err)
		g_propagate_error(err,local_err);
	return -1;
}

/**
 * facq_comedi_misc_get_polynomial:
 * @dev: A comedi_t device.
 * @subindex: The subdevice index.
 * @chanlist: A #FacqChanlist object.
 * @err: A #GError.
 *
 * The functions makes some magic to obtain a per channel,
 * comedi_polynomial_t array. Each polynomial can be used
 * to convert from comedi data to physical samples.
 * See <function>comedi_to_physical()</function> for more 
 * info.
 *
 * Returns: An array of comedi_polynomial_t members. Free it
 * when it's no longer needed. %NULL in case of error.
 * The array length equals the number of I/O Channels in the
 * chanlist.
 */
comedi_polynomial_t *facq_comedi_misc_get_polynomial(comedi_t *dev,guint subindex,const FacqChanlist *chanlist,GError **err)
{
	guint subd_flags = 0;
	guint i = 0, chan = 0, range = 0, aref = 0, iochans_n = 0;
	guint chanspec = 0;
	gchar *cal_filename = NULL;
	comedi_calibration_t *cc = NULL;
	GError *local_err = NULL;
	comedi_polynomial_t *p = NULL;

	g_return_val_if_fail(FACQ_IS_CHANLIST(chanlist),NULL);
	iochans_n = facq_chanlist_get_io_chans_n(chanlist);
	if(iochans_n < 1){
		g_set_error_literal(&local_err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				"Chanlist is empty");
		goto error;
	}
	cal_filename = comedi_get_default_calibration_path(dev);
	if(!cal_filename){
		g_set_error_literal(&local_err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		goto error;
	}
	cc = comedi_parse_calibration_file(cal_filename);
	if(!cc){
		g_set_error_literal(&local_err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		goto error;
	}
	g_free(cal_filename);
	subd_flags = comedi_get_subdevice_flags(dev,subindex);
	if(subd_flags < 0){
		g_set_error_literal(&local_err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		goto error;
	}
	for(i = 0;i < iochans_n;i++){
		chanspec = facq_chanlist_get_io_chanspec(chanlist,i);
		facq_chanlist_chanspec_to_src_values(chanspec,
						     &chan,
						     &range,
						     &aref,
						     NULL);
		if( comedi_apply_parsed_calibration(dev,subindex,
						chan,
						range,
						aref,
						cc) < 0){
			g_set_error_literal(&local_err,FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					comedi_strerror(comedi_errno()));
			goto error;
		}
	}
	if(subd_flags & SDF_SOFT_CALIBRATED){
		p = facq_comedi_misc_get_polynomial_soft(dev,
							 subindex,
							 chanlist,
							 cc,
							 &local_err);
		comedi_cleanup_calibration(cc);
	}
	else {
		comedi_cleanup_calibration(cc);
		p = facq_comedi_misc_get_polynomial_hard(dev,
							 subindex,
							 chanlist,
							 &local_err);
	}
	if(!p)
		goto error;

	return p;

	error:
	if(cal_filename)
		g_free(cal_filename);
	if(cc)
		comedi_cleanup_calibration(cc);
	if(local_err)
		g_propagate_error(err,local_err);
	return NULL;
}

/**
 * facq_comedi_misc_get_units:
 * @dev: A comedi_t device.
 * @subindex: The subdevice index.
 * @chanlist: A #FacqChanlist object.
 * @err: A #GError.
 *
 * Creates a new FacqUnits array according to the device and the
 * chanlist info. The length of this array is equal to the number
 * of I/O channels in the chanlist.
 *
 * Returns: A #FacqUnits array, or %NULL in case of error.
 */
FacqUnits *facq_comedi_misc_get_units(comedi_t *dev,guint subindex,const FacqChanlist *chanlist,GError **err)
{
	guint iochans_n = 0, i = 0, chanspec = 0, chan = 0,range = 0;
	GError *local_err = NULL;
	FacqUnits *units = NULL;
	comedi_range *rng = NULL;

	g_return_val_if_fail(FACQ_IS_CHANLIST(chanlist),NULL);

	iochans_n = facq_chanlist_get_io_chans_n(chanlist);
	if(!iochans_n){
		g_set_error_literal(&local_err,
			FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					"The chanlist is empty");
		goto error;
	}
	units = g_new0(FacqUnits,iochans_n);
	for(i = 0;i < iochans_n;i++){
		chanspec = facq_chanlist_get_io_chanspec(chanlist,i);
		facq_chanlist_chanspec_to_src_values(chanspec,&chan,
						     &range,NULL,NULL);
		rng = comedi_get_range(dev,subindex,chan,range);
		if(!rng){
			g_set_error_literal(&local_err,
				FACQ_COMEDI_MISC_ERROR,
					FACQ_COMEDI_MISC_ERROR_FAILED,
						comedi_strerror(comedi_errno()));
		}
		units[i] = rng->unit;
	}

	return units;
	
	error:
	if(units)
		g_free(units);
	if(local_err)
		g_propagate_error(err,local_err);
	return NULL;
}

/**
 * facq_comedi_misc_get_max:
 * @dev: A comedi_t device.
 * @subindex: The subdevice index.
 * @chanlist: A #FacqChanlist object.
 * @err: A #GError.
 *
 * Returns an array with the maximum real expected value per I/O channel.
 * The length of this new array is equal to the number of I/O channels
 * in the chanlist.
 *
 * Returns: A gdouble array. Free it with g_free().
 */
gdouble *facq_comedi_misc_get_max(comedi_t *dev,guint subindex,const FacqChanlist *chanlist,GError **err)
{
	guint iochans_n = 0, i = 0, chanspec = 0, chan = 0,range = 0;
	GError *local_err = NULL;
	gdouble *max = NULL;
	comedi_range *rng = NULL;

	g_return_val_if_fail(FACQ_IS_CHANLIST(chanlist),NULL);

	iochans_n = facq_chanlist_get_io_chans_n(chanlist);
	if(!iochans_n){
		g_set_error_literal(&local_err,
			FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					"The chanlist is empty");
		goto error;
	}
	max = g_new0(gdouble,iochans_n);
	for(i = 0;i < iochans_n;i++){
		chanspec = facq_chanlist_get_io_chanspec(chanlist,i);
		facq_chanlist_chanspec_to_src_values(chanspec,&chan,
						     &range,NULL,NULL);
		rng = comedi_get_range(dev,subindex,chan,range);
		if(!rng){
			g_set_error_literal(&local_err,
				FACQ_COMEDI_MISC_ERROR,
					FACQ_COMEDI_MISC_ERROR_FAILED,
						comedi_strerror(comedi_errno()));
		}
		max[i] = rng->max;
	}

	return max;
	
	error:
	if(max)
		g_free(max);
	if(local_err)
		g_propagate_error(err,local_err);
	return NULL;
}

/**
 * facq_comedi_misc_get_min:
 * @dev: A comedi_t device.
 * @subindex: The subdevice index.
 * @chanlist: A #FacqChanlist object.
 * @err: A #GError.
 *
 * It's equal to facq_comedi_misc_get_max() but instead of returning the
 * maximum values, returns the minimum expected real values.
 *
 * Returns: A gdouble array. Free it with g_free():
 */
gdouble *facq_comedi_misc_get_min(comedi_t *dev,guint subindex,const FacqChanlist *chanlist,GError **err)
{
	guint iochans_n = 0, i = 0, chanspec = 0, chan = 0,range = 0;
	GError *local_err = NULL;
	gdouble *min = NULL;
	comedi_range *rng = NULL;

	g_return_val_if_fail(FACQ_IS_CHANLIST(chanlist),NULL);

	iochans_n = facq_chanlist_get_io_chans_n(chanlist);
	if(!iochans_n){
		g_set_error_literal(&local_err,
			FACQ_COMEDI_MISC_ERROR,
				FACQ_COMEDI_MISC_ERROR_FAILED,
					"The chanlist is empty");
		goto error;
	}
	min = g_new0(gdouble,iochans_n);
	for(i = 0;i < iochans_n;i++){
		chanspec = facq_chanlist_get_io_chanspec(chanlist,i);
		facq_chanlist_chanspec_to_src_values(chanspec,&chan,
						     &range,NULL,NULL);
		rng = comedi_get_range(dev,subindex,chan,range);
		if(!rng){
			g_set_error_literal(&local_err,
				FACQ_COMEDI_MISC_ERROR,
					FACQ_COMEDI_MISC_ERROR_FAILED,
						comedi_strerror(comedi_errno()));
		}
		min[i] = rng->min;
	}

	return min;
	
	error:
	if(min)
		g_free(min);
	if(local_err)
		g_propagate_error(err,local_err);
	return NULL;
}

/**
 * facq_comedi_misc_get_bps:
 * @dev:A comedi_t device.
 * @subindex: The subdevice index.
 * @err: A #GError.
 *
 * Checks the subdevice flags to get the number of bytes per sample
 * used by the subdevice.
 *
 * Returns: The number of bytes per sample, or 0 in case of error.
 */
guint facq_comedi_misc_get_bps(comedi_t *dev,guint subindex,GError **err)
{
	guint subd_flags = 0;

	subd_flags = comedi_get_subdevice_flags(dev,subindex);
	if(subd_flags < 0){
		g_set_error_literal(err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		return 0;
	}
	if(subd_flags & SDF_LSAMPL)
		return sizeof(lsampl_t);
	else
		return sizeof(sampl_t);
}

/*
 * facq_comedi_misc_can_poll:
 * @dev: A comedi_t device.
 * @err: A #GError NOT optional, check it.
 *
 * Some drivers can make the kernel crash when using poll()
 * with them, we blacklist them here to know if we can do
 * poll() or not.
 *
 * Returns: %TRUE if you can do poll() or %FALSE in the other case.
 */
gboolean facq_comedi_misc_can_poll(comedi_t *dev,GError **err)
{
	const gchar * const drivers[] =
	{
		"comedi_test"
	};
	guint drivers_len = 1, i = 0;
	const gchar *driver_name = NULL;

	driver_name = comedi_get_driver_name(dev);
	if(!driver_name){
		g_set_error_literal(err,FACQ_COMEDI_MISC_ERROR,
			FACQ_COMEDI_MISC_ERROR_FAILED,
				comedi_strerror(comedi_errno()));
		return FALSE;
	}
	for(i = 0;i < drivers_len;i++)
		if(g_strcmp0(driver_name,drivers[i]) == 0)
			return FALSE;

	return TRUE;
}
#endif //USE_COMEDI

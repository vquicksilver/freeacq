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
#if USE_NIDAQ
#include <glib.h>
#include <gio/gio.h>
#include <math.h>
#include "facqlog.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqnidaq.h"

/* To compile this module on Windows XP with NIDAQmx and mingw32 use 
 * the following command line:
 * 
 * $ gcc Acq-IntClk.c 
 * -L"/c/opt/NI/NI-DAQ/DAQmx ANSI C Dev/lib/msvc/" 
 * -I"/c/opt/NI/NI-DAQ/DAQmx ANSI C Dev/include/" -lnidaqmx
 *
 *  $ CPPFLAGS=-I/c/opt/NI/Shared/ExternalCompilerSupport/C/include 
 *  LDFLAGS="-L/c/WINDOWS/system32 -Wl,-enable-stdcall-fixup" 
 *  ./configure --enable-nidaqmx --disable-comedi --disable-fftw3 
 *  --disable-gtk-doc --disable-nidaqmxbase
 */

/*
 *  A good reference for learning how to code with this software
 * is http://www.ni.com/white-paper/5409/en
 *
 * Basic steps for using a DAQ device with NIDAQmx software are:
 * - Create a task with: 
 *	DAQmxBaseCreateTask(const char *task_name,       0 | W >0 | E < 0
 *			    TaskHandle *task_handle).
 * - Create a virtual channel and add it to the task with: 
 *	DAQmxBaseCreateAIVoltageChan  0
 *		(
 *		TaskHandle *task_handle,
 *		const char *physicalChannel,
 *		const char *nameToAssignToChannel,
 *		int32 terminalConfig,//Cfg_Default||RSE||NRSE||DIFF||PseudoDiff
 *		float64 minVal,
 *		float64 maxVal,
 *		int32 units,//DAQmx_Val_Volts
 *		const char *customScaleName
 *		)
 *   or DAQmxBaseCreateAIThrmcplChan() for analog input channels,
 *   or with DAQmxBaseCreateDIChan() for digital input.
 * - Set the sampling rate to control the rate of acquisition:
 *	DAQmxBaseCfgSampClkTiming
 *		(
 *		TaskHandle *task_handle,
 *		const char *source, //NULL or OnboardClock for internal.
 *		float64 rate, //sampling rate in samples per 
 *				second per channel
 *		int32 activeEdge, //DAQmx_Val_Rising || DAQmx_Val_Falling
 *		int32 sampleMode, //DAQmx_Val_FiniteSamps || DAQmx_Val_ContSamps
 *		uInt64 sampsPerChanToAcquire
 *		).
 * - Set triggers (Optional), with the triggering functions. For example this
 *   seems like a good idea, configure the task to start acquiring or generating
 *   samples inmmediately upon starting the task:
 *	DAQmxBaseDisableStartTrig(TaskHandle taskHandle) 0 |W > 0|E < 0
 * - Start the task with DAQmxBaseStartTask() (Also optional).
 * - Read or write to the device with DAQmxBaseRead*() or 
 *   DAQmxBaseWrite*() functions.
 * - Stop the task with DAQmxBaseStopTask().
 * - Destroy the task with DAQmxBaseClearTask().
 *
 *   AI ground settings:
 *   - DIFF: differential mode between 2 analog input signals.
 *   - RSE: Referenced Single Ended. Relative to GND.
 *   - NRSE: One analog input signal referenced to AI SENSE, or AI SENSE 2.
 */

/**
 * SECTION:facqnidaq
 * @short_description: Miscelaneous functions for dealing with NIDAQ software.
 * @title: FacqNIDAQ
 * @include: facqnidaq.h
 *
 * This is the FacqNIDAQ description.
 *
 * <sect1 id="nidaq-simulated-devices">
 *  <title>Working with NI-DAQmx simulated devices</title>
 *  <sect2 id="timming-triggering">
 *   <title>Timing and triggering considerations</title>
 *   <para>
 *   With NI-DAQmx simulated devices, the following timing and triggering considerations exist:
 *   
 *   - NI-DAQmx simulated devices simulate timing for continuous analog input, 
 *     digital input, and all output tasks. Timing is not simulated for counter tasks. 
 *   - NI-DAQmx simulated devices do not cause a timed loop to execute. 
 *   - NI-DAQmx simulated devices support software events. However, events that rely on the hardware, 
 *     such as a sample clock event, are not supported. 
 *   - Triggers always occur immediately. 
 *   - Watchdog timers do not expire.
 *   </para>
 *  </sect2>
 *  <sect2 id="task-behavior">
 *   <title>Task Behavior of NI-DAQmx Simulated Devices</title>
 *    <para>
 *    NI-DAQmx tasks using NI-DAQmx simulated devices are verified just as tasks 
 *    are on physical devices. If a property is set to an invalid value, the error 
 *    returned for an NI-DAQmx simulated device is identical to the error returned 
 *    for a real device. All resources necessary for the task are reserved for NI-DAQmx 
 *    simulated devices. RTSI lines, PXI Trigger lines, DMA channels, counters, and so on 
 *    are counted and reserved just as on physical devices.
 *    </para>
 *  </sect2>
 *  <sect2 id="read-write">
 *   <title>Reading and Writing Data with NI-DAQmx Simulated Devices</title>
 *   <para>
 *   All NI-DAQmx simulated devices return analog input data in the form of a full-scale 
 *   sine wave with 3% of full-scale noise. When multiple channels are in the task, the data 
 *   for each channel is offset 5 ° in time. Digital data is always returned as if each 
 *   eight-bit port were a binary counter. Counter data is always returned as 0. 
 *   Data written to an NI-DAQmx simulated device is scaled as if the device were real. 
 *   </para>
 *  </sect2>
 * </sect1>
 *
 * <sect1 id="data-format">
 *  <title>Data format</title>
 *   <para>
 *   Data format description.
 *   </para>
 * </sect1>
 * 
 * <sect1 id="nidaq-channels">
 *  <title>Channels in NIDAQ software</title>
 *  <para>
 *  A physical channel is a terminal or pin at which you can measure 
 *  or generate an analog or digital signal. A single physical channel 
 *  can include more than one terminal, as in the case of a differential 
 *  analog input channel or a digital port of eight lines. 
 *  Every physical channel on a device has a unique name (for instance, 
 *  SC1Mod4/ai0, Dev2/ao5, and Dev6/ctr3) that follows the NI-DAQmx 
 *  physical channel naming convention.
 *
 *  Virtual channels are software entities that encapsulate the physical 
 *  channel along with other channel specific information range, terminal 
 *  configuration, and custom scaling that formats the data. To create virtual 
 *  channels, use the DAQmx Create Virtual Channel function/VI or the DAQ Assistant.
 *  </para>
 *  <sect2 id="virtual-channels">
 *   <title>Types of virtual channels</title>
 *   <para>
 *   You can create a number of different types of virtual channels, depending on 
 *   the signal type: analog, digital, or counter and direction (input or output).
 *   </para>
 *   <sect3 id="analog-input">
 *    <title>Analog input channels</title>
 *      <para>
 *      Analog input channels measure different physical phenomena 
 *      using a variety of sensors. The type of channel to create 
 *      depends on the type of sensor and/or phenomenon you want to 
 *      read. For instance, you can create channels for measuring 
 *      temperature with a thermocouple, measuring current, measuring voltage, 
 *      and measuring voltage with excitation.
 *      </para>
 *   </sect3>
 *   <sect3 id="analog-output">
 *    <title>Analog output channels</title>
 *     <para>
 *     NI-DAQmx supports two types of phenomena, voltage and current. 
 *     You can use custom scales if the output from the device relates 
 *     to another unit of measure.
 *     </para>
 *   </sect3>
 *   <sect3 id="digital-io">
 *    <title>Digital I/O channels</title>
 *    <para>
 *    For digital channels, you can create both line-based and port-based 
 *    digital channels. A line-based channel can contain one or more digital 
 *    lines from one or more ports on a device. Reading or writing to a 
 *    line-based channel does not affect other lines on the hardware. 
 *    You can split lines in a particular port between multiple channels 
 *    and use those channels simultaneously within one or multiple tasks, 
 *    but the lines in a given channel must all be input lines or all be 
 *    output lines. Additionally, all channels in a task must be either 
 *    input channels or output channels. Some devices also require that 
 *    the lines of a given port all be input lines or output lines. 
 *    Check your device documentation for the capabilities of your device.
 *
 *    A port-based channel represents a fixed collection of lines on the device. 
 *    Reading or writing to a port affects all the lines on the port. 
 *    The number of lines in the port (commonly referred to as port width) 
 *    is hardware dependent and typically varies from 8 lines (MIO device) 
 *    to 32 lines (SCXI digital modules).
 *    </para>
 *   </sect3>
 *  </sect2>
 *  <sect2 id="phys-chan-syntax">
 *   <title>Physical Channel Syntax</title>
 *   <para>
 *   Use this syntax to refer to physical channels and groups of physical channels in NI-DAQmx.  
 *   </para>
 *   <sect3 id="phys-chan-names">
 *    <title>Physical Channel Names</title>
 *    <para>
 *    Physical channel names consist of a device identifier and a 
 *    slash (/) followed by a channel identifier. For example, if 
 *    the physical channel is Dev1/ai1, the device identifier is 
 *    Dev1, and the channel identifier is ai1. MAX assigns device 
 *    identifiers to devices in the order they are installed in the 
 *    system, such as Dev0 and Dev1. You also can assign arbitrary 
 *    device identifiers with MAX.
 *
 *    For analog I/O and counter I/O, channel identifiers combine 
 *    the type of the channel, such as analog input (ai), analog output (ao), 
 *    and counter (ctr), with a channel number such as the following:
 *    ai1, ctr0
 *
 *    For digital I/O, channel identifiers specify a port, which includes 
 *    all lines within a port: 
 *    port0
 *    
 *    Or, the channel identifier can specify a line within a port:
 *    port0/line1
 *
 *    All lines have a unique identifier. Therefore, you can use lines 
 *    without specifying which port they belong to. For example, line31 
 *    is equivalent to port3/line7 on a device with four 8-bit ports.
 *    </para>
 *   </sect3>
 *   <sect3 id="phys-chan-ranges">
 *    <title>Physical Channel Ranges</title>
 *    <para>
 *    To specify a range of physical channels, use a colon between two channel 
 *    numbers or two physical channel names:
 *    Dev1/ai0:4 or Dev1/ai0:Dev1/ai4
 *
 *    For digital I/O, you can specify a range of ports with a colon 
 *    between two port numbers:
 *    Dev1/port0:1
 *
 *    You also can specify a range of lines:
 *    Dev1/port0/line0:4
 *    Dev1/line0:31
 *
 *    You can specify channel ranges in reverse order:
 *    Dev1/ai4:0
 *    Dev1/ai4:Dev1/ai0
 *    Dev1/port1/line3:0
 *    </para>
 *   </sect3>
 *   <sect3 id="phys-chan-lists">
 *    <title>Physical Channel Lists</title>
 *    <para>
 *    Use commas to separate physical channel names and ranges in a list as follows:
 *    Dev1/ai0, Dev1/ai3:6 
 *    Dev1/port0, Dev1/port1/line0:2
 *    </para>
 *   </sect3>
 *  </sect2>
 * </sect1>
 */

GQuark facq_nidaq_error_quark(void)
{
	return g_quark_from_static_string("facq-nidaq-error-quark");
}

static gchar *facq_nidaq_check_error(gint32 error)
{
	gchar *err = NULL;
	gint32 err_size = 0;

	if( DAQmxFailed(error) ){
		err_size = _PREFIX_(GetExtendedErrorInfo(NULL,0));
		if(err_size == 0 || err_size < 0)
			return NULL;
		err = g_malloc0(err_size);
		_PREFIX_(GetExtendedErrorInfo(err,(guint32)err_size));
	}
	return err;
}

/**
 * facq_nidaq_task_new:
 * @name: (allow-none): The name of the task, can be %NULL.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 * 
 * Creates a new #FacqNIDAQTask structure.
 *
 * Returns: A new #FacqNIDAQTask structure.
 */
FacqNIDAQTask *facq_nidaq_task_new(const gchar *name,GError **err)
{
	FacqNIDAQTask *task = NULL;
	gint32 status = 0;
	gchar *errmsg = NULL;

	task = g_malloc0(sizeof(FacqNIDAQTask));

	status = _PREFIX_(CreateTask(name,&task->taskHandle));
	errmsg = facq_nidaq_check_error(status);
	if(errmsg){
		if(err != NULL)
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
		g_free(errmsg);
		g_free(task);
		task = NULL;
	}
	if(!task->taskHandle){
		g_free(task);
		task = NULL;
		if(err != NULL)
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,"Error creating task");
	}
	return task;
}

/**
 * facq_nidaq_task_start:
 * @task: A #FacqNIDAQTask structure.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * Starts a previously created and configured task. For doing something useful
 * the task needs to be configured first, see facq_nidaq_task_add_virtual_chan()
 * and facq_nidaq_task_setup_timing() for configuration options.
 *
 * Before you can read or write to the channels you must call this function.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_nidaq_task_start(FacqNIDAQTask *task,GError **err)
{
	gint32 status = 0;
	gchar *errmsg = NULL;

	if(!task)
		return FALSE;

	status = _PREFIX_(StartTask(task->taskHandle));
	errmsg = facq_nidaq_check_error(status);
	if(errmsg){
		if(err != NULL)
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
		g_free(errmsg);
		return FALSE;
	}
	return TRUE;
}

/**
 * facq_nidaq_task_add_virtual_chan:
 * @task: A #FacqNIDAQTask object.
 * @dev: The NIDAQmx device name.
 * @chanlist: A #FacqChanlist object. The first I/O Channel in the chanlist will
 * set the analog reference and the direction (Input or output) for all the other channels.
 * @max: The maximum value you expect to read/write.
 * @min: The minimum value you expect to read/write.
 * @err: (allow-none): A #GError, if not %NULL it will be set in case of error.
 *
 * Adds a virtual channel (One or more physical channels) to the task. When a
 * read or write operation is invoked using the task, @task, the physical
 * channels will be used. Note that the first channel in the chanlist will set
 * the analog reference of the DAQ card and the direction of all the other
 * channels.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_nidaq_task_add_virtual_chan(FacqNIDAQTask *task,const gchar *dev,const FacqChanlist *chanlist,gdouble max,gdouble min,GError **err)
{
	gint32 status = 0;
	gboolean ret = FALSE;
	gchar *errmsg = NULL;
	gint32 nidaq_aref = 0;
	FacqChanDir dir;
	
	g_return_val_if_fail(task && task->taskHandle && FACQ_IS_CHANLIST(chanlist) && dev,FALSE);
	g_return_val_if_fail(facq_chanlist_get_io_chans_n(chanlist) != 0,FALSE);

	if(max <= min){
		if(err != NULL)
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,
							"max should be greater than min");
		return ret;
	}

	task->phys_channel = facq_chanlist_to_nidaq_chanlist(dev,chanlist,&task->n_channels);
	nidaq_aref = (gint32) facq_chanlist_get_io_chanspec(chanlist,0);
	dir = facq_chanlist_get_io_chan_direction(chanlist,0);
	facq_chanlist_chanspec_to_src_values( (guint) nidaq_aref,
					NULL,NULL,(guint *)&nidaq_aref,NULL);
	
	if(dir == CHAN_INPUT){
		switch(nidaq_aref){
		case AREF_GROUND:
			nidaq_aref = DAQmx_Val_RSE;
		break;
		case AREF_COMMON:
			nidaq_aref = DAQmx_Val_NRSE;
		break;
		case AREF_DIFF:
			nidaq_aref = DAQmx_Val_Diff;
		break;
		case AREF_OTHER:
			nidaq_aref = DAQmx_Val_Cfg_Default;
		break;
		default:
			nidaq_aref = DAQmx_Val_Cfg_Default;
		}
		status = _PREFIX_(CreateAIVoltageChan(task->taskHandle,
				task->phys_channel,NULL,nidaq_aref,
					min,max,DAQmx_Val_Volts,NULL));
	}
	if(dir == CHAN_OUTPUT)
		status =  _PREFIX_(CreateAOVoltageChan(task->taskHandle,
				task->phys_channel,NULL,
					min,max,DAQmx_Val_Volts,NULL));

	errmsg = facq_nidaq_check_error(status);
	
	if(errmsg){
		if(err != NULL)
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
		g_free(errmsg);
		return FALSE;
	}
	
	return TRUE;
}

/**
 * facq_nidaq_task_done:
 * @task: A #FacqNIDAQTask structure.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * Returns the status of a task. A task is done when all the requested samples
 * have been read or written to the DAQ card.
 *
 * Returns: %TRUE if done, %FALSE in other case.
 */
gboolean facq_nidaq_task_done(FacqNIDAQTask *task,GError **err)
{
	gint32 status = 0;
	gboolean ret = FALSE;
	gchar *errmsg = NULL;

	status = _PREFIX_(IsTaskDone(task->taskHandle,(bool32 *)&ret));
	errmsg = facq_nidaq_check_error(status);
	if(errmsg){
		if(err != NULL)
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
		g_free(errmsg);
	}
	return ret;
}

/**
 * facq_nidaq_task_stop:
 * @task: A #FacqNIDAQTask structure.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Stops a previously started task. After the task has been stopped you
 * shouldn't read or write more samples to the card.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_nidaq_task_stop(FacqNIDAQTask *task,GError **err)
{
	gint32 status = 0;
	gchar *errmsg = NULL;

	if(!task)
		return TRUE;
	if(!task->taskHandle)
		return TRUE;

	status = _PREFIX_(StopTask(task->taskHandle));
	errmsg = facq_nidaq_check_error(status);
	if(errmsg){
		if(err != NULL)
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
		g_free(errmsg);
		return FALSE;
	}
	return TRUE;
}

/**
 * facq_nidaq_task_setup_timing:
 * @task: A #FacqNIDAQTask object.
 * @period: The desired sampling period in seconds.
 * @samps_per_chan: The expected number of samples per physical channel that will be read or
 * written.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * Configures the task, setting up the sampling period and the expected number
 * of samples per physical channel.
 *
 * Returns: %TRUE if successful, %FALSE in other case.
 */
gboolean facq_nidaq_task_setup_timing(FacqNIDAQTask *task,gdouble period,guint32 samps_per_chan,GError **err)
{
	gint32 status = 0;
	gchar *errmsg = NULL;
	gdouble rate = 0;

	g_return_val_if_fail(task && task->taskHandle 
				&& period > 0 && samps_per_chan > 0,FALSE);

	rate = 1/period;

#if ENABLE_DEBUG
	facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
			 "setup timming, period=%.9f rate=%.9f samples per chan=%u",
			 period,rate,samps_per_chan);
#endif
	status = _PREFIX_(CfgSampClkTiming(task->taskHandle,"OnboardClock",rate,
				DAQmx_Val_Rising,DAQmx_Val_ContSamps,samps_per_chan));

	errmsg = facq_nidaq_check_error(status);
	if(errmsg){
		if(err != NULL)
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
		g_free(errmsg);
		return FALSE;
	}
	return TRUE;
}

/**
 * facq_nidaq_task_setup_input_buffer:
 * @task: A #FacqNIDAQTask structure.
 * @samps_per_chan: The number of samples per channel.
 *
 * Allocates a NIDAQ buffer of @samps_per_chan per number of physical 
 * channels size.
 * For example if you use 2 channels and this value is 1e6 the function 
 * will allocate a buffer of 2e6 samples.
 *
 * Returns: %TRUE if successful, %FALSE in case of error.
 */
gboolean facq_nidaq_task_setup_input_buffer(FacqNIDAQTask *task,guint32 samps_per_chan,GError **err)
{
	gint32 status = 0;
	gchar *errmsg = NULL;

	g_return_val_if_fail(task,FALSE);

#if ENABLE_DEBUG
	facq_log_write_v(FACQ_LOG_MSG_TYPE_INFO,
			 "Setting NIDAQ buffer to %u samples per channel",
			 samps_per_chan);
#endif

	status = _PREFIX_(CfgInputBuffer(task->taskHandle,samps_per_chan));
	errmsg = facq_nidaq_check_error(status);
	if(errmsg){
		if(err != NULL)
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
		g_free(errmsg);
		return FALSE;
	}
	return TRUE;
}

/**
 * facq_nidaq_task_get_read_avail_samples_per_chan:
 * @task: A #FacqNIDAQTask structure.
 * @err: A @GError, it will be set in case of error, if not %NULL.
 *
 * Gets the number of available samples ready to be read per channel 
 * in the NIDAQ buffer. All the channels will have the same number of available
 * samples. This function can be used for polling checking the returned value and
 * sleeping after the check with for example g_usleep().
 *
 * Returns: The number of available samples per channel in the buffer.
 */
guint32 facq_nidaq_task_get_read_avail_samples_per_chan(FacqNIDAQTask *task,GError **err)
{
	guint32 ret = 0;
	gint32 status = 0;
	gchar *errmsg = NULL;

#if USE_NIDAQMX
	status = DAQmxGetReadAvailSampPerChan(task->taskHandle,(uInt32 *)&ret);
#elif USE_NIDAQMXBASE
	status = DAQmxBaseGetReadAttribute(task->taskHandle,DAQmx_Read_AvailSampPerChan,(void *)&ret);
#endif
	if(status < 0){
		errmsg = facq_nidaq_check_error(status);
		if(err != NULL){
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
			g_free(errmsg);
		}
		ret = 0;
	}

	return ret;
}

gchar *facq_nidaq_task_get_read_wait_mode(FacqNIDAQTask *task,gint32 *mode,GError **err)
{
	gchar *ret = NULL;
	gint32 status = 0, retmode = 0;
	gchar *errmsg = NULL;

#if USE_NIDAQMX
	status = DAQmxGetReadWaitMode(task->taskHandle,(int32 *)&retmode);
#elif USE_NIDAQMXBASE
	//status = DAQmxBaseGetReadAttribute(task->taskHandle,DAQmx_ReadWaitMode,(int32 *)&retmode);
#endif
	if(status < 0){
		errmsg = facq_nidaq_check_error(status);
		if(err != NULL){
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
			g_free(errmsg);
		}
		return NULL;
	}
	switch(retmode){
#if USE_NIDAQMX
	case DAQmx_Val_WaitForInterrupt:
		ret = g_strdup("Wait For Interrupt Read Wait Mode");
	break;
	case DAQmx_Val_Poll:
		ret = g_strdup("Poll Read Wait Mode");
	break;
	case DAQmx_Val_Yield:
		ret = g_strdup("Yield Read Wait Mode");
	break;
	case DAQmx_Val_Sleep:
		ret = g_strdup("Sleep Read Wait Mode");
	break;
#endif
	default:
		ret = g_strdup("Unknown Read Wait Mode");
	}
	if(mode)
		*mode = retmode;
	return ret;
}

gdouble facq_nidaq_task_get_read_sleep_time(FacqNIDAQTask *task,GError **err)
{
	gint32 status = 0;
	gchar *errmsg = NULL;
	gdouble sleep_seconds = 0.0;

#if USE_NIDAQMX
	status = DAQmxGetReadSleepTime(task->taskHandle,(float64 *)&sleep_seconds);
#endif
	if(status < 0){
		errmsg = facq_nidaq_check_error(status);
		if(err != NULL){
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
			g_free(errmsg);
		}
	}
	return sleep_seconds;
}

gboolean facq_nidaq_task_get_read_all_avail_samples(FacqNIDAQTask *task,GError **err)
{
	gint32 status = 0;
	gchar *errmsg = NULL;
	gboolean ret = FALSE;

#if USE_NIDAQMX
	status = DAQmxGetReadReadAllAvailSamp(task->taskHandle,(bool32 *)&ret);
#elif USE_NIDAQMXBASE
	status = DAQmxBaseGetReadAttribute(task->taskHandle,DAQmx_Read_ReadAllAvailSamp,(bool32 *)&ret);
#endif
	if(status < 0){
		errmsg = facq_nidaq_check_error(status);
		if(err != NULL){
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
			g_free(errmsg);
		}
	}
	return ret;
}

void facq_nidaq_task_set_read_all_avail_samples(FacqNIDAQTask *task,gboolean value,GError **err)
{
	gint32 status = 0;
	gchar *errmsg = NULL;

#if USE_NIDAQMX
	status = DAQmxSetReadReadAllAvailSamp(task->taskHandle,(bool32)value);
#elif USE_NIDAQMXBASE
	//How can we set this value in NIDAQmxBase?
#endif
	if(status < 0){
		errmsg = facq_nidaq_check_error(status);
		if(err != NULL){
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
			g_free(errmsg);
		}
	}
}

gchar *facq_nidaq_task_get_xfer_mode(FacqNIDAQTask *task,GError **err)
{
	gchar *ret = NULL;
	gint32 status = 0, xfermode = 0;
	gchar *errmsg = NULL;

#if USE_NIDAQMX
	status = DAQmxGetAIDataXferMech(task->taskHandle,task->phys_channel,(int32 *)&xfermode);
#elif USE_NIDAQMXBASE
	status = DAQmxBaseGetChanAttribute(task->taskHandle,task->phys_channel,DAQmx_AI_DataXferMech,(void *)&xfermode);
#endif
	if(status < 0){
		errmsg = facq_nidaq_check_error(status);
		if(err != NULL){
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
			g_free(errmsg);
		}
		return NULL;
	}
	switch(xfermode){
	case DAQmx_Val_DMA:
		ret = g_strdup("DMA transfer mode");
	break;
	case DAQmx_Val_Interrupts:
		ret = g_strdup("Interrupt transfer mode");
	break;
	case DAQmx_Val_ProgrammedIO:
		ret = g_strdup("PIO transfer mode");
	break;
#if USE_NIDAQMX
	/* note that DAQmx_Val_USBbulk is not defined on base */
	case DAQmx_Val_USBbulk:
		ret = g_strdup("USB (Bulk frames) transfer mode");
	break;
#endif
	default:
		ret = g_strdup("Unknown transfer mode");
	}

	return ret;
}

/**
 * facq_nidaq_task_set_ai_data_xfer_req_cond:
 * @task: A #FacqNIDAQTask structure.
 * @value: Valid values are, DAQmx_Val_OnBrdMemMoreThanHalfFull,
 * DAQmx_Val_OnBrdMemNotEmpty, DAQmx_Val_OnbrdMemCustomThreshold,
 * DAQmx_Val_WhenAcqComplete, the last two are only available on NIDAQmx
 * but not on NIDAQmxBase.
 * @custhreshold: Number of samples per channel that will determine when the
 * data will be transfered from the onboard memory to the buffer, this value is
 * needed when you use DAQmx_Val_OnbrdMemCustomThreshold with NIDAQmx ,
 * set to any other value in other case it will be ignored.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Sets the DAQmx_AI_DataXferReqCond condition. @value will set the way that
 * NIDAQ software will use to transfer the data from the onboard memory to the NIDAQ
 * buffer. DAQmx_Val_OnBrdMemNotEmpty is the default value used by NIDAQ, and
 * means that the data should be transfered as soon as possible,
 * DAQmx_Val_OnBrdMemMoreThanHalfFull will transfer the data when the onboard
 * memory is half full. DAQmx_Val_OnbrdMemCustomThreshold allows to set a custom
 * number of samples per channel, when this number of samples is written on the
 * onboard memory the data is transfered to the NIDAQ buffer.
 * DAQmx_Val_WhenAcqComplete transfers the data when the acquisition is
 * complete.
 *
 */
void facq_nidaq_task_set_ai_data_xfer_req_cond(FacqNIDAQTask *task,gint32 value,guint32 custhreshold,GError **err)
{
	gint32 status = 0;
	gchar *errmsg = NULL;

	g_return_if_fail(task);

#if USE_NIDAQMX
	status = DAQmxSetAIDataXferReqCond(task->taskHandle,
					   task->phys_channel,
					   (int32)value);
#elif USE_NIDAQMXBASE
	status = DAQmxBaseSetChanAttribute(task->taskHandle,
					   task->phys_channel,
					   DAQmx_AI_DataXferReqCond,
					   (int32)value);
#endif

	if(status < 0){
		errmsg = facq_nidaq_check_error(status);
		if(err != NULL){
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
			g_free(errmsg);
		}
		return;
	}

#if USE_NIDAQMX
	if(value == DAQmx_Val_OnbrdMemCustomThreshold){
		status = DAQmxSetAIDataXferCustomThreshold(task->taskHandle,
						  task->phys_channel,
						  (uInt32)custhreshold);
		if(status < 0){
			errmsg = facq_nidaq_check_error(status);
			if(err != NULL){
				g_set_error_literal(err,FACQ_NIDAQ_ERROR,
							FACQ_NIDAQ_ERROR_FAILED,errmsg);
				g_free(errmsg);
			}
		}
	}
#endif
}

gchar *facq_nidaq_task_get_ai_data_xfer_req_cond(FacqNIDAQTask *task,GError **err)
{
	gchar *ret = NULL;
	gint32 status = 0, reqcond = 0;
	gchar *errmsg = NULL;
	guint32 custhreshold = 0;

#if USE_NIDAQMX
	status = DAQmxGetAIDataXferReqCond(task->taskHandle,task->phys_channel,(int32 *)&reqcond);
#elif USE_NIDAQMXBASE
	status = DAQmxBaseGetChanAttribute(task->taskHandle,task->phys_channel,DAQmx_AI_DataXferReqCond,(void *)&reqcond);
#endif
	if(status < 0){
		errmsg = facq_nidaq_check_error(status);
		if(err != NULL){
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
			g_free(errmsg);
		}
		return NULL;
	}
	switch(reqcond){
	case DAQmx_Val_OnBrdMemMoreThanHalfFull:
		ret = g_strdup_printf("Requesting data when more than half of the onboard memory is filled");
	break;
	case DAQmx_Val_OnBrdMemNotEmpty:
		ret = g_strdup_printf("Requesting data when available in the onboard memory");
	break;
#if USE_NIDAQMX
	/* DAQmx_Val_OnbrdmemCustomThreshold and DAQmx_Val_WhenAcqComplete not
	 * defined on base */
	case DAQmx_Val_OnbrdMemCustomThreshold:
		/* first get the custom threshold value */
		status = DAQmxGetAIDataXferCustomThreshold(task->taskHandle,
							   task->phys_channel,
							   (uInt32 *)&custhreshold);

		if(status < 0){
			errmsg = facq_nidaq_check_error(status);
			if(err != NULL){
				g_set_error_literal(err,FACQ_NIDAQ_ERROR,
							FACQ_NIDAQ_ERROR_FAILED,errmsg);
				g_free(errmsg);
			}
			return NULL;
		}
		ret = g_strdup_printf("Requesting data when device FIFO has %u samples",custhreshold);
	break;
	case DAQmx_Val_WhenAcqComplete:
		ret = g_strdup("Requesting data when acquisition is complete");
	break;
#endif
	default:
		ret = g_strdup("Unknown request condition");
	}

	return ret;
}

/**
 * facq_nidaq_task_get_onboard_buffer_size:
 * @task: A #FacqNIDAQTask structure.
 * @err: A #GError it will be set in case of error if not %NULL.
 *
 * Gets the number of samples per channel that will be stored on the onboard
 * memory of the DAQ card. This functionality is only available when using
 * NIDAQmx, else this function will return always 0, because NIDAQmxBase
 * doesn't provide a way to read this property.
 *
 * Returns: The number of samples per channel that will be stored on the onboard
 * memory of the DAQ card.
 */
guint32 facq_nidaq_task_get_onboard_buffer_size(FacqNIDAQTask *task,GError **err)
{
	guint32 ret = 0;
	gint32 status = 0;
	gchar *errmsg = NULL;

#if USE_NIDAQMX
	status = DAQmxGetBufInputOnbrdBufSize(task->taskHandle,(uInt32 *)&ret);
#elif USE_NIDAQMXBASE
	//TODO How can we get the value on NIDAQmxBase?
#endif
	if(status < 0){
		errmsg = facq_nidaq_check_error(status);
		if(err != NULL){
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
			g_free(errmsg);
		}
		return 0;
	}
	return ret;
}

/**
 * facq_nidaq_task_write:
 * @task: A #FacqNIDAQTask structure.
 * @buffer: A real buffer containing the samples in interleaved format.
 * @samps_per_chan: The number of samples per physical channel.
 * @timeout: The timeout for the function in seconds.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Writes the real samples in @buffer to the NIDAQ device and channels
 * configured in the #FacqNIDAQTask, @task. The function will take at 
 * most @timeout seconds to complete, if the timeout elapses the function 
 * will return an error.
 *
 * Returns: The number of written samples per channel or -1 in case of error.
 */
gssize facq_nidaq_task_write(FacqNIDAQTask *task,gdouble *buffer,gint32 samps_per_chan,gdouble timeout,GError **err)
{
	gint32 ret = 0;
	gint32 status = 0;
	gchar *errmsg = NULL;
	gboolean autostart = FALSE;

	g_return_val_if_fail(task && task->taskHandle &&
					buffer && samps_per_chan > 0 &&
							timeout > 0,-1);

	status = _PREFIX_(
			WriteAnalogF64(task->taskHandle,
					samps_per_chan,(bool32)autostart,
						timeout,DAQmx_Val_GroupByScanNumber,
							buffer,(int32 *)&ret,NULL) );
	
	errmsg = facq_nidaq_check_error(status);
	if(errmsg){
		if(err != NULL)
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
		g_free(errmsg);
		return -1;
	}
	return (gssize)ret;
}

/**
 * facq_nidaq_task_read:
 * @task: A #FacqNIDAQTask structure.
 * @buffer: A real data array.
 * @samples: The size in samples of the buffer.
 * @samps_per_chan: The number of samples you expect to read per physical
 * channel.
 * @timeout: Timeout for the function in seconds.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Read @samps_per_chan samples per physical channel, and places the resulting
 * samples in interleaved format in @buffer which length is given by @samples. 
 * The function will take at most @timeout seconds to complete, if the timeout 
 * elapses the function will return an error.
 *
 * Returns: The number of read samples per channel or -1 in case of error.
 */
gssize facq_nidaq_task_read(FacqNIDAQTask *task,gdouble *buffer,guint32 samples,gint32 samps_per_chan,gdouble timeout,GError **err)
{
	gint32 ret = 0;
	gint32 status = 0;
	gchar *errmsg = NULL;

	g_return_val_if_fail(task && task->taskHandle 
					&& buffer && samples > 0 
						&& samps_per_chan > 0 
							&& timeout > 0,-1);
#if ENABLE_DEBUG
	facq_log_write_v(FACQ_LOG_MSG_TYPE_DEBUG,
			 "samples per channel=%d timeout=%.9f buffer=%p samples=%u",
			 samps_per_chan,timeout,buffer,samples);
#endif

	status = _PREFIX_(
			ReadAnalogF64(task->taskHandle,samps_per_chan,
				timeout,DAQmx_Val_GroupByScanNumber,
					buffer,samples,
						(int32 *)&ret,NULL) );
	
	errmsg = facq_nidaq_check_error(status);
	if(errmsg){
		if(err != NULL)
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
		g_free(errmsg);
		return -1;
	}
	return (gssize)ret;
}

/**
 * facq_nidaq_task_free:
 * @task: A #FacqNIDAQTask struct.
 * 
 * Destroys a given FacqNIDAQTask structure. You should stop the task calling
 * facq_nidaq_task_stop before freeing it.
 */
void facq_nidaq_task_free(FacqNIDAQTask *task)
{
	if(task && task->taskHandle){
		_PREFIX_(ClearTask(task->taskHandle));
	}
	if(task)
		g_free(task);
}

/**
 * facq_nidaq_device_serial_get:
 * @dev: The device name.
 * @err: (allow-none): A #GError, it will be set in case of error if not %NULL.
 *
 * Returns the serial number of the DAQ card identified by @dev. You should
 * check @err, because this function can return 0 for the serial number in a
 * successful call.
 *
 * Returns: An unsigned 32 bits integer that contains the serial number.
 */
guint32 facq_nidaq_device_serial_get(const gchar *dev,GError **err)
{
	guint32 serial = 0;
	gint32 status = 0;
	gchar *errmsg = NULL;

	status = _PREFIX_(GetDevSerialNum(dev,(uInt32 *)&serial));
	if(status < 0){
		errmsg = facq_nidaq_check_error(status);
		if(err != NULL){
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
			g_free(errmsg);
		}
	}
	return serial;
}

/**
 * facq_nidaq_device_reset:
 * @dev: The device name, for example Dev1.
 * @err: A #GError it will be set in case of error if not %NULL.
 *
 * Aborts all task associated with a device, returning the device to an
 * initialized state.
 */
void facq_nidaq_device_reset(const gchar *dev,GError **err)
{
	gint32 status = 0;
	gchar *errmsg = NULL;

	status = _PREFIX_(ResetDevice(dev));
	if(status < 0){
		errmsg = facq_nidaq_check_error(status);
		if(err != NULL){
			g_set_error_literal(err,FACQ_NIDAQ_ERROR,
						FACQ_NIDAQ_ERROR_FAILED,errmsg);
			g_free(errmsg);
		}
	}
}

#endif //USE_NIDAQ

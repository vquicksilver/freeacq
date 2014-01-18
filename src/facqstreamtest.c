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
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqchunk.h"
#include "facqbuffer.h"
#include "facqstreamdata.h"
#include "facqfile.h"
#include "facqsource.h"
#include "facqoperation.h"
#include "facqoperationplug.h"
#include "facqoperationlist.h"
#include "facqsink.h"
#include "facqpipelinemessage.h"
#include "facqpipelinemonitor.h"
#include "facqpipeline.h"
#include "facqsourcesoft.h"
#include "facqsinkfile.h"
#include "facqcatalog.h"
#include "facqstream.h"
#if USE_COMEDI
#include "facqsourcecomedisync.h"
#include "facqsourcecomediasync.h"
#endif
#if USE_NIDAQ
#include "facqnidaq.h"
#include "facqsourcenidaq.h"
#include "facqsinknidaq.h"
#endif

static void error_callback(FacqPipelineMessage *msg,gpointer data)
{
	g_print("On error callback\n");
}

static void stop_callback(FacqPipelineMessage *msg,gpointer data)
{
        g_print("On stop callback\n");
}

int main(int argc,char **argv)
{
	FacqSource *src = NULL;
	FacqSink *sink = NULL;
	FacqStream *stream = NULL;
	GError *err = NULL;
	FacqChanlist *chanlist = NULL;
	FacqOperation *plug = NULL;        

#if GLIB_MINOR_VERSION < 36
        g_type_init();
#endif
#if GLIB_MINOR_VERSION < 32
	g_thread_init(NULL);
#endif

	facq_log_enable();
	facq_log_set_mask(FACQ_LOG_MSG_TYPE_DEBUG);
	facq_log_toggle_out(FACQ_LOG_OUT_STDOUT,NULL);

	src = FACQ_SOURCE(facq_source_soft_new(FACQ_FUNC_TYPE_COS,5,10,0.01,3,&err));

	//src = FACQ_SOURCE(facq_source_comedi_sync_new(0,0,0,500000000,0,0,&err));
	//chanlist = facq_chanlist_new();
	//facq_chanlist_add_chan(chanlist,0,0,AREF_GROUND,0,CHAN_INPUT);
	//facq_chanlist_add_chan(chanlist,1,0,0,0,CHAN_INPUT);
	//facq_chanlist_add_chan(chanlist,2,0,0,0,CHAN_INPUT);
	//src = FACQ_SOURCE(facq_source_comedi_async_new(0,0,0,chanlist,0.01,&err));
	//src = FACQ_SOURCE(facq_source_nidaq_new("Dev1",chanlist,200000,0.000004000,10.0,-10.0,&err));

	if(!src || err)
		goto error;
	sink = FACQ_SINK(facq_sink_file_new("test.baf",&err));
	//chanlist = facq_chanlist_new();
	//facq_chanlist_add_chan(chanlist,0,0,0,0,CHAN_OUTPUT);
	//sink = FACQ_SINK(facq_sink_nidaq_new("Dev1",chanlist,10,-10,&err));
	if(!sink || err)
		goto error;
	
	//plug = FACQ_OPERATION(facq_operation_plug_new("127.0.0.1",3000));

	stream = facq_stream_new("New Stream",32,stop_callback,error_callback,NULL);
	facq_stream_set_source(stream,src);
	//facq_stream_append_operation(stream,plug);
	facq_stream_set_sink(stream,sink);

	if(!facq_stream_start(stream,&err)){
		goto error;
	}
	g_usleep(10*G_USEC_PER_SEC);
	facq_stream_stop(stream);
	facq_stream_free(stream);
	facq_file_to_human("test.baf","test.baf.txt",&err);
	if(err)
		goto error;
	facq_log_disable();
	return 0;

	error:
	if(err)
		facq_log_write(err->message,FACQ_LOG_MSG_TYPE_DEBUG);
	facq_log_disable();
	return 1;
}

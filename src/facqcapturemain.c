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
#include <gtk/gtk.h>
#include <stdlib.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqi18n.h"
#include "facqlog.h"
#include "facqglibcompat.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqresources.h"
#include "facqresourcesicons.h"
#include "facqcapture.h"
#include "facqsource.h"
#include "facqsourcesoft.h"
#if USE_COMEDI
#include "facqsourcecomediasync.h"
#include "facqsourcecomedisync.h"
#endif
#ifdef USE_NIDAQ
#include "facqnidaq.h"
#include "facqsourcenidaq.h"
#endif
#include "facqchunk.h"
#include "facqoperation.h"
#include "facqoperationplug.h"
#include "facqsink.h"
#include "facqsinkfile.h"
#include "facqsinknull.h"
#ifdef USE_NIDAQ
#include "facqsinknidaq.h"
#endif

int main(int argc,char **argv)
{
	FacqCapture *cap = NULL;
	FacqCatalog *cat = NULL;

#if ENABLE_NLS
	setlocale(LC_ALL,"");
	bindtextdomain(PACKAGE,LOCALEDIR);
	bind_textdomain_codeset(PACKAGE,"UTF-8");
        textdomain(PACKAGE);
#endif

#if GLIB_MINOR_VERSION < 32
        g_thread_init(NULL);
#endif
	
	gtk_init(&argc,&argv);

	facq_log_enable();

#if ENABLE_DEBUG
	facq_log_set_mask(FACQ_LOG_MSG_TYPE_DEBUG);
#else
	facq_log_set_mask(FACQ_LOG_MSG_TYPE_INFO);
#endif
	facq_log_toggle_out(FACQ_LOG_OUT_FILE,NULL);

	cat = facq_catalog_new();

	/* fill the catalog with the supported components */

	/* Sources */
	facq_catalog_append_source(cat,
				   facq_resources_names_source_soft(),
				   facq_resources_descs_source_soft(),
				   "FUNCTION,Function:/"
				   "DOUBLE,Amplitude:,10,1,5,0.5,2/"
				   "DOUBLE,Wave period:,4294967295,0.001,1,1,3/"
				   "DOUBLE,Period:,4294967295,0.001,1,1,3/"
				   "UINT,Channels:,255,1,1,1",
				   facq_resources_icons_source_soft(),
				   facq_source_soft_constructor,
				   facq_source_soft_key_constructor);
#if USE_COMEDI
	facq_catalog_append_source(cat,
				   facq_resources_names_source_comedi_sync(),
				   facq_resources_descs_source_comedi_sync(),
				   "UINT,Device:,255,0,0,1/"
				   "UINT,Subdevice:,255,0,0,1/"
				   "DOUBLE,Period:,4294967295,1e-3,1,1,3/"
				   "CHANLIST,1,1,1,0",
				   facq_resources_icons_source_comedi_sync(),
				   facq_source_comedi_sync_constructor,
				   facq_source_comedi_sync_key_constructor);

	facq_catalog_append_source(cat,
				   facq_resources_names_source_comedi_async(),
				   facq_resources_descs_source_comedi_async(),
				   "UINT,Device:,255,0,0,1/"
				   "UINT,Subdevice:,255,0,0,1/"
				   "UINT,Flags:,4294967295,0,0,1/"
				   "DOUBLE,Period:,4.294967295,1e-9,1,1,9/"
				   "CHANLIST,1,1,256,0"
				   ,facq_resources_icons_source_comedi_async(),
				   facq_source_comedi_async_constructor,
				   facq_source_comedi_async_key_constructor);
#endif

#ifdef USE_NIDAQ
	facq_catalog_append_source(cat,
				   facq_resources_names_source_nidaq(),
				   facq_resources_descs_source_nidaq(),
				   "STRING,Device:,Dev1/"
				   "UINT,Buffer size (samps per chan):,4294967295,1,1e6,1/"
				   "DOUBLE,Period:,4294967295,1e-9,1,1,9/"
				   "DOUBLE,Max:,100,-100,5,0.5,3/"
				   "DOUBLE,Min:,100,-100,0,0.5,3/"
				   "UINT,Poll interval (microseconds):,4294967295,0,0,1/"
				   "CHANLIST,1,0,256,1",
				   facq_resources_icons_source_nidaq(),
				   facq_source_nidaq_constructor,
				   facq_source_nidaq_key_constructor);
#endif

	/* Operations */

	facq_catalog_append_operation(cat,
				      facq_resources_names_operation_plug(),
				      facq_resources_descs_operation_plug(),
				      "STRING,""Address:"",127.0.0.1/"
				      "UINT,""Port:"",65535,0,3000,1",
				      facq_resources_icons_operation_plug(),
				      facq_operation_plug_constructor,
				      facq_operation_plug_key_constructor);

	/* Sinks */

	facq_catalog_append_sink(cat,
				 facq_resources_names_sink_file(),
				 facq_resources_descs_sink_file(),
				 "FILENAME,0,baf,""Binary Acquisition File",
				 facq_resources_icons_sink_file(),
				 facq_sink_file_constructor,
				 facq_sink_file_key_constructor);
	
	facq_catalog_append_sink(cat,
				 facq_resources_names_sink_null(),
				 facq_resources_descs_sink_null(),
				 "NOPARAMETERS",
				 facq_resources_icons_sink_null(),
				 facq_sink_null_constructor,
				 facq_sink_null_key_constructor);

#ifdef USE_NIDAQ
	facq_catalog_append_sink(cat,
				 facq_resources_names_sink_nidaq(),
				 facq_resources_descs_sink_nidaq(),
				 "STRING,""Device:"",Dev1/"
				 "DOUBLE,Max:,100,-100,5,0.5/"
				 "DOUBLE,Min:,100,-100,0,0.5/"
				 "CHANLIST,0,0,256,0",
				 facq_resources_icons_sink_nidaq(),
				 facq_sink_nidaq_constructor,
				 facq_sink_nidaq_key_constructor);
#endif

	cap = facq_capture_new(cat);

	gtk_main();

	facq_capture_free(cap);
	facq_catalog_free(cat);
	facq_log_disable();

	return EXIT_SUCCESS;
}

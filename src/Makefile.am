if ENABLE_NLS
NLS_FLAGS = -DLOCALEDIR=\"$(localedir)\"
else
NLS_FLAGS = 
endif

noinst_LIBRARIES = libfreeacq.a

bin_PROGRAMS = facqstreamtest

if ENABLE_GTK
bin_PROGRAMS += facqcapture facqplethysmograph
noinst_LIBRARIES += libfacqcapture.a libfacqplethysmograph.a
noinst_bindir = $(top_builddir)/tests
noinst_bin_PROGRAMS = facqstreamtest
endif

if ENABLE_GTKDATABOX
bin_PROGRAMS += facqoscilloscope facqviewer
noinst_LIBRARIES += libfacqoscope.a libfacqbafview.a
endif

if TARGET_WIN32
.rc.o:
	windres -o $@ $<

if ENABLE_DEBUG
WINDOWS_GRAPHAPPFLAGS =
else
WINDOWS_GRAPHAPPFLAGS = -mwindows
endif

EXTRA_facqoscilloscope_SOURCES = ../mingw/oscope.rc
EXTRA_facqcapture_SOURCES = ../mingw/capture.rc
EXTRA_facqviewer_SOURCES = ../mingw/viewer.rc
EXTRA_facqplethysmograph_SOURCES = ../mingw/plethysmograph.rc
endif

LIBFREEACQ_SOURCES = \
	facqlog.c \
	facqlog.h \
	gdouble.c \
	gdouble.h \
	facqnet.c \
	facqnet.h \
	facqcatalog.c \
	facqcatalog.h \
	facqresources.c \
	facqresources.h \
	facqcomplex.c \
	facqcomplex.h \
	facqfft.c \
	facqfft.h \
	facqwindowfun.c \
	facqwindowfun.h \
	facqpid.c \
	facqpid.h \
	facqglibcompat.c \
	facqglibcompat.h \
	facqunits.h \
	facqunits.c \
	facqchanlist.h \
	facqchanlist.c \
	facqbuffer.c \
	facqbuffer.h \
	facqchunk.h \
	facqchunk.c \
	facqstreamdata.h \
	facqstreamdata.c \
	facqfile.h \
	facqfile.c \
	facqsource.h \
	facqsource.c \
	facqoperation.h \
	facqoperation.c \
	facqoperationlist.h \
	facqoperationlist.c \
	facqoperationplug.h \
	facqoperationplug.c \
	facqsink.h \
	facqsink.c \
	facqpipelinemessage.h \
	facqpipelinemessage.c \
	facqpipelinemonitor.h \
	facqpipelinemonitor.c \
	facqpipeline.h \
	facqpipeline.c \
	facqmisc.h \
	facqmisc.c \
	facqstream.h \
	facqstream.c \
	facqsourcesoft.h \
	facqsourcesoft.c \
	facqsinknull.h \
	facqsinknull.c \
	facqsinkfile.h \
	facqsinkfile.c

if ENABLE_COMEDI
COMEDI_SOURCES = \
	facqcomedimisc.c \
	facqcomedimisc.h \
	facqsourcecomedisync.c \
	facqsourcecomedisync.h \
	facqsourcecomediasync.c \
	facqsourcecomediasync.h
else
COMEDI_SOURCES = \
	facqnocomedi.h
endif

if ENABLE_NIDAQMX
NIDAQ_SOURCES = \
	facqnidaq.c \
	facqnidaq.h \
	facqsourcenidaq.c \
	facqsourcenidaq.h \
	facqsinknidaq.c \
	facqsinknidaq.h
NIDAQ_CFLAGS = $(NIDAQMX_CFLAGS)
NIDAQ_LIBS = $(NIDAQMX_LIBS)
endif

if ENABLE_NIDAQMXBASE
NIDAQ_SOURCES = \
	facqnidaq.c \
	facqnidaq.h \
	facqsourcenidaq.c \
	facqsourcenidaq.h \
	facqsinknidaq.c \
	facqsinknidaq.h
NIDAQ_CFLAGS = $(NIDAQMXBASE_CFLAGS)
NIDAQ_LIBS = $(NIDAQMXBASE_LIBS)
endif

libfreeacq_a_SOURCES = $(LIBFREEACQ_SOURCES) $(COMEDI_SOURCES) $(NIDAQ_SOURCES)
libfreeacq_a_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	$(COMEDI_CFLAGS) \
	$(NIDAQ_CFLAGS) \
	$(FFTW3_CFLAGS)

if ENABLE_GTK
libfacqoscope_a_SOURCES = \
	facqi18n.h \
	facqglibcompat.c \
	facqglibcompat.h \
	facqbuffer.c \
	facqbuffer.h \
	facqchunk.c \
	facqchunk.h \
	facqmisc.c \
	facqmisc.h \
	facqoscope.c \
	facqoscope.h \
	facqresourcesicons.c \
	facqresourcesicons.h \
	facqoscopetoolbarcallbacks.h \
	facqoscopetoolbarcallbacks.c \
	facqoscopetoolbar.c \
	facqoscopetoolbar.h \
	facqoscopemenu.h \
	facqoscopemenu.c \
	facqoscopemenucallbacks.h \
	facqoscopemenucallbacks.c \
	facqoscopeplot.h \
	facqoscopeplot.c \
	facqstatusbar.h \
	facqstatusbar.c \
	facqlegend.c \
	facqlegend.h \
	facqcolor.h \
	facqcolor.c \
	facqplugdialog.h \
	facqplugdialog.c \
	facqplug.h \
	facqplug.c \
	facqnet.h \
	facqnet.c \
	facqstreamdata.h \
	facqstreamdata.c \
	facqchanlist.h \
	facqchanlist.c \
	facqunits.c \
	facqunits.h \
	facqlog.h \
	facqlog.c \
	gdouble.h \
	gdouble.c


libfacqoscope_a_CPPFLAGS = \
	$(GTK_CFLAGS) \
	$(GTKDATABOX_CFLAGS)

libfacqbafview_a_SOURCES = \
	facqi18n.h \
	facqglibcompat.h \
	facqglibcompat.c \
	facqlegend.h \
	facqlegend.c \
	facqcolor.h \
	facqcolor.c \
	facqlog.h \
	facqlog.c \
	facqnet.h \
	facqnet.c \
	gdouble.h \
	gdouble.c \
	facqunits.h \
	facqunits.c \
	facqchanlist.h \
	facqchanlist.c \
	facqstreamdata.h \
	facqstreamdata.c \
	facqchunk.h \
	facqchunk.c \
	facqfile.h \
	facqfile.c \
	facqbafview.h \
	facqbafview.c \
	facqbafviewmenucallbacks.h \
	facqbafviewmenucallbacks.c \
	facqbafviewmenu.h \
	facqbafviewmenu.c \
	facqbafviewtoolbarcallbacks.h \
	facqbafviewtoolbarcallbacks.c \
	facqbafviewtoolbar.h \
	facqbafviewtoolbar.c \
	facqfilechooser.h \
	facqfilechooser.c \
	facqbafviewplot.h \
	facqbafviewplot.c \
	facqbafviewdialog.h \
	facqbafviewdialog.c \
	facqstatusbar.h \
	facqstatusbar.c \
	facqresourcesicons.h \
	facqresourcesicons.c


libfacqbafview_a_CPPFLAGS = \
	$(GTK_CFLAGS) \
	$(GTKDATABOX_CFLAGS)

libfacqcapture_a_SOURCES = \
	facqi18n.h \
	facqglibcompat.h \
	facqglibcompat.c \
	facqlog.h \
	facqlog.c \
	gdouble.h \
	gdouble.c \
	facqnet.h \
	facqnet.c \
	facqunits.h \
	facqunits.c \
	facqchunk.h \
	facqchunk.c \
	facqbuffer.h \
	facqbuffer.c \
	facqchanlist.h \
	facqchanlist.c \
	facqstreamdata.h \
	facqstreamdata.c \
	facqsource.h \
	facqsource.c \
	facqoperation.h \
	facqoperation.c \
	facqoperationlist.h \
	facqoperationlist.c \
	facqsink.h \
	facqsink.c \
	facqpipelinemessage.h \
	facqpipelinemessage.c \
	facqpipelinemonitor.h \
	facqpipelinemonitor.c \
	facqpipeline.h \
	facqpipeline.c \
	facqmisc.h \
	facqmisc.c \
	facqstream.h \
	facqstream.c \
	facqcapturemenucallbacks.h \
	facqcapturemenucallbacks.c \
	facqcapturemenu.h \
	facqcapturemenu.c \
	facqcapturetoolbarcallbacks.h \
	facqcapturetoolbarcallbacks.c \
	facqcapturetoolbar.h \
	facqcapturetoolbar.c \
	facqstatusbar.h \
	facqstatusbar.c \
	facqstreamview.h \
	facqstreamview.c \
	facqstreamdialog.h \
	facqstreamdialog.c \
	facqdyndialog.h \
	facqdyndialog.c \
	facqlogwindow.h \
	facqlogwindow.c \
	facqcapture.h \
	facqcapture.c \
	facqresources.h \
	facqresources.c \
	facqresourcesicons.h \
	facqresourcesicons.c \
	facqcatalog.h \
	facqcatalog.c \
	facqcatalogdialog.h \
	facqcatalogdialog.c \
	facqchanlisteditor.h \
	facqchanlisteditor.c \
	facqsourcesoft.h \
	facqsourcesoft.c \
	facqoperationplug.h \
	facqoperationplug.c \
	facqfilechooser.h \
	facqfilechooser.c \
	facqfile.h \
	facqfile.c \
	facqsink.h \
	facqsink.c \
	facqsinkfile.h \
	facqsinkfile.c \
	facqsinknull.h \
	facqsinknull.c $(COMEDI_SOURCES) $(NIDAQ_SOURCES)

libfacqcapture_a_CPPFLAGS = \
	$(GTK_CFLAGS)       \
	$(COMEDI_CFLAGS)    \
	$(NIDAQ_CFLAGS)     \
	$(NLS_FLAGS)

libfacqplethysmograph_a_SOURCES = \
	facqi18n.h \
	facqplethysmograph.c \
	facqplethysmograph.h \
	facqplethysmographmenu.c \
	facqplethysmographmenu.h \
	facqplethysmographtoolbar.c \
	facqplethysmographtoolbar.h \
	facqplethysmographmenucallbacks.c \
	facqplethysmographmenucallbacks.h \
	facqplethysmographtoolbarcallbacks.c \
	facqplethysmographtoolbarcallbacks.h \
	facqcolor.c \
	facqcolor.h \
	facqdisplay.c \
	facqdisplay.h \
	facqdisplaymatrix.c \
	facqdisplaymatrix.h \
	facqbpm.c \
	facqbpm.h \
	facqstatusbar.c \
	facqstatusbar.h \
	facqglibcompat.c \
	facqglibcompat.h \
	facqresourcesicons.c \
	facqresourcesicons.h \
	facqplug.c \
	facqplug.h \
	facqplugdialog.c \
	facqplugdialog.h \
	facqlog.c \
	facqlog.h \
	facqbuffer.c \
	facqbuffer.h \
	facqchanlist.c \
	facqchanlist.h \
	facqchunk.c \
	facqchunk.h \
	facqmisc.c \
	facqmisc.h \
	facqnet.c \
	facqnet.h \
	facqstreamdata.c \
	facqstreamdata.h \
	facqunits.c \
	facqunits.h \
	gdouble.c \
	gdouble.h
	
libfacqplethysmograph_a_CPPFLAGS = \
	$(GTK_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(NLS_FLAGS)

facqoscilloscope_SOURCES = \
	facqoscopemain.c \
	$(EXTRA_facqoscilloscope_SOURCES)

facqoscilloscope_CPPFLAGS =  \
	$(GTK_CFLAGS)        \
	$(GTKDATABOX_CFLAGS) \
	$(GLIB_CFLAGS)       \
	$(NLS_FLAGS)
	
facqoscilloscope_LDFLAGS = $(WINDOWS_GRAPHAPPFLAGS)
facqoscilloscope_LDADD = \
	libfacqoscope.a \
	$(LIBINTL) \
	$(GLIB_LIBS) \
	$(GTK_LIBS) \
	$(GTKDATABOX_LIBS) \
	-lm

facqviewer_SOURCES = \
	facqviewermain.c \
	$(EXTRA_facqviewer_SOURCES)

facqviewer_CPPFLAGS = \
	$(GTK_CFLAGS) \
	$(GTKDATABOX_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(NLS_FLAGS)

facqviewer_LDFLAGS = $(WINDOWS_GRAPHAPPFLAGS)
facqviewer_LDADD = \
	libfacqbafview.a \
	$(LIBINTL) \
	$(GLIB_LIBS) \
	$(GTK_LIBS) \
	$(GTKDATABOX_LIBS) \
	-lm

facqcapture_SOURCES = \
	facqcapturemain.c \
	$(EXTRA_facqcapture_SOURCES)

facqcapture_CPPFLAGS =   \
	$(GTK_CFLAGS)    \
	$(GLIB_CFLAGS)   \
	$(COMEDI_CFLAGS) \
	$(NIDAQ_CFLAGS)  \
	$(NLS_FLAGS)
	
facqcapture_LDFLAGS = $(WINDOWS_GRAPHAPPFLAGS)
facqcapture_LDADD = \
	libfacqcapture.a \
	$(LIBINTL) \
	$(GLIB_LIBS) \
	$(GTK_LIBS) \
	$(COMEDI_LIBS) \
	$(NIDAQ_LIBS) \
	-lm

facqplethysmograph_SOURCES = \
	facqplethysmographmain.c \
	$(EXTRA_facqplethysmograph_SOURCES)

facqplethysmograph_CPPFLAGS =  \
	$(GTK_CFLAGS)        \
	$(GLIB_CFLAGS)       \
	$(NLS_FLAGS)
	
facqplethysmograph_LDFLAGS = $(WINDOWS_GRAPHAPPFLAGS)
facqplethysmograph_LDADD = \
	libfacqplethysmograph.a \
	$(LIBINTL) \
	$(GLIB_LIBS) \
	$(GTK_LIBS) \
	-lm
else

endif

facqstreamtest_SOURCES = facqstreamtest.c
facqstreamtest_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	$(COMEDI_CFLAGS) \
	$(NIDAQ_CFLAGS) \
	$(NLS_FLAGS)

facqstreamtest_LDADD = \
	libfreeacq.a \
	$(LIBINTL) \
	$(GLIB_LIBS) \
	$(COMEDI_LIBS) \
	$(NIDAQ_LIBS) \
	-lm


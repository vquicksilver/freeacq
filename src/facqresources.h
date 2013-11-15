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
#ifndef _FREEACQ_RESOURCES_H
#define _FREEACQ_RESOURCES_H
#if HAVE_CONFIG_H
#include <config.h>
#endif

G_BEGIN_DECLS

/* data sources */
const gchar *facq_resources_names_source_soft(void);
const gchar *facq_resources_descs_source_soft(void);

#if USE_COMEDI
const gchar *facq_resources_names_source_comedi_async(void);
const gchar *facq_resources_descs_source_comedi_async(void);

const gchar *facq_resources_names_source_comedi_sync(void);
const gchar *facq_resources_descs_source_comedi_sync(void);
#endif

#ifdef USE_NIDAQ
const gchar *facq_resources_names_source_nidaq(void);
const gchar *facq_resources_descs_source_nidaq(void);
#endif

/* operations */
const gchar *facq_resources_names_operation_plug(void);
const gchar *facq_resources_descs_operation_plug(void);

/* sinks */
const gchar *facq_resources_names_sink_null(void);
const gchar *facq_resources_descs_sink_null(void);

const gchar *facq_resources_names_sink_file(void);
const gchar *facq_resources_descs_sink_file(void);

#ifdef USE_NIDAQ
const gchar *facq_resources_names_sink_nidaq(void);
const gchar *facq_resources_descs_sink_nidaq(void);
#endif

G_END_DECLS

#endif

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
#include <glib.h>
#include "facqi18n.h"
#include "facqresources.h"

/**
 * SECTION:facqresources
 * @title:FacqResources
 * @short_description: Stores names and descriptions of different items.
 * @include:facqresources.h
 *
 * This module stores names and descriptions of different items that you can
 * add to the streams. Each item should add 2 functions to this module with the
 * following prototype:
 * <informalexample>
 * <programlisting>
 * const gchar *facq_resources_names_[source/operation/sink]_itemname(void);
 * const gchar *facq_resources_descs_[source/operation/sink]_itemname(void);
 * </programlisting>
 * </informalexample>
 * And you can use the following implementation, for example for a source:
 * <informalexample>
 * <programlisting>
 * const gchar *facq_resources_names_source_mysource(void)
 * {
 *	const gchar *name = "My Source";
 *	return name;
 * }
 *
 * const gchar *facq_resources_descs_source_mysource(void)
 * {
 *	const gchar *desc = N_("My very special Source");
 *	return desc;
 * }
 * </programlisting>
 * </informalexample>
 */

/**
 * facq_resources_names_source_soft:
 *
 * Gets the name for the Software source (#FacqSourceSoft).
 *
 * Returns: The name of the Software source, you shouldn't use g_free() on it.
 */
const gchar *facq_resources_names_source_soft(void)
{
	const gchar *name = "Software";
	return name;
}

/**
 * facq_resources_descs_source_soft:
 *
 * Gets the description for the Software source (#FacqSourceSoft).
 *
 * Returns: The description of the Software source, you shouldn't use g_free() on it.
 */
const gchar *facq_resources_descs_source_soft(void)
{
	const gchar *desc = N_("Software only data source");
	return desc;
}

#if USE_COMEDI
/**
 * facq_resources_names_source_comedi_async:
 *
 * Gets the name for the Async Comedi source (#FacqSourceComediAsync).
 *
 * Returns: The name of the Async Comedi source , you shouldn't use g_free() on it.
 */
const gchar *facq_resources_names_source_comedi_async(void)
{
	const gchar *name = "COMEDI-Async";
	return name;
}

/**
 * facq_resources_descs_source_comedi_async:
 *
 * Gets the description for the Async Comedi source (#FacqSourceComediAsync).
 *
 * Returns: The description of the Async Comedi source, you shouldn't use g_free() on it.
 */
const gchar *facq_resources_descs_source_comedi_async(void)
{
	const gchar *desc = N_("Requires comedi and an async subdevice");
	return desc;
}

/**
 * facq_resources_names_source_comedi_sync:
 *
 * Gets the name for the Sync Comedi source (#FacqSourceComediSync).
 *
 * Returns: The name of the Sync Comedi source, you shouldn't use g_free() on it.
 */
const gchar *facq_resources_names_source_comedi_sync(void)
{
	const gchar *name = "COMEDI-Sync";
	return name;
}

/**
 * facq_resources_descs_source_comedi_sync:
 *
 * Gets the description for the Sync Comedi source (#FacqSourceComediSync).
 *
 * Returns: The description of the Sync Comedi source, you shouldn't use g_free() on it.
 */
const gchar *facq_resources_descs_source_comedi_sync(void)
{
	const gchar *desc = N_("Requires comedi and a sync subdevice");
	return desc;
}
#endif

#ifdef USE_NIDAQ
/**
 * facq_resources_names_source_nidaq:
 *
 * Gets the name for the NIDAQ source (#FacqSourceNidaq).
 *
 * Returns: The name of the NIDAQ source, you shouldn't use g_free() on it.
 */
const gchar *facq_resources_names_source_nidaq(void)
{
	const gchar *name = "NIDAQReader";
	return name;
}

/**
 * facq_resources_descs_source_nidaq:
 *
 * Gets the description for the NIDAQ source (#FacqSourceNidaq).
 *
 * Returns: The description of the NIDAQ source, you shouldn't use g_free() on it.
 */
const gchar *facq_resources_descs_source_nidaq(void)
{
	const gchar *desc = N_("Reads data using NIDAQ");
	return desc;
}
#endif

/* operations */

/**
 * facq_resources_names_operation_plug:
 *
 * Gets the name for the Plug operation (#FacqOperationPlug).
 *
 * Returns: The name of the Plug operation, you shouldn't use g_free() on it.
 */
const gchar *facq_resources_names_operation_plug(void)
{
	const gchar *name = "Plug";
	return name;
}

/**
 * facq_resources_descs_operation_plug:
 *
 * Gets the description for the Plug operation (#FacqOperationPlug).
 *
 * Returns: The description of the Plug operation, you shouldn't use g_free() on it.
 */
const gchar *facq_resources_descs_operation_plug(void)
{
	const gchar *desc = N_("Plug for virtual instruments");
	return desc;
}

/* sinks */

/**
 * facq_resources_names_sink_null:
 *
 * Gets the name for the Null sink.
 *
 * Returns: The name of the Null sink, you shouldn't use g_free() on it.
 */
const gchar *facq_resources_names_sink_null(void)
{
	const gchar *name = "Null";
	return name;
}

/**
 * facq_resources_descs_sink_null:
 *
 * Gets the description for the Null sink (#FacqSinkNull).
 *
 * Returns: The description of the Null sink , you shouldn't use g_free() on it.
 */
const gchar *facq_resources_descs_sink_null(void)
{
	const gchar *name = N_("Nullifies input data");
	return name;
}

/**
 * facq_resources_names_sink_file:
 *
 * Gets the name for the File sink.
 *
 * Returns: The name of the File sink, you shouldn't use g_free() on it.
 */
const gchar *facq_resources_names_sink_file(void)
{
	const gchar *name = "File";
	return name;
}

/**
 * facq_resources_descs_sink_file:
 *
 * Gets the description for the File sink (#FacqSinkFile).
 *
 * Returns: The description of the File sink, you shouldn't use g_free() on it.
 */
const gchar *facq_resources_descs_sink_file(void)
{
	const gchar *desc = N_("Writes data to a binary acquisition file");
	return desc;
}

#ifdef USE_NIDAQ
/**
 * facq_resources_names_sink_nidaq:
 *
 * Gets the name for the NIDAQ sink.
 *
 * Returns: The name of the NIDAQ sink , you shouldn't use g_free() on it.
 */
const gchar *facq_resources_names_sink_nidaq(void)
{
	const gchar *name = "NIDAQWriter";
	return name;
}

/**
 * facq_resources_descs_sink_nidaq:
 *
 * Gets the description for the NIDAQ sink (#FacqSinkNidaq).
 *
 * Returns: The description of the NIDAQ sink, you shouldn't use g_free() on it.
 */
const gchar *facq_resources_descs_sink_nidaq(void)
{
	const gchar *desc = N_("Writes data using NIDAQ");
	return desc;
}

#endif

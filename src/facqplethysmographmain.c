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
#include <stdlib.h>
#include <gtk/gtk.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqi18n.h"
#include "facqlog.h"
#include "facqplethysmograph.h"

int main(int argc,char **argv)
{
	FacqPlethysmograph *plethysmograph = NULL;
	GError *local_err = NULL;

#if ENABLE_NLS
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
        facq_log_toggle_out(FACQ_LOG_OUT_STDOUT,NULL);

	plethysmograph = facq_plethysmograph_new("127.0.0.1",3001,&local_err);
	if(local_err){
		g_printerr("%s\n",local_err->message);
		g_clear_error(&local_err);
		return EXIT_FAILURE;
	}

	gtk_widget_show_all(facq_plethysmograph_get_widget(plethysmograph));

	gtk_main();

	facq_plethysmograph_free(plethysmograph);
	facq_log_disable();

	return EXIT_SUCCESS;
}

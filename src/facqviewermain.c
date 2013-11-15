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
#include <gtk/gtk.h>
#include <stdlib.h>
#include "facqi18n.h"
#include "facqlog.h"
#include "facqbafview.h"

int main(int argc,char **argv)
{
	FacqBAFView *view = NULL;

#if ENABLE_NLS
	bindtextdomain(PACKAGE,LOCALEDIR);
	bind_textdomain_codeset(PACKAGE,"UTF-8");
        textdomain(PACKAGE);
#endif

	gtk_init(&argc,&argv);

	facq_log_enable();
#if ENABLE_DEBUG
	facq_log_set_mask(FACQ_LOG_MSG_TYPE_DEBUG);
#else
	facq_log_set_mask(FACQ_LOG_MSG_TYPE_INFO);
#endif
	facq_log_toggle_out(FACQ_LOG_OUT_STDOUT,NULL);

	view = facq_baf_view_new(10.0);

	gtk_main();

	facq_baf_view_free(view);
	facq_log_disable();

	return EXIT_SUCCESS;
}

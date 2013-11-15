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
#ifndef _FREEACQ_PLUG_H_
#define _FREEACQ_PLUG_H_

#include "facqlog.h"
#include "gdouble.h"
#include "facqnet.h"
#include "facqunits.h"
#include "facqchanlist.h"
#include "facqstreamdata.h"
#include "facqglibcompat.h"
#include "facqchunk.h"
#include "facqbuffer.h"
#include "facqmisc.h"

G_BEGIN_DECLS

#define FACQ_PLUG_ERROR facq_plug_error_quark()

#define FACQ_TYPE_PLUG (facq_plug_get_type())
#define FACQ_PLUG(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_PLUG,FacqPlug))
#define FACQ_PLUG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_PLUG, FacqPlugClass))
#define FACQ_IS_PLUG(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_PLUG))
#define FACQ_IS_PLUG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_PLUG))
#define FACQ_PLUG_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_PLUG, FacqPlugClass))

typedef enum {
	FACQ_PLUG_ERROR_FAILED
} FacqPlugError;

typedef struct _FacqPlug FacqPlug;
typedef struct _FacqPlugClass FacqPlugClass;
typedef struct _FacqPlugPrivate FacqPlugPrivate;
typedef gboolean(*FacqPlugFunc)(FacqChunk *chunk,gpointer data);

struct _FacqPlug {
	/*< private >*/
	GObject parent_instance;
	FacqPlugPrivate *priv;
};

struct _FacqPlugClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_plug_get_type(void) G_GNUC_CONST;

FacqPlug *facq_plug_new(const gchar *address,guint16 port,FacqPlugFunc fun,gpointer fun_data,guint timeout_ms,GError **err);
gchar *facq_plug_get_client_address(FacqPlug *plug,GError **err);
gboolean facq_plug_set_listen_address(FacqPlug *plug,const gchar *address,guint16 port,GError **err);
gchar *facq_plug_get_address(const FacqPlug *plug);
guint16 facq_plug_get_port(const FacqPlug *plug);
void facq_plug_disconnect(FacqPlug *plug);
FacqStreamData *facq_plug_get_stream_data(FacqPlug *plug);
void facq_plug_free(FacqPlug *plug);

G_END_DECLS

#endif

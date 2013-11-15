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
#ifndef _FREEACQ_SOURCE_SOFT_H_
#define _FREEACQ_SOURCE_SOFT_H_

G_BEGIN_DECLS

#define FACQ_SOURCE_SOFT_ERROR facq_source_soft_error_quark()

#define FACQ_TYPE_SOURCE_SOFT (facq_source_soft_get_type())
#define FACQ_SOURCE_SOFT(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_SOURCE_SOFT,FacqSourceSoft))
#define FACQ_SOURCE_SOFT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_SOURCE_SOFT, FacqSourceSoftClass))
#define FACQ_IS_SOURCE_SOFT(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_SOURCE_SOFT))
#define FACQ_IS_SOURCE_SOFT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_SOURCE_SOFT))
#define FACQ_SOURCE_SOFT_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_SOURCE_SOFT, FacqSourceSoftClass))

typedef enum {
	FACQ_SOURCE_SOFT_ERROR_FAILED
} FacqSourceSoftError;

typedef enum facq_func_type {
	FACQ_FUNC_TYPE_RAN,
	FACQ_FUNC_TYPE_SIN,
	FACQ_FUNC_TYPE_COS,
	FACQ_FUNC_TYPE_FLA,
	FACQ_FUNC_TYPE_SAW,
	FACQ_FUNC_TYPE_SQU,
	/*< private >*/
	FACQ_FUNC_TYPE_N
} FacqFuncType;

typedef struct _FacqSourceSoft FacqSourceSoft;
typedef struct _FacqSourceSoftClass FacqSourceSoftClass;
typedef struct _FacqSourceSoftPrivate FacqSourceSoftPrivate;

struct _FacqSourceSoft {
	/*< private >*/
	FacqSource parent_instance;
	FacqSourceSoftPrivate *priv;
};

struct _FacqSourceSoftClass {
	/*< private >*/
	FacqSourceClass parent_class;
};

GType facq_source_soft_get_type(void) G_GNUC_CONST;

void facq_source_soft_to_file(FacqSource *src,GKeyFile *file,const gchar *group);
gpointer facq_source_soft_key_constructor(const gchar *group_name,GKeyFile *key_file,GError **err);
gpointer facq_source_soft_constructor(const GPtrArray *user_input,GError **err);
FacqSourceSoft *facq_source_soft_new(FacqFuncType fun,gdouble amplitude,gdouble wave_period,gdouble period,guint n_channels,GError **error);
/* virtual implementations */
gint facq_source_soft_poll(FacqSource *src);
GIOStatus facq_source_soft_read(FacqSource *src,gchar *buf,gsize count,gsize *bytes_read,GError **err);
void facq_source_soft_free(FacqSource *src);

G_END_DECLS

#endif


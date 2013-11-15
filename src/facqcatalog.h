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
#ifndef _FREEACQ_CATALOG_H_
#define _FREEACQ_CATALOG_H_

G_BEGIN_DECLS

typedef enum {
	FACQ_CATALOG_ERROR_FAILED
} FacqCatalogError;

typedef enum catalog_type {
	FACQ_CATALOG_TYPE_SOURCE,
	FACQ_CATALOG_TYPE_OPERATION,
	FACQ_CATALOG_TYPE_SINK,
	/*< private >*/
	FACQ_CATALOG_TYPE_N
} FacqCatalogType;

typedef gpointer(*CIConstructor)(const GPtrArray *,GError **);
typedef gpointer(*CIKeyConstructor)(const gchar *,GKeyFile *,GError **);

#ifndef __GTK_DOC_IGNORE__
/* hide this private type to gtk-doc */
typedef struct _CatalogItem {
	gchar *name;
	gchar *desc;
	gchar *dyn_dialog_string;
	gpointer icon;
	CIConstructor constructor;
	CIKeyConstructor keyconstructor;
} CatalogItem;
#endif

#define FACQ_CATALOG_ERROR facq_catalog_error_quark()

#define FACQ_TYPE_CATALOG (facq_catalog_get_type ())
#define FACQ_CATALOG(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_CATALOG, FacqCatalog))
#define FACQ_CATALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_CATALOG, FacqCatalogClass))
#define FACQ_IS_CATALOG(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_CATALOG))
#define FACQ_IS_CATALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_CATALOG))
#define FACQ_CATALOG_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_CATALOG, FacqCatalogClass))

typedef struct _FacqCatalog FacqCatalog;
typedef struct _FacqCatalogClass FacqCatalogClass;
typedef struct _FacqCatalogPrivate FacqCatalogPrivate;

struct _FacqCatalog {
	/*< private >*/
	GObject parent_instance;
	FacqCatalogPrivate *priv;
};

struct _FacqCatalogClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_catalog_get_type(void) G_GNUC_CONST;

FacqCatalog *facq_catalog_new(void);
void facq_catalog_append_source(FacqCatalog *cat,const gchar *name,const gchar *desc,const gchar *ddstring,gpointer icon,CIConstructor cons,CIKeyConstructor kcons);
void facq_catalog_append_operation(FacqCatalog *cat,const gchar *name,const gchar *desc,const gchar *ddstring,gpointer icon,CIConstructor cons,CIKeyConstructor kcons);
void facq_catalog_append_sink(FacqCatalog *cat,const gchar *name,const gchar *desc,const gchar *ddstring,gpointer icon,CIConstructor cons,CIKeyConstructor kcons);
GArray *facq_catalog_get_sources(const FacqCatalog *cat);
GArray *facq_catalog_get_operations(const FacqCatalog *cat);
GArray *facq_catalog_get_sinks(const FacqCatalog *cat);
gchar *facq_catalog_get_dyn_diag_string(const FacqCatalog *cat,FacqCatalogType type,guint index);
gchar *facq_catalog_get_name(const FacqCatalog *cat,FacqCatalogType type,guint index);
gchar *facq_catalog_get_description(const FacqCatalog *cat,FacqCatalogType type,guint index);
gpointer facq_catalog_constructor_call(const FacqCatalog *cat,FacqCatalogType type,guint index,const GPtrArray *params,GError **error);
gpointer facq_catalog_get_icon(const FacqCatalog *cat,FacqCatalogType type,guint index);
gpointer facq_catalog_item_from_key_file(GKeyFile *key_file,const gchar *group_name,const gchar *name,const FacqCatalog *cat,FacqCatalogType type,GError **err);
void facq_catalog_free(FacqCatalog *catalog);

G_END_DECLS

#endif

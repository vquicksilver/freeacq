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
#include <glib.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqi18n.h"
#include "facqglibcompat.h"
#include "facqcatalog.h"
#include "facqcatalogdialog.h"

/**
 * SECTION:facqcatalogdialog
 * @short_description: Provides a gtk dialog for displaying items in a FacqCatalog.
 * @include:facqcatalogdialog.h
 *
 * Provides a gtk dialog for displaying the items added to a #FacqCatalog
 * object, allowing to choose the type of items showed, and allowing the user
 * to choose one of the items showed in the screen.
 * When an item is selected the dialog will show at the right side the name of
 * the item and the description present in the #FacqCatalog info with the icon.
 * If you choose to show sources, only sources will be showed to the user, but
 * if you choose to select operations or sinks and extra combobox will appear
 * on top of the dialog allowing the user to choose between operations and
 * sinks.
 *
 * For creating a new #FacqCatalogDialog use facq_catalog_dialog_new(), note
 * that you will need a non empty #FacqCatalog object too, and you must specify
 * the type of objects that you want to be shown to the user. After this you
 * must call facq_catalog_dialog_run() to show the dialog to the user allowing
 * it to select an item in the catalog. To know the user input use the function
 * facq_catalog_dialog_get_input(). Finally to destroy a no longer needed
 * #FacqCatalogDialog object use facq_catalog_dialog_free().
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * #FacqCatalogDialog uses the following objects to provide it's functionality.
 * A #GtkComboBox in the case of showing sinks or operations, A #GtkDialog, 
 * A #GtkTreeStore as the model and a #GtkIconView as the view, the icon view
 * is packed inside a #GtkScrolledWindow and the #GtkScrolledWindow is packed
 * inside a #GtkFrame. At the right side of the dialog a
 * #GtkFrame is used that will show the name, the description and a icon for
 * the selected item, using also a #GtkAlignment, a vertical #GtkBox is packed
 * inside the #GtkFrame, with a #GtkLabel for the name, a #GtkImage for the icon
 * and another #GtkLabel for the description.
 *
 * Finally the left #GtkFrame and the right #GtkAlignment will be packed in an
 * horizontal #GtkBox.
 *
 * The "changed" signal from the #GtkComboBox will be used when the user chooses
 * between operation and sinks, to update the model and the view and the
 * "selection-changed" signal will be used in the #GtkIconView to update the
 * selection in the right #GtkAlignment and store the value of the selected
 * item.
 * </para>
 * </sect1>
 */

/**
 * FacqCatalogDialog:
 *
 * Contains the private details for the #FacqCatalogDialog.
 */

/**
 * FacqCatalogDialogClass:
 *
 * Class for the #FacqCatalogDialogClass.
 */

G_DEFINE_TYPE(FacqCatalogDialog,facq_catalog_dialog,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_TOP_WINDOW,
	PROP_CATALOG,
	PROP_TYPE
};

enum {
	NAME_COLUMN,
	ICON_COLUMN,
	N_COLUMNS
};

struct _FacqCatalogDialogPrivate {
	GtkWidget *top_window;
	FacqCatalog *catalog;
	FacqCatalogType type;
	GtkWidget *dialog;
	GtkWidget *combobox;
	GtkListStore *store;
	GtkWidget *label_name;
	GtkWidget *image;
	GtkWidget *label_desc;
	gboolean really_selected; //will be set to true if the user choosed an item.
	guint selected; //stores the index of the selected item
};

/*****--- Private methods ---*****/

void facq_catalog_tree_store_populate(GtkListStore *store,GArray *array)
{
	CatalogItem item;
	guint i = 0;
	GtkTreeIter iter;

	for(i = 0;i < array->len;i++){
		item = g_array_index(array,CatalogItem,i);
		gtk_list_store_append(store,&iter);
		gtk_list_store_set(store,&iter,
					NAME_COLUMN,item.name,
					ICON_COLUMN,item.icon,
					-1);
	}
}

GtkListStore *facq_catalog_tree_store_get(GArray *array)
{
	GtkListStore *store = gtk_list_store_new(N_COLUMNS,
							G_TYPE_STRING,
								GDK_TYPE_PIXBUF);

	facq_catalog_tree_store_populate(store,array);

	return store;
}

/*****--- Callbacks ---*****/

/* This callback happens when the user changes the selection in the combobox
 * with the signal "changed" */
static void facq_catalog_dialog_combobox_changed(GtkComboBox *combobox,gpointer data)
{
	FacqCatalogDialog *dialog = FACQ_CATALOG_DIALOG(data);
	FacqCatalogType prev_type = 0;
	gchar *active_selection = NULL;
	GArray *items = NULL;

	/* re-populate the icon view widget with the new data if selection
	 * differs from the current view */
	prev_type = dialog->priv->type;
	active_selection = 
#if GTK_MAJOR_VERSION > 2
		gtk_combo_box_text_get_active_text(
				GTK_COMBO_BOX_TEXT(dialog->priv->combobox));
#else
		gtk_combo_box_get_active_text(combobox);
#endif

	if(g_strcmp0(active_selection,_("Operations")) == 0){
		dialog->priv->type = FACQ_CATALOG_TYPE_OPERATION;
		items = facq_catalog_get_operations(dialog->priv->catalog);
	}
	else if(g_strcmp0(active_selection,_("Sinks")) == 0){
		dialog->priv->type = FACQ_CATALOG_TYPE_SINK;
		items = facq_catalog_get_sinks(dialog->priv->catalog);
	}
	g_free(active_selection);

	if(dialog->priv->type != prev_type){
		gtk_list_store_clear(dialog->priv->store);
		facq_catalog_tree_store_populate(dialog->priv->store,items);
		dialog->priv->really_selected = FALSE;
		gtk_label_set_text(GTK_LABEL(dialog->priv->label_name),"");
		gtk_image_clear(GTK_IMAGE(dialog->priv->image));
		gtk_label_set_text(GTK_LABEL(dialog->priv->label_desc),"");
		dialog->priv->selected = 0;
	}
}

/* This callback happens when the user chooses an icon from the icon view with
 * the signal "selection-changed" */
static void selection_changed(GtkIconView *icon_view,gpointer data)
{
	GList *list = NULL;
	gchar *string = NULL;
	GdkPixbuf *pixbuf = NULL;
	FacqCatalogDialog *dialog = FACQ_CATALOG_DIALOG(data);

	list = gtk_icon_view_get_selected_items(icon_view);
	if(list){
		string = gtk_tree_path_to_string(list->data);
		dialog->priv->selected = (guint) g_ascii_strtoull(string,NULL,10);
		g_free(string);
		g_list_free_full(list,(GDestroyNotify)gtk_tree_path_free);
		
		string = facq_catalog_get_name(dialog->priv->catalog,
							dialog->priv->type,
								dialog->priv->selected);
		gtk_label_set_text(GTK_LABEL(dialog->priv->label_name),string);
		g_free(string);
		pixbuf = facq_catalog_get_icon(dialog->priv->catalog,
							dialog->priv->type,
								dialog->priv->selected);
		gtk_image_set_from_pixbuf(GTK_IMAGE(dialog->priv->image),pixbuf);
		string = facq_catalog_get_description(dialog->priv->catalog,
								dialog->priv->type,
									dialog->priv->selected);
		gtk_label_set_text(GTK_LABEL(dialog->priv->label_desc),string);
		g_free(string);
		dialog->priv->really_selected = TRUE;
	}
}

/*****--- GObject magic ---*****/
static void facq_catalog_dialog_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqCatalogDialog *dialog = FACQ_CATALOG_DIALOG(self);

	switch(property_id){
	case PROP_TOP_WINDOW: dialog->priv->top_window = g_value_get_pointer(value);
	break;
	case PROP_CATALOG: dialog->priv->catalog = g_value_get_pointer(value);
	break;
	case PROP_TYPE: dialog->priv->type = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(dialog,property_id,pspec);
	}
}

static void facq_catalog_dialog_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqCatalogDialog *dialog = FACQ_CATALOG_DIALOG(self);

	switch(property_id){
	case PROP_TOP_WINDOW: g_value_set_pointer(value,dialog->priv->top_window);
	break;
	case PROP_CATALOG: g_value_set_pointer(value,dialog->priv->catalog);
	break;
	case PROP_TYPE: g_value_set_uint(value,dialog->priv->type);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(dialog,property_id,pspec);
	}
}

static void facq_catalog_dialog_constructed(GObject *self)
{
	FacqCatalogDialog *dialog = FACQ_CATALOG_DIALOG(self);
	GtkWidget *align = NULL, *ca = NULL;
	GtkWidget *icon_view = NULL, *frame = NULL, *scroll_window = NULL;
	GtkWidget *combobox = NULL;
	GtkListStore *store = NULL;
	GArray *store_data = NULL;
#if GTK_MAJOR_VERSION > 2
	GtkWidget *grid = NULL, *hgrid = NULL, *vgrid = NULL;
#else
	GtkWidget *main_vbox = NULL, *hbox = NULL, *vbox = NULL;
#endif

	switch(dialog->priv->type){
	case FACQ_CATALOG_TYPE_SOURCE:
		store_data = facq_catalog_get_sources(dialog->priv->catalog);
	break;
	case FACQ_CATALOG_TYPE_OPERATION:
		store_data = facq_catalog_get_operations(dialog->priv->catalog);
#if GTK_MAJOR_VERSION > 2
		combobox = gtk_combo_box_text_new();
		grid = gtk_grid_new();
		gtk_grid_set_row_spacing(GTK_GRID(grid),0);
		gtk_grid_set_row_homogeneous(GTK_GRID(grid),FALSE);
		gtk_orientable_set_orientation(GTK_ORIENTABLE(grid),GTK_ORIENTATION_VERTICAL);
#else
		combobox = gtk_combo_box_new_text();
		main_vbox = gtk_vbox_new(FALSE,0);
#endif
	break;
	case FACQ_CATALOG_TYPE_SINK:
		store_data = facq_catalog_get_sinks(dialog->priv->catalog);
#if GTK_MAJOR_VERSION > 2
		combobox = gtk_combo_box_text_new();
		grid = gtk_grid_new();
		gtk_grid_set_row_spacing(GTK_GRID(grid),0);
		gtk_grid_set_row_homogeneous(GTK_GRID(grid),FALSE);
		gtk_orientable_set_orientation(GTK_ORIENTABLE(grid),GTK_ORIENTATION_VERTICAL);
#else
		combobox = gtk_combo_box_new_text();
		main_vbox = gtk_vbox_new(FALSE,0);
#endif
	break;
	default:
		g_assert(1);
	}

#if GTK_MAJOR_VERSION > 2
	dialog->priv->dialog =
		gtk_dialog_new_with_buttons(_("Choose a component"),
				GTK_WINDOW(dialog->priv->top_window),
					GTK_DIALOG_MODAL |
						GTK_DIALOG_DESTROY_WITH_PARENT,
							_("_Cancel"),GTK_RESPONSE_CANCEL,
								_("_OK"),GTK_RESPONSE_OK,NULL);
#else
	dialog->priv->dialog = 
		gtk_dialog_new_with_buttons(_("Choose a component"),
				GTK_WINDOW(dialog->priv->top_window),
					GTK_DIALOG_MODAL | 
						GTK_DIALOG_DESTROY_WITH_PARENT,
							GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
								GTK_STOCK_OK,GTK_RESPONSE_OK,NULL);
#endif

	ca = gtk_dialog_get_content_area(GTK_DIALOG(dialog->priv->dialog));
	store = facq_catalog_tree_store_get(store_data);
	icon_view = gtk_icon_view_new_with_model(GTK_TREE_MODEL(store));
	dialog->priv->store = store;
	gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(icon_view),GTK_SELECTION_BROWSE);
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(icon_view),NAME_COLUMN);
	gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(icon_view),ICON_COLUMN);
	gtk_icon_view_set_columns(GTK_ICON_VIEW(icon_view),-1);

	frame = gtk_frame_new(_("Items"));
	scroll_window = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(scroll_window),
                                                                        GTK_SHADOW_ETCHED_IN);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scroll_window),
                                                GTK_POLICY_AUTOMATIC,
                                                        GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(frame),scroll_window);
	gtk_container_add(GTK_CONTAINER(scroll_window),icon_view);

#if GTK_MAJOR_VERSION > 2
	hgrid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(hgrid),0);
	gtk_grid_set_column_homogeneous(GTK_GRID(hgrid),FALSE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(hgrid),
					GTK_ORIENTATION_HORIZONTAL);
#else
	hbox = gtk_hbox_new(FALSE,0);
#endif

	if(combobox){
#if GTK_MAJOR_VERSION > 2
		gtk_combo_box_text_insert_text(
				GTK_COMBO_BOX_TEXT(combobox),0,_("Operations"));
		gtk_combo_box_text_insert_text(
				GTK_COMBO_BOX_TEXT(combobox),1,_("Sinks"));
#else
		gtk_combo_box_append_text(GTK_COMBO_BOX(combobox),_("Operations"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(combobox),_("Sinks"));
#endif
		gtk_combo_box_set_active(GTK_COMBO_BOX(combobox),0);
		g_signal_connect(combobox,"changed",
					G_CALLBACK(facq_catalog_dialog_combobox_changed),dialog);
		dialog->priv->combobox = combobox;

#if GTK_MAJOR_VERSION > 2
		gtk_container_add(GTK_GRID(grid),combobox);
		gtk_container_add(GTK_GRID(grid),hgrid);
		gtk_container_add(GTK_CONTAINER(ca),grid);
#else
		gtk_box_pack_start(GTK_BOX(main_vbox),combobox,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(main_vbox),hbox,TRUE,TRUE,0);
		gtk_box_pack_start(GTK_BOX(ca),main_vbox,TRUE,TRUE,0);
#endif
	} else {
#if GTK_MAJOR_VERSION > 2
	//TODO
		gtk_container_add(GTK_CONTAINER(ca),hgrid);
#else
		gtk_box_pack_start(GTK_BOX(ca),hbox,TRUE,TRUE,0);
#endif
	}

#if GTK_MAJOR_VERSION > 2
	//TODO
	gtk_container_add(GTK_CONTAINER(hgrid),frame);
#else
	gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);
#endif

	align = gtk_alignment_new(0.5,0.0,0.0,0.0);
	frame = gtk_frame_new(_("Selected Item"));
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_NONE);

#if GTK_MAJOR_VERSION > 2
	vgrid = gtk_grid_new();
	gtk_container_add(GTK_CONTAINER(align),frame);
	gtk_container_add(GTK_CONTAINER(frame),vgrid);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(vgrid),GTK_ORIENTATION_VERTICAL);
#else
	vbox = gtk_vbox_new(FALSE,8);
	gtk_container_add(GTK_CONTAINER(align),frame);
	gtk_container_add(GTK_CONTAINER(frame),vbox);
#endif
	dialog->priv->label_name = gtk_label_new("");
	dialog->priv->image = gtk_image_new();
	dialog->priv->label_desc = gtk_label_new("");
	gtk_widget_set_size_request(dialog->priv->label_desc,128,-1);
	gtk_label_set_line_wrap(GTK_LABEL(dialog->priv->label_desc),TRUE);
	gtk_label_set_line_wrap_mode(GTK_LABEL(dialog->priv->label_desc),PANGO_WRAP_WORD);

#if GTK_MAJOR_VERSION > 2
	gtk_container_add(GTK_CONTAINER(vgrid),dialog->priv->label_name);
	gtk_container_add(GTK_CONTAINER(vgrid),dialog->priv->image);
	gtk_container_add(GTK_CONTAINER(vgrid),dialog->priv->label_desc);
	gtk_container_add(GTK_CONTAINER(hgrid),align);
#else
	gtk_box_pack_start(GTK_BOX(vbox),dialog->priv->label_name,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),dialog->priv->image,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(vbox),dialog->priv->label_desc,FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(hbox),align,FALSE,FALSE,0);
#endif

	g_signal_connect(icon_view,"selection-changed",
			G_CALLBACK(selection_changed),dialog);

	gtk_widget_show_all(icon_view);
	gtk_widget_set_size_request(dialog->priv->dialog,400,300);
	gtk_widget_show_all(dialog->priv->dialog);
}

static void facq_catalog_dialog_finalize(GObject *self)
{
	FacqCatalogDialog *dialog = FACQ_CATALOG_DIALOG(self);

	if(G_IS_OBJECT(dialog->priv->store))
		g_object_unref(G_OBJECT(dialog->priv->store));

	if(GTK_IS_WIDGET(dialog->priv->combobox))
		gtk_widget_destroy(dialog->priv->combobox);

	if(GTK_IS_WIDGET(dialog->priv->label_name));
		gtk_widget_destroy(dialog->priv->label_name);

	if(GTK_IS_WIDGET(dialog->priv->image))
		gtk_widget_destroy(dialog->priv->image);

	if(GTK_IS_WIDGET(dialog->priv->label_desc))
		gtk_widget_destroy(dialog->priv->label_desc);

	if(GTK_IS_WIDGET(dialog->priv->dialog))
		gtk_widget_destroy(dialog->priv->dialog);

	G_OBJECT_CLASS(facq_catalog_dialog_parent_class)->finalize(self);
}

static void facq_catalog_dialog_class_init(FacqCatalogDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqCatalogDialogPrivate));

	object_class->set_property = facq_catalog_dialog_set_property;
	object_class->get_property = facq_catalog_dialog_get_property;
	object_class->constructed = facq_catalog_dialog_constructed;
	object_class->finalize = facq_catalog_dialog_finalize;

	g_object_class_install_property(object_class,PROP_TOP_WINDOW,
					g_param_spec_pointer("top-window",
							     "Top window",
							     "The application top window",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_CATALOG,
					g_param_spec_pointer("catalog",
							     "Catalog",
							     "A FacqCatalog object",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_TYPE,
					g_param_spec_uint("type",
							  "Type",
							  "The type of elements to show in the dialog",
							  0,
							  FACQ_CATALOG_TYPE_N-1,
							  0,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));
}

static void facq_catalog_dialog_init(FacqCatalogDialog *dialog)
{
	dialog->priv = G_TYPE_INSTANCE_GET_PRIVATE(dialog,FACQ_TYPE_CATALOG_DIALOG,FacqCatalogDialogPrivate);
	dialog->priv->top_window = NULL;
	dialog->priv->selected = 0;
	dialog->priv->really_selected = FALSE;
}

/**
 * facq_catalog_dialog_new:
 * @top_window: The application top window.
 * @cat: A #FacqCatalog object.
 * @type: A #FacqCatalogType valid value.
 *
 * Creates a new #FacqCatalogDialog object, that can be used later to show the
 * different items supported by the system, allowing the user to choose one of
 * them. The type of items showed to the user depends on the @type parameter, if
 * you choose %FACQ_CATALOG_TYPE_SOURCE only sources will be showed to the user,
 * if you choose %FACQ_CATALOG_TYPE_OPERATION or %FACQ_CATALOG_TYPE_SINK the
 * user will be able to switch between the operations and sinks item with an
 * extra combobox that will be showed on the dialog when using
 * facq_catalog_dialog_run().
 *
 * Returns: A new #FacqCatalogDialog object.
 */
FacqCatalogDialog *facq_catalog_dialog_new(GtkWidget *top_window,const FacqCatalog *cat,FacqCatalogType type)
{
	return g_object_new(FACQ_TYPE_CATALOG_DIALOG,"top-window",top_window,
						  "catalog",cat,
						  "type",type,
						  NULL);
}

/**
 * facq_catalog_dialog_run:
 * @dialog: A #FacqCatalogDialog object.
 *
 * Shows the dialog to the user allowing it to select one of the items in the
 * #FacqCatalog. Also depending on the parameters used on
 * facq_catalog_dialog_new() the user will be able to switch between operations
 * and sinks catalogs.
 *
 * Returns: %GTK_RESPONSE_OK if the user accepted the selection,
 * %GTK_RESPONSE_CANCEL in any other case.
 */
gint facq_catalog_dialog_run(FacqCatalogDialog *dialog)
{
	g_return_val_if_fail(FACQ_IS_CATALOG_DIALOG(dialog),-1);

	if( gtk_dialog_run(GTK_DIALOG(dialog->priv->dialog)) != GTK_RESPONSE_OK){
		return GTK_RESPONSE_CANCEL;
	}
	return GTK_RESPONSE_OK;
}

/**
 * facq_catalog_dialog_get_input:
 * @dialog: A #FacqCatalogDialog object.
 * @selected: If the user choosed an item it will be set to %TRUE.
 * @type: This will be set to the type of element chosen by the user.
 *
 * Returns the index of the type of the element chosen by the user.
 * You should call this function only in the case that the
 * facq_catalog_dialog_run() function returns %GTK_RESPONSE_OK.
 *
 * Returns: The index of the item type in it's list.
 */
guint facq_catalog_dialog_get_input(const FacqCatalogDialog *dialog,gboolean *selected,FacqCatalogType *type)
{
	g_return_val_if_fail(FACQ_IS_CATALOG_DIALOG(dialog),0);
	if(selected)
		*selected = dialog->priv->really_selected;
	if(type)
		*type = dialog->priv->type;

	return dialog->priv->selected;
}

/**
 * facq_catalog_dialog_free:
 * @dialog: A #FacqCatalogDialog object.
 *
 * Destroys a no longer needed #FacqCatalogDialog object.
 */
void facq_catalog_dialog_free(FacqCatalogDialog *dialog)
{
	g_return_if_fail(FACQ_IS_CATALOG_DIALOG(dialog));
	g_object_unref(G_OBJECT(dialog));
}

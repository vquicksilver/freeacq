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
#include <string.h>
#include "facqchanlist.h"
#include "facqchanlisteditor.h"
#include "facqfilechooser.h"
#include "facqdyndialog.h"

/**
 * SECTION:facqdyndialog
 * @short_description: Create gtk dialogs from strings.
 * @title:FacqDynDialog
 * @see_also:#GInitable
 * @include:facqdyndialog.h
 *
 * This module is able to create a dialog (A window with a cancel and an accept
 * button) along with other widgets that the user can manipulate. The number and
 * type of this other widgets is determined by the contents of a string,
 * simplifying the process of creating a dialog.
 *
 * The string needs to follow a syntax that depends on the type of widget that
 * the user want to display in the dialog. For creating a new #FacqDynDialog
 * object facq_dyn_dialog_new() should be called. After the object has been
 * created facq_dyn_dialog_run() can be called to display the dialog to the
 * user. If the user confirms the dialog facq_dyn_dialog_get_input() can be used
 * to retrieve the user inputs.
 * When finished facq_dyn_dialog_free() should be called to free the associated
 * resources.
 *
 * <sect1 id="string-syntax">
 *  <title>String syntax</title>
 *  <para>
 *  This section describes the syntax that needs to be used with the strings,
 *  and what is the result of using it.
 *  </para>
 *   <sect2 id="string-noparameters">
 *    <title>Empty dialog (No dialog)</title>
 *    <para>
 *    Use this if you don't need any dialog at all. This is supposed to be alone
 *    without any other strings.
 *    <programlisting>
 *    "NOPARAMETERS"
 *    </programlisting>
 *    </para>
 *   </sect2>
 *   <sect2 id="string-boolean">
 *    <title>Boolean values</title>
 *    <para>
 *    For reading boolean values from the dialog we must use the following
 *    syntax.
 *    <programlisting>
 *    "BOOLEAN,Text for label,DefaultValue"
 *    </programlisting>
 *    DefaultValue can be 0 for %FALSE or 1 for %TRUE.
 *    The dialog will have a #GtkCheckButton if you use this string.
 *    </para>
 *   </sect2>
 *   <sect2 id="string-uint">
 *    <title>Unsigned integers</title>
 *    <para>
 *    For reading unsigned integers from the dialog we must use the following
 *    syntax.
 *    <programlisting>
 *    "UINT,Text for the label,Maximum,Minimum,Default,Step"
 *    </programlisting>
 *    Note that Step should be an integer value.
 *    The dialog will have a #GtkSpinButton if you use this string.
 *    </para>
 *   </sect2>
 *   <sect2 id="string-double">
 *    <title>Real values</title>
 *    <para>
 *    For reading double (Real values) from the dialog we must use the following
 *    syntax.
 *    <programlisting>
 *    "DOUBLE,Text for the label,Maximum,Minimum,Default,Step,Digits"
 *    </programlisting>
 *    The dialog will have a #GtkSpinButton if you use this string.
 *    </para>
 *   </sect2>
 *   <sect2 id="string-string">
 *    <title>Text strings</title>
 *    <para>
 *    For reading text strings from the dialog we must use the following syntax.
 *    <programlisting>
 *    "STRING,Text for the label,Default value"
 *    </programlisting>
 *    The dialog will have a #GtkEntry if you use this string.
 *    </para>
 *   </sect2>
 *   <sect2 id="string-chanlist">
 *    <title>List of channels</title>
 *    <para>
 *    For reading a channel list from the dialog we must use the following
 *    syntax.
 *    <programlisting>
 *    "CHANLIST,Input,Advanced,MaxChannels,ExtraAref"
 *    </programlisting>
 *    Note that Input, Advanced and ExtraAref can be 0 or 1. Input controls the direction
 *    of the channels while Advanced controls if the range and analog reference
 *    parameters while be showed to the user. MaxChannels corresponds to an
 *    unsigned integer, it's the maximum number of channels that the user can
 *    add to the list of channels. The ExtraAref parameter can be used for
 *    showing an extra combobox that will allow the user to set an analog
 *    reference value for all the channels when the Advanced value is 0.
 *    The widget is #FacqChanlistEditor and the input will be in form of a #FacqChanlist.
 *    </para>
 *   </sect2>
 *   <sect2 id="string-function">
 *    <title>Functions (Used in #FacqSourceSoft)</title>
 *    <para>
 *    <programlisting>
 *     "FUNCTION,Text for the label"
 *    </programlisting>
 *    The dialog will have a new #GtkComboBox with the different kind of
 *    functions supported by #FacqSource.
 *    </para>
 *   </sect2>
 *   <sect2 id="string-filename">
 *    <title>Filenames</title>
 *    For reading a filename from the dialog we must use the following syntax,
 *    note that in this case, this should be the only string in the string from
 *    the dialog, cause this time the widget is a full #FacqFileChooser object.
 *    <para>
 *    <programlisting>
 *    "FILENAME,Mode,Extension,Description"
 *    </programlisting>
 *    Where mode can be 0 or 1. 0 means that you are going to save a file
 *    and 1 means that you are going to read a file, behaviour of the
 *    dialog changes depending on this parameter, Extension is a 3 letters
 *    string describing the kind of file for example, txt, and Description
 *    is the description of the file for example Plain Text File. An
 *    example for saving plain text files would be, "FILENAME,0,txt,Plain Text
 *    File" .
 *    </para>
 *   </sect2>
 * </sect1>
 *
 * <sect1 id="reading-input">
 *  <title>Reading the input data</title>
 *  <para>
 *  To get the input values you should call facq_dyn_dialog_get_input(). This
 *  function returns a #GPtrArray where each pointer points to an allocated
 *  variable of the type defined by the syntax in the string, in order.
 *  So for example for a string like "UINT,.../DOUBLE,..." the first pointer in
 *  the GPtrArray will point to an unsigned integer variable in memory and the
 *  second pointer will point to a double variable in memory. This pointers can
 *  be accessed with the g_ptr_array_index() macro, and doing some casting it's
 *  easy to obtain the input from the user.
 *  </para>
 * </sect1>
 */

/**
 * FacqDynDialog:
 *
 * Contains the internal details of a #FacqDynDialog.
 */

/**
 * FacqDynDialogClass:
 *
 * Class for the #FacqDynDialog objects.
 */

/**
 * FacqDynDialogError:
 * @FACQ_DYN_DIALOG_ERROR_FAILED: An error happened in the #FacqDynDialog.
 *
 * Enum for the possible error values of #FacqDynDialog.
 */

static void facq_dyn_dialog_initable_iface_init(GInitableIface  *iface);
static gboolean facq_dyn_dialog_initable_init(GInitable *initable,GCancellable *cancellable,GError **error);

G_DEFINE_TYPE_WITH_CODE(FacqDynDialog,facq_dyn_dialog,G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_INITABLE,facq_dyn_dialog_initable_iface_init));

enum {
	PROP_0,
	PROP_TOP_WINDOW,
	PROP_DESCRIPTION
};

struct _FacqDynDialogPrivate {
	GtkWidget *dialog;
	GtkWidget *top_window;
	gchar *description;
	guint n_tokens;
	gchar **tokens;
	GPtrArray *variables;
	GPtrArray *widgets;
	GError *construct_error;
};

GQuark facq_dyn_dialog_error_quark(void)
{
	return g_quark_from_static_string("facq-dyn-dialog-error-quark");
}

/* A description with only one element doesn't have any ':',
 * A description with 2 elements has one ':',
 * A description with 3 elements has two ':',
 * ....
 * A description with n elements has n-1 ':',*/
static guint facq_dyn_dialog_get_n_tokens(const gchar *description)
{
	guint i = 0, j = 1;
	gsize len = 0;

	if(!description)
		return 0;
	
	len = strlen(description);
	for(i = 0;i < len;i++){
		if(description[i] == '/')
			j++;
	}
	return j;
}

static gchar **facq_dyn_dialog_split_description(const gchar *description)
{
	guint max_tokens = 0;
	gchar **ret = NULL;

	max_tokens = facq_dyn_dialog_get_n_tokens(description);
	ret = g_strsplit_set(description,"/",max_tokens);
	return ret;
}

static gchar **facq_dyn_dialog_get_details_from_token(const gchar *token)
{
	gchar **ret = NULL;

	ret = g_strsplit_set(token,",",-1);
	return ret;
}

static GtkWidget *facq_dyn_dialog_get_widget_for_numeric(gchar **details)
{
	GtkWidget *widget = NULL;
	gdouble max = 0, min = 0, def = 0, step = 1;
	guint digits = 3;
	
	max = g_ascii_strtod(details[2],NULL);
	min = g_ascii_strtod(details[3],NULL);
	def = g_ascii_strtod(details[4],NULL);
	step = g_ascii_strtod(details[5],NULL);
	if(g_strcmp0(details[0],"DOUBLE") == 0){
		if(details[6] != NULL){
			digits = (guint) g_ascii_strtod(details[6],NULL);
		}
		widget = gtk_spin_button_new_with_range(min,max,step);
		gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget),digits);
	}
	else {
		widget = gtk_spin_button_new_with_range(min,max,step);
		gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget),0);
	}

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),def);

	return widget;
}

static GtkWidget *facq_dyn_dialog_get_widget_for_boolean(gchar **details)
{
	GtkWidget *ret = NULL;
	gboolean state = FALSE;

	ret = gtk_check_button_new();
	state = (gboolean) g_ascii_strtod(details[2],NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ret),state);

	return ret;
}

static GtkWidget *facq_dyn_dialog_get_widget_for_text(gchar **details)
{
	GtkWidget *ret = NULL;
	
	ret = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(ret),details[2]);

	return ret;
}

static GtkWidget *facq_dyn_dialog_get_widget_for_function(gchar **details)
{
	GtkWidget *ret = NULL;

	/* deprecated code, but it's the only way to be compatible */

	ret = gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(ret),"Random");
	gtk_combo_box_append_text(GTK_COMBO_BOX(ret),"Sine");
	gtk_combo_box_append_text(GTK_COMBO_BOX(ret),"Cosine");
	gtk_combo_box_append_text(GTK_COMBO_BOX(ret),"Flat");
	gtk_combo_box_append_text(GTK_COMBO_BOX(ret),"Sawtooth");
	gtk_combo_box_append_text(GTK_COMBO_BOX(ret),"Square");
	gtk_combo_box_set_active(GTK_COMBO_BOX(ret),0);

	return ret;
}

static FacqChanlistEditor *facq_dyn_dialog_get_editor_for_chanlist(gchar **details)
{
	FacqChanlistEditor *ed = NULL;
	gboolean input = TRUE;
	gboolean advanced = FALSE;
	guint max_channels = 1;
	gboolean extra_aref = FALSE;

	input = (gboolean) g_ascii_strtod(details[1],NULL);
	advanced = (gboolean) g_ascii_strtod(details[2],NULL);
	max_channels = (guint) g_ascii_strtod(details[3],NULL);
	extra_aref = (gboolean) g_ascii_strtod(details[4],NULL);

	ed = facq_chanlist_editor_new(input,advanced,max_channels,extra_aref);
	return ed;
}

static FacqFileChooser *facq_dyn_dialog_get_dialog_for_filechooser(gchar **details,GtkWidget *top_window)
{
	FacqFileChooser *chooser = NULL;
	const gchar *extension = NULL, *description = NULL;
	guint mode = 0;

	mode = (guint) g_ascii_strtod(details[1],NULL);
	description = details[3];
	extension = details[2];

	chooser = facq_file_chooser_new(top_window,mode,extension,description);

	return chooser;
}

static gpointer facq_dyn_dialog_get_widget_from_details(gchar **details,GtkWidget *top_window)
{
	GtkWidget *hbox = NULL, *label = NULL, *widget = NULL;
	FacqChanlistEditor *ed = NULL;
	FacqFileChooser *chooser = NULL;

	if(g_strcmp0(details[0],"CHANLIST") != 0 && g_strcmp0(details[0],"FILENAME") != 0){
		hbox = gtk_hbox_new(FALSE,0);
		label = gtk_label_new(details[1]);
		gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

		if(g_strcmp0(details[0],"BOOLEAN") == 0){
			widget = facq_dyn_dialog_get_widget_for_boolean(details);
		}
		if(g_strcmp0(details[0],"UINT") == 0){
			widget = facq_dyn_dialog_get_widget_for_numeric(details);
		}
		if(g_strcmp0(details[0],"DOUBLE") == 0){
			widget = facq_dyn_dialog_get_widget_for_numeric(details);
		}
		if(g_strcmp0(details[0],"STRING") == 0){
			widget = facq_dyn_dialog_get_widget_for_text(details);
		}
		if(g_strcmp0(details[0],"FUNCTION") == 0){
			widget = facq_dyn_dialog_get_widget_for_function(details);
		}
		gtk_box_pack_end(GTK_BOX(hbox),widget,FALSE,FALSE,0);
		return hbox;
	}
	else {
		if(g_strcmp0(details[0],"CHANLIST") == 0){
			ed = facq_dyn_dialog_get_editor_for_chanlist(details);
			return ed;
		}
		if(g_strcmp0(details[0],"FILENAME") == 0){
			chooser = facq_dyn_dialog_get_dialog_for_filechooser(details,top_window);
			return chooser;
		}
	}
	
	return NULL;
}

static gpointer facq_dyn_dialog_get_variable_from_details(const gchar *detail0)
{
	gpointer ret = NULL;

	if(g_strcmp0(detail0,"BOOLEAN") == 0){
		ret = g_malloc0(sizeof(gboolean));
	}
	if(g_strcmp0(detail0,"UINT") == 0 || 
			g_strcmp0(detail0,"FUNCTION") == 0){
				ret = g_malloc0(sizeof(guint));
	}
	if(g_strcmp0(detail0,"DOUBLE") == 0){
		ret = g_malloc0(sizeof(gdouble));
	}
	if(g_strcmp0(detail0,"STRING") == 0){
		ret = NULL;
	}
	if(g_strcmp0(detail0,"CHANLIST") == 0){
		ret = NULL;
	}
	if(g_strcmp0(detail0,"FILENAME") == 0){
		ret = NULL;
	}
	return ret;
}

static void facq_dyn_dialog_get_value_from_uint_widget(gpointer var,GtkWidget *hbox)
{
	guint *integer = (guint *)var;
	gdouble tmp = 0;
	GtkWidget *widget = NULL;
	GList *list = NULL;

	g_return_if_fail(GTK_IS_BOX(hbox));

	list = gtk_container_get_children(GTK_CONTAINER(hbox));
	while(list != NULL){
		widget = list->data;
		if(GTK_IS_SPIN_BUTTON(widget))
			break;
		else
			list = list->next;
	}

	g_return_if_fail(GTK_IS_SPIN_BUTTON(widget));
	tmp = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
	*integer = (guint)tmp;
}

static void facq_dyn_dialog_get_value_from_boolean_widget(gpointer var,GtkWidget *hbox)
{
	gboolean *boolean = (gboolean *)var;
	GtkWidget *widget = NULL;
	GList *list = NULL;
	gboolean tmp = FALSE;

	g_return_if_fail(GTK_IS_BOX(hbox));
	list = gtk_container_get_children(GTK_CONTAINER(hbox));
	while(list != NULL){
		widget = list->data;
		if(GTK_IS_SPIN_BUTTON(widget))
			break;
		else
			list = list->next;
	}

	g_return_if_fail(GTK_IS_CHECK_BUTTON(widget));
	tmp = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	*boolean = tmp;
}

static void facq_dyn_dialog_get_value_from_double_widget(gpointer var,GtkWidget *hbox)
{
	gdouble *real = (gdouble *)var;
	gdouble tmp = 0;
	GtkWidget *widget = NULL;
	GList *list = NULL;

	g_return_if_fail(GTK_IS_BOX(hbox));

	list = gtk_container_get_children(GTK_CONTAINER(hbox));
	while(list != NULL){
		widget = list->data;
		if(GTK_IS_SPIN_BUTTON(widget))
			break;
		else
			list = list->next;
	}

	tmp = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
	*real = tmp;
}

static void facq_dyn_dialog_get_value_from_function_widget(gpointer var,GtkWidget *hbox)
{
	GtkWidget *widget = NULL;
	GList *list = NULL;
	gchar *active_string = NULL;
	guint *value = (guint *)var;

	g_return_if_fail(GTK_IS_BOX(hbox));

	list = gtk_container_get_children(GTK_CONTAINER(hbox));
	while(list != NULL){
		widget = list->data;
		if(GTK_IS_COMBO_BOX(widget))
			break;
		else
			list = list->next;
	}

	active_string = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));
	
	if(g_strcmp0(active_string,"Random") == 0){
		*value = 0;
	}
	if(g_strcmp0(active_string,"Sine") == 0){
		*value = 1;
	}
	if(g_strcmp0(active_string,"Cosine") == 0){
		*value = 2;
	}
	if(g_strcmp0(active_string,"Flat") == 0){
		*value = 3;
	}
	if(g_strcmp0(active_string,"Sawtooth") == 0){
		*value = 4;
	}
	if(g_strcmp0(active_string,"Square") == 0){
		*value = 5;
	}
	g_free(active_string);
}

static gchar *facq_dyn_dialog_get_value_from_string_widget(GtkWidget *hbox)
{
	GtkWidget *widget = NULL;
	GList *list = NULL;

	g_return_val_if_fail(GTK_IS_BOX(hbox),NULL);

	list = gtk_container_get_children(GTK_CONTAINER(hbox));
	while(list != NULL){
		widget = list->data;
		if(GTK_IS_ENTRY(widget))
			break;
		else
			list = list->next;
	}

	return g_strdup(gtk_entry_get_text(GTK_ENTRY(widget)));
}

static FacqChanlist *facq_dyn_dialog_get_value_from_chanlist_editor(FacqChanlistEditor *ed)
{
	FacqChanlist *chanlist = NULL;

	chanlist = facq_chanlist_editor_get_chanlist(ed);
	return chanlist;
}

static gchar *facq_dyn_dialog_get_filename_from_filechooser(FacqFileChooser *chooser)
{
	gchar *ret = NULL;

	ret = facq_file_chooser_get_filename_for_system(chooser);
	return ret;
}

static void facq_dyn_dialog_get_value_from_widget(const gchar *details0,GPtrArray *widgets,GPtrArray *variables,guint i)
{
	if(g_strcmp0(details0,"BOOLEAN") == 0){
		facq_dyn_dialog_get_value_from_boolean_widget(
				g_ptr_array_index(variables,i),
					g_ptr_array_index(widgets,i));
	}
	if(g_strcmp0(details0,"UINT") == 0){
		facq_dyn_dialog_get_value_from_uint_widget(
				g_ptr_array_index(variables,i),
					g_ptr_array_index(widgets,i));
	}
	if(g_strcmp0(details0,"DOUBLE") == 0){
		facq_dyn_dialog_get_value_from_double_widget(
				g_ptr_array_index(variables,i),
					g_ptr_array_index(widgets,i));
	}
	if(g_strcmp0(details0,"STRING") == 0){
		g_ptr_array_index(variables,i) = (gpointer) 
			facq_dyn_dialog_get_value_from_string_widget(
						g_ptr_array_index(widgets,i));
	}
	if(g_strcmp0(details0,"CHANLIST") == 0){
		g_ptr_array_index(variables,i) = (gpointer)
			facq_dyn_dialog_get_value_from_chanlist_editor(
						g_ptr_array_index(widgets,i));
	}
	if(g_strcmp0(details0,"FUNCTION") == 0){
		facq_dyn_dialog_get_value_from_function_widget(
				g_ptr_array_index(variables,i),
					g_ptr_array_index(widgets,i));
	}
	if(g_strcmp0(details0,"FILENAME") == 0){
		g_ptr_array_index(variables,i) = (gpointer)
			facq_dyn_dialog_get_filename_from_filechooser(
						g_ptr_array_index(widgets,i));
	}
}

static void facq_dyn_dialog_widget_destructor(gpointer object)
{
	if(GTK_IS_WIDGET(object)){
		gtk_widget_destroy(object);
		return;
	}
	if(FACQ_IS_CHANLIST_EDITOR(object)){
		facq_chanlist_editor_free(object);
		return;
	}
	if(FACQ_IS_FILE_CHOOSER(object)){
		facq_file_chooser_free(object);
		return;
	}
}

static void facq_dyn_dialog_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqDynDialog *dialog = FACQ_DYN_DIALOG(self);

	switch(property_id){
	case PROP_TOP_WINDOW: g_value_set_pointer(value,dialog->priv->top_window);
	break;
	case PROP_DESCRIPTION: g_value_set_string(value,dialog->priv->description);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(dialog,property_id,pspec);
	}
}

static void facq_dyn_dialog_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqDynDialog *dialog = FACQ_DYN_DIALOG(self);

	switch(property_id){
	case PROP_TOP_WINDOW: dialog->priv->top_window = g_value_get_pointer(value);
	break;
	case PROP_DESCRIPTION: dialog->priv->description = g_value_dup_string(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(dialog,property_id,pspec);
	}
}

static void facq_dyn_dialog_constructed(GObject *self)
{
	FacqDynDialog *dialog = FACQ_DYN_DIALOG(self);
	guint i = 0;
	gchar **details = NULL;
	GtkWidget *widget = NULL, *vbox = NULL;
	gpointer variable = NULL;
	gboolean widget_is_dialog = FALSE;
	
	dialog->priv->n_tokens = facq_dyn_dialog_get_n_tokens(dialog->priv->description);
	if(dialog->priv->n_tokens == 0){
		g_set_error_literal(&dialog->priv->construct_error,FACQ_DYN_DIALOG_ERROR,
						FACQ_DYN_DIALOG_ERROR_FAILED,"Invalid description");
		return;
	}

	dialog->priv->tokens = facq_dyn_dialog_split_description(dialog->priv->description);
	
	dialog->priv->widgets = 
		g_ptr_array_new_with_free_func((GDestroyNotify)facq_dyn_dialog_widget_destructor);
	
	dialog->priv->variables = g_ptr_array_sized_new(dialog->priv->n_tokens);

	for(i = 0;i < dialog->priv->n_tokens;i++){
		details = 
			facq_dyn_dialog_get_details_from_token(dialog->priv->tokens[i]);
		if(g_strcmp0(details[0],"FILENAME") == 0){
			widget_is_dialog = TRUE;
		}
		if(g_strcmp0(details[0],"NOPARAMETERS") == 0){
			g_strfreev(details);
			return;
		}
		widget = facq_dyn_dialog_get_widget_from_details(details,dialog->priv->top_window);
		g_ptr_array_add(dialog->priv->widgets,widget);
		variable = facq_dyn_dialog_get_variable_from_details(details[0]);
		g_ptr_array_add(dialog->priv->variables,variable);
		g_strfreev(details);
		if(widget_is_dialog)
			break;
		}

	if(!widget_is_dialog){
		dialog->priv->dialog = gtk_dialog_new_with_buttons("Preferences",
						GTK_WINDOW(dialog->priv->top_window),
							GTK_DIALOG_MODAL |
								GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										GTK_STOCK_OK, GTK_RESPONSE_OK,NULL);

		vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog->priv->dialog));

		for(i = 0;i < dialog->priv->widgets->len;i++){
			if(GTK_IS_WIDGET(g_ptr_array_index(dialog->priv->widgets,i)))
				gtk_box_pack_start(GTK_BOX(vbox),
						g_ptr_array_index(dialog->priv->widgets,i),
							FALSE,FALSE,0);
			else if(FACQ_IS_CHANLIST_EDITOR(g_ptr_array_index(dialog->priv->widgets,i)))
				gtk_box_pack_start(GTK_BOX(vbox),
						facq_chanlist_editor_get_widget(
							g_ptr_array_index(dialog->priv->widgets,i)), 
											TRUE,TRUE,0);
		}
	}
	else {
		dialog->priv->dialog = NULL;
	}

	if(dialog->priv->dialog)
		gtk_widget_show_all(dialog->priv->dialog);
}

static void facq_dyn_dialog_finalize(GObject *self)
{
	FacqDynDialog *dialog = FACQ_DYN_DIALOG(self);
	guint i = 0;
	gchar **details = NULL;
	gpointer var = NULL;

	if(dialog->priv->widgets){
		g_ptr_array_free(dialog->priv->widgets,TRUE);
	}

	if(dialog->priv->description)
		g_free(dialog->priv->description);
	
	if(dialog->priv->variables){
		if(dialog->priv->variables->len > 0){
			for(i = 0;i < dialog->priv->n_tokens;i++){
				details = facq_dyn_dialog_get_details_from_token(dialog->priv->tokens[i]);
				var = g_ptr_array_index(dialog->priv->variables,i);
				if(g_strcmp0(details[0],"CHANLIST") == 0){
					if(FACQ_IS_CHANLIST(var))
						facq_chanlist_free(var);
				}
				else{
					if(var)
						g_free(var);
				}
				g_strfreev(details);
			}
		}
		g_ptr_array_free(dialog->priv->variables,TRUE);
	}

	if(dialog->priv->tokens)
		g_strfreev(dialog->priv->tokens);

	g_clear_error(&dialog->priv->construct_error);

	if(GTK_IS_WIDGET(dialog->priv->dialog))
		gtk_widget_destroy(dialog->priv->dialog);
}

static void facq_dyn_dialog_class_init(FacqDynDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqDynDialogPrivate));

	object_class->set_property = facq_dyn_dialog_set_property;
	object_class->get_property = facq_dyn_dialog_get_property;
	object_class->finalize = facq_dyn_dialog_finalize;
	object_class->constructed = facq_dyn_dialog_constructed;
	
	g_object_class_install_property(object_class,PROP_TOP_WINDOW,
					g_param_spec_pointer("top-window",
							     "Top window",
							     "The application top window",
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_DESCRIPTION,
					g_param_spec_string("description",
							    "Description",
							    "The dialog description string",
							     NULL,
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_STATIC_STRINGS));
}

static void facq_dyn_dialog_init(FacqDynDialog *dialog)
{
	dialog->priv = G_TYPE_INSTANCE_GET_PRIVATE(dialog,FACQ_TYPE_DYN_DIALOG,FacqDynDialogPrivate);
	dialog->priv->top_window = NULL;
	dialog->priv->description = NULL;
	dialog->priv->tokens = NULL;
	dialog->priv->variables = NULL;
	dialog->priv->widgets = NULL;
	dialog->priv->construct_error = NULL;
	dialog->priv->n_tokens = 0;
}

/***** GInitable *****/
static void facq_dyn_dialog_initable_iface_init(GInitableIface *iface)
{
        iface->init = facq_dyn_dialog_initable_init;
}

static gboolean facq_dyn_dialog_initable_init(GInitable *initable,GCancellable *cancellable,GError **error)
{
	FacqDynDialog *dialog;
        
	g_return_val_if_fail(FACQ_IS_DYN_DIALOG(initable),FALSE);
	dialog = FACQ_DYN_DIALOG(initable);
	if(cancellable != NULL){
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
			"Cancellable initialization not supported");
		return FALSE;
	}
	if(dialog->priv->construct_error){
		if(error)
			*error = g_error_copy(dialog->priv->construct_error);
		return FALSE;
	}
	return TRUE;
}

/***** public methods *****/
/**
 * facq_dyn_dialog_new:
 * @top_window: The dialog desired top window, usually the application top
 * window.
 * @description: A string that describes how the dialog will be.
 * @err: A #GError, it will be set in case of error if not %NULL.
 *
 * Creates a new #FacqDynDialog object. The object can be used to show to the
 * user a #GtkDialog with a number of widgets that depends on the @description
 * string. See the string syntax section in this page for more details.
 *
 * Returns: A new #FacqDynDialog object, or %NULL in case of error.
 */
FacqDynDialog *facq_dyn_dialog_new(GtkWidget *top_window,const gchar *description,GError **err)
{
	return g_initable_new(FACQ_TYPE_DYN_DIALOG,NULL,err,
				"top-window",top_window,
				"description",description,NULL);
}

/**
 * facq_dyn_dialog_run:
 * @dialog: A #FacqDynDialog object.
 *
 * Runs the dialog displaying it to the user.
 *
 * Returns: %GTK_RESPONSE_OK or %GTK_RESPONSE_CANCEL depending on the user
 * response.
 */
gint facq_dyn_dialog_run(FacqDynDialog *dialog)
{
	guint i = 0;
	gchar **details = NULL;

	g_return_val_if_fail(FACQ_IS_DYN_DIALOG(dialog),GTK_RESPONSE_CANCEL);

	/* No parameters */
	if(dialog->priv->widgets->len == 0)
		return GTK_RESPONSE_OK;

	/* Only a FILENAME parameter */
	if( FACQ_IS_FILE_CHOOSER(g_ptr_array_index(dialog->priv->widgets,0)) ){
		if( facq_file_chooser_run_dialog(g_ptr_array_index(dialog->priv->widgets,0)) 
					!= GTK_RESPONSE_ACCEPT){
			return GTK_RESPONSE_CANCEL;
		}
	}
	else {
		/* Parameters without FILENAME */
		if( gtk_dialog_run(GTK_DIALOG(dialog->priv->dialog)) != GTK_RESPONSE_OK)
			return GTK_RESPONSE_CANCEL;
	}
	
	for(i = 0;i < dialog->priv->n_tokens;i++){
		details = facq_dyn_dialog_get_details_from_token(dialog->priv->tokens[i]);
		facq_dyn_dialog_get_value_from_widget(details[0],
							dialog->priv->widgets,
								dialog->priv->variables,i);
		g_strfreev(details);
	}

	return GTK_RESPONSE_OK;
}

/**
 * facq_dyn_dialog_get_input:
 * @dialog: A #FacqDynDialog object.
 *
 * Gets the user input in form of a #GPtrArray. The type and number of members
 * of this pointer array depends on the string you used to create the #FacqDynDialog.
 *
 * Returns: A #GPtrArray you shouldn't free it.
 */
const GPtrArray *facq_dyn_dialog_get_input(FacqDynDialog *dialog)
{
	g_return_val_if_fail(FACQ_IS_DYN_DIALOG(dialog),NULL);
	return dialog->priv->variables;
}

/**
 * facq_dyn_dialog_free:
 * @dialog: A #FacqDynDialog object.
 *
 * Destroys a no longer needed #FacqDynDialog object.
 */
void facq_dyn_dialog_free(FacqDynDialog *dialog)
{
	g_return_if_fail(FACQ_IS_DYN_DIALOG(dialog));
	g_object_unref(G_OBJECT(dialog));
}

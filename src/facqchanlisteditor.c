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
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include "facqi18n.h"
#include "facqchanlist.h"
#include "facqchanlisteditor.h"

/**
 * SECTION:facqchanlisteditor
 * @title:FacqChanlistEditor
 * @short_description: Graphical widget for editing chanlists.
 * @include:facqchanlisteditor.h
 *
 * Allows the user to graphically edit a list of channels, and allows the
 * developer to retrieve this info in an easy way, in form of a #FacqChanlist
 * object. Different types of dialogs can be showed to the user depending on
 * the number of parameters per channel that you need to ask to the user.
 * For example comedi devices require that you specify the number of the
 * channel, the range, and the analog reference for each channel, while NIDAQ
 * software only allows to use one analog reference for each channel, and
 * doesn't understand the concept of ranges. #FacqChanlistEditor was designed
 * looking at this differences and allows to request which parameters should be
 * showed to the user.
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * A #FacqChanlistEditor uses the following objects to operate.
 *
 * In the first row we use a horizontal #GtkBox along a #GtkLabel and a #GtkSpinButton,
 * for setting the number of channels, the signal "value-changed" is used in
 * the #GtkSpinButton, this signal is emitted each time the value changes,
 * and a callback is called that adds or removes channels according to the new
 * value.
 *
 * If the extra_aref parameter is set to true, another horizontal #GtkBox is
 * created along another #GtkLabel and a #GtkComboBox with the valid values for
 * the analog references. The "changed" signal of this latter #GtkComboBox will
 * be emitted each time the user manipulates the value, taking care of storing
 * the value the user selected.
 *
 * Depending on the advanced property, a #GtkListStore (This is the model) will
 * be created with 3 (If advanced is %TRUE) or 1 (If advanced is %FALSE)
 * columns, for storing the number of the channels, the range, and the analog
 * reference. A #GtkTreeView is created for displaying the previous
 * #GtkListStore (This is the view). The #GtkTreeView is packed inside a
 * #GtkScrolledWindow. Finally the #GtkTreeView uses one or more (Again
 * depending on the advanced property) #GtkCellRenderer objects along with one 
 * or various #GtkTreeViewColumn objects, callbacks signals are connected for
 * each renderer (See the "edited" signal) and an initial value for the model is set.
 *
 * All of this gets packed on a vertical #GtkBox that will work as the top level
 * container widget.
 * </para>
 * </sect1>
 */

/**
 * FacqChanlistEditor:
 *
 * Contains the private details of #FacqChanlistEditor
 */

/**
 * FacqChanlistEditorClass:
 *
 * Class for all the #FacqChanlistEditor objects.
 */

G_DEFINE_TYPE(FacqChanlistEditor,facq_chanlist_editor,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_INPUT,
	PROP_ADVANCED,
	PROP_MAX_CHANNELS,
	PROP_EXTRA_AREF
};

enum {
	CHAN_COLUMN,
	CHAN_RANGE,
	CHAN_AREF,
	N_COLUMNS
};

struct _FacqChanlistEditorPrivate {
	GtkListStore *store;
	gboolean input;
	gboolean advanced;
	gboolean extra_aref;
	guint max_channels;
	GtkWidget *spin_button;
	GtkWidget *list;
	GtkWidget *vbox;
	GtkComboBox *extra_combobox;
	guint extra_aref_value;
	guint prev_n_channels;
};

/*****--- Private methods ---*****/
static GtkComboBox *facq_chanlist_editor_new_extra_combobox(void)
{
	GtkComboBox *combobox = NULL;

#if GTK_MAJOR_VERSION > 2
	combobox = GTK_COMBO_BOX(gtk_combo_box_text_new());
	
	gtk_combo_box_text_append_text(
			GTK_COMBO_BOX_TEXT(combobox),_("Ground/RSE"));
	gtk_combo_box_text_append_text(
			GTK_COMBO_BOX_TEXT(combobox),_("Common/NRSE"));
	gtk_combo_box_text_append_text(
			GTK_COMBO_BOX_TEXT(combobox),_("Differential"));
	gtk_combo_box_text_append_text(
			GTK_COMBO_BOX_TEXT(combobox),_("Other/Default"));
#else
	combobox = GTK_COMBO_BOX(gtk_combo_box_new_text());

	gtk_combo_box_append_text(combobox,_("Ground/RSE"));
	gtk_combo_box_append_text(combobox,_("Common/NRSE"));
	gtk_combo_box_append_text(combobox,_("Differential"));
	gtk_combo_box_append_text(combobox,_("Other/Default"));
#endif

	return combobox;
}

static guint aref_from_string(const gchar *string)
{
	guint aref = 0;

	if(g_strcmp0(string,_("Ground/RSE")) == 0)
		aref = AREF_GROUND;
	if(g_strcmp0(string,_("Common/NRSE")) == 0)
		aref = AREF_COMMON;
	if(g_strcmp0(string,_("Differential")) == 0)
		aref = AREF_DIFF;
	if(g_strcmp0(string,_("Other/Default")) == 0)
		aref = AREF_OTHER;

	return aref;
}

static GtkListStore *facq_chanlist_editor_model_for_combo(void)
{
	GtkListStore *store = NULL;
	GtkTreeIter iter;

	store = gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_INT);

	gtk_list_store_append(store,&iter);
	gtk_list_store_set(store,&iter,0,_("Ground/RSE"),-1);
	
	gtk_list_store_append(store,&iter);
	gtk_list_store_set(store,&iter,0,_("Common/NRSE"),-1);
	
	gtk_list_store_append(store,&iter);
	gtk_list_store_set(store,&iter,0,_("Differential"),-1);
	
	gtk_list_store_append(store,&iter);
	gtk_list_store_set(store,&iter,0,_("Other/Default"),-1);

	return store;
}

static void append_channels(FacqChanlistEditor *ed,guint channels)
{
	guint i = 0;
	GtkTreeIter iter;
	gchar *channel = NULL;

	for(i = 0;i < channels;i++){
		gtk_list_store_append(ed->priv->store,&iter);
		channel = g_strdup_printf("%u",ed->priv->prev_n_channels);
		if(ed->priv->advanced)
			gtk_list_store_set(ed->priv->store,&iter,
						CHAN_COLUMN,channel,
						CHAN_RANGE,"0",
						CHAN_AREF,_("Ground/RSE"),
						-1);
		else
			gtk_list_store_set(ed->priv->store,&iter,
						CHAN_COLUMN,channel,
						-1);

		g_free(channel);
		ed->priv->prev_n_channels++;
	}
}

static void remove_channels(FacqChanlistEditor *ed,guint channels)
{
	guint i = 0;
	GtkTreeIter iter;
	gchar *path = NULL;

	for(i = 0;i < channels;i++){
		path = g_strdup_printf("%u",ed->priv->prev_n_channels-1);
		gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(ed->priv->store),&iter,path);
		g_free(path);
		gtk_list_store_remove(ed->priv->store,&iter);
		ed->priv->prev_n_channels--;
	}
}

/*****--- Callbacks ---*****/
/* This callbacks runs when some of the widgets on a row changes it's value
 * after the "edited" signal is emitted */
static void channel_widget_edited(GtkCellRendererText *renderer,gchar *path,gchar *new_text,gpointer data)
{
	FacqChanlistEditor *ed = FACQ_CHANLIST_EDITOR(data);
	GtkTreeIter iter;

	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(ed->priv->store),&iter,path);
	gtk_list_store_set(ed->priv->store,&iter,CHAN_COLUMN,new_text,-1);
}

static void range_widget_edited(GtkCellRendererText *renderer,gchar *path,gchar *new_text,gpointer data)
{
	FacqChanlistEditor *ed = FACQ_CHANLIST_EDITOR(data);
	GtkTreeIter iter;

	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(ed->priv->store),&iter,path);
	gtk_list_store_set(ed->priv->store,&iter,CHAN_RANGE,new_text,-1);
}

static void aref_widget_edited(GtkCellRendererText *renderer,gchar *path,gchar *new_text,gpointer data)
{
	FacqChanlistEditor *ed = FACQ_CHANLIST_EDITOR(data);
	GtkTreeIter iter;

	gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(ed->priv->store),&iter,path);
	gtk_list_store_set(ed->priv->store,&iter,CHAN_AREF,new_text,-1);
}

/* This callback is run when the spin button controlling the number of channels
 * changes it's value emitting the value-changed signal */
static void n_channels_changed(GtkSpinButton *spinbutton,gpointer data)
{
	FacqChanlistEditor *ed = FACQ_CHANLIST_EDITOR(data);
	guint n_channels = (guint)gtk_spin_button_get_value_as_int(spinbutton);

	if(n_channels != ed->priv->prev_n_channels){
		if(n_channels > ed->priv->prev_n_channels)
			append_channels(ed,n_channels - ed->priv->prev_n_channels);
		else
			remove_channels(ed,ed->priv->prev_n_channels - n_channels);

		ed->priv->prev_n_channels = n_channels;
	}
}
/* This callbacks runs when the user changes the value of the extra combobox,
 * in response to the "changed" signal */
void extra_combobox_changed(GtkComboBox *combobox,gpointer data)
{
	FacqChanlistEditor *ed = FACQ_CHANLIST_EDITOR(data);
	gchar *analog_aref = NULL;

#if GTK_MAJOR_VERSION > 2
	analog_aref = 
		gtk_combo_box_text_get_active_text(
				GTK_COMBO_BOX_TEXT(combobox));
#else
	analog_aref = gtk_combo_box_get_active_text(combobox);
#endif
	ed->priv->extra_aref_value = aref_from_string(analog_aref);
	g_free(analog_aref);
}

/*****--- GObject magic ---*****/
static void facq_chanlist_editor_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqChanlistEditor *ed = FACQ_CHANLIST_EDITOR(self);

	switch(property_id){
	case PROP_INPUT: ed->priv->input = g_value_get_boolean(value);
	break;
	case PROP_ADVANCED: ed->priv->advanced = g_value_get_boolean(value);
	break;
	case PROP_MAX_CHANNELS: ed->priv->max_channels = g_value_get_uint(value);
	break;
	case PROP_EXTRA_AREF: ed->priv->extra_aref = g_value_get_boolean(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(ed,property_id,pspec);
	}
}

static void facq_chanlist_editor_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqChanlistEditor *ed = FACQ_CHANLIST_EDITOR(self);

	switch(property_id){
	case PROP_INPUT: g_value_set_boolean(value,ed->priv->input);
	break;
	case PROP_ADVANCED: g_value_set_boolean(value,ed->priv->advanced);
	break;
	case PROP_MAX_CHANNELS: g_value_set_uint(value,ed->priv->max_channels);
	break;
	case PROP_EXTRA_AREF: g_value_set_boolean(value,ed->priv->extra_aref);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(ed,property_id,pspec);
	}
}

static void facq_chanlist_editor_constructed(GObject *self)
{
	FacqChanlistEditor *ed = FACQ_CHANLIST_EDITOR(self);
	GtkWidget *label = NULL, *hbox = NULL, *scroll_window = NULL;
	GtkCellRenderer *renderer = NULL;
	GtkTreeViewColumn *column = NULL;
	GtkAdjustment *adj = NULL;
	GtkTreeIter iter;
	GtkListStore *combobox_store = NULL;

	label = gtk_label_new(_("Number of channels"));
	gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
	hbox = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
	ed->priv->spin_button = gtk_spin_button_new_with_range(1,ed->priv->max_channels,1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ed->priv->spin_button),1.0);
	gtk_box_pack_start(GTK_BOX(hbox),ed->priv->spin_button,FALSE,FALSE,0);
	g_signal_connect(ed->priv->spin_button,"value-changed",
				G_CALLBACK(n_channels_changed),ed);
	ed->priv->vbox = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(ed->priv->vbox),hbox,FALSE,FALSE,0);

	if(ed->priv->extra_aref){
		label = gtk_label_new(_("Analog reference"));
		gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
		hbox = gtk_hbox_new(FALSE,0);
		gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
		ed->priv->extra_combobox = facq_chanlist_editor_new_extra_combobox();
		gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(ed->priv->extra_combobox),FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(ed->priv->vbox),hbox,FALSE,FALSE,0);
		gtk_combo_box_set_active(ed->priv->extra_combobox,0);
		g_signal_connect(ed->priv->extra_combobox,"changed",
					G_CALLBACK(extra_combobox_changed),ed);
	}

	if(ed->priv->advanced){
		ed->priv->store = 
			gtk_list_store_new(N_COLUMNS,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);
	}
	else {
		ed->priv->store =
			gtk_list_store_new(CHAN_COLUMN+1,G_TYPE_STRING);
	}

	ed->priv->list = 
			gtk_tree_view_new_with_model(GTK_TREE_MODEL(ed->priv->store));
	scroll_window = 
		gtk_scrolled_window_new(NULL,NULL);
	
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll_window),
									GTK_SHADOW_ETCHED_IN);
	
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window),
						GTK_POLICY_AUTOMATIC,
							GTK_POLICY_AUTOMATIC);

	gtk_container_add(GTK_CONTAINER(scroll_window),ed->priv->list);

	if(ed->priv->advanced){

		adj = (GtkAdjustment *)gtk_adjustment_new(0.0,0.0,255.0,1.0,10.0,0.0);
		renderer = 
			gtk_cell_renderer_spin_new();
		g_signal_connect(renderer,"edited",
					G_CALLBACK(channel_widget_edited),ed);
		g_object_set(renderer,
			     "editable",TRUE,
			     "adjustment",adj,
			     NULL);
		column = 
			gtk_tree_view_column_new_with_attributes(_("Channel"),renderer,"text",CHAN_COLUMN,NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(ed->priv->list),column);

		adj = (GtkAdjustment *)gtk_adjustment_new(0.0,0.0,255.0,1.0,10.0,0.0);
		renderer = 
			gtk_cell_renderer_spin_new();
		g_signal_connect(renderer,"edited",
					G_CALLBACK(range_widget_edited),ed);
		g_object_set(renderer,
			     "editable",TRUE,
			     "adjustment",adj,
			     NULL);
		column = 
			gtk_tree_view_column_new_with_attributes(_("Range"),renderer,"text",CHAN_RANGE,NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(ed->priv->list),column);

		renderer = gtk_cell_renderer_combo_new();
		g_signal_connect(renderer,"edited",
					G_CALLBACK(aref_widget_edited),ed);
		combobox_store = facq_chanlist_editor_model_for_combo();
		g_object_set(renderer,
			     "has-entry",FALSE,
			     "model",GTK_TREE_MODEL(combobox_store),
			     "text-column",0,
			     "editable",TRUE,NULL);
		g_object_unref(combobox_store);
		column = gtk_tree_view_column_new_with_attributes(_("Analog reference"),renderer,"text",CHAN_AREF,NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(ed->priv->list),column);

		gtk_list_store_append(ed->priv->store,&iter);
		gtk_list_store_set(ed->priv->store,&iter,
					CHAN_COLUMN,"0",
					CHAN_RANGE,"0",
					CHAN_AREF,_("Ground/RSE"),
					-1);
	}
	else {
		adj = (GtkAdjustment *)gtk_adjustment_new(0.0,0.0,255.0,1.0,10.0,0.0);
		renderer = 
			gtk_cell_renderer_spin_new();
		g_signal_connect(renderer,"edited",
					G_CALLBACK(channel_widget_edited),ed);
		g_object_set(renderer,
			     "editable",TRUE,
			     "adjustment",adj,
			     NULL);
		column = 
			gtk_tree_view_column_new_with_attributes(_("Channel"),renderer,"text",CHAN_COLUMN,NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(ed->priv->list),column);
		
		gtk_list_store_append(ed->priv->store,&iter);
		gtk_list_store_set(ed->priv->store,&iter,
					CHAN_COLUMN,"0",
					-1);
	}

	gtk_box_pack_start(GTK_BOX(ed->priv->vbox),scroll_window,TRUE,TRUE,0);

	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(ed->priv->list));
	
	gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(ed->priv->list),
				GTK_TREE_VIEW_GRID_LINES_HORIZONTAL);

	ed->priv->prev_n_channels = 1;
}

static void facq_chanlist_editor_finalize(GObject *self)
{
	FacqChanlistEditor *ed = FACQ_CHANLIST_EDITOR(self);

	if(GTK_IS_WIDGET(ed->priv->list))
		gtk_widget_destroy(ed->priv->list);

	if(GTK_IS_WIDGET(ed->priv->vbox))
		gtk_widget_destroy(ed->priv->vbox);

	if(G_IS_OBJECT(ed->priv->store))
		g_object_unref(G_OBJECT(ed->priv->store));

	G_OBJECT_CLASS(facq_chanlist_editor_parent_class)->finalize(self);
}

static void facq_chanlist_editor_class_init(FacqChanlistEditorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqChanlistEditorPrivate));

	object_class->set_property = facq_chanlist_editor_set_property;
	object_class->get_property = facq_chanlist_editor_get_property;
        object_class->constructed = facq_chanlist_editor_constructed;
        object_class->finalize = facq_chanlist_editor_finalize;

        g_object_class_install_property(object_class,PROP_INPUT,
                                        g_param_spec_boolean("input",
                                                            "Input",
                                                            "The input direction of the channels",
                                                            TRUE,
                                                            G_PARAM_CONSTRUCT_ONLY |
                                                            G_PARAM_READWRITE |
                                                            G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_ADVANCED,
                                        g_param_spec_boolean("advanced",
                                                            "Advanced",
                                                            "Enables the advanced view, showing range, aref and flags",
                                                            FALSE,
                                                            G_PARAM_CONSTRUCT_ONLY |
                                                            G_PARAM_READWRITE |
                                                            G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_MAX_CHANNELS,
					g_param_spec_uint("max-channels",
							  "Maximum channels",
							  "The maximum number of channels to show",
							  1,
							  256,
							  1,
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_READWRITE |
							  G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_EXTRA_AREF,
					g_param_spec_boolean("extra-aref",
							     "Extra aref",
							     "Enables and extra combobox for AREF value in NIDAQ",
							     FALSE,
							     G_PARAM_CONSTRUCT_ONLY |
							     G_PARAM_READWRITE |
							     G_PARAM_STATIC_STRINGS));
}

static void facq_chanlist_editor_init(FacqChanlistEditor *ed)
{
	ed->priv = G_TYPE_INSTANCE_GET_PRIVATE(ed,FACQ_TYPE_CHANLIST_EDITOR,FacqChanlistEditorPrivate);
	ed->priv->input = TRUE;
	ed->priv->advanced = FALSE;
	ed->priv->max_channels = 1;
	ed->priv->extra_aref = FALSE;
}

/****--- public methods ---*****/
/**
 * facq_chanlist_editor_new:
 * @input: %TRUE if the direction of the channels is input, %FALSE otherwise.
 * @advanced: If %TRUE range and analog reference parameters will be shown to
 * the user.
 * @max_channels: The maximum number of channels to show to the user.
 * @extra_aref: When @advanced is %FALSE, and this %TRUE #FacqChanlistEditor 
 * will show an extra #GtkCombobox with the different kinds of analog 
 * references, the value selected will be applied to all the channels.
 *
 * Creates a new #FacqChanlistEditor that is able to retrieve enough information
 * from the user to create a #FacqChanlist. Depending on the source/sink you
 * want different kind of options to be displayed to the user, for the moment
 * use the following parameters for the following objects.
 *
 * #FacqSourceComediSync: TRUE,TRUE,1,FALSE
 * #FacqSourceComediAsync: TRUE,TRUE,256,FALSE
 * #FacqSourceNidaq: TRUE,FALSE,256,TRUE
 * #FacqSinkNidaq: FALSE,FALSE,256,FALSE
 *
 * Returns: A new #FacqChanlistEditor object.
 */
FacqChanlistEditor *facq_chanlist_editor_new(gboolean input,gboolean advanced,guint max_channels,gboolean extra_aref)
{
	return g_object_new(FACQ_TYPE_CHANLIST_EDITOR,"input",input,
						      "advanced",advanced,
						      "max-channels",max_channels,
						      "extra-aref",extra_aref,NULL);
}

/**
 * facq_chanlist_editor_get_widget:
 * @ed: A #FacqChanlistEditor object.
 *
 * Gets the top level #GtkWidget of the #FacqChanlistEditor, so you can
 * add it to your application, and show it to the user.
 *
 * Returns: The top level widget.
 */
GtkWidget *facq_chanlist_editor_get_widget(const FacqChanlistEditor *ed)
{
	g_return_val_if_fail(FACQ_IS_CHANLIST_EDITOR(ed),NULL);
	return ed->priv->vbox;
}

/**
 * facq_chanlist_editor_get_chanlist:
 * @ed: A #FacqChanlistEditor object.
 *
 * Creates a new #FacqChanlist from the information stored by the user in a
 * #FacqChanlistEditor object.
 *
 * Returns: A new #FacqChanlist object.
 */
FacqChanlist *facq_chanlist_editor_get_chanlist(const FacqChanlistEditor *ed)
{
	FacqChanlist *chanlist = NULL;
	GtkTreeIter iter;
	guint i = 0, chan = 0, rng = 0, aref = 0;
	gchar *index = NULL, *channel = NULL, *range = NULL, *analog_ref = NULL; 
	FacqChanDir dir = 0;

	dir = (ed->priv->input) ? CHAN_INPUT : CHAN_OUTPUT;

	if(ed->priv->extra_aref){
		aref = ed->priv->extra_aref_value;
	}

	chanlist = facq_chanlist_new();
	for(i = 0;i < ed->priv->prev_n_channels;i++){
		index = g_strdup_printf("%u",i);
		gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(ed->priv->store),&iter,index);
		if(ed->priv->advanced){
			gtk_tree_model_get(GTK_TREE_MODEL(ed->priv->store),
						&iter,CHAN_COLUMN,&channel,
						CHAN_RANGE,&range,
						CHAN_AREF,&analog_ref,
						-1);
			chan = (guint) g_ascii_strtoull(channel,NULL,10);
			rng = (guint) g_ascii_strtoull(range,NULL,10);
			aref = aref_from_string(analog_ref);
			g_free(channel);
			g_free(range);
			g_free(analog_ref);
			facq_chanlist_add_chan(chanlist,chan,rng,aref,0,dir);
		} else {
			gtk_tree_model_get(GTK_TREE_MODEL(ed->priv->store),
						&iter,CHAN_COLUMN,&channel,-1);
			chan = (guint) g_ascii_strtoull(channel,NULL,10);
			g_free(channel);
			facq_chanlist_add_chan(chanlist,chan,0,aref,0,dir);
		}
	}

	return chanlist;
}

/**
 * facq_chanlist_editor_free:
 * @ed: A #FacqChanlistEditor object.
 *
 * Destroys a no longer needed #FacqChanlistEditor object.
 */
void facq_chanlist_editor_free(FacqChanlistEditor *ed)
{
	g_return_if_fail(FACQ_IS_CHANLIST_EDITOR(ed));
	g_object_unref(G_OBJECT(ed));
}

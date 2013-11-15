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
#include "facqcolor.h"
#include "facqdisplay.h"

/**
 * SECTION:facqdisplay
 * @title:FacqDisplay
 * @short_description:Provides a widget for displaying the number of pulses per
 * minute.
 * @include:facqdisplay.h
 *
 * Provides a graphical widget that is able to display the number of pulses per
 * minute, the number of the physical channel used, and the patient name.
 * Alternatively you can set any other text in the title or in the footer.
 * The display value should be between 0 and 999.9 any higher value will be
 * threated like 999.9 and any value minor than 0 will be threated like 0.
 *
 * Also it's used by #FacqDisplayMatrix.
 *
 * To create a new #FacqDisplay object use facq_display_new(), to get the
 * toplevel widget use facq_display_get_widget(), to manage the displayed
 * numeric value use facq_display_set_value() and facq_display_get_value(),
 * to set the title of the toplevel frame use facq_display_set_title(), to
 * set the text on the footer label use facq_display_set_footer() and finally
 * to manage the text in the #GtkEntry use facq_display_get_entry_text() and
 * facq_display_set_entry_text().
 *
 * <sect1 id="internal-details">
 * <title>Internal details</title>
 * <para>
 * Internally a #FacqDisplay uses the following widgets, a #GtkFrame as the
 * toplevel widget, inside this widget we use a vertical #GtkBox that contains
 * the other elements, a #GtkEntry, a #GtkDrawingArea and a #GtkLabel.
 * </para>
 * </sect1>
 */

/**
 * FacqDisplay:
 *
 * Contains the private members of the objects.
 */

/**
 * FacqDisplayClass:
 *
 * Class for the #FacqDisplay objects.
 */

G_DEFINE_TYPE(FacqDisplay,facq_display,G_TYPE_OBJECT);

static gboolean facq_display_expose_event(GtkWidget *dra,GdkEventExpose *event,gpointer display);

enum {
	PROP_0,
	PROP_TITLE,
	PROP_ENTRYTEXT,
	PROP_FOOTER,
	PROP_INDEX
};

struct _FacqDisplayPrivate {
	GtkWidget *vbox;
	GtkWidget *frame;
	GtkWidget *entry;
	GtkWidget *dra;
	GtkWidget *footer_label;
	gchar *title;
	gchar *entrytext;
	gchar *footer;
	guint index;
	gdouble value;
};

/*****--- GObject magic ---*****/
static void facq_display_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqDisplay *dis = FACQ_DISPLAY(self);

	switch(property_id){
	case PROP_TITLE: dis->priv->title = g_value_dup_string(value);
	break;
	case PROP_ENTRYTEXT: dis->priv->entrytext = g_value_dup_string(value);
	break;
	case PROP_FOOTER: dis->priv->footer = g_value_dup_string(value);
	break;
	case PROP_INDEX: dis->priv->index = g_value_get_uint(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(dis,property_id,pspec);
	}
}

static void facq_display_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqDisplay *dis = FACQ_DISPLAY(self);

	switch(property_id){
	case PROP_TITLE: g_value_set_string(value,dis->priv->title);
	break;
	case PROP_ENTRYTEXT: g_value_set_string(value,dis->priv->entrytext);
	break;
	case PROP_FOOTER: g_value_set_string(value,dis->priv->footer);
	break;
	case PROP_INDEX: g_value_set_uint(value,dis->priv->index);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(dis,property_id,pspec);
	}
}

static void facq_display_constructed(GObject *self)
{
	FacqDisplay *dis = FACQ_DISPLAY(self);
	GtkWidget *vbox = NULL;
	GtkWidget *frame = NULL;
	GtkWidget *dra = NULL;
	GtkWidget *footer_label = NULL;
	GtkWidget *entry = NULL;

	/* frame */
	frame = gtk_frame_new(dis->priv->title);
	gtk_frame_set_label_align(GTK_FRAME(frame),0.5,0.0);
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	/* entry */
	entry = gtk_entry_new();
	if(dis->priv->entrytext)
		gtk_entry_set_text(GTK_ENTRY(entry),dis->priv->entrytext);
	//gtk_entry_set_overwrite_mode(GTK_ENTRY(entry),TRUE);
	gtk_entry_set_alignment(GTK_ENTRY(entry),0.5);

	/* drawing area */
	dra = gtk_drawing_area_new();

	/* label */
	footer_label = gtk_label_new(dis->priv->footer);

	/* vbox */
	vbox = gtk_vbox_new(FALSE,0);

	gtk_container_add(GTK_CONTAINER(frame),vbox);

	gtk_box_pack_start(GTK_BOX(vbox),
			   entry,FALSE,FALSE,3);

	gtk_box_pack_start(GTK_BOX(vbox),
			   dra,TRUE,TRUE,3);

	gtk_box_pack_end(GTK_BOX(vbox),
			 footer_label,FALSE,FALSE,3);

	/* connect draw function on expose-event */
	g_signal_connect(dra,"expose-event",
			G_CALLBACK(facq_display_expose_event),dis);


	gtk_widget_set_size_request(dra,150,50);

	gtk_widget_show_all(vbox);

	dis->priv->vbox = vbox;
	dis->priv->frame = frame;
	dis->priv->entry = entry;
	dis->priv->dra = dra;
	dis->priv->footer_label = footer_label;
}

static void facq_display_finalize(GObject *self)
{
	FacqDisplay *dis = FACQ_DISPLAY(self);

	if(GTK_IS_WIDGET(dis->priv->dra))
		gtk_widget_destroy(dis->priv->dra);

	if(GTK_IS_WIDGET(dis->priv->frame))
		gtk_widget_destroy(dis->priv->frame);

	if(GTK_IS_WIDGET(dis->priv->footer_label))
		gtk_widget_destroy(dis->priv->footer_label);

	if(GTK_IS_WIDGET(dis->priv->entry))
		gtk_widget_destroy(dis->priv->entry);

	if(dis->priv->title)
		g_free(dis->priv->title);

	if(dis->priv->entrytext)
		g_free(dis->priv->entrytext);

	if(dis->priv->footer)
		g_free(dis->priv->footer);

	G_OBJECT_CLASS(facq_display_parent_class)->finalize(self);
}

static void facq_display_class_init(FacqDisplayClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqDisplayPrivate));

	object_class->set_property = facq_display_set_property;
	object_class->get_property = facq_display_get_property;
        object_class->constructed = facq_display_constructed;
        object_class->finalize = facq_display_finalize;

	g_object_class_install_property(object_class,PROP_TITLE,
					g_param_spec_string("title",
							    "Title",
							    "The text in the frame's label",
							    NULL,
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_ENTRYTEXT,
					g_param_spec_string("entry-text",
							    "The entry text",
							    "The initial text in the GtkEntry",
							    "Unknown patient",
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_FOOTER,
					g_param_spec_string("footer",
							    "Footer",
							    "The text in the widget's footer",
							    NULL,
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT_ONLY |
							    G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class,PROP_INDEX,
					g_param_spec_uint("index",
							  "Index",
							  "The channel index",
							  0,
							  G_MAXUINT8,
							  0,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS));
}

static void facq_display_init(FacqDisplay *dis)
{
	dis->priv = G_TYPE_INSTANCE_GET_PRIVATE(dis,FACQ_TYPE_DISPLAY,FacqDisplayPrivate);
	dis->priv->value = 0.00;
	dis->priv->index = 0;
}

/****--- private callbacks ---*****/
static gboolean facq_display_expose_event(GtkWidget *dra,GdkEventExpose *event,gpointer display)
{
	FacqDisplay *dis = FACQ_DISPLAY(display);
	cairo_t *cr = NULL;
	cairo_text_extents_t extents;
	GdkColor color;
	gchar *text_value = NULL;
	gint width = 0, height = 0;

	cr = gdk_cairo_create(dra->window);

	/* get drawing area size */
#if GTK_MAJOR_VERSION == 2 && GTK_MINOR_VERSION < 24
	gdk_drawable_get_size(GDK_DRAWABLE(dra->window),&width,&height);
#else
	width = gdk_window_get_width(dra->window);
	height = gdk_window_get_height(dra->window);
#endif

	/* draw black bottom */
	cairo_set_source_rgb(cr,0,0,0);
	cairo_rectangle(cr,0,0,100,100);
	cairo_paint(cr);

	/* draw the value */
	facq_gdk_color_from_index(dis->priv->index,&color);
	gdk_cairo_set_source_color(cr,&color);
	cairo_select_font_face(cr,"Sans",
			CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr,40.0);
	text_value = g_strdup_printf("%.2f",dis->priv->value);
	cairo_text_extents(cr,text_value,&extents);
	cairo_move_to(cr,width/2 - extents.width/2,height/2 + extents.height/2);
	cairo_show_text(cr,text_value);
	g_free(text_value);

	/* destroy the cairo context */
	cairo_destroy(cr);

	return TRUE;
}

/****--- public methods ---*****/
/**
 * facq_display_new:
 * @title: (allow-none): A string for the toplevel frame title or %NULL.
 * @entrytext: A string for the #GtkEntry or %NULL.
 * @footer: A string for the footer #GtkLabel or %NULL.
 * @channel: The channel index, the color of the displayed value will depend 
 * on the value of this parameter, see #FacqColor for more details.
 * It should be between 0 and 255.
 *
 * Creates a new #FacqDisplay object, with the requested parameters.
 *
 * Returns: A new #FacqDisplay object.
 */
FacqDisplay *facq_display_new(const gchar *title,const gchar *entrytext,const gchar *footer,guint channel)
{
	return g_object_new(FACQ_TYPE_DISPLAY,
			    "title",title,
			    "entry-text",entrytext,
			    "footer",footer,
			    "index",channel,NULL);
}

/**
 * facq_display_get_widget:
 * @dis: A #FacqDisplay object.
 *
 * Gets the toplevel #GtkWidget from a #FacqDisplay object, so you can add it
 * to your application.
 *
 * Returns: The toplevel #GtkWidget.
 */
GtkWidget *facq_display_get_widget(const FacqDisplay *dis)
{
	g_return_val_if_fail(FACQ_IS_DISPLAY(dis),NULL);
	return dis->priv->frame;
}

/**
 * facq_display_get_value:
 * @dis: A #FacqDisplay object:
 *
 * Gets the last value set in the #FacqDisplay object.
 *
 * Returns: A #gdouble with the last value set.
 */
gdouble facq_display_get_value(const FacqDisplay *dis)
{
	g_return_val_if_fail(FACQ_IS_DISPLAY(dis),-1);
	return dis->priv->value;
}

/**
 * facq_display_set_value:
 * @dis: A #FacqDisplay object.
 * @value: A #gdouble, note that only values between 0 and 999.9 will be showed.
 *
 * Sets the value to be displayed by the #FacqDisplay. If the value is lower
 * than 0, the display will show 0, if the value is higher than 999.9 the
 * display will show 999.9.
 */
void facq_display_set_value(FacqDisplay *dis,gdouble value)
{
	g_return_if_fail(FACQ_IS_DISPLAY(dis));

	if(value <= 999.99 && value >= 0)
		dis->priv->value = value;
	else if(value < 0)
		dis->priv->value = 0.0;
	else
		dis->priv->value = 999.99;

	gtk_widget_queue_draw(dis->priv->dra);
}

/**
 * facq_display_set_title:
 * @dis: A #FacqDisplay object.
 * @title: A string with the text for the title.
 *
 * Sets the text in the label of the toplevel #GtkFrame.
 */
void facq_display_set_title(FacqDisplay *dis,const gchar *title)
{
	g_return_if_fail(FACQ_IS_DISPLAY(dis));
	g_return_if_fail(title);

	if(dis->priv->title)
		g_free(dis->priv->title);

	dis->priv->title = g_strdup(title);
	gtk_frame_set_label(GTK_FRAME(dis->priv->frame),dis->priv->title);
}

/**
 * facq_display_set_entry_text:
 * @dis: A #FacqDisplay object.
 * @entrytext: A string with the text for the #GtkEntry.
 *
 * Sets the text in the #GtkEntry.
 */
void facq_display_set_entry_text(FacqDisplay *dis,const gchar *entrytext)
{
	g_return_if_fail(FACQ_IS_DISPLAY(dis));
	g_return_if_fail(entrytext);

	if(dis->priv->entrytext)
		g_free(dis->priv->entrytext);
	dis->priv->entrytext = g_strdup(entrytext);
	gtk_entry_set_text(GTK_ENTRY(dis->priv->entry),dis->priv->entrytext);
}

/**
 * facq_display_get_entry_text:
 * @dis: A #FacqDisplay object.
 *
 * Gets the last text written to the #GtkEntry of the #FacqDisplay.
 *
 * Returns: The last text written, free it with g_free() when no longer needed.
 */
gchar *facq_display_get_entry_text(FacqDisplay *dis)
{
	g_return_val_if_fail(FACQ_IS_DISPLAY(dis),NULL);

	return g_strdup(gtk_entry_get_text(GTK_ENTRY(dis->priv->entry)));
}

/**
 * facq_display_set_footer:
 * @dis: A #FacqDisplay object.
 * @footer: A string with the text for the footer label.
 * 
 * Sets the text in the footer's #GtkLabel.
 */
void facq_display_set_footer(FacqDisplay *dis,const gchar *footer)
{
	g_return_if_fail(FACQ_IS_DISPLAY(dis));
	g_return_if_fail(footer);

	if(dis->priv->footer)
		g_free(dis->priv->footer);
	dis->priv->footer = g_strdup(footer);
	gtk_label_set_text(GTK_LABEL(dis->priv->footer_label),dis->priv->footer);
}

/**
 * facq_display_free:
 * @dis: A #FacqDisplay object.
 *
 * Destroys a no longer needed #FacqDisplay object.
 */
void facq_display_free(FacqDisplay *dis)
{
	g_return_if_fail(FACQ_IS_DISPLAY(dis));
	g_object_unref(G_OBJECT(dis));
}

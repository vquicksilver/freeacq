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
#include "facqstatusbar.h"

#define INITIAL_MESSAGE " "
#define DEFAULT_CONTEXT_ID "FREEACQ_STATUSBAR"

static const gchar *DCI = DEFAULT_CONTEXT_ID;
static const gchar *IM = INITIAL_MESSAGE;

/**
 * SECTION:facqstatusbar
 * @include:facqstatusbar.h
 * @short_description: A simplified statusbar, based on the Gtk statusbar.
 *
 * This module provides a simplified statusbar, giving application writers an
 * easy way to use the #GtkStatusbar.
 */

/**
 * FacqStatusbar:
 *
 * Contains the private fields of the #FacqStatusbar.
 */

/**
 * FacqStatusbarClass:
 *
 * The #FacqStatusbar class.
 */

G_DEFINE_TYPE(FacqStatusbar,facq_statusbar,G_TYPE_OBJECT);

enum {
	PROP_0,
};

struct _FacqStatusbarPrivate {
	GtkWidget *statusbar;
	guint dci;
	guint msg_id;
};

/*****--- GObject Magic ---*****/
static void facq_statusbar_finalize(GObject *self)
{
	FacqStatusbar *statusbar = FACQ_STATUSBAR(self);

	if(GTK_IS_WIDGET(statusbar->priv->statusbar)){
		gtk_statusbar_remove(GTK_STATUSBAR(statusbar->priv->statusbar),
				     statusbar->priv->dci,
				     statusbar->priv->msg_id);
		gtk_widget_destroy(statusbar->priv->statusbar);
	}

	G_OBJECT_CLASS(facq_statusbar_parent_class)->finalize(self);
}

static void facq_statusbar_constructed(GObject *self)
{
	FacqStatusbar *statusbar = FACQ_STATUSBAR(self);
	
	statusbar->priv->statusbar = gtk_statusbar_new();
	
	statusbar->priv->dci = 
		gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar->priv->statusbar),DCI);
	
	statusbar->priv->msg_id = 
		gtk_statusbar_push(GTK_STATUSBAR(statusbar->priv->statusbar),statusbar->priv->dci,IM);
	
	gtk_widget_show(statusbar->priv->statusbar);
}

static void facq_statusbar_class_init(FacqStatusbarClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqStatusbarPrivate));

	object_class->finalize = facq_statusbar_finalize;
	object_class->constructed = facq_statusbar_constructed;
}

static void facq_statusbar_init(FacqStatusbar *statusbar)
{
	statusbar->priv = G_TYPE_INSTANCE_GET_PRIVATE(statusbar,FACQ_TYPE_STATUSBAR,FacqStatusbarPrivate);
	statusbar->priv->statusbar = NULL;
	statusbar->priv->dci = 0;
	statusbar->priv->msg_id = 0;
}

/*****--- Public methods ---*****/
/**
 * facq_statusbar_new:
 *
 * Creates a new #FacqStatusbar object.
 *
 * Returns: A new #FacqStatusbar object.
 */
FacqStatusbar *facq_statusbar_new(void)
{
	return g_object_new(FACQ_TYPE_STATUSBAR,NULL);
}

/**
 * facq_statusbar_get_widget:
 * @statusbar: A #FacqStatusbar object.
 *
 * Gets the #GtkWidget, (A #GtkStatusbar) from the #FacqStatusbar object,
 * so you can add it to your application.
 *
 * Returns: A #GtkWidget.
 */
GtkWidget *facq_statusbar_get_widget(const FacqStatusbar *statusbar)
{
	g_return_val_if_fail(FACQ_IS_STATUSBAR(statusbar),NULL);
	return statusbar->priv->statusbar;
}

/**
 * facq_statusbar_write_msg:
 * @statusbar: A #FacqStatusbar object.
 * @format: a standard printf() format string. See g_strdup_vprintf() for
 * details.
 * @...: the parameters to insert into the format string
 *
 * Writes a <function>printf()</function> string to the statusbar, removing the
 * previous message if any.
 */
void facq_statusbar_write_msg(FacqStatusbar *statusbar,const gchar *format,...)
{
	va_list args;
        gchar *msg = NULL;
	
	g_return_if_fail(FACQ_IS_STATUSBAR(statusbar));

	va_start(args,format);
	msg = g_strdup_vprintf(format,args);

	gtk_statusbar_remove(GTK_STATUSBAR(statusbar->priv->statusbar),
					statusbar->priv->dci,statusbar->priv->msg_id);
	
	statusbar->priv->msg_id = gtk_statusbar_push(GTK_STATUSBAR(statusbar->priv->statusbar),
							statusbar->priv->dci,msg);
	g_free(msg);
        va_end(args);
}

/**
 * facq_statusbar_free:
 * @statusbar: A #FacqStatusbar object.
 *
 * Destroys a no longer needed #FacqStatusbar object.
 */
void facq_statusbar_free(FacqStatusbar *statusbar)
{
	g_return_if_fail(FACQ_IS_STATUSBAR(statusbar));
	g_object_unref(G_OBJECT(statusbar));
}

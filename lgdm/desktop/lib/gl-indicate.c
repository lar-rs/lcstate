/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gui-process-new
 * Copyright (C) sascha 2011 <sascha@sascha-desktop>
 * 
gui-process-new is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gui-process-new is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl-indicate.h"

struct _GlIndicatePrivate
{
	gchar**        image_prof;
	gint           image_counter;

	guint          user_ask;
	gboolean       remove;


// full info ----------------------
	GtkWidget*     user_fi_box;
	GtkWidget*     yesButton;
	GtkWidget*     noButton;
	gboolean       is_fi_box;
// short info ---------------------
	GtkWidget*     user_i_box;
	GtkWidget*     image;
	gboolean       is_i_box;
	GtkWidget     *yesImage;
	GtkWidget     *noImage;

};

#define GL_INDICATE_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_INDICATE, GlIndicatePrivate))

enum {
	PROP_INDICATE_0,
	PROP_INDICATE_NAME,
	PROP_INDICATE_USER_QUESTION,
	PROP_INDICATE_USER_YES_IMAGE,
	PROP_INDICATE_USER_NO_IMAGE
	//PROP_PLUGIN_WIDGET_SIZE
};

G_DEFINE_TYPE (GlIndicate, gl_indicate, MKT_TYPE_ATOM);
enum
{
	GL_INDICATE_CLICK_START,
	GL_INDICATE_CLICK_YES,
	GL_INDICATE_CLICK_NO,
	GL_INDICATE_START_INDICATE,
	GL_INDICATE_STOP_INDICATE,

	LAST_SIGNAL
};


static guint gl_indicate_signals[LAST_SIGNAL] = { 0 };


void gl_indicate_click_yes_button(GtkWidget *widget , GlIndicate *indicate)
{
	g_signal_emit(indicate,gl_indicate_signals[GL_INDICATE_CLICK_YES],0);
	gtk_widget_set_sensitive(indicate->ind_window,FALSE);
	gtk_widget_hide(indicate->ind_window);
}

void gl_indicate_click_no_button(GtkWidget *widget , GlIndicate *indicate)
{
	g_signal_emit(indicate,gl_indicate_signals[GL_INDICATE_CLICK_NO],0);
	gtk_widget_set_sensitive(indicate->ind_window,FALSE);
	gtk_widget_hide(indicate->ind_window);
}

static void
gl_indicate_start_click(GtkWidget *widget , GlIndicate *indicate)
{
	g_signal_emit(indicate,gl_indicate_signals[GL_INDICATE_CLICK_START],0);
	if(indicate->priv->user_ask > GL_INDICATE_NO_ASK)
	{
		gtk_widget_show(indicate->ind_window);
		gtk_widget_set_sensitive(indicate->ind_window,TRUE);
	}
}

void
gl_indicate_create_info ( GlIndicate *object )
{
	if(object->ind_box == NULL)
	{
		object->ind_box =  gtk_button_new ( );
		GtkWidget *hbox =  gtk_hbox_new(FALSE,1);
		gtk_container_add(GTK_CONTAINER(object->ind_box),hbox);
		object->priv->image = gtk_image_new();
		gtk_widget_set_size_request(object->priv->image,20,20);
		gtk_box_pack_start(GTK_BOX(hbox),object->priv->image,FALSE,FALSE,2);
		object->priv->user_i_box = gtk_hbox_new(TRUE,1);
		gtk_box_pack_start(GTK_BOX(hbox),object->priv->user_i_box,TRUE,TRUE,1);
		gtk_widget_show(object->priv->image);
		gtk_widget_show(object->priv->user_i_box);
		gtk_widget_show(hbox);
		gtk_widget_ref(object->ind_box);
		object->priv->is_i_box = FALSE;
		g_signal_connect(object->ind_box,"clicked",G_CALLBACK(gl_indicate_start_click),(gpointer) object);
	}

}


void
gl_indicate_create_window( GlIndicate *object )
{
	if(object->ind_window == NULL)
	{
		object->ind_window     = gtk_window_new(GTK_WINDOW_POPUP);
		gtk_window_set_modal(GTK_WINDOW(object->ind_window),TRUE);
		GtkWidget *viewport  = gtk_viewport_new(NULL,NULL);
		gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport),GTK_SHADOW_OUT);
		gtk_container_add(GTK_CONTAINER (object->ind_window),viewport);
		GtkWidget   *vbox      = gtk_vbox_new(FALSE,1);
		gtk_container_add(GTK_CONTAINER (viewport),vbox);
		gtk_window_set_position(GTK_WINDOW(object->ind_window),GTK_WIN_POS_CENTER);
		gtk_widget_set_size_request (object->ind_window, 500 , 300 );

		GtkWidget *scrolled_wid = g_object_new ( GTK_TYPE_SCROLLED_WINDOW,
				                                 "hscrollbar_policy",GTK_POLICY_AUTOMATIC,
				                                 "vscrollbar_policy",GTK_POLICY_AUTOMATIC,
				                                 "window_placement", GTK_CORNER_TOP_LEFT,
				                                  NULL);


		object->priv->user_fi_box  = gtk_viewport_new(NULL,NULL);
		gtk_container_add(GTK_CONTAINER(scrolled_wid),object->priv->user_fi_box);
		gtk_viewport_set_shadow_type(GTK_VIEWPORT(object->priv->user_fi_box),GTK_SHADOW_OUT);

		gtk_box_pack_start(GTK_BOX(vbox),scrolled_wid,TRUE,TRUE,1);

		object->priv->yesButton    = gtk_button_new();
		object->priv->noButton     = gtk_button_new();
		object->priv->yesImage     = gtk_image_new_from_file((const gchar*)object->priv->yesImage);
		object->priv->noImage      = gtk_image_new_from_file((const gchar*)object->priv->noImage);

		gtk_image_set_from_file(GTK_IMAGE(object->priv->yesImage),"/lar/gui/button_ok.png");
		gtk_image_set_from_file(GTK_IMAGE(object->priv->noImage),"/lar/gui/cancel.png");
		gtk_container_add(GTK_CONTAINER(object->priv->yesButton),object->priv->yesImage);
		gtk_container_add(GTK_CONTAINER(object->priv->noButton),object->priv->noImage);
		gtk_widget_show(object->priv->yesImage);
		gtk_widget_show(object->priv->noImage);

		g_signal_connect(object->priv->yesButton, "clicked" , G_CALLBACK(gl_indicate_click_yes_button), (gpointer) object );
		g_signal_connect(object->priv->noButton , "clicked" , G_CALLBACK(gl_indicate_click_no_button) , (gpointer) object );
		GtkWidget *hbox = gtk_hbox_new (FALSE,1);
		GtkWidget *platz = gtk_label_new("");
		gtk_box_pack_start(GTK_BOX(hbox),platz,FALSE,FALSE,1);
		gtk_widget_set_size_request (object->priv->yesButton,44,44);
		gtk_widget_set_size_request (object->priv->noButton ,44,44);
		gtk_box_pack_end(GTK_BOX(hbox),object->priv->yesButton,FALSE,FALSE,1);
		gtk_box_pack_end(GTK_BOX(hbox),object->priv->noButton,FALSE,FALSE,1);
		object->priv->is_fi_box = FALSE;
		gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,1);
		gtk_widget_show(object->priv->user_fi_box);
		gtk_widget_show(scrolled_wid);
		gtk_widget_show(hbox);
		gtk_widget_show(vbox);
		gtk_widget_show(viewport);
	}
}

void
gl_indicate_change_user_question( GlIndicate * indicate )
{
	if(indicate->priv->user_ask == GL_INDICATE_ASK_CONFIRM)
	{
		if(indicate->priv->yesButton)gtk_widget_show(indicate->priv->yesButton);
		if(indicate->priv->noButton) gtk_widget_show(indicate->priv->noButton);
	}
	else if(indicate->priv->user_ask == GL_INDICATE_ASK_INFO)
	{
		if(indicate->priv->yesButton)gtk_widget_show(indicate->priv->yesButton);
		if(indicate->priv->noButton) gtk_widget_hide(indicate->priv->noButton);
	}
	else
	{
		if(indicate->priv->yesButton)gtk_widget_hide(indicate->priv->yesButton);
		if(indicate->priv->noButton) gtk_widget_hide(indicate->priv->noButton);
	}
}


static void
gl_indicate_start_real ( GlIndicate *indicate )
{

}

static void
gl_indicate_init (GlIndicate *object)
{
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_INDICATE,GlIndicatePrivate);

	object->ind_window               = NULL;
	object->ind_box                  = NULL;
	object->ind_image                = NULL;


	object->priv->user_fi_box        = NULL;
	object->priv->yesButton          = NULL;
	object->priv->noButton           = NULL;

	object->isStart                  = FALSE;


	object->priv->image_counter      = 0;
	object->priv->image_prof         = g_strsplit_set("/lar/gui/help-hint.png",",",20);

	object->priv->remove             = TRUE;
	object->priv->user_ask           = 0;


	object->ind_image  = gtk_image_new();
	gtk_widget_ref(object->ind_image);
	gl_indicate_create_info   ( object );
	gl_indicate_create_window ( object );

}

static void
gl_indicate_finalize (GObject *object)
{
	GlIndicate *indicate = GL_INDICATE(object);
	g_strfreev(indicate->priv->image_prof );
	gtk_widget_unref(indicate->ind_image);
	gtk_widget_unref(indicate->ind_box);
	gtk_widget_unref(indicate->ind_image);
	gtk_widget_unref(indicate->ind_box);
	if(indicate->ind_window)gtk_widget_destroy(indicate->ind_window);
	G_OBJECT_CLASS (gl_indicate_parent_class)->finalize (object);
}


static void
gl_indicate_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail ( GL_INDICATE (object));
	GlIndicate* indicate = GL_INDICATE(object);
	switch (prop_id)
	{
	case PROP_INDICATE_USER_QUESTION:
		/* TODO: Add setter for "user_question" property here */
		indicate->priv->user_ask = g_value_get_uint(value);
		gl_indicate_change_user_question(indicate);
		break;
	case PROP_INDICATE_USER_YES_IMAGE:
		/* TODO: Add setter for "user_question" property here */
		gtk_image_set_from_file(GTK_IMAGE(indicate->priv->yesImage), g_value_get_string(value));
		break;
	case PROP_INDICATE_USER_NO_IMAGE:
		/* TODO: Add setter for "user_question" property here */
		gtk_image_set_from_file(GTK_IMAGE(indicate->priv->noImage), g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
	//printf("Set (GI_INDICATE) property end\n");
}

static void
gl_indicate_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail ( GL_INDICATE (object));
	GlIndicate* indicate = GL_INDICATE(object);
	//printf("Set (GL_INDICATE) property \n");
	switch (prop_id)
	{
	case PROP_INDICATE_USER_QUESTION:
		/* TODO: Add setter for "user_question" property here */
		 g_value_set_uint ( value,indicate -> priv->user_ask );
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
	//printf("Set (GL_INDICATE) property end\n");
}

static void
gl_indicate_class_init (GlIndicateClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GlIndicatePrivate));

	object_class->finalize      = gl_indicate_finalize;
	object_class->set_property  = gl_indicate_set_property;
	object_class->get_property  = gl_indicate_get_property;


	klass   -> click_yes      = NULL;
	klass   -> click_no       = NULL;
	klass   -> click_start    = NULL;
	klass   -> start_indicate = NULL;
	klass   -> stop_indicate  = NULL;

	GParamSpec *pspec;

	pspec = g_param_spec_uint("user-ask",
			"Indicate user question",
			"Set indicate user question",
			GL_INDICATE_NO_ASK,GL_INDICATE_ASK_CONFIRM,GL_INDICATE_NO_ASK,
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property (object_class,
			PROP_INDICATE_USER_QUESTION,pspec);

	pspec = g_param_spec_string("yes-image",
			"Indicate user question",
			"Set indicate user question",
			"/lar/gui/button_ok.png",
			G_PARAM_WRITABLE);
	g_object_class_install_property (object_class,
			PROP_INDICATE_USER_YES_IMAGE,pspec);

	pspec = g_param_spec_string("no-image",
			"Indicate user question",
			"Set indicate user question",
			"/lar/gui/cancel.png",
			G_PARAM_WRITABLE);
	g_object_class_install_property (object_class,
			PROP_INDICATE_USER_NO_IMAGE,pspec);

	gl_indicate_signals[GL_INDICATE_CLICK_START] =
			g_signal_new ("click_start",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlIndicateClass, click_start),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	gl_indicate_signals[GL_INDICATE_CLICK_YES] =
			g_signal_new ("click_yes",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlIndicateClass, click_yes),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	gl_indicate_signals[GL_INDICATE_CLICK_NO] =
			g_signal_new ("click_no",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlIndicateClass, click_no),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	gl_indicate_signals[GL_INDICATE_START_INDICATE] =
			g_signal_new ("start_indicate",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlIndicateClass, start_indicate),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	gl_indicate_signals[GL_INDICATE_STOP_INDICATE] =
			g_signal_new ("stop_indicate",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlIndicateClass, stop_indicate),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

}

static void
gl_indicate_update_image ( GlIndicate *indicate  )
{
	//g_printf("gl_indicate_update_image\n");
	g_return_if_fail ( GL_INDICATE (indicate));
	if(indicate->priv->image_prof != NULL)
	{
		if(indicate->priv->image_prof[indicate->priv->image_counter] != NULL)
		{
			//g_printf("gl_indicate_set_image= %d %s\n",indicate->priv->image_counter,indicate->priv->image_prof[indicate->priv->image_counter]);
			gtk_image_set_from_file(GTK_IMAGE(indicate->priv->image),(const gchar*)indicate->priv->image_prof[indicate->priv->image_counter]);
			gtk_image_set_from_file(GTK_IMAGE(indicate->ind_image),(const gchar*)indicate->priv->image_prof[indicate->priv->image_counter]);
			indicate->priv->image_counter++;
		}
		else
		{
			indicate->priv->image_counter = 0;
			if(indicate->priv->image_prof[indicate->priv->image_counter] != NULL)
			{
				//g_printf("gl_indicate_set_image= %d %s\n",indicate->priv->image_counter,indicate->priv->image_prof[indicate->priv->image_counter]);
				gtk_image_set_from_file(GTK_IMAGE(indicate->priv->image),(const gchar*)indicate->priv->image_prof[indicate->priv->image_counter]);
				gtk_image_set_from_file(GTK_IMAGE(indicate->ind_image),(const gchar*)indicate->priv->image_prof[indicate->priv->image_counter]);
				indicate->priv->image_counter++;
			}
		}

	}
}

void
gl_indicate_update   ( GlIndicate *indicate )
{
	g_return_if_fail (indicate!=NULL);
	g_return_if_fail ( GL_INDICATE (indicate));
	if(indicate->isStart)
	{
		gl_indicate_update_image(indicate);
	}

}

void
gl_indicate_start    ( GlIndicate *indicate )
{
	g_return_if_fail (indicate!=NULL);
	g_return_if_fail ( GL_INDICATE (indicate));
	gtk_widget_show(indicate->ind_box);
	gtk_widget_set_sensitive(indicate->ind_box,TRUE);
	gtk_widget_show(indicate->ind_image);
	gtk_widget_set_sensitive(indicate->ind_image,TRUE);
	indicate->isStart = TRUE;
	indicate->priv->image_counter = 0;
	gl_indicate_update_image ( indicate  );
	g_signal_emit(indicate,gl_indicate_signals[GL_INDICATE_START_INDICATE],0);

}
void
gl_indicate_stop     ( GlIndicate *indicate )
{
	g_return_if_fail (indicate!=NULL);
	g_return_if_fail ( GL_INDICATE (indicate));
	gtk_widget_hide(indicate->ind_box);
	gtk_widget_set_sensitive(indicate->ind_box,FALSE);
	gtk_widget_hide(indicate->ind_image);
	gtk_widget_set_sensitive(indicate->ind_image,FALSE);
	gtk_widget_hide(indicate->ind_window);
	gtk_widget_set_sensitive(indicate->ind_window,FALSE);
	indicate->isStart = FALSE;
	g_signal_emit(indicate,gl_indicate_signals[GL_INDICATE_STOP_INDICATE],0);
}

void
gl_indicate_set_indicate_profile (GlIndicate *indicate , gchar *images  )
{
	g_return_if_fail ( GL_INDICATE (indicate));
	g_return_if_fail ( images != NULL );
	g_strfreev(indicate->priv->image_prof);
	indicate->priv->image_prof  = g_strsplit_set(images,",",20);
	indicate->priv->image_counter = 0;
}

void
gl_indicate_set_indicate_box     (GlIndicate *indicate , GtkWidget *widget )
{
	g_return_if_fail ( GL_INDICATE (indicate));
	if(NULL == gtk_container_get_children(GTK_CONTAINER(indicate->priv->user_i_box)))
	{
		gtk_box_pack_start(GTK_BOX(indicate->priv->user_i_box),widget,TRUE,TRUE,1);
		indicate->priv->is_i_box = TRUE;
	}
}
void
gl_indicate_set_indicate_window  (GlIndicate *indicate , GtkWidget *widget )
{
	g_return_if_fail ( GL_INDICATE (indicate));
	if(NULL == gtk_container_get_children(GTK_CONTAINER(indicate->priv->user_fi_box)))
	{
		gtk_container_add(GTK_CONTAINER(indicate->priv->user_fi_box),widget);
		gint w,h;
		gtk_widget_get_size_request(widget,&w,&h);
		if(h>0 && h<300)
		{
			gtk_widget_set_size_request(indicate->ind_window,500,h+100);
			gtk_window_resize(GTK_WINDOW(indicate->ind_window),500,h+100);

		}
		indicate->priv->is_fi_box = TRUE;
	}
}

gboolean
gl_indicate_is_user_info ( GlIndicate *indicate )
{
	g_return_val_if_fail ( indicate!=NULL ,FALSE);
	g_return_val_if_fail ( GL_INDICATE (indicate),FALSE);
	return indicate->isStart && indicate->priv->is_i_box;
}

GlIndicate*
gl_indicate_new      ( const gchar *id , guint user_ask )
{
	return GL_INDICATE(mkt_atom_object_new( GL_TYPE_INDICATE,MKT_ATOM_PN_ID,id,"user-ask",user_ask,NULL));
}


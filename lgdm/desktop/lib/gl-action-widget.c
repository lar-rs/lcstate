/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * src
 * Copyright (C) sascha 2012 <sascha@sascha-desktop>
 * 
src is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * src is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl-action-widget.h"
#include "gl-draganddrop.h"
#include "gl-level-manager.h"
#include "gl-connection.h"
#include "mkt-collector.h"
#include "gl-widget-option.h"
#include <stdlib.h>

enum {
	PROP_ACTION_WIDGET_NULL,
	PROP_ACTION_WIDGET_PATH,
	PROP_ACTION_WIDGET_NICK,
	PROP_ACTION_WIDGET_TEXT,
	PROP_ACTION_WIDGET_ACTIVE,
	PROP_ACTION_WIDGET_SENSETIVE,
	PROP_ACTION_WIDGET_MOVE,
	PROP_ACTION_WIDGET_LEVEL,
	PROP_ACTION_WIDGET_PLACE,
	PROP_ACTION_WIDGET_PARENT,
	PROP_ACTION_WIDGET_POSX,
	PROP_ACTION_WIDGET_POSY,
	PROP_ACTION_WIDGET_ICON,
	PROP_ACTION_WIDGET_FONT,
};


struct _GlActionWidgetPrivate
{
	GtkWidget  *action_widget;
	GtkWidget*  image;
	GtkWidget*  label;
	GtkWidget  *vbox;

	gboolean    IS_LOAD;

	gchar      *nick;
	gboolean    active;
	gboolean    sensetive;
	gboolean    is_move;
	gchar      *place;
	gchar      *icon;
	gchar      *font;
	gchar      *text;
	gint        posX;
	gint        posY;
};

enum
{
	GL_ACTION_START,
	LAST_SIGNAL
};


static guint gl_action_signals[LAST_SIGNAL] = { 0 };


#define GL_ACTION_WIDGET_MAIN "Action"

#define GL_ACTION_WIDGET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_ACTION_WIDGET, GlActionWidgetPrivate))



static void          gl_action_widget_redraw ( GlActionWidget *widget );


G_DEFINE_TYPE (GlActionWidget, gl_action_widget, MKT_TYPE_WINDOW );






static void
gl_action_widget_set_drag_source ( GlActionWidget *widget )
{
	if(!mkt_window_is_realized(MKT_WINDOW(widget))) return;
	g_return_if_fail( mkt_window_is_realized(MKT_WINDOW(widget)) );
	gtk_drag_source_unset (GTK_WIDGET(widget->priv->action_widget));
	if( widget->priv->active  )
	{
		if( widget->priv->is_move )
		gtk_drag_source_set (GTK_WIDGET(widget->priv->action_widget), GDK_BUTTON1_MASK | GDK_BUTTON3_MASK,
				gl_drag_and_drop_get_source_list(mktAPGet(widget,"level",uint)),gl_drag_and_drop_get_n_sources(mktAPGet(widget,"level",uint)),
				GDK_ACTION_COPY | GDK_ACTION_MOVE);
	}
}

static void
gl_action_widget_drag_and_drop_changed_cb ( GlBinding *binding , GlActionWidget *action )
{
	//TEST:
/*	const mktItem_t *it = gl_binding_get_input_item(binding);
	if(it && it->type == MKT_ITEM_TYPE_bool32)
	{
		//TEST:
		g_debug ("TEST DRAG AND DROP item %d",it->value.bool32);
		mktAPSet(action,"active",it->value.bool32);
	}*/
}

// Drag and Drop signals --------------------------------------------------

static void
gl_action_widget_drag_begin(GtkWidget *widget, GdkDragContext *dc, gpointer data)
{
	//TEST:g_debug("action drag begin");
	//GdkScreen *screen     = gtk_widget_get_screen(widget);
    //GdkVisual *visual     = gdk_screen_get_rgba_visual (screen);

    //GdkColormap *colormap = gdk_screen_get_default_colormap(screen);
	//cr = gdk_cairo_create (pixmap)
	//GdkPixbuf  *pixbuf    = gdk_pixbuf_get_from_drawable(NULL,GDK_DRAWABLE(widget),colormap,widget->allocation.x,widget->allocation.y,0,0,widget->allocation.width,widget->allocation.height);
	//GdkPixbuf  *npixbuf = gdk_pixbuf_copy(pixbuf);
	/*gtk_drag_set_icon_pixbuf(dc,
			pixbuf,
			 widget->allocation.width/2,
			 widget->allocation.height/2);*/

}
static gboolean
gl_action_widget_drag_motion(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, guint t, gpointer data)
{
	//TEST:g_debug("action drag begin motion\n");
	return TRUE;
}
static void
gl_action_widget_drag_data_get(GtkWidget *widget, GdkDragContext *dc, GtkSelectionData *selection_data, guint info, guint t, gpointer data)
{
	//TEST:g_debug("action drag begin data get\n");
}
static void
gl_action_widget_drag_data_delete(GtkWidget *widget, GdkDragContext *dc, gpointer data)
{
	//TEST:g_debug("action drag begin data delete\n");
}
static gboolean
gl_action_widget_drag_drop(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, guint t, gpointer data)
{
	//TEST:g_debug("action drag begin drop\n");
	return TRUE;
}
static void
gl_action_widget_drag_end(GtkWidget *widget, GdkDragContext *dc, gpointer data)
{
	//TEST:g_debug("action drag begin end\n");
}

static gboolean
gl_action_widget_button_clicked_cb( GtkWidget *widget , GlActionWidget *action )
{
	g_return_val_if_fail(action!= NULL,FALSE);
	g_return_val_if_fail(GL_IS_ACTION_WIDGET(action),FALSE);
	//TEST:g_debug("gl_action_widget_button_clicked_cb");
	g_signal_emit(action,gl_action_signals[GL_ACTION_START],0);
	return TRUE;
}


static void
gl_action_widget_realize ( MktWindow *win )
{
	GlActionWidget *widget = GL_ACTION_WIDGET(win);

//#ifdef IF_GTK_LIBRARY
	widget->priv->action_widget = gtk_button_new();
	widget->priv->vbox          = gtk_box_new(GTK_ORIENTATION_VERTICAL,1);
	widget->priv->image         = gtk_image_new_from_file( widget->priv->icon );
	gtk_box_pack_start(GTK_BOX(widget->priv->vbox),widget->priv->image,TRUE,TRUE,0);
	gtk_widget_show(widget->priv->image);
	gchar *name = g_strdup_printf("TRANSLATE_label_Name_%s",mkt_atom_get_id(MKT_ATOM(win)));
	widget->priv->label = gtk_label_new(mkt_atom_get_id(MKT_ATOM(win)));
	gl_widget_option_set_name(G_OBJECT(widget->priv->label),name);
	g_free(name);
	gl_binding_insert_widget(widget->priv->label);
	gtk_widget_override_font(GTK_WIDGET(widget->priv->label),pango_font_description_from_string(mktAPGet(widget,"font",string)));
	gtk_box_pack_start(GTK_BOX(widget->priv->vbox),widget->priv->label,FALSE,FALSE,0);
	gtk_widget_show(widget->priv->label);
	gtk_container_add(GTK_CONTAINER(widget -> priv -> action_widget ) ,widget->priv->vbox );
	gtk_widget_show(widget->priv->vbox);
	gtk_widget_set_sensitive(GTK_WIDGET(widget -> priv -> action_widget ),widget->priv->sensetive );
	gtk_widget_show(widget -> priv -> action_widget);
	g_signal_connect(widget -> priv -> action_widget,"clicked",G_CALLBACK(gl_action_widget_button_clicked_cb),win);

	g_signal_connect (widget -> priv -> action_widget, "drag_begin",
			G_CALLBACK (gl_action_widget_drag_begin), widget);
	g_signal_connect (widget -> priv -> action_widget, "drag_motion",
			G_CALLBACK (gl_action_widget_drag_motion), widget);
	g_signal_connect (widget -> priv -> action_widget, "drag_data_get",
			G_CALLBACK (gl_action_widget_drag_data_get), widget);
	g_signal_connect (widget -> priv -> action_widget, "drag_data_delete",
			G_CALLBACK (gl_action_widget_drag_data_delete), widget);
	g_signal_connect (widget -> priv -> action_widget, "drag_drop",
			G_CALLBACK (gl_action_widget_drag_drop), widget);
	g_signal_connect (widget -> priv -> action_widget, "drag_end",
			G_CALLBACK (gl_action_widget_drag_end), widget);
	mktAPSet ( win , "position-type" , MKT_WINDOW_POS_ACTION);
	//gl_connection_connect_binding_signal("parameter___drag_and_drop",G_CALLBACK(gl_action_widget_drag_and_drop_changed_cb) ,widget);
	gtk_drag_source_unset (GTK_WIDGET(widget->priv->action_widget));
	mkt_window_set_gtk_container ( MKT_WINDOW(win),widget -> priv -> action_widget );
	if( widget->priv->active  )
	{
		if( widget->priv->is_move )
			gtk_drag_source_set (GTK_WIDGET(widget->priv->action_widget), GDK_BUTTON1_MASK | GDK_BUTTON3_MASK,
					gl_drag_and_drop_get_source_list(mktAPGet(widget,"level",uint)),gl_drag_and_drop_get_n_sources(mktAPGet(widget,"level",uint)),
					GDK_ACTION_COPY | GDK_ACTION_MOVE);
	}

//#endif
}

static void
gl_action_widget_init (GlActionWidget *object)
{
	object -> priv = G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_ACTION_WIDGET,GlActionWidgetPrivate);
	object -> priv -> nick      = g_strdup("none");
	object -> priv -> active    = TRUE;
	object -> priv -> is_move   = TRUE;
	object -> priv -> text      = g_strdup("---");
	object -> priv -> place     = g_strdup("search");
	object -> priv -> icon      = g_strdup("default");
	object -> priv -> font      = g_strdup("default");
	object -> priv -> posX      = 0;
	object -> priv -> posY      = 0;
	object -> priv -> sensetive = TRUE;
	object -> priv -> IS_LOAD   = FALSE;
	object -> priv -> action_widget = NULL;
}

static void
gl_action_widget_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	GlActionWidget *widget = GL_ACTION_WIDGET(object);
	//TEST:g_debug("PATH %s;delim=%s;Nick=%s",widget->priv->path,path_delim,widget -> priv -> nick);
	gtk_widget_destroy( widget -> priv -> action_widget );
	if(widget -> priv -> nick)  g_free(widget -> priv -> nick);
	if(widget -> priv -> text)  g_free(widget -> priv -> text);
	if(widget -> priv -> place) g_free(widget -> priv -> place);
	if(widget -> priv -> icon)  g_free(widget -> priv -> icon);
	if(widget -> priv -> font)  g_free(widget -> priv -> font);
	mkt_atom_remove_file(MKT_ATOM(widget));
	G_OBJECT_CLASS (gl_action_widget_parent_class)->finalize (object);
}

static void
gl_action_widget_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_ACTION_WIDGET (object));
	GlActionWidget* widget = GL_ACTION_WIDGET(object);
	//g_debug ("Set Property ...id%d - %s ",prop_id,g_param_spec_get_name(pspec));
	switch (prop_id)
	{
	case PROP_ACTION_WIDGET_NICK:
		if(widget -> priv -> nick ) g_free(widget -> priv -> nick );
		widget -> priv -> nick     = g_value_dup_string (value);
		break;
	case PROP_ACTION_WIDGET_TEXT:
		if(widget -> priv -> text )g_free(widget -> priv ->text);
		widget -> priv ->text     = g_value_dup_string (value);
		gl_action_widget_redraw(widget);
		break;
	case PROP_ACTION_WIDGET_ACTIVE:
		widget -> priv ->active = g_value_get_boolean(value);
		gl_action_widget_set_drag_source(widget);
		break;
	case PROP_ACTION_WIDGET_SENSETIVE:
		widget->priv->sensetive =  g_value_get_boolean(value);
		gl_action_widget_redraw(widget);
		break;
	case PROP_ACTION_WIDGET_MOVE:
		widget -> priv ->is_move = g_value_get_boolean(value);
		gl_action_widget_set_drag_source(widget);
		break;
	case PROP_ACTION_WIDGET_PLACE:
		if(widget -> priv ->place)g_free(widget -> priv ->place);
		widget -> priv ->place = g_value_dup_string(value);
		break;
	case PROP_ACTION_WIDGET_ICON: // widget redraw
		if(widget -> priv ->icon)g_free(widget -> priv ->icon);
		widget -> priv ->icon = mkt_atom_build_path(MKT_ATOM(widget),g_value_get_string(value));
		//TEST:g_debug("Test .. Action Icon %s",widget -> priv ->icon);
		gl_action_widget_redraw(widget);
		break;
	case PROP_ACTION_WIDGET_FONT: // widget redraw
		if(widget -> priv ->font)g_free(widget -> priv ->font);
		widget -> priv ->font = g_value_dup_string(value);
		gl_action_widget_redraw(widget);
		break;
	case PROP_ACTION_WIDGET_POSX:
		widget -> priv -> posX = g_value_get_uint(value);
		break;
	case PROP_ACTION_WIDGET_POSY:
		widget -> priv -> posY = g_value_get_uint(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}
static void
gl_action_widget_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_ACTION_WIDGET (object));
	GlActionWidget* widget = GL_ACTION_WIDGET(object);
	switch (prop_id)
	{
	case PROP_ACTION_WIDGET_NICK:
		g_value_set_string(value,widget->priv->nick);
		break;
	case PROP_ACTION_WIDGET_ACTIVE:
		g_value_set_boolean(value,widget->priv->active);
		break;
	case PROP_ACTION_WIDGET_SENSETIVE:
		g_value_set_boolean(value,widget->priv->sensetive);
		break;
	case PROP_ACTION_WIDGET_TEXT:
		g_value_set_string(value,widget -> priv ->text);
		break;
	case PROP_ACTION_WIDGET_PLACE:
		g_value_set_string(value,widget->priv->place);
		break;
	case PROP_ACTION_WIDGET_ICON:
		g_value_set_string(value,widget->priv->icon);
		break;
	case PROP_ACTION_WIDGET_FONT:
		g_value_set_string(value,widget->priv->font);
		break;
	case PROP_ACTION_WIDGET_MOVE:
		g_value_set_boolean(value,widget->priv->is_move);
		break;
	case PROP_ACTION_WIDGET_POSX:
		g_value_set_uint(value,widget -> priv ->posX );
		break;
	case PROP_ACTION_WIDGET_POSY:
		g_value_set_uint(value,widget -> priv ->posY );
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_action_widget_class_init ( GlActionWidgetClass *klass )
{
	GObjectClass*   object_class     =  G_OBJECT_CLASS (klass);
	MktWindowClass* parent_class     =  MKT_WINDOW_CLASS (klass);

	object_class -> set_property     =  gl_action_widget_set_property;
	object_class -> get_property     =  gl_action_widget_get_property;
	object_class -> finalize         =  gl_action_widget_finalize;

	parent_class -> realize          =  gl_action_widget_realize;

	klass->action_start              =  NULL;


	g_type_class_add_private (klass, sizeof (GlActionWidgetPrivate));

	GParamSpec *pspec;

	pspec = g_param_spec_string ("nick",
			"Action widget nick",
			"Set/Get module widget place",
			"action",
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY );
	g_object_class_install_property (object_class,
			PROP_ACTION_WIDGET_NICK,pspec);

	pspec = g_param_spec_string ("text",
				"Action widget text",
				"Set/Get Action widget text",
				"action",
				G_PARAM_READABLE | G_PARAM_WRITABLE | MKT_PARAM_SAVE );
		g_object_class_install_property (object_class,
				PROP_ACTION_WIDGET_TEXT,pspec);

	pspec = g_param_spec_boolean("active" ,
			"Action Drag and Drop",
			"Set/Get Drag and Drop",
			TRUE,
			G_PARAM_READABLE | G_PARAM_WRITABLE | MKT_PARAM_SAVE );
	g_object_class_install_property (object_class,
			PROP_ACTION_WIDGET_ACTIVE,pspec);

	pspec = g_param_spec_boolean("sensetive" ,
			"Plugin sensetive",
			"Set/Get plugin sensetive",
			TRUE,
			G_PARAM_READABLE | G_PARAM_WRITABLE | MKT_PARAM_SAVE  );
	g_object_class_install_property (object_class,
			PROP_ACTION_WIDGET_SENSETIVE,pspec);

	pspec = g_param_spec_boolean("move",
				"Action move",
				"Set/Get action move",
				TRUE,
				G_PARAM_READABLE  | G_PARAM_WRITABLE | MKT_PARAM_SAVE );
		g_object_class_install_property (object_class,
				PROP_ACTION_WIDGET_MOVE,pspec);

	pspec = g_param_spec_string ("place",
			"Plugin widget place",
			"Set/Get module widget place",
			"search",
			G_PARAM_READABLE | G_PARAM_WRITABLE | MKT_PARAM_SAVE );
	g_object_class_install_property (object_class,
			PROP_ACTION_WIDGET_PLACE,pspec);

	pspec = g_param_spec_uint("posX",
			"Plugin widget position",
			"Set/Get widget position",
			0,G_MAXUINT32,0,
			G_PARAM_READABLE | G_PARAM_WRITABLE | MKT_PARAM_SAVE );
	g_object_class_install_property (object_class,
			PROP_ACTION_WIDGET_POSX,pspec);

	pspec = g_param_spec_uint("posY",
			"Plugin widget position",
			"Set/Get widget position",
			0,G_MAXUINT32,0,
			G_PARAM_READABLE | G_PARAM_WRITABLE | MKT_PARAM_SAVE );
	g_object_class_install_property (object_class,
			PROP_ACTION_WIDGET_POSY,pspec);

	pspec = g_param_spec_string("icon",
			"Plugin widget size",
			"Set/Get widget size",
			"default",
			G_PARAM_READABLE | G_PARAM_WRITABLE | MKT_PARAM_SAVE );
	g_object_class_install_property (object_class,
			PROP_ACTION_WIDGET_ICON,pspec);

	pspec = g_param_spec_string("font",
			"Plugin widget size",
			"Set/Get widget size",
			"default",
			G_PARAM_READABLE | G_PARAM_WRITABLE | MKT_PARAM_SAVE );
	g_object_class_install_property (object_class,
			PROP_ACTION_WIDGET_FONT,pspec);

	gl_action_signals[GL_ACTION_START] =
				g_signal_new ("start_action",
						G_TYPE_FROM_CLASS (klass),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET ( GlActionWidgetClass , action_start ),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


void
gl_action_widget_redraw ( GlActionWidget *widget )
{
	if(!mkt_window_is_realized(MKT_WINDOW(widget))) return;
	//TEST:g_return_if_fail( mkt_window_is_realized(MKT_WINDOW(widget)) );
	gtk_image_set_from_file( GTK_IMAGE(widget->priv-> image) ,mktAPGet( widget,"icon",string ) );
	gtk_widget_show(widget->priv->image);
	gtk_widget_override_font(GTK_WIDGET(widget->priv->label),pango_font_description_from_string(mktAPGet(widget,"font",string)));
	gtk_widget_show(widget->priv->label);
	gtk_widget_set_sensitive(GTK_WIDGET(widget -> priv -> action_widget),widget->priv->sensetive);
}


MktAtom*
gl_action_widget_new_for_parent ( MktWindow *window , const gchar *id ,const gchar *path ,const gchar *nick,  gboolean active  )
{

	MktAtom *action = mkt_collector_get_atom_static(id);
	if( action == NULL )
	{
		action = mkt_atom_object_new (GL_TYPE_ACTION_WIDGET,MKT_ATOM_PN_ID ,id,
				                                            MKT_ATOM_PN_PATH, path,
				                                         	"nick",nick,
				                                            "active",active, NULL);

		mktAPSet(action,"level", mktAPGet(window,"level",uint));
		//TEST:g_debug("gl_action_widget_new_for_parent action %s",mkt_atom_get_id(action));
		return action;
	}
	return NULL;
}

MktAtom*
gl_action_creat_copy  ( GlActionWidget *action , const gchar *new_id , const gchar *place )
{
	g_return_val_if_fail(action!=NULL,NULL);

	if(mkt_collector_get_atom_static(new_id)!=NULL)
	{
		g_warning("Action atom id %s redefined",new_id);
		return NULL;
	}
	mkt_atom_copy_file (MKT_ATOM(action), new_id );
	MktAtom *copy = mkt_atom_object_new ( GL_TYPE_ACTION_WIDGET,MKT_ATOM_PN_ID,new_id,
			                                                    MKT_ATOM_PN_PATH,mkt_atom_get_path(MKT_ATOM(action)),
			                                                    "place",place,NULL);
	//mkt_atom_save(copy);
	//TEST:g_debug("gl_action_creat_copy action %s",mkt_atom_get_id(copy));
	return copy;
}


const gchar*
gl_action_widget_get_nick  ( GlActionWidget *action )
{
	g_return_val_if_fail(action != NULL ,NULL);
	g_return_val_if_fail(GL_IS_ACTION_WIDGET(action)  ,NULL);
	return action->priv->nick;
}

const gchar*
gl_action_widget_get_place  ( GlActionWidget *action )
{
	g_return_val_if_fail(action != NULL ,NULL);
	g_return_val_if_fail(GL_IS_ACTION_WIDGET(action)  ,NULL);
	return action->priv->place;
}

void
gl_action_widget_set_sensitive             ( GlActionWidget *action, gboolean sensitive)
{
	g_return_if_fail(action!=NULL);
	g_return_if_fail(GL_ACTION_WIDGET(action));
	if(sensitive!= action->priv->sensetive) mktAPSet(action,"sensetive",sensitive);
//	if(action -> priv -> action_widget)	gtk_widget_set_sensitive(action -> priv -> action_widget,sensitive);
}



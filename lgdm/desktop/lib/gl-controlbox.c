/*
 * gtklarcontrolbox.c
 *
 *  Created on: 12.10.2010
 *      Author: asmolkov
 */




#include "gl-controlbox.h"
#include "gl-plugin.h"
#include "gl-draganddrop.h"
#include "gl-action-widget.h"
#include "gl-xkbd.h"

#include <mkt-collector.h>



static gboolean gl_control_box_open_close_signal_clicked(GtkWidget *button,GlControlBox *ctrlbox);
static gboolean gl_control_box_viewport_signal_changed(GtkWidget *button,GlControlBox *ctrlbox);
static gboolean gl_control_box_open_close_xvkbd_clicked(GtkWidget *button,GtkWidget *keybutton);


static gboolean gl_control_box_left_right_signal_relased(GtkWidget *button,GdkEventButton *event,GlControlBox *ctrlbox);
static gboolean gl_control_box_left_signal_pressed(GtkWidget *button,GdkEventButton *event,GlControlBox *ctrlbox);
static gboolean gl_control_box_right_signal_pressed(GtkWidget *button,GdkEventButton *event,GlControlBox *ctrlbox);

static gboolean gl_control_box_repack_action (  GlControlBox *ctrlbox , MktWindow *action , gboolean expand , gboolean fill , gint padding );

G_DEFINE_TYPE ( GlControlBox, gl_control_box, MKT_TYPE_WINDOW );

enum {
  OPEN_CONTROL_BOX_SIGNAL,
  CLOSE_CONTROL_BOX_SIGNAL,
  CLICK_CONTROL_BOX_BUTTON_SIGNAL,
  LAST_SIGNAL
};


enum {
	PROP_CONTROL_BOX0,
	PROP_CONTROL_BOX_DIRECTION
};

struct _GlControlBoxPrivate
{
	GlControBoxDirectionType  dType;

#ifdef IF_GTK_LIBRARY
	GtkWidget                *window;
#endif
};


static GlControBoxDirectionType __last_direction;

// ---------------------------------------------------------------------------------------------------------------

static gboolean __key_button_press = FALSE;



gboolean
__key_button_up_press(gpointer data)
{

	GtkAdjustment *adj = NULL;
	if(!GL_IS_CONTROL_BOX(data ))
		return FALSE;
	GlControlBox *ctrlbox = GL_CONTROL_BOX(data);
	if(ctrlbox->priv->dType == CONTROL_BOX_HBOX_DIRECTION)
		adj = gtk_viewport_get_hadjustment(GTK_VIEWPORT(ctrlbox->viewport));
	else
		adj = gtk_viewport_get_vadjustment(GTK_VIEWPORT(ctrlbox->viewport));
	gdouble        val;
    val = gtk_adjustment_get_value(adj);
    val -= adj-> step_increment;
    if( val < adj-> lower )
    {
    	val = adj-> lower;
    	gtk_widget_set_sensitive(ctrlbox->inRecht,TRUE);
    	gtk_widget_set_sensitive(ctrlbox->inLink,FALSE);
    }
    else
    {
    	gtk_widget_set_sensitive(ctrlbox->inRecht,TRUE);
    	gtk_widget_set_sensitive(ctrlbox->inLink,TRUE);
    }
    gtk_adjustment_set_value(adj,val);
   // g_print("Page=%.3f;Upper=%.3f;Vla=%.3f;Lover=%.3f;PageInc=%.3f\n",adj->page_size,adj->upper,
   // 		adj->value,adj->lower,adj->page_increment);
	return __key_button_press;
}


gboolean
__key_button_down_press(gpointer data)
{
	GtkAdjustment *adj = NULL;
	if(!GL_IS_CONTROL_BOX(data ))
		return FALSE;
	GlControlBox *ctrlbox = GL_CONTROL_BOX(data);
	if(ctrlbox->priv->dType == CONTROL_BOX_HBOX_DIRECTION)
		adj = gtk_viewport_get_hadjustment(GTK_VIEWPORT(ctrlbox->viewport));
	else
		adj = gtk_viewport_get_vadjustment(GTK_VIEWPORT(ctrlbox->viewport));
	gdouble        val;
    val = gtk_adjustment_get_value(adj);
    val += adj-> step_increment;
    if( (val + adj-> page_size) > (adj-> upper) )
    {
      	val = adj-> upper - adj-> page_size;
      	gtk_widget_set_sensitive(ctrlbox->inRecht,FALSE);
      	gtk_widget_set_sensitive(ctrlbox->inLink,TRUE);
    }
    else
    {
    	gtk_widget_set_sensitive(ctrlbox->inRecht,TRUE);
    	gtk_widget_set_sensitive(ctrlbox->inLink,TRUE);
    }

   // g_print("Page=%.3f;Upper=%.3f;Vla=%.3f;Lover=%.3f;PageInc=%.3f\n",adj->page_size,adj->upper,
    //		adj->value,adj->lower,adj->page_increment);
    gtk_adjustment_set_value(adj,val);
   	return __key_button_press;
}
// ---------------------------------------------------------------------------------------------------------------


gint
gl_control_box_get_childs (GlControlBox *box)
{
	gint ret = 0;
	GList *childs = gtk_container_get_children(GTK_CONTAINER(box->ihbox));
	while(childs)
	{
		ret++;
		childs = childs->next;
	}
	return ret;
}
// ----------------------DragAndDrop--------------------------------------

gboolean
gl_control_box_drag_motion(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, guint t, gpointer data)
{
	//g_printf("gl_main_build_drag_motion\n");
	return FALSE;

}
gboolean
gl_control_box_drag_data_delete(GtkWidget *widget, GdkDragContext *dc, gpointer data)
{
	return FALSE;
}
gboolean
gl_control_box_drag_drop(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, guint t, gpointer data)
{
	g_return_val_if_fail(data!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_CONTROL_BOX(data),FALSE);
	//TEST:g_debug( "Test drag and drop par x = %d y = %d t=%d",x,y,t);
	GtkWidget *wi = gtk_drag_get_source_widget(dc);
	g_return_val_if_fail(wi!=NULL,FALSE);
	if(gl_control_box_get_childs(GL_CONTROL_BOX(data))<GL_CONTROL_BOX(data)->max_widget )
	{
		MktWindow *win = mkt_window_gtk_widget_get_parent(wi);
		if( win && !g_str_has_suffix(mkt_atom_get_id(MKT_ATOM(win)),":copy"))
		{
			gchar *new_id = g_strdup_printf("%s:copy",mkt_atom_get_id(MKT_ATOM(win)));
			MktAtom *new_action = mkt_collector_get_atom_static(new_id);
			if(new_action == NULL )
			{
				new_action = gl_action_creat_copy(GL_ACTION_WIDGET(win),new_id,mkt_atom_get_id(MKT_ATOM(data)));
				mktAPSet(new_action,"place",mkt_atom_get_id(MKT_ATOM(data)));
				gl_control_box_repack_action(GL_CONTROL_BOX(data),MKT_WINDOW(new_action),FALSE,FALSE,1 );

			}
			else if(MKT_IS_WINDOW(new_action))
			{
				mktAPSet(new_action,"place",mkt_atom_get_id(MKT_ATOM(data)));
				gl_control_box_repack_action(GL_CONTROL_BOX(data),MKT_WINDOW(new_action),FALSE,FALSE,1 );
			}
			g_free(new_id);
		}
		else
		{
			mktAPSet(win,"place",mkt_atom_get_id(MKT_ATOM(data)));
			gl_control_box_repack_action(GL_CONTROL_BOX(data),MKT_WINDOW(win),FALSE,FALSE,1 );
		}
	}
	return  FALSE;
}

void
gl_control_box_drag_data_received(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, gpointer data)
{
}

/*static void
gl_control_box_change_level(GlLevelManager *level ,GlControlBox *box)
{
	if(box->dType == CONTROL_BOX_VBOX_DIRECTION)
	{
		gtk_drag_dest_unset (GTK_WIDGET(box));
		if(gl_level_manager_is_tru_user(level,GUI_USER_ROOT_TYPE) )
		{
			gtk_drag_dest_set(GTK_WIDGET(box),
					GTK_DEST_DEFAULT_ALL,
					gl_drag_and_drop_get_target_list(0),gl_drag_and_drop_get_n_targets(0),
					GDK_ACTION_COPY | GDK_ACTION_MOVE);
		}
	}
	gl_control_box_close(box);
	gl_control_box_open(box);
	gl_control_box_refresh(box);
}*/

static void
gl_control_box_remouve_plugin_signal (GtkContainer *container, GtkWidget    *widget, gpointer      user_data)
{
	g_return_if_fail(GL_IS_CONTROL_BOX(user_data));
	gl_control_box_refresh(GL_CONTROL_BOX(user_data));
	//GlPlugin *plugin = GL_PLUGIN(gtk_object_get_data(GTK_OBJECT(widget),"plugin"));
	//gl_manager_remove_plugin(GL_MANAGER(user_data),plugin,TRUE);
}

/*
gboolean
gl_control_box_pack_plugin(GlManager *manager,GlPlugin *plugin,gboolean user_move)
{
	g_return_val_if_fail(GL_IS_CONTROL_BOX(manager),FALSE);
	g_return_val_if_fail(GL_IS_PLUGIN(plugin),FALSE);
	GlControlBox *box = GL_CONTROL_BOX(manager);
	gboolean pack_ok = FALSE;

	gchar *place  = (char*) gl_manager_get_place(manager);
	GList *actions = gl_plugin_get_actions_for_place(plugin,gl_manager_get_place(manager));

	GList *curr = actions;
	while (curr)
	{
		if(curr -> data && GL_IS_ACTION_WIDGET(curr -> data) )
		{
			if(gl_control_box_add_widget(GL_CONTROL_BOX(manager),GTK_WIDGET(curr -> data),FALSE,FALSE,1))
			{
				pack_ok = TRUE;
			}
		}
		curr = curr->next;
	}
	g_list_free(actions);
	return pack_ok;
}
*/

/*
gboolean
gl_control_box_remove_plugin(GlManager *manager,GlPlugin *plugin ,gboolean user_move)
{
	g_return_val_if_fail(GL_IS_CONTROL_BOX(manager),FALSE);
	g_return_val_if_fail(GL_IS_PLUGIN(plugin),FALSE);
	GlControlBox *box = GL_CONTROL_BOX(manager);
	gl_control_box_refresh(box);
	return TRUE;
}
*/

static guint controlbox_signals[LAST_SIGNAL] = { 0 };

static void
gl_control_box_start_xkbd ( GlXKbd *xkbd ,GtkWidget *keytaste )
{

	//TEST:g_debug("gl_control_box_start_xkbd");
	gtk_widget_show(keytaste);
}

static void
gl_control_box_stop_xkbd ( GlXKbd *xkbd ,GtkWidget *keytaste )
{
	//TEST:g_debug("gl_control_box_stop_xkbd ");
	gtk_widget_hide(keytaste);
}
static void
gl_control_box_realize ( MktWindow *window )
{
	GlControlBox *ctrlbox = GL_CONTROL_BOX(window);
	ctrlbox->priv->window       =  gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_window_set_decorated ( GTK_WINDOW (ctrlbox->priv->window) , FALSE );
	ctrlbox->can_close = TRUE;
	ctrlbox->place = g_strdup(GL_CONTROL_BOX_PLACE_NAME);
	if(ctrlbox->priv->dType == CONTROL_BOX_HBOX_DIRECTION)
		ctrlbox->mhbox =  gtk_hbox_new(FALSE,1);
	else if(ctrlbox->priv->dType == CONTROL_BOX_VBOX_DIRECTION)
	{
		ctrlbox->mhbox =  gtk_vbox_new(FALSE,1);
	}
	gtk_container_add(GTK_CONTAINER(ctrlbox->priv->window ),ctrlbox->mhbox);
	if(ctrlbox->priv->dType == CONTROL_BOX_HBOX_DIRECTION)
	{
		ctrlbox->ochbox =  gtk_hbox_new(FALSE,0);
	}
	else if(ctrlbox->priv->dType == CONTROL_BOX_VBOX_DIRECTION)
	{
		ctrlbox->ochbox =  gtk_vbox_new(FALSE,0);
	}
	ctrlbox->cbOC      = gtk_button_new();
	ctrlbox->Image[CONTROL_BOX_OPEN_IMAGE]    = gtk_image_new();
	ctrlbox->Image[CONTROL_BOX_CLOSE_IMAGE]   = gtk_image_new();
	GtkWidget *opBbox    = gtk_vbox_new(FALSE,2);
	gtk_container_add(GTK_CONTAINER(ctrlbox->cbOC),GTK_WIDGET(opBbox));
	gtk_box_pack_start(GTK_BOX(opBbox),ctrlbox->Image[CONTROL_BOX_OPEN_IMAGE],TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(opBbox),ctrlbox->Image[CONTROL_BOX_CLOSE_IMAGE],TRUE,TRUE,0);
	gtk_widget_show(opBbox);

	if(ctrlbox->priv->dType == CONTROL_BOX_HBOX_DIRECTION)
	{
		gtk_widget_set_size_request(GTK_WIDGET(ctrlbox->cbOC),44,55);
		gtk_container_set_border_width(GTK_CONTAINER(ctrlbox->cbOC),3);
	}
	else
	{
		gtk_widget_set_size_request(GTK_WIDGET(ctrlbox->cbOC),55,44);
		gtk_container_set_border_width(GTK_CONTAINER(ctrlbox->cbOC),3);
	}
	gtk_box_pack_start(GTK_BOX(ctrlbox->mhbox),GTK_WIDGET(ctrlbox->ochbox),FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(ctrlbox->mhbox),GTK_WIDGET(ctrlbox->cbOC),FALSE,FALSE,0);
	ctrlbox->keytaste = gtk_button_new();
	gtk_widget_set_size_request(GTK_WIDGET(ctrlbox->cbOC),55,55);
	GtkWidget *image = gtk_image_new();
	gtk_image_set_from_file(GTK_IMAGE(image),"/lar/gui/gnome-settings-accessibility-keyboard.png");
	gtk_container_add(GTK_CONTAINER(ctrlbox->keytaste ),image);
	gtk_box_pack_end(GTK_BOX(ctrlbox->mhbox),GTK_WIDGET(ctrlbox->keytaste),FALSE,FALSE,0);
	if(ctrlbox->priv->dType == CONTROL_BOX_VBOX_DIRECTION)
	{
		gtk_widget_show(image);
		gtk_widget_show(ctrlbox->keytaste);
		//FIX : jetzt direct start ..
		g_signal_connect (ctrlbox->keytaste, "clicked",	G_CALLBACK (gl_control_box_open_close_xvkbd_clicked),ctrlbox->keytaste);
		g_signal_connect(gl_xkbd_get_keyboard(),"xkbd-start",G_CALLBACK(gl_control_box_start_xkbd),ctrlbox->keytaste);
		g_signal_connect(gl_xkbd_get_keyboard(),"xkbd-stop",G_CALLBACK(gl_control_box_stop_xkbd),ctrlbox->keytaste);
	}
	g_signal_connect (ctrlbox->cbOC, "clicked",
			G_CALLBACK (gl_control_box_open_close_signal_clicked), (gpointer) ctrlbox);
	//--------------------------------------------------------------------------------------------
	GlControlBoxLastOperationWidget count = CONTROL_BOX_LAST_OPERATION_NULL;
	for(;count<=CONTROL_BOX_LAST_OPERATION_LAST;count++)
		ctrlbox->lastSignal[count] = NULL;
	ctrlbox->inLink      = gtk_button_new();
	ctrlbox->Image[CONTROL_BOX_LEFT_IMAGE]    = gtk_image_new();
	gtk_container_add(GTK_CONTAINER(ctrlbox->inLink),GTK_WIDGET(ctrlbox->Image[CONTROL_BOX_LEFT_IMAGE]));
	if(ctrlbox->priv->dType == CONTROL_BOX_HBOX_DIRECTION)
	{
		gtk_widget_set_size_request(GTK_WIDGET(ctrlbox->inLink),40,55);
		gtk_container_set_border_width(GTK_CONTAINER(ctrlbox->inLink),5);
	}
	else
		gtk_widget_set_size_request(GTK_WIDGET(ctrlbox->inLink),55,40);
	ctrlbox->inRecht    = gtk_button_new();
	ctrlbox->Image[CONTROL_BOX_RIGHT_IMAGE]   = gtk_image_new();
	gtk_container_add(GTK_CONTAINER(ctrlbox->inRecht),GTK_WIDGET(ctrlbox->Image[CONTROL_BOX_RIGHT_IMAGE] ));
	if(ctrlbox->priv->dType == CONTROL_BOX_HBOX_DIRECTION)
	{
		gtk_widget_set_size_request(GTK_WIDGET(ctrlbox->inRecht),40,55);
		gtk_container_set_border_width(GTK_CONTAINER(ctrlbox->inRecht),5);
	}
	else
		gtk_widget_set_size_request(GTK_WIDGET(ctrlbox->inRecht),55,40);

	g_signal_connect (ctrlbox->inLink, "button_press_event",
			G_CALLBACK (gl_control_box_left_signal_pressed), (gpointer) ctrlbox);
	g_signal_connect (ctrlbox->inLink, "button_release_event",
			G_CALLBACK (gl_control_box_left_right_signal_relased), (gpointer) ctrlbox);
	g_signal_connect (ctrlbox->inRecht, "button_press_event",
			G_CALLBACK (gl_control_box_right_signal_pressed), (gpointer) ctrlbox);
	g_signal_connect (ctrlbox->inRecht, "button_release_event",
			G_CALLBACK (gl_control_box_left_right_signal_relased), (gpointer) ctrlbox);
	ctrlbox->viewport = gtk_viewport_new(NULL,NULL);
	gtk_container_set_border_width(GTK_CONTAINER(ctrlbox->viewport),0);
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(ctrlbox->viewport),GTK_SHADOW_NONE);
	if(ctrlbox->priv->dType == CONTROL_BOX_HBOX_DIRECTION)
		ctrlbox->ihbox =  gtk_hbox_new(FALSE,1);
	else if(ctrlbox->priv->dType == CONTROL_BOX_VBOX_DIRECTION)
		ctrlbox->ihbox =  gtk_vbox_new(FALSE,1);
	gtk_container_add(GTK_CONTAINER(ctrlbox->viewport),ctrlbox->ihbox);
	/*GtkWidget *test = gtk_label_new("Test label .... ");
	gtk_box_pack_start(GTK_BOX(ctrlbox->ihbox),GTK_WIDGET(test),TRUE,TRUE,0);
	gtk_widget_show(test);*/

	gtk_box_pack_start(GTK_BOX(ctrlbox->ochbox),GTK_WIDGET(ctrlbox->inLink),FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(ctrlbox->ochbox),GTK_WIDGET(ctrlbox->viewport),TRUE,TRUE,0);
	//Tgtk_box_pack_start(GTK_BOX(ctrlbox->ochbox),GTK_WIDGET(ctrlbox->ihbox),TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(ctrlbox->ochbox),GTK_WIDGET(ctrlbox->inRecht),FALSE,FALSE,2);
	g_signal_connect (gtk_viewport_get_hadjustment(GTK_VIEWPORT(ctrlbox->viewport)), "changed",
			G_CALLBACK (gl_control_box_viewport_signal_changed), (gpointer) ctrlbox);
	//---------------------------------------------------------------------------------------------
	if(ctrlbox->priv->dType == CONTROL_BOX_HBOX_DIRECTION)
	{
			ctrlbox -> max_widget = 11;
	}
	else if(ctrlbox->priv->dType == CONTROL_BOX_VBOX_DIRECTION)
	{
			ctrlbox -> max_widget = 8;
	}
	gtk_widget_show(ctrlbox->Image[CONTROL_BOX_LEFT_IMAGE]);
	gtk_widget_show(ctrlbox->inLink);
	gtk_widget_show(ctrlbox->Image[CONTROL_BOX_RIGHT_IMAGE]);
	gtk_widget_show(ctrlbox->inRecht );
	gtk_widget_show(ctrlbox->ihbox);
	gtk_widget_show(ctrlbox->viewport);
	gtk_widget_set_sensitive(ctrlbox->inLink,FALSE);
	gtk_widget_set_sensitive(ctrlbox->inRecht,FALSE);
	gtk_widget_set_sensitive(ctrlbox->ochbox,FALSE);
	gtk_widget_hide(ctrlbox->ochbox);
	gtk_widget_show(ctrlbox->Image[CONTROL_BOX_OPEN_IMAGE]);
	gtk_widget_show(ctrlbox->cbOC);
	gtk_widget_show(ctrlbox->mhbox );

	gtk_widget_hide(ctrlbox->inRecht);
	gtk_widget_hide(ctrlbox->inLink);


	gtk_drag_dest_set(GTK_WIDGET(ctrlbox->mhbox),
			GTK_DEST_DEFAULT_ALL,
			gl_drag_and_drop_get_target_list(0),gl_drag_and_drop_get_n_targets(0),
			GDK_ACTION_COPY | GDK_ACTION_MOVE);
	/* All possible destination signals */

	g_signal_connect (GTK_WIDGET(ctrlbox->mhbox), "drag_motion",
			G_CALLBACK ( gl_control_box_drag_motion ), ctrlbox);

	g_signal_connect ( GTK_WIDGET(ctrlbox->mhbox), "drag_drop",
			G_CALLBACK ( gl_control_box_drag_drop ), ctrlbox);

	g_signal_connect (GTK_WIDGET(ctrlbox->mhbox), "drag_data_received",
			G_CALLBACK ( gl_control_box_drag_data_received ), ctrlbox);

	g_signal_connect(ctrlbox->ihbox, "remove",
			G_CALLBACK ( gl_control_box_remouve_plugin_signal ), (gpointer) ctrlbox);
#ifdef IF_GTK_LIBRARY  // Set module window and container
	mkt_window_set_gtk_container ( MKT_WINDOW(window),ctrlbox->mhbox );
	mkt_window_set_gtk_window ( MKT_WINDOW(window),ctrlbox->priv->window );
#endif
}

gboolean
gl_control_box_add_action ( GlControlBox *ctrlbox , MktWindow *action ,gboolean expand,gboolean fill,gint padding )
{
	//TEST:g_debug("add action %s to control box %s ",mkt_atom_get_id(MKT_ATOM(action)),mkt_atom_get_id(MKT_ATOM(ctrlbox)));
	g_return_val_if_fail (ctrlbox != NULL,FALSE);
	g_return_val_if_fail (GL_IS_CONTROL_BOX(ctrlbox),FALSE);
	GtkWidget *widget = mkt_window_get_gtk_container(MKT_WINDOW(action));
	g_return_val_if_fail(widget !=NULL , FALSE );
	if(gl_control_box_get_childs(ctrlbox)>=ctrlbox->max_widget) return FALSE;
	GtkWidget *parent = gtk_widget_get_parent(widget);
	if(parent == NULL )
	{
		//TEST:g_debug("gl_control_box_add_action 5");
		gtk_box_pack_start(GTK_BOX(ctrlbox->ihbox),widget,expand,fill,padding);
		mkt_atom_object_set(MKT_ATOM(action),"width_pos",50,"height_pos",50,"font","Oreal 5","place",mkt_atom_get_id(MKT_ATOM(ctrlbox)),NULL);
		gtk_widget_show(widget);
		gtk_widget_show(ctrlbox->ihbox);
		gl_control_box_close(ctrlbox);
		gl_control_box_open(ctrlbox);
		gl_control_box_refresh(ctrlbox);
	}
	return TRUE;
}

static gboolean
gl_control_box_repack_action (  GlControlBox *ctrlbox , MktWindow *action ,gboolean expand,gboolean fill,gint padding )
{
	g_return_val_if_fail (ctrlbox != NULL,FALSE);
	g_return_val_if_fail (GL_IS_CONTROL_BOX(ctrlbox),FALSE);
	GtkWidget *widget = mkt_window_get_gtk_container(MKT_WINDOW(action));
	g_return_val_if_fail(widget !=NULL , FALSE );
	if(gl_control_box_get_childs(ctrlbox)>=ctrlbox->max_widget) return FALSE;
	GtkWidget *parent = gtk_widget_get_parent(widget);
	if(0==g_strcmp0(mkt_atom_get_id(MKT_ATOM(ctrlbox)),mktAPGet(action,"place",string)))
	{
		gtk_widget_ref(widget);
		gtk_container_remove(GTK_CONTAINER(parent),widget);
		gtk_box_pack_start(GTK_BOX(ctrlbox->ihbox),widget,expand,fill,padding);
		mkt_atom_object_set(MKT_ATOM(action),"width_pos",50,"height_pos",50,"font","Oreal 5","place",mkt_atom_get_id(MKT_ATOM(ctrlbox)),NULL);
		gtk_widget_show(widget);
		gtk_widget_show(ctrlbox->ihbox);
		gl_control_box_close(ctrlbox);
		gl_control_box_open(ctrlbox);
		gl_control_box_refresh(ctrlbox);
	}
	return TRUE;
}

static void
gl_control_box_new_atom_cb ( MktCollector *collctor , MktAtom *atom , GlControlBox *ctrlbox)
{
	if(GL_IS_ACTION_WIDGET(atom))
	{
		//TEST:g_debug("manager %s new action %s place %s",mkt_atom_get_id(MKT_ATOM(ctrlbox)),mkt_atom_get_id(atom),gl_action_widget_get_place(GL_ACTION_WIDGET (atom)));
		if(0==g_strcmp0(mkt_atom_get_id(MKT_ATOM(ctrlbox)),gl_action_widget_get_place(GL_ACTION_WIDGET (atom))))
			gl_control_box_add_action ( ctrlbox , MKT_WINDOW(atom) ,FALSE,FALSE,1);
	}
}

static void
gl_control_box_remove_atom_cb ( MktCollector *collctor , MktAtom *atom , GlControlBox *ctrlbox )
{
	//TEST:g_debug("gl_level_notebook_remove_atom_cb");
}


static void
gl_control_box_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_CONTROL_BOX (object));
	GlControlBox *crtlbox = GL_CONTROL_BOX(object);

	switch (prop_id)
	{
	case PROP_CONTROL_BOX_DIRECTION:
		crtlbox->priv->dType        =  g_value_get_uint(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID ( object, prop_id, pspec );
		break;
	}
}

static void
gl_control_box_get_property ( GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_CONTROL_BOX (object));
	GlControlBox *crtlbox = GL_CONTROL_BOX(object);

	switch (prop_id)
	{
	case PROP_CONTROL_BOX_DIRECTION:
		g_value_set_uint( value , crtlbox->priv->dType );
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}



static void
gl_control_box_class_init(GlControlBoxClass *klass)
{
	GObjectClass*         object_class     =  G_OBJECT_CLASS (klass);
	MktWindowClass*       parent_class     =  MKT_WINDOW_CLASS(klass);
	g_type_class_add_private (klass, sizeof (GlControlBoxPrivate));

	object_class -> set_property           =  gl_control_box_set_property;
	object_class -> get_property           =  gl_control_box_get_property;

	parent_class -> realize                =  gl_control_box_realize;


	g_object_class_install_property( object_class,
			PROP_CONTROL_BOX_DIRECTION,
				g_param_spec_uint("direction",
						"direction type",
						"Set/Get control box direction type ",
						MKT_WINDOW_POS_TOP,MKT_WINDOW_POS_LEFT,MKT_WINDOW_POS_TOP,
						G_PARAM_READABLE | G_PARAM_WRITABLE ));

	controlbox_signals[OPEN_CONTROL_BOX_SIGNAL] =
	    g_signal_new ("open_control_box",
	                  G_TYPE_FROM_CLASS (klass),
	                  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
	                  G_STRUCT_OFFSET (GlControlBoxClass, controlbox),
	                  NULL, NULL,
	                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	controlbox_signals[CLOSE_CONTROL_BOX_SIGNAL] =
		g_signal_new ("close_control_box",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlControlBoxClass, controlbox),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	controlbox_signals[CLICK_CONTROL_BOX_BUTTON_SIGNAL] =
			g_signal_new ("click_control_box_button",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlControlBoxClass, controlbox),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

}

static void
gl_control_box_init(GlControlBox *ctrlbox)
{
	ctrlbox->priv        = G_TYPE_INSTANCE_GET_PRIVATE(ctrlbox,GL_CONTROL_BOX_TYPE,GlControlBoxPrivate);
	ctrlbox->priv->dType = CONTROL_BOX_HBOX_DIRECTION;
	g_signal_connect ( mkt_collector_get_static(),"new-atom",G_CALLBACK(gl_control_box_new_atom_cb),ctrlbox);
	g_signal_connect ( mkt_collector_get_static(),"remove_atom",G_CALLBACK(gl_control_box_remove_atom_cb),ctrlbox);

}


MktAtom*
gl_control_box_new_full(const gchar *id,GlControBoxDirectionType type,guint x, guint y, guint width , guint height , guint pos  )
{
	MktAtom* controlbox;
	__last_direction = type;
	controlbox = mkt_atom_object_new (GL_CONTROL_BOX_TYPE, MKT_ATOM_PN_ID , id, "direction" , type ,
			                                                        "x_pos", x , "y_pos" , y ,
			                                                        "width_pos" , width , "height_pos" , height ,
			                                                        "position-type" , pos ,NULL);
	return controlbox;
}

gboolean
gl_control_box_open(GlControlBox *ctrlbox)
{
	//TEST:g_debug("gl_control_box_open ... ");
	g_return_val_if_fail (ctrlbox != NULL,FALSE);
	g_return_val_if_fail (GL_IS_CONTROL_BOX(ctrlbox),FALSE);
	gtk_widget_hide(ctrlbox->Image[CONTROL_BOX_OPEN_IMAGE]);
	gtk_widget_show(ctrlbox->Image[CONTROL_BOX_CLOSE_IMAGE]);
	gtk_widget_show(ctrlbox->ochbox);
	gtk_widget_set_sensitive(ctrlbox->ochbox,TRUE);
	//g_signal_emit (ctrlbox,
	//		controlbox_signals[OPEN_CONTROL_BOX_SIGNAL], 0);
	return FALSE;
}
gboolean
gl_control_box_close(GlControlBox *ctrlbox)
{
	g_return_val_if_fail (ctrlbox != NULL,FALSE);
	g_return_val_if_fail (GL_IS_CONTROL_BOX(ctrlbox),FALSE);
	gtk_widget_hide(ctrlbox->Image[CONTROL_BOX_CLOSE_IMAGE]);
	gtk_widget_show(ctrlbox->Image[CONTROL_BOX_OPEN_IMAGE]);
	gtk_widget_hide(ctrlbox->ochbox);
	gtk_widget_set_sensitive(ctrlbox->ochbox,FALSE);
	//g_signal_emit (ctrlbox,
	//			controlbox_signals[CLOSE_CONTROL_BOX_SIGNAL], 0);
	return FALSE;
}

void
gl_control_box_set_max_widgets  ( GlControlBox *ctrlbox,gint  max )
{
	g_return_if_fail (ctrlbox != NULL);
	g_return_if_fail (GL_IS_CONTROL_BOX(ctrlbox));
	ctrlbox -> max_widget = max;
}


void
gl_control_box_can_close (GlControlBox *ctrlbox,gboolean can_close)
{
	g_return_if_fail (ctrlbox != NULL);
	g_return_if_fail (GL_IS_CONTROL_BOX(ctrlbox));
	ctrlbox->can_close = can_close;

	if(ctrlbox->can_close)
	{
		gtk_widget_show(ctrlbox->cbOC);
	}
	else
	{
		gtk_widget_hide(ctrlbox->cbOC);
		gl_control_box_open(ctrlbox);
	}

}

gboolean
gl_control_box_open_close(GlControlBox *ctrlbox)
{
	g_return_val_if_fail (ctrlbox != NULL,FALSE);
	g_return_val_if_fail (GL_IS_CONTROL_BOX(ctrlbox),FALSE);
	if(GTK_WIDGET_SENSITIVE(ctrlbox->ochbox))
	{
		gl_control_box_close(ctrlbox);
	}
	else
	{
		gl_control_box_open(ctrlbox);
	}
	return FALSE;
}

void
gl_control_box_set_max_size(GlControlBox *ctrlbox,gint width,gint higth)
{
	g_return_if_fail (ctrlbox != NULL);
	g_return_if_fail (GL_IS_CONTROL_BOX(ctrlbox));
	ctrlbox ->maxHigth  = higth;
	ctrlbox ->maxWidth  = width;
	gtk_widget_set_size_request(GTK_WIDGET(ctrlbox->mhbox),width,higth);
}

// ---------------------------------------Signals------------------------------------------
gboolean
gl_control_box_open_close_signal_clicked(GtkWidget *button,GlControlBox *ctrlbox)
{
	gl_control_box_open_close(ctrlbox);
	return TRUE;
}

gboolean
gl_control_box_open_close_xvkbd_clicked(GtkWidget *button,GtkWidget *keytaste)
{
	//TEST:g_debug("gl_control_box_open_close_xvkbd_clicked");
	gtk_widget_show(keytaste);
	//TEST:g_debug("gl_control_box_open_close_xvkbd_clicked end");
	gtk_widget_show(button);
	gl_xkbd_stop();
	return TRUE;
}

gboolean
gl_control_box_viewport_signal_changed(GtkWidget *button,GlControlBox *ctrlbox)
{
	g_return_val_if_fail (ctrlbox != NULL,FALSE);
	g_return_val_if_fail (GL_IS_CONTROL_BOX(ctrlbox),FALSE);
	return TRUE;
}
gboolean
gl_control_box_left_signal_pressed(GtkWidget *button,GdkEventButton *event,GlControlBox *ctrlbox)
{
	g_return_val_if_fail (ctrlbox != NULL,FALSE);
	g_return_val_if_fail (GL_IS_CONTROL_BOX(ctrlbox),FALSE);
	if(!__key_button_press)
	{
		__key_button_press = TRUE;
		__key_button_up_press((gpointer) (ctrlbox));
		gtk_timeout_add(50,__key_button_up_press,(gpointer) (ctrlbox));
	}
	return FALSE;
}
gboolean
gl_control_box_right_signal_pressed(GtkWidget *button,GdkEventButton *event,GlControlBox *ctrlbox)
{
	g_return_val_if_fail (ctrlbox != NULL,FALSE);
	g_return_val_if_fail (GL_IS_CONTROL_BOX(ctrlbox),FALSE);
	if(!__key_button_press)
	{
		__key_button_press = TRUE;
		__key_button_down_press((gpointer) (ctrlbox));
		gtk_timeout_add(50,__key_button_down_press,(gpointer) (ctrlbox));
	}
	return FALSE;
}

static gboolean
gl_control_box_left_right_signal_relased(GtkWidget *button,GdkEventButton *event,
														  GlControlBox *ctrlbox)
{
	__key_button_press = FALSE;
	return FALSE;
}


// ----------------------Image Options-----------------------------------------------------


gboolean
gl_control_box_set_images_from_files (GlControlBox *ctrlbox,GlControBoxImageType _type,char *file)
{
	g_return_val_if_fail (ctrlbox != NULL,FALSE);
	g_return_val_if_fail (GL_IS_CONTROL_BOX(ctrlbox),FALSE);
	gtk_image_set_from_file(GTK_IMAGE(ctrlbox->Image[_type]),file);
	return TRUE;
}

// --------------------- Control Options----------------------------------------------------


void
__internal_set_all_control_items_default(GtkWidget *widget,   gpointer data)
{
	//gtk_widget_set_size_request(widget,44,44);
	g_return_if_fail (data != NULL);
	g_return_if_fail (GL_IS_CONTROL_BOX(data));
	GlControlBox *crtbox = GL_CONTROL_BOX(data);
	if(crtbox->priv->dType == CONTROL_BOX_HBOX_DIRECTION)
		gtk_container_set_border_width(GTK_CONTAINER(widget),5);
}

void
__internal_set_all_not_sensetive_control_items_default(GtkWidget *widget,   gpointer data)
{
	g_return_if_fail (data != NULL);
	g_return_if_fail (GL_IS_CONTROL_BOX(data));
	GlControlBox *crtbox = GL_CONTROL_BOX(data);
	if(!GTK_WIDGET_IS_SENSITIVE(widget)&&(crtbox->priv->dType == CONTROL_BOX_HBOX_DIRECTION))
		gtk_container_set_border_width(GTK_CONTAINER(widget),5);

}

gboolean
gl_control_box_remove_action(GlControlBox *ctrlbox,GlActionWidget *action)
{
	g_return_val_if_fail (ctrlbox != NULL,FALSE);
	g_return_val_if_fail (GL_IS_CONTROL_BOX(ctrlbox),FALSE);
	//TEST:g_debug("Control box %s remove atom %s",mkt_atom_get_id(MKT_ATOM(ctrlbox)),mkt_atom_get_id(MKT_ATOM(action)));
	g_object_unref(MKT_ATOM(action));
	gl_control_box_close(ctrlbox);
	gl_control_box_open(ctrlbox);
	gl_control_box_refresh(ctrlbox);
	return TRUE;
}



gboolean
gl_control_box_refresh(GlControlBox *ctrlbox)
{
	g_return_val_if_fail (ctrlbox != NULL,FALSE);
	g_return_val_if_fail (GL_IS_CONTROL_BOX(ctrlbox),FALSE);
	ctrlbox -> aButtonLen = 0;
	//if(GTK_WIDGET_IS_SENSITIVE(ctrlbox->ochbox))
	if(NULL == gtk_container_get_children(GTK_CONTAINER(ctrlbox->ihbox)))
	{
		if(ctrlbox->can_close)gl_control_box_close(ctrlbox);
	}
	gtk_container_resize_children(GTK_CONTAINER(ctrlbox->priv->window));
	//gtk_container_foreach(GTK_CONTAINER(ctrlbox->ihbox),__internal_set_all_not_sensetive_control_items_default,ctrlbox);
	return TRUE;
}


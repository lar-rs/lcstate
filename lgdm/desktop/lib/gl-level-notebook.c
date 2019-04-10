/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-level-notebook.c
 * Copyright (C) A.Smolkov 2011 <asmolkov@lar.com>
 * 
 * gl-level-notebook.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-level-notebook.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "gl-level-notebook.h"
#include "gl-action-widget.h"
#include "gl-draganddrop.h"
#include "gl-translation.h"
#include "gl-controlbox.h"
#include "gl-level-manager.h"
#include "gl-plugin.h"

#include <mkt-collector.h>
#include <string.h>


G_DEFINE_TYPE (GlLevelNotebook, gl_level_notebook, MKT_TYPE_WINDOW);


typedef struct
{
	GlPlugin   *plugin;
	GtkWidget  *widget;
}GlLevelNotebookSearch;

typedef struct
{
	GtkWidget *tab;
	GtkWidget *container[GL_LEVEL_NOTEBOOK_MAX_X][GL_LEVEL_NOTEBOOK_MAX_Y];
}GlLevelNotebookTable;


struct _GlLevelNotebookPrivate
{
#ifdef IF_GTK_LIBRARY
	GtkWidget              *window;
	GtkWidget              *mBox;
	GtkWidget              *notebook;
	GtkWidget              *image [GUI_USER_LAST_TYPE+1];
	GtkWidget              *label [GUI_USER_LAST_TYPE+1];
	GlLevelNotebookTable    table [GUI_USER_LAST_TYPE+1];
#endif
	GList                  *search_action;
	GList                  *pMenu;
};
enum
{
	PROP_NULL,
};

enum
{
	GL_PLUGIN_CHANGE_ACTIVE_PLUGIN,
	LAST_SIGNAL
};


//static guint gl_level_notebook_signals[LAST_SIGNAL] = { 0 };


static void                     gl_level_notebook_add_action                  ( GlLevelNotebook *notebook , GlActionWidget *action );
static void                     gl_level_notebook_repack_action               ( GlLevelNotebook *notebook , GlActionWidget *action );


static gboolean
gl_level_notebook_get_free_position ( GlLevelNotebook *object, GlLevelManagerUserType level, GdkPoint *p )
{
	g_return_val_if_fail(object!=NULL,FALSE);
	g_return_val_if_fail (GL_IS_LEVEL_NOTEBOOK(object),FALSE);
	g_return_val_if_fail(p!=NULL,FALSE);
	g_return_val_if_fail(level<GUI_USER_LAST_TYPE+1,FALSE);
	if((p->x>=0&&p->x<GL_LEVEL_NOTEBOOK_MAX_X)&&(p->y>=0&&p->y<GL_LEVEL_NOTEBOOK_MAX_Y))
	{
		if(!gtk_container_get_children(GTK_CONTAINER(object->priv->table[level].container[p->x][p->y])))return TRUE;
		gint i,j;
		for(i=0;i<GL_LEVEL_NOTEBOOK_MAX_X;i++)
		{
			for(j=0;j<GL_LEVEL_NOTEBOOK_MAX_Y;j++)
			{
				if(!gtk_container_get_children(GTK_CONTAINER(object->priv->table[level].container[i][j]))){p->x=i;p->y=j;return TRUE;}
			}
		}

	}
	return FALSE;
}

gboolean
gl_level_notebook_drag_motion(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, guint t, gpointer data)
{
	//g_printf("gl_level_notebook_drag_motion\n");
	return TRUE;

}

gboolean
gl_level_notebook_drag_data_delete(GtkWidget *widget, GdkDragContext *dc, gpointer data)
{
	//g_printf("gl_level_notebook_drag_drop\n");
	return TRUE;

}

gboolean
gl_level_notebook_drag_drop(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, guint t, gpointer data)
{
	//TEST:g_debug ( "level notebook drop widget");
	g_return_val_if_fail(data  != NULL,FALSE);
	g_return_val_if_fail(GL_IS_LEVEL_NOTEBOOK(data),FALSE);
	GtkWidget *wi     = gtk_drag_get_source_widget(dc);
	MktAtom   *atom   = MKT_ATOM(mkt_window_gtk_widget_get_parent(wi));

	if(atom!=NULL && GL_IS_ACTION_WIDGET(atom))
	{
		if(0!=g_strcmp0(mkt_atom_get_id(MKT_ATOM(data)),gl_action_widget_get_place(GL_ACTION_WIDGET (atom))))
		{
			g_object_unref(atom);
			return TRUE;
		}
		GdkPoint  *point;
		guint     *level;
		point =(GdkPoint*) gtk_object_get_data(GTK_OBJECT(widget),"position");
		level =(gint*)     gtk_object_get_data(GTK_OBJECT(widget),"level");
		mkt_atom_object_set(MKT_ATOM(atom),"level",*level,"posX",point->x,"posY",point->y,NULL);
		GtkWidget *parent = gtk_widget_get_parent(wi);
		gtk_widget_ref(wi);
		gtk_container_remove(GTK_CONTAINER(parent),wi);
		gl_level_notebook_repack_action ( GL_LEVEL_NOTEBOOK(data), GL_ACTION_WIDGET(atom) );

	}
	return TRUE;
}


gboolean
gl_level_notebook_drag_to_level(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, guint t, gpointer data)
{
	g_return_val_if_fail(data  != NULL,FALSE);
	g_return_val_if_fail(GL_IS_LEVEL_NOTEBOOK(data),FALSE);
	//TEST:g_debug ( "gl_level_notebook_drag_to_level");
	GtkWidget *wi     = gtk_drag_get_source_widget(dc);
	MktAtom   *atom   = MKT_ATOM(mkt_window_gtk_widget_get_parent(wi));

	if(atom!=NULL && GL_IS_ACTION_WIDGET(atom))
	{
		guint     *level;
		level =(gint*)      gtk_object_get_data(GTK_OBJECT(widget),"level");
		mkt_atom_object_set(MKT_ATOM(atom),"level",*level,NULL);
		GtkWidget *parent = gtk_widget_get_parent(wi);
		gtk_widget_ref(wi);
		gtk_container_remove(GTK_CONTAINER(parent),wi);
		gl_level_notebook_repack_action ( GL_LEVEL_NOTEBOOK(data), GL_ACTION_WIDGET(atom) );
	}
	return FALSE;
}





void
gl_level_notebook_drag_data_received(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, gpointer data)
{
	g_debug("gl_level_notebook_drag_data_received\n");
}

gboolean gl_level_notebook_close_plugin_signal (GtkButton *button, gpointer data)
{
	g_return_val_if_fail(data  != NULL,FALSE);
	g_return_val_if_fail(GL_IS_LEVEL_NOTEBOOK(data),FALSE);
	/*if(NULL != gl_manager_get_active_plugin_menu(GL_MANAGER(data)))
		gl_plugin_close_menu(gl_manager_get_active_plugin_menu(GL_MANAGER(data)));*/
	return TRUE;
}
/* drag_begin	void (*drag_begin)(GtkWidget *widget, GdkDragContext *dc, gpointer data)
	 drag_motion	gboolean (*drag_motion)(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, guint t, gpointer data)
	 drag_data_get	void (*drag_data_get)(GtkWidget *widget, GdkDragContext *dc, GtkSelectionData *selection_data, guint info, guint t, gpointer data)
	 drag_data_delete	void (*drag_data_delete)(GtkWidget *widget, GdkDragContext *dc, gpointer data)
	 drag_drop	gboolean (*drag_drop)(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, guint t, gpointer data)
	 drag_end	void (*drag_end)(GtkWidget *widget, GdkDragContext *dc, gpointer data)*/


static void
gl_level_notebook_realize ( MktWindow *win )
{
	GlLevelNotebook *object = GL_LEVEL_NOTEBOOK(win);
	gint  count;
	if( object->priv->window != NULL  )
	{
		gtk_widget_destroy ( object->priv->window );
	}
	object->priv->window       =  gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_decorated ( GTK_WINDOW (object->priv->window) , FALSE );
	GtkWidget *scrolled_win    =  gtk_scrolled_window_new(NULL,NULL);
	GtkWidget *viewport        =  gtk_viewport_new(NULL,NULL);
	object->priv->mBox         =  gtk_vbox_new(FALSE,1);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_win),viewport);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_win),GTK_SHADOW_NONE);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	gtk_container_add ( GTK_CONTAINER(viewport ) , object->priv->mBox );
	gtk_container_add(GTK_CONTAINER(object->priv->window ),scrolled_win);
	gtk_widget_show(viewport);
	gtk_widget_show(scrolled_win);
	object->priv->notebook = gtk_notebook_new();

	for(count = GUI_USER_DEVICE_TYPE;count<= GUI_USER_LAST_TYPE; count++)
	{
		object->priv->table[count].tab = gtk_table_new(20,8,FALSE);
		object->priv->image[count]     = gtk_image_new();
		GtkWidget  *hbox               = gtk_hbox_new(FALSE,0);
		switch(count)
		{
		case GUI_USER_DEVICE_TYPE         : gtk_image_set_from_file(GTK_IMAGE(object->priv->image[count]),"/lar/gui/buttons-level/level_I.png");  break;
		case GUI_USER_OPERATOR_TYPE       : gtk_image_set_from_file(GTK_IMAGE(object->priv->image[count]),"/lar/gui/buttons-level/level_II.png"); break;
		case GUI_USER_SUPER_OPERATOR_TYPE : gtk_image_set_from_file(GTK_IMAGE(object->priv->image[count]),"/lar/gui/buttons-level/level_III.png");break;
		case GUI_USER_SERVICE_TYPE        : gtk_image_set_from_file(GTK_IMAGE(object->priv->image[count]),"/lar/gui/buttons-level/level_IV.png"); break;
		case GUI_USER_ROOT_TYPE           : gtk_image_set_from_file(GTK_IMAGE(object->priv->image[count]),"/lar/gui/buttons-level/level_V.png");  break;
		case GUI_USER_LAST_TYPE           : gtk_image_set_from_file(GTK_IMAGE(object->priv->image[count]),"/lar/gui/edit-find.png");              break;
		default :gtk_image_set_from_file(GTK_IMAGE(object->priv->image[count]),"/lar/gui/bruce_lee.png"); break;
		}
		//object->priv->label[count] = gtk_label_new(level);
		//gtk_widget_set_name(object->priv->label[count] ,(const char*)levelWname);
		gtk_box_pack_start(GTK_BOX(hbox),object->priv->image[count],TRUE,TRUE,2);
		//gtk_box_pack_start(GTK_BOX(hbox),object->priv->label[count] ,TRUE,TRUE,2);
		if(count <  GUI_USER_LAST_TYPE)
		{
			gint *level =  g_malloc0(sizeof(gint));
			*level = count;
			gtk_object_set_data_full(GTK_OBJECT(hbox),"level",(gpointer)level,g_free);
			gtk_drag_dest_set(hbox,
					GTK_DEST_DEFAULT_ALL,
					gl_drag_and_drop_get_target_list(0),gl_drag_and_drop_get_n_targets(0),
					GDK_ACTION_COPY | GDK_ACTION_MOVE);
			g_signal_connect (GTK_WIDGET(hbox), "drag_drop",
					G_CALLBACK (gl_level_notebook_drag_to_level), object);
		}
		gtk_widget_set_size_request(hbox,85,30);
		GtkWidget *viewport = gtk_viewport_new ( NULL, NULL );
		GtkWidget *scroolled_window = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroolled_window),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroolled_window),GTK_SHADOW_NONE);
		gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(scroolled_window),GTK_CORNER_TOP_LEFT);
		gtk_container_add(GTK_CONTAINER(scroolled_window),viewport);
		gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport),GTK_SHADOW_NONE);
		gtk_container_add(GTK_CONTAINER(viewport),object->priv->table[count].tab);

		gtk_widget_show( viewport );
		gtk_widget_show( scroolled_window );

		gtk_notebook_append_page(GTK_NOTEBOOK(object->priv->notebook),scroolled_window,hbox);

		gtk_widget_show(object->priv->table[count].tab);
		//gtk_widget_show(object->priv->label[count]);
		gtk_widget_show(object->priv->image[count]);
		gtk_widget_show(hbox);
		int i,j;
		for(i = 0; i<GL_LEVEL_NOTEBOOK_MAX_X;i++)
		{
			for(j=0;j<GL_LEVEL_NOTEBOOK_MAX_Y;j++)
			{
				object->priv->table[count].container[i][j] = gtk_hbox_new(TRUE,1);
				/*GtkWidget *label;
				gchar ln[40] = "";
				sprintf(ln,"i=%d;j=%d",i,j);
				label = gtk_label_new(ln);
				gtk_box_pack_start(GTK_BOX(object->table[count].container[i][j]),label,TRUE,TRUE,1);
				gtk_widget_show(label);*/
				gtk_widget_set_size_request(object->priv->table[count].container[i][j],80,80);
				gtk_table_attach(GTK_TABLE(object->priv->table[count].tab),object->priv->table[count].container[i][j],i,i+1,j,j+1,GTK_FILL,GTK_FILL,2,2);
				GdkPoint *pos = g_malloc0(sizeof(GdkPoint));
				pos->x = i;
				pos->y = j;
				gint *level =  g_malloc0(sizeof(gint));
				*level = count;
				gtk_object_set_data_full(GTK_OBJECT(object->priv->table[count].container[i][j]),"position",(gpointer)pos,g_free);
				gtk_object_set_data_full(GTK_OBJECT(object->priv->table[count].container[i][j]),"level",(gpointer)level,g_free);
				if(count <  GUI_USER_LAST_TYPE)
				{
					gtk_drag_dest_set(object->priv->table[count].container[i][j],
							GTK_DEST_DEFAULT_ALL,
							gl_drag_and_drop_get_target_list(0),gl_drag_and_drop_get_n_targets(0),
							GDK_ACTION_COPY | GDK_ACTION_MOVE);
					/* All possible destination signals */

					g_signal_connect (GTK_WIDGET(object->priv->table[count].container[i][j]), "drag_motion",
							G_CALLBACK (gl_level_notebook_drag_motion), object);

					g_signal_connect (GTK_WIDGET(object->priv->table[count].container[i][j]), "drag_drop",
							G_CALLBACK (gl_level_notebook_drag_drop), object);

					g_signal_connect (object->priv->table[count].container[i][j], "drag_data_received",
							G_CALLBACK (gl_level_notebook_drag_data_received), object);
				}
				gtk_widget_show(object->priv->table[count].container[i][j]);
			}
		}
	}
	object->priv->search_action = NULL;

	gtk_box_pack_start(GTK_BOX(object->priv->mBox),object->priv->notebook,TRUE,TRUE,1);
	gtk_widget_show(object->priv->notebook);
	gtk_widget_show(object->priv->mBox);
#ifdef IF_GTK_LIBRARY  // Set modul window and container
	mkt_window_set_gtk_container ( MKT_WINDOW(object),scrolled_win );
	mkt_window_set_gtk_window ( MKT_WINDOW(object),object->priv->window );
#endif
	/* TODO: Add initialization code here */
}

MktAtom*
gl_level_notebook_new ( const gchar *id )
{
	MktAtom *notebook = mkt_atom_object_new (GL_TYPE_LEVEL_NOTEBOOK , MKT_ATOM_PN_ID , id ,"x_pos", 0 , "y_pos" , 37 , "width_pos" ,744 , "height_pos" , 504 , NULL);
	return notebook;
}

MktAtom*
gl_level_notebook_new_full ( const gchar *id , guint x , guint y , guint width , guint height )
{
	MktAtom *notebook = mkt_atom_object_new (GL_TYPE_LEVEL_NOTEBOOK , MKT_ATOM_PN_ID , id ,
			                                                          "x_pos", x , "y_pos" , y ,
			                                                          "width_pos" , width , "height_pos" , height ,
			                                                          "position-type" , MKT_WINDOW_POS_TYPE_CENTER , NULL);
	return notebook;
}

static void
gl_level_notebook_change_level ( GlLevelManager *level ,GlLevelNotebook *notebook )
{
	g_return_if_fail(GL_IS_LEVEL_NOTEBOOK(notebook));
	g_return_if_fail(GL_IS_LEVEL_MANAGER(level));

	gint  count;
	for(count = GUI_USER_DEVICE_TYPE;count< GUI_USER_LAST_TYPE; count++)
	{
		GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook->priv->notebook),count);
		if(!gl_level_manager_is_tru_user(count))
		{
			if(page != NULL) gtk_widget_hide(page);
		}
		else
		{
			if(page != NULL) gtk_widget_show(page);
		}
	}
}

/*
static gboolean
gl_level_notebook_has_plugin_menu( GlLevelNotebook *notebook , GlPlugin *plugin )
{
	g_return_val_if_fail(GL_IS_LEVEL_NOTEBOOK(notebook),FALSE);
	g_return_val_if_fail(GL_IS_PLUGIN(plugin),FALSE);
	GList *pm = notebook->priv->pMenu;
	while( pm )
	{
		if(pm->data)
		{
			if( (GTK_IS_WIDGET(pm->data)) && (gl_plugin_get_menu ( plugin ) != NULL ))
			{
				if( GTK_WIDGET(pm->data) == gl_plugin_get_menu ( plugin )  ) return TRUE;
			}
		}
		pm=pm->next;
	}
	return FALSE;
}


static gboolean
gl_level_notebook_is_search_plugin (GlLevelNotebook *manager , GlPlugin *plugin)
{
	GList *curr = manager->priv->search_action;
	while( curr )
	{
		if(curr->data != NULL)
		{
			GlLevelNotebookSearch *search = (GlLevelNotebookSearch *)curr->data;
			if(plugin == search -> plugin) return TRUE;
		}
		curr = curr -> next ;
	}
	return FALSE;
}

static gboolean
gl_level_notebook_set_search_plugin (GlLevelNotebook *manager , GlPlugin *plugin)
{
	g_return_val_if_fail(plugin != NULL,FALSE);
	g_return_val_if_fail(!gl_level_notebook_is_search_plugin(manager,plugin),FALSE);
	GtkWidget *search = gl_plugin_get_action(plugin,"search");
	if(search == NULL)
	{
		search = gl_plugin_create_action(plugin , "search" ,FALSE );
	}
	if(  search )
	{
		//FIX : if search plugin initialized free search widget ---->
		GlLevelNotebookSearch *sdata = g_malloc( sizeof (GlLevelNotebookSearch));
		sdata -> plugin = plugin;
		sdata -> widget = search;
		sdata -> widget = gtk_widget_ref(sdata -> widget);
		manager->priv->search_action = g_list_append(manager->priv->search_action,sdata);
		GL_ACTION_SET_PARAM(sdata -> widget,"place","search_notebook");

	}
	return TRUE;
	//FIX: destroy widget signal callback (free widget list object)
}*/

gboolean
gl_level_notebook_pack_level_widget ( GlLevelNotebook *manager ,GlLevelManagerUserType level ,GtkWidget *widget )
{
	g_return_val_if_fail(widget != NULL,FALSE);
	g_return_val_if_fail(gtk_widget_get_parent(widget)== NULL,FALSE);
	/*if ( GL_IS_ACTION_WIDGET(widget))
	{
		GdkPoint p ;
		p.x = (gint) GL_ACTION_GET_PARAM(widget,"posX",uint);
		p.y = (gint) GL_ACTION_GET_PARAM(widget,"posY",uint);
		if(gl_level_notebook_get_free_position(manager,level,&p))
		{
			gtk_container_add(GTK_CONTAINER(manager->priv->table[level].container[p.x][p.y]),widget);
			GL_ACTION_SET_PARAM(widget,"posX",p.x);
			GL_ACTION_SET_PARAM(widget,"posY",p.y);
			GL_ACTION_SET_PARAM(widget,"size",GL_ACTION_SIZE_DEFAULT);
			GL_ACTION_SET_PARAM(widget,"font",GL_ACTION_SIZE_DEFAULT);
			GL_ACTION_SET_PARAM(widget,"place",gl_manager_get_place( GL_MANAGER(manager) ) );
			gtk_widget_show(widget);
			return TRUE;
		}
	}*/
	return FALSE;
}

void
gl_level_notebook_destroyed_search_cb ( MktWindow *win ,  GlLevelNotebook *notebook )
{
	g_return_if_fail(win != NULL);
	g_return_if_fail(MKT_IS_WINDOW(win));
	g_return_if_fail(notebook != NULL);
	g_return_if_fail(GL_IS_LEVEL_MANAGER(notebook));
	notebook->priv->search_action = g_list_remove(notebook->priv->search_action,win);
	g_object_unref(win);
}

static void
gl_level_notebook_pack_action ( GlLevelNotebook *notebook , GlActionWidget *action ,  GtkWidget *pack_widget )
{
	//TEST:g_debug("gl_level_notebook_pack_action %s ",mkt_atom_get_id(MKT_ATOM(action)));
	guint level = mktAPGet(action,"level",uint);
	GdkPoint p;
	p.x = (gint) mktAPGet(action,"posX",uint);
	p.y = (gint) mktAPGet(action,"posY",uint);
	//TEST:g_debug("Level %d X=%d D=%d",level,p.x,p.y);
	if(gl_level_notebook_get_free_position(notebook,level,&p))
	{
		//TEST:g_debug("Level %d X=%d D=%d",level,p.x,p.y);
		gtk_container_add(GTK_CONTAINER(notebook->priv->table[level].container[p.x][p.y]),pack_widget);
		mkt_atom_object_set(MKT_ATOM(action),"width_pos",70,"height_pos",70,"posX",p.x,"posY",p.y,"font","Oreal 7","place",mkt_atom_get_id(MKT_ATOM(notebook)),NULL);
		gtk_widget_show(pack_widget);
	}
}




void
gl_level_notebook_repack_action ( GlLevelNotebook *notebook , GlActionWidget *action )
{
	GtkWidget *widget = mkt_window_get_gtk_container(MKT_WINDOW(action));
	g_return_if_fail(widget !=NULL );
	GtkWidget *parent = gtk_widget_get_parent(widget);
	if(parent == NULL )
	{
		gl_level_notebook_pack_action(notebook,action , widget);
	}
	else if(GTK_IS_CONTAINER(parent))
	{
		gtk_widget_ref(widget);
		gtk_container_remove(GTK_CONTAINER(parent),widget);
		gl_level_notebook_pack_action(notebook,action , widget );
	}
}
void
gl_level_notebook_action_change_level_notify (MktAtom *action ,  GParamSpec *pspec ,GlLevelNotebook *notebook )
{
	g_return_if_fail(notebook!=NULL);
	g_return_if_fail(action!=NULL);
	g_return_if_fail(GL_IS_LEVEL_NOTEBOOK(notebook));
	g_return_if_fail(GL_IS_ACTION_WIDGET(action));
	if(0==g_strcmp0(mkt_atom_get_id(MKT_ATOM(notebook)),gl_action_widget_get_place(GL_ACTION_WIDGET (action))))
		gl_level_notebook_repack_action(notebook,GL_ACTION_WIDGET(action));
	//mktAPSet(plugin,"level",mkt_window_get_level(MKT_WINDOW(child)));
}


void
gl_level_notebook_add_action ( GlLevelNotebook *notebook , GlActionWidget *action )
{
	GtkWidget *widget = mkt_window_get_gtk_container(MKT_WINDOW(action));
	g_return_if_fail(widget !=NULL );
	GtkWidget *parent = gtk_widget_get_parent(widget);
	if(parent == NULL )
	{
		g_signal_connect (action, "notify::level", G_CALLBACK (gl_level_notebook_action_change_level_notify), notebook);
		gl_level_notebook_pack_action(notebook,action , widget);
	}
}




void
gl_level_notebook_remove_action ( GlLevelNotebook *lnotebook , GlActionWidget *action )
{

}

static void
gl_level_notebook_new_atom_cb ( MktCollector *collctor , MktAtom *atom , GlLevelNotebook *notebook )
{
	if(GL_IS_ACTION_WIDGET(atom))
	{
		gchar *search_place = g_strdup_printf("%s.search",mkt_atom_get_id(MKT_ATOM(notebook)));
		//TEST:g_debug("manager %s new action %s place %s",mkt_atom_get_id(MKT_ATOM(notebook)),mkt_atom_get_id(atom),gl_action_widget_get_place(GL_ACTION_WIDGET (atom)));
		if(0==g_strcmp0(mkt_atom_get_id(MKT_ATOM(notebook)),gl_action_widget_get_place(GL_ACTION_WIDGET (atom))))
			gl_level_notebook_add_action( notebook , GL_ACTION_WIDGET (atom) );
		else if(0==g_strcmp0(search_place,gl_action_widget_get_place(GL_ACTION_WIDGET (atom))))
		{
			//TEST:g_debug("Find search action .....%s\n",mkt_atom_get_id(atom));
			notebook -> priv->search_action = g_list_append(notebook -> priv->search_action,atom );
			mkt_atom_object_set(MKT_ATOM(atom),"width_pos",70,"height_pos",70,"posX",0,"posY",0,"font","Oreal 7","active",FALSE,NULL);
			g_signal_connect ( atom , "will-destroyed" , G_CALLBACK(gl_level_notebook_destroyed_search_cb),notebook);
		}
		g_free(search_place);
	}
}

static void
gl_level_notebook_remove_atom_cb ( MktCollector *collctor , MktAtom *atom , GlLevelNotebook *notebook )
{
	//TEST:g_debug("gl_level_notebook_remove_atom_cb");
	if(GL_IS_ACTION_WIDGET(atom))
	{
		notebook -> priv->search_action = g_list_remove( notebook->priv->search_action , atom );
	}
}



static void
gl_level_notebook_init ( GlLevelNotebook *notebook )
{
	notebook->priv = G_TYPE_INSTANCE_GET_PRIVATE(notebook,GL_TYPE_LEVEL_NOTEBOOK,GlLevelNotebookPrivate);
	notebook->priv->window = NULL;
	notebook->priv->mBox   = NULL;
	// Connect all action widgets and Plug-in's ...
	MktAtom *lmanager = MKT_ATOM(gl_level_manager_get_static());
	if(lmanager)g_signal_connect(lmanager,"change_gui_level",G_CALLBACK(gl_level_notebook_change_level),notebook);
	else g_warning("com.lar.GlLevelManager.ultra-security object not found");
	g_signal_connect ( mkt_collector_get_static(),"new-atom",G_CALLBACK(gl_level_notebook_new_atom_cb),notebook);
	g_signal_connect ( mkt_collector_get_static(),"remove_atom",G_CALLBACK(gl_level_notebook_remove_atom_cb),notebook);
}

static void
gl_level_notebook_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	GlLevelNotebook* notebook = GL_LEVEL_NOTEBOOK(object);
	gtk_widget_unref(notebook->priv->mBox);
	G_OBJECT_CLASS (gl_level_notebook_parent_class)->finalize (object);
}

static void
gl_level_notebook_class_init (GlLevelNotebookClass *klass)
{
	GObjectClass*     object_class     =  G_OBJECT_CLASS (klass);
	MktWindowClass*   parent_class     =  MKT_WINDOW_CLASS(klass);
	g_type_class_add_private (klass, sizeof (GlLevelNotebookPrivate));
	object_class -> finalize           =  gl_level_notebook_finalize;
	parent_class -> realize            =  gl_level_notebook_realize;
}


void
gl_level_notebook_set_search  ( GlLevelNotebook *notebook, const gchar *muster )
{
	//TEST:	g_debug ( "SEARCH: %s",muster);
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook->priv->notebook))!= GUI_USER_LAST_TYPE)
		gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook->priv->notebook),GUI_USER_LAST_TYPE);
	if( muster == NULL ) return ;
	GList *curr = notebook->priv->search_action;
	while( curr )
	{
		if(curr->data != NULL)
		{
			GlPlugin *plugin = GL_PLUGIN(mkt_atom_get_parent(MKT_ATOM(curr->data)));
			if( plugin != NULL && GL_IS_PLUGIN(plugin))
			{
				GtkWidget *button = mkt_window_get_gtk_container(MKT_WINDOW(curr->data));
				GtkWidget *parent = gtk_widget_get_parent(button);
				gtk_widget_ref(button);
				if(parent)gtk_container_remove(GTK_CONTAINER(parent),button);
				if(gl_plugin_is_search_muster(plugin,muster) && gl_level_manager_is_tru_user(mktAPGet(plugin,"level",uint)))
				{
					GdkPoint p;p.x = 0;p.y=0;
					if(gl_level_notebook_get_free_position(notebook,GUI_USER_LAST_TYPE,&p))
					{
						gtk_container_add(GTK_CONTAINER(notebook->priv->table[GUI_USER_LAST_TYPE].container[p.x][p.y]),button);
						mkt_window_show(MKT_WINDOW(curr->data));
					}
				}
			}
		}
		curr = curr -> next ;
	}
}


void
gl_level_notebook_search_signal_cb ( MktWindow * window, const gchar *text , GlLevelNotebook *lnotebook )
{
	//TEST:g_debug("Test new search text:%s|",text);
	gl_level_notebook_set_search(lnotebook,text);
}


/*
 * gl-update.h
 *
 *  Created on: 22.02.2013
 *      Author: sascha
 */

/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * dbusexample
 * Copyright (C) sascha 2012 <sascha@sascha-ThinkPad-X61>
 *
dbusexample is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dbusexample is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <wait.h>

#include <mkt-value.h>

#include "gl-update.h"
#include "gl-indicate.h"
#include "gl-connection.h"
#include "gl-action-widget.h"
#include "gl-translation.h"
#include "gl-level-manager.h"
#include "gl-widget-option.h"

#include <market-translation.h>
#include "../lgdm-status.h"

#define USB_UPDATE_PATH  "/lar/usbstick"
#define UPDATE_SELECTION "/lar/var/lar-path-selection"

enum
{
	GL_UPDATE_STATUS_NEW_PACKAGE,
	GL_UPDATE_STATUS_DEFAULT,
	GL_UPDATE_NEED_RESTART,
	GL_UPDATE_STATUS_LAST

};



enum
{
	GL_UPDATE_TREE_COLUMN_NAME,
	GL_UPDATE_TREE_COLUMN_ACTION,
	GL_UPDATE_TREE_COLUMN_PACKAGE,
	GL_UPDATE_TREE_COLUMN_VERSION,
	GL_UPDATE_TREE_COLUMN_DESCRIPTION,
	GL_UPDATE_TREE_COLUMN_LAST,

};


struct _GlUpdatePrivate
{
	GlIndicate   *start_indicate;
	gchar        *status [ GL_UPDATE_STATUS_LAST ];
	GList        *package;
	GList        *package_to_install;
	GtkWidget    *tree;

	GtkWidget    *accept;
	GtkWidget    *cancel;

	GtkTreeIter   to_insall;
	GtkTreeIter   to_remove;


};


#define GL_UPDATE_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_UPDATE, GlUpdatePrivate))

G_DEFINE_TYPE (GlUpdate, gl_update, GL_TYPE_PLUGIN);

enum
{
	GL_UPDATE_PROP0,

};

enum
{
	GL_UPDATE_LAST_SIGNAL,
};


//static guint gl_log_signals[GL_LOG_LAST_SIGNAL] = { 0 };



void
gl_update_start_process_wait (gchar *program , gchar *ar,...)
{
	char **argv = &ar;
	pid_t pid ;
	pid = fork();
	int status ;
	if(pid != 0)
	{
		printf("Start programm %s pid %d\n",program,pid);
	}
	else
	{
		execvp(program,argv);
		abort();
	}
	waitpid (pid, &status, 0);
}


static void
gl_update_set_all_childs ( GtkTreeModel *model,GtkTreeIter *iter , gboolean set  )
{
	if(!gtk_tree_store_iter_is_valid(GTK_TREE_STORE(model),iter)) return ;
	gtk_tree_store_set(GTK_TREE_STORE (model),iter,GL_UPDATE_TREE_COLUMN_ACTION,set,-1);
	gint   n_childs =  gtk_tree_model_iter_n_children( model,iter);
	gint count = 0;
	for(count = 0;count<n_childs;count++)
	{
		GtkTreeIter child ;
		if(gtk_tree_model_iter_nth_child ( model,&child,iter,count))
		{
			gl_update_set_all_childs(model,&child,set);
		}
	}
}

static void
gl_update_check_need_install(GlUpdate *update)
{
	g_return_if_fail(update != NULL);
	g_return_if_fail(GL_IS_UPDATE(update));
	if(update->priv->package_to_install)mkt_list_free_full(update->priv->package_to_install,g_free);
	update->priv->package_to_install = NULL;
	GtkTreeModel *model  = gtk_tree_view_get_model(GTK_TREE_VIEW(update->priv->tree));

	gint   n_childs =  gtk_tree_model_iter_n_children( model,&update->priv->to_insall);
	gint count = 0;
	gboolean need_install = FALSE;
	for(count = 0;count<n_childs;count++)
	{
		GtkTreeIter child;
		if(gtk_tree_model_iter_nth_child ( model,&child,&update->priv->to_insall,count))
		{
			gboolean value ;
			gchar *package;
			gtk_tree_model_get (model,&child,GL_UPDATE_TREE_COLUMN_NAME,&package,GL_UPDATE_TREE_COLUMN_ACTION,&value,-1);
			if (value && package)
			{
				update->priv->package_to_install = g_list_append(update->priv->package_to_install,package);
				need_install= TRUE;
			}
		}
	}
	if(update->priv->accept)gtk_widget_set_sensitive(update->priv->accept,need_install);
	if(update->priv->cancel)gtk_widget_set_sensitive(update->priv->cancel,need_install);
}


void
gl_update_cell_toggled_callback (GtkCellRendererToggle *cell, gchar *path_string, gpointer  user_data)
{
	//	g_debug("gl_data_base_cell_toggled_callback");
	g_return_if_fail(user_data != NULL);
	g_return_if_fail(GL_IS_UPDATE(user_data));
	GlUpdate *update = GL_UPDATE(user_data);
	g_return_if_fail(update->priv->tree!=NULL);
	GtkTreeModel *model  = gtk_tree_view_get_model(GTK_TREE_VIEW(update->priv->tree));
	g_return_if_fail(model!=NULL);
	GtkTreePath *to_install_path = NULL;
	GtkTreePath *obj_path        = gtk_tree_path_new_from_string (path_string);
	g_return_if_fail(obj_path!=NULL);
	if(gtk_tree_store_iter_is_valid(GTK_TREE_STORE(model),&update->priv->to_insall))
	{
		to_install_path = gtk_tree_model_get_path (model,&update->priv->to_insall);

		GtkTreeIter obj_iter;
		gtk_tree_model_get_iter_from_string(model,&obj_iter,path_string);
		if(0==gtk_tree_path_compare(to_install_path,obj_path))
		{
			gboolean set;
			gtk_tree_model_get (model,&update->priv->to_insall,GL_UPDATE_TREE_COLUMN_ACTION,&set,-1);
			gboolean __ON = !set;
			gl_update_set_all_childs(model,&update->priv->to_insall,__ON);
		}
		else if( gtk_tree_path_is_ancestor (to_install_path,obj_path ) )
		{
			gboolean set;
			gtk_tree_model_get (model,&obj_iter,GL_UPDATE_TREE_COLUMN_ACTION,&set,-1);
			gboolean __ON = !set;
			gl_update_set_all_childs(model,&obj_iter,__ON);
		}
		if(to_install_path)	 gtk_tree_path_free(to_install_path);
		gl_update_check_need_install(update);
	}
	gtk_tree_path_free(obj_path);
}


static gboolean
gl_update_package_is_installed ( GlUpdate *update , const gchar *name )
{
	gchar  *contents  = NULL;
	gsize   length    = 0;
	GError *error     = NULL;
	gchar **strs      = NULL;
	gboolean is_test  = FALSE;
	if( g_file_get_contents ("/lar/var/lar-path-selection",&contents,&length,&error))
	{
		strs = g_strsplit_set(contents,"\n",-1);
		gint i = 0;
		for(i=0;strs!=NULL && strs[i]!=NULL;i++)
		{
			if(strlen(strs[i])>10)
			{
				if(g_str_has_prefix(strs[i],name))
				{
					is_test = TRUE;
				}
			}
		}
		if(strs)g_strfreev(strs);
		g_free(contents);
	}
	return is_test;
}

static void
gl_update_load_tree_new ( GlUpdate *update )
{
	g_return_if_fail(update!=NULL);
	g_return_if_fail(GL_IS_UPDATE(update));
	g_return_if_fail(update->priv->tree != NULL);
	GList *list;
	GList *columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(update->priv->tree ));
	list = columns;
	while(list!= NULL)
	{
		gtk_tree_view_remove_column(GTK_TREE_VIEW(update->priv->tree ),GTK_TREE_VIEW_COLUMN( list->data));
		list = list->next;
	}
	g_list_free(columns);
	if(update->priv->package_to_install)mkt_list_free_full(update->priv->package_to_install,g_free);
	update->priv->package_to_install = NULL;
	GtkTreeViewColumn *col;
	GtkCellRenderer   *renderer;
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col,_TR_("TRANSLATION_static_Update_LarPackage","LAR-Package"));
	gtk_tree_view_column_set_alignment(col,0.1);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col,renderer,TRUE);
	gtk_tree_view_column_add_attribute(col,renderer,"text",GL_UPDATE_TREE_COLUMN_NAME);
	gtk_tree_view_append_column(GTK_TREE_VIEW(update->priv->tree),col);

	renderer = gtk_cell_renderer_toggle_new();
	g_signal_connect(renderer,"toggled",G_CALLBACK(gl_update_cell_toggled_callback),update);
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes ("  ",
			renderer,
			"active", GL_UPDATE_TREE_COLUMN_ACTION,
			NULL );
	gtk_tree_view_column_set_fixed_width ( GTK_TREE_VIEW_COLUMN (column), 30 );
	gtk_tree_view_column_set_sizing ( GTK_TREE_VIEW_COLUMN (column),
			GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_alignment(GTK_TREE_VIEW_COLUMN (column),0.1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(update->priv->tree),column);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col,_TR_("TRANSLATION_static_Update_DebianPackage","Package"));
	gtk_tree_view_column_set_alignment(col,0.1);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col,renderer,TRUE);
	gtk_tree_view_column_add_attribute(col,renderer,"text",GL_UPDATE_TREE_COLUMN_PACKAGE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(update->priv->tree),col);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col,_TR_("TRANSLATION_static_Update_Version","Version"));
	gtk_tree_view_column_set_alignment(col,0.1);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col,renderer,TRUE);
	gtk_tree_view_column_add_attribute(col,renderer,"text",GL_UPDATE_TREE_COLUMN_VERSION);
	gtk_tree_view_append_column(GTK_TREE_VIEW(update->priv->tree),col);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col,_TR_("TRANSLATION_static_Update_Description","Description"));
	gtk_tree_view_column_set_alignment(col,0.1);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col,renderer,TRUE);
	gtk_tree_view_column_add_attribute(col,renderer,"text",GL_UPDATE_TREE_COLUMN_DESCRIPTION);
	gtk_tree_view_append_column(GTK_TREE_VIEW(update->priv->tree),col);

	GtkTreeStore       *treestore;
	treestore = gtk_tree_store_new( GL_UPDATE_TREE_COLUMN_LAST,G_TYPE_STRING,G_TYPE_BOOLEAN,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);

	gchar  *contents  = NULL;
	gsize   length    = 0;
	GError *error     = NULL;
	gchar **strs      = NULL;
	if( g_file_get_contents ("/lar/var/lar-path-selection",&contents,&length,&error))
	{
		strs = g_strsplit_set(contents,"\n",-1);
		gint i = 0;
		for(i=0;strs!=NULL && strs[i]!=NULL;i++)
		{
			if(strlen(strs[i])>10)
			{
				GtkTreeIter iter ;
				gchar **parts = g_strsplit_set(strs[i]," 	",-1);
				if(parts)
				{
					if( !gtk_tree_store_iter_is_valid(treestore,&update->priv->to_remove))
					{
						gtk_tree_store_append(treestore,&update->priv->to_remove,NULL);
						gtk_tree_store_set(treestore,&update->priv->to_remove,GL_UPDATE_TREE_COLUMN_NAME,_TR_("TRANSLATE_column_package_installed","Installed"),1,TRUE,-1);
					}
					gtk_tree_store_append(treestore,&iter,&update->priv->to_remove);
					if(parts[0])
					{
						gtk_tree_store_set(treestore,&iter,GL_UPDATE_TREE_COLUMN_NAME,parts[0],GL_UPDATE_TREE_COLUMN_ACTION,TRUE,-1);

						int i ;
						int column = 2;
						for(i=1;parts!=NULL&&parts[i]!=NULL;i++)
						{
							if(strlen(parts[i])>2)
							{
								gtk_tree_store_set(treestore,&iter,column,parts[i],-1);
								column++;
							}
						}
					}
					g_strfreev(parts);
				}
			}
		}
		if(strs)g_strfreev(strs);
		g_free(contents);
	}

	GList *l = NULL ;
	for(l=update->priv->package;l!=NULL;l=l->next)
	{

		GtkTreeIter iter ;
		gchar *file = (gchar * ) l->data;
		if( !gtk_tree_store_iter_is_valid(treestore,&update->priv->to_insall) )
		{
			gtk_tree_store_append(treestore,&update->priv->to_insall,NULL);
			gtk_tree_store_set(treestore,&update->priv->to_insall,GL_UPDATE_TREE_COLUMN_NAME,_TR_("TRANSLATE_column_package_new","Updates"),-1);

		}
		gtk_tree_store_append(treestore,&iter,&update->priv->to_insall);
		gtk_tree_store_set(treestore,&iter,GL_UPDATE_TREE_COLUMN_NAME,file,GL_UPDATE_TREE_COLUMN_ACTION,FALSE,
				                           GL_UPDATE_TREE_COLUMN_PACKAGE,_TR_("TRANSLATE_column_package_not_install","not installed"),
				                           GL_UPDATE_TREE_COLUMN_VERSION,_TR_("TRANSLATE_column_package_not_install","not installed")
				                          ,-1);
		gtk_tree_store_set(treestore,&iter,GL_UPDATE_TREE_COLUMN_DESCRIPTION,_TR_("TRANSLATE_column_package_install_new_disc","install new LAR software package..."),-1);
	}
	gtk_tree_view_set_model(GTK_TREE_VIEW(update->priv->tree),GTK_TREE_MODEL(treestore));
	g_object_unref( treestore );
}


static void
gl_update_manager_on_indicate_clicked ( GlIndicate *indicate , GlUpdate *update )
{
	//TEST:g_debug("gl_logs_start_on_indicate_clicked");
	g_return_if_fail(update!=NULL);
	g_return_if_fail(GL_IS_UPDATE(update));
	mkt_window_show(MKT_WINDOW(update));
}


static void
gl_update_manager_load_package_from_usb ( GlUpdate *update )
{
	g_return_if_fail(update!=NULL);
	g_return_if_fail(GL_IS_UPDATE(update));
	if(update->priv->package) mkt_list_free_full(update->priv->package,g_free);
	update->priv->package = NULL;
	GDir *dir;
	GError *error = NULL;
	gboolean new_package=FALSE;
	dir = g_dir_open (USB_UPDATE_PATH, 0, &error );
	if(dir== NULL || error != NULL)
	{
		g_critical("%s", error->message );
		g_error_free(error);
		return ;
	}
	const gchar *name = NULL;
	while( (name = g_dir_read_name (dir)) )
	{
		if(2 < strlen ( name ))
		{
			gchar *package_name      = g_build_path("/",USB_UPDATE_PATH,name,NULL);
			if(g_file_test(package_name,G_FILE_TEST_EXISTS) && g_str_has_suffix(package_name,".lar"))
			{
				if(!gl_update_package_is_installed(update,name))
				{
					update->priv->package = g_list_append(update->priv->package,g_strdup(name));
					new_package = TRUE;
				}
			}
			else
			{
				g_free(package_name);
			}
		}
	}
	g_dir_close (dir);
	if(new_package)
	{
		gl_indicate_set_indicate_profile(update->priv->start_indicate,update->priv->status[GL_UPDATE_STATUS_NEW_PACKAGE]);
		gl_indicate_start(update->priv->start_indicate);
	}
	else
	{
		gl_indicate_stop(update->priv->start_indicate);
	}
}

void
gl_update_manager_mount_usb_signal_cb  ( GlLevelManager * manager,GlUpdate *update )
{
	//g_debug("UPDATE_MANAGER TEST .... MOUNT USB");
	g_return_if_fail(update!=NULL);
	g_return_if_fail(GL_IS_UPDATE(update));
	gl_update_manager_load_package_from_usb(update);
	gl_update_load_tree_new(GL_UPDATE(update));

}

void
gl_update_manager_umount_usb_signal_cb ( GlLevelManager * manager,GlUpdate *update )
{
	//g_debug("UPDATE_MANAGER TEST .... UMOUNT USB");
	g_return_if_fail(update!=NULL);
	g_return_if_fail(GL_IS_UPDATE(update));
	if(update->priv->package) mkt_list_free_full(update->priv->package,g_free);
	update->priv->package = NULL;
	gl_update_load_tree_new(GL_UPDATE(update));
	gl_indicate_stop(update->priv->start_indicate);
	//gl_indicate_set_indicate_profile(update->priv->start_indicate,update->priv->status[GL_UPDATE_STATUS_DEFAULT]);
}



void
gl_update_change_change_level_signal ( GlLevelManager *lmanager , GlUpdate *update )
{
	GtkWidget *restart   = mkt_window_find_widget(MKT_WINDOW(update),"TRANSLATE_button_UpdataModule_restart");
	if(gl_level_manager_is_tru_user(GUI_USER_SERVICE_TYPE))
	{
		if(restart) gtk_widget_set_sensitive(restart,TRUE);
	}
	else if(gl_level_manager_is_tru_user(GUI_USER_SUPER_OPERATOR_TYPE))
	{
		if(restart) gtk_widget_set_sensitive(restart,TRUE);
	}
	else
	{
		if(restart) gtk_widget_set_sensitive(restart,FALSE);
	}


}


void
gl_update_start ( MktWindow *window ,gpointer user_data)
{
	g_return_if_fail(window!=NULL);
	g_return_if_fail(GL_IS_UPDATE(window));
	GlUpdate *update = GL_UPDATE(window);
	gl_update_manager_load_package_from_usb(update);
	gl_update_load_tree_new(GL_UPDATE(update));
}

static void
gl_update_init (GlUpdate *update)
{
    GlUpdatePrivate *priv = GL_UPDATE_GET_PRIVATE(update);
    update->priv   = priv;
    update->priv->package = NULL;
    update->priv->package_to_install = NULL;
    update->priv->start_indicate = gl_indicate_new("com.lar.GlIndicate.Update",GL_INDICATE_NO_ASK);
    GtkWidget *indicate_label = gtk_label_new(_TR_("TRANSLATE_label_UpdateManager_status","update"));
    gl_widget_option_set_name(G_OBJECT(indicate_label),"TRANSLATE_label_UpdateManager_status");
   	gtk_misc_set_alignment(GTK_MISC(indicate_label),0.01,0.5);
    gtk_widget_modify_font(GTK_WIDGET(indicate_label),pango_font_description_from_string("Oreal 8"));
    gtk_widget_show(indicate_label);
    gl_indicate_set_indicate_box(update->priv->start_indicate,indicate_label);
    gl_indicate_set_indicate_profile(update->priv->start_indicate,update->priv->status[GL_UPDATE_STATUS_NEW_PACKAGE]);
   	g_signal_connect( update->priv->start_indicate,"click_start",G_CALLBACK(gl_update_manager_on_indicate_clicked),update);
 	g_signal_connect (gl_level_manager_get_static(),"mount_usb",G_CALLBACK(gl_update_manager_mount_usb_signal_cb),update);
 	g_signal_connect (gl_level_manager_get_static(),"umount_usb",G_CALLBACK(gl_update_manager_umount_usb_signal_cb),update);

 	g_signal_connect(gl_level_manager_get_static(),"change_gui_level",G_CALLBACK(gl_update_change_change_level_signal),update);

	g_signal_connect(update,"window_show",G_CALLBACK(gl_update_start),NULL);

}

static void
gl_update_finalize (GObject *object)
{
	GlUpdate *update = GL_UPDATE(object);
	if(update->priv->package_to_install)mkt_list_free_full(update->priv->package_to_install,g_free);
	if(update->priv->package)mkt_list_free_full(update->priv->package,g_free);
	G_OBJECT_CLASS (gl_update_parent_class)->finalize (object);
}



void
gl_update_set_property(  GObject        *object,
		guint           prop_id,
		const GValue   *value,
		GParamSpec     *pspec)
{
	//GlUpdate *update = GL_UPDATE(object);
	switch(prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}
void
gl_update_get_property(  GObject        *object,
		guint           prop_id,
		GValue         *value,
		GParamSpec     *pspec)
{
	//GlUpdate *update = GL_UPDATE(object);
	switch(prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}


void
gl_update_realized ( MktWindow *window )
{
	GlUpdate *update = GL_UPDATE(window);
	update->priv->status[GL_UPDATE_STATUS_NEW_PACKAGE] = mkt_atom_build_path(MKT_ATOM(update),"~/icons/apport.svg");
	update->priv->status[GL_UPDATE_STATUS_DEFAULT] = mkt_atom_build_path(MKT_ATOM(update),"~/icons/apport.svg");
	/*gl_indicate_set_indicate_profile(update->priv->start_indicate,update->priv->status[GL_UPDATE_STATUS_DEFAULT]);
	gl_indicate_start(update->priv->start_indicate);*/
	update->priv->accept = mkt_window_find_widget(window,"TRANSLATE_button_GlUpdate_Accept");
	update->priv->cancel = mkt_window_find_widget(window,"TRANSLATE_button_GlUpdate_Cancel");


	if(update->priv->accept)gtk_widget_set_sensitive(update->priv->accept,FALSE);
	if(update->priv->cancel)gtk_widget_set_sensitive(update->priv->cancel,FALSE);
}


static void
gl_update_class_init (GlUpdateClass *klass)
{
	GObjectClass*   object_class  = G_OBJECT_CLASS (klass);
	MktWindowClass *mktdraw_class = MKT_WINDOW_CLASS(klass);
	g_type_class_add_private (klass, sizeof (GlUpdatePrivate));
	object_class->finalize          = gl_update_finalize;
	object_class->set_property      = gl_update_set_property;
	object_class->get_property      = gl_update_get_property;
	mktdraw_class ->realized        = gl_update_realized;

}

void
gl_update_realize_update_package_tree (  MktWindow *window, GtkWidget *tree )
{
	//TEST:g_debug( "gl_log_system_realize_system_log for %s %s",gl_widget_option_get_name(tree),G_OBJECT_TYPE_NAME(tree));
	g_return_if_fail(window != NULL );
	g_return_if_fail(GL_IS_UPDATE(window));
	GL_UPDATE(window)->priv->tree = tree;
	gl_update_manager_load_package_from_usb(GL_UPDATE(window));
	gl_update_load_tree_new(GL_UPDATE(window));
}

gboolean
gl_update_shotdown_clicked(GtkWidget *widget ,gpointer user_data)
{
	g_return_val_if_fail(user_data != NULL,FALSE );
	g_return_val_if_fail(GL_IS_UPDATE(user_data),FALSE);
	return TRUE;
}

gboolean
gl_update_restart_clicked(GtkWidget *widget ,gpointer user_data)
{
	//g_debug("gl_update_restart_clicked");
	g_return_val_if_fail(user_data != NULL,FALSE );
	g_return_val_if_fail(GL_IS_UPDATE(user_data),FALSE);
	gl_update_load_tree_new(GL_UPDATE(user_data));
	//GlUpdate *update = GL_UPDATE(user_data);
	return TRUE;
}


gboolean
gl_update_wait_update ( gpointer user_data )
{
	GlUpdate *update = GL_UPDATE(user_data);
	if(update->priv->tree)  gtk_widget_set_sensitive(update->priv->tree,TRUE);
	gl_update_manager_load_package_from_usb(update);
	gl_update_load_tree_new(GL_UPDATE(user_data));

	return FALSE;
}

gboolean
gl_update_install_clicked_cb (  GtkWidget *widget ,gpointer user_data )
{
	//g_debug("gl_update_install_clicked_cb");
	g_return_val_if_fail(user_data != NULL,FALSE );
	g_return_val_if_fail(GL_IS_UPDATE(user_data),FALSE);
	GlUpdate *update = GL_UPDATE(user_data);
	if(update->priv->accept)gtk_widget_set_sensitive(update->priv->accept,FALSE);
	if(update->priv->cancel)gtk_widget_set_sensitive(update->priv->cancel,FALSE);
	if(update->priv->tree)  gtk_widget_set_sensitive(update->priv->tree,FALSE);
	GList *l=NULL;
	for(l=update->priv->package_to_install;l!=NULL;l=l->next)
	{
		if(l->data)
		{
			gchar *pck_name = (gchar *) l->data;
			//g_debug("pck_name=%s",pck_name);
			gl_update_start_process_wait("/lar/bin/larinstall","/bin/bash","bash","/lar/bin/larinstall",pck_name,NULL);
		}
	}
	gtk_timeout_add(900,gl_update_wait_update,user_data);

	return TRUE;
}

gboolean
gl_update_cancel_clicked_cb (  GtkWidget *widget ,gpointer user_data )
{
	//g_debug("gl_update_cancel_clicked_cb");
	g_return_val_if_fail(user_data != NULL,FALSE );
	g_return_val_if_fail(GL_IS_UPDATE(user_data),FALSE);
	GlUpdate *update = GL_UPDATE(user_data);
	if(update->priv->accept)gtk_widget_set_sensitive(update->priv->accept,FALSE);
	if(update->priv->cancel)gtk_widget_set_sensitive(update->priv->cancel,FALSE);
	gl_update_manager_load_package_from_usb(user_data);
	gl_update_load_tree_new(GL_UPDATE(user_data));
	return TRUE;
}

gboolean
gl_update_shutdown_clicked_cb ( GtkWidget *widget ,gpointer user_data )
{
	//g_debug("gl_update_cancel_clicked_cb");
	g_return_val_if_fail(user_data != NULL,FALSE );
	g_return_val_if_fail(GL_IS_UPDATE(user_data),FALSE);
	//GlUpdate *update = GL_UPDATE(user_data);
	gl_update_start_process_wait ("/lar/bin/shotdown","/bin/bash","bash","/lar/bin/shotdown",NULL);
	return TRUE;
}

gboolean
gl_update_restart_clicked_cb ( GtkWidget *widget ,gpointer user_data )
{
	//g_debug("gl_update_cancel_clicked_cb");
	g_return_val_if_fail(user_data != NULL,FALSE );
	g_return_val_if_fail(GL_IS_UPDATE(user_data),FALSE);
	//GlUpdate *update = GL_UPDATE(user_data);
	gl_update_start_process_wait ("/lar/bin/restart","/bin/bash","bash","/lar/bin/restart",NULL);
	return TRUE;
}

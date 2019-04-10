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

#include "gl-wizard-manager.h"
#include "gl-indicate.h"
#include "gl-connection.h"
#include "gl-action-widget.h"
#include "gl-translation.h"
#include "gl-wizard.h"
#include "gl-widget-option.h"

#include <mkt-collector.h>
#include <mkt-value.h>
#include <market-translation.h>

#include <string.h>
#include "../lgdm-status.h"



struct _GlWizardManagerPrivate
{
	GtkWidget   *wizards_box;
	GlIndicate  *start_indicate;
	GList       *wizards;
	GList       *usb_wizards;

};


#define GL_WIZARD_MANAGER_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_WIZARD_MANAGER, GlWizardManagerPrivate))

G_DEFINE_TYPE (GlWizardManager, gl_wizard_manager, MKT_TYPE_WINDOW);



#define USB_WIZARD_PATH  "/lar/usbstick/wizards"


enum
{
	GL_WIZARD_MANAGER_PROP0,
};

enum
{
	GL_WIZARD_MANAGER_LAST_SIGNAL
};


//static guint gl_log_signals[GL_LOG_LAST_SIGNAL] = { 0 };

static void
gl_wizard_manager_start_wizard_clicked ( GtkWidget *widget , GlWizard *wizard )
{
	g_return_if_fail(wizard!=NULL);
	g_return_if_fail(GL_IS_WIZARD(wizard));
	gl_wizard_start( wizard );
}


static void
gl_wizard_manager_hide_manager  ( GtkWidget *widget , GlWizardManager *wm )
{
	g_return_if_fail(wm!=NULL);
	g_return_if_fail(GL_IS_WIZARD_MANAGER(wm));
	mkt_window_hide(MKT_WINDOW(wm));
}

static void
gl_wizard_manager_realize_box ( GlWizardManager *wm )
{
	//g_debug("gl_wizard_manager_realize_box");
	g_return_if_fail(wm!=NULL);
	g_return_if_fail(GL_IS_WIZARD_MANAGER(wm));
	gl_widget_option_remove_all_childs(wm->priv->wizards_box);
	gint find_wizards = 0;
	GList *l = NULL ;
	for ( l= wm->priv->wizards;l!=NULL;l=l->next)
	{
		if(l->data && GL_IS_WIZARD(l->data))
		{
			find_wizards++;

			GtkWidget *startb   = gtk_button_new();
			gtk_widget_set_size_request(startb,-1,30);
			g_signal_connect(startb,"clicked",G_CALLBACK(gl_wizard_manager_start_wizard_clicked),l->data);
			g_signal_connect(startb,"clicked",G_CALLBACK(gl_wizard_manager_hide_manager),wm);
			GtkWidget *hbox     = gtk_hbox_new ( FALSE,1 );
			GtkWidget *image    = gtk_image_new( );
			gchar *tr_id        = g_strdup_printf("TRANSLATE_wizard_label_%s",mkt_atom_get_id(MKT_ATOM(l->data)));
			GtkWidget *label    = gtk_label_new(_TR_(tr_id,mkt_atom_get_id(MKT_ATOM(l->data))));
			gtk_box_pack_start(GTK_BOX(hbox),image,FALSE,FALSE,1);
			gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,1);
			//gchar *icon_path = mkt_atom_get_avatar_path(MKT_ATOM(l->data));
			//g_debug("Wizard icon path %s",icon_path);
			/*if(g_file_test(icon_path,G_FILE_TEST_EXISTS)&& g_str_has_suffix(icon_path,".png"))
			{

				gtk_image_set_from_file(GTK_IMAGE(image),icon_path);
			}*/
			//g_debug("TEST11135 %s",icon_path);
			gtk_container_add(GTK_CONTAINER(startb),hbox);
			gl_widget_option_set_name(G_OBJECT(label),tr_id);
			gl_binding_insert_widget(label);
			gtk_widget_show(image);
			gtk_widget_show(label);
			gtk_widget_show(hbox);
			gtk_widget_show(startb);
			gtk_box_pack_start(GTK_BOX(wm->priv->wizards_box),startb,FALSE,FALSE,3);
		}
	}
	if( find_wizards > 0 )gl_indicate_start(wm->priv->start_indicate);
	else gl_indicate_stop(wm->priv->start_indicate);
	//g_debug("TEST33333");
}



static void
gl_wizard_manager_on_indicate_clicked ( GlIndicate *indicate , GlWizardManager *wm)
{
	//TEST:g_debug("gl_logs_start_on_indicate_clicked");
	g_return_if_fail(wm!=NULL);
	g_return_if_fail(GL_IS_WIZARD_MANAGER(wm));
	guint len = g_list_length(wm->priv->wizards);
	if(len > 0) mkt_window_show(MKT_WINDOW(wm));
}

static void
gl_wizard_manager_atom_will_destroy ( MktAtom *atom , GlWizardManager *wm )
{
	if(GL_IS_WIZARD(atom))
	{
		guint len = 0;
		if(wm->priv->wizards!=NULL)
		{
			len = g_list_length(wm->priv->wizards);
			wm->priv->wizards = g_list_remove(wm->priv->wizards,atom);
		}
		if(len >= 1)
		{
			wm->priv->wizards = NULL;
			gl_indicate_stop(wm->priv->start_indicate);
		}
		gl_wizard_manager_realize_box(wm);
	}
}

static void
gl_wizard_manager_new_atom_cb ( MktCollector *collctor , MktAtom *atom , GlWizardManager *wm )
{
	if(GL_IS_WIZARD(atom))
	{
		//g_debug("Add and realize %s",mkt_atom_get_id(atom));
		wm->priv->wizards = g_list_append(wm->priv->wizards,atom);
		g_signal_connect ( atom ,"destroy", G_CALLBACK(gl_wizard_manager_atom_will_destroy) , wm );
		gl_wizard_manager_realize_box(wm);
	}
}


static gboolean
gl_wizard_manager_load_atom (GlWizardManager *wm ,const gchar *filepath )
{
	g_message ( "main build load atom %s",filepath);
	g_return_val_if_fail(GL_IS_WIZARD_MANAGER(wm),FALSE);
	MktAtom *atom = mkt_atom_object_new_from_file ( filepath );
	if(atom!=NULL) 	return TRUE;
	return FALSE;
}

void
gl_wizard_manager_mount_usb_signal_cb            (GlLevelManager * manager,GlWizardManager *wm )
{
	g_return_if_fail(wm!=NULL);
	g_return_if_fail(GL_IS_WIZARD_MANAGER(wm));
	if(wm->priv->usb_wizards) mkt_list_free_full(wm->priv->usb_wizards,g_free);
	GDir *dir;
	GError *error = NULL;
	dir = g_dir_open (USB_WIZARD_PATH, 0, &error );
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
			gchar *module_name      = g_build_path("/",USB_WIZARD_PATH,name,NULL);
			//g_debug("Test path %s",module_name);
			if(g_file_test(module_name,G_FILE_TEST_IS_DIR))
			{
				mkt_collector_load_atoms_from_directory(module_name);
			}
			g_free(module_name);
		}
	}
	g_dir_close (dir);
}

void
gl_wizard_manager_change_level_cb            (GlLevelManager * manager,GlWizardManager *wm )
{
	g_return_if_fail(wm!=NULL);
	g_return_if_fail(GL_IS_WIZARD_MANAGER(wm));
	gl_wizard_manager_realize_box(wm);

}

static void
gl_wizard_manager_init (GlWizardManager *wm)
{
    GlWizardManagerPrivate *priv = GL_WIZARD_MANAGER_GET_PRIVATE(wm);
    wm->priv   = priv;
    wm->priv->wizards = NULL;
    wm->priv->start_indicate = gl_indicate_new("com.lar.GlIndicate.SystemCheck",GL_INDICATE_NO_ASK);

    GtkWidget *indicate_label = gtk_label_new(_TR_("TRANSLATE_label_WizardsManager_wizards_status","Wizards"));
   	gl_widget_option_set_name(G_OBJECT(indicate_label),"TRANSLATE_label_WizardsManager_wizards_status");
  	gtk_misc_set_alignment(GTK_MISC(indicate_label),0.01,0.5);
   	gtk_widget_modify_font(GTK_WIDGET(indicate_label),pango_font_description_from_string("Oreal 8"));
   	gtk_widget_show(indicate_label);
   	gl_indicate_set_indicate_box(wm->priv->start_indicate,indicate_label);
   	g_signal_connect( wm->priv->start_indicate,"click_start",G_CALLBACK(gl_wizard_manager_on_indicate_clicked),wm);
    g_signal_connect ( mkt_collector_get_static(),"new-atom",G_CALLBACK(gl_wizard_manager_new_atom_cb),wm);
	g_signal_connect (gl_level_manager_get_static(),"mount_usb",G_CALLBACK(gl_wizard_manager_mount_usb_signal_cb),wm);
	g_signal_connect (gl_level_manager_get_static(),"change_gui_level",G_CALLBACK(gl_wizard_manager_change_level_cb),wm);
}

static void
gl_wizard_manager_finalize (GObject *object)
{
	//GlWizardManager *update = GL_WIZARD_MANAGER(object);
	G_OBJECT_CLASS (gl_wizard_manager_parent_class)->finalize (object);
}

void
gl_wizard_manager_set_property(  GObject        *object,
		guint           prop_id,
		const GValue   *value,
		GParamSpec     *pspec)
{
	//GlWizardManager *wm = GL_WIZARD_MANAGER(object);
	switch(prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}
void
gl_wizard_manager_get_property(  GObject        *object,
		guint           prop_id,
		GValue         *value,
		GParamSpec     *pspec)
{
	//GlWizardManager *wm = GL_WIZARD_MANAGER(object);
	switch(prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}

void
gl_wizard_manager_realized ( MktWindow *window )
{
	GlWizardManager *wm = GL_WIZARD_MANAGER(window);
	gchar *start_indicate_profile = mkt_atom_build_path(MKT_ATOM(wm),"~/icons/help-hint-smal.png");
	gl_indicate_set_indicate_profile(wm->priv->start_indicate,start_indicate_profile);
	g_free(start_indicate_profile);
}

static void
gl_wizard_manager_class_init (GlWizardManagerClass *klass)
{
	GObjectClass*   object_class  = G_OBJECT_CLASS (klass);
	MktWindowClass *mktdraw_class = MKT_WINDOW_CLASS(klass);
	g_type_class_add_private (klass, sizeof (GlWizardManagerPrivate));
	object_class->finalize          = gl_wizard_manager_finalize;
	object_class->set_property      = gl_wizard_manager_set_property;
	object_class->get_property      = gl_wizard_manager_get_property;
	mktdraw_class->realized         = gl_wizard_manager_realized;
}

void
gl_wizard_manager_realize_wizards_box (  MktWindow *window, GtkWidget *box )
{
	//TEST:g_debug( "gl_log_system_realize_system_log for %s %s",gl_widget_option_get_name(tree),G_OBJECT_TYPE_NAME(tree));
	g_return_if_fail(window != NULL );
	g_return_if_fail(GL_IS_WIZARD_MANAGER(window));
	g_return_if_fail(box != NULL);
	GL_WIZARD_MANAGER(window)->priv->wizards_box = box;
	gl_wizard_manager_realize_box(GL_WIZARD_MANAGER(window));
}


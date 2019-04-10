/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-wizard.c
 * Copyright (C) sascha 2012 <sascha@sascha-desktop>
 * 
gl-wizard.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-wizard.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "gl-wizard.h"
#include "gl-widget-option.h"
#include "gl-action-widget.h"
#include "gl-plugin.h"

#include <mkt-collector.h>
#include <mkt-atom.h>

typedef struct
{
	guint width;
	guint height;
	guint x;
	guint y;


}GlWizardWin;

enum
{
	GL_WIZARD_TOP_WINDOW,
	GL_WIZARD_BOTTOM_WINDOW,
	GL_WIZARD_LAST_WINDOW,

};


#define GL_WIZARD_MAX_USER_MENUS 10

struct _GlWizardPrivate
{
	gchar      *step;
	gchar      *starter;
	gchar      *stop_window;

	gchar      *hide_show_window;
	gboolean    is_hidden;
	guint       start_level;

	gint        max_step;
	gint        current_step;

	guint       next_idle;
	guint       pre_idle;
	guint       hide_show_idle;

};


#define GL_WIZARD_KEY_FILE                    "wizard.run"

#define GL_WIZARD_MAIN_GROUP                  "WizardMain"
#define GL_WIZARD_MAIN_NICK                   "nick"
#define GL_WIZARD_MAIN_IMAGE                  "image"
#define GL_WIZARD_MAIN_BUTTON_IMAGE           "button_image"


#define GL_WIZARD_STEP_PREFIX                 "wizard_step"

#define GL_WIZARD_STEP_PROP_STOP              "wizard_step_stop"


#define GL_WIZARD_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_WIZARD, GlWizardPrivate))


enum
{
	GL_WIZARD_PROP0,
	GL_WIZZARD_STEP,
	GL_WIZZARD_START_WINDOW,
	GL_WIZZARD_STOP_WINDOW,
	GL_WIZZARD_HIDE_STOP_WINDOW,
	GL_WIZZARD_START_LEVEL

};



enum
{
	GL_WIZARD_CHANGE_STEP,
	LAST_SIGNAL
};


static guint gl_wizard_signals[LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE (GlWizard, gl_wizard, MKT_TYPE_ATOM);

/*static void
gl_wizard_free_user_menus (GlWizard *wizard )
{
	int i;
	for(i=0;i<GL_WIZARD_MAX_USER_MENUS;i++)
	{
		if(wizard->priv->user_menus[i]!=NULL&&GTK_IS_WIDGET(wizard->priv->user_menus[i]))
		{
			gtk_widget_destroy(wizard->priv->user_menus[i]);
			wizard->priv->user_menus[i]=NULL;
		}
	}
	if(wizard->priv->user_builder) g_object_unref(wizard->priv->user_builder);
	wizard->priv->user_builder = NULL;

}
static void
gl_wizard_load_user_menus(GlWizard *wizard)
{
	gl_wizard_free_user_menus(wizard);

	g_debug("Load user menu ... ");

	wizard->priv->user_builder = gtk_builder_new();
	GError *error = NULL;

	if(0==gtk_builder_add_from_file(wizard->priv->user_builder ,wizard->priv->xml_files,&error))
	{
		if(error)
		{
			g_warning("Load user xml menus %s failed :%d %s",wizard->priv->xml_files,error->code,error->message);
			g_error_free(error);
		}
		if(wizard->priv->user_builder) g_object_unref(wizard->priv->user_builder);
		wizard->priv->user_builder = NULL;
		return;
	}

	GSList *wlist = gtk_builder_get_objects(wizard->priv->user_builder);
	GSList *sl = NULL;
	int wcount = 0;
	for(sl=wlist ; sl!=NULL;sl=sl->next)
	{
		if(sl->data && GTK_IS_WIDGET(sl->data))
		{
			if(NULL==gtk_widget_get_parent(GTK_WIDGET(sl->data)))
			{
				if(GTK_IS_WINDOW(sl->data) && wcount < GL_WIZARD_MAX_USER_MENUS)
				{
					wizard->priv->user_menus[wcount]= GTK_WIDGET(sl->data);
					gtk_widget_show_all(wizard->priv->user_menus[wcount]);
					gint w,h;
					gtk_window_get_default_size(GTK_WINDOW(wizard->priv->user_menus[wcount]),&w,&h);
					gint x_root = 0,y_root = 0;
					gint width_root,height_root;
					GtkWidget *main_window = GTK_WIDGET(gl_system_get_object(  "main-window" ));
					g_return_if_fail(main_window!=NULL);
					gdk_window_get_root_origin(main_window->window,&x_root,&y_root);
					gtk_window_get_size(GTK_WINDOW(main_window),&width_root,&height_root);
					g_debug("ROOT :w=%d;h=%d;x=%d;y=%d",width_root,height_root,x_root,y_root);
					gtk_window_move(GTK_WINDOW (wizard->priv->user_menus[wcount]),x_root+w,y_root+h);
					g_debug ( " Width=%d Height=%d",w,h);
					wcount++;
				}
				else
				{
					gtk_widget_destroy(GTK_WIDGET(sl->data));
				}

			}
		}
	}
	gtk_builder_connect_signals(wizard->priv->user_builder,wizard);

	g_slist_free(wlist);
}*/


gchar*
gl_wizard_get_pre_step ( GlWizard *wizard  )
{
	g_return_val_if_fail(wizard!=NULL,NULL);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),NULL);
	gsize len = 0;
	gchar *pre_step=NULL;
	gchar **groups = g_key_file_get_groups(mkt_atom_get_object_file(MKT_ATOM(wizard)),&len);
	int i =0;
	for(i=0;i<len&&groups!=NULL&&groups[i]!=NULL;i++)
	{
		if(g_str_has_prefix(groups[i],GL_WIZARD_STEP_PREFIX))
		{
			if(wizard->priv->step==NULL)
			{
				pre_step = g_strdup(GL_WIZARD_STEP_PROP_STOP);
			}
			else
			{
				if(0==g_strcmp0(groups[i],wizard->priv->step))
				{
					if(i>0){pre_step = groups[i-1]!=NULL?g_strdup(groups[i-1]):NULL;}
				}
			}
		}
	}
	if(groups)g_strfreev(groups);
	return pre_step;
}

gchar*
gl_wizard_get_next_step ( GlWizard *wizard  )
{
	g_return_val_if_fail(wizard!=NULL,NULL);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),NULL);
	//g_debug("Test .. next_step  any key ");
	gsize  len = 0;
	gchar *next_step=NULL;
	gchar **groups = g_key_file_get_groups(mkt_atom_get_object_file(MKT_ATOM(wizard)),&len);
	int i =0;

	for(i=0;i<len&&groups!=NULL&&groups[i]!=NULL;i++)
	{
		if(g_str_has_prefix(groups[i],GL_WIZARD_STEP_PREFIX))
		{
			if(wizard->priv->step==NULL)
			{
				next_step = groups[i+1]!=NULL?g_strdup(groups[i+1]):NULL;
			}
			else
			{
				if(0==g_strcmp0(groups[i],wizard->priv->step))
				{
					if(i<len-1){next_step = groups[i+1]!=NULL?g_strdup(groups[i+1]):NULL;}
				}
			}
		}
	}
	if(groups)g_strfreev(groups);
	return next_step;
}


gboolean
gl_wizard_set_property_step(GlWizard * wizard ,const gchar *step ,const gchar *key)
{
	g_return_val_if_fail(wizard!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),FALSE);
	g_return_val_if_fail(step!=NULL,FALSE);
	g_return_val_if_fail(key!=NULL,FALSE);
	gchar *t_setring = NULL;
	GParamSpec *pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(wizard),key);
	if(pspec!=NULL)
	{
		switch(pspec->value_type)
		{
		case G_TYPE_INT :
			g_object_set(wizard,key,g_key_file_get_integer(mkt_atom_get_object_file(MKT_ATOM(wizard)),step,key,NULL),NULL);
			break;
		case G_TYPE_UINT :
			g_object_set(wizard,key,g_key_file_get_integer(mkt_atom_get_object_file(MKT_ATOM(wizard)),step,key,NULL),NULL);
			break;
		case G_TYPE_BOOLEAN :
			g_object_set(wizard,key,g_key_file_get_boolean(mkt_atom_get_object_file(MKT_ATOM(wizard)),step,key,NULL),NULL);
			break;
		case G_TYPE_STRING:
			t_setring = g_key_file_get_string(mkt_atom_get_object_file(MKT_ATOM(wizard)),step,key,NULL);
			if(t_setring)
			{
				g_object_set(wizard,key,t_setring,NULL);
				g_free(t_setring);
			}
			break;

		default:
			g_warning("Wizard property unknown type %s",g_type_name(pspec->value_type));
			break;
		}

	}
	else
	{
		//g_debug("Wizard key %s not available ", key);
	}


	return TRUE;
}

static gboolean
gl_wizard_set_step (GlWizard * wizard )
{
	g_return_val_if_fail(wizard!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),FALSE);
	g_return_val_if_fail(wizard->priv->step!=NULL,FALSE);
	//g_debug("Set step %s",wizard->priv->step);
	gsize len = 0;
	gchar **groups = g_key_file_get_groups(mkt_atom_get_object_file(MKT_ATOM(wizard)),&len);
	int i =0;
	wizard->priv->max_step = 0;
	wizard->priv->current_step=0;
	for(i=0;i<len&&groups!=NULL&&groups[i]!=NULL;i++)
	{
		if(g_str_has_prefix(groups[i],GL_WIZARD_STEP_PREFIX))
		{
			if(0!=g_strcmp0(groups[i],GL_WIZARD_STEP_PROP_STOP))
			{
				wizard->priv->max_step++;
				if(0==g_strcmp0(groups[i],wizard->priv->step))
				{
					wizard->priv->current_step=wizard->priv->max_step;
				}
			}
		}
	}
	if(groups)g_strfreev(groups);
	if(g_key_file_has_group(mkt_atom_get_object_file(MKT_ATOM(wizard)),wizard->priv->step))
	{
		GError *error = NULL;
		guint len = 0;
		gchar **keys = g_key_file_get_keys(mkt_atom_get_object_file(MKT_ATOM(wizard)),wizard->priv->step,&len,&error);
		if( error )
		{
			//g_warning("Wizard step brocked - %s",error->message);
			g_error_free(error);
		}
		else
		{
			gint i = 0;
			for(i=0;i<len&&keys!=NULL&&keys[i]!=NULL;i++)
			{
				//g_debug("set step key %d = %s",i,keys[i]);
				gl_wizard_set_property_step(wizard,wizard->priv->step,keys[i]);
			}
		}
		if(keys)g_strfreev(keys);

	}
	if(0==g_strcmp0(GL_WIZARD_STEP_PROP_STOP,wizard->priv->step))
	{
		GList *list = mkt_atom_get_children(MKT_ATOM(wizard));
		for(;list!=NULL;list=list->next)
		{
			if(list->data && GL_IS_PLUGIN(list->data))
			{
				gl_plugin_close(GL_PLUGIN(list->data));
			}
			else if(list->data && MKT_IS_WINDOW(list->data))
			{
				mkt_window_hide(MKT_WINDOW(list->data));
			}
		}
	}

	return TRUE;
}

static gboolean
gl_wizard_set_starter ( GlWizard * wizard )
{
	g_return_val_if_fail(wizard!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),FALSE);
	g_return_val_if_fail(wizard->priv->starter!=NULL,FALSE);
	gchar **starter_ar = g_strsplit_set(wizard->priv->starter,",",-1);
	int i = 0;
	for ( i= 0 ;starter_ar!= NULL && starter_ar[i]!= NULL ; i++)
	{
		MktAtom *atom = mkt_collector_get_atom_static(starter_ar[i]);
		//g_debug("Start window %s ",starter_ar[i]);
		if(atom && GL_IS_PLUGIN(atom))
		{
			gl_plugin_open(GL_PLUGIN(atom));
		}
		else if(atom && MKT_IS_WINDOW(atom))
		{
			mkt_window_show(MKT_WINDOW(atom));
		}
	}
	if(starter_ar) g_strfreev(starter_ar);
	return TRUE;
}

static gboolean
gl_wizard_stop_window(GlWizard * wizard)
{
	g_return_val_if_fail(wizard!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),FALSE);
	g_return_val_if_fail(wizard->priv->stop_window!=NULL,FALSE);
	gchar **starter_ar = g_strsplit_set(wizard->priv->stop_window,",",-1);
	int i = 0;
	for ( i= 0 ;starter_ar!= NULL && starter_ar[i]!= NULL ; i++)
	{
		MktAtom *atom = mkt_collector_get_atom_static(starter_ar[i]);
		//g_debug("Stop window %s ",starter_ar[i]);
		if(atom && GL_IS_PLUGIN(atom))
		{
			gl_plugin_close(GL_PLUGIN(atom));
		}
		else if(atom && MKT_IS_WINDOW(atom))
		{
			mkt_window_hide(MKT_WINDOW(atom));
		}
	}
	if(starter_ar) g_strfreev(starter_ar);
	return TRUE;
}

static gboolean
gl_wizard_hide_show_window(GlWizard * wizard)
{
	g_return_val_if_fail(wizard!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),FALSE);
	g_return_val_if_fail(wizard->priv->stop_window!=NULL,FALSE);
	gchar **starter_ar = g_strsplit_set(wizard->priv->hide_show_window,",",-1);
	int i = 0;
	for ( i= 0 ;starter_ar!= NULL && starter_ar[i]!= NULL ; i++)
	{
		MktAtom *atom = mkt_collector_get_atom_static(starter_ar[i]);
		//g_debug("Stop window %s ",starter_ar[i]);
		if(atom && MKT_IS_WINDOW(atom)&&!wizard->priv->is_hidden)
		{
			mkt_window_hide(MKT_WINDOW(atom));
			wizard->priv->is_hidden = TRUE;
		}
		else if(atom && MKT_IS_WINDOW(atom))
		{
			mkt_window_show(MKT_WINDOW(atom));
			wizard->priv->is_hidden = FALSE;
		}
	}
	if(starter_ar) g_strfreev(starter_ar);
	return TRUE;
}


gboolean
gl_wizard_close_button_clicked ( GtkWidget *widget , MktAtom *atom  )
{
	g_return_val_if_fail(atom!=NULL,FALSE);
	g_return_val_if_fail(MKT_IS_ATOM(atom),FALSE);
	GlWizard *wizard = GL_WIZARD(mkt_atom_get_parent(atom));
	g_return_val_if_fail(wizard!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),FALSE);
	mktAPSet(wizard,"wizard_step",GL_WIZARD_STEP_PROP_STOP);
	wizard->priv->is_hidden = FALSE;
	return TRUE;
}

gboolean
gl_wizard_next_step_cb( gpointer data )
{
	g_return_val_if_fail(data!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_WIZARD(data),FALSE);
	GlWizard *wizard = GL_WIZARD(data);
	gchar *next_step = NULL;
	next_step = gl_wizard_get_next_step(wizard);
	//	g_debug("Wizard %s next_step = %s",mkt_atom_get_id(MKT_ATOM(wizard)),next_step);
	if(next_step != NULL)
	{
		mktAPSet(wizard,"wizard_step",next_step);
		g_free(next_step);
	}
	else
	{
		mktAPSet(wizard,"wizard_step",GL_WIZARD_STEP_PROP_STOP);
	}
	wizard->priv->is_hidden = FALSE;
	g_signal_emit(wizard,gl_wizard_signals[GL_WIZARD_CHANGE_STEP],0);
	wizard->priv->next_idle = 0;
	return FALSE;
}

gboolean
gl_wizard_pre_step_cb( gpointer data )
{
	g_return_val_if_fail(data!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_WIZARD(data),FALSE);
	GlWizard *wizard = GL_WIZARD(data);
	gchar *pre_step = NULL;
	pre_step = gl_wizard_get_pre_step(wizard);
	//g_debug("Wizard %s next_step = %s",mkt_atom_get_id(atom),pre_step);
	if(pre_step != NULL)
	{
		mktAPSet(wizard,"wizard_step",pre_step);
		g_free(pre_step);
	}
	else
	{
		mktAPSet(wizard,"wizard_step",GL_WIZARD_STEP_PROP_STOP);
	}
	wizard->priv->is_hidden = FALSE;
	g_signal_emit(wizard,gl_wizard_signals[GL_WIZARD_CHANGE_STEP],0);
	wizard->priv->pre_idle = 0;
	return FALSE;
}

gboolean
gl_wizard_hide_show_cb( gpointer data )
{
	g_return_val_if_fail(data!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_WIZARD(data),FALSE);
	GlWizard *wizard = GL_WIZARD(data);
	gl_wizard_hide_show_window(GL_WIZARD(wizard));
	wizard->priv->hide_show_idle = 0;
	return FALSE;
}


gboolean
gl_wizard_next_button_clicked  ( GtkWidget *widget , MktAtom *atom )
{
	g_return_val_if_fail(atom!=NULL,FALSE);
	g_return_val_if_fail(MKT_IS_ATOM(atom),FALSE);
	GlWizard *wizard = GL_WIZARD(mkt_atom_get_parent(atom));
	g_return_val_if_fail(wizard!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),FALSE);
	if(wizard->priv->next_idle == 0)wizard->priv->next_idle =g_timeout_add (500, gl_wizard_next_step_cb, (gpointer)wizard);
	return TRUE;
}

gboolean
gl_wizard_pre_button_clicked  ( GtkWidget *widget , MktAtom *atom  )
{
	g_return_val_if_fail(atom!=NULL,FALSE);
	g_return_val_if_fail(MKT_IS_ATOM(atom),FALSE);
	GlWizard *wizard = GL_WIZARD(mkt_atom_get_parent(atom));
	g_return_val_if_fail(wizard!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),FALSE);
	if(wizard->priv->pre_idle == 0)wizard->priv->pre_idle =g_timeout_add (500, gl_wizard_pre_step_cb, (gpointer)wizard);
	return TRUE;
}

gboolean
gl_wizard_start                 ( GlWizard *wizard )
{
	g_return_val_if_fail(wizard!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),FALSE);
	if(!gl_level_manager_is_tru_user(wizard->priv->start_level)) return FALSE;
	gchar *next_step = NULL;
	//g_debug("WIZARD %s STEP starter:%s",mkt_atom_get_id(MKT_ATOM(wizard)),wizard->priv->step);
	g_object_set(wizard,"wizard_step",GL_WIZARD_STEP_PROP_STOP,NULL);
	if(0==g_strcmp0(wizard->priv->step,GL_WIZARD_STEP_PROP_STOP))
	{
		next_step =gl_wizard_get_next_step(wizard);
	}
	//g_debug("next_step=%s",next_step);
	if(next_step != NULL)
	{
		mktAPSet(wizard,"wizard_step",next_step);
		g_free(next_step);
	}
	else
	{
		mktAPSet(wizard,"wizard_step",GL_WIZARD_STEP_PROP_STOP);
	}
	wizard->priv->is_hidden = FALSE;
	g_signal_emit(wizard,gl_wizard_signals[GL_WIZARD_CHANGE_STEP],0);
	return TRUE;
}



gboolean
gl_wizard_hide_show_button_clicked ( GtkWidget *widget , MktAtom *atom  )
{
	g_return_val_if_fail(atom!=NULL,FALSE);
	g_return_val_if_fail(MKT_IS_ATOM(atom),FALSE);
	GlWizard *wizard = GL_WIZARD(mkt_atom_get_parent(atom));
	g_return_val_if_fail(wizard!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),FALSE);
	if(wizard->priv->hide_show_idle == 0)wizard->priv->hide_show_idle =g_timeout_add (500, gl_wizard_hide_show_cb, (gpointer)wizard);
	return TRUE;
}

gboolean
gl_wizard_starter_clicked ( GlActionWidget *widget, GlWizard *wizard )
{
	g_return_val_if_fail(wizard!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),FALSE);
	return gl_wizard_start(wizard);
}

/*
static void
gl_wizard_create_start_button ( GlWizard *wizard )
{
	wizard->priv->start_button   = gtk_button_new();
	GError *error = NULL;
	gchar *wisard_nick = g_key_file_get_string(wizard->priv->key_file,GL_WIZARD_MAIN_GROUP,GL_WIZARD_MAIN_NICK,&error);
	if(error)
	{
		g_warning ( "Wizard create starter error - %s",error->message);
		g_error_free(error);
	}
	error = NULL;
	gchar *wisard_icon = g_key_file_get_string(wizard->priv->key_file,GL_WIZARD_MAIN_GROUP,GL_WIZARD_MAIN_BUTTON_IMAGE,&error);
	if(error)
	{
		g_warning ( "Wizard create starter error - %s",error->message);
		g_error_free(error);
	}
	//g_debug("Wisards Icon %s",wisard_nick);
	//g_debug("Wisards Icon %s",wisard_icon);
	GtkWidget *box  = gtk_hbox_new(FALSE,1);
	GtkWidget *icon = gtk_image_new_from_file(wisard_icon);
	gtk_box_pack_start(GTK_BOX(box),icon,FALSE,FALSE,2);
	gtk_widget_set_size_request(icon,30,30);
	gchar *wizzard_name = g_strdup_printf("%s %s","Wizard",wisard_nick);
	GtkWidget *label = gtk_label_new( wizzard_name );
	gtk_misc_set_alignment(GTK_MISC(label),0.01,0.5);
	gtk_box_pack_start(GTK_BOX(box),label,TRUE,TRUE,2);
	gtk_widget_show(icon);
	gtk_widget_show(label);
	gtk_widget_show(box);

	gtk_container_add(GTK_CONTAINER(wizard->priv->start_button),box);
	if(wisard_nick)  g_free(wisard_nick);
	if(wisard_icon)  g_free(wisard_icon);
	if(wizzard_name) g_free(wizzard_name);

	g_signal_connect(wizard->priv->start_button,"clicked",G_CALLBACK(gl_wizard_starter_clicked),wizard);

}*/

void
gl_wizard_add_child ( MktAtom *parent ,MktAtom *child )
{
	g_return_if_fail (parent!= NULL);
	g_return_if_fail (GL_IS_WIZARD (parent));
	//g_debug("Add Wizard Child %s",mkt_atom_get_id(child));
	if(GL_IS_ACTION_WIDGET(child))
	{
		g_signal_connect(child,"start_action",G_CALLBACK(gl_wizard_starter_clicked),(gpointer)parent);
	}
}


static void
gl_wizard_init (GlWizard *wizard)
{
	GlWizardPrivate *priv = GL_WIZARD_PRIVATE(wizard);
	wizard->priv   = priv;
	wizard->priv->starter = g_strdup("no_module");
	wizard->priv->stop_window = g_strdup("no_module");
	wizard->priv->hide_show_window = g_strdup("no_module");
	wizard->priv->step = g_strdup(GL_WIZARD_STEP_PROP_STOP);
	wizard->priv->is_hidden = FALSE;
	wizard->priv->next_idle = 0;
	wizard->priv->pre_idle  = 0;
	wizard->priv->hide_show_idle = 0;
	wizard->priv->start_level = 0;

}

static void
gl_wizard_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	GlWizard *wizard = GL_WIZARD(object);
	if( wizard->priv->step )         g_free ( wizard->priv->step );
	if( wizard->priv->starter )      g_free ( wizard->priv->starter );
	if( wizard->priv->stop_window)   g_free ( wizard->priv->stop_window);
	G_OBJECT_CLASS (gl_wizard_parent_class)->finalize (object);
}

void
gl_wizard_set_property(  GObject        *object,
		guint           prop_id,
		const GValue   *value,
		GParamSpec     *pspec)
{
	//g_debug("SET WIZARD %s Property %s",mkt_atom_get_id(MKT_ATOM(object)),pspec->name);
	GlWizard *wizard = GL_WIZARD(object);
	switch(prop_id)
	{
	case GL_WIZZARD_STEP:
		if(wizard->priv->step)g_free(wizard->priv->step);
		wizard->priv->step = g_value_dup_string(value);
		gl_wizard_set_step ( wizard );
		break;
	case GL_WIZZARD_START_WINDOW:
		if(wizard->priv->starter)g_free(wizard->priv->starter);
		wizard->priv->starter = g_value_dup_string(value);
		gl_wizard_set_starter ( wizard );
		break;
	case GL_WIZZARD_STOP_WINDOW:
		if(wizard->priv->stop_window)g_free(wizard->priv->stop_window);
		wizard->priv->stop_window = g_value_dup_string(value);
		gl_wizard_stop_window ( wizard );
		break;
	case GL_WIZZARD_HIDE_STOP_WINDOW:
		if(wizard->priv->hide_show_window)g_free(wizard->priv->hide_show_window);
		wizard->priv->hide_show_window = g_value_dup_string(value);
		break;
	case GL_WIZZARD_START_LEVEL:
		wizard->priv->start_level = g_value_get_uint(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}

void
gl_wizard_get_property(  GObject        *object,
		guint           prop_id,
		GValue         *value,
		GParamSpec     *pspec)
{
	GlWizard *wizard = GL_WIZARD(object);
	switch(prop_id)
	{
	case GL_WIZZARD_STEP:
		g_value_set_string(value,wizard->priv->step);
		break;
	case GL_WIZZARD_START_LEVEL:
		g_value_set_uint(value,wizard->priv->start_level);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}

static void
gl_wizard_class_init (GlWizardClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	MktAtomClass* parent_class = MKT_ATOM_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GlWizardPrivate));

	object_class->finalize = gl_wizard_finalize;
	object_class->set_property = gl_wizard_set_property;
	object_class->get_property = gl_wizard_get_property;
	parent_class->add_child    = gl_wizard_add_child;

	GParamSpec *pspec;
	pspec = g_param_spec_string ("starter",
			"Wizard starter",
			"Set Wizard starter",
			GL_WIZARD_STEP_PROP_STOP,
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT );
	g_object_class_install_property (object_class,
			GL_WIZZARD_START_WINDOW,pspec);
	pspec = g_param_spec_string ("stop_window",
			"Wizard starter",
			"Set Wizard starter",
			GL_WIZARD_STEP_PROP_STOP,
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT );
	g_object_class_install_property (object_class,
			GL_WIZZARD_STOP_WINDOW,pspec);
	pspec = g_param_spec_string ("hide_show",
			"Wizard hide_show",
			"Set Wizard starter",
			GL_WIZARD_STEP_PROP_STOP,
			G_PARAM_READABLE | G_PARAM_WRITABLE  );
	g_object_class_install_property (object_class,
			GL_WIZZARD_HIDE_STOP_WINDOW,pspec);

	pspec = g_param_spec_string ("wizard_step",
			"Wizard step",
			"Set Wizard step",
			GL_WIZARD_STEP_PROP_STOP,
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT );
	g_object_class_install_property (object_class,
			GL_WIZZARD_STEP,pspec);


	pspec = g_param_spec_uint ("start_level",
				"Wizard start level",
				"Set Wizard minimum level",
				0,5,0,
				G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT );
	g_object_class_install_property (object_class,
	     		GL_WIZZARD_START_LEVEL,pspec);

	gl_wizard_signals[GL_WIZARD_CHANGE_STEP] =
					g_signal_new ("change-step",
							G_TYPE_FROM_CLASS (klass),
							G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
							G_STRUCT_OFFSET ( GlWizardClass, change_step),
							NULL, NULL,
							g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}



gint
gl_wizard_get_current_step      ( GlWizard *wizard )
{
	g_return_val_if_fail(wizard!=NULL,0);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),0);
	return wizard->priv->current_step;
}

gint
gl_wizard_get_nth_steps         ( GlWizard *wizard )
{
	g_return_val_if_fail(wizard!=NULL,0);
	g_return_val_if_fail(GL_IS_WIZARD(wizard),0);
	return wizard->priv->max_step;
}


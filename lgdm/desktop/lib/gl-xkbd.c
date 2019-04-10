/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * src
 * Copyright (C) sascha 2011 <sascha@sascha-desktop>
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

#include "gl-xkbd.h"
#include "gl-string.h"
#include "gl-translation.h"
#include "gl-widget-option.h"

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gdk/gdkx.h>
#include <signal.h>
#include <unistd.h>
#include <wait.h>

static GlXKbd * XVKBD_Keyboard = NULL;

struct _GlXKbdPrivate
{
	gchar            *name;
	gchar            *path;
	gchar           **argv_compact;
	gchar           **argv_keypad;
	gchar           **argv_load;
	pid_t             pid;
	gint              type;
	GtkWidget        *last_focus;
	unsigned int      winid;
	gint             ref_user;

};

#define GL_XKBD_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_XKBD, GlXKbdPrivate))

enum
{
	PROP_0,
	PROP_PATH,
	PROP_ARGV_COMPACT,
	PROP_ARGV_KEYPAD


};

enum
{
	GL_XKBD_START,
	GL_XKBD_STOP,
	LAST_SIGNAL
};


static guint gl_xkbd_signals[LAST_SIGNAL] = { 0 };



G_DEFINE_TYPE (GlXKbd, gl_xkbd, GTK_TYPE_BUTTON);

#define DEBUG_XVKBD 1

static void
gl_xkbd_stop_programm (GlXKbd *xkbd)
{
	//	g_debug("gl_xkbd_stop_programm");
	//mkt_trace("gl_xkbd_stop_programm\n");
	g_return_if_fail ( GL_IS_XKBD ( xkbd )) ;
	/*GlTranslation *translation = GL_TRANSLATION(gl_system_get_object("translation"));
	 *
	gchar *lcid = gl_translation_get_active_language_LCID_name ( translation );

	g_debug( "XKBD LCID %s",lcid);*/
	//gchar *lcid = market_translation_get_active_LCID_name (  );
	g_signal_emit(xkbd,gl_xkbd_signals[GL_XKBD_STOP],0);
#ifndef DEBUG_XVKBD
	xkbd->priv->pid = 0;
	mktKbdmgr (MKT_KBD_KILL, NULL, NULL, NULL);
#else
	if(xkbd->priv->pid <= 0) return;
	int status;
	g_debug("Close Pid %d", xkbd->priv->pid);
	if(!kill(	xkbd->priv->pid , SIGKILL))
	{
		waitpid(xkbd->priv->pid,&status,0);
		xkbd->priv->pid = 0;
	}
	else
	{
		xkbd->priv->pid = 0;
		g_warning("XKBD an error occurred in kill( process:%s pid=%d;signal:SIGTERM);",xkbd->priv->name,xkbd->priv->pid);
	}
#endif
}



static void
gl_xkbd_start_programm (GlXKbd *xkbd)
{
	//
	g_debug("gl_xkbd_start_programm");
	//	mkt_trace("gl_xkbd_start_programm\n");
	g_return_if_fail ( GL_IS_XKBD ( xkbd )) ;
	g_signal_emit(xkbd,gl_xkbd_signals[GL_XKBD_START],0);
#ifndef DEBUG_XVKBD
	const gchar *lcid = market_translation_get_active_LCID_name( );
	g_debug( "XKBD LCID %s",lcid);
	if (strcmp ("-compact", xkbd->priv->argv_load[1]) && g_strv_length(xkbd->priv->argv_load)>5)
		mktKbdmgr (MKT_KBD_KEYPAD,        xkbd->priv->argv_load[3], xkbd->priv->argv_load[5], lcid);
	else if(g_strv_length(xkbd->priv->argv_load)>5)
		mktKbdmgr (MKT_KBD_ALPHAKEYBOARD, xkbd->priv->argv_load[3], xkbd->priv->argv_load[5], lcid);
	xkbd->priv->pid  = 99;
#else

	if( xkbd->priv->pid > 0 )
	{
		//gl_xkbd_stop_programm(xkbd);
		return;
	}
	//g_debug("xkbd load start %s", gl_system_get_main_winid());
	int i;
	for(i =0;xkbd->priv->argv_load[i]!= NULL;i++) g_debug("%s ",xkbd->priv->argv_load[i]);

	const gchar *lcid = market_translation_get_active_LCID_name( );
	g_debug("lang = %s\n",lcid);
	xkbd->priv->pid = fork();
	if(xkbd->priv->pid != 0)
	{
		g_debug("XKDB start %s pid %d\n",xkbd->priv->name,xkbd->priv->pid);
	}
	else
	{
		gchar *to_file = g_strdup_printf("%s/XVkbd",  g_get_home_dir ());
		gchar *from_file;
		if(xkbd->priv->type == GL_XKBD_TYPE_KEYPAD)
		{
			//g_remove(to_file);
			from_file ="/lar/resource/extern/xvkbd/XVkbd-keypad.ad";
		}
		else
		{

			if(lcid == NULL  )
			{
				from_file ="/lar/resource/extern/xvkbd/XVkbd-uk.ad";
			}
			else
			{
				if(g_str_has_prefix(lcid,"da"))	from_file ="/lar/resource/extern/xvkbd/XVkbd-danish.ad";
				else if(g_str_has_prefix(lcid,"el"))from_file ="/lar/resource/extern/xvkbd/XVkbd-greek.ad";
				else if(g_str_has_prefix(lcid,"he"))from_file ="/lar/resource/extern/xvkbd/XVkbd-hebrew.ad";
				else if(g_str_has_prefix(lcid,"is"))from_file ="/lar/resource/extern/xvkbd/XVkbd-icelandic.ad";
				else if(g_str_has_prefix(lcid,"it"))from_file ="/lar/resource/extern/xvkbd/XVkbd-italian.ad";
				else if(g_str_has_prefix(lcid,"ja"))from_file ="/lar/resource/extern/xvkbd/XVkbd-jisx6002.ad";
				else if(g_str_has_prefix(lcid,"ko"))from_file ="/lar/resource/extern/xvkbd/XVkbd-korean.ad";
				else if(g_str_has_prefix(lcid,"no"))from_file ="/lar/resource/extern/xvkbd/XVkbd-norwegian.ad";
				else if(g_str_has_prefix(lcid,"pt"))from_file ="/lar/resource/extern/xvkbd/XVkbd-portuguese.ad";
				else if(g_str_has_prefix(lcid,"ru"))from_file ="/lar/resource/extern/xvkbd/XVkbd-russian.ad";
				else if(g_str_has_prefix(lcid,"sl"))from_file ="/lar/resource/extern/xvkbd/XVkbd-slovene.ad";
				else if(g_str_has_prefix(lcid,"es"))from_file ="/lar/resource/extern/xvkbd/XVkbd-spanish.ad";
				else if(g_str_has_prefix(lcid,"sv"))from_file ="/lar/resource/extern/xvkbd/XVkbd-swedish.ad";
				else if(g_str_has_prefix(lcid,"tr"))from_file ="/lar/resource/extern/xvkbd/XVkbd-turkish.ad";
				else if(g_str_has_prefix(lcid,"de"))
				{
					if(g_str_has_suffix(lcid,"CH"))from_file ="/lar/resource/extern/xvkbd/XVkbd-swissgerman.ad";
					else from_file ="/lar/resource/extern/xvkbd/XVkbd-german.ad";

				}
				else if(g_str_has_prefix(lcid,"en"))
				{
					if(g_str_has_suffix(lcid,"CH"))from_file ="/lar/resource/extern/xvkbd/XVkbd-uk.ad";
					else from_file ="/lar/resource/extern/xvkbd/XVkbd-uk.ad";
				}
				else if(g_str_has_prefix(lcid,"fr"))
				{
					if(g_str_has_suffix(lcid,"BE"))from_file ="/lar/resource/extern/xvkbd/XVkbd-belgian.ad";
					else from_file ="/lar/resource/extern/xvkbd/XVkbd-french.ad";
				}
				else
				{
					from_file ="/lar/resource/extern/xvkbd/XVkbd-uk.ad";
				}
			}

		}
		gsize len =0;
		gchar *content = NULL;
		if(g_file_get_contents(from_file,&content,&len,NULL))
		{
			g_file_set_contents(to_file,content,len,NULL);
			g_free(content);
		}
		if(to_file)g_free(to_file);
		execvp(xkbd->priv->path,xkbd->priv->argv_load);
		abort();
	}
#endif
}
/*
static void
gl_xkbd_start_clicked (GlXKbd *xkbd)
{
	g_return_if_fail ( GL_IS_XKBD ( xkbd )) ;

	if( xkbd->priv->pid > 0 )
	{
		gl_xkbd_stop_programm(xkbd);
		gtk_widget_hide (GTK_WIDGET(xkbd) );
	}
	else
	{

		GError *error = NULL;
		g_spawn_async("/usr/bin/",
				xkbd->priv->argv_load,
				NULL,
				0,
				NULL,
				NULL,
				&xkbd->priv->pid,
				&error);
		g_debug("Pid ID %d", xkbd->priv->pid);
		if(error )
		{
			g_critical ( "XKBD error : %s" ,error -> message );
		}
	}
	g_debug("Pid ID all %d", xkbd->priv->pid);

}*/

gboolean
gl_xkbd_run  ( gboolean stoppen)
{
	g_return_val_if_fail(XVKBD_Keyboard != NULL ,FALSE);
	g_return_val_if_fail(GL_IS_XKBD(XVKBD_Keyboard),FALSE);
	if( XVKBD_Keyboard->priv->pid >0 && stoppen )
	{
		gl_xkbd_stop_programm(XVKBD_Keyboard);
	}
	else if( XVKBD_Keyboard->priv->pid <= 0 )
	{
		gl_xkbd_start_programm ( XVKBD_Keyboard );
		//gtk_widget_show (GTK_WIDGET(XVKBD_Keyboard));
		gtk_widget_grab_focus(XVKBD_Keyboard->priv->last_focus);
	}
	return TRUE;
}

gboolean
gl_xkbd_is_run   ( )
{
	g_return_val_if_fail(XVKBD_Keyboard != NULL ,FALSE);
	g_return_val_if_fail(GL_IS_XKBD(XVKBD_Keyboard),FALSE);
	return XVKBD_Keyboard->priv->pid >0;
}


gboolean
gl_xkbd_stop  ( )
{
	g_debug("gl_xkbd_stop");
	g_return_val_if_fail(XVKBD_Keyboard != NULL ,FALSE);
	g_return_val_if_fail(GL_IS_XKBD(XVKBD_Keyboard),FALSE);

	//mkt_trace("xkbd Pid == %d\n",xkbd->priv->pid);
	if( XVKBD_Keyboard->priv->pid >0 )
	{
		gl_xkbd_stop_programm( XVKBD_Keyboard );
	}
	if(XVKBD_Keyboard->priv->last_focus)
	{
		//gchar *testwn = gl_widget_option_get_name(xkbd->priv->last_focus);
		//mkt_trace("focus==%s\n",testwn?testwn:"unknown");
		//gtk_widget_grab_default(XVKBD_Keyboard->priv->last_focus);
		XVKBD_Keyboard->priv->last_focus = NULL;
	}
	return TRUE;
}

gboolean
gl_xkbd_start  ( )
{
	//mkt_trace("gl_xkbd_start\n");
	g_return_val_if_fail(XVKBD_Keyboard != NULL ,FALSE);
	g_return_val_if_fail(GL_IS_XKBD(XVKBD_Keyboard),FALSE);
	//mkt_trace(" xkbd Pid == %d\n",xkbd->priv->pid);
	if( XVKBD_Keyboard->priv->pid > 0 )
	{
		gl_xkbd_stop_programm( XVKBD_Keyboard );
	}
	gl_xkbd_start_programm( XVKBD_Keyboard );
	gtk_widget_grab_focus(XVKBD_Keyboard->priv->last_focus);
	return TRUE;
}
gboolean
gl_xkbd_restart  ( )
{
	g_return_val_if_fail(XVKBD_Keyboard != NULL ,FALSE);
	g_return_val_if_fail(GL_IS_XKBD(XVKBD_Keyboard),FALSE);
	if( XVKBD_Keyboard->priv->pid >0 )
	{
		gl_xkbd_stop_programm(XVKBD_Keyboard);
	}
	gl_xkbd_start_programm ( XVKBD_Keyboard );
	//gtk_widget_show (GTK_WIDGET(XVKBD_Keyboard));
	return TRUE;
}


static gboolean
gl_xkbd_start_clicked ( GtkWidget *button , gpointer data)
{
	g_return_val_if_fail ( GL_IS_XKBD ( XVKBD_Keyboard ),FALSE) ;
	g_debug("gl_xkbd_start_clicked\n");

	XVKBD_Keyboard->priv->ref_user = 0;
	gl_xkbd_run ( TRUE);
	return TRUE;
}

static void
gl_xkbd_init (GlXKbd *object)
{
	object->priv =  G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_XKBD,GlXKbdPrivate);

	object->priv->path         = g_strdup("/usr/bin/xvkbd");
	object->priv->name         = g_strdup("xvkbd");

	gchar *argv_compact[] =
	{
			"/usr/bin/xvkbd",
			"-compact",
			"-geometry",
			"660x150+0-5",
			"-window",
			NULL
	};
	gchar *argv_keypad[] =
	{
			"/usr/bin/xvkbd",
			"-compact",
			"-geometry",
			"250x150+0-5",
			"-window",
			NULL
	};

	object->priv->argv_compact = g_strdupv(argv_compact);
	object->priv->argv_keypad  = g_strdupv(argv_keypad );
	object->priv->argv_load    = g_strdupv(argv_compact);
	object->priv->pid          = (-100);
	object->priv->type         = 0;
	object->priv->winid        = 0;
	object->priv->ref_user     = 0;
	GtkWidget* image = NULL;
	image = gtk_image_new_from_file("/lar/gui/preferences-desktop-keyboard.png");
	//g_printf("ICON_PATH:|%s|\n",object->priv->wimage);
	gtk_widget_show(image);
	gtk_container_add(GTK_CONTAINER(object),image);
	g_signal_connect ( ( gpointer ) object , "clicked" , G_CALLBACK (gl_xkbd_start_clicked) ,NULL );
	gchar *conf_content = NULL;
	gsize len =0;
	if(g_file_get_contents("/lar/resource/extern/xvkbd/xvkbd-conf",&conf_content,&len,NULL))
	{
		gchar *new_conf = g_strdup_printf("%s/.xvkbd",g_get_home_dir());
		g_file_set_contents(new_conf,conf_content,len,NULL);
		g_free(new_conf);
	}
	if(conf_content) g_free(conf_content);
}

static void
gl_xkbd_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	GlXKbd    *xkbd = GL_XKBD(object);
	g_free(xkbd->priv->path);
	g_free(xkbd->priv->name);
	g_strfreev(xkbd->priv->argv_compact);
	g_strfreev(xkbd->priv->argv_keypad);

	G_OBJECT_CLASS (gl_xkbd_parent_class)->finalize (object);
}

void
gl_xkbd_load ( )
{
	if(XVKBD_Keyboard == NULL)XVKBD_Keyboard = 	GL_XKBD ( g_object_new ( GL_TYPE_XKBD,NULL ));
}

GtkWidget*
gl_xkbd_get_widget ( )
{
	if(XVKBD_Keyboard == NULL)XVKBD_Keyboard = 	GL_XKBD ( g_object_new ( GL_TYPE_XKBD,NULL ));
	return GTK_WIDGET(XVKBD_Keyboard);
}

GlXKbd*
gl_xkbd_get_keyboard ( )
{
	if(XVKBD_Keyboard == NULL)XVKBD_Keyboard = 	GL_XKBD ( g_object_new ( GL_TYPE_XKBD,NULL ));
	return XVKBD_Keyboard;
}


void
gl_xkbd_need_restart()
{


}

static void
gl_xkbd_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_XKBD (object));
	GlXKbd *xkbd = GL_XKBD(object);
	switch (prop_id)
	{
	case PROP_PATH:
		g_free (xkbd->priv->path);
		xkbd->priv->path = g_value_dup_string(value);
		/* TODO: Add setter for "path" property here */
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_xkbd_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_XKBD (object));
	GlXKbd *xkbd = GL_XKBD(object);
	switch (prop_id)
	{
	case PROP_PATH:
		g_value_set_string(value,(const gchar *) xkbd->priv->path);
		/* TODO: Add getter for "path" property here */
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_xkbd_class_init (GlXKbdClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	//GObjectClass* parent_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GlXKbdPrivate));

	object_class->finalize = gl_xkbd_finalize;
	object_class->set_property = gl_xkbd_set_property;
	object_class->get_property = gl_xkbd_get_property;

	GParamSpec *pspec;
		pspec = g_param_spec_string ("path",
				"xvkbd path",
				"Set|Get xvkbd path",
				"/usr/bin/xvkbd",
				G_PARAM_READABLE | G_PARAM_WRITABLE);
		g_object_class_install_property (object_class,
				PROP_PATH,pspec);




		gl_xkbd_signals[GL_XKBD_START] =
				g_signal_new ("xkbd-start",
						G_TYPE_FROM_CLASS (klass),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET ( GlXKbdClass, start),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

		gl_xkbd_signals[GL_XKBD_STOP] =
				g_signal_new ("xkbd-stop",
						G_TYPE_FROM_CLASS (klass),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET ( GlXKbdClass, stop),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}



void
gl_xkbd_set_compact_params ( gchar* strarr,...)
{
	g_return_if_fail( XVKBD_Keyboard != NULL ) ;
	g_return_if_fail( GL_IS_XKBD(XVKBD_Keyboard) );
	char **argv = &strarr;
	g_strfreev(XVKBD_Keyboard->priv->argv_compact);
	XVKBD_Keyboard->priv->argv_compact = g_strdupv(argv);
	gl_xkbd_need_restart();
}

void
gl_xkbd_set_keypad_params ( gchar* strarr,... )
{
	g_return_if_fail( XVKBD_Keyboard != NULL ) ;
	g_return_if_fail( GL_IS_XKBD(XVKBD_Keyboard) );
	char **argv = &strarr;
	g_strfreev(XVKBD_Keyboard->priv->argv_keypad);
	XVKBD_Keyboard->priv->argv_keypad = g_strdupv(argv);
	gl_xkbd_need_restart();
}

void
gl_xkbd_set_set_type    ( gint type )
{
	g_return_if_fail( XVKBD_Keyboard != NULL ) ;
	g_return_if_fail( GL_IS_XKBD(XVKBD_Keyboard) );
	if (XVKBD_Keyboard->priv -> type != type )
	{
		if( XVKBD_Keyboard->priv->pid > 0 )
		{
			gl_xkbd_stop_programm   ( XVKBD_Keyboard );
			//gl_xkbd_start_programm  ( xkbd );
			//if(xkbd->priv->last_focus)gtk_widget_grab_focus(xkbd->priv->last_focus);
			//gtk_widget_show (GTK_WIDGET(xkbd) );
		}
	}
	XVKBD_Keyboard->priv->type = type;
}

gint
gl_xkbd_get_key_type( )
{
	g_return_val_if_fail(XVKBD_Keyboard != NULL ,0);
	g_return_val_if_fail(GL_IS_XKBD(XVKBD_Keyboard),0);
	return XVKBD_Keyboard->priv->type;
}

void
gl_xkbd_ref_user    ( GlXKbd *xkbd  )
{
	g_return_if_fail( XVKBD_Keyboard != NULL ) ;
	g_return_if_fail( GL_IS_XKBD(XVKBD_Keyboard) );
	XVKBD_Keyboard->priv->ref_user++;
}

void
gl_xkbd_unref_user   (   )
{
	g_return_if_fail( XVKBD_Keyboard != NULL ) ;
	g_return_if_fail( GL_IS_XKBD(XVKBD_Keyboard) );
	XVKBD_Keyboard->priv->ref_user--;
	if(XVKBD_Keyboard->priv->ref_user < 0 )XVKBD_Keyboard->priv->ref_user = 0;
}

void
gl_xkbd_set_winid        ( const gchar* winId,GtkWidget *child )
{
	g_return_if_fail( XVKBD_Keyboard != NULL ) ;

	gchar *wID = NULL;
	if( child != NULL )
	{

		GtkWidget *parent = gl_widget_option_get_root_widow(child);
		XVKBD_Keyboard->priv->winid = GDK_WINDOW_XID(parent->window);
		wID = g_strdup_printf("0x%X",XVKBD_Keyboard->priv->winid);
		gtk_window_set_focus_on_map(GTK_WINDOW(parent),TRUE);
		g_debug("Test Win ID=%s",wID);
	}
	else
	{
		wID = g_strdup(winId);
	}

	if(XVKBD_Keyboard->priv->argv_load) g_strfreev(XVKBD_Keyboard->priv->argv_load);
	switch (XVKBD_Keyboard->priv -> type )
	{
	case GL_XKBD_TYPE_COMPACT :XVKBD_Keyboard->priv->argv_load = gl_strvadd_part((const gchar**)XVKBD_Keyboard->priv->argv_compact,(const gchar*)wID); break;
	case GL_XKBD_TYPE_KEYPAD  :XVKBD_Keyboard->priv->argv_load = gl_strvadd_part((const gchar**)XVKBD_Keyboard->priv->argv_keypad,(const gchar*)wID);  break;
	default : XVKBD_Keyboard->priv->argv_load = gl_strvadd_part((const gchar**)XVKBD_Keyboard->priv->argv_compact,(const gchar*) wID); break;
	}
	if( wID!=NULL)g_free(wID);
}


void
gl_xkbd_set_winid_for_main       (  )
{
	g_return_if_fail( XVKBD_Keyboard != NULL ) ;
	char newId[20] = "0x11";
	memset(newId,0,20);
	//FIXME: soll main windows
	GtkWidget *wid = gl_widget_option_get_root_widow(XVKBD_Keyboard->priv->last_focus);
	XVKBD_Keyboard->priv->winid = GDK_WINDOW_XID(wid->window);
	sprintf(newId,"0x%X",XVKBD_Keyboard->priv->winid);
    gtk_window_set_focus_on_map(GTK_WINDOW(wid), TRUE);
	g_strfreev(XVKBD_Keyboard->priv->argv_load);
	switch (XVKBD_Keyboard->priv -> type )
	{
	case GL_XKBD_TYPE_COMPACT :XVKBD_Keyboard->priv->argv_load = gl_strvadd_part((const gchar**)XVKBD_Keyboard->priv->argv_compact,(const gchar*)newId); break;
	case GL_XKBD_TYPE_KEYPAD  :XVKBD_Keyboard->priv->argv_load = gl_strvadd_part((const gchar**)XVKBD_Keyboard->priv->argv_keypad ,(const gchar*)newId);  break;
	default : XVKBD_Keyboard->priv->argv_load = gl_strvadd_part((const gchar**)XVKBD_Keyboard->priv->argv_compact,(const gchar*) newId); break;
	}
}

void
gl_xkbd_need_keyboard     ( GtkWidget *widget ,GtkWidget *focus, gint type )
{
	g_return_if_fail( XVKBD_Keyboard != NULL ) ;
	g_return_if_fail( GL_IS_XKBD(XVKBD_Keyboard) );
	g_return_if_fail( widget != NULL ) ;
	g_return_if_fail( GTK_IS_WIDGET(widget) );
	if( (XVKBD_Keyboard->priv->pid >0) && (focus == XVKBD_Keyboard->priv->last_focus ) ) return;
	if(gl_xkbd_is_run()) gl_xkbd_stop();
	//if(XVKBD_Keyboard->priv->last_focus && XVKBD_Keyboard->priv->last_focus !=widget)gtk_widget_grab_default(XVKBD_Keyboard->priv->last_focus);
	g_debug ( " need keyboard ");
	char winId[] = "0x11";
	/*memset(winId,0,20);
	GdkWindow *wid = gtk_widget_get_root_window(widget);
	xkbd->priv->winid = GDK_WINDOW_XID(wid);
	sprintf(winId,"0x%X",xkbd->priv->winid);*/
	gtk_widget_grab_focus(focus);
	XVKBD_Keyboard->priv->last_focus = focus;
	gl_xkbd_set_set_type ( type );
	gl_xkbd_set_winid(NULL,widget);
	gl_xkbd_start  ( );
	//gtk_widget_show ( GTK_WIDGET(XVKBD_Keyboard) );
}


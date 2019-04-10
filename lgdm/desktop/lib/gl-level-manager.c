/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-level-manager.c
 * Copyright (C) Sascha 2011 <sascha@sascha-desktop>
 * 
gl-level-manager.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-level-manager.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl-level-manager.h"
#include "gl-translation.h"
#include "gl-action-widget.h"
#include "gl-xkbd.h"
#include "gl-extern-process.h"
#include "gl-widget-option.h"

#include <mkt-error-message.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <wait.h>

struct _GlLevelManagerPrivate
{
	guint         level;
	gchar        *level_name;
	gboolean      key_open;
	GtkWidget    *window;
	GtkWidget    *load_box;
	GtkWidget    *progress;
	GtkWidget    *module_load;
	GtkWidget    *small_progress;
	GtkWidget    *button;
	GtkWidget    *msg;
	GtkWidget    *password;

	GtkWidget    *pass_label;
	GtkWidget    *msg_label;

	gboolean      wait_status;
	gint          market_watch;
	gint          signal_watch;
	gdouble       fraction;
	gdouble       fraction_small;
};


static GlLevelManager *security_level_manager = NULL;

#define GL_LEVEL_MANAGER_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_LEVEL_MANAGER, GlLevelManagerPrivate))

enum
{
	PROP_0,
	PROP_LEVEL,
	PROP_LEVEL_NAME
};



typedef enum
{
	GUI_USER_DEVICE     = 1 << 1,        // "Device"
	GUI_USER_OPERATOR   = 1 << 2,        // "Operator"
	GUI_USER_SUPER_OPE  = 1 << 3,        // "Super operator"
	GUI_USER_SERVICE    = 1 << 4,        // "Service"
	GUI_USER_ROOT       = 1 << 5,        // "Root"
	GUI_USER_LAST

}GuiUserType;


typedef enum
{
	GUI_USER_AUTORIZATION_DEVICE     =  GUI_USER_DEVICE ,
	GUI_USER_AUTORIZATION_OPERATOR   =  GUI_USER_DEVICE | GUI_USER_OPERATOR ,
	GUI_USER_AUTORIZATION_SUPER_OPE  =  GUI_USER_DEVICE | GUI_USER_OPERATOR | GUI_USER_SUPER_OPE ,
	GUI_USER_AUTORIZATION_SERVICE    =  GUI_USER_DEVICE | GUI_USER_OPERATOR | GUI_USER_SUPER_OPE | GUI_USER_SERVICE ,
	GUI_USER_AUTORIZATION_ROOT       =  GUI_USER_DEVICE | GUI_USER_OPERATOR | GUI_USER_SUPER_OPE | GUI_USER_SERVICE | GUI_USER_ROOT,
}GuiUserAutorization;


G_DEFINE_TYPE (GlLevelManager, gl_level_manager, MKT_TYPE_ATOM);

static gboolean gl_level_manager_show_welcome_window ( GlLevelManager *lm );
static gboolean gl_level_manager_hide_welcome_window ( GlLevelManager *lm );

static gboolean gui_system_signal_handler_idle       ( gpointer data );

guint  gl_level_manager_get_level(guint level)
{
	switch(level)
	{
	case GUI_USER_DEVICE_TYPE            : return GUI_USER_DEVICE;
	case GUI_USER_OPERATOR_TYPE          : return GUI_USER_OPERATOR;
	case GUI_USER_SUPER_OPERATOR_TYPE    : return GUI_USER_SUPER_OPE;
	case GUI_USER_SERVICE_TYPE		     : return GUI_USER_SERVICE;
	case GUI_USER_ROOT_TYPE    		     : return GUI_USER_ROOT;
	default                    		     : return GUI_USER_DEVICE;

	}
}

guint
gl_level_manager_get_level_autorization(guint level)
{
	switch(level)
	{
	case GUI_USER_DEVICE_TYPE            : return GUI_USER_AUTORIZATION_DEVICE;
	case GUI_USER_OPERATOR_TYPE          : return GUI_USER_AUTORIZATION_OPERATOR;
	case GUI_USER_SUPER_OPERATOR_TYPE    : return GUI_USER_AUTORIZATION_SUPER_OPE;
	case GUI_USER_SERVICE_TYPE		     : return GUI_USER_AUTORIZATION_SERVICE;
	case GUI_USER_ROOT_TYPE    		     : return GUI_USER_AUTORIZATION_ROOT;
	default                    		     : return GUI_USER_AUTORIZATION_DEVICE;

	}
}

gchar*
gl_level_manager_get_level_name_from_id(guint level)
{
	static gchar user_name[128];
	memset(user_name,0,128);
	switch(level)
	{
	case GUI_USER_DEVICE_TYPE         : g_stpcpy(user_name,"device")  ;break;
	case GUI_USER_OPERATOR_TYPE       : g_stpcpy(user_name,"operator");break;
	case GUI_USER_SUPER_OPERATOR_TYPE : g_stpcpy(user_name,"super_operator");break;
	case GUI_USER_SERVICE_TYPE        : g_stpcpy(user_name,"service") ;break;
	case GUI_USER_ROOT_TYPE           : g_stpcpy(user_name,"root")    ;break;
	default                           : g_stpcpy(user_name,"unknown") ;break;

	}
	return user_name;
}

gchar*
gl_level_manager_get_name_for_noob_from_id(guint level)
{
	static gchar user_name[128];
	memset(user_name,0,128);
	switch(level)
	{
	case GUI_USER_DEVICE_TYPE   : g_stpcpy(user_name,"Level I")  ;break;
	case GUI_USER_OPERATOR_TYPE : g_stpcpy(user_name,"Level II") ;break;
	case GUI_USER_SUPER_OPERATOR_TYPE: g_stpcpy(user_name,"Level III");break;
	case GUI_USER_SERVICE_TYPE  : g_stpcpy(user_name,"Level IV") ;break;
	case GUI_USER_ROOT_TYPE     : g_stpcpy(user_name,"Level V")  ;break;
	default                     : g_stpcpy(user_name,"unknown")  ;break;

	}
	return user_name;
}

gchar*
gl_level_manager_get_level_name()
{
	if(security_level_manager == NULL ) return "unknown";
	static gchar user_name[128];
	memset(user_name,0,128);
	switch(security_level_manager->priv->level)
	{
	case GUI_USER_AUTORIZATION_DEVICE   : g_stpcpy(user_name,"device")  ;break;
	case GUI_USER_AUTORIZATION_OPERATOR : g_stpcpy(user_name,"operator");break;
	case GUI_USER_AUTORIZATION_SUPER_OPE: g_stpcpy(user_name,"super_operator");break;
	case GUI_USER_AUTORIZATION_SERVICE  : g_stpcpy(user_name,"service") ;break;
	case GUI_USER_AUTORIZATION_ROOT     : g_stpcpy(user_name,"root")    ;break;
	default                             : g_stpcpy(user_name,"unknown") ;break;

	}
	return user_name;
}

gchar*
gl_level_manager_get_level_name_for_noob()
{
	static gchar user_name[128];
	memset(user_name,0,128);
	switch(security_level_manager->priv->level)
	{
	case GUI_USER_AUTORIZATION_DEVICE   : g_stpcpy(user_name,"Level I")  ;break;
	case GUI_USER_AUTORIZATION_OPERATOR : g_stpcpy(user_name,"Level II") ;break;
	case GUI_USER_AUTORIZATION_SUPER_OPE: g_stpcpy(user_name,"Level III");break;
	case GUI_USER_AUTORIZATION_SERVICE  : g_stpcpy(user_name,"Level IV") ;break;
	case GUI_USER_AUTORIZATION_ROOT     : g_stpcpy(user_name,"Level V")  ;break;
	default                             : g_stpcpy(user_name,"unknown")  ;break;

	}
	return user_name;
}

guint
gl_level_manager_get_level_type_from_name( gchar *level_name)
{
	if(security_level_manager == NULL )   return  GUI_USER_LAST_TYPE;
	if(level_name == NULL ) return  GUI_USER_LAST_TYPE;

	if(0==g_strcmp0(level_name,"device"))	           return GUI_USER_DEVICE_TYPE;
	else if(0==g_strcmp0(level_name,"operator"))       return GUI_USER_OPERATOR_TYPE;
	else if(0==g_strcmp0(level_name,"super_operator")) return GUI_USER_SUPER_OPERATOR_TYPE;
	else if(0==g_strcmp0(level_name,"service"))        return GUI_USER_SERVICE_TYPE;
	else if(0==g_strcmp0(level_name,"root"))           return GUI_USER_ROOT_TYPE;
	else return GUI_USER_LAST_TYPE;
}


enum
{
	GL_LEVEL_MANAGER_CHANGE_LEVEL,
	GL_LEVEL_MANAGER_MOUNT_USB,
	GL_LEVEL_MANAGER_UMOUNT_USB,
	GL_LEVEL_MANAGER_KEY_OPEN,
	GL_LEVEL_MANAGER_KEY_CLOSE,
	LAST_SIGNAL
};

static guint gl_level_manager_signals[LAST_SIGNAL] = { 0 };

#define GL_LEVEL_MANAGER_RECHTELEVEL "/lar/usbstick/.rechtslevel"

static void
gl_level_manager_real_key_open(GlLevelManager *lm)
{
	//TEST:g_print("gl_level_manager_real_key_open\n");
	lm->priv->key_open = TRUE;
	g_signal_emit(lm,gl_level_manager_signals[GL_LEVEL_MANAGER_KEY_OPEN],0);

}

static void
gl_level_manager_real_key_close(GlLevelManager *lm)
{
	//TEST:g_print("gl_level_manager_real_key_close\n");
	lm->priv->key_open = FALSE;
	g_signal_emit(lm,gl_level_manager_signals[GL_LEVEL_MANAGER_KEY_CLOSE],0);
}

static void
gl_level_manager_change_level(GlLevelManager *lmanager,guint level)
{
	g_return_if_fail(GL_IS_LEVEL_MANAGER(lmanager));
	g_free(lmanager->priv->level_name);
	if(lmanager->priv->level   != gl_level_manager_get_level_autorization(level))
	lmanager->priv->level       = gl_level_manager_get_level_autorization(level);
	lmanager->priv->level_name  = g_strdup(gl_level_manager_get_level_name(lmanager));
	g_signal_emit(lmanager,gl_level_manager_signals[GL_LEVEL_MANAGER_CHANGE_LEVEL],0);
	if ( lmanager->priv->level <= GUI_USER_AUTORIZATION_DEVICE ) gl_level_manager_real_key_close(lmanager);
	else gl_level_manager_real_key_open(lmanager);
}

gboolean
gl_level_manager_close( )
{
	g_return_val_if_fail(security_level_manager != NULL,FALSE);
	g_return_val_if_fail(GL_IS_LEVEL_MANAGER(security_level_manager),FALSE);
	gl_level_manager_hide_welcome_window(security_level_manager);
	g_object_set(G_OBJECT(security_level_manager),"level",GUI_USER_DEVICE_TYPE,NULL);

	return TRUE;
}


gboolean
__gui_rechtslevel_test(char *level,char *device)
{
	gui_extern_process_init("bash","/lar/bin/createinfo");
	gui_extern_process_stop("bash");
	pid_t pid = gui_extern_process_start("bash","/bin/bash","bash","/lar/bin/createinfo",level,device,NULL);
	int status=0;
	waitpid(pid,&status,0);
	FILE *f    = fopen(GL_LEVEL_MANAGER_RECHTELEVEL,"r");
	FILE *info = fopen("/lar/ramdisk/.device.info","r");
	gboolean ret = FALSE;
	if( (f!= NULL)&&(info!=NULL) )
	{
		unsigned char hash[4096];
		memset(hash,0,sizeof(hash));
		unsigned char rstr[4096];
		memset(rstr,0,sizeof(rstr));
		char *p = fgets((char*)rstr,sizeof(rstr),f);
		//TEST:printf("rechtslevel |%s|\n",rstr);
		unsigned char inforstr[4096];
		memset(inforstr,0,sizeof(inforstr));
		p = fgets((char*)inforstr,sizeof(inforstr),info);
		//TEST:printf("info |%s|\n",inforstr);
		if(0==strcmp((const char*)inforstr,(const char*)rstr)) ret = TRUE;
	//	SHA512((unsigned char *) inforstr, strlen(inforstr), hash);
	//	printf("HASH:|%s|\n",hash);
	}
	if(f    != NULL)fclose(f);
	if(info != NULL)fclose(info);
	return ret;
}

char*
__giu_rechtslevel_get_usb_device()
{
	static char ret[15] = "";
	memset(ret,0,sizeof(ret));
	int c = 0;
	char ts1[12];char ts2[12];
	for(c = 97; c <= 122;c++)
	{
		memset(ts1,0,sizeof(ts1));memset(ts2,0,sizeof(ts2));
		snprintf(ts1,sizeof(ts1),"/dev/sd%c1",(unsigned char) c);
		snprintf(ts2,sizeof(ts2),"/dev/sd%c2",(unsigned char) c);
		if((0==access(ts1,0))&&(access(ts2,0)))
		{
			memset(ret,0,sizeof(ret));
			snprintf(ret,sizeof(ret),"/dev/sd%c",(unsigned char) c);
			return ret;
		}
	}
	strcpy(ret,"/dev/hdc");
	return ret;
}

#define GUI_DEBUG
static gboolean
gl_level_manager_set_level_from_file(GlLevelManager *lm)
{
	g_return_val_if_fail(lm != NULL,FALSE);
	g_return_val_if_fail(GL_IS_LEVEL_MANAGER(lm),FALSE);

	char *device = __giu_rechtslevel_get_usb_device();
//	device = "/dev/sdb";
//printf("DEVICE Find:%s\n",device);

#ifdef GUI_DEBUG
	gl_level_manager_change_level(lm,GUI_USER_ROOT_TYPE);
	return TRUE;
#endif

	if(__gui_rechtslevel_test("operator",device))
	{
		gl_level_manager_change_level(lm,GUI_USER_OPERATOR_TYPE);
		return TRUE;
	}
	else if(__gui_rechtslevel_test("super_operator",device))
	{
		gl_level_manager_change_level(lm,GUI_USER_SUPER_OPERATOR_TYPE);
		return TRUE;
	}
	else if(__gui_rechtslevel_test("service",device))
	{
		gl_level_manager_change_level(lm,GUI_USER_SERVICE_TYPE);
		return TRUE;
	}
	else if(__gui_rechtslevel_test("root",device))
	{
		gl_level_manager_change_level(lm,GUI_USER_ROOT_TYPE);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static gboolean
gl_level_manager_is_operator_password(GlLevelManager *lm)
{

	//check user object ...
	/*if(mkIget(parameter___operator_password_activ))
	{
		const char *password = gtk_entry_get_text(GTK_ENTRY(lm->priv->password));
		if(password != NULL && 0==strcmp(password,mkIget(parameter___operator_password)))
			return TRUE;
	}*/
	return FALSE;
}

gboolean
gl_level_manager_go_signal_clicked(GtkWidget *button,GlLevelManager *lm)
{
	g_return_val_if_fail(GL_IS_LEVEL_MANAGER(lm),FALSE);
	gl_level_manager_hide_welcome_window(lm);
	//const gchar *password = gtk_entry_get_text(GTK_ENTRY(lm->priv->password));
	//const gchar *msg      = gtk_entry_get_text(GTK_ENTRY(lm->priv->msg));
	if(gl_level_manager_set_level_from_file(lm))
	{
		mkt_log_system_message("Open level %s",gl_level_manager_get_level_name_for_noob());
	}
	else if(gl_level_manager_is_operator_password(lm))
	{
		gl_level_manager_change_level(lm,GUI_USER_OPERATOR_TYPE);
	}
	else
	{
		gl_level_manager_change_level(lm,GUI_USER_DEVICE_TYPE);
	}
	gl_xkbd_stop();
	return TRUE;
}


gboolean
gl_level_manager_focus_in_signal (GtkWidget *widget,GdkEvent *event, gpointer data)
{
	//g_debug("gl_level_manager_focus_in_signal");
	//mkt_trace("gl_level_manager_focus_in_signal\n");

	//mkt_trace("xkbd find\n");
	gint type = 0;
	type = GL_XKBD_TYPE_COMPACT;
	gl_xkbd_need_keyboard (widget,widget, type);
	//gl_xkbd_set_winid (xkbd,NULL);
	//gl_xkbd_start ();

	return FALSE;
}

gboolean
gl_level_manager_password_change_signal(GtkWidget *widget,gpointer main)
{
	GlLevelManager *manager = GL_LEVEL_MANAGER(main);
	gchar *pass = (gchar *) gtk_entry_get_text(GTK_ENTRY(widget));
	if(0==strcmp(pass,"root"))
	{
		gtk_widget_set_sensitive(GTK_WIDGET(manager->priv->button),TRUE);
	}
	return TRUE;
}

gboolean
gl_level_manager_market_watch_cb(gpointer data)
{
	g_return_val_if_fail(data != NULL,FALSE);
	g_return_val_if_fail(GL_IS_LEVEL_MANAGER(data),FALSE);
	GlLevelManager *manager = GL_LEVEL_MANAGER(data);
	//gdouble fraction  = ((gdouble) mkIget(control_subscription__internalStatus)) /90.+0.1;
	//g_debug("LEVEL_MANAGER WATCH FRACTION %f status = %d",fraction, mkIget(control_subscription__internalStatus));
	if(manager->priv->fraction < 0.0 || manager->priv->fraction >= 1.0)
	{
		gtk_widget_hide(manager->priv->progress);
		gtk_widget_hide(manager->priv->small_progress);
		gtk_widget_set_sensitive(manager->priv->button,TRUE);
		manager->priv->wait_status = FALSE;
	}
	else
	{
		//gl_level_manager_show_welcome_window(manager);
		manager->priv->wait_status = TRUE;
		gtk_widget_show(manager->priv->load_box);
		gtk_widget_show(manager->priv->progress);
		gtk_widget_show(manager->priv->small_progress);
		if(GTK_IS_PROGRESS_BAR(manager->priv->progress))
		{
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(manager->priv->progress),manager->priv->fraction);
			//gtk_progress_bar_set_text(GTK_PROGRESS_BAR(manager->priv->progress),mkIget(control_subscription__internalControlState));
		}
		if(GTK_IS_PROGRESS_BAR(manager->priv->small_progress))
		{
			gtk_progress_bar_pulse(GTK_PROGRESS_BAR(manager->priv->small_progress));
		}
	}
	return TRUE;
}


void
gl_level_manager_translate_signal  ( GlTranslation *tr  )
{
	g_return_if_fail (security_level_manager != NULL);
	g_return_if_fail (GL_IS_LEVEL_MANAGER (security_level_manager));
	g_return_if_fail (GL_IS_TRANSLATION (tr));

// TODO:TRANSLATE...
	//gtk_label_set_text(GTK_LABEL(lmanager->priv->pass_label),GL_TRANSLATE(tr,NULL,"TRANSLATE_label_LevelManager_password","Password:"));
	//gtk_label_set_text(GTK_LABEL(lmanager->priv->msg_label),GL_TRANSLATE(tr,NULL,"TRANSLATE_label_LevelManager_operatormsg","Operator log:"));

}


static gboolean
gl_level_manager_create_welcome_window ( GlLevelManager *lmanager )
{
	g_return_val_if_fail(lmanager != NULL,FALSE);
	g_return_val_if_fail(GL_IS_LEVEL_MANAGER(lmanager),FALSE);
	//TEST:g_debug("Test ... create welcome window ... ");
	if( lmanager->priv->window != NULL ) return TRUE;
	GtkWidget *label;
	GtkWidget *separator;
	lmanager->priv->window    = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_decorated ( GTK_WINDOW (lmanager->priv->window) , FALSE );
	gtk_window_set_modal(GTK_WINDOW(lmanager->priv->window),TRUE);
	//
	gl_widget_option_set_name(G_OBJECT(lmanager->priv->window),mkt_atom_get_id(MKT_ATOM(lmanager)));

	gtk_window_set_default_size(GTK_WINDOW(lmanager->priv->window),370,350);
	//gtk_window_set_
	gtk_widget_set_sensitive(lmanager->priv->window ,FALSE);
	GtkWidget *viewport = gtk_viewport_new (NULL,NULL);
	//gtk_widget_set_size_request(GTK_WIDGET(lmanager->priv->progress),370,300);
	GtkWidget *vbox = gtk_vbox_new(FALSE,3);
	GtkWidget *hbox = gtk_hbox_new(FALSE,3);
	gtk_container_set_border_width(GTK_CONTAINER (viewport),3);
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport),GTK_SHADOW_ETCHED_OUT);
	gtk_container_add (GTK_CONTAINER (lmanager->priv->window), viewport);
	gtk_container_add (GTK_CONTAINER (viewport), vbox);
	//gtk_container_set_border_width(GTK_CONTAINER(vbox),4);
	GtkWidget *welcomImage = gtk_image_new_from_file("/lar/gui/welcomLar.png");
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,4);
	gtk_box_pack_start(GTK_BOX(hbox),welcomImage,FALSE,FALSE,4);
	//GtkWidget *label = gtk_label_new(" load forgang");
	//gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,4);
	separator = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox),separator,FALSE,FALSE,4);
	gtk_widget_show(separator);
	gtk_widget_show(welcomImage);
	gtk_widget_show(hbox);

	lmanager->priv->load_box = gtk_hbox_new(FALSE,3);
	gtk_box_pack_start(GTK_BOX(vbox),lmanager->priv->load_box,FALSE,FALSE,4);

	lmanager->priv->progress       = gtk_progress_bar_new();
	lmanager->priv->small_progress = gtk_progress_bar_new();
	lmanager->priv->module_load    = gtk_progress_bar_new();
	gtk_widget_set_size_request(GTK_WIDGET(lmanager->priv->progress),300,15);
	gtk_widget_set_size_request(GTK_WIDGET(lmanager->priv->small_progress),300,6);
	gtk_widget_set_size_request(GTK_WIDGET(lmanager->priv->module_load),300,10);
	gtk_widget_set_sensitive(lmanager->priv->module_load,FALSE);
	GtkWidget *nvbox = gtk_vbox_new(FALSE,3);
	gtk_widget_set_size_request(GTK_WIDGET(nvbox),300,40);
	gtk_box_pack_start(GTK_BOX(nvbox),lmanager->priv->progress ,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(nvbox),lmanager->priv->module_load ,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(nvbox),lmanager->priv->small_progress ,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(lmanager->priv->load_box),nvbox,FALSE,FALSE,4);



	separator = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox),separator,FALSE,FALSE,4);
	gtk_widget_show(separator);
	gtk_widget_show(lmanager->priv->progress);
	gtk_widget_show(lmanager->priv->small_progress);
	gtk_widget_show(lmanager->priv->load_box);
	gtk_widget_show(nvbox);

	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(lmanager->priv->progress),"wait market...");
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(lmanager->priv->module_load),"load modules...");

	hbox = gtk_hbox_new(FALSE,3);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,4);
	//gtk_entry_set_visibility(GTK_ENTRY(lmanager->priv->password),FALSE);
	lmanager->priv->pass_label = gtk_label_new("Password:");
	lmanager->priv->password = gtk_entry_new();
	gtk_widget_set_size_request(GTK_WIDGET(lmanager->priv->pass_label),100,15);
	gtk_entry_set_visibility(GTK_ENTRY(lmanager->priv->password),FALSE);
	gtk_box_pack_start(GTK_BOX(hbox),lmanager->priv->pass_label,FALSE,FALSE,4);
	gtk_box_pack_start(GTK_BOX(hbox),lmanager->priv->password,FALSE,FALSE,4);

	gtk_widget_show(lmanager->priv->pass_label);
	gtk_widget_show(lmanager->priv->password);
	gtk_widget_show(hbox);
	g_signal_connect(lmanager->priv->password,"focus_in_event",
				 G_CALLBACK (gl_level_manager_focus_in_signal), (gpointer) lmanager);
	g_signal_connect(lmanager->priv->password,"changed",
					 G_CALLBACK (gl_level_manager_password_change_signal), (gpointer) lmanager);

	separator = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox),separator,FALSE,FALSE,4);
	gtk_widget_show(separator);

	hbox = gtk_hbox_new(FALSE,3);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,4);


	lmanager->priv->msg_label = gtk_label_new("Operator log:");
	gtk_widget_set_size_request(GTK_WIDGET(lmanager->priv->msg_label),100,15);
	gtk_box_pack_start(GTK_BOX(hbox),lmanager->priv->msg_label,FALSE,FALSE,4);
	lmanager->priv->msg = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),lmanager->priv->msg,FALSE,FALSE,4);
	gtk_widget_show(lmanager->priv->msg_label);
	g_signal_connect(lmanager->priv->msg,"focus_in_event",
			 G_CALLBACK (gl_level_manager_focus_in_signal), (gpointer) lmanager);

	lmanager->priv->button  = gtk_button_new();
	GtkWidget *image   = gtk_image_new_from_file("/lar/gui/dialog-ok-apply.png");
	gtk_widget_set_size_request(GTK_WIDGET(lmanager->priv->button),50,50);
	gtk_widget_set_sensitive(lmanager->priv->button,TRUE);
	gtk_container_add(GTK_CONTAINER(lmanager->priv->button),image);
	gtk_widget_set_sensitive(lmanager->priv->button,FALSE);
	label = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,4);
	g_signal_connect (lmanager->priv->button, "clicked",
					  G_CALLBACK (gl_level_manager_go_signal_clicked), (gpointer) lmanager);
	gtk_box_pack_start(GTK_BOX(hbox),lmanager->priv->button,FALSE,FALSE,4);

	gtk_widget_show(label);
	gtk_widget_show(lmanager->priv->msg);
	gtk_widget_show(image);
	gtk_widget_show(lmanager->priv->button);
	gtk_widget_show(hbox);

	gtk_widget_show(welcomImage);
	gtk_widget_show(label);
	gtk_widget_show(hbox);
	gtk_widget_show(vbox);
	gtk_widget_show(viewport);


	lmanager->priv->signal_watch = gtk_timeout_add(500,gui_system_signal_handler_idle,lmanager);

	return TRUE;
}

gboolean
gl_level_manager_show_welcome_window ( GlLevelManager  *lm )
{
	//printf("gui_lizens_show_welcome_window\n");
	g_return_val_if_fail(lm != NULL,FALSE);
	g_return_val_if_fail(GL_IS_LEVEL_MANAGER(lm),FALSE);
	if(gl_level_manager_create_welcome_window(lm))
	{
		if(GTK_WIDGET_IS_SENSITIVE(lm->priv->window)) return TRUE;
		gtk_widget_set_sensitive(lm->priv->window,TRUE);
		gtk_window_set_position(GTK_WINDOW(lm->priv->window ),GTK_WIN_POS_CENTER);
		gint x,y;
		gtk_window_get_position(GTK_WINDOW(lm->priv->window ),&x,&y);
		gtk_window_move(GTK_WINDOW (lm->priv->window),x,y-60);
		gtk_widget_show(lm->priv->window);
		gtk_entry_set_text(GTK_ENTRY(lm->priv->password),"");

		//mkt_trace("start level screen xkbd find\n");
		//mkt_trace("xkbd find\n");
		gl_xkbd_stop( );
		gint type = 0;
		type = GL_XKBD_TYPE_COMPACT;
		gl_xkbd_need_keyboard (lm->priv->password, lm->priv->password,type);
		//gl_xkbd_set_winid (xkbd,NULL);
		//gl_xkbd_start ();

	}
	if(lm->priv->market_watch == 0)
	{
		lm->priv->market_watch = gtk_timeout_add(900,gl_level_manager_market_watch_cb,lm);
	}
	gl_level_manager_change_level(lm,GUI_USER_DEVICE_TYPE);
	//printf("gui_lizens_show_welcome_window ende\n");
	return TRUE;
}

gboolean
gl_level_manager_hide_welcome_window(GlLevelManager *lm)
{
	g_return_val_if_fail(lm != NULL,TRUE);
	g_return_val_if_fail(GL_IS_LEVEL_MANAGER(lm),TRUE);
	if(gl_level_manager_create_welcome_window(lm))
	{
		gl_xkbd_stop();
		gtk_widget_set_sensitive(lm->priv->window,FALSE);
		gtk_widget_hide(lm->priv->window);
		if(lm->priv->market_watch)g_source_remove(lm->priv->market_watch);
		lm->priv->market_watch=0;
	}
	return TRUE;
}

static void
gl_level_manager_init (GlLevelManager *object)
{
	/* TODO: Add initialization code here */
	object->priv                    = G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_LEVEL_MANAGER,GlLevelManagerPrivate);
	object->priv->level             = GUI_USER_AUTORIZATION_DEVICE;
	object->priv->level_name        = g_strdup(gl_level_manager_get_level_name(object));
	object->priv->window            = NULL;
	object->priv->wait_status       = FALSE;
	object->priv->fraction          = FALSE;
	object->priv->fraction_small    = FALSE;

	//OLD:mkt_model_create(MKT_TYPE_ERROR_MESSAGE);
	//OLD:mkt_model_create(MKT_TYPE_LOG_MESSAGE);
}

static void
gl_level_manager_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	GlLevelManager *lmanager = (GL_LEVEL_MANAGER(object));
	g_free(lmanager->priv->level_name);
	G_OBJECT_CLASS (gl_level_manager_parent_class)->finalize (object);
}

static void
gl_level_manager_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_LEVEL_MANAGER (object));
	GlLevelManager *lmanager = (GL_LEVEL_MANAGER(object));
	switch (prop_id)
	{
	case PROP_LEVEL:
		gl_level_manager_change_level(lmanager,g_value_get_uint(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_level_manager_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_LEVEL_MANAGER (object));
	GlLevelManager *lmanager = (GL_LEVEL_MANAGER(object));
	switch (prop_id)
	{
	case PROP_LEVEL:
		g_value_set_uint(value,lmanager->priv->level);
		break;
	case PROP_LEVEL_NAME:
		g_value_set_static_string(value,lmanager->priv->level_name);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_level_manager_class_init (GlLevelManagerClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
//	GObjectClass* parent_class = G_OBJECT_CLASS (klass);
	g_type_class_add_private (klass, sizeof (GlLevelManagerPrivate));

	object_class->finalize     = gl_level_manager_finalize;
	object_class->set_property = gl_level_manager_set_property;
	object_class->get_property = gl_level_manager_get_property;

	klass -> change_gui_level  = NULL;
	klass -> key_open          = NULL;
	klass -> key_close         = NULL;
	GParamSpec *pspec;
	pspec = g_param_spec_uint("level",
			"Level",
			"Set/Get plugin level",
			0,G_MAXUINT32,GUI_USER_AUTORIZATION_DEVICE,
			G_PARAM_READABLE | G_PARAM_WRITABLE );
	g_object_class_install_property (object_class,
			PROP_LEVEL,pspec);
	pspec = g_param_spec_string("level_name",
			"Level name",
			"Get plugin level name",
			"device",
			G_PARAM_READABLE );
	g_object_class_install_property (object_class,
			PROP_LEVEL_NAME,pspec);

	gl_level_manager_signals[GL_LEVEL_MANAGER_CHANGE_LEVEL] =
			g_signal_new ("change_gui_level",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlLevelManagerClass, change_gui_level),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	gl_level_manager_signals[GL_LEVEL_MANAGER_MOUNT_USB] =
			g_signal_new ("mount_usb",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlLevelManagerClass, mount_usb),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	gl_level_manager_signals[GL_LEVEL_MANAGER_UMOUNT_USB] =
			g_signal_new ("umount_usb",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlLevelManagerClass, umount_usb),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	gl_level_manager_signals[GL_LEVEL_MANAGER_KEY_CLOSE] =
			g_signal_new ("level_key_close",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlLevelManagerClass, key_close),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	gl_level_manager_signals[GL_LEVEL_MANAGER_KEY_OPEN] =
			g_signal_new ("level_key_open",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlLevelManagerClass, key_open),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

GlLevelManager*
gl_level_manager_new ( const gchar *id , guint level )
{
	GlLevelManager *manager = GL_LEVEL_MANAGER( mkt_atom_object_new(GL_TYPE_LEVEL_MANAGER,MKT_ATOM_PN_ID,id,"level",level,NULL));
	return manager;
}

GlLevelManager*
gl_level_manager_get_static ( )
{
	if( security_level_manager == NULL )
	{
		security_level_manager =  gl_level_manager_new("com.lar.GlLevelManager.device-security",GUI_USER_DEVICE_TYPE);
	}
	return security_level_manager;
}

void
gl_level_manager_load ( )
{
	if( security_level_manager == NULL )
	{
		security_level_manager =  gl_level_manager_new("com.lar.GlLevelManager.device-security",GUI_USER_DEVICE_TYPE);
	}
}



gboolean
gl_level_manager_is_tru_user   ( GlLevelManagerUserType user_type)
{
	g_return_val_if_fail(security_level_manager != NULL,FALSE);
	g_return_val_if_fail(GL_IS_LEVEL_MANAGER(security_level_manager),FALSE);

	guint level = gl_level_manager_get_level(user_type);
	return (level &  security_level_manager->priv->level);
}

gboolean
gl_level_manager_key_open_close  ()
{
	g_return_val_if_fail(security_level_manager != NULL,FALSE);
	g_return_val_if_fail(GL_IS_LEVEL_MANAGER(security_level_manager),FALSE);
	if(!security_level_manager->priv->key_open) gl_level_manager_show_welcome_window(security_level_manager);
	else gl_level_manager_close(security_level_manager);
	return TRUE;
}


guint
gl_level_manager_get_file_level_type    (  const gchar *filename )
{
	g_return_val_if_fail(security_level_manager != NULL,GUI_USER_LAST_TYPE);
	g_return_val_if_fail(GL_IS_LEVEL_MANAGER(security_level_manager),GUI_USER_LAST_TYPE);
	if(g_str_has_prefix(filename,"measurement")&& g_str_has_suffix(filename,".csv"))
		return GUI_USER_DEVICE_TYPE;
	if(g_str_has_prefix(filename,"measurement"))
		return GUI_USER_DEVICE_TYPE;
	if(g_str_has_prefix(filename,"signal")&& g_str_has_suffix(filename,".csv"))
		return GUI_USER_SERVICE_TYPE;
	if(g_str_has_prefix(filename,"signal"))
		return GUI_USER_SERVICE_TYPE;
	if(g_str_has_prefix(filename,"logbook")&& g_str_has_suffix(filename,".csv"))
		return GUI_USER_OPERATOR_TYPE;
	if(g_str_has_suffix(filename,".csv"))
		return GUI_USER_ROOT_TYPE;

	return GUI_USER_ROOT_TYPE;
}


void
gl_level_manager_set_load_module_fraction   ( gdouble fraction )
{
	//TEST:g_debug("gl_level_manager_set_load_module_fraction %f ",fraction);
	g_return_if_fail(security_level_manager != NULL);
	g_return_if_fail(GL_IS_LEVEL_MANAGER(security_level_manager));
	g_return_if_fail( (security_level_manager->priv->module_load && GTK_IS_WIDGET(security_level_manager->priv->module_load)) );
	if(fraction < 0.0 || fraction > 1.0)
	{
		gtk_widget_hide(security_level_manager->priv->module_load);
	}
	else
	{
		//TEST:g_debug("Test fraction=%f ",fraction);
		gtk_widget_show(security_level_manager->priv->load_box);
		gtk_widget_show(security_level_manager->priv->module_load);
		if(GTK_IS_PROGRESS_BAR(security_level_manager->priv->module_load))
		{
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(security_level_manager->priv->module_load),fraction);
		}
	}

}

static void
gl_level_manager_mount_usb ( GlLevelManager * level_manager )
{
	g_return_if_fail(level_manager != NULL);
	g_return_if_fail(GL_IS_LEVEL_MANAGER(level_manager));
	g_signal_emit(level_manager,gl_level_manager_signals[GL_LEVEL_MANAGER_MOUNT_USB],0);
}

static void
gl_level_manager_umount_usb ( GlLevelManager * level_manager )
{
	g_return_if_fail(level_manager != NULL);
	g_return_if_fail(GL_IS_LEVEL_MANAGER(level_manager));
	g_signal_emit(level_manager,gl_level_manager_signals[GL_LEVEL_MANAGER_UMOUNT_USB],0);
}


static void
gl_level_manager_system_larkey_open(GlLevelManager * level_manager)
{
	//TEST:	g_debug("gl_level_manager_system_larkey_open\n");
	g_return_if_fail(level_manager != NULL);
	g_return_if_fail(GL_IS_LEVEL_MANAGER(level_manager));
	if(g_file_test (GL_LEVEL_MANAGER_RECHTELEVEL, G_FILE_TEST_EXISTS))
	{
		g_warning("show welcome windows");
		gl_level_manager_show_welcome_window(level_manager);

	}
	else
	{
		g_warning("Rechtelevel file not exist... ");
	}
}

static void
gl_level_manager_system_larkey_close(GlLevelManager * level_manager)
{
	//g_debug("gl_level_manager_system_larkey_close\n");
	g_return_if_fail(level_manager != NULL);
	g_return_if_fail(GL_IS_LEVEL_MANAGER(level_manager));
	if(!level_manager->priv->wait_status )
		gl_level_manager_close(level_manager);
	else if(g_file_test (GL_LEVEL_MANAGER_RECHTELEVEL, G_FILE_TEST_EXISTS))
		g_object_set(G_OBJECT(level_manager),"level",GUI_USER_DEVICE_TYPE,NULL);
}

typedef struct __GuiSysSignal  GuiSysSignal;

struct __GuiSysSignal
{
	unsigned int sigin;
	unsigned int signum;

};


static GuiSysSignal SysSignal = {0,0};

gboolean
gui_system_signal_handler_idle(gpointer data)
{
	if( SysSignal.sigin )
	{
		switch(SysSignal.signum)
		{
			case SIGUSR1:gl_level_manager_mount_usb(GL_LEVEL_MANAGER(data));gl_level_manager_system_larkey_open (GL_LEVEL_MANAGER(data));break;
			case SIGUSR2:gl_level_manager_umount_usb(GL_LEVEL_MANAGER(data));gl_level_manager_system_larkey_close(GL_LEVEL_MANAGER(data));break;
			default:      break;
		}
		SysSignal.sigin = 0;
	}
	return TRUE;
}

void
gl_level_manager_system_signal_handler(int signum)
{
	SysSignal.signum = signum;
	SysSignal.sigin  = 1;
}


void
gl_level_manager_mount_usb_force   ( )
{
	gl_level_manager_mount_usb(security_level_manager);
}
void
gl_level_manager_umount_usb_force   ( )
{
	gl_level_manager_umount_usb(security_level_manager);
}


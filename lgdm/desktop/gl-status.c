/*
 * gtklarstatus.c
 *
 *  Created on: 24.05.2011
 *      Author: asmolkov
 */

#include "gl-status.h"

#include <mktbus.h>
#include <string.h>

#include "../config.h"
#include <glib/gi18n-lib.h>

#define STATUS_MAX_PARAMS  15




//static GlStatus *__gui_process_status = NULL;

struct _GlStatusPrivate
{
	GtkButton               *show_desktop;
	GtkButton               *time_button;
	GtkLabel                *labeltime;
	GtkLabel                *labellevel;

	GtkBox                  *process_box;
	GtkSpinner              *process_runned;
	GtkRevealer             *status_revealer;
	GtkLabel                *status_message;
	GtkBox                  *status_action_box;

	guint                    level;
	gint                     timer_tag;
	GSettings               *status_settings;
	//  status windows ----------------------------
	GSList                  *last_plugin;
	GList                   *indicate;
	GList                   *widgets;
	GTimer                  *info_timer;
	LarpcDevice             *pc_device;
};


enum {
	GL_STATUS_PROP_NULL,
	GL_STATUS_PROP_POSITION,
};


enum
{
	GL_STATUS_SHOW_DESKTOP,
	GL_STATUS_SHOW_STATUS,
	GL_STATUS_LAST_SIGNAL
};


static guint gl_status_signals[GL_STATUS_LAST_SIGNAL] = { 0 };


G_DEFINE_TYPE_WITH_PRIVATE (GlStatus, gl_status, GTK_TYPE_BOX);


static void
gl_status_show_desktop ( GlStatus *status )
{
}

static void
show_process_clicked_cb (GlStatus *status, GtkButton *buttton)
{
	g_return_if_fail(status != NULL);
	g_return_if_fail(GL_IS_STATUS(status));
	if(gtk_widget_is_visible(GTK_WIDGET(status->priv->status_revealer)))
		gtk_widget_hide(GTK_WIDGET(status->priv->status_revealer));
	else
		gtk_widget_show(GTK_WIDGET(status->priv->status_revealer));
}

static void
show_status_clicked_cb (GlStatus *status, GtkButton *buttton)
{
	g_return_if_fail(status != NULL);
	g_return_if_fail(GL_IS_STATUS(status));
	g_signal_emit(status, gl_status_signals[GL_STATUS_SHOW_STATUS],0);
}

static void
show_desktop_clicked_cb (GlStatus *status, GtkButton *buttton)
{
	g_return_if_fail(status != NULL);
	g_return_if_fail(GL_IS_STATUS(status));
	g_signal_emit(status, gl_status_signals[GL_STATUS_SHOW_DESKTOP],0);
	//GlStatus *status = GL_STATUS(data);
	//mkt_window_manager_show_desktop();
}

static void
time_button_clicked_cb (GlStatus *status, GtkButton *buttton)
{
	g_return_if_fail(status != NULL);
	g_return_if_fail(GL_IS_STATUS(status));
	//GlStatus *status = GL_STATUS(data);
	//mkt_window_manager_show_desktop();
}

/*
static void
gl_status_change_level(GlManager *manager,GlLevelManager *level)
{

}

*/

static void
gl_status_level_update          ( GlStatus *status  )
{
	gchar *level_str= g_strdup_printf("Level %d",security_device_get_level(TERA_GUARD()));
	gtk_label_set_text(status->priv->labellevel,level_str);
	status->priv->level=security_device_get_level(TERA_GUARD());
}

static void
gl_status_level_changed_desktop ( SecurityDevice  *guard ,GParamSpec *pspec , GlStatus *status )
{
	if(security_device_get_level(TERA_GUARD())<status->priv->level)
	{
		gboolean is_done = FALSE;
		g_signal_emit(status, gl_status_signals[GL_STATUS_SHOW_DESKTOP],0);
		security_device_call_logout_sync(TERA_GUARD(),&is_done,NULL,NULL);
	}
	gl_status_level_update(status);
}


static gboolean
gl_status_update_time    ( GlStatus *status )
{
	g_return_val_if_fail(status  != NULL,FALSE);
	g_return_val_if_fail(GL_IS_STATUS(status),FALSE);
	gchar *format = g_settings_get_string(status->priv->status_settings,"time-format");
	time_t T;
	struct tm  Td;
	char timeBuf[64];
	memset(timeBuf,0,64);
	T  = time(NULL);
	Td = *localtime(&T);
	strftime(timeBuf,30,format,&Td);
	//g_debug("Set time label \n");
	gtk_label_set_text(GTK_LABEL(status->priv->labeltime),timeBuf);
	return TRUE;
}

static void
gl_status_init(GlStatus *status)
{
	g_return_if_fail (status != NULL);
	g_return_if_fail (GL_IS_STATUS(status));
	status->priv = gl_status_get_instance_private (status);
	gtk_widget_init_template (GTK_WIDGET (status));
	status->priv->indicate           = NULL;
	status->priv->last_plugin        = NULL;
	status->priv->info_timer         = g_timer_new();
	status->priv->status_settings    = g_settings_new("com.lar.LGDM.Status");
	status->priv->level              = 0;

	//gtk_window_set_decorated(GTK_WINDOW(status),FALSE);
	status->priv->timer_tag = g_timeout_add(200,(GSourceFunc)gl_status_update_time,status);

	g_signal_connect(TERA_GUARD(),"notify::level",G_CALLBACK(gl_status_level_changed_desktop),status);


//  Status window -----------------------------------------------------------
}

static void
gl_status_constructed (GObject *object)
{
	GlStatus* status = GL_STATUS(object);
	if(ultra_control_client_object()!=NULL)
	{
		g_object_bind_property(ultra_control_client_object(),"busy",status->priv->process_box,"visible",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
		g_object_bind_property(ultra_control_client_object(),"busy",status->priv->process_runned,"active",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
		g_object_bind_property(ultra_control_client_object(),"status",status->priv->status_message,"label",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
	}
	gl_status_level_update(status);
	G_OBJECT_CLASS (gl_status_parent_class)->constructed(object);
}


static void
gl_status_finalize (GObject *object)
{
	GlStatus* status = GL_STATUS(object);
	if(status->priv->info_timer)g_timer_destroy(status->priv->info_timer);
	if(status->priv->timer_tag)g_source_remove(status->priv->timer_tag);
	G_OBJECT_CLASS (gl_status_parent_class)->finalize(object);
}

static void
gl_status_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Set (GL_MANAGER) property \n");
	g_return_if_fail (GL_IS_STATUS(object));
	//GlStatus* status = GL_STATUS(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_status_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Get (GL_MANAGER) property \n");
	g_return_if_fail (GL_IS_STATUS(object));
	//GlStatus* status = GL_STATUS(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}



void gl_status_class_init(GlStatusClass *klass)
{
	GObjectClass*         object_class     =  G_OBJECT_CLASS (klass);

	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/layout/status.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GlStatus, show_desktop);
	gtk_widget_class_bind_template_child_private (widget_class, GlStatus, time_button);
	gtk_widget_class_bind_template_child_private (widget_class, GlStatus, labeltime);
	gtk_widget_class_bind_template_child_private (widget_class, GlStatus, labellevel);
	gtk_widget_class_bind_template_child_private (widget_class, GlStatus, process_box);
	gtk_widget_class_bind_template_child_private (widget_class, GlStatus, status_revealer);
	gtk_widget_class_bind_template_child_private (widget_class, GlStatus, status_message);
	gtk_widget_class_bind_template_child_private (widget_class, GlStatus, process_runned);
	gtk_widget_class_bind_template_child_private (widget_class, GlStatus, status_action_box);




	gtk_widget_class_bind_template_callback (widget_class, show_desktop_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, show_status_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, show_process_clicked_cb);



	gtk_widget_class_bind_template_callback (widget_class, time_button_clicked_cb);

	object_class -> finalize           =  gl_status_finalize;
	object_class -> set_property       =  gl_status_set_property;
	object_class -> get_property       =  gl_status_get_property;
	object_class -> constructed        =  gl_status_constructed;
	klass->show_desktop                =  gl_status_show_desktop;



	gl_status_signals[GL_STATUS_SHOW_DESKTOP] =
			g_signal_new ("show-desktop",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET ( GlStatusClass, show_desktop),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	gl_status_signals[GL_STATUS_SHOW_STATUS] =
			g_signal_new ("show-status",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					0,
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

}

GlStatus*
gl_status_new ( )
{
	GlStatus  *status;
	status   = GL_STATUS(g_object_new( GL_TYPE_STATUS,NULL));
	return     status;
}

void
gl_status_add_action              ( GlStatus *status , GtkWidget *action )
{
	g_return_if_fail (status!=NULL);
	g_return_if_fail (GL_IS_STATUS(status));
	g_return_if_fail (action!=NULL);
	g_return_if_fail (GTK_IS_WIDGET(action));
	gtk_box_pack_start(status->priv->status_action_box,action,TRUE,TRUE,1);
}


void
gl_status_set_application_name    ( GlStatus *status , const gchar *application_name)
{
	g_return_if_fail (status!=NULL);
	g_return_if_fail (GL_IS_STATUS(status));
}


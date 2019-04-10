/*
 *  Status bar .
 *  author: asmolkov@lar.com
 *  LAR 2013
 */

#include "lgdm-status.h"
#include "lgdm-desktop.h"

// #include <mktbus.h>
#include <string.h>

#include "ultimate-config.h"
#include <glib/gi18n-lib.h>

#define STATUS_MAX_PARAMS  15




//static LgdmStatus *__gui_process_status = NULL;

struct _LgdmStatusPrivate
{
	GtkButton               *show_desktop;
	GtkButton               *time_button;
	GtkLabel                *labeltime;
	GtkLabel                *labellevel;
	GtkWidget               *vnc_activ;
	GtkBox                  *process_box;
	GtkSpinner              *process_runned;
	GtkRevealer             *status_revealer;
	GtkLabel                *status_message;
	GtkBox                  *status_action_box;

	gint                     timer_tag;
	GSettings               *status_settings;
	//  status windows ----------------------------
	GSList                  *last_plugin;
	GList                   *indicate;
	GList                   *widgets;
	GTimer                  *info_timer;
};


enum {
	LGDM_STATUS_PROP_NULL,
	LGDM_STATUS_PROP_POSITION,
};


enum
{
	LGDM_STATUS_SHOW_DESKTOP,
	LGDM_STATUS_SHOW_STATUS,
	LGDM_STATUS_LAST_SIGNAL
};


static guint lgdm_status_signals[LGDM_STATUS_LAST_SIGNAL] = { 0 };


G_DEFINE_TYPE_WITH_PRIVATE (LgdmStatus, lgdm_status, GTK_TYPE_BOX);


static void
show_process_clicked_cb (LgdmStatus *status, GtkButton *buttton)
{
	g_return_if_fail(status != NULL);
	g_return_if_fail(LGDM_IS_STATUS(status));
	if(gtk_widget_is_visible(GTK_WIDGET(status->priv->status_revealer)))
		gtk_widget_hide(GTK_WIDGET(status->priv->status_revealer));
	else
		gtk_widget_show(GTK_WIDGET(status->priv->status_revealer));
}

static void
show_status_clicked_cb (LgdmStatus *status, GtkButton *buttton)
{
	g_return_if_fail(status != NULL);
	g_return_if_fail(LGDM_IS_STATUS(status));
	g_signal_emit(status, lgdm_status_signals[LGDM_STATUS_SHOW_STATUS],0);
}

static void
show_desktop_clicked_cb (LgdmStatus *status, GtkButton *buttton)
{
	g_return_if_fail(status != NULL);
	g_return_if_fail(LGDM_IS_STATUS(status));
	g_signal_emit(status, lgdm_status_signals[LGDM_STATUS_SHOW_DESKTOP],0);
	//LgdmStatus *status = LGDM_STATUS(data);
	//mkt_window_manager_show_desktop();
}

static void
time_button_clicked_cb (LgdmStatus *status, GtkButton *buttton)
{
	g_return_if_fail(status != NULL);
	g_return_if_fail(LGDM_IS_STATUS(status));
	//LgdmStatus *status = LGDM_STATUS(data);
	//mkt_window_manager_show_desktop();
}

static void
lgdm_status_level_update          ( LgdmStatus *status  )
{
    gchar *level_str= g_strdup_printf("Level %d",lgdm_state_get_level(lgdm_state()));
	gtk_label_set_text(status->priv->labellevel,level_str);
	g_free(level_str);
}



static void
level_changed ( LgdmState *state ,GParamSpec *pspec , LgdmStatus *status )
{
	// if( tera_security_client_get_security() && security_device_get_level(TERA_GUARD())<status->priv->level)
	// {
	// 	gboolean is_done = FALSE;
	// 	g_signal_emit(status, lgdm_status_signals[LGDM_STATUS_SHOW_DESKTOP],0);
	// 	security_device_call_logout_sync(TERA_GUARD(),&is_done,NULL,NULL);
	// }
	lgdm_status_level_update(status);
}

static gboolean
lgdm_status_update_time    ( LgdmStatus *status )
{
	g_return_val_if_fail(status  != NULL,FALSE);
	g_return_val_if_fail(LGDM_IS_STATUS(status),FALSE);
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
static gboolean
lgdm_status_update_vnc    ( LgdmStatus *status )
{
	g_return_val_if_fail(status  != NULL,FALSE);
	g_return_val_if_fail(LGDM_IS_STATUS(status),FALSE);
	static guint count =1;
	if(gtk_widget_get_visible(status->priv->vnc_activ)){
		if(count>3)count=1;
		gchar *res_name = g_strdup_printf("/lgdm/image/vnc-0%d.svg",count);
		gtk_image_set_from_resource(GTK_IMAGE(status->priv->vnc_activ),res_name);
		count++;

	}
	return TRUE;
}

static void
lgdm_status_init(LgdmStatus *status)
{
	g_return_if_fail (status != NULL);
	g_return_if_fail (LGDM_IS_STATUS(status));
	status->priv = lgdm_status_get_instance_private (status);
	gtk_widget_init_template (GTK_WIDGET (status));
	status->priv->indicate           = NULL;
	status->priv->last_plugin        = NULL;
	status->priv->info_timer         = g_timer_new();
	// status->priv->status_settings    = g_settings_new("com.lar.LGDM.Status");

	//gtk_window_set_decorated(GTK_WINDOW(status),FALSE);
	status->priv->timer_tag = g_timeout_add(200,(GSourceFunc)lgdm_status_update_time,status);
	g_timeout_add(400,(GSourceFunc)lgdm_status_update_vnc,status);
}

static void
lgdm_status_constructed (GObject *object)
{
	LgdmStatus* status = LGDM_STATUS(object);
    g_signal_connect(lgdm_state(),"notify::level",G_CALLBACK(level_changed),status);
    g_object_bind_property(lgdm_state(),"vnc-connected",status->priv->vnc_activ,"visible",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
    lgdm_status_level_update(status);
    g_object_bind_property(lgdm_state(),"busy",status->priv->process_box,"visible",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
    g_object_bind_property(lgdm_state(),"busy",status->priv->process_runned,"active",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
    g_object_bind_property(lgdm_state(),"status",status->priv->status_message,"label",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);

	G_OBJECT_CLASS (lgdm_status_parent_class)->constructed(object);
}


static void
lgdm_status_finalize (GObject *object)
{
	LgdmStatus* status = LGDM_STATUS(object);
	if(status->priv->info_timer)g_timer_destroy(status->priv->info_timer);
	if(status->priv->timer_tag)g_source_remove(status->priv->timer_tag);
	G_OBJECT_CLASS (lgdm_status_parent_class)->finalize(object);
}

static void
lgdm_status_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Set (LGDM_MANAGER) property \n");
	g_return_if_fail (LGDM_IS_STATUS(object));
	//LgdmStatus* status = LGDM_STATUS(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
lgdm_status_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Get (LGDM_MANAGER) property \n");
	g_return_if_fail (LGDM_IS_STATUS(object));
	//LgdmStatus* status = LGDM_STATUS(object);
	switch (prop_id)
	{
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}



void lgdm_status_class_init(LgdmStatusClass *klass)
{
	GObjectClass*         object_class     =  G_OBJECT_CLASS (klass);

	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/layout/status.ui");
	gtk_widget_class_bind_template_child_private (widget_class, LgdmStatus, show_desktop);
	gtk_widget_class_bind_template_child_private (widget_class, LgdmStatus, time_button);
	gtk_widget_class_bind_template_child_private (widget_class, LgdmStatus, labeltime);
	gtk_widget_class_bind_template_child_private (widget_class, LgdmStatus, labellevel);
	gtk_widget_class_bind_template_child_private (widget_class, LgdmStatus, process_box);
	gtk_widget_class_bind_template_child_private (widget_class, LgdmStatus, status_revealer);
	gtk_widget_class_bind_template_child_private (widget_class, LgdmStatus, status_message);
	gtk_widget_class_bind_template_child_private (widget_class, LgdmStatus, process_runned);
	gtk_widget_class_bind_template_child_private (widget_class, LgdmStatus, status_action_box);

	gtk_widget_class_bind_template_child_private (widget_class, LgdmStatus,vnc_activ);



	gtk_widget_class_bind_template_callback (widget_class, show_desktop_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, show_status_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, show_process_clicked_cb);



	gtk_widget_class_bind_template_callback (widget_class, time_button_clicked_cb);

	object_class -> finalize           =  lgdm_status_finalize;
	object_class -> set_property       =  lgdm_status_set_property;
	object_class -> get_property       =  lgdm_status_get_property;
	object_class -> constructed        =  lgdm_status_constructed;


}

LgdmStatus*
lgdm_status_new ( )
{
	LgdmStatus  *status;
	status   = LGDM_STATUS(g_object_new( LGDM_TYPE_STATUS,NULL));
	return     status;
}

void
lgdm_status_add_action              ( LgdmStatus *status , GtkWidget *action )
{
	g_return_if_fail (status!=NULL);
	g_return_if_fail (LGDM_IS_STATUS(status));
	g_return_if_fail (action!=NULL);
	g_return_if_fail (GTK_IS_WIDGET(action));
	gtk_box_pack_start(status->priv->status_action_box,action,TRUE,TRUE,1);
}


void
lgdm_status_set_application_name    ( LgdmStatus *status , const gchar *application_name)
{
	g_return_if_fail (status!=NULL);
	g_return_if_fail (LGDM_IS_STATUS(status));
}


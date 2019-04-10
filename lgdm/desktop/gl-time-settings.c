/*
 * @ingroup GlTimeSettings
 * @{
 * @file  gl-desktop-action.c	LGDM desktop action button
 * @brief LGDM desktop action button.
 *
 *
 *  Copyright (C) LAR 2014
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */



#include "gl-time-settings.h"

#include <string.h>
#include <math.h>

#include "../config.h"
#include <glib/gi18n-lib.h>




//static GlTimeSettings *__gui_process_desktop = NULL;

struct _GlTimeSettingsPrivate
{


	GtkSpinButton           *hours;
	GtkSpinButton           *minutes;
	GtkSpinButton           *seconds;
	guint                    move_tag;

	GtkBox                  *hours_box;
	GtkBox                  *minutes_box;
	GtkBox                  *seconds_box;

	GtkLabel                *value_name;
	GtkLabel                *cancel_name;
	GtkLabel                *hours_name;
	GtkLabel                *minutes_name;
	GtkLabel                *seconds_name;

	gdouble                  h;
	gdouble                  m;
	gdouble                  s;
	gdouble                  total_sec;
	gboolean                 activate_sec;
};


enum {
	GL_TIME_SETTINGS_PROP_NULL,
	GL_TIME_SETTINGS_VALUE_NAME,
	GL_TIME_SETTINGS_HOUR,
	GL_TIME_SETTINGS_MINUTES,
	GL_TIME_SETTINGS_SECONDS,
	GL_TIME_SETTINGS_ACTIVATE_SECONDS,
	GL_TIME_SETTINGS_TOTAL_SECONDS

};


enum
{
	GL_TIME_SETTINGS_LAST_SIGNAL
};


//static guint gl_time_settings_signals[GL_TIME_SETTINGS_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlTimeSettings, gl_time_settings, GTK_TYPE_WINDOW);



//static gboolean total_seconds_realise_freeze = FALSE;


static void
time_wraped_seconds ( GlTimeSettings* time , GtkSpinButton *button )
{
	if(gtk_spin_button_get_value(button)<0.1)
	{
		g_object_set(time,"minutes",time->priv->m+1,NULL);
	}
	else
	{
		g_object_set(time,"minutes",time->priv->m-1,NULL);
	}
}

static void
time_wraped_minutes ( GlTimeSettings* time , GtkSpinButton *button )
{
	if(gtk_spin_button_get_value(button)<0.1)
	{
		g_object_set(time,"hours",time->priv->h+1,NULL);
	}
	else
	{
		g_object_set(time,"hours",time->priv->h-1,NULL);
	}
}


static void
time_realize_hours(GlTimeSettings* time)
{

	GtkAdjustment *adj = gtk_spin_button_get_adjustment(time->priv->hours);
	gdouble lower = gtk_adjustment_get_lower(adj);
	gdouble upper = gtk_adjustment_get_upper(adj);
	if(time->priv->h > upper)
	{
		g_object_set(time,"hours",lower,NULL);
	}
	else if(time->priv->h < lower)
	{
		g_object_set(time,"hours",upper,NULL);
	}
}

static void
time_realize_minutes(GlTimeSettings* time)
{
	if(time->priv->m > 59.0)
	{
		g_object_set(time,"hours",time->priv->h+1,"minutes",0.0,NULL);

	}
	else if(time->priv->m < 0.0)
	{
		g_object_set(time,"hours",time->priv->h-1,"minutes",59.0,NULL);
	}
}

static void
time_realize_seconds(GlTimeSettings* time)
{
}
static void
time_realize_total_seconds(GlTimeSettings* time)
{

	//g_debug("%d:%d:%d",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	gdouble h = round((time->priv->total_sec/3600.0)-0.500);
	if(h<1.0)h=0.0;
	gdouble m = round((time->priv->total_sec - (h*3600.0))/60);
	if(m<1.0)m=0.0;
	gdouble s = (time->priv->total_sec - (h*3600.0)) -(m*60);
	if(s<1.0)s=0.0;
	g_object_set(time,"hours",h,"minutes",m,"seconds",s,NULL);

}

static void
time_settings_set_clicked_cb  ( GlTimeSettings *time , GtkButton *button )
{
	//g_debug("%f - %f - %f",time->priv->h,time->priv->m,time->priv->s);
	gdouble hs = time->priv->h*60.0*60.0;
	gdouble ms = time->priv->m*60.0;
	gdouble ss = time->priv->s*60.0;
	if(!time->priv->activate_sec) ss = 0.0;
	gl_time_settings_stop(time);
	g_object_set(time,"total-seconds",hs+ms+ss,NULL);
	gtk_widget_hide(GTK_WIDGET(time));
}

static void
time_settings_cancel_cb ( GlTimeSettings *time , GtkButton *button )
{
	gtk_widget_hide(GTK_WIDGET(time));
}



static gboolean
time_setting_dialog_move_callback ( gpointer user_data )
{
	GlTimeSettings* dialog = GL_TIME_SETTINGS(user_data);
	gtk_window_move(GTK_WINDOW(dialog),1,1);
	dialog->priv->move_tag = 0;
	return FALSE;
}


static void
time_settings_dialog_start_visible (GObject *object ,GParamSpec *pspec , GlTimeSettings *dialog)
{
	time_realize_total_seconds(dialog);
	if(dialog->priv->move_tag == 0)
		g_timeout_add(20,time_setting_dialog_move_callback,dialog);
}

static void
gl_time_settings_init(GlTimeSettings *time)
{
	g_return_if_fail (time != NULL);
	g_return_if_fail (GL_IS_TIME_SETTINGS(time));
	time->priv = gl_time_settings_get_instance_private (time);
	gtk_widget_init_template (GTK_WIDGET (time));

}



static void
gl_time_settings_finalize (GObject *object)
{
	//GlTimeSettings* desktop = GL_TIME_SETTINGS(object);
	G_OBJECT_CLASS (gl_time_settings_parent_class)->finalize(object);
}

static void
gl_time_settings_constructed (GObject *object)
{
	GlTimeSettings* time = GL_TIME_SETTINGS(object);
	time->priv->move_tag = 0;
	g_object_bind_property(time->priv->hours,"value",time,"hours",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(time->priv->minutes,"value",time,"minutes",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(time->priv->seconds,"value",time,"seconds",G_BINDING_BIDIRECTIONAL);
	g_object_bind_property(time->priv->seconds_box,"visible",time,"activate-seconds",G_BINDING_BIDIRECTIONAL);
	g_signal_connect(time,"notify::visible",G_CALLBACK(time_settings_dialog_start_visible),time);

	gtk_label_set_text(time->priv->cancel_name,_("CANCEL"));
	gtk_label_set_text(time->priv->hours_name,_("Hours"));
	gtk_label_set_text(time->priv->minutes_name,_("Minutes"));
	gtk_label_set_text(time->priv->seconds_name,_("Seconds"));

	if(G_OBJECT_CLASS (gl_time_settings_parent_class)->constructed)
		G_OBJECT_CLASS (gl_time_settings_parent_class)->constructed(object);
}



static void
gl_time_settings_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Set (GL_MANAGER) property \n");
	g_return_if_fail (GL_IS_TIME_SETTINGS(object));
	GlTimeSettings* time = GL_TIME_SETTINGS(object);
	switch (prop_id)
	{
	case GL_TIME_SETTINGS_VALUE_NAME:
		gtk_label_set_text(time->priv->value_name,g_value_get_string(value));
		break;
	case GL_TIME_SETTINGS_HOUR:
		time->priv->h = g_value_get_double(value);
		time_realize_hours(time);
		break;
	case GL_TIME_SETTINGS_MINUTES:
		time->priv->m = g_value_get_double(value);
		time_realize_minutes(time);
		break;
	case GL_TIME_SETTINGS_SECONDS:
		time->priv->s = g_value_get_double(value);
		time_realize_seconds(time);
		break;
	case GL_TIME_SETTINGS_ACTIVATE_SECONDS:
		time->priv->activate_sec = g_value_get_boolean(value);
		break;
	case GL_TIME_SETTINGS_TOTAL_SECONDS:
		time->priv->total_sec = g_value_get_double(value);
		time_realize_total_seconds (time);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_time_settings_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_TIME_SETTINGS(object));
	GlTimeSettings* time = GL_TIME_SETTINGS(object);
	switch (prop_id)
	{
	case GL_TIME_SETTINGS_VALUE_NAME:
		g_value_set_string(value,gtk_label_get_text(time->priv->value_name));
		break;
	case GL_TIME_SETTINGS_HOUR:
		g_value_set_double(value,time->priv->h);
		break;
	case GL_TIME_SETTINGS_MINUTES:
		g_value_set_double(value,time->priv->m);
		break;
	case GL_TIME_SETTINGS_SECONDS:
		g_value_set_double(value,time->priv->s);
		break;
	case GL_TIME_SETTINGS_ACTIVATE_SECONDS:
		g_value_set_boolean(value,time->priv->activate_sec);
		break;
	case GL_TIME_SETTINGS_TOTAL_SECONDS:
		g_value_set_double(value,time->priv->total_sec);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_time_settings_class_init(GlTimeSettingsClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	object_class -> finalize               =  gl_time_settings_finalize;
	object_class -> set_property           =  gl_time_settings_set_property;
	object_class -> get_property           =  gl_time_settings_get_property;
	object_class -> constructed            =  gl_time_settings_constructed;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/dialog/time_setting.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, value_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, hours);
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, minutes);
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, seconds);
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, hours_box);
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, minutes_box);
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, seconds_box);

	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, cancel_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, hours_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, minutes_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlTimeSettings, seconds_name);



	gtk_widget_class_bind_template_callback (widget_class, time_wraped_minutes);
	gtk_widget_class_bind_template_callback (widget_class, time_wraped_seconds);
	gtk_widget_class_bind_template_callback (widget_class, time_settings_set_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, time_settings_cancel_cb);


	g_object_class_install_property (object_class,GL_TIME_SETTINGS_VALUE_NAME,
			g_param_spec_string ("value-name",
					"Time settings show seconds",
					"Time settings show seconds",
					"No value",
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));

	g_object_class_install_property (object_class,GL_TIME_SETTINGS_HOUR,
			g_param_spec_double  ("hours",
					"Time settings hours",
					"Time settings hours",
					-1.0,240.0,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
	g_object_class_install_property (object_class,GL_TIME_SETTINGS_MINUTES,
			g_param_spec_double  ("minutes",
					"Time settings hours",
					"Time settings hours",
					-1.0,60.0,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
	g_object_class_install_property (object_class,GL_TIME_SETTINGS_SECONDS,
			g_param_spec_double  ("seconds",
					"Time settings hours",
					"Time settings hours",
					-1.0,60.0,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
	g_object_class_install_property (object_class,GL_TIME_SETTINGS_ACTIVATE_SECONDS,
			g_param_spec_boolean ("activate-seconds",
					"Time settings show seconds",
					"Time settings show seconds",
					TRUE,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
	g_object_class_install_property (object_class,GL_TIME_SETTINGS_TOTAL_SECONDS,
			g_param_spec_double  ("total-seconds",
					"Time settings total seconds",
					"Time settings total seconds",
					0.0,864000,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));

}

//FIXME : plug in info window (last plugin ) Start last plugin from list..
GtkWidget*
gl_time_settings_new ( )
{
	GtkWidget *action;
	action   = GTK_WIDGET(g_object_new( GL_TYPE_TIME_SETTINGS,NULL));
	return     action;
}

void
gl_time_settings_stop                    ( GlTimeSettings *time )
{
	g_return_if_fail(time!=NULL);
	g_return_if_fail(GL_IS_TIME_SETTINGS(time));
	gtk_widget_hide(GTK_WIDGET(time));
}

void
gl_time_settings_start                    ( GlTimeSettings *time )
{
	g_return_if_fail(time!=NULL);
	g_return_if_fail(GL_IS_TIME_SETTINGS(time));
	gtk_widget_show(GTK_WIDGET(time));
}

void
gl_time_settings_set_hour_interval            ( GlTimeSettings *time ,  gdouble min , gdouble max )
{
	GtkAdjustment *adj = gtk_spin_button_get_adjustment(time->priv->hours);
	gtk_adjustment_set_upper(adj,max);
	gtk_adjustment_set_lower(adj,min);
}


gdouble
gl_time_settings_get_total_seconds       ( GlTimeSettings *time )
{
	g_return_val_if_fail(time != NULL,0.0);
	g_return_val_if_fail(GL_IS_TIME_SETTINGS(time),0.0);
	return time->priv->total_sec;
}


/** @} */

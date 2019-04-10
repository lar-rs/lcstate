/*
 * @ingroup GlDateSettings
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



#include "gl-date-settings.h"

#include <string.h>
#include <mktlib.h>

#include "../config.h"
#include <glib/gi18n-lib.h>



//static GlDateSettings *__gui_process_desktop = NULL;

struct _GlDateSettingsPrivate
{


	GtkSpinButton           *day;
	GtkSpinButton           *month;
	GtkSpinButton           *year;
	GtkAdjustment           *days_adj;

	GtkBox                  *date_box;
	GtkBox                  *day_box;
	GtkBox                  *month_box;
	GtkBox                  *year_box;
	GtkLabel                *value_name;
	GtkLabel                *cancel_name;
	GtkLabel                *set_label;

	GtkLabel                *day_headline;
	GtkLabel                *month_headline;
	GtkLabel                *year_headline;

	gdouble                  total_sec;

	guint                    move_tag;
};


enum {
	GL_DATE_SETTINGS_PROP_NULL,
	GL_DATE_SETTINGS_VALUE_NAME,
	GL_DATE_SETTINGS_YEAR,
	GL_DATE_SETTINGS_DAY,
	GL_DATE_SETTINGS_MOUNTH,
	GL_DATE_SETTINGS_TOTAL_SECONDS

};


enum
{
	GL_DATE_SETTINGS_LAST_SIGNAL
};


//static guint gl_date_settings_signals[GL_DATE_SETTINGS_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlDateSettings, gl_date_settings, GTK_TYPE_WINDOW);



//static gboolean total_seconds_realise_freeze = FALSE;


double time_set_local_date_from_dmy (int D, int M, int Y)
{
	struct tm *timeinfo;
	time_t t = (time_t)market_db_time_now();
	timeinfo = localtime(&t);
	timeinfo->tm_sec  = 50;
	timeinfo->tm_min  = 59;
	timeinfo->tm_hour = 23;
	timeinfo->tm_mon  = M - 1;
	timeinfo->tm_mday = D;
	timeinfo->tm_year = Y - 1900;
	gdouble time = 0.0;
	time =(gdouble) mktime(timeinfo);

	return time;
}

static void
time_realize_in_total_seconds ( GlDateSettings* time )
{
	gdouble D  = gtk_spin_button_get_value(time->priv->day);
	gdouble M  = gtk_spin_button_get_value(time->priv->month);
	gdouble Y  = gtk_spin_button_get_value(time->priv->year);
	gdouble dt = time_set_local_date_from_dmy (D, M, Y);
	g_object_set                      (time, "total-seconds", dt,           NULL);
	//g_debug ("TOTAL SECONDS:%f", dt);
}




static void
date_realize_total_seconds(GlDateSettings* time)
{
	struct tm *timeinfo;
	time_t t = (glong)  time->priv->total_sec;
	timeinfo = localtime(&t);
//	g_debug("%d-%d-%d  %d:%d:%d",timeinfo->tm_mday,timeinfo->tm_mon,timeinfo->tm_year+1900,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
	gtk_spin_button_set_value(time->priv->day,((gdouble)timeinfo->tm_mday));
	gtk_spin_button_set_value(time->priv->month,((gdouble)timeinfo->tm_mon+1));    // tm_mon zählt von 0 bis 11. Sehen will man aber 1 bis 12.
	gtk_spin_button_set_value(time->priv->year,((gdouble)timeinfo->tm_year+1900));
	gdouble D  = gtk_spin_button_get_value(time->priv->day);
	gdouble M  = gtk_spin_button_get_value(time->priv->month);
	gdouble Y  = gtk_spin_button_get_value(time->priv->year);
	time->priv->total_sec = time_set_local_date_from_dmy (D, M, Y);
//	g_debug("NEW total seconds %f",time->priv->total_sec);
}

static void
date_settings_set_clicked_cb  ( GlDateSettings *time , GtkButton *button )
{
	//g_debug("%f - %f - %f",time->priv->h,time->priv->m,time->priv->s);
	time_realize_in_total_seconds(time);
	gtk_widget_hide(GTK_WIDGET(time));
}

static void
date_settings_cancel_cb ( GlDateSettings *time , GtkButton *button )
{
	gtk_widget_hide(GTK_WIDGET(time));
}


static void
gl_date_settings_init(GlDateSettings *time)
{
	g_return_if_fail (time != NULL);
	g_return_if_fail (GL_IS_DATE_SETTINGS(time));
	time->priv = gl_date_settings_get_instance_private (time);
	gtk_widget_init_template (GTK_WIDGET (time));

}


static gboolean
date_setting_dialog_move_callback ( gpointer user_data )
{
	GlDateSettings* dialog = GL_DATE_SETTINGS(user_data);
	gtk_window_move(GTK_WINDOW(dialog),1,1);
	dialog->priv->move_tag = 0;
	return FALSE;
}


static void
date_settings_dialog_start_visible (GObject *object ,GParamSpec *pspec , GlDateSettings *dialog)
{
	date_realize_total_seconds(dialog);
	if(dialog->priv->move_tag == 0)
		g_timeout_add(20,date_setting_dialog_move_callback,dialog);
}

static gdouble
get_days_in_month ( guint year, guint month )
{
	gboolean isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	switch(month)
	   {
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
		       return 31.0;
		       break;
		case 2:return isLeapYear?29.0:28.0;
		       break;
		case 4:
		case 6:
		case 9:
		case 11:
		       return 30.0;
		       break;

	default:
	printf("invalid Month number\nPlease try again ....\n");
	break;
	}
	return 32.0;
}

static void
date_day_new_realize_callbcak(GtkSpinButton *spinn, GlDateSettings *settings )
{
	gdouble D  = gtk_spin_button_get_value(settings->priv->day);
	gdouble M  = gtk_spin_button_get_value(settings->priv->month);
	gdouble Y  = gtk_spin_button_get_value(settings->priv->year);
	gdouble max_day = get_days_in_month(Y,M);
	gtk_adjustment_set_upper(settings->priv->days_adj,max_day);
	if(D>max_day)
	{
		gtk_spin_button_set_value(settings->priv->day,max_day);
	}
}

static void
gl_date_settings_constructed ( GObject *object )
{
	GlDateSettings* settings = GL_DATE_SETTINGS(object);
	g_signal_connect(settings,"notify::visible",G_CALLBACK(date_settings_dialog_start_visible),settings);
	g_signal_connect(settings->priv->month,"value-changed",G_CALLBACK(date_day_new_realize_callbcak),settings);
	g_signal_connect(settings->priv->year,"value-changed",G_CALLBACK(date_day_new_realize_callbcak),settings);
	gtk_label_set_text(settings->priv->cancel_name,_("CANCEL"));
	gtk_label_set_text(settings->priv->set_label,_("OK"));
	gtk_label_set_text(settings->priv->day_headline,_("Day"));
	gtk_label_set_text(settings->priv->month_headline,_("Month"));
	gtk_label_set_text(settings->priv->year_headline,_("Year"));


	if(G_OBJECT_CLASS (gl_date_settings_parent_class)->constructed)
		G_OBJECT_CLASS (gl_date_settings_parent_class)->constructed(object);
}

static void
gl_date_settings_finalize (GObject *object)
{
	//GlDateSettings* desktop = GL_DATE_SETTINGS(object);
	G_OBJECT_CLASS (gl_date_settings_parent_class)->finalize(object);
}



static void
gl_date_settings_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	////TEST:g_debug("Set (GL_MANAGER) property \n");
	g_return_if_fail (GL_IS_DATE_SETTINGS(object));
	GlDateSettings* time = GL_DATE_SETTINGS(object);
	switch (prop_id)
	{
	case GL_DATE_SETTINGS_VALUE_NAME:
		gtk_label_set_text(time->priv->value_name,g_value_get_string(value));
		break;
	case GL_DATE_SETTINGS_TOTAL_SECONDS:
		time->priv->total_sec = g_value_get_double(value);
		date_realize_total_seconds (time);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_date_settings_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_DATE_SETTINGS(object));
	GlDateSettings* time = GL_DATE_SETTINGS(object);
	switch (prop_id)
	{
	case GL_DATE_SETTINGS_VALUE_NAME:
		g_value_set_string(value,gtk_label_get_text(time->priv->value_name));
		break;
	case GL_DATE_SETTINGS_TOTAL_SECONDS:
		g_value_set_double(value,time->priv->total_sec);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_date_settings_class_init(GlDateSettingsClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	object_class -> finalize               =  gl_date_settings_finalize;
	object_class -> set_property           =  gl_date_settings_set_property;
	object_class -> get_property           =  gl_date_settings_get_property;
	object_class -> constructed            =  gl_date_settings_constructed;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/dialog/date_settings.ui");
	gtk_widget_class_bind_template_child_private (widget_class, GlDateSettings, value_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlDateSettings, cancel_name);
	gtk_widget_class_bind_template_child_private (widget_class, GlDateSettings, set_label);
	gtk_widget_class_bind_template_child_private (widget_class, GlDateSettings, day);
	gtk_widget_class_bind_template_child_private (widget_class, GlDateSettings, month);
	gtk_widget_class_bind_template_child_private (widget_class, GlDateSettings, year);
	gtk_widget_class_bind_template_child_private (widget_class, GlDateSettings, date_box);
	gtk_widget_class_bind_template_child_private (widget_class, GlDateSettings, month_box);
	gtk_widget_class_bind_template_child_private (widget_class, GlDateSettings, year_box);
	gtk_widget_class_bind_template_child_private (widget_class, GlDateSettings, days_adj);


	gtk_widget_class_bind_template_child_private (widget_class, GlDateSettings, day_headline);
	gtk_widget_class_bind_template_child_private (widget_class, GlDateSettings, month_headline);
	gtk_widget_class_bind_template_child_private (widget_class, GlDateSettings, year_headline);

	gtk_widget_class_bind_template_callback (widget_class, date_settings_set_clicked_cb);
	gtk_widget_class_bind_template_callback (widget_class, date_settings_cancel_cb);


	g_object_class_install_property (object_class,GL_DATE_SETTINGS_VALUE_NAME,
			g_param_spec_string ("value-name",
					"Time settings show seconds",
					"Time settings show seconds",
					"No value",
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));


	g_object_class_install_property (object_class,GL_DATE_SETTINGS_TOTAL_SECONDS,
			g_param_spec_double  ("total-seconds",
					"Time settings total seconds",
					"Time settings total seconds",
					0.0,G_MAXDOUBLE,0.0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));

}

gdouble
gl_date_settings_get_total_sec           ( GlDateSettings *settings )
{
	g_return_val_if_fail(settings!=NULL,0.0);
	return settings->priv->total_sec;
}

void
gl_date_settings_set_total_sec           ( GlDateSettings *settings , gdouble total_sec)
{
	g_return_if_fail(settings!=NULL);
	g_object_set(settings,"total-seconds",total_sec,NULL);
	date_realize_total_seconds(settings);
}

/** @} */

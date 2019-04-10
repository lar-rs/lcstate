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

#include "gl-date-time-window.h"
#include "gl-indicate.h"
#include "gl-connection.h"
#include "gl-action-widget.h"
#include "gl-translation.h"
#include <market-time.h>
#include "../lgdm-status.h"


struct _GlDateTimeWindowPrivate
{

	GtkWidget  *window;
	GtkWidget  *wfrom;
	GtkWidget  *wto;

	GtkWidget  *wlfrom;
	GtkWidget  *wlto;

	GtkWidget  *whscalefrom;
	GtkWidget  *whscaleto;




	gdouble     block_changed;
	gdouble     from_date;
	gdouble     to_date;
	gdouble     from_time;
	gdouble     to_time;
	gboolean    is_set;



};


#define GL_DATE_TIME_WINDOW_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_DATE_TIME_WINDOW, GlDateTimeWindowPrivate))

G_DEFINE_TYPE (GlDateTimeWindow, gl_date_time_window, MKT_TYPE_WINDOW);

enum
{
	GL_DATE_TIME_WINDOW_PROP0,
	GL_DATE_TIME_WINDOW_FROM_DATE,
	GL_DATE_TIME_WINDOW_TO_DATE,
	GL_DATE_TIME_WINDOW_FROM_TIME,
	GL_DATE_TIME_WINDOW_TO_TIME,

};

enum
{
	GL_DATE_TIME_CHANGED_DATE_TIME,
	GL_DATE_TIME_WINDOW_LAST_SIGNAL
};


static guint gl_date_time_window_signals[GL_DATE_TIME_WINDOW_LAST_SIGNAL] = { 0 };


void
gl_data_time_window_close_cb ( GtkWidget *widget , GlDateTimeWindow *dt )
{
	g_return_if_fail(dt != NULL);
	g_return_if_fail(MKT_IS_WINDOW(dt));
	mkt_window_hide ( MKT_WINDOW(dt));
	//g_debug("SET TIME .......................................");
	//g_debug("TEST From %s",market_db_get_date(dt->priv->from_date+dt->priv->from_time));
	//g_debug("TO %s",market_db_get_date( dt->priv->to_date+dt->priv->to_time));
	g_signal_emit(dt,gl_date_time_window_signals[GL_DATE_TIME_CHANGED_DATE_TIME],0);
}

void
gl_data_time_window_changed_time_from_cb ( GtkRange *range,GlDateTimeWindow *dt )
{
	//g_debug("gl_data_time_window_changed_time_from_cb");
	if(dt->priv->block_changed) return;
	g_return_if_fail(dt!=NULL);
	g_return_if_fail(GL_IS_DATE_TIME_WINDOW(dt));

	gdouble val = gtk_range_get_value(range);
	//g_debug("value : %f",val);
	dt->priv->from_time = val;
	if(dt->priv->from_time >= dt->priv->to_time)
	{
		GtkWidget *tscale = mkt_window_find_widget(MKT_WINDOW(dt),"GlDateTime_time_scale_to");
		if(tscale) gtk_range_set_value(GTK_RANGE(tscale),val+1);
	}

	//g_debug("TEST date %s",market_db_get_date_hms(dt->priv->from_time));
	GtkWidget *label = mkt_window_find_widget(MKT_WINDOW(dt),"GlDateTime_time_label_from");
	gtk_label_set_text(GTK_LABEL(label),market_db_get_date_hms(dt->priv->from_time));
	//g_debug("TEST From %s",market_db_get_date(dt->priv->from_date+dt->priv->from_time));
	//g_debug("TO %s",market_db_get_date( dt->priv->to_date+dt->priv->to_time));
	//g_debug("From: %f TO: %f",dt->priv->from_time,dt->priv->to_time);
	mktAPSet(dt,"from-time",dt->priv->from_time);
}
void
gl_data_time_window_changed_time_to_cb ( GtkRange *range,GlDateTimeWindow *dt )
{
	if(dt->priv->block_changed) return;
	g_return_if_fail(dt!=NULL);
	g_return_if_fail(GL_IS_DATE_TIME_WINDOW(dt));

	gdouble val = gtk_range_get_value(range);
	dt->priv->to_time = val;
	if(dt->priv->from_date+dt->priv->from_time >= dt->priv->to_date+dt->priv->to_time)
	{
		gtk_signal_handler_block_by_func(GTK_OBJECT(range), GTK_SIGNAL_FUNC(gl_data_time_window_changed_time_to_cb), dt);
		dt->priv->to_time = dt->priv->from_time + 10;
		gtk_range_set_value(GTK_RANGE(range),dt->priv->to_time);
		gtk_signal_handler_unblock_by_func(GTK_OBJECT(range), GTK_SIGNAL_FUNC(gl_data_time_window_changed_time_to_cb),dt);
	}
	GtkWidget *label = mkt_window_find_widget(MKT_WINDOW(dt),"GlDateTime_time_label_to");
	gtk_label_set_text(GTK_LABEL(label),market_db_get_date_hms(dt->priv->to_time));
	mktAPSet(dt,"to-time",dt->priv->to_time);
}

void
gl_data_time_window_changed_date_from_cb ( GtkCalendar *calendr ,GlDateTimeWindow *dt )
{
	if(dt->priv->block_changed) return;
	g_return_if_fail(dt!=NULL);
	g_return_if_fail(GL_IS_DATE_TIME_WINDOW(dt));
	guint d,m,y;
	gtk_calendar_get_date(GTK_CALENDAR(calendr),&y,&m,&d);
	dt->priv->from_date =market_db_date_from_dmy(d,m,y);
	if(dt->priv->from_date > dt->priv->to_date)
	{
		GtkWidget *to_cal = mkt_window_find_widget(MKT_WINDOW(dt),"GlDateTime_time_calender_to");
		if(to_cal)
		{
			//gtk_signal_handler_block_by_func(GTK_OBJECT(to_cal), GTK_SIGNAL_FUNC(gl_data_time_window_changed_date_to_cb), dt);
			gtk_calendar_select_month(GTK_CALENDAR(to_cal),m,y);
			gtk_calendar_select_day(GTK_CALENDAR(to_cal),d);

			//gtk_signal_handler_unblock_by_func(GTK_OBJECT(to_cal), GTK_SIGNAL_FUNC(gl_data_time_window_changed_date_to_cb),dt);
		}
	}
	dt->priv->is_set = TRUE;
	GtkWidget *fscale = mkt_window_find_widget(MKT_WINDOW(dt),"GlDateTime_time_scale_from");
	dt->priv->from_time = 1.0;
	if(fscale) gtk_range_set_value(GTK_RANGE(fscale),1.0);
	mktAPSet(dt,"from-date",dt->priv->from_date);

}

void
gl_data_time_window_changed_date_to_cb( GtkCalendar *calendr ,GlDateTimeWindow *dt )
{
	if(dt->priv->block_changed) return;
	g_return_if_fail(dt!=NULL);
	g_return_if_fail(GL_IS_DATE_TIME_WINDOW(dt));
	guint d,m,y;
	gtk_calendar_get_date(GTK_CALENDAR(calendr),&y,&m,&d);
	dt->priv->to_date =market_db_date_from_dmy(d,m,y);
	if(dt->priv->from_date>dt->priv->to_date)
	{
		dt->priv->to_date = dt->priv->from_date;
		GtkWidget *from_cal = mkt_window_find_widget(MKT_WINDOW(dt),"GlDateTime_time_calender_from");
		gtk_calendar_get_date(GTK_CALENDAR(from_cal),&y,&m,&d);
		gtk_signal_handler_block_by_func(GTK_OBJECT(calendr), GTK_SIGNAL_FUNC(gl_data_time_window_changed_date_to_cb), dt);
		gtk_calendar_select_month(GTK_CALENDAR(calendr),m,y);
		gtk_signal_handler_unblock_by_func(GTK_OBJECT(calendr), GTK_SIGNAL_FUNC(gl_data_time_window_changed_date_to_cb), dt);
		gtk_calendar_select_day(GTK_CALENDAR(calendr),d);
	}
	dt->priv->is_set = TRUE;
	GtkWidget *tscale = mkt_window_find_widget(MKT_WINDOW(dt),"GlDateTime_time_scale_to");
	dt->priv->to_time = 86390.;
	if(tscale) gtk_range_set_value(GTK_RANGE(tscale),dt->priv->to_time);
	mktAPSet(dt,"to-date",dt->priv->to_date);
}

static void
gl_date_time_window_init (GlDateTimeWindow *dt)
{
    GlDateTimeWindowPrivate *priv = GL_DATE_TIME_WINDOW_GET_PRIVATE(dt);
    dt->priv   = priv;

    dt->priv->from_date        = market_db_data_curr_day(market_db_time_now());
    dt->priv->to_date          = market_db_time_now();
    dt->priv->from_time        = 0.1;
    dt->priv->to_time          = 0.0;
    dt->priv->is_set           = FALSE;
    dt->priv->block_changed    = FALSE;
    mktAPSet(dt,"position-type",MKT_WINDOW_POS_TYPE_POPUP);
}

static void
gl_date_time_window_finalize (GObject *object)
{
	//GlDateTimeWindow *dt = GL_DATE_TIME_WINDOW(object);

	G_OBJECT_CLASS (gl_date_time_window_parent_class)->finalize (object);
}

void
gl_date_time_window_set_property(  GObject        *object,
		guint           prop_id,
		const GValue   *value,
		GParamSpec     *pspec)
{
	GlDateTimeWindow *dt = GL_DATE_TIME_WINDOW(object);
	switch(prop_id)
	{
	case GL_DATE_TIME_WINDOW_FROM_DATE:
		 dt->priv->from_date = g_value_get_double(value);
		 break;
	case GL_DATE_TIME_WINDOW_TO_DATE:
		dt->priv->to_date = g_value_get_double(value);
		break;
	case GL_DATE_TIME_WINDOW_FROM_TIME:
		dt->priv->from_time = g_value_get_double(value);
		break;
	case GL_DATE_TIME_WINDOW_TO_TIME:
		dt->priv->to_time = g_value_get_double(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}
void
gl_date_time_window_get_property(  GObject        *object,
		guint           prop_id,
		GValue         *value,
		GParamSpec     *pspec)
{
	GlDateTimeWindow *dt = GL_DATE_TIME_WINDOW(object);
	switch(prop_id)
	{
	case GL_DATE_TIME_WINDOW_FROM_DATE:
		 g_value_set_double(value,dt->priv->from_date);
		 break;
	case GL_DATE_TIME_WINDOW_TO_DATE:
		 g_value_set_double(value,dt->priv->to_date);
		break;
	case GL_DATE_TIME_WINDOW_FROM_TIME:
		g_value_set_double(value,dt->priv->from_date);
		break;
	case GL_DATE_TIME_WINDOW_TO_TIME:
		g_value_set_double(value,dt->priv->to_date);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}

static void
gl_date_time_window_class_init      ( GlDateTimeWindowClass *klass )
{
	GObjectClass*   object_class    = G_OBJECT_CLASS (klass);
	//MktWindowClass *win_class       = MKT_WINDOW_CLASS(klass);
	g_type_class_add_private (klass, sizeof (GlDateTimeWindowPrivate));
	object_class->finalize          = gl_date_time_window_finalize;
	object_class->set_property      = gl_date_time_window_set_property;
	object_class->get_property      = gl_date_time_window_get_property;

	klass ->changet_date_time       = NULL;

	g_object_class_install_property (object_class,GL_DATE_TIME_WINDOW_FROM_DATE,
			g_param_spec_double ("from-date",
					"Measurement measurement property",
					"Set get sensor measurement property",
					0.,G_MAXDOUBLE,0.,
					G_PARAM_READWRITE | MKT_PARAM_SAVE ));
	g_object_class_install_property (object_class,GL_DATE_TIME_WINDOW_TO_DATE,
			g_param_spec_double ("to-date",
					"Date time window to property",
					"Set get date time window to  property",
					0.,G_MAXDOUBLE,0.,
					G_PARAM_READWRITE | MKT_PARAM_SAVE ));
	g_object_class_install_property (object_class,GL_DATE_TIME_WINDOW_FROM_TIME,
			g_param_spec_double ("from-time",
					"Measurement measurement property",
					"Set get sensor measurement property",
					0.,G_MAXDOUBLE,0.,
					G_PARAM_READWRITE | MKT_PARAM_SAVE ));
	g_object_class_install_property (object_class,GL_DATE_TIME_WINDOW_TO_TIME,
			g_param_spec_double ("to-time",
					"Date time window to property",
					"Set get date time window to  property",
					0.,G_MAXDOUBLE,0.,
					G_PARAM_READWRITE | MKT_PARAM_SAVE ));


	gl_date_time_window_signals[GL_DATE_TIME_CHANGED_DATE_TIME] =
			g_signal_new ("changed-date-time",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET ( GlDateTimeWindowClass, changet_date_time),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

}

gdouble
gl_date_time_window_get_from        ( GlDateTimeWindow *dt )
{
	g_return_val_if_fail(dt!=NULL,0.0);
	g_return_val_if_fail(GL_IS_DATE_TIME_WINDOW(dt),0.0);

	return dt->priv->from_date+dt->priv->from_time;
}

gdouble
gl_date_time_window_get_to          ( GlDateTimeWindow *dt )
{
	g_return_val_if_fail(dt!=NULL,0.0);
	g_return_val_if_fail(GL_IS_DATE_TIME_WINDOW(dt),0.0);

	return dt->priv->to_date+dt->priv->to_time;
}

gboolean
gl_date_time_window_is_set          ( GlDateTimeWindow *dt )
{
	g_return_val_if_fail(dt!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_DATE_TIME_WINDOW(dt),FALSE);
	return dt->priv->is_set;
}

void
gl_date_time_window_clean          ( GlDateTimeWindow *dt )
{
	g_return_if_fail(dt!=NULL);
	g_return_if_fail(GL_IS_DATE_TIME_WINDOW(dt));
    dt->priv->is_set=FALSE;
    g_signal_emit(dt,gl_date_time_window_signals[GL_DATE_TIME_CHANGED_DATE_TIME],0);
}

void
gl_date_time_set_current            ( GlDateTimeWindow *dt )
{
	time_t t = (guint) dt->priv->from_date;
	if(t == 0)time(&t);
	struct tm *info  = localtime(&t);
	dt->priv->block_changed=TRUE;


	GtkWidget *from_cal = mkt_window_find_widget(MKT_WINDOW(dt),"GlDateTime_time_calender_from");
	if(from_cal)
	{
		gtk_calendar_select_month(GTK_CALENDAR(from_cal),info->tm_mon,info->tm_year+1900);
		gtk_calendar_select_day(GTK_CALENDAR(from_cal),info->tm_mday);
		GtkWidget *fscale = mkt_window_find_widget(MKT_WINDOW(dt),"GlDateTime_time_scale_from");
		if(fscale) gtk_range_set_value(GTK_RANGE(fscale),dt->priv->from_time);
	}
	t = (guint) dt->priv->to_date;
	if(t == 0)time(&t);
	info  = localtime(&t);
	GtkWidget *to_cal = mkt_window_find_widget(MKT_WINDOW(dt),"GlDateTime_time_calender_to");
	if(to_cal)
	{
		gtk_calendar_select_month(GTK_CALENDAR(to_cal),info->tm_mon,info->tm_year+1900);
		gtk_calendar_select_day(GTK_CALENDAR(to_cal),info->tm_mday);
		GtkWidget *tscale = mkt_window_find_widget(MKT_WINDOW(dt),"GlDateTime_time_scale_to");
		if(tscale) gtk_range_set_value(GTK_RANGE(tscale),dt->priv->to_time);
	}
	dt->priv->block_changed=FALSE;
	dt->priv->is_set = TRUE;
	g_signal_emit(dt,gl_date_time_window_signals[GL_DATE_TIME_CHANGED_DATE_TIME],0);

}


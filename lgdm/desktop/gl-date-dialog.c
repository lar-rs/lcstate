/*
 * @ingroup GlDateDialog
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

#include "gl-date-dialog.h"

#include <mktlib.h>
#include <string.h>

#include "../config.h"
#include <glib/gi18n-lib.h>

// static GlDateDialog *__gui_process_desktop = NULL;

struct _GlDateDialogPrivate {

    GtkSpinButton *day;
    GtkSpinButton *month;
    GtkSpinButton *year;
    GtkSpinButton *hours;
    GtkSpinButton *minuts;
    GtkSpinButton *seconds;

    GtkBox *  date_box;
    GtkBox *  day_box;
    GtkBox *  month_box;
    GtkBox *  year_box;
    GtkBox *  time_box;
    GtkBox *  hours_box;
    GtkBox *  minutes_box;
    GtkBox *  seconds_box;
    GtkLabel *value_name;
    gdouble   total_sec;

    GtkLabel *day_label;
    GtkLabel *month_label;
    GtkLabel *year_label;
    GtkLabel *hours_label;
    GtkLabel *minutes_label;
    GtkLabel *seconds_label;

    gboolean activate_sec;
    gboolean activate_time;
};

enum {
    GL_DATE_DIALOG_PROP_NULL,
    GL_DATE_DIALOG_VALUE_NAME,
    GL_DATE_DIALOG_ACTIVATE_TIME,
    GL_DATE_DIALOG_ACTIVATE_SECONDS,

    GL_DATE_DIALOG_HOUR,
    GL_DATE_DIALOG_MINUTES,
    GL_DATE_DIALOG_SECONDS,
    GL_DATE_DIALOG_YEAR,
    GL_DATE_DIALOG_DAY,
    GL_DATE_DIALOG_MOUNTH,

    GL_DATE_DIALOG_TOTAL_SECONDS

};

enum { GL_DATE_DIALOG_LAST_SIGNAL };

// static guint gl_date_dialog_signals[GL_DATE_DIALOG_LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(GlDateDialog, gl_date_dialog, GTK_TYPE_LIST_BOX);

// static gboolean total_seconds_realise_freeze = FALSE;

static void time_wraped_seconds(GlDateDialog *time, GtkSpinButton *button) {
    if (gtk_spin_button_get_value(button) < 0.1) {
        gtk_spin_button_set_value(time->priv->minuts, gtk_spin_button_get_value(time->priv->minuts) + 1);
    } else {
        gtk_spin_button_set_value(time->priv->minuts, gtk_spin_button_get_value(time->priv->minuts) - 1);
    }
}

static void time_wraped_minutes(GlDateDialog *time, GtkSpinButton *button) {
    if (gtk_spin_button_get_value(button) < 0.1) {
        gtk_spin_button_set_value(time->priv->hours, gtk_spin_button_get_value(time->priv->hours) + 1);
    } else {
        gtk_spin_button_set_value(time->priv->hours, gtk_spin_button_get_value(time->priv->hours) - 1);
    }
}

/*
static void
time_realize_hours(GlDateDialog* time)
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
time_realize_minutes(GlDateDialog* time)
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
*/
double time_set_local_date_from_dmy_hms(int D, int M, int Y, int h, int m, int s) {
    struct tm *timeinfo;
    time_t     t      = (time_t)market_db_time_now();
    timeinfo          = localtime(&t);
    timeinfo->tm_sec  = s;
    timeinfo->tm_min  = m;
    timeinfo->tm_hour = h;
    timeinfo->tm_mon  = M - 1;
    timeinfo->tm_mday = D;
    timeinfo->tm_year = Y - 1900;
    gdouble time      = 0.0;
    time              = (gdouble)mktime(timeinfo);

    return time;
}

static void time_realize_in_total_seconds(GlDateDialog *time) {
    gdouble D = gtk_spin_button_get_value(time->priv->day);
    gdouble M = gtk_spin_button_get_value(time->priv->month);
    gdouble Y = gtk_spin_button_get_value(time->priv->year);

    gdouble h = gtk_spin_button_get_value(time->priv->hours);
    gdouble m = gtk_spin_button_get_value(time->priv->minuts);
    gdouble s = gtk_spin_button_get_value(time->priv->seconds);

    // g_debug("%d-%d-%d  %d:%d:%d",D,M,Y,h,m,s);

    gdouble dt = time_set_local_date_from_dmy_hms(D, M, Y, h, m, s);

    g_signal_handlers_block_by_func(time, time_realize_in_total_seconds, NULL);
    g_object_set(time, "total-seconds", dt, NULL);
    g_signal_handlers_unblock_by_func(time, time_realize_in_total_seconds, NULL);

    // g_debug ("TOTAL SECONDS:%f", dt);
}

static gboolean _BLOCK_HANDLER_1 = FALSE;

static void time_realize_total_seconds(GlDateDialog *time) {
    struct tm *timeinfo;
    time_t     t = (glong)time->priv->total_sec;
    timeinfo     = localtime(&t);
    //	g_debug("%d-%d-%d  %d:%d:%d",timeinfo->tm_mday,timeinfo->tm_mon,timeinfo->tm_year+1900,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
    _BLOCK_HANDLER_1 = TRUE;
    gtk_spin_button_set_value(time->priv->day, ((gdouble)timeinfo->tm_mday));
    gtk_spin_button_set_value(time->priv->month, ((gdouble)timeinfo->tm_mon + 1)); // tm_mon zählt von 0 bis 11. Sehen will man aber 1 bis 12.
    gtk_spin_button_set_value(time->priv->year, ((gdouble)timeinfo->tm_year + 1900));

    gtk_spin_button_set_value(time->priv->hours, ((gdouble)timeinfo->tm_hour));
    gtk_spin_button_set_value(time->priv->minuts, ((gdouble)timeinfo->tm_min));
    gtk_spin_button_set_value(time->priv->seconds, ((gdouble)timeinfo->tm_sec));
    _BLOCK_HANDLER_1 = FALSE;
}

static void gl_date_dialog_init(GlDateDialog *time) {
    g_return_if_fail(time != NULL);
    g_return_if_fail(GL_IS_DATE_DIALOG(time));
    time->priv = gl_date_dialog_get_instance_private(time);
    gtk_widget_init_template(GTK_WIDGET(time));
}

static void gl_date_dialog_constructed(GObject *object) {
    GlDateDialog *dialog = GL_DATE_DIALOG(object);
    g_object_bind_property(dialog, "activate-seconds", dialog->priv->seconds_box, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);
    g_object_bind_property(dialog, "activate-time", dialog->priv->time_box, "visible", G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE);

    gtk_label_set_text(dialog->priv->day_label, _("Day"));
    gtk_label_set_text(dialog->priv->month_label, _("Month"));
    gtk_label_set_text(dialog->priv->year_label, _("Year"));
    gtk_label_set_text(dialog->priv->hours_label, _("Hours"));
    gtk_label_set_text(dialog->priv->minutes_label, _("Minutes"));
    gtk_label_set_text(dialog->priv->seconds_label, _("Seconds"));
    //
    if (G_OBJECT_CLASS(gl_date_dialog_parent_class)->constructed) G_OBJECT_CLASS(gl_date_dialog_parent_class)->constructed(object);
}

static void gl_date_dialog_finalize(GObject *object) {
    // GlDateDialog* desktop = GL_DATE_DIALOG(object);
    G_OBJECT_CLASS(gl_date_dialog_parent_class)->finalize(object);
}

static void gl_date_dialog_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    ////TEST:g_debug("Set (GL_MANAGER) property \n");
    g_return_if_fail(GL_IS_DATE_DIALOG(object));
    GlDateDialog *time = GL_DATE_DIALOG(object);
    switch (prop_id) {
    case GL_DATE_DIALOG_VALUE_NAME:
        gtk_label_set_text(time->priv->value_name, g_value_get_string(value));
        break;
    case GL_DATE_DIALOG_ACTIVATE_SECONDS:
        time->priv->activate_sec = g_value_get_boolean(value);
        break;
    case GL_DATE_DIALOG_ACTIVATE_TIME:
        time->priv->activate_time = g_value_get_boolean(value);
        break;
    case GL_DATE_DIALOG_TOTAL_SECONDS:
        time->priv->total_sec = g_value_get_double(value);
        // time_realize_total_seconds (time);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gl_date_dialog_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(GL_IS_DATE_DIALOG(object));
    GlDateDialog *time = GL_DATE_DIALOG(object);
    switch (prop_id) {
    case GL_DATE_DIALOG_VALUE_NAME:
        g_value_set_string(value, gtk_label_get_text(time->priv->value_name));
        break;
    case GL_DATE_DIALOG_ACTIVATE_SECONDS:
        g_value_set_boolean(value, time->priv->activate_sec);
        break;
    case GL_DATE_DIALOG_ACTIVATE_TIME:
        g_value_set_boolean(value, time->priv->activate_time);
        break;
    case GL_DATE_DIALOG_TOTAL_SECONDS:
        g_value_set_double(value, time->priv->total_sec);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void gl_date_dialog_class_init(GlDateDialogClass *klass) {
    GObjectClass *  object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    object_class->finalize       = gl_date_dialog_finalize;
    object_class->set_property   = gl_date_dialog_set_property;
    object_class->get_property   = gl_date_dialog_get_property;
    object_class->constructed    = gl_date_dialog_constructed;

    gtk_widget_class_set_template_from_resource(widget_class, "/lgdm/ui/dialog/date_dialog.ui");
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, value_name);

    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, day);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, month);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, year);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, seconds);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, date_box);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, month_box);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, year_box);

    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, time_box);

    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, hours);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, minuts);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, seconds);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, hours_box);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, minutes_box);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, seconds_box);

    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, day_label);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, month_label);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, year_label);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, hours_label);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, minutes_label);
    gtk_widget_class_bind_template_child_private(widget_class, GlDateDialog, seconds_label);

    gtk_widget_class_bind_template_callback(widget_class, time_wraped_minutes);
    gtk_widget_class_bind_template_callback(widget_class, time_wraped_seconds);

    g_object_class_install_property(
        object_class, GL_DATE_DIALOG_VALUE_NAME,
        g_param_spec_string("value-name", "Time settings show seconds", "Time settings show seconds", "No value", G_PARAM_WRITABLE | G_PARAM_READABLE));
    g_object_class_install_property(
        object_class, GL_DATE_DIALOG_ACTIVATE_SECONDS,
        g_param_spec_boolean("activate-seconds", "Time settings show seconds", "Time settings show seconds", TRUE, G_PARAM_WRITABLE | G_PARAM_READABLE));

    g_object_class_install_property(object_class, GL_DATE_DIALOG_ACTIVATE_TIME,
                                    g_param_spec_boolean("activate-time", "Time settings show seconds", "Time settings show seconds", TRUE,
                                                         G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property(object_class, GL_DATE_DIALOG_TOTAL_SECONDS,
                                    g_param_spec_double("total-seconds", "Time settings total seconds", "Time settings total seconds", 0.0, G_MAXDOUBLE, 0.0,
                                                        G_PARAM_WRITABLE | G_PARAM_READABLE));
}

gdouble gl_date_dialog_get_total_sec(GlDateDialog *dialog) {
    time_realize_in_total_seconds(dialog);
    return dialog->priv->total_sec;
}

void gl_date_dialog_set_total_sec(GlDateDialog *dialog, gdouble total_sec) {
    g_object_set(dialog, "total-seconds", total_sec, NULL);
    time_realize_total_seconds(dialog);
}

/** @} */

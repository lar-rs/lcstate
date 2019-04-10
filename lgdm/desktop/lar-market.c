/*
 * @ingroup LarMarket
 * @{
 * @file  lar-desktop-place.c	generated object file
 * @brief generated object file 
 *
 *
 *  Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */



#include "lgdm-desktop-place.h"
#include "lar-market.h"

#include "../config.h"
#include <glib/gi18n-lib.h>

#include <mktlib.h>






//static LarMarket *__gui_process_desktop = NULL;

struct _LarMarketPrivate
{
	GtkRevealer             *log_revealer;
	GtkTreeView             *log_tree;
	GtkRevealer             *logbook_settings_revealer;
	GtkRevealer             *errors_settings_revealer;
	GtkListBoxRow           *message_type;
	GtkListBoxRow           *show_all_errors_row;
	GtkSwitch               *show_all_error_switch;
	GtkLabel                *message_type_name;


	GtkListBoxRow           *time_interval;

	//GtkRevealer             *logbook_settings_revealer;
	GtkRevealer             *error_revealer;
	GtkTreeView             *error_tree;

	gboolean                 log_activated;
	gint                     message_type_value;
	guint                    offset;
	GCancellable            *log_cancel;
	GCancellable            *error_cancel;
	GEnumClass              *message_enum;


	GtkBox                  *auto_update_box;
	GtkBox                  *intervall_box;


	GtkLabel                *interval_from_value;
	GtkLabel                *interval_to_value;


	gulong                   parent_signal;
	guint                    update_tag;
	gdouble                  last_changed;

	gboolean                 is_on_map;
};


enum {
	LAR_MARKET_PROP_NULL,
	LAR_MARKET_PROP_LOG_ACTIVATED,

};


enum
{
	LAR_MARKET_LAST_SIGNAL
};


//static guint lar_market_signals[LAR_MARKET_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (LarMarket, lar_market, GTK_TYPE_BOX);


// -------------------------------------- Bitte hier alle Error Verwaltungsfunktionen ---------------------------------------------


static void
lar_market_init(LarMarket *lar_market)
{
	g_return_if_fail (lar_market != NULL);
	g_return_if_fail (LAR_IS_MARKET(lar_market));
	lar_market->priv = lar_market_get_instance_private (lar_market);
	lar_market->priv->log_cancel       = g_cancellable_new();
	lar_market->priv->error_cancel     = NULL;
	lar_market->priv->update_tag   = 0;
	lar_market->priv->message_type_value =  -1;
	lar_market->priv->message_enum = g_type_class_ref (MKT_TYPE_LOG_MESSAGE_TYPE);
	gtk_widget_init_template (GTK_WIDGET (lar_market));
}


static void
lar_market_constructed (GObject *object)
{
	LarMarket* logbook = LAR_MARKET(object);

	g_object_bind_property(logbook ,"log-activated",logbook->priv->log_revealer,"visible",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
	g_object_bind_property(logbook ,"log-activated",logbook->priv->error_revealer,"visible",G_BINDING_INVERT_BOOLEAN|G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
	g_object_bind_property(logbook ,"log-activated",logbook->priv->logbook_settings_revealer,"visible",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
	g_object_bind_property(logbook ,"log-activated",logbook->priv->errors_settings_revealer,"visible",G_BINDING_INVERT_BOOLEAN|G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);

	G_OBJECT_CLASS (lar_market_parent_class)->constructed(object);
}


static void
lar_market_finalize (GObject *object)
{
	LarMarket* lar_market = LAR_MARKET(object);
	g_type_class_unref (lar_market->priv->message_enum);
	G_OBJECT_CLASS (lar_market_parent_class)->finalize(object);
}





static void
lar_market_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (LAR_IS_MARKET(object));
	LarMarket* lar_market = LAR_MARKET(object);
	switch (prop_id)
	{
	case LAR_MARKET_PROP_LOG_ACTIVATED:
		lar_market->priv->log_activated = g_value_get_boolean(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
lar_market_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (LAR_IS_MARKET(object));
	LarMarket* lar_market = LAR_MARKET(object);
	switch (prop_id)
	{
	case LAR_MARKET_PROP_LOG_ACTIVATED:
		g_value_set_boolean(value,lar_market->priv->log_activated);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}



static void
lar_market_class_init(LarMarketClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	//LarLayoutClass        *layout_class     =  LAR_LAYOUT_CLASS (klass);
	object_class -> finalize               =  lar_market_finalize;
	object_class -> set_property           =  lar_market_set_property;
	object_class -> get_property           =  lar_market_get_property;
	object_class -> constructed            =  lar_market_constructed;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/layout/logbook-layout.ui");




	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, log_revealer);
	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, log_tree);
	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, message_type);
	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, time_interval);
	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, logbook_settings_revealer);
	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, errors_settings_revealer);
	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, error_revealer);
	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, error_tree);
	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, message_type_name);

	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, show_all_errors_row);
	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, show_all_error_switch);

	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, auto_update_box);
	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, intervall_box);
	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, interval_from_value);
	gtk_widget_class_bind_template_child_private (widget_class, LarMarket, interval_to_value);


	g_object_class_install_property (object_class,LAR_MARKET_PROP_LOG_ACTIVATED,
			g_param_spec_boolean ("log-activated",
					"Log is activated",
					"Desktop level",
					TRUE,
					G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT ));
}



/** @} */

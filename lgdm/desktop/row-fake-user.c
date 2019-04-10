/*
 * @ingroup RowFakeUser
 * @{
 * @file  row-channel-info.c	generated object file
 * @brief generated object file 
 *
 *
 *  Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */



#include <largdm.h>
#include <mktlib.h>
#include <mktbus.h>

#include "../config.h"
#include <glib/gi18n-lib.h>

#include "row-fake-user.h"




//static RowFakeUser *__gui_process_desktop = NULL;

struct _RowFakeUserPrivate
{
	UsersObject         *users_object;

	GtkLabel            *fake_user_name;
	GtkLabel            *fake_user_level;
	GlStringDialog      *dialog;

};

enum {
	ROW_FAKE_USER_PROP_NULL,
	ROW_FAKE_USER_OBJECT,
	ROW_FAKE_USER_NAME,
	ROW_FAKE_USER_LEVEL,
};


enum
{
	ROW_FAKE_USER_LOGIN,
	ROW_FAKE_USER_LAST_SIGNAL
};


static guint row_fake_users_signals[ROW_FAKE_USER_LAST_SIGNAL] = { 0 };



G_DEFINE_TYPE_WITH_PRIVATE (RowFakeUser, row_fake_user, GTK_TYPE_LIST_BOX_ROW);



static void
login_button_clicked_cb ( RowFakeUser *row_fake_user, GtkButton *button )
{
	row_fake_user_login(row_fake_user);
}

static void
row_fake_user_init(RowFakeUser *row_fake_user)
{
	g_return_if_fail (row_fake_user != NULL);
	g_return_if_fail (ROW_IS_FAKE_USER(row_fake_user));
	row_fake_user->priv               = row_fake_user_get_instance_private (row_fake_user);
	row_fake_user->priv->users_object = NULL;
	gtk_widget_init_template (GTK_WIDGET (row_fake_user));
}

static void
create_fake_level (RowFakeUser* row_fake_user, guint level )
{
	gchar *internal_value = g_strdup_printf(_("LEVEL %d"), level);
	gtk_label_set_text(row_fake_user->priv->fake_user_level,internal_value);
	g_free(internal_value);
}

static void
row_fake_user_constructed ( GObject *object )
{
	//RowFakeUser* row_fake_user = ROW_FAKE_USER(object);
	if(G_OBJECT_CLASS (row_fake_user_parent_class)->constructed)
		G_OBJECT_CLASS (row_fake_user_parent_class)->constructed(object);
}

static void
row_fake_user_finalize (GObject *object)
{
	RowFakeUser* row_fake_user = ROW_FAKE_USER(object);
	if(row_fake_user->priv->users_object) g_object_unref(row_fake_user->priv->users_object);
	G_OBJECT_CLASS (row_fake_user_parent_class)->finalize(object);
}

static void
row_fake_user_set_property (GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec)
{
	g_return_if_fail (ROW_IS_FAKE_USER(object));
	RowFakeUser* row_fake_user = ROW_FAKE_USER(object);
	switch (prop_id)
	{
	case ROW_FAKE_USER_OBJECT:
		if(row_fake_user->priv->users_object)g_free(row_fake_user->priv->users_object);
		row_fake_user->priv->users_object = g_value_dup_object(value);
		break;
	case ROW_FAKE_USER_NAME:
		gtk_label_set_text(row_fake_user->priv->fake_user_name,g_value_get_string(value));
		break;
	case ROW_FAKE_USER_LEVEL:
		create_fake_level(row_fake_user,g_value_get_uint(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
row_fake_user_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (ROW_IS_FAKE_USER(object));
	RowFakeUser* row_fake_user = ROW_FAKE_USER(object);
	switch (prop_id)
	{
	case ROW_FAKE_USER_OBJECT:
		g_value_set_object(value,row_fake_user->priv->users_object);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
row_fake_user_class_init(RowFakeUserClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	object_class -> finalize               =  row_fake_user_finalize;
	object_class -> set_property           =  row_fake_user_set_property;
	object_class -> get_property           =  row_fake_user_get_property;
	object_class -> constructed            =  row_fake_user_constructed;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/row/fake-user-row.ui");

	gtk_widget_class_bind_template_child_private (widget_class, RowFakeUser, fake_user_name);
	gtk_widget_class_bind_template_child_private (widget_class, RowFakeUser, fake_user_level);

	gtk_widget_class_bind_template_callback (widget_class, login_button_clicked_cb);

	g_object_class_install_property (object_class,ROW_FAKE_USER_OBJECT,
				g_param_spec_object ("fake-user",
						"FakeUser DBus id name",
						"FakeUser DBus id name",
						USERS_TYPE_OBJECT,
						G_PARAM_WRITABLE | G_PARAM_READABLE |  G_PARAM_CONSTRUCT_ONLY ));

	g_object_class_install_property (object_class,ROW_FAKE_USER_NAME,
			g_param_spec_string ("fake-user-name",
					"FakeUser DBus id name",
					"FakeUser DBus id name",
					"Expert level",
					G_PARAM_WRITABLE | G_PARAM_READABLE |  G_PARAM_CONSTRUCT_ONLY ));
	g_object_class_install_property (object_class,ROW_FAKE_USER_LEVEL,
			g_param_spec_uint ("fake-user-level",
					"FakeUser DBus id name",
					"FakeUser DBus id name",
					1,5,1,
					G_PARAM_WRITABLE | G_PARAM_READABLE |  G_PARAM_CONSTRUCT_ONLY ));

	row_fake_users_signals[ROW_FAKE_USER_LOGIN] =
				g_signal_new ("login",
						G_TYPE_FROM_CLASS (klass),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						0,
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

	/*

	gtk_widget_class_bind_template_callback (widget_class, example_signal_callback);*/
}


gboolean row_fake_user_login (RowFakeUser* row)
{
	g_return_val_if_fail (row,                   FALSE);
	g_return_val_if_fail (ROW_IS_FAKE_USER(row), FALSE);
	gtk_widget_hide      (GTK_WIDGET(row->priv->dialog));

//	if (users_user_get_auto_login(users_object_get_user(row->priv->users_object)))
//	{
//		gboolean result = FALSE;
//		users_user_call_log_in_sync(users_object_get_user(row->priv->users_object),"no-need",&result,NULL,NULL);
//	}
/*	else*/ if(users_user_get_level(users_object_get_user(row->priv->users_object)) <= security_device_get_level(TERA_GUARD()) )
	{
		gboolean result = FALSE;
		users_user_call_log_in_sync(users_object_get_user(row->priv->users_object),"no-need",&result,NULL,NULL);

	}
	return FALSE;
}


/** @} */

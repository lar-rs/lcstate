/*
 * @ingroup GlSelectDevice
 * @{
 * @file  gl-desktop-place.c	generated object file
 * @brief generated object file
 *
 *
 *  Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */



#include "gl-select-device.h"

#include "../config.h"
#include <glib/gi18n-lib.h>

#include <mktlib.h>
#include <mktbus.h>






//static GlSelectDevice *__gui_process_desktop = NULL;

struct _GlSelectDevicePrivate
{

	GtkRevealer             *select_device;
	GtkRevealer             *install_device;
	GtkListBoxRow           *waiting_for_row;
	GtkListBoxRow           *ultra_device_install;
	GtkSpinner              *install_spinner;
	GtkLabel                *errors_label;
	GtkLabel                *status_label;
	GtkButton               *reboot_button;

	LarpcApt                *apt;
	GCancellable            *cancel;
};


enum {
	GL_SELECT_DEVICE_PROP_NULL,

};


enum
{
	GL_SELECT_DEVICE_LAST_SIGNAL
};


//static guint gl_select_device_signals[GL_SELECT_DEVICE_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlSelectDevice, gl_select_device, GTK_TYPE_WINDOW);




static void
device_select_start_intall_package ( GlSelectDevice *select_device, const gchar *name )
{
	larpc_apt_call_install(select_device->priv->apt,name,select_device->priv->cancel,NULL,NULL);
	gtk_widget_hide(GTK_WIDGET(select_device->priv->select_device));
	gtk_widget_show(GTK_WIDGET(select_device->priv->install_device));
	gtk_widget_show(GTK_WIDGET(select_device->priv->reboot_button));
}

static void
device_select_row_activated_cb ( GlSelectDevice *gl_select_device , GtkListBoxRow *row , GtkListBox *list_box )
{
	if((gpointer)row == (gpointer)gl_select_device->priv->ultra_device_install)
	{
		device_select_start_intall_package(gl_select_device,"ultracontrol");
	}
}
static void
reboot_button_clicked_cb ( GlSelectDevice *select_device , GtkButton *button )
{

	if(mkt_pc_manager_client_get_device())
		larpc_device_call_reboot(mkt_pc_manager_client_get_device(),NULL,NULL,NULL);
}

/*
static void
device_select_change_apt_status_callback( LarpcApt *apt, GParamSpec *pspec, GlSelectDevice* select_device )
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(select_device->priv->install_log);
	gtk_text_buffer_set_text(buffer,larpc_apt_get_status(apt),strlen(larpc_apt_get_status(apt)));
}

static void
device_select_change_apt_error_msg_callback (  LarpcApt *apt, GParamSpec *pspec, GlSelectDevice* select_device )
{
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(select_device->priv->install_log);
	gtk_text_buffer_set_text(buffer,larpc_apt_get_status(apt),strlen(larpc_apt_get_error_msg(apt)));

}*/

static void
gl_select_device_init(GlSelectDevice *gl_select_device)
{
	g_return_if_fail (gl_select_device != NULL);
	g_return_if_fail (GL_IS_SELECT_DEVICE(gl_select_device));
	gl_select_device->priv = gl_select_device_get_instance_private (gl_select_device);
	gl_select_device->priv->cancel       = g_cancellable_new();
	gtk_widget_init_template (GTK_WIDGET (gl_select_device));
}

static void
gl_select_device_constructed (GObject *object)
{
	GlSelectDevice* select_device = GL_SELECT_DEVICE(object);
	TeraClientObject *pc_lient = tera_client_lookup("com.lar.service.device");
	if(pc_lient)
	{
		select_device->priv->apt =	mkt_pc_manager_client_get_apt();
		if(select_device->priv->apt)
		{
			g_object_bind_property(select_device->priv->apt,"busy",select_device->priv->waiting_for_row,"visible",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
			g_object_bind_property(select_device->priv->apt,"busy",select_device->priv->ultra_device_install,"visible",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE|G_BINDING_INVERT_BOOLEAN);
			g_object_bind_property(select_device->priv->apt,"busy",select_device->priv->install_spinner,"visible",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
			g_object_bind_property(select_device->priv->apt,"busy",select_device->priv->install_spinner,"active",G_BINDING_DEFAULT);
			g_object_bind_property(select_device->priv->apt,"busy",select_device->priv->reboot_button,"sensitive",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE|G_BINDING_INVERT_BOOLEAN);

			g_object_bind_property(select_device->priv->apt,"status",select_device->priv->status_label,"label",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
			g_object_bind_property(select_device->priv->apt,"error-msg",select_device->priv->errors_label,"label",G_BINDING_DEFAULT|G_BINDING_SYNC_CREATE);
		}

	}

	G_OBJECT_CLASS (gl_select_device_parent_class)->constructed(object);
}


static void
gl_select_device_finalize (GObject *object)
{
	GlSelectDevice* gl_select_device = GL_SELECT_DEVICE(object);
	g_object_unref(gl_select_device->priv->cancel);
	G_OBJECT_CLASS (gl_select_device_parent_class)->finalize(object);
}





static void
gl_select_device_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_SELECT_DEVICE(object));
	//GlSelectDevice* gl_select_device = GL_SELECT_DEVICE(object);
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_select_device_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_SELECT_DEVICE(object));
	//GlSelectDevice* gl_select_device = GL_SELECT_DEVICE(object);
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}



static void
gl_select_device_class_init(GlSelectDeviceClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	//GlLayoutClass        *layout_class     =  GL_LAYOUT_CLASS (klass);
	object_class -> finalize               =  gl_select_device_finalize;
	object_class -> set_property           =  gl_select_device_set_property;
	object_class -> get_property           =  gl_select_device_get_property;
	object_class -> constructed            =  gl_select_device_constructed;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/layout/select-device.ui");


	gtk_widget_class_bind_template_child_private (widget_class, GlSelectDevice, select_device);
	gtk_widget_class_bind_template_child_private (widget_class, GlSelectDevice, ultra_device_install);
	gtk_widget_class_bind_template_child_private (widget_class, GlSelectDevice, waiting_for_row);
	gtk_widget_class_bind_template_child_private (widget_class, GlSelectDevice, install_spinner);
	gtk_widget_class_bind_template_child_private (widget_class, GlSelectDevice, install_device);
	gtk_widget_class_bind_template_child_private (widget_class, GlSelectDevice, errors_label);
	gtk_widget_class_bind_template_child_private (widget_class, GlSelectDevice, status_label);
	gtk_widget_class_bind_template_child_private (widget_class, GlSelectDevice, reboot_button);


	gtk_widget_class_bind_template_callback (widget_class, device_select_row_activated_cb);
	gtk_widget_class_bind_template_callback (widget_class, reboot_button_clicked_cb);




}



/** @} */

/*
 * @ingroup GlSpinButton
 * @{
 * @file  gl-spin-button.c	generated object file
 * @brief generated object file 
 *
 *
 *  Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */



#include "gl-spin-button.h"

#define GETTEXT_PACKAGE "GlSpinButton"
#include <glib/gi18n-lib.h>



//static GlSpinButton *__gui_process_desktop = NULL;

struct _GlSpinButtonPrivate
{


};


enum {
	GL_SPIN_BUTTON_PROP_NULL,

};


enum
{
	GL_SPIN_BUTTON_LAST_SIGNAL
};


//static guint gl_spin_button_signals[GL_SPIN_BUTTON_LAST_SIGNAL] = { 0 };




G_DEFINE_TYPE_WITH_PRIVATE (GlSpinButton, gl_spin_button, GTK_TYPE_BOX);






static void
gl_spin_button_init(GlSpinButton *gl_spin_button)
{
	g_return_if_fail (gl_spin_button != NULL);
	g_return_if_fail (GL_IS_SPIN_BUTTON(gl_spin_button));
	gl_spin_button->priv = gl_spin_button_get_instance_private (gl_spin_button);
	gtk_widget_init_template (GTK_WIDGET (gl_spin_button));


}



static void
gl_spin_button_finalize (GObject *object)
{
	//GlSpinButton* gl_spin_button = GL_SPIN_BUTTON(object);
	G_OBJECT_CLASS (gl_spin_button_parent_class)->finalize(object);
}





static void
gl_spin_button_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_SPIN_BUTTON(object));
	//GlSpinButton* gl_spin_button = GL_SPIN_BUTTON(object);
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_spin_button_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_SPIN_BUTTON(object));
	//GlSpinButton* gl_spin_button = GL_SPIN_BUTTON(object);
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}


static void
gl_spin_button_class_init(GlSpinButtonClass *klass)
{
	GObjectClass         *object_class     =  G_OBJECT_CLASS (klass);
	GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
	object_class -> finalize               =  gl_spin_button_finalize;
	object_class -> set_property           =  gl_spin_button_set_property;
	object_class -> get_property           =  gl_spin_button_get_property;

	gtk_widget_class_set_template_from_resource  (widget_class, "/lgdm/ui/layout/spinner-dialog.ui");



	/*
	gtk_widget_class_bind_template_child_private (widget_class, GlSpinButton, example_object);
	gtk_widget_class_bind_template_callback (widget_class, spinner_dialog_change_value);*/


}

/** @} */

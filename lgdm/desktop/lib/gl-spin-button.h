/**
 * @defgroup LgdmLibrary
 * @defgroup GlSpinButton
 * @ingroup  GlSpinButton
 * @{
 * @file  gl-spin-button.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_SPIN_BUTTON_H_
#define GL_SPIN_BUTTON_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_SPIN_BUTTON    			           (gl_spin_button_get_type())
#define GL_SPIN_BUTTON(obj)			       (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_SPIN_BUTTON, GlSpinButton))
#define GL_SPIN_BUTTON_CLASS(klass)		   (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_SPIN_BUTTON, GlSpinButtonClass))
#define GL_IS_SPIN_BUTTON(obj)		               (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_SPIN_BUTTON))
#define GL_IS_SPIN_BUTTON_CLASS(klass)              (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_SPIN_BUTTON))

typedef struct _GlSpinButton			        GlSpinButton;
typedef struct _GlSpinButtonClass		        GlSpinButtonClass;
typedef struct _GlSpinButtonPrivate           GlSpinButtonPrivate;

struct _GlSpinButtonClass
{
	GtkBoxClass                           parent_class;
};

struct _GlSpinButton
{
	GtkBox                                parent;
	GlSpinButtonPrivate                       *priv;
};


GType 		         gl_spin_button_get_type                ( void );



G_END_DECLS
#endif /* GL_SPIN_BUTTON_H_ */

/** @} */

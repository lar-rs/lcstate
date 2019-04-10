/**
 * @defgroup LgdmLibrary
 * @defgroup GlBooleanDialog
 * @ingroup  GlBooleanDialog
 * @{
 * @file  gl-boolean-dialog.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_BOOLEAN_DIALOG_H_
#define GL_BOOLEAN_DIALOG_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_BOOLEAN_DIALOG    			           (gl_boolean_dialog_get_type())
#define GL_BOOLEAN_DIALOG(obj)			       (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_BOOLEAN_DIALOG, GlBooleanDialog))
#define GL_BOOLEAN_DIALOG_CLASS(klass)		   (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_BOOLEAN_DIALOG, GlBooleanDialogClass))
#define GL_IS_BOOLEAN_DIALOG(obj)		               (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_BOOLEAN_DIALOG))
#define GL_IS_BOOLEAN_DIALOG_CLASS(klass)              (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_BOOLEAN_DIALOG))

typedef struct _GlBooleanDialog			        GlBooleanDialog;
typedef struct _GlBooleanDialogClass		        GlBooleanDialogClass;
typedef struct _GlBooleanDialogPrivate           GlBooleanDialogPrivate;

struct _GlBooleanDialogClass
{
	GtkWindowClass                           parent_class;
};

struct _GlBooleanDialog
{
	GtkWindow                                parent;
	GlBooleanDialogPrivate                       *priv;
};


GType 		         gl_boolean_dialog_get_type                ( void );


void                 gl_boolean_dialog_change_user_info        ( GlBooleanDialog *gl_boolean_dialog, const gchar *test  );


G_END_DECLS
#endif /* GL_BOOLEAN_DIALOG_H_ */

/** @} */

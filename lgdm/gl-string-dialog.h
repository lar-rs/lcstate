/**
 * @defgroup LgdmLibrary
 * @defgroup GlStringDialog
 * @ingroup  GlStringDialog
 * @{
 * @file  gl-string-dialog.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_STRING_DIALOG_H_
#define GL_STRING_DIALOG_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_STRING_DIALOG    			           (gl_string_dialog_get_type())
#define GL_STRING_DIALOG(obj)			       (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_STRING_DIALOG, GlStringDialog))
#define GL_STRING_DIALOG_CLASS(klass)		   (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_STRING_DIALOG, GlStringDialogClass))
#define GL_IS_STRING_DIALOG(obj)		               (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_STRING_DIALOG))
#define GL_IS_STRING_DIALOG_CLASS(klass)              (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_STRING_DIALOG))

typedef struct _GlStringDialog			        GlStringDialog;
typedef struct _GlStringDialogClass		        GlStringDialogClass;
typedef struct _GlStringDialogPrivate           GlStringDialogPrivate;

struct _GlStringDialogClass
{
	GtkWindowClass                           parent_class;
};

struct _GlStringDialog
{
	GtkWindow                                parent;
	GlStringDialogPrivate                       *priv;
};


GType 		         gl_string_dialog_get_type                ( void );


void                 gl_string_dialog_change_input_purpose    ( GlStringDialog *gl_string_dialog, GtkInputPurpose purpose );
const gchar*         gl_string_dialog_get_value               ( GlStringDialog *gl_string_dialog );
void                 gl_string_dialog_clean                   ( GlStringDialog *gl_string_dialog );

G_END_DECLS
#endif /* GL_STRING_DIALOG_H_ */

/** @} */

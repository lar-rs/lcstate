/**
 * @defgroup LgdmLibrary
 * @defgroup GlSpinnerDialog
 * @ingroup  GlSpinnerDialog
 * @{
 * @file  gl-spinner-dialog.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_SPINNER_DIALOG_H_
#define GL_SPINNER_DIALOG_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_SPINNER_DIALOG    			           (gl_spinner_dialog_get_type())
#define GL_SPINNER_DIALOG(obj)			       (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_SPINNER_DIALOG, GlSpinnerDialog))
#define GL_SPINNER_DIALOG_CLASS(klass)		   (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_SPINNER_DIALOG, GlSpinnerDialogClass))
#define GL_IS_SPINNER_DIALOG(obj)		               (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_SPINNER_DIALOG))
#define GL_IS_SPINNER_DIALOG_CLASS(klass)              (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_SPINNER_DIALOG))

typedef struct _GlSpinnerDialog			        GlSpinnerDialog;
typedef struct _GlSpinnerDialogClass		        GlSpinnerDialogClass;
typedef struct _GlSpinnerDialogPrivate           GlSpinnerDialogPrivate;

struct _GlSpinnerDialogClass
{
	GtkWindowClass                           parent_class;
};

struct _GlSpinnerDialog
{
	GtkWindow                                parent;
	GlSpinnerDialogPrivate                       *priv;
};


GType 		         gl_spinner_dialog_get_type                ( void );


void                 gl_spinner_dialog_set_digits              ( GlSpinnerDialog* spinner_dialog , guint digits );
void                 gl_spinner_dialog_set_increments          ( GlSpinnerDialog* spinner_dialog , gdouble step,  gdouble page );
void                 gl_spinner_dialog_set_range               ( GlSpinnerDialog* spinner_dialog , gdouble min,  gdouble max );
void                 gl_spinner_dialog_set_numeric             ( GlSpinnerDialog* spinner_dialog , gboolean numeric );
GtkAdjustment*       gl_spinner_dialog_get_adjustment          ( GlSpinnerDialog* spinner_dialog );



G_END_DECLS
#endif /* GL_SPINNER_DIALOG_H_ */

/** @} */

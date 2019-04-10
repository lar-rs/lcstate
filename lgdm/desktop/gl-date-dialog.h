/**
 * @defgroup LgdmLibrary
 * @defgroup GlDateDialog
 * @ingroup  GlDateDialog
 * @{
 * @file  gl-desktop-action.h object header
 * @brief This is LGDM action button object header file.
 * 	 Copyright (C) LAR 2013
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_LARDATE_DIALOG_H_
#define GL_LARDATE_DIALOG_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_DATE_DIALOG    			      (gl_date_dialog_get_type())
#define GL_DATE_DIALOG(obj)			          (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_DATE_DIALOG, GlDateDialog))
#define GL_DATE_DIALOG_CLASS(klass)		      (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_DATE_DIALOG, GlDateDialogClass))
#define GL_IS_DATE_DIALOG(obj)		          (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_DATE_DIALOG))
#define GL_IS_DATE_DIALOG_CLASS(klass)          (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_DATE_DIALOG))

typedef struct _GlDateDialog			          GlDateDialog;
typedef struct _GlDateDialogClass		          GlDateDialogClass;
typedef struct _GlDateDialogPrivate               GlDateDialogPrivate;

struct _GlDateDialogClass
{
	GtkListBoxClass                               parent_class;
};

struct _GlDateDialog
{
	GtkListBox                                    action;
	GlDateDialogPrivate                          *priv;
};


GType 		         gl_date_dialog_get_type                ( void );

gdouble              gl_date_dialog_get_total_sec           ( GlDateDialog *dialog );

void                 gl_date_dialog_set_total_sec           ( GlDateDialog *dialog , gdouble total_sec);

//---------------------------info ------------------------------------------------------------------


//-------------------------------progress bar control ----------------------------------------------


G_END_DECLS
#endif /* GL_LARDATE_DIALOG_H_ */

/** @} */

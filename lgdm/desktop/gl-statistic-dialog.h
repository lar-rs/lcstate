/**
 * @defgroup LgdmLibrary
 * @defgroup GlStatisticDialog
 * @ingroup  GlStatisticDialog
 * @{
 * @file  gl-statistic-dialog.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_STATISTIC_DIALOG_H_
#define GL_STATISTIC_DIALOG_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_STATISTIC_DIALOG    			           (gl_statistic_dialog_get_type())
#define GL_STATISTIC_DIALOG(obj)			       (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_STATISTIC_DIALOG, GlStatisticDialog))
#define GL_STATISTIC_DIALOG_CLASS(klass)		   (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_STATISTIC_DIALOG, GlStatisticDialogClass))
#define GL_IS_STATISTIC_DIALOG(obj)		               (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_STATISTIC_DIALOG))
#define GL_IS_STATISTIC_DIALOG_CLASS(klass)              (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_STATISTIC_DIALOG))

typedef struct _GlStatisticDialog			        GlStatisticDialog;
typedef struct _GlStatisticDialogClass		        GlStatisticDialogClass;
typedef struct _GlStatisticDialogPrivate            GlStatisticDialogPrivate;

struct _GlStatisticDialogClass
{
	GtkWindowClass                                  parent_class;
};

struct _GlStatisticDialog
{
	GtkWindow                                       parent;
	GlStatisticDialogPrivate                       *priv;
};


GType 		         gl_statistic_dialog_get_type                ( void );



G_END_DECLS
#endif /* GL_STATISTIC_DIALOG_H_ */

/** @} */

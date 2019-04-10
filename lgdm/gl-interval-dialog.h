/**
 * @defgroup LgdmLibrary
 * @defgroup GlIntervalDialog
 * @ingroup  GlIntervalDialog
 * @{
 * @file  gl-interval-dialog.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_INTERVAL_DIALOG_H_
#define GL_INTERVAL_DIALOG_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_INTERVAL_DIALOG    			           (gl_interval_dialog_get_type())
#define GL_INTERVAL_DIALOG(obj)			               (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_INTERVAL_DIALOG, GlIntervalDialog))
#define GL_INTERVAL_DIALOG_CLASS(klass)		           (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_INTERVAL_DIALOG, GlIntervalDialogClass))
#define GL_IS_INTERVAL_DIALOG(obj)		               (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_INTERVAL_DIALOG))
#define GL_IS_INTERVAL_DIALOG_CLASS(klass)             (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_INTERVAL_DIALOG))

typedef struct _GlIntervalDialog			           GlIntervalDialog;
typedef struct _GlIntervalDialogClass		           GlIntervalDialogClass;
typedef struct _GlIntervalDialogPrivate                GlIntervalDialogPrivate;

struct _GlIntervalDialogClass
{
	GtkWindowClass                                     parent_class;
};

struct _GlIntervalDialog
{
	GtkWindow                                          parent;
	GlIntervalDialogPrivate                           *priv;
};


GType 		         gl_interval_dialog_get_type                ( void );




gdouble              gl_interval_get_start_time                 ( GlIntervalDialog *dialog );
gdouble              gl_interval_get_stop_time                  ( GlIntervalDialog *dialog );

G_END_DECLS
#endif /* GL_INTERVAL_DIALOG_H_ */

/** @} */

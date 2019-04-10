/**
 * @ingroup  GlDateCompleteDialog
 * @{
 * @file  gl-date-complete-dialog.h object header
 * 	 Copyright (C) LAR 2015
 *
 * @author G.Stützer <gstuetzer@lar.com>
 *
 */

#ifndef GL_DATE_COMPLETE_DIALOG_H_
#define GL_DATE_COMPLETE_DIALOG_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_DATE_COMPLETE_DIALOG            (gl_date_complete_dialog_get_type())
#define GL_DATE_COMPLETE_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_DATE_COMPLETE_DIALOG, GlDateCompleteDialog))
#define GL_DATE_COMPLETE_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  GL_TYPE_DATE_COMPLETE_DIALOG, GlDateCompleteDialogClass))
#define GL_IS_DATE_COMPLETE_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GL_TYPE_DATE_COMPLETE_DIALOG))
#define GL_IS_DATE_COMPLETE_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  GL_TYPE_DATE_COMPLETE_DIALOG))

typedef struct _GlDateCompleteDialog        GlDateCompleteDialog;
typedef struct _GlDateCompleteDialogClass   GlDateCompleteDialogClass;
typedef struct _GlDateCompleteDialogPrivate GlDateCompleteDialogPrivate;

struct _GlDateCompleteDialogClass {
	GtkWindowClass parent_class;
};

struct _GlDateCompleteDialog {
	GtkWindow                    parent;
	GlDateCompleteDialogPrivate* priv;
};


GType 	gl_date_complete_dialog_get_type             (void);


gdouble gl_date_complete_dialog_get_time (GlDateCompleteDialog* gl_date_complete_dialog);
void    gl_date_complete_dialog_set_time (GlDateCompleteDialog* gl_date_complete_dialog, gdouble total_sec);


G_END_DECLS
#endif /* GL_DATE_COMPLETE_DIALOG_H_ */

/** @} */

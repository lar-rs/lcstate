/**
 * @defgroup LgdmLibrary
 * @defgroup GlComboDialog
 * @ingroup  GlComboDialog
 * @{
 * @file  gl-string-dialog.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_COMBO_DIALOG_H_
#define GL_COMBO_DIALOG_H_
#include <gtk/gtk.h>
#include <glib.h>
#include "gl-combo-row.h"

G_BEGIN_DECLS


#define GL_TYPE_COMBO_DIALOG    			           (gl_combo_dialog_get_type())
#define GL_COMBO_DIALOG(obj)			               (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_COMBO_DIALOG, GlComboDialog))
#define GL_COMBO_DIALOG_CLASS(klass)		           (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_COMBO_DIALOG, GlComboDialogClass))
#define GL_IS_COMBO_DIALOG(obj)		                   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_COMBO_DIALOG))
#define GL_IS_COMBO_DIALOG_CLASS(klass)                (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_COMBO_DIALOG))

typedef struct _GlComboDialog			        GlComboDialog;
typedef struct _GlComboDialogClass		        GlComboDialogClass;
typedef struct _GlComboDialogPrivate            GlComboDialogPrivate;

struct _GlComboDialogClass
{
	GtkWindowClass                              parent_class;
};

struct _GlComboDialog
{
	GtkWindow                                   parent;
	GlComboDialogPrivate                       *priv;
};


GType 		         gl_combo_dialog_get_type                ( void );


void                 gl_combo_dialog_add_row                 ( GlComboDialog *dialog , GlComboRow *row );
const gchar*         gl_combo_dialog_get_value               ( GlComboDialog *dialog );
GlComboRow*          gl_combo_row_get_activated              ( GlComboDialog *dialog );
guint                gl_combo_dialog_get_index               ( GlComboDialog *dialog );

G_END_DECLS
#endif /* GL_COMBO_DIALOG_H_ */

/** @} */

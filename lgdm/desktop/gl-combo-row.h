/**
 * @defgroup LgdmLibrary
 * @defgroup GlComboRow
 * @ingroup  GlComboRow
 * @{
 * @file  gl-combo-row.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_COMBO_ROW_H_
#define GL_COMBO_ROW_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_COMBO_ROW    			           (gl_combo_row_get_type())
#define GL_COMBO_ROW(obj)			       (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_COMBO_ROW, GlComboRow))
#define GL_COMBO_ROW_CLASS(klass)		   (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_COMBO_ROW, GlComboRowClass))
#define GL_IS_COMBO_ROW(obj)		               (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_COMBO_ROW))
#define GL_IS_COMBO_ROW_CLASS(klass)              (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_COMBO_ROW))

typedef struct _GlComboRow			        GlComboRow;
typedef struct _GlComboRowClass		        GlComboRowClass;
typedef struct _GlComboRowPrivate           GlComboRowPrivate;

struct _GlComboRowClass
{
	GtkListBoxRowClass                           parent_class;
};

struct _GlComboRow
{
	GtkListBoxRow                                parent;
	GlComboRowPrivate                           *priv;
};


GType 		         gl_combo_row_get_type                ( void );
void                 gl_combo_row_join_group              ( GlComboRow *row , GlComboRow *source );
void                 gl_combo_row_activate                ( GlComboRow *row );
void                 gl_combo_row_set_name                ( GlComboRow *row , const gchar *name );
const gchar*         gl_combo_row_get_value               ( GlComboRow *row );
const gchar*         gl_combo_row_get_name                ( GlComboRow *row );
void                 gl_combo_row_pack_content            ( GlComboRow *row, GtkWidget *widget, gboolean expand, gboolean fill, guint padding );
GtkWidget*           gl_combo_row_get_content             ( GlComboRow *row );


G_END_DECLS
#endif /* GL_COMBO_ROW_H_ */

/** @} */

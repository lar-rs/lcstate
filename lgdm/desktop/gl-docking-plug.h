/**
 * @defgroup LgdmLibrary
 * @defgroup GlDockingPlug
 * @ingroup  GlDockingPlug
 * @{
 * @file  gl-desktop-action.h object header
 * @brief This is LGDM action button object header file.
 * 	 Copyright (C) LAR 2013
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_LARDOCKING_PLUG_H_
#define GL_LARDOCKING_PLUG_H_
#include <gtk/gtk.h>
#include <glib.h>
#include <gio/gdesktopappinfo.h>

G_BEGIN_DECLS


#define GL_TYPE_DOCKING_PLUG    			     (gl_docking_plug_get_type())
#define GL_DOCKING_PLUG(obj)			         (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_DOCKING_PLUG, GlDockingPlug))
#define GL_DOCKING_PLUG_CLASS(klass)		     (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_DOCKING_PLUG, GlDockingPlugClass))
#define GL_IS_DOCKING_PLUG(obj)		             (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_DOCKING_PLUG))
#define GL_IS_DOCKING_PLUG_CLASS(klass)          (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_DOCKING_PLUG))

typedef struct _GlDockingPlug		              GlDockingPlug;
typedef struct _GlDockingPlugClass		          GlDockingPlugClass;
typedef struct _GlDockingPlugPrivate              GlDockingPlugPrivate;

struct _GlDockingPlugClass
{
	GtkBoxClass                                   parent_class;

};

struct _GlDockingPlug
{
	GtkBox                                      object;
	GlDockingPlugPrivate                       *priv;
};


GType 		         gl_docking_plug_get_type                ( void );

void                 gl_docking_plug_write_pipe              ( GlDockingPlug *docking , const gchar *format,...)G_GNUC_PRINTF (2, 3);



G_END_DECLS
#endif /* GL_LARDOCKING_PLUG_H_ */

/** @} */

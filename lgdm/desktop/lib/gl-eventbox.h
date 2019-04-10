/**
 * @defgroup LgdmLibrary
 * @defgroup GlEventbox
 * @ingroup  GlEventbox
 * @{
 * @file  gl-desktop-action.h object header
 * @brief This is LGDM action button object header file.
 * 	 Copyright (C) LAR 2013
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef __GL_EVENTBOX_H__
#define __GL_EVENTBOX_H__
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_EVENTBOX    			     (gl_eventbox_get_type())
#define GL_EVENTBOX(obj)			         (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_EVENTBOX, GlEventbox))
#define GL_EVENTBOX_CLASS(klass)		     (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_EVENTBOX, GlEventboxClass))
#define GL_IS_EVENTBOX(obj)		             (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_EVENTBOX))
#define GL_IS_EVENTBOX_CLASS(klass)          (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_EVENTBOX))
#define GL_EVENTBOX_GET_CLASS(obj)           (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_EVENTBOX, GlEventboxClass))

typedef struct _GlEventbox			          GlEventbox;
typedef struct _GlEventboxClass		          GlEventboxClass;
typedef struct _GlEventboxPrivate             GlEventboxPrivate;

struct _GlEventboxClass
{
	GtkEventBoxClass                          parent_class;
};

struct _GlEventbox
{
	GtkEventBox                               eventbox;
	GlEventboxPrivate                        *priv;
};


GType 		         gl_eventbox_get_type                        ( void );



G_END_DECLS
#endif /* __GL_EVENTBOX_H__ */

/** @} */

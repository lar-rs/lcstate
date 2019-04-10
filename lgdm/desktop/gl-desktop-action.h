/**
 * @defgroup LgdmLibrary
 * @defgroup GlDesktopAction
 * @ingroup  GlDesktopAction
 * @{
 * @file  gl-desktop-action.h object header
 * @brief This is LGDM action button object header file.
 * 	 Copyright (C) LAR 2013
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_LARDESKTOP_ACTION_H_
#define GL_LARDESKTOP_ACTION_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_DESKTOP_ACTION    			     (gl_desktop_action_get_type())
#define GL_DESKTOP_ACTION(obj)			         (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_DESKTOP_ACTION, GlDesktopAction))
#define GL_DESKTOP_ACTION_CLASS(klass)		     (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_DESKTOP_ACTION, GlDesktopActionClass))
#define GL_IS_DESKTOP_ACTION(obj)		         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_DESKTOP_ACTION))
#define GL_IS_DESKTOP_ACTION_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_DESKTOP_ACTION))

typedef struct _GlDesktopAction			          GlDesktopAction;
typedef struct _GlDesktopActionClass		      GlDesktopActionClass;
typedef struct _GlDesktopActionPrivate            GlDesktopActionPrivate;

struct _GlDesktopActionClass
{
	GtkBoxClass                      parent_class;
	void                           (*action_start)       ( GlDesktopAction *action );
};

struct _GlDesktopAction
{
	GtkBox                           action;
	GlDesktopActionPrivate          *priv;
};


GType 		         gl_desktop_action_get_type                ( void );
GtkWidget*  	     gl_desktop_action_new                     (  );



//---------------------------info ------------------------------------------------------------------


//-------------------------------progress bar control ----------------------------------------------


G_END_DECLS
#endif /* GL_LARDESKTOP_ACTION_H_ */

/** @} */

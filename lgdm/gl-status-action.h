/**
 * @defgroup LgdmLibrary
 * @defgroup GlStatusAction
 * @ingroup  GlStatusAction
 * @{
 * @file  gl-desktop-action.h object header
 * @brief This is LGDM action button object header file.
 * 	 Copyright (C) LAR 2013
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_LARSTATUS_ACTION_H_
#define GL_LARSTATUS_ACTION_H_
#include <gtk/gtk.h>
#include <glib.h>

G_BEGIN_DECLS


#define GL_TYPE_STATUS_ACTION    			     (gl_status_action_get_type())
#define GL_STATUS_ACTION(obj)			         (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_STATUS_ACTION, GlStatusAction))
#define GL_STATUS_ACTION_CLASS(klass)		     (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_STATUS_ACTION, GlStatusActionClass))
#define GL_IS_STATUS_ACTION(obj)		         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_STATUS_ACTION))
#define GL_IS_STATUS_ACTION_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_STATUS_ACTION))

typedef struct _GlStatusAction			          GlStatusAction;
typedef struct _GlStatusActionClass		          GlStatusActionClass;
typedef struct _GlStatusActionPrivate             GlStatusActionPrivate;

struct _GlStatusActionClass
{
	GtkBoxClass                      parent_class;
	void                           (*action_start)       ( GlStatusAction *action );
};

struct _GlStatusAction
{
	GtkBox                           action;
	GlStatusActionPrivate          *priv;
};


GType 		         gl_status_action_get_type                ( void );

//---------------------------info ------------------------------------------------------------------


//-------------------------------progress bar control ----------------------------------------------


G_END_DECLS
#endif /* GL_LARSTATUS_ACTION_H_ */

/** @} */

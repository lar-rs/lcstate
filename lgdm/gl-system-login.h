/**
 * @defgroup LgdmLibrary
 * @defgroup GlSystemLogin
 * @ingroup  GlSystemLogin
 * @{
 * @file  lgdm-desktop-place.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_SYSTEM_LOGIN_H_
#define GL_SYSTEM_LOGIN_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_SYSTEM_LOGIN    			    (gl_system_login_get_type())
#define GL_SYSTEM_LOGIN(obj)			        (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_SYSTEM_LOGIN, GlSystemLogin))
#define GL_SYSTEM_LOGIN_CLASS(klass)		    (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_SYSTEM_LOGIN, GlSystemLoginClass))
#define GL_IS_SYSTEM_LOGIN(obj)		            (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_SYSTEM_LOGIN))
#define GL_IS_SYSTEM_LOGIN_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_SYSTEM_LOGIN))

typedef struct _GlSystemLogin			        GlSystemLogin;
typedef struct _GlSystemLoginClass		        GlSystemLoginClass;
typedef struct _GlSystemLoginPrivate            GlSystemLoginPrivate;

struct _GlSystemLoginClass
{
	GtkWindowClass                                  parent_class;
};

struct _GlSystemLogin
{
	GtkWindow                                       parent;
	GlSystemLoginPrivate                            *priv;
};


GType 		         gl_system_login_get_type                ( void );


G_END_DECLS
#endif /* GL_SYSTEM_LOGIN_H_ */

/** @} */

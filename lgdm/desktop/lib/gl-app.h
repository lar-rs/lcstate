/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @defgroup GlLibrary
 * @defgroup GlApp
 * @ingroup  GlApp
 * @{
 * @file  gl-app.h	APP interface model header
 * @brief This is APP interface model object header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 * $Id: $ $URL: $
 */

#ifndef _GL_APP_H_
#define _GL_APP_H_

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <stdlib.h> /* exit, EXIT_FAILURE */
#include <string.h> /* strcmp */
#include "mkt-dbus.h"
#include "dbus-interface/gl_app-client-interface.h"




G_BEGIN_DECLS

#define GL_TYPE_APP                 (gl_app_get_type ())
#define GL_APP(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_APP, GlApp))
#define GL_IS_APP(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_APP))
#define GL_APP_GET_INTERFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), GL_TYPE_APP, GlAppInterface))



typedef struct _GlApp               GlApp;
typedef struct _GlAppInterface      GlAppInterface;

typedef enum
{
	GL_APP_STATE_PAUSE,
	GL_APP_STATE_ACTIVITY
}GlAppStates;


struct _GlAppInterface
{
    GTypeInterface      parent_iface;
    gboolean          (*open)                                                  ( GlApp *self );
    gboolean          (*release)                                               ( GlApp *self );
    gboolean          (*hold)                                                  ( GlApp *self );
    gboolean          (*close)                                                 ( GlApp *self );

};


GType                 gl_app_get_type                                          ( void );

gboolean              gl_app_start                                             ( MktDbus *app );
gboolean              gl_app_stop                                              ( MktDbus *app );
gboolean              gl_app_activate                                          ( MktDbus *app );
gboolean              gl_app_deaktivate                                        ( MktDbus *app );





#endif /* _GL_APPEN_H_ */
/** @} */

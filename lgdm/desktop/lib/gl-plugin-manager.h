/*
 * gl-updade.h
 *
 *  Created on: 22.02.2013
 *      Author: sascha
 */

#ifndef GL_PLUGIN_MANAGER_H_
#define GL_PLUGIN_MANAGER_H_

#include "gl-plugin.h"

G_BEGIN_DECLS

#define GL_TYPE_PLUGIN_MANAGER                 (gl_plugin_manager_get_type ())
#define GL_PLUGIN_MANAGER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_PLUGIN_MANAGER,  GlPluginManager))
#define GL_PLUGIN_MANAGER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass),  GL_TYPE_PLUGIN_MANAGER,  GlPluginManagerClass))
#define GL_IS_PLUGIN_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_PLUGIN_MANAGER))
#define GL_IS_PLUGIN_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass),  GL_TYPE_PLUGIN_MANAGER))
#define GL_PLUGIN_MANAGER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj),  GL_TYPE_PLUGIN_MANAGER,  GlPluginManagerClass))

typedef struct _GlPluginManagerClass   GlPluginManagerClass;
typedef struct _GlPluginManager        GlPluginManager;
typedef struct _GlPluginManagerPrivate GlPluginManagerPrivate;


struct _GlPluginManagerClass
{
	MktWindowClass                 parent_class;

};

struct _GlPluginManager
{
	MktWindow                      parent_instance;
	GlPluginManagerPrivate               *priv;
};

GType                        gl_plugin_manager_get_type        ( void ) G_GNUC_CONST;



#endif /* GL_UPDADE_H_ */

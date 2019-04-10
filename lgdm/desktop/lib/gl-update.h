/*
 * gl-updade.h
 *
 *  Created on: 22.02.2013
 *      Author: sascha
 */

#ifndef GL_UPDADE_H_
#define GL_UPDADE_H_

#include "gl-plugin.h"

G_BEGIN_DECLS

#define GL_TYPE_UPDATE                 (gl_update_get_type ())
#define GL_UPDATE(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_UPDATE,  GlUpdate))
#define GL_UPDATE_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass),  GL_TYPE_UPDATE,  GlUpdateClass))
#define GL_IS_UPDATE(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_UPDATE))
#define GL_IS_UPDATE_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass),  GL_TYPE_UPDATE))
#define GL_UPDATE_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj),  GL_TYPE_UPDATE,  GlUpdateClass))

typedef struct _GlUpdateClass   GlUpdateClass;
typedef struct _GlUpdate        GlUpdate;
typedef struct _GlUpdatePrivate GlUpdatePrivate;


struct _GlUpdateClass
{
	GlPluginClass                 parent_class;

};

struct _GlUpdate
{
	GlPlugin                       parent_instance;
	GlUpdatePrivate               *priv;
};

GType                        gl_update_get_type        ( void ) G_GNUC_CONST;



#endif /* GL_UPDADE_H_ */

/*
 * gl-updade.h
 *
 *  Created on: 22.02.2013
 *      Author: sascha
 */

#ifndef GL_WIZARD_MANAGER_H_
#define GL_WIZARD_MANAGER_H_

#include "gl-plugin.h"

G_BEGIN_DECLS

#define GL_TYPE_WIZARD_MANAGER                 (gl_wizard_manager_get_type ())
#define GL_WIZARD_MANAGER(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_WIZARD_MANAGER,  GlWizardManager))
#define GL_WIZARD_MANAGER_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass),  GL_TYPE_WIZARD_MANAGER,  GlWizardManagerClass))
#define GL_IS_WIZARD_MANAGER(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_WIZARD_MANAGER))
#define GL_IS_WIZARD_MANAGER_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass),  GL_TYPE_WIZARD_MANAGER))
#define GL_WIZARD_MANAGER_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj),  GL_TYPE_WIZARD_MANAGER,  GlWizardManagerClass))

typedef struct _GlWizardManagerClass   GlWizardManagerClass;
typedef struct _GlWizardManager        GlWizardManager;
typedef struct _GlWizardManagerPrivate GlWizardManagerPrivate;


struct _GlWizardManagerClass
{
	MktWindowClass                 parent_class;

};

struct _GlWizardManager
{
	MktWindow                      parent_instance;
	GlWizardManagerPrivate               *priv;
};

GType                        gl_wizard_manager_get_type        ( void ) G_GNUC_CONST;



#endif /* GL_WIZARD_MANAGER_H_ */

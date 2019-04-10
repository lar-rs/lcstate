/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-wizard.c
 * Copyright (C) sascha 2012 <sascha@sascha-desktop>
 * 
gl-wizard.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-wizard.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GL_WIZARD_H_
#define _GL_WIZARD_H_

#include "mkt-atom.h"
#include <gtk/gtk.h>


G_BEGIN_DECLS

#define GL_TYPE_WIZARD             (gl_wizard_get_type ())
#define GL_WIZARD(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_WIZARD, GlWizard))
#define GL_WIZARD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_WIZARD, GlWizardClass))
#define GL_IS_WIZARD(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_WIZARD))
#define GL_IS_WIZARD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_WIZARD))
#define GL_WIZARD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_WIZARD, GlWizardClass))

typedef struct _GlWizardClass    GlWizardClass;
typedef struct _GlWizard         GlWizard;
typedef struct _GlWizardPrivate  GlWizardPrivate;


struct _GlWizardClass
{
	MktAtomClass    parent_class;
	void          (*change_step)    ( GlWizard *wizard );
};

struct _GlWizard
{
	MktAtom            parent_instance;
	GlWizardPrivate   *priv;

};

GType                         gl_wizard_get_type              ( void ) G_GNUC_CONST;

GlWizard*                     gl_wizard_new                   ( const gchar *path );

GtkWidget*                    gl_wizard_get_starter           ( GlWizard *wisard );

gchar*                        gl_wizard_get_nick              ( GlWizard *wisard );

gboolean                      gl_wizard_start                 ( GlWizard *wizard );


gint                          gl_wizard_get_current_step      ( GlWizard *wizard );
gint                          gl_wizard_get_nth_steps         ( GlWizard *wizard );

G_END_DECLS

#endif /* _GL_WIZARD_H_ */

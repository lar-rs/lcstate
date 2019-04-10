/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * dbusexample
 * Copyright (C) sascha 2012 <sascha@sascha-ThinkPad-X61>
 * 
dbusexample is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * dbusexample is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GL_LOG_H_
#define _GL_LOG_H_

#include "gl-plugin.h"

G_BEGIN_DECLS

#define GL_TYPE_LOG                 (gl_log_get_type ())
#define GL_LOG(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_LOG, GlLog))
#define GL_LOG_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_LOG, GlLogClass))
#define GL_IS_LOG(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_LOG))
#define GL_IS_LOG_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_LOG))
#define GL_LOG_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_LOG, GlLogClass))

typedef struct _GlLogClass   GlLogClass;
typedef struct _GlLog        GlLog;
typedef struct _GlLogPrivate GlLogPrivate;


struct _GlLogClass
{
	GlPluginClass              parent_class;

};

struct _GlLog
{
	GlPlugin                   parent_instance;
    GlLogPrivate              *priv;
};

GType                        gl_log_get_type        ( void ) G_GNUC_CONST;


const gchar*                 gl_log_get_error_color ( GlLog *log ,const gchar *error);


G_END_DECLS

#endif /* _GL_LOG_H_ */

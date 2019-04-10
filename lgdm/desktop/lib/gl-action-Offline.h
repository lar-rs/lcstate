/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * src
 * Copyright (C) sascha 2012 <sascha@sascha-desktop>
 * 
src is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * src is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GL_ACTION_OFFLINE_H_
#define _GL_ACTION_OFFLINE_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include "gl-action-widget.h"

G_BEGIN_DECLS

#define GL_TYPE_ACTION_OFFLINE             (gl_action_Offline_get_type ())
#define GL_ACTION_OFFLINE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_ACTION_OFFLINE, GlActionOffline))
#define GL_ACTION_OFFLINE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GL_TYPE_ACTION_OFFLINE, GlActionOfflineClass))
#define GL_IS_ACTION_OFFLINE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_ACTION_OFFLINE))
#define GL_IS_ACTION_OFFLINE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GL_TYPE_ACTION_OFFLINE))
#define GL_ACTION_OFFLINE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GL_TYPE_ACTION_OFFLINE, GlActionOfflineClass))

typedef struct _GlActionOfflineClass      GlActionOfflineClass;
typedef struct _GlActionOffline           GlActionOffline ;
typedef struct _GlActionOfflinePrivate    GlActionOfflinePrivate;


struct _GlActionOfflineClass
{
	GlActionWidgetClass        parent_class;
};

struct _GlActionOffline
{
	GlActionWidget             parent_instance;
	GlActionOfflinePrivate     *priv;
};

GType                gl_action_Offline_get_type                  (void) G_GNUC_CONST;



G_END_DECLS

#endif /* _GL_ACTION_OFFLINE_H_ */

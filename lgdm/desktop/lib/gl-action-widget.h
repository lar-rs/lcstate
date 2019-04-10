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

#ifndef _GL_ACTION_WIDGET_H_
#define _GL_ACTION_WIDGET_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include "mkt-window.h"

G_BEGIN_DECLS

#define GL_TYPE_ACTION_WIDGET             (gl_action_widget_get_type ())
#define GL_ACTION_WIDGET(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_ACTION_WIDGET, GlActionWidget))
#define GL_ACTION_WIDGET_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_ACTION_WIDGET, GlActionWidgetClass))
#define GL_IS_ACTION_WIDGET(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_ACTION_WIDGET))
#define GL_IS_ACTION_WIDGET_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_ACTION_WIDGET))
#define GL_ACTION_WIDGET_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_ACTION_WIDGET, GlActionWidgetClass))

typedef struct _GlActionWidgetClass      GlActionWidgetClass;
typedef struct _GlActionWidget           GlActionWidget;
typedef struct _GlActionWidgetPrivate    GlActionWidgetPrivate;

#define   GL_ACTION_SIZE_DEFAULT    "default"
#define   GL_ACTION_SIZE_SMALL      "small"
#define   GL_ACTION_SIZE_LARGE      "large"



struct _GlActionWidgetClass
{
	MktWindowClass             parent_class;
	void                      (*action_start)  (GlActionWidget *widget );
};

struct _GlActionWidget
{
	MktWindow                  parent_instance;
	GlActionWidgetPrivate     *priv;
};

GType                gl_action_widget_get_type                  (void) G_GNUC_CONST;

MktAtom*             gl_action_widget_new_for_parent            ( MktWindow *window , const gchar *id ,const gchar *path,const gchar *nick,  gboolean active  );

MktAtom*             gl_action_creat_copy                       ( GlActionWidget *action , const gchar *new_id , const gchar *place );

const GValue*        gl_action_widget_get_parameter             ( GlActionWidget *action , const gchar *parameter );

const gchar*         gl_action_widget_get_nick                  ( GlActionWidget *action );
const gchar*         gl_action_widget_get_place                 ( GlActionWidget *action );

void                 gl_action_widget_set_sensitive             ( GlActionWidget *action, gboolean sensitive);


G_END_DECLS

#endif /* _GL_ACTION_WIDGET_H_ */

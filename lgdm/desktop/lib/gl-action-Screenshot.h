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

#ifndef _GL_SCREENSHOT_H_
#define _GL_SCREENSHOT_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include "gl-action-widget.h"

G_BEGIN_DECLS

#define GL_TYPE_SCREENSHOT             (gl_screenshot_get_type ())
#define GL_SCREENSHOT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_SCREENSHOT, GlScreenshot))
#define GL_SCREENSHOT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  GL_TYPE_SCREENSHOT, GlScreenshotClass))
#define GL_IS_SCREENSHOT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_SCREENSHOT))
#define GL_IS_SCREENSHOT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  GL_TYPE_SCREENSHOT))
#define GL_SCREENSHOT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  GL_TYPE_SCREENSHOT, GlScreenshotClass))

typedef struct _GlScreenshotClass      GlScreenshotClass;
typedef struct _GlScreenshot           GlScreenshot ;
typedef struct _GlScreenshotPrivate    GlScreenshotPrivate;


struct _GlScreenshotClass
{
	GlActionWidgetClass        parent_class;
};

struct _GlScreenshot
{
	GlActionWidget             parent_instance;
	GlScreenshotPrivate     *priv;
};

GType                gl_screenshot_get_type                  (void) G_GNUC_CONST;



G_END_DECLS

#endif /* _GL_SCREENSHOT_H_ */

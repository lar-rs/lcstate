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

#include "gl-action-Screenshot.h"
//#include "gl-system.h"
#include "gl-draganddrop.h"
#include "gl-level-manager.h"
//#include "gl-connection.h"
#include "mkt-collector.h"
#include <stdlib.h>



struct _GlScreenshotPrivate
{
	gchar *screen;


};


enum {
	PROP_SCREENSHOT_0,
	PROP_SCREENSHOT_ICON_OPEN,
	PROP_SCREENSHOT_ICON_CLOSE
};


#define GL_ACTION_WIDGET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_SCREENSHOT, GlScreenshotPrivate))



G_DEFINE_TYPE (GlScreenshot, gl_screenshot, GL_TYPE_ACTION_WIDGET );



static void
gl_screenshot_action_start ( GlActionWidget *action )
{
	//TEST:g_debug("gl_screenshot_action_start");
	//FIXME: run screen shot bash script
	//gl_system_start_process (mkIget(control_gui__filenameExecutableScreenshot),
	//			"/bin/bash","bash",mkIget(control_gui__filenameExecutableScreenshot),NULL);
}


static void
gl_screenshot_init (GlScreenshot *object)
{
	object -> priv = G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_SCREENSHOT,GlScreenshotPrivate);

}

static void
gl_screenshot_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
//	GlScreenshot *widget = GL_SCREENSHOT(object);


	G_OBJECT_CLASS (gl_screenshot_parent_class)->finalize (object);
}
static void
gl_screenshot_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_ACTION_WIDGET (object));
	//GlScreenshot* widget = GL_SCREENSHOT(object);
	//g_debug ("Set Property ...id%d - %s ",prop_id,g_param_spec_get_name(pspec));
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}
static void
gl_screenshot_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_ACTION_WIDGET (object));
	//GlScreenshot* widget = GL_SCREENSHOT(object);
	switch (prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_screenshot_class_init ( GlScreenshotClass *klass )
{
	GObjectClass*        object_class     =  G_OBJECT_CLASS (klass);
	GlActionWidgetClass* parent_class     =  GL_ACTION_WIDGET_CLASS(klass);
	object_class -> set_property          =  gl_screenshot_set_property;
	object_class -> get_property          =  gl_screenshot_get_property;
	object_class -> finalize              =  gl_screenshot_finalize;
	parent_class -> action_start          =  gl_screenshot_action_start;

	g_type_class_add_private (klass, sizeof (GlScreenshotPrivate));


	/*GParamSpec *pspec;

	pspec = g_param_spec_string("open_icon",
				"Action widget open icon",
				"Set/Get open icon path",
				"default",
				G_PARAM_READABLE | G_PARAM_WRITABLE );
		g_object_class_install_property (object_class,
				PROP_SCREENSHOT_ICON_OPEN,pspec);

	pspec = g_param_spec_string("close_icon",
			"Action widget close icon",
			"Set/Get close icon path",
			"default",
			G_PARAM_READABLE | G_PARAM_WRITABLE  );
	g_object_class_install_property (object_class,
			PROP_SCREENSHOT_ICON_CLOSE,pspec);*/
}


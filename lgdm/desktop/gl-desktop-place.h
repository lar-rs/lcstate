/**
 * @defgroup LgdmLibrary
 * @defgroup GlDesktopPlace
 * @ingroup  GlDesktopPlace
 * @{
 * @file  gl-desktop-place.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_DESKTOP_PLACE_H_
#define GL_DESKTOP_PLACE_H_
#include <gtk/gtk.h>
#include <glib.h>
#include "gl-app-launcher.h"


G_BEGIN_DECLS


#define GL_TYPE_DESKTOP_PLACE    			           (gl_desktop_place_get_type())
#define GL_DESKTOP_PLACE(obj)			       (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_DESKTOP_PLACE, GlDesktopPlace))
#define GL_DESKTOP_PLACE_CLASS(klass)		   (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_DESKTOP_PLACE, GlDesktopPlaceClass))
#define GL_IS_DESKTOP_PLACE(obj)		               (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_DESKTOP_PLACE))
#define GL_IS_DESKTOP_PLACE_CLASS(klass)              (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_DESKTOP_PLACE))

typedef struct _GlDesktopPlace			        GlDesktopPlace;
typedef struct _GlDesktopPlaceClass		        GlDesktopPlaceClass;
typedef struct _GlDesktopPlacePrivate           GlDesktopPlacePrivate;

struct _GlDesktopPlaceClass
{
	GtkBoxClass                                  parent_class;
};

struct _GlDesktopPlace
{
	GtkBox                                       parent;
	GlDesktopPlacePrivate                       *priv;
};


GType 		         gl_desktop_place_get_type                ( void );

gboolean             gl_desktop_place_add_action             ( GlDesktopPlace *desktop , GtkWidget *action , guint x_pos , guint y_pos , guint level );



G_END_DECLS
#endif /* GL_DESKTOP_PLACE_H_ */

/** @} */

/**
 * @defgroup LgdmLibrary
 * @defgroup LgdmDesktopPlace
 * @ingroup  LgdmDesktopPlace
 * @{
 * @file  lgdm-desktop-place.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef Lgdm_DESKTOP_PLACE_H_
#define Lgdm_DESKTOP_PLACE_H_
#include <gtk/gtk.h>
#include <glib.h>

#include "lgdm-app-launcher.h"


G_BEGIN_DECLS


#define LGDM_TYPE_DESKTOP_PLACE    			           (lgdm_desktop_place_get_type())
#define LGDM_DESKTOP_PLACE(obj)			               (G_TYPE_CHECK_INSTANCE_CAST((obj),LGDM_TYPE_DESKTOP_PLACE, LgdmDesktopPlace))
#define LGDM_DESKTOP_PLACE_CLASS(klass)		           (G_TYPE_CHECK_CLASS_CAST((klass) ,LGDM_TYPE_DESKTOP_PLACE, LgdmDesktopPlaceClass))
#define LGDM_IS_DESKTOP_PLACE(obj)		               (G_TYPE_CHECK_INSTANCE_TYPE((obj),LGDM_TYPE_DESKTOP_PLACE))
#define LGDM_IS_DESKTOP_PLACE_CLASS(klass)             (G_TYPE_CHECK_CLASS_TYPE((klass) ,LGDM_TYPE_DESKTOP_PLACE))

typedef struct _LgdmDesktopPlace			        LgdmDesktopPlace;
typedef struct _LgdmDesktopPlaceClass		        LgdmDesktopPlaceClass;
typedef struct _LgdmDesktopPlacePrivate             LgdmDesktopPlacePrivate;

struct _LgdmDesktopPlaceClass
{
	GtkBoxClass                                  parent_class;
};

struct _LgdmDesktopPlace
{
	GtkBox                                       parent;
	LgdmDesktopPlacePrivate                       *priv;
};


GType 		         lgdm_desktop_place_get_type                ( void );

gboolean             lgdm_desktop_place_add_action             ( LgdmDesktopPlace *desktop , GtkWidget *action , guint x_pos , guint y_pos , guint level );



G_END_DECLS
#endif /* Lgdm_DESKTOP_PLACE_H_ */

/** @} */

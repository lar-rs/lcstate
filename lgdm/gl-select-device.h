/**
 * @defgroup LgdmLibrary
 * @defgroup GlSelectDevice
 * @ingroup  GlSelectDevice
 * @{
 * @file  lgdm-desktop-place.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_SELECT_DEVICE_H_
#define GL_SELECT_DEVICE_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_SELECT_DEVICE    			    (gl_select_device_get_type())
#define GL_SELECT_DEVICE(obj)			        (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_SELECT_DEVICE, GlSelectDevice))
#define GL_SELECT_DEVICE_CLASS(klass)		    (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_SELECT_DEVICE, GlSelectDeviceClass))
#define GL_IS_SELECT_DEVICE(obj)		            (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_SELECT_DEVICE))
#define GL_IS_SELECT_DEVICE_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_SELECT_DEVICE))

typedef struct _GlSelectDevice			        GlSelectDevice;
typedef struct _GlSelectDeviceClass		        GlSelectDeviceClass;
typedef struct _GlSelectDevicePrivate            GlSelectDevicePrivate;

struct _GlSelectDeviceClass
{
	GtkWindowClass                                  parent_class;
};

struct _GlSelectDevice
{
	GtkWindow                                       parent;
	GlSelectDevicePrivate                            *priv;
};


GType 		         gl_select_device_get_type                ( void );


void                 gl_select_device_run                     ( GlSelectDevice *select_device );

G_END_DECLS
#endif /* GL_SELECT_DEVICE_H_ */

/** @} */

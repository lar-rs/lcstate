/**
 * @defgroup UltimateBusLibrary
 * @defgroup UltimateProcessObject
 * @ingroup  UltimateProcessObject
 * @{
 * @file  ultimate-process_object.h	ProcessObject object header
 * @brief   ProcessObject object header file.
 *
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author       A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef _ULTIMATE_PROCESS_OBJECT_H_
#define _ULTIMATE_PROCESS_OBJECT_H_

#include <glib-object.h>
#include <mktbus.h>
#include "mkt-process-object.h"

G_BEGIN_DECLS

#define ULTIMATE_TYPE_PROCESS_OBJECT             (ultimate_process_object_get_type ())
#define ULTIMATE_PROCESS_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTIMATE_TYPE_PROCESS_OBJECT, UltimateProcessObject))
#define ULTIMATE_PROCESS_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTIMATE_TYPE_PROCESS_OBJECT, UltimateProcessObjectClass))
#define ULTIMATE_IS_PROCESS_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTIMATE_TYPE_PROCESS_OBJECT))
#define ULTIMATE_IS_PROCESS_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTIMATE_TYPE_PROCESS_OBJECT))
#define ULTIMATE_PROCESS_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTIMATE_TYPE_PROCESS_OBJECT, UltimateProcessObjectClass))

typedef struct _UltimateProcessObjectClass     UltimateProcessObjectClass;
typedef struct _UltimateProcessObject          UltimateProcessObject;
typedef struct _UltimateProcessObjectPrivate   UltimateProcessObjectPrivate;

GType ultimate_process_object_get_type (void) G_GNUC_CONST;


struct _UltimateProcessObjectClass
{
	MktProcessObjectClass                       parent_class;


};

struct _UltimateProcessObject
{
	MktProcessObject                            parent_instance;
	UltimateProcessObjectPrivate               *priv;
};



StreamsObject*                                  ultimate_process_stream                  ( UltimateProcessObject *process );

G_END_DECLS

#endif /* _ULTIMATE_PROCESS_OBJECT_H_ */

/** @} */

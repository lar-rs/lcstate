/**
 * @defgroup UltimateBusLibrary
 * @defgroup UltimateProcessRinsing
 * @ingroup  UltimateProcessRinsing
 * @{
 * @file  ultimate-process_rinsing.h	ProcessObject object header
 * @brief   ProcessObject object header file.
 *
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author       A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef _ULTIMATE_PROCESS_RINSING_H_
#define _ULTIMATE_PROCESS_RINSING_H_

#include <glib-object.h>
#include <mktbus.h>

G_BEGIN_DECLS

#define ULTIMATE_TYPE_PROCESS_RINSING             (ultimate_process_rinsing_get_type ())
#define ULTIMATE_PROCESS_RINSING(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTIMATE_TYPE_PROCESS_RINSING, UltimateProcessRinsing))
#define ULTIMATE_PROCESS_RINSING_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ULTIMATE_TYPE_PROCESS_RINSING, UltimateProcessRinsingClass))
#define ULTIMATE_IS_PROCESS_RINSING(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTIMATE_TYPE_PROCESS_RINSING))
#define ULTIMATE_IS_PROCESS_RINSING_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ULTIMATE_TYPE_PROCESS_RINSING))
#define ULTIMATE_PROCESS_RINSING_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ULTIMATE_TYPE_PROCESS_RINSING, UltimateProcessRinsingClass))

typedef struct _UltimateProcessRinsingClass     UltimateProcessRinsingClass;
typedef struct _UltimateProcessRinsing          UltimateProcessRinsing;
typedef struct _UltimateProcessRinsingPrivate   UltimateProcessRinsingPrivate;

GType ultimate_process_rinsing_get_type (void) G_GNUC_CONST;


struct _UltimateProcessRinsingClass
{
	MktProcessObjectClass                       parent_class;


};

struct _UltimateProcessRinsing
{
	MktProcessObject                            parent_instance;
	UltimateProcessRinsingPrivate               *priv;
};




G_END_DECLS

#endif /* _ULTIMATE_PROCESS_RINSING_H_ */

/** @} */

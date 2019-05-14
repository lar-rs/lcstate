/**
 * @file  ultimate-process_hold.h	ProcessObject object header
 * @brief   ProcessObject object header file.
 *
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author       A.Smolkov <asmolkov@lar.com>
 *
 */

#ifndef _ULTIMATE_PROCESS_HOLD_H_
#define _ULTIMATE_PROCESS_HOLD_H_

#include <glib-object.h>
#include <mktbus.h>
#include "mkt-process-object.h"
G_BEGIN_DECLS

#define ULTIMATE_TYPE_PROCESS_HOLD (ultimate_process_hold_get_type())
#define ULTIMATE_PROCESS_HOLD(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTIMATE_TYPE_PROCESS_HOLD, UltimateProcessHold))
#define ULTIMATE_PROCESS_HOLD_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTIMATE_TYPE_PROCESS_HOLD, UltimateProcessHoldClass))
#define ULTIMATE_IS_PROCESS_HOLD(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTIMATE_TYPE_PROCESS_HOLD))
#define ULTIMATE_IS_PROCESS_HOLD_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTIMATE_TYPE_PROCESS_HOLD))
#define ULTIMATE_PROCESS_HOLD_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTIMATE_TYPE_PROCESS_HOLD, UltimateProcessHoldClass))

typedef struct _UltimateProcessHoldClass   UltimateProcessHoldClass;
typedef struct _UltimateProcessHold        UltimateProcessHold;
typedef struct _UltimateProcessHoldPrivate UltimateProcessHoldPrivate;

GType ultimate_process_hold_get_type(void) G_GNUC_CONST;

struct _UltimateProcessHoldClass {
    MktProcessObjectClass parent_class;
};

struct _UltimateProcessHold {
    MktProcessObject            parent_instance;
    UltimateProcessHoldPrivate *priv;
};

void ultimate_process_hold_reinit(UltimateProcessHold *hold);

G_END_DECLS

#endif /* _ULTIMATE_PROCESS_HOLD_H_ */

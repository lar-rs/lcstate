/**
 * @defgroup MktBusLibrary
 * @defgroup InjValve
 * @ingroup  InjValve
 * @{
 * @file  mkt-task_timer.h	TaskTimer object header
 * @brief   TaskTimer object header file.
 *
 * @copyright	 Copyright (C) LAR 2015
 *
 * @author       A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef _INJ_VALVE_H_
#define _INJ_VALVE_H_

#include <glib-object.h>
#include <mktbus.h>
#include <mktlib.h>

G_BEGIN_DECLS

#define INJ_TYPE_VALVE (inj_valve_get_type())
#define INJ_VALVE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), INJ_TYPE_VALVE, InjValve))
#define INJ_VALVE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), INJ_TYPE_VALVE, InjValveClass))
#define INJ_IS_VALVE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), INJ_TYPE_VALVE))
#define INJ_IS_VALVE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), INJ_TYPE_VALVE))
#define INJ_VALVE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), INJ_TYPE_VALVE, InjValveClass))

typedef struct _InjValveClass   InjValveClass;
typedef struct _InjValve        InjValve;
typedef struct _InjValvePrivate InjValvePrivate;

GType inj_valve_get_type(void) G_GNUC_CONST;

struct _InjValveClass {
    MktTaskObjectClass parent_class;
};

struct _InjValve {
    MktTaskObject    parent_instance;
    InjValvePrivate *priv;
};

G_END_DECLS

MktTaskObject *inj_valve_new(gboolean is_open);

MktTaskObject *injection_ptp_air(GScanner *scanner);

#endif /* _INJ_VALVE_H_ */

/** @} */

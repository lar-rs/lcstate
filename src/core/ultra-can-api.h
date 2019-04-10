/*
 * Copyright (C) LAR 2019
 *
 */

#ifndef _ULTRA_CAN_API_H_
#define _ULTRA_CAN_API_H_


#include <glib.h>
#include <gio/gio.h>
#include "can4linux.h"


typedef enum {
    CAN_ERROR_NONE,
    CAN_ERROR_OPEN,
    CAN_ERROR_READ,
    CAN_ERROR_WRITE,
}CanErrors;


gboolean can_check();


guint analog2_get_in01();
guint analog2_get_in02();
guint analog2_get_in03();
guint analog2_get_in04();
guint analog2_get_in05();
guint analog2_get_out();
void     analog2_set_out(gdouble value);


#endif

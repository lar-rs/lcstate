/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-control-data.c
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 *
mkt-control-data.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mkt-control-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __ULTRA_PROCESS_CONTROL_H_
#define __ULTRA_PROCESS_CONTROL_H_

#include "ultra-stream-object.h"
#include <mktbus.h>
#include <mktlib.h>
#include <ultimate-library.h>

G_BEGIN_DECLS

#define ULTRA_TYPE_CONTROL_PROCESS (ultra_control_process_get_type())
#define ULTRA_CONTROL_PROCESS(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_CONTROL_PROCESS, UltraControlProcess))
#define ULTRA_CONTROL_PROCESS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTRA_TYPE_CONTROL_PROCESS, UltraControlProcessClass))
#define ULTRA_IS_CONTROL_PROCESS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_CONTROL_PROCESS))
#define ULTRA_IS_CONTROL_PROCESS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTRA_TYPE_CONTROL_PROCESS))
#define ULTRA_CONTROL_PROCESS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTRA_TYPE_CONTROL_PROCESS, UltraControlProcessClass))

typedef struct _UltraControlProcessClass   UltraControlProcessClass;
typedef struct _UltraControlProcess        UltraControlProcess;
typedef struct _UltraControlProcessPrivate UltraControlProcessPrivate;

struct _UltraControlProcessClass {
    GObjectClass parent_class;
};

struct _UltraControlProcess {
    GObject                     parent_instance;
    UltraControlProcessPrivate *priv;
};

GType ultra_control_process_get_type(void) G_GNUC_CONST;

void ultra_control_new(); 
UltraControlProcess *ULTRA_CONTROL(void);
void control_status(const gchar *format, ...) G_GNUC_PRINTF(1, 2);

void ultra_control_init_stirrers();
GQuark control_error_quark(void);
gboolean control_add_process(MktProcessObject *process);
gboolean control_remove_process(MktProcessObject *process);
gboolean control_add_range1(MktProcessObject *process);
gboolean control_add_range2(MktProcessObject *process);


void control_was_rinsed(void);
void control_need_rinsing(VesselsObject *vessel);
void control_need_hold(void);
void control_need_nothig(void);
void contron_maintenance_off(void);
G_END_DECLS

#endif /* _ULTRA_HARDWARE_OBJECT_H_ */

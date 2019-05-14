/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-measurement-data.c
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 *
mkt-measurement-data.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mkt-measurement-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __ULTRA_PROCESS_SEQUENCE_H_
#define __ULTRA_PROCESS_SEQUENCE_H_

#include "sequence-process.h"
#include <mktbus.h>
#include <mktlib.h>
#include <ultimate-library.h>

G_BEGIN_DECLS

#define ULTRA_TYPE_SEQUENCE_PROCESS (ultra_sequence_process_get_type())
#define ULTRA_SEQUENCE_PROCESS(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_SEQUENCE_PROCESS, UltraSequenceProcess))
#define ULTRA_SEQUENCE_PROCESS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTRA_TYPE_SEQUENCE_PROCESS, UltraSequenceProcessClass))
#define ULTRA_IS_SEQUENCE_PROCESS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_SEQUENCE_PROCESS))
#define ULTRA_IS_SEQUENCE_PROCESS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTRA_TYPE_SEQUENCE_PROCESS))
#define ULTRA_SEQUENCE_PROCESS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTRA_TYPE_SEQUENCE_PROCESS, UltraSequenceProcessClass))

typedef struct _UltraSequenceProcessClass   UltraSequenceProcessClass;
typedef struct _UltraSequenceProcess        UltraSequenceProcess;
typedef struct _UltraSequenceProcessPrivate UltraSequenceProcessPrivate;

struct _UltraSequenceProcessClass {
    SequenceObjectSkeletonClass parent_class;
    void (*run)(UltraSequenceProcess *sequence_process, GTask *task);
    void (*cancel)(UltraSequenceProcess *sequence_process);

    gboolean (*stop)(UltraSequenceProcess *sequence_process);
};

struct _UltraSequenceProcess {
    SequenceObjectSkeleton       parent_instance;
    UltraSequenceProcessPrivate *priv;
};

GType ultra_sequence_process_get_type(void) G_GNUC_CONST;

#define HOLD_Y_AXIS_TASK "MoveY(1,1,1);SensorY();HoldY(1,1);"

void sequence_workers_process_change_status(SequenceWorkersProcess *process, const gchar *format, ...) G_GNUC_PRINTF(2, 3);
void sequence_workers_process_change_status_error(SequenceWorkersProcess *process, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

G_END_DECLS

#endif /* _ULTRA_HARDWARE_OBJECT_H_ */

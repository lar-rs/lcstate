/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * lar-mongo-database.h
 * Copyright (C) 2017 sascha.smolkov <saschasmolkov@saschasmolkov>
 *
 * lar-mongo-database.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * lar-mongo-database.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LAR_TIMER_H_
#define _LAR_TIMER_H_

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
G_BEGIN_DECLS

#define LAR_TYPE_TIMER (lar_timer_get_type())
#define LAR_TIMER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), LAR_TYPE_TIMER, LarTimer))
#define LAR_TIMER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), LAR_TYPE_TIMER, LarTimerClass))
#define LAR_IS_TIMER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), LAR_TYPE_TIMER))
#define LAR_IS_TIMER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), LAR_TYPE_TIMER))
#define LAR_TIMER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), LAR_TYPE_TIMER, LarTimerClass))

typedef struct _LarTimerClass   LarTimerClass;
typedef struct _LarTimer        LarTimer;
typedef struct _LarTimerPrivate LarTimerPrivate;

struct _LarTimerClass {
    GObjectClass parent_class;
};

struct _LarTimer {
    GObject parent_instance;

    LarTimerPrivate *priv;
};

GType lar_timer_get_type(void) G_GNUC_CONST;

gboolean lar_timer_default_run(GCancellable *cancellable, GAsyncReadyCallback callback, gdouble seconds, gpointer user_data);
gboolean lar_timer_default_finish(GAsyncResult *result, GError **error);
LarTimer *lar_timer_new(gdouble seconds);
gboolean lar_timer_run(LarTimer *timer, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);

G_END_DECLS

#endif /* _LAR_TIMER_H_ */

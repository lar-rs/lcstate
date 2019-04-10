/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * src
 * Copyright (C) sascha 2011 <sascha@sascha-desktop>
 * 
src is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * src is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GL_XKBD_H_
#define _GL_XKBD_H_

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GL_TYPE_XKBD             (gl_xkbd_get_type ())
#define GL_XKBD(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_XKBD, GlXKbd))
#define GL_XKBD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_XKBD, GlXKbdClass))
#define GL_IS_XKBD(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_XKBD))
#define GL_IS_XKBD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_XKBD))
#define GL_XKBD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_XKBD, GlXKbdClass))

typedef struct _GlXKbdClass      GlXKbdClass;
typedef struct _GlXKbd           GlXKbd;
typedef struct _GlXKbdPrivate    GlXKbdPrivate;


enum
{
	GL_XKBD_TYPE_COMPACT,
	GL_XKBD_TYPE_KEYPAD
};

struct _GlXKbdClass
{
	GtkButtonClass      parent_class;
	void              (*start)       ( GlXKbd *xkbd );
	void              (*stop)        ( GlXKbd *xkbd );
};

struct _GlXKbd
{
	GtkButton           parent_instance;
	GlXKbdPrivate      *priv;
};

GType            gl_xkbd_get_type (void) G_GNUC_CONST;

void             gl_xkbd_load          ( );
GtkWidget*       gl_xkbd_get_widget    ( );
GlXKbd*          gl_xkbd_get_keyboard  ( );

//void             gl_xkbd_set_compact_params ( GlXKbd *xkbd , gchar* strarr,...);
//void             gl_xkbd_set_keypad_params  ( GlXKbd *xkbd , gchar* strarr,... );
void             gl_xkbd_set_set_type        ( gint type );
gint             gl_xkbd_get_key_type        ( );
void             gl_xkbd_need_keyboard      ( GtkWidget *widget , GtkWidget *focus , gint type );
void             gl_xkbd_ref_user           (  );
void             gl_xkbd_unref_user         (  );
void             gl_xkbd_set_winid          ( const gchar* winId ,GtkWidget *child );
void             gl_xkbd_set_winid_for_main (  );

gboolean         gl_xkbd_is_run              (  );
gboolean         gl_xkbd_run                 ( gboolean stoppen );
gboolean         gl_xkbd_start               (  );
gboolean         gl_xkbd_stop                (  );
gboolean         gl_xkbd_restart             (  );

G_END_DECLS

#endif /* _GL_XKBD_H_ */

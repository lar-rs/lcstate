/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gui-process-new
 * Copyright (C) sascha 2011 <sascha@sascha-desktop>
 * 
gui-process-new is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gui-process-new is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GL_INDICATE_H_
#define _GL_INDICATE_H_

#include <gtk/gtk.h>
#include <glib.h>
#include "mkt-atom.h"

G_BEGIN_DECLS

#define GL_TYPE_INDICATE             (gl_indicate_get_type ())
#define GL_INDICATE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_INDICATE, GlIndicate))
#define GL_INDICATE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_INDICATE, GlIndicateClass))
#define GL_IS_INDICATE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_INDICATE))
#define GL_IS_INDICATE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_INDICATE))
#define GL_INDICATE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_INDICATE, GlIndicateClass))

typedef struct _GlIndicateClass      GlIndicateClass;
typedef struct _GlIndicate           GlIndicate;
typedef struct _GlIndicatePrivate    GlIndicatePrivate;



#define  GL_INDICATE_IS_RUN(indicate) ( (indicate != NULL && GL_IS_INDICATE(indicate))?indicate->isStart:FALSE)


enum {
	GL_INDICATE_NO_ASK,
	GL_INDICATE_ASK_INFO,
	GL_INDICATE_ASK_CONFIRM
};

struct _GlIndicateClass
{
	MktAtomClass          parent_class;

	void                (*click_yes)      (GlIndicate *indicate);
	void                (*click_no )      (GlIndicate *indicate);
	void                (*click_start )   (GlIndicate *indicate);
	void                (*stop_indicate)  (GlIndicate *indicate);
	void                (*start_indicate) (GlIndicate *indicate);



};

struct _GlIndicate
{
	MktAtom               parent_instance;
	GlIndicatePrivate    *priv;
	GtkWidget            *ind_box;
	GtkWidget            *ind_image;
	GtkWidget            *ind_window;
	gboolean              isStart;

};

GType              gl_indicate_get_type ( void ) G_GNUC_CONST;

GlIndicate*        gl_indicate_new      ( const gchar *id , guint user_ask );

void               gl_indicate_start    ( GlIndicate *indicate );
void               gl_indicate_stop     ( GlIndicate *indicate );

void               gl_indicate_update   ( GlIndicate *indicate );

void               gl_indicate_set_indicate_profile (GlIndicate *indicate , gchar     *images );
void               gl_indicate_set_indicate_box     (GlIndicate *indicate , GtkWidget *widget );
void               gl_indicate_set_indicate_window  (GlIndicate *indicate , GtkWidget *widget );


gboolean           gl_indicate_is_user_info ( GlIndicate *indicate );

G_END_DECLS

#endif /* _GL_INDICATE_H_ */

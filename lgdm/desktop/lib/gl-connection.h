/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-item-manager.c
 * Copyright (C) A.Smolkov 2011 <asmolkov@lar.com>
 * 
 * gl-item-manager.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-item-manager.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GL_CONNECTION_H_
#define _GL_CONNECTION_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include "mkt-atom.h"
#include "gl-translation.h"
#include "mkt-model.h"
#include "mkt-item.h"
#include "mkt-item-object.h"
#include "mkt-param.h"

G_BEGIN_DECLS


#define MKT_TYPE_FIFO                           (mkt_fifo_get_type ())
#define MKT_FIFO (obj)                          (G_TYPE_CHECK_INSTANCE_CAST ((obj),     MKT_TYPE_FIFO, MktFifo))
#define MKT_IS_FIFO(obj)                        (G_TYPE_CHECK_INSTANCE_TYPE ((obj),     MKT_TYPE_FIFO))
#define MKT_FIFO_GET_INTERFACE(inst)            (G_TYPE_INSTANCE_GET_INTERFACE ((inst), MKT_TYPE_FIFO, MktFifoInterface))


typedef struct _MktFifo             MktFifo; /* dummy object */
typedef struct _MktFifoInterface    MktFifoInterface;

struct _MktFifoInterface
{
	GTypeInterface  parent_iface;
	gboolean        (*transmit)         (MktFifo *fifo , const gchar *prop);
};

GType                            mkt_fifo_get_type                              ( void );
gboolean                         mkt_fifo_transmit                              ( MktFifo *fifo , const gchar *prop );




#define GL_TYPE_BINDING                (gl_binding_get_type ())
#define GL_BINDING(obj)                (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_BINDING,  GlBinding))
#define GL_BINDING_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST ((klass),  GL_TYPE_BINDING,  GlBindingClass))
#define GL_IS_BINDING(obj)             (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_BINDING))
#define GL_IS_BINDING_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass),  GL_TYPE_BINDING))
#define GL_BINDING_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj),  GL_TYPE_BINDING,  GlBindingClass))


typedef struct _GlBindingClass         GlBindingClass;
typedef struct _GlBinding              GlBinding;
typedef struct _GlBindingPrivate       GlBindingPrivate;

struct _GlBindingClass
{
	GObjectClass             parent_class;
	gboolean               ( *realize  )                 ( GlBinding *binding ,GObject *object );
	gboolean               ( *incoming_item  )           ( GlBinding *binding );
	gboolean               ( *destroy_binding  )         ( GlBinding *binding );


};

struct _GlBinding
{
	GObject                  parent_instance;
	GlBindingPrivate        *priv;
};

#define GL_TYPE_CONNECTION             (gl_connection_get_type ())
#define GL_CONNECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_CONNECTION, GlConnection))
#define GL_CONNECTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_CONNECTION,  GlConnectionClass))
#define GL_IS_CONNECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_CONNECTION))
#define GL_IS_CONNECTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_CONNECTION))
#define GL_CONNECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_CONNECTION, GlConnectionClass))

typedef struct _GlConnectionClass      GlConnectionClass;
typedef struct _GlConnection           GlConnection;
typedef struct _GlConnectionPrivate    GlConnectionPrivate;


GType                      gl_binding_get_type                     ( void) G_GNUC_CONST;



GlBinding*                 gl_binding_new                          ( const gchar *name );

const gchar*               gl_binding_get_name                     ( GlBinding *binding);

gboolean                   gl_binding_add_translation              ( GlBinding *binding ,GlTranslation *translation );
GlBinding*                 gl_binding_insert_widget                ( GtkWidget *widget  );
GlBinding*                 gl_binding_insert_widget_full           ( GtkWidget *widget , guint flag );
GlBinding*                 gl_binding_insert_object                ( GObject *object );
GlBinding*                 gl_binding_insert_object_full           ( GObject *object , guint flag );


void                       gl_binding_transmit_value_name          ( const gchar *id ,  GValue *value );
void                       gl_binding_transmit_adrressed_value     ( GlBinding *binding , GValue *value );
void                       gl_binding_set_flag                     ( GlBinding *binding , guint flag );
void                       gl_connection_create_indicate           ( GlConnection *connection );


#define GL_CONTROL_BOX_PLACE_NAME "control_box"


enum
{
	GL_CONNECTION_WIDGET_INPUT  = 1 << 1,
	GL_CONNECTION_WIDGET_OUTPUT = 1 << 2,
	GL_CONNECTION_WIDGET_NOSAVE = 1 << 3,
};


enum
{
	GL_CONNECTION_TYPE_ITEM,
	GL_CONNECTION_TYPE_MODEL
};

struct _GlConnectionClass
{
	MktAtomClass             parent_class;

	void                   ( *must_saved     )                 (GlConnection *connection);
	void                   ( *need_keyboard  )                 (GlConnection *connection);

};

struct _GlConnection
{
	MktAtom                 parent_instance;
	GlConnectionPrivate    *priv;
	GList                  *save_items;

};

GType                    gl_connection_get_type               ( void) G_GNUC_CONST;

GlConnection*            gl_connection_market_get             ( );
GlBinding*               gl_connection_get_binding            ( const gchar *name  );
gboolean                 gl_connection_add_binding            ( GlBinding *binding );
gboolean                 gl_connection_connect_binding_signal ( const gchar *name , GCallback callback , gpointer user_data );
GlConnection*            gl_connection_new                    ( const gchar *id,const gchar *rxPaht ,const  gchar *txPath);
gboolean                 gl_connection_open                   ( GlConnection *connection );
gboolean                 gl_connection_run                    ( GlConnection *connection );
gboolean                 gl_connection_change_value           ( GlConnection *connection , GlBinding *binding );
gboolean                 gl_connection_is_signal_block        ( GlConnection *connection );
void                     gl_connection_set_signal_block       ( GlConnection *connection , gboolean block );
void                     gl_connection_add_model_container    ( MktModel *model , GtkWidget *widget );

G_END_DECLS

#endif /* _GL_CONNECTION_H_ */

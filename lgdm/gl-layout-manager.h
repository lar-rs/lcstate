/**
 * file  gl-desktop-action.h object header
 *
 * Copyright (C) LAR 2013-2019
 *
 * author A.Smolkov <asmolkov@lar.com>
 *
 */

#ifndef GL_LARLAYOUT_MANAGER_H_
#define GL_LARLAYOUT_MANAGER_H_
#include <gtk/gtk.h>
#include <glib.h>

#include "gl-layout.h"
#include "lgdm-desktop-generated-code.h"

G_BEGIN_DECLS


#define GL_TYPE_LAYOUT_MANAGER    			     (gl_layout_manager_get_type())
#define GL_LAYOUT_MANAGER(obj)			         (G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_LAYOUT_MANAGER, GlLayoutManager))
#define GL_LAYOUT_MANAGER_CLASS(klass)		     (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_LAYOUT_MANAGER, GlLayoutManagerClass))
#define GL_IS_LAYOUT_MANAGER(obj)		         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_LAYOUT_MANAGER))
#define GL_IS_LAYOUT_MANAGER_CLASS(klass)        (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_LAYOUT_MANAGER))

typedef struct _GlLayoutManager			          GlLayoutManager;
typedef struct _GlLayoutManagerClass		      GlLayoutManagerClass;
typedef struct _GlLayoutManagerPrivate            GlLayoutManagerPrivate;

struct _GlLayoutManagerClass
{
	GtkBoxClass                      parent_class;
	void                              (*action_start)       ( GlLayoutManager *action );
};

struct _GlLayoutManager
{
	GtkBox                              action;
	GlLayoutManagerPrivate             *priv;
};


GType 		         gl_layout_manager_get_type                        ( void );
GlLayoutManager*     gl_layout_manager_new                             ( );
gboolean             gl_layout_manager_default_add_layout              ( GlLayout *lyaout );
gboolean             gl_layout_manager_default_activate_layout         ( GlLayout *layout );
gboolean             gl_layout_manager_default_activate_named          ( const gchar* id  );
gboolean             gl_layout_manager_default_activate_root           (  );

GlLayoutManager*     gl_layout_manager_get_default                     ( );
void                 gl_layout_manager_default_change_level            ( guint levvel );
GlLayout*            gl_layout_manager_get_visible_layout              ( );

G_END_DECLS
#endif

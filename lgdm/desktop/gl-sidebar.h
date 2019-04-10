/**
 * @defgroup LgdmLibrary
 * @defgroup GlSidebar
 * @ingroup  GlSidebar
 * @{
 * @file  gl-sidebar.h object header
 * @brief This is LGDM side bar object header file.
 *
 * 	 Copyright (C) LAR 2013
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef GL_LARSIDEBAR_H_
#define GL_LARSIDEBAR_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_SIDEBAR    			    (gl_sidebar_get_type())
#define GL_SIDEBAR(obj)			     	(G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_SIDEBAR, GlSidebar))
#define GL_SIDEBAR_CLASS(klass)		    (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_SIDEBAR, GlSidebarClass))
#define GL_IS_SIDEBAR(obj)		        (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_SIDEBAR))
#define GL_IS_SIDEBAR_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_SIDEBAR))

typedef struct _GlSidebar			        GlSidebar;
typedef struct _GlSidebarClass		        GlSidebarClass;
typedef struct _GlSidebarPrivate            GlSidebarPrivate;




struct _GlSidebar
{
	GtkBox                sidebar;
	GlSidebarPrivate      *priv;
};

struct _GlSidebarClass
{
	GtkBoxClass         parent_class;

	void                  (*online  )                    ( GlSidebar *bar );
	void                  (*offline )                    ( GlSidebar *bar );
	void                  (*larkey  )                    ( GlSidebar *bar );
	void                  (*screenshot )                 ( GlSidebar *bar );
};

GType 		         gl_sidebar_get_type                 ( void );
GtkWidget*  	     gl_sidebar_new                      ( );

gboolean             gl_sidebar_online_sensetive         ( GlSidebar *bar , gboolean sesetive );
gboolean             gl_sidebar_offline_sensetive        ( GlSidebar *bar , gboolean sesetive );
gboolean             gl_sidebar_larkey_sensetive         ( GlSidebar *bar , gboolean sesetive );
gboolean             gl_sidebar_screenshot_sensetive     ( GlSidebar *bar , gboolean sesetive );
void                 gl_sidebar_screenshot_processed     ( GlSidebar *bar , gboolean processed);




//---------------------------info ------------------------------------------------------------------


//-------------------------------progress bar control ----------------------------------------------


G_END_DECLS
#endif /* GL_LARSIDEBAR_H_ */


/** @} */

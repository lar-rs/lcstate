/**
 * @defgroup LgdmLibrary
 * @defgroup LgdmSidebar
 * @ingroup  LgdmSidebar
 * @{
 * @file  lgdm-sidebar.h object header
 * @brief This is LGDM side bar object header file.
 *
 * 	 Copyright (C) LAR 2013
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef LGDM_LARSIDEBAR_H_
#define LGDM_LARSIDEBAR_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define LGDM_TYPE_SIDEBAR    			    (lgdm_sidebar_get_type())
#define LGDM_SIDEBAR(obj)			     	(G_TYPE_CHECK_INSTANCE_CAST((obj),LGDM_TYPE_SIDEBAR, LgdmSidebar))
#define LGDM_SIDEBAR_CLASS(klass)		    (G_TYPE_CHECK_CLASS_CAST((klass) ,LGDM_TYPE_SIDEBAR, LgdmSidebarClass))
#define LGDM_IS_SIDEBAR(obj)		        (G_TYPE_CHECK_INSTANCE_TYPE((obj),LGDM_TYPE_SIDEBAR))
#define LGDM_IS_SIDEBAR_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE((klass) ,LGDM_TYPE_SIDEBAR))

typedef struct _LgdmSidebar			        LgdmSidebar;
typedef struct _LgdmSidebarClass		        LgdmSidebarClass;
typedef struct _LgdmSidebarPrivate            LgdmSidebarPrivate;




struct _LgdmSidebar
{
	GtkBox                sidebar;
	LgdmSidebarPrivate      *priv;
};

struct _LgdmSidebarClass
{
	GtkBoxClass         parent_class;

	void                  (*online  )                    ( LgdmSidebar *bar );
	void                  (*offline )                    ( LgdmSidebar *bar );
	void                  (*larkey  )                    ( LgdmSidebar *bar );
	void                  (*screenshot )                 ( LgdmSidebar *bar );
};

GType 		         lgdm_sidebar_get_type                 ( void );
GtkWidget*  	     lgdm_sidebar_new                      ( );

gboolean             lgdm_sidebar_online_sensetive         ( LgdmSidebar *bar , gboolean sesetive );
gboolean             lgdm_sidebar_offline_sensetive        ( LgdmSidebar *bar , gboolean sesetive );
gboolean             lgdm_sidebar_larkey_sensetive         ( LgdmSidebar *bar , gboolean sesetive );
gboolean             lgdm_sidebar_screenshot_sensetive     ( LgdmSidebar *bar , gboolean sesetive );
void                 lgdm_sidebar_screenshot_processed     ( LgdmSidebar *bar , gboolean processed);




//---------------------------info ------------------------------------------------------------------


//-------------------------------progress bar control ----------------------------------------------


G_END_DECLS
#endif /* LGDM_LARSIDEBAR_H_ */


/** @} */

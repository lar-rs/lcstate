/*
 * gtklarstatus.h
 *
 *  Created on: 24.05.2011
 *      Author: asmolkov
 */

#ifndef GL_LARSTATUS_H_
#define GL_LARSTATUS_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define GL_TYPE_STATUS    			    (gl_status_get_type())
#define GL_STATUS(obj)			     	(G_TYPE_CHECK_INSTANCE_CAST((obj),GL_TYPE_STATUS, GlStatus))
#define GL_STATUS_CLASS(klass)		    (G_TYPE_CHECK_CLASS_CAST((klass) ,GL_TYPE_STATUS, GlStatusClass))
#define GL_IS_STATUS(obj)		        (G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_TYPE_STATUS))
#define GL_IS_STATUS_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_TYPE_STATUS))

typedef struct _GlStatus			        GlStatus;
typedef struct _GlStatusClass		        GlStatusClass;
typedef struct _GlStatusPrivate             GlStatusPrivate;




struct _GlStatus
{
	GtkBox                status;
	GlStatusPrivate      *priv;
};

struct _GlStatusClass
{
	GtkBoxClass          parent_class;
	void                (*show_desktop )       ( GlStatus *larstatus);
};

GType 		     gl_status_get_type                ( void);
GlStatus*  	     gl_status_new                     ( );

void             gl_status_set_application_name    ( GlStatus *status , const gchar *application_name);


void             gl_status_add_action              ( GlStatus *desktop , GtkWidget *action );



//---------------------------info ------------------------------------------------------------------


//-------------------------------progress bar control ----------------------------------------------


G_END_DECLS
#endif /* GL_LARSTATUS_H_ */

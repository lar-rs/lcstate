/*
 * gtklarstatus.h
 *
 *  Created on: 24.05.2011
 *      Author: asmolkov
 */

#ifndef LGDM_LARSTATUS_H_
#define LGDM_LARSTATUS_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define LGDM_TYPE_STATUS    			    (lgdm_status_get_type())
#define LGDM_STATUS(obj)			     	(G_TYPE_CHECK_INSTANCE_CAST((obj),LGDM_TYPE_STATUS, LgdmStatus))
#define LGDM_STATUS_CLASS(klass)		    (G_TYPE_CHECK_CLASS_CAST((klass) ,LGDM_TYPE_STATUS, LgdmStatusClass))
#define LGDM_IS_STATUS(obj)		        (G_TYPE_CHECK_INSTANCE_TYPE((obj),LGDM_TYPE_STATUS))
#define LGDM_IS_STATUS_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass) ,LGDM_TYPE_STATUS))

typedef struct _LgdmStatus			        LgdmStatus;
typedef struct _LgdmStatusClass		        LgdmStatusClass;
typedef struct _LgdmStatusPrivate             LgdmStatusPrivate;




struct _LgdmStatus
{
	GtkBox                status;
	LgdmStatusPrivate      *priv;
};

struct _LgdmStatusClass
{
	GtkBoxClass          parent_class;
	void                (*show_desktop )       ( LgdmStatus *larstatus);
};

GType 		     lgdm_status_get_type                ( void);
LgdmStatus*  	     lgdm_status_new                     ( );

void             lgdm_status_set_application_name    ( LgdmStatus *status , const gchar *application_name);


void             lgdm_status_add_action              ( LgdmStatus *desktop , GtkWidget *action );



//---------------------------info ------------------------------------------------------------------


//-------------------------------progress bar control ----------------------------------------------


G_END_DECLS
#endif /* LGDM_LARSTATUS_H_ */

/**
 * @defgroup LgdmLibrary
 * @defgroup RowService
 * @ingroup  RowService
 * @{
 * @file  row-channel-info.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef ROW_SERVICE_H_
#define ROW_SERVICE_H_
#include <gtk/gtk.h>
#include <glib.h>
#include <gl-layout.h>


G_BEGIN_DECLS


#define ROW_TYPE_SERVICE    			           (row_service_get_type())
#define ROW_SERVICE(obj)			               (G_TYPE_CHECK_INSTANCE_CAST((obj),ROW_TYPE_SERVICE, RowService))
#define ROW_SERVICE_CLASS(klass)		           (G_TYPE_CHECK_CLASS_CAST((klass) ,ROW_TYPE_SERVICE, RowServiceClass))
#define ROW_IS_SERVICE(obj)		                   (G_TYPE_CHECK_INSTANCE_TYPE((obj),ROW_TYPE_SERVICE))
#define ROW_IS_SERVICE_CLASS(klass)                (G_TYPE_CHECK_CLASS_TYPE((klass) ,ROW_TYPE_SERVICE))
#define ROW_SERVICE_GET_CLASS(obj)                 (G_TYPE_INSTANCE_GET_CLASS ((obj),  ROW_TYPE_SERVICE, RowServiceClass))

typedef struct _RowService			        RowService;
typedef struct _RowServiceClass		        RowServiceClass;
typedef struct _RowServicePrivate             RowServicePrivate;

struct _RowServiceClass
{
	GtkListBoxRowClass                           parent_class;

	void                                        (*run)         ( RowService *service );
	void                                        (*clicked)     ( RowService *service );
};

struct _RowService
{
	GtkListBoxRow                                parent;
	RowServicePrivate                           *priv;
};


GType 		         row_service_get_type                ( void );



void                 row_service_wait_service            ( RowService *row );
void                 row_service_run_spinner             ( RowService *row );
void                 row_service_error                   ( RowService *row );
void                 row_service_done                    ( RowService *row );

gboolean             row_service_get_done                ( RowService *row );
const gchar*         row_service_get_name                ( RowService *row );
const gchar*         row_service_get_id                  ( RowService *row );

G_END_DECLS
#endif /* ROW_SERVICE_H_ */

/** @} */

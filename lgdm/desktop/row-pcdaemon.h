/**
 * @defgroup LgdmLibrary
 * @defgroup RowPcdaemon
 * @ingroup  RowPcdaemon
 * @{
 * @file  row-channel-info.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef ROW_PCDAEMON_H_
#define ROW_PCDAEMON_H_
#include <gtk/gtk.h>
#include <glib.h>
#include <gl-layout.h>
#include "row-service.h"


G_BEGIN_DECLS


#define ROW_TYPE_PCDAEMON    			           (row_pcdaemon_get_type())
#define ROW_PCDAEMON(obj)			               (G_TYPE_CHECK_INSTANCE_CAST((obj),ROW_TYPE_PCDAEMON, RowPcdaemon))
#define ROW_PCDAEMON_CLASS(klass)		           (G_TYPE_CHECK_CLASS_CAST((klass) ,ROW_TYPE_PCDAEMON, RowPcdaemonClass))
#define ROW_IS_PCDAEMON(obj)		               (G_TYPE_CHECK_INSTANCE_TYPE((obj),ROW_TYPE_PCDAEMON))
#define ROW_IS_PCDAEMON_CLASS(klass)               (G_TYPE_CHECK_CLASS_TYPE((klass) ,ROW_TYPE_PCDAEMON))

typedef struct _RowPcdaemon			        RowPcdaemon;
typedef struct _RowPcdaemonClass		        RowPcdaemonClass;
typedef struct _RowPcdaemonPrivate             RowPcdaemonPrivate;

struct _RowPcdaemonClass
{
	RowServiceClass                           parent_class;
};

struct _RowPcdaemon
{
	RowService                                 parent;
	RowPcdaemonPrivate                       *priv;
};


GType 		         row_pcdaemon_get_type                ( void );



void                 row_pcdaemon_wait_service            ( RowPcdaemon *row );

G_END_DECLS
#endif /* ROW_PCDAEMON_H_ */

/** @} */

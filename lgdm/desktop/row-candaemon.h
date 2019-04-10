/**
 * @defgroup LgdmLibrary
 * @defgroup RowCandaemon
 * @ingroup  RowCandaemon
 * @{
 * @file  row-channel-info.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef ROW_CANDAEMON_H_
#define ROW_CANDAEMON_H_
#include <gtk/gtk.h>
#include <glib.h>
#include <gl-layout.h>
#include "row-service.h"


G_BEGIN_DECLS


#define ROW_TYPE_CANDAEMON    			           (row_candaemon_get_type())
#define ROW_CANDAEMON(obj)			               (G_TYPE_CHECK_INSTANCE_CAST((obj),ROW_TYPE_CANDAEMON, RowCandaemon))
#define ROW_CANDAEMON_CLASS(klass)		           (G_TYPE_CHECK_CLASS_CAST((klass) ,ROW_TYPE_CANDAEMON, RowCandaemonClass))
#define ROW_IS_CANDAEMON(obj)		               (G_TYPE_CHECK_INSTANCE_TYPE((obj),ROW_TYPE_CANDAEMON))
#define ROW_IS_CANDAEMON_CLASS(klass)               (G_TYPE_CHECK_CLASS_TYPE((klass) ,ROW_TYPE_CANDAEMON))

typedef struct _RowCandaemon			        RowCandaemon;
typedef struct _RowCandaemonClass		        RowCandaemonClass;
typedef struct _RowCandaemonPrivate             RowCandaemonPrivate;

struct _RowCandaemonClass
{
	RowServiceClass                           parent_class;
};

struct _RowCandaemon
{
	RowService                                 parent;
	RowCandaemonPrivate                       *priv;
};


GType 		         row_candaemon_get_type                ( void );



void                 row_candaemon_wait_service            ( RowCandaemon *row );

G_END_DECLS
#endif /* ROW_CANDAEMON_H_ */

/** @} */

/**
 * @defgroup LgdmLibrary
 * @defgroup RowFakeUser
 * @ingroup  RowFakeUser
 * @{
 * @file  row-channel-info.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef ROW_FAKE_USER_H_
#define ROW_FAKE_USER_H_
#include <gtk/gtk.h>
#include <glib.h>
#include <gl-layout.h>


G_BEGIN_DECLS


#define ROW_TYPE_FAKE_USER    			           (row_fake_user_get_type())
#define ROW_FAKE_USER(obj)			               (G_TYPE_CHECK_INSTANCE_CAST((obj),ROW_TYPE_FAKE_USER, RowFakeUser))
#define ROW_FAKE_USER_CLASS(klass)		           (G_TYPE_CHECK_CLASS_CAST((klass) ,ROW_TYPE_FAKE_USER, RowFakeUserClass))
#define ROW_IS_FAKE_USER(obj)		               (G_TYPE_CHECK_INSTANCE_TYPE((obj),ROW_TYPE_FAKE_USER))
#define ROW_IS_FAKE_USER_CLASS(klass)               (G_TYPE_CHECK_CLASS_TYPE((klass) ,ROW_TYPE_FAKE_USER))

typedef struct _RowFakeUser			               RowFakeUser;
typedef struct _RowFakeUserClass		           RowFakeUserClass;
typedef struct _RowFakeUserPrivate                 RowFakeUserPrivate;

struct _RowFakeUserClass
{
	GtkListBoxRowClass                         parent_class;
};

struct _RowFakeUser
{
	GtkListBoxRow                              parent;
	RowFakeUserPrivate                            *priv;
};


GType 		         row_fake_user_get_type                ( void );



gboolean             row_fake_user_login                   ( RowFakeUser *row );


G_END_DECLS
#endif /* ROW_FAKE_USER_H_ */

/** @} */

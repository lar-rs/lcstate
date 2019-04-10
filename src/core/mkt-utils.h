/*
 * mkt-utils.h
 *
 *  Created on: 13.08.2013
 *      Author: sascha
 */

#ifndef MKT_UTILS_H_
#define MKT_UTILS_H_


#include "mkt-value.h"
#include <glib.h>
#include <gio/gio.h>


#define	MKT_UTIL_VALUE_COLLECT_FORMAT_MAX_LENGTH	(8)

enum	/*< skip >*/
{
  MKT_VALUE_COLLECT_INT		= 'i',
  MKT_VALUE_COLLECT_LONG	= 'l',
  MKT_VALUE_COLLECT_INT64   = 'q',
  MKT_VALUE_COLLECT_DOUBLE	= 'd',
  MKT_VALUE_COLLECT_POINTER	= 'p'
};


#define MKT_UTIL_VALUE_COLLECT_INIT(value, _value_type, var_args, flags, __error)		\
G_STMT_START {										\
  GValue *_val = (value);								\
  guint _flags = (flags);								\
  GTypeValueTable *_vtab = g_type_value_table_peek (_value_type);			\
  const gchar *_collect_format = _vtab->collect_format;					\
  GTypeCValue _cvalues[MKT_UTIL_VALUE_COLLECT_FORMAT_MAX_LENGTH] = { { 0, }, };		\
  guint _n_values = 0;									\
                                                                                        \
  _val->g_type = _value_type;		/* value_meminit() from gvalue.c */		\
  while (*_collect_format)								\
    {											\
      GTypeCValue *_cvalue = _cvalues + _n_values++;					\
                                                                                        \
      switch (*_collect_format++)							\
	{										\
	case MKT_VALUE_COLLECT_INT:							\
	  _cvalue->v_int = va_arg ((var_args), gint);					\
	  break;									\
	case MKT_VALUE_COLLECT_LONG:							\
	  _cvalue->v_long = va_arg ((var_args), glong);					\
	  break;									\
	case MKT_VALUE_COLLECT_INT64:							\
	  _cvalue->v_int64 = va_arg ((var_args), gint64);				\
	  break;									\
	case MKT_VALUE_COLLECT_DOUBLE:							\
	  _cvalue->v_double = va_arg ((var_args), gdouble);				\
	  break;									\
	case MKT_VALUE_COLLECT_POINTER:							\
	  _cvalue->v_pointer = va_arg ((var_args), gpointer);				\
	  break;									\
	default:									\
	  g_assert_not_reached ();							\
	}										\
    }											\
  *(__error) = _vtab->collect_value (_val,						\
				       _n_values,					\
				       _cvalues,					\
				       _flags);						\
} G_STMT_END



gboolean             mkt_is_dir                                  ( const gchar* dir_name );
gboolean             mkt_make_dir                                ( const gchar* dir_name );
gboolean             mkt_keyfile_check                           ( const gchar *path , const gchar *group , const gchar *key , const GValue *check_value );
GValue*              mkt_keyfile_get                             ( const gchar *path , const gchar *group , const gchar *key , GType type );

gchar*               mkt_utils_dup_dbus_path                     ( const gchar *object_name );
gchar*               mkt_utils_dup_dbus_name                     ( const gchar *object_path );



gboolean             mkt_utils_settings_check_schema             ( const gchar *schema_id );
gboolean             mkt_utils_settings_check_key                ( GSettings *settings, const gchar *key );
gboolean             mkt_utils_settings_check_child              ( GSettings *settings, const gchar *child );
GSettings*           mkt_utils_settings_get_child                ( GSettings *settings, const gchar *child );


// GSettings convert functions
gboolean             mkt_utils_settings_mapping_get_uint32_to_string ( GValue  *value, GVariant  *variant, gpointer user_data);
gboolean             mkt_utils_settings_mapping_get_int32_to_string  ( GValue  *value, GVariant  *variant, gpointer user_data);
gboolean             mkt_utils_settings_mapping_get_double_to_string ( GValue  *value, GVariant  *variant, gpointer user_data);

gboolean             mkt_utils_settings_mapping_get_uint32_to_double ( GValue  *value, GVariant  *variant, gpointer user_data);
gboolean             mkt_utils_settings_mapping_get_double_to_uint   ( GValue  *value, GVariant  *variant, gpointer user_data);
GVariant*            mkt_utils_settings_mapping_set_uint_to_double   ( const GValue *value,const GVariantType *expected_type,gpointer user_data);
GVariant*            mkt_utils_settings_mapping_set_double_to_uint32 ( const GValue *value,const GVariantType *expected_type,gpointer user_data);

gboolean             mkt_utils_settings_mapping_get_int32_to_double  ( GValue  *value, GVariant  *variant, gpointer user_data);
gboolean             mkt_utils_settings_mapping_get_double_to_int    ( GValue  *value, GVariant  *variant, gpointer user_data);
GVariant*            mkt_utils_settings_mapping_set_int_to_double    ( const GValue *value,const GVariantType *expected_type,gpointer user_data);
GVariant*            mkt_utils_settings_mapping_set_double_to_int32  ( const GValue *value,const GVariantType *expected_type,gpointer user_data);

gboolean             mkt_utils_transform_binding_all_to_string       ( GBinding     *binding, const GValue *from_value,GValue *to_value, gpointer  user_data);
gboolean             mkt_utils_transform_binding_all_from_string     ( GBinding     *binding, const GValue *from_value,GValue *to_value, gpointer  user_data);

gboolean             mkt_utils_transform_binding_double_to_string    ( GBinding     *binding, const GValue *from_value,GValue *to_value, gpointer  user_data);



gboolean             mkt_utils_transform_binding_double_to_uint      ( GBinding     *binding, const GValue *from_value,GValue *to_value, gpointer  user_data);
gboolean             mkt_utils_transform_binding_uint_to_double      ( GBinding     *binding, const GValue *from_value,GValue *to_value, gpointer  user_data);
gboolean             mkt_utils_transform_binding_double_to_concentration      ( GBinding     *binding, const GValue *from_value,GValue *to_value, gpointer  user_data);


#endif /* MKT_UTILS_H_ */

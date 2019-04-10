/*
 * mkt-utils.c
 *
 *  Created on: 13.08.2013
 *      Author: sascha
 */


#include "mkt-utils.h"
#include "mkt-value.h"
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <malloc.h>
#include <gobject/gvaluecollector.h>


gboolean
mkt_is_dir (const gchar* dirName)
{
	struct stat fileInfo;
	return ! stat (dirName, &fileInfo) && S_ISDIR (fileInfo.st_mode);
}

gboolean
mkt_make_dir (const gchar* dirName)
{
	char* lastSlash;

	if (mkt_is_dir (dirName))
		return TRUE;

	lastSlash = strrchr (dirName, '/');

	if (lastSlash && dirName!=lastSlash) {
		gboolean success;

		* lastSlash = 0;
		success     = mkt_make_dir (dirName);
		* lastSlash = '/';

		if (! success)
			return FALSE;
	}
	return ! mkdir (dirName, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
}


gboolean
mkt_keyfile_check   ( const gchar *path , const gchar *group , const gchar *key , const GValue *check_value )
{
	g_return_val_if_fail(check_value != NULL,FALSE);
	g_return_val_if_fail(path != NULL,FALSE);
	g_return_val_if_fail(group != NULL,FALSE);
	g_return_val_if_fail(key != NULL,FALSE);

	GKeyFile *keyfile = g_key_file_new ();
	GError *error =NULL;
	gboolean result = FALSE;
	if(!g_key_file_load_from_file(keyfile,path,G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS ,&error))
	{
		g_warning ( "Key file load from file %s error : %s ",path,error!=NULL?error->message:"unknown");
		if(error ) g_error_free(error);
	}
	else
	{
		gchar *val = g_key_file_get_value   (keyfile,group,key,&error);
		if(val == NULL)
		{
			//g_warning ( "Key file get value error : %s ",error!=NULL?error->message:"unknown");
		}
		else
		{
			//g_debug("Check %s == %s",mkt_value_stringify(check_value),val);
			GValue *check = mkt_value_new(check_value->g_type);
			if(mkt_set_gvalue_from_string(check,val))
			{
				//g_debug("Values is setted ");
				result = mkt_value_g_value_equal(check_value,check);
			}
			mkt_value_free(check);
			g_free(val);
		}
	}
	g_key_file_free(keyfile);
	return result;
}

GValue*
mkt_keyfile_get   ( const gchar *path , const gchar *group , const gchar *key , GType type )
{
	g_return_val_if_fail(path != NULL,FALSE);
	g_return_val_if_fail(group != NULL,FALSE);
	g_return_val_if_fail(key != NULL,FALSE);
	GValue   *result    =NULL;
	GKeyFile *keyfile = g_key_file_new ();
	GError *error =NULL;
	if(!g_key_file_load_from_file(keyfile,path,G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS ,&error))
	{
		g_warning ( "Key file load from file %s error : %s ",path,error!=NULL?error->message:"unknown");
		if(error ) g_error_free(error);
	}
	else
	{
		gchar *val = g_key_file_get_value   (keyfile,group,key,&error);
		if(val == NULL)
		{
			//g_warning ( "Key file get value error : %s ",error!=NULL?error->message:"unknown");
		}
		else
		{
			result = mkt_value_new(type);
			if(!mkt_set_gvalue_from_string(result,val))
			{
				g_warning( "Can not set value type %s from key file:%s  [%s] %s",g_type_name(type),path,group,key);
				mkt_value_free(result);
				result = NULL;
			}
			g_free(val);
		}
	}
	g_key_file_free(keyfile);
	return result;
}


GParamSpec *
mkt_utils_find_interface_pspec ( GType model_type , const gchar *pname )
{
	guint ni = 0;
	GType *iface = g_type_interfaces (model_type,&ni);
	int i = 0;
	GParamSpec *pspec = NULL;
	for(i=0;i<ni;i++)
	{
		//g_debug("Test ... wass soll dass %s",g_type_name(iface[i]));
		gpointer itable   =  g_type_default_interface_ref  (iface[i]);
		if(itable)
		{
			//g_debug("find property=%s",pname);
			pspec =  g_object_interface_find_property(itable,pname);
			if(itable)g_type_default_interface_unref(itable);
			if(pspec != NULL)	break;
		}
	}
	g_free(iface);
	return pspec;
}


GParameter*
mkt_utils_keyfile_create_parameter (GType otype, const gchar *path  ,const gchar *group ,gsize *plen )
{
	GParameter *param = NULL;
	*plen = 0;
	gsize len;
	GKeyFile *keyfile = g_key_file_new ();
	GError *error =NULL;
	int npar = 0;
	if(!g_key_file_load_from_file(keyfile,path,G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS ,&error))
	{
		g_warning ( "Key file load from file %s error : %s ",path,error!=NULL?error->message:"unknown");
		if(error ) g_error_free(error);

	}
	else
	{

		gchar **keys = g_key_file_get_keys   (keyfile,group,&len,&error );
		if(keys == NULL || error )
		{
			g_warning ( "Key file sort keys  error : %s ",error!=NULL?error->message:"unknown");
			if(error) g_error_free(error );
		}
		else
		{
			int i=0;
			param = calloc(sizeof(GParameter),len);
			for(i=0;i<len && keys[i]!=NULL;i++)
			{
				error = NULL;
				gchar *val = g_key_file_get_value   (keyfile,group,keys[i],&error);
				if(val == NULL)
				{
					g_warning ( "Key file get value error : %s ",error!=NULL?error->message:"unknown");
				}
				else
				{
					GString *string = g_string_new(keys[i]);
					string = g_string_ascii_down(string);
					//g_debug("check %s=%s",string->str,val);
					GParamSpec *pspec = mkt_utils_find_interface_pspec(otype,string->str);
					if(pspec)
					{
						GValue *value = mkt_value_new(pspec->value_type);
						if(!mkt_set_gvalue_from_string(value,val)) // FIX: need string escape.
						{
							value = NULL;
							g_warning("Object value %s:%s transform failed ",pspec->name, val ? val : "NULL");
							//exit(1);
						}
						else
						{
							param[npar].name   = pspec->name;
							g_value_init(&param[npar].value ,value->g_type);
							g_value_copy(value,&param[npar].value);
							npar++;
						}
						mkt_value_free(value);
					}
					g_free(val);
					g_string_free(string,TRUE);
				}
			}
			if(keys) g_strfreev(keys);
		}
	}
	if(param && npar == 0)
	{
		g_free(param);
		param = NULL;
	}
	if(keyfile)g_key_file_free(keyfile);
	*plen = npar;
	return param;
}


GParameter*
mkt_utils_valist_create_parameter ( GType object_type , gsize *plen , const gchar *first_property_name, va_list var_args )
{
	GParameter *params;
	const gchar *name;
	guint n_params = 0, n_alloced_params = 36;
	g_return_val_if_fail (G_TYPE_IS_OBJECT (object_type), NULL);
	if (!first_property_name)
		return NULL;

	params = calloc (sizeof(GParameter), n_alloced_params);
	name = first_property_name;
	while (name)
	{
		gchar *error = NULL;
		GParamSpec *pspec = mkt_utils_find_interface_pspec(object_type,name);
		if (!pspec)
		{
			g_warning ("%s: object class `%s' has no property named `%s'",
					G_STRFUNC,
					g_type_name (object_type),
					name);
			break;
		}
		if (n_params >= n_alloced_params)
		{
			break;
			//n_alloced_params += 16;
			//params = g_renew (GParameter, params, n_alloced_params);
			//memset (params + n_params, 0, 16 * (sizeof *params));
		}
		params[n_params].name = pspec->name;
		MKT_UTIL_VALUE_COLLECT_INIT (&params[n_params].value, pspec->value_type,
				var_args, 0, &error);
		if (error)
		{
			g_warning ("%s: %s", G_STRFUNC, error);
			g_free (error);
			g_value_unset (&params[n_params].value);
			break;
		}
		n_params++;
		name = va_arg (var_args, gchar*);
	}
	if(n_params==0)
	{
		g_free(params);
		params = NULL;
	}
	*plen=n_params;
	return params;
}

static void
mkt_utils_unset_parameter ( GParameter *src ,  gsize len )
{
	if(src== 0 || len == 0)return;
	int i;
	for(i=0;i<len;i++)
	{
		if(src[i].value.g_type > G_TYPE_NONE)
			g_value_unset (&src[i].value);
	}
}
static gsize
mkt_utils_copy_parameter ( GParameter *dest , GParameter *src , gsize dest_len , gsize src_len )
{
	dest = realloc(dest,sizeof(GParameter)*(dest_len+src_len+1));
	int i = 0;
	int j = 0 ;
	gsize len = dest_len;
	for(i =dest_len; i<  dest_len+src_len ; i++ , j++)
	{
		len++;
		if(j<src_len)
		{
			//dest[i].value.g_type = 0;
			//g_debug("copy src[j].name=%s to %d %s : %s",src[j].name,i,g_type_name(dest[i].value.g_type),g_type_name(src[j].value.g_type));
			dest[i].name =src[j].name;
			g_value_init(&dest[i].value,src[j].value.g_type);
			g_value_copy(&src[j].value,&dest[i].value);
		}
	}
	return len;
}


GObject*
mkt_utils_keyfile_new_object  (GType otype , const gchar *path ,const gchar *group)
{
	GObject *object = NULL;
	gsize plen=0;
	GParameter *param = mkt_utils_keyfile_create_parameter(otype,path,group,&plen);
	if(plen>0)
	{
		object = g_object_newv(otype,plen,param);
		int i=0;
		for(i=0;i<plen ;i++ )
		{
			if(param[i].value.g_type > G_TYPE_NONE)
				g_value_unset(&param[i].value);
		}
	}
	if(param ) g_free(param);
	return object;
}

GObject*
mkt_utils_keyfile_new_objectv  (GType otype , const gchar *path ,const gchar *group,const gchar *first_property_name,...)
{
	GObject *object = NULL;
	GParameter *user_param = NULL;
	gsize ulen = 0;
	GParameter *key_par    = NULL;
	gsize klen = 0;
	va_list var_args;
	g_return_val_if_fail (G_TYPE_IS_OBJECT (otype), NULL);

	  /* short circuit for calls supplying no properties */
	if (!first_property_name)
	    return mkt_utils_keyfile_new_object(otype,path,group);

	key_par = mkt_utils_keyfile_create_parameter(otype,path,group,&klen);
	va_start (var_args, first_property_name);
	user_param = mkt_utils_valist_create_parameter (otype,&ulen, first_property_name,var_args);
	va_end (var_args);

	if(user_param && key_par )
	{
		gsize nplen = mkt_utils_copy_parameter(user_param,key_par,ulen,klen);
		if(nplen>0)
		{
			object = g_object_newv(otype,nplen,user_param);
		}
		/*for(i=0;i<nplen ;i++ )
		{
			g_debug("Test Param :%s",user_param[i].name);
		}*/

		mkt_utils_unset_parameter(user_param,nplen);
		mkt_utils_unset_parameter(key_par,klen);
	}
	if(key_par) g_free(key_par);
	if(user_param) g_free(user_param);
	return object;
}

gchar*
mkt_utils_dup_dbus_path                         ( const gchar *object_name )
{
	g_return_val_if_fail(object_name!=NULL,NULL);
	GString *string = g_string_new("");
	gchar **path_tail = g_strsplit_set(object_name,".",-1);
	int i=0;
	for(i=0;path_tail!=NULL&&path_tail[i]!=NULL;i++)
	{
		string = g_string_append_c(string,'/');
		string = g_string_append(string,path_tail[i]);
	}
	gchar *ret = string->str;
	if(path_tail)g_strfreev(path_tail);
	g_string_free(string,FALSE);
	return ret;
}

gchar*
mkt_utils_dup_dbus_name                         ( const gchar *object_path )
{
	g_return_val_if_fail(object_path!=NULL,NULL);
	GString *string = g_string_new("");
	gchar **path_tail = g_strsplit_set(object_path,"/",-1);
	int i=0;
	for(i=0;path_tail!=NULL&&path_tail[i]!=NULL;i++)
	{
		if(i!=0)string = g_string_append_c(string,'.');
		string = g_string_append(string,path_tail[i]);
	}
	gchar *ret = string->str;
	if(path_tail)g_strfreev(path_tail);
	g_string_free(string,FALSE);
	return ret;
}

gboolean
mkt_utils_settings_check_schema               ( const gchar *schema_id )
{
	g_return_val_if_fail(schema_id!=NULL,FALSE);
	gboolean is_foud = FALSE;
	GSettingsSchema *schema        = g_settings_schema_source_lookup(g_settings_schema_source_get_default(),schema_id,TRUE);
	if(schema)
	{
		is_foud = TRUE;
		g_settings_schema_unref (schema);
	}
	return is_foud;
}

gboolean
mkt_utils_settings_check_key                ( GSettings *settings, const gchar *key )
{
	gchar **keys = 	g_settings_list_keys(settings);
	gboolean has_key = FALSE;
	gint i = 0;
	for(i=0;keys !=NULL&&keys[i]!=NULL;i++)
		if(0==g_strcmp0(keys[i],key))has_key = TRUE;
	if(keys)g_strfreev(keys);
	return has_key;
}



gboolean
mkt_utils_settings_check_child               ( GSettings *settings , const gchar *child )
{

	g_return_val_if_fail(settings!=NULL,FALSE);
	g_return_val_if_fail(G_IS_SETTINGS(settings),FALSE);
	gchar **children = 	g_settings_list_children (settings);
	gboolean has_child = FALSE;
	gint i = 0;
	for(i=0;children !=NULL&&children[i]!=NULL;i++)
		if(0==g_strcmp0(children[i],child))has_child = TRUE;
	if(children)g_strfreev(children);
	return has_child;
}

GSettings*
mkt_utils_settings_get_child                 ( GSettings *settings , const gchar *child )
{
	g_return_val_if_fail(settings!=NULL,FALSE);
	g_return_val_if_fail(G_IS_SETTINGS(settings),FALSE);
	gchar **children = 	g_settings_list_children (settings);
	GSettings *child_settings= NULL;;
	gint i = 0;
	for(i=0;children !=NULL&&children[i]!=NULL;i++)
		if(0==g_strcmp0(children[i],child))child_settings = g_settings_get_child(settings,child);
	return child_settings;
}

gboolean
mkt_utils_settings_mapping_get_uint32_to_string ( GValue  *value, GVariant  *variant, gpointer            user_data)
{
	g_return_val_if_fail(G_VALUE_HOLDS(value,G_TYPE_STRING),FALSE);
	g_return_val_if_fail(g_variant_is_of_type(variant,G_VARIANT_TYPE_UINT32),FALSE);
	guint u   = g_variant_get_uint32(variant);
	gchar *s  = g_strdup_printf("%d",u);
	g_value_set_string(value,s);
	g_free(s);
	return TRUE;
}

gboolean
mkt_utils_settings_mapping_get_double_to_string ( GValue  *value, GVariant  *variant, gpointer            user_data)
{
	g_return_val_if_fail(G_VALUE_HOLDS(value,G_TYPE_STRING),FALSE);
	g_return_val_if_fail(g_variant_is_of_type(variant,G_VARIANT_TYPE_DOUBLE),FALSE);
	gdouble d  = g_variant_get_double(variant);
	gchar  *s  = g_strdup_printf("%f",d);
	g_value_set_string(value,s);
	g_free(s);
	return TRUE;
}

gboolean
mkt_utils_settings_mapping_get_int32_to_string ( GValue  *value, GVariant  *variant, gpointer            user_data)
{
	g_return_val_if_fail(G_VALUE_HOLDS(value,G_TYPE_STRING),FALSE);
	g_return_val_if_fail(g_variant_is_of_type(variant,G_VARIANT_TYPE_INT32),FALSE);
	guint u   = g_variant_get_int32(variant);
	gchar *s  = g_strdup_printf("%d",u);
	g_value_set_string(value,s);
	g_free(s);
	return TRUE;
}




// GVarinat uint32 -> GValue double

gboolean
mkt_utils_settings_mapping_get_uint32_to_double ( GValue  *value, GVariant  *variant, gpointer user_data)
{
	g_return_val_if_fail(G_VALUE_HOLDS(value,G_TYPE_DOUBLE),FALSE);
	g_return_val_if_fail(g_variant_is_of_type(variant,G_VARIANT_TYPE_UINT32),FALSE);
	gdouble dv= (gdouble) g_variant_get_uint32(variant);
	g_value_set_double(value,dv);
	return TRUE;
}
// GVarinat double -> GValue uint
gboolean
mkt_utils_settings_mapping_get_double_to_uint ( GValue  *value, GVariant  *variant, gpointer user_data)
{
	g_return_val_if_fail(G_VALUE_HOLDS(value,G_TYPE_UINT),FALSE);
	g_return_val_if_fail(g_variant_is_of_type(variant,G_VARIANT_TYPE_DOUBLE),FALSE);
	guint dv= (guint) g_variant_get_double(variant);
	g_value_set_uint(value,dv);
	return TRUE;
}

GVariant*
mkt_utils_settings_mapping_set_uint_to_double  ( const GValue *value,const GVariantType *expected_type,gpointer user_data)
{
	g_return_val_if_fail(G_VALUE_HOLDS(value,G_TYPE_UINT),FALSE);
	g_return_val_if_fail(g_variant_type_equal(expected_type,G_VARIANT_TYPE_DOUBLE),FALSE);
	gdouble vd = (double)g_value_get_uint(value);
	return g_variant_new_double(vd);
}

GVariant*
mkt_utils_settings_mapping_set_double_to_uint32  ( const GValue *value,const GVariantType *expected_type,gpointer user_data)
{
	g_return_val_if_fail(G_VALUE_HOLDS(value,G_TYPE_DOUBLE),FALSE);
	g_return_val_if_fail(g_variant_type_equal(expected_type,G_VARIANT_TYPE_UINT32),FALSE);
	GValue uint_val = {0};
	g_value_init(&uint_val,G_TYPE_UINT);
	GVariant *variant = NULL;
	if(g_value_transform(value,&uint_val))
	{
			g_variant_new_uint32(g_value_get_uint(&uint_val));
	}
	return variant;
}


gboolean
mkt_utils_settings_mapping_get_int32_to_double ( GValue  *value, GVariant  *variant, gpointer user_data)
{
	g_return_val_if_fail(G_VALUE_HOLDS(value,G_TYPE_DOUBLE),FALSE);
	g_return_val_if_fail(g_variant_is_of_type(variant,G_VARIANT_TYPE_INT32),FALSE);
	gdouble dv= (gdouble) g_variant_get_int32(variant);
	g_value_set_double(value,dv);
	return TRUE;
}
// GVarinat double -> GValue uint
gboolean
mkt_utils_settings_mapping_get_double_to_int ( GValue  *value, GVariant  *variant, gpointer user_data)
{
	g_return_val_if_fail(G_VALUE_HOLDS(value,G_TYPE_INT),FALSE);
	g_return_val_if_fail(g_variant_is_of_type(variant,G_VARIANT_TYPE_DOUBLE),FALSE);
	gint dv= (gint) g_variant_get_double(variant);
	g_value_set_int(value,dv);
	return TRUE;
}

GVariant*
mkt_utils_settings_mapping_set_int_to_double  ( const GValue *value,const GVariantType *expected_type,gpointer user_data)
{
	g_return_val_if_fail(G_VALUE_HOLDS(value,G_TYPE_INT),FALSE);
	g_return_val_if_fail(g_variant_type_equal(expected_type,G_VARIANT_TYPE_DOUBLE),FALSE);
	gdouble vd = (gdouble)g_value_get_int(value);
	return g_variant_new_double(vd);
}

GVariant*
mkt_utils_settings_mapping_set_double_to_int32  ( const GValue *value,const GVariantType *expected_type,gpointer user_data)
{
	g_return_val_if_fail(G_VALUE_HOLDS(value,G_TYPE_DOUBLE),FALSE);
	g_return_val_if_fail(g_variant_type_equal(expected_type,G_VARIANT_TYPE_INT32),FALSE);
	GValue int_val = {0};
	g_value_init(&int_val,G_TYPE_INT);
	GVariant *variant = NULL;
	if(g_value_transform(value,&int_val))
	{
			g_variant_new_int32(g_value_get_int(&int_val));
	}
	return variant;
}

gboolean
mkt_utils_transform_binding_double_to_string    ( GBinding     *binding, const GValue *from_value,GValue *to_value, gpointer  user_data)
{
	g_return_val_if_fail(from_value!= NULL,FALSE);
	g_return_val_if_fail(to_value!= NULL,FALSE);
	g_return_val_if_fail(G_VALUE_HOLDS_STRING(to_value),FALSE);
	g_return_val_if_fail(G_VALUE_HOLDS_DOUBLE(from_value),FALSE);
	g_value_set_string(to_value,mkt_value_stringify_double(g_value_get_double(from_value)));
	return TRUE;
}


gboolean
mkt_utils_transform_binding_all_to_string      ( GBinding     *binding, const GValue *from_value,GValue *to_value, gpointer  user_data)
{
	g_return_val_if_fail(from_value!= NULL,FALSE);
	g_return_val_if_fail(to_value!= NULL,FALSE);
	g_return_val_if_fail(G_VALUE_HOLDS_STRING(to_value),FALSE);
	gchar *str = mkt_value_stringify(from_value);
	g_value_set_string(to_value,str);
	g_free(str);
	return TRUE;
}

gboolean
mkt_utils_transform_binding_all_from_string      ( GBinding     *binding, const GValue *from_value,GValue *to_value, gpointer  user_data)
{
	g_return_val_if_fail(from_value!= NULL,FALSE);
	g_return_val_if_fail(to_value!= NULL,FALSE);
	g_return_val_if_fail(G_VALUE_HOLDS_STRING(from_value),FALSE);
	return mkt_set_gvalue_from_string(to_value,g_value_get_string(from_value));
}


gboolean
mkt_utils_transform_binding_double_to_uint      ( GBinding     *binding, const GValue *from_value,GValue *to_value, gpointer  user_data)
{
	g_return_val_if_fail(from_value!= NULL,FALSE);
	g_return_val_if_fail(to_value!= NULL,FALSE);
	g_return_val_if_fail(G_VALUE_HOLDS_DOUBLE(from_value),FALSE);
	g_return_val_if_fail(G_VALUE_HOLDS_UINT(to_value),FALSE);
	return g_value_transform(from_value,to_value);
}

gboolean
mkt_utils_transform_binding_uint_to_double      ( GBinding     *binding, const GValue *from_value,GValue *to_value, gpointer  user_data)
{
	g_return_val_if_fail(from_value!= NULL,FALSE);
	g_return_val_if_fail(to_value!= NULL,FALSE);
	g_return_val_if_fail(G_VALUE_HOLDS_DOUBLE(to_value),FALSE);
	g_return_val_if_fail(G_VALUE_HOLDS_UINT(from_value),FALSE);
	return g_value_transform(from_value,to_value);
}

gboolean
mkt_utils_transform_binding_double_to_concentration      ( GBinding     *binding, const GValue *from_value,GValue *to_value, gpointer  user_data)
{
	g_return_val_if_fail(from_value!= NULL,FALSE);
	g_return_val_if_fail(to_value!= NULL,FALSE);
	g_return_val_if_fail(G_VALUE_HOLDS_STRING(to_value),FALSE);
	g_return_val_if_fail(G_VALUE_HOLDS_DOUBLE(from_value),FALSE);
	gchar *internal_value = g_strdup_printf("%.0f", g_value_get_double(from_value));
	g_value_set_string(to_value,internal_value);
	g_free(internal_value);
	return TRUE;
}




/*
// GVarinat uint32 -> GValue uint
gboolean
mkt_utils_settings_mapping_get_uint32_to_uint64 ( GValue  *value, GVariant  *variant, gpointer user_data)
{
	g_return_val_if_fail(G_VALUE_HOLDS(value,G_TYPE_UINT),FALSE);
	g_return_val_if_fail(g_variant_is_of_type(variant,G_VARIANT_TYPE_UINT32),FALSE);
	guint dv= (guint) g_variant_get_ui(variant);
	g_value_set_uint(value,dv);
	return TRUE;
}

GVariant*
mkt_utils_settings_mapping_set_uint64_to_uint32  ( const GValue *value,const GVariantType *expected_type,gpointer user_data)
{
	g_return_val_if_fail(G_VALUE_HOLDS(value,G_TYPE_UINT),FALSE);
	g_return_val_if_fail(g_variant_type_equal(expected_type,G_VARIANT_TYPE_DOUBLE),FALSE);
	gdouble vd = (double)g_value_get_uint(value);
	return g_variant_new_double(vd);
}

GVariant*
mkt_utils_settings_mapping_set_uint32_to_uint64  ( const GValue *value,const GVariantType *expected_type,gpointer user_data)
{
	g_return_val_if_fail(G_VALUE_HOLDS(value,G_TYPE_DOUBLE),FALSE);
	g_return_val_if_fail(g_variant_type_equal(expected_type,G_VARIANT_TYPE_UINT32),FALSE);
	GValue uint_val = {0};
	g_value_init(&uint_val,G_TYPE_UINT);
	GVariant *variant = NULL;
	if(g_value_transform(value,&uint_val))
	{
			g_variant_new_uint32(g_value_get_uint(&uint_val));
	}
	return variant;
}*/


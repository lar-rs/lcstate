/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * @ingroup NodeIndex
 * @{
 * @file  mkt-measurement.c	Measurement model interface
 * @brief This is Measurement model interface description.
 *
 *
 *  Copyright (C) A.Smolkov 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 */


#include "node-index.h"

#if GLIB_CHECK_VERSION(2,31,7)
static GRecMutex init_rmutex;
#define MUTEX_LOCK() g_rec_mutex_lock(&init_rmutex)
#define MUTEX_UNLOCK() g_rec_mutex_unlock(&init_rmutex)
#else
static GStaticRecMutex init_mutex = G_STATIC_REC_MUTEX_INIT;
#define MUTEX_LOCK() g_static_rec_mutex_lock(&init_mutex)
#define MUTEX_UNLOCK() g_static_rec_mutex_unlock(&init_mutex)
#endif

static void
node_index_base_init (gpointer g_iface)
{
	static gboolean is_node_index_initialized = FALSE;
	MUTEX_LOCK();
	if (!is_node_index_initialized)
	{
		g_object_interface_install_property (g_iface,
				g_param_spec_string ("indexid",
						"Index id",
						"Get index id name",
						"unknown",
						G_PARAM_READABLE | G_PARAM_WRITABLE ));
		g_object_interface_install_property (g_iface,
				g_param_spec_string ("parametername",
						"Index Parameter name",
						"Get index parameter name ",
						"unknown",
						G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
		g_object_interface_install_property (g_iface,
				g_param_spec_uint ("objecttype",
						"Index object type",
						"Get index object type",
						0,G_MAXUINT32,0,
						G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
		g_object_interface_install_property (g_iface,
				g_param_spec_uint ("datatype",
						"Index data type",
						"Get index data type",
						0,G_MAXUINT32,0,
						G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
		g_object_interface_install_property (g_iface,
				g_param_spec_string ("accesstype",
						"Index access type",
						"Get index access type",
						"ro",
						G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
		g_object_interface_install_property (g_iface,
				g_param_spec_string ("defaultvalue",
						"Index default value",
						"Get index default value",
						"---",
						G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
		g_object_interface_install_property (g_iface,
				g_param_spec_uint ("pdomapping",
						"Index PDO mapping",
						"Get index PDO mapping",
						0,G_MAXUINT32,0,
						G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

		g_object_interface_install_property (g_iface,
				g_param_spec_uint ("lowlimit",
						"Index low limit",
						"Get index low limit",
						0,G_MAXUINT32,0,
						G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
		g_object_interface_install_property (g_iface,
				g_param_spec_uint ("highlimit",
						"Index high limit",
						"Get index high limit",
						0,G_MAXUINT32,0,
						G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

		g_object_interface_install_property (g_iface,
				g_param_spec_uint ("nodeid",
						"Index object type",
						"Get index object type",
						0,G_MAXUINT32,1,
						G_PARAM_READABLE | G_PARAM_WRITABLE ));
		g_object_interface_install_property (g_iface,
				g_param_spec_uint ("index",
						"Index object type",
						"Get index object type",
						0,G_MAXUINT32,0,
						G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

		g_object_interface_install_property (g_iface,
				g_param_spec_uint ("subindex",
						"Index object type",
						"Get index object type",
						0,G_MAXUINT32,0,
						G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));



		is_node_index_initialized = TRUE;
	}
	MUTEX_UNLOCK();
}

GType
node_index_get_type (void)
{
	static GType iface_type = 0;
	if (iface_type == 0)
	{
		static const GTypeInfo info = {
				sizeof (NodeIndexInterface),
				(GBaseInitFunc) node_index_base_init,
				(GBaseFinalizeFunc) NULL,
				(GClassInitFunc) NULL,
				NULL,
				NULL,
				0,
				0,
				(GInstanceInitFunc) NULL,
				0
		};
		MUTEX_LOCK();
		if (iface_type == 0)
		{
			iface_type = g_type_register_static (G_TYPE_INTERFACE, "NodeIndexInterface",&info, 0);
		}
		MUTEX_UNLOCK();
	}
	return iface_type;
}


const gchar*
node_index_indexid                               ( NodeIndex *index )
{
	g_return_val_if_fail(index != NULL , NULL);
	g_return_val_if_fail(NODE_IS_INDEX(index) , NULL);
	if(NODE_INDEX_GET_INTERFACE(index)->indexid )
			return NODE_INDEX_GET_INTERFACE(index)->indexid(index);
	return NULL;
}
const gchar*
node_index_parametername                         ( NodeIndex *index )
{
	g_return_val_if_fail(index != NULL , NULL);
	g_return_val_if_fail(NODE_IS_INDEX(index) , NULL);
	if(NODE_INDEX_GET_INTERFACE(index)->parametername )
			return NODE_INDEX_GET_INTERFACE(index)->parametername(index);
	return NULL;
}

guint
node_index_objecttype                            ( NodeIndex *index )
{
	g_return_val_if_fail(index != NULL , 0);
	g_return_val_if_fail(NODE_IS_INDEX(index) , 0);
	if(NODE_INDEX_GET_INTERFACE(index)->objecttype )
			return NODE_INDEX_GET_INTERFACE(index)->objecttype(index);
	return 0;
}

guint
node_index_datatype                              ( NodeIndex *index )
{
	g_return_val_if_fail(index != NULL , 0);
	g_return_val_if_fail(NODE_IS_INDEX(index) , 0);
	if(NODE_INDEX_GET_INTERFACE(index)->datatype )
			return NODE_INDEX_GET_INTERFACE(index)->datatype(index);
	return 0;
}

const gchar*
node_index_accesstype                            ( NodeIndex *index )
{
	g_return_val_if_fail(index != NULL , NULL);
	g_return_val_if_fail(NODE_IS_INDEX(index) , NULL);
	if(NODE_INDEX_GET_INTERFACE(index)->accesstype )
			return NODE_INDEX_GET_INTERFACE(index)->accesstype(index);
	return NULL;
}

const gchar*
node_index_defaultvalue                          ( NodeIndex *index )
{
	g_return_val_if_fail(index != NULL , NULL);
	g_return_val_if_fail(NODE_IS_INDEX(index) , NULL);
	if(NODE_INDEX_GET_INTERFACE(index)->defaultvalue )
			return NODE_INDEX_GET_INTERFACE(index)->defaultvalue(index);
	return NULL;
}

guint
node_index_pdomapping                            ( NodeIndex *index )
{
	g_return_val_if_fail(index != NULL , 0);
	g_return_val_if_fail(NODE_IS_INDEX(index) , 0);
	if(NODE_INDEX_GET_INTERFACE(index)->pdomapping )
			return NODE_INDEX_GET_INTERFACE(index)->pdomapping(index);
	return 0;
}

guint
node_index_lowlimit                              ( NodeIndex *index )
{
	g_return_val_if_fail(index != NULL , 0);
	g_return_val_if_fail(NODE_IS_INDEX(index) , 0);
	if(NODE_INDEX_GET_INTERFACE(index)->lowlimit )
			return NODE_INDEX_GET_INTERFACE(index)->lowlimit(index);
	return 0;
}

guint
node_index_highlimit                             ( NodeIndex *index )
{
	g_return_val_if_fail(index != NULL , 0);
	g_return_val_if_fail(NODE_IS_INDEX(index) , 0);
	if(NODE_INDEX_GET_INTERFACE(index)->highlimit )
			return NODE_INDEX_GET_INTERFACE(index)->highlimit(index);
	return 0;
}

guint node_index_nodeid (NodeIndex* index)
{
	g_return_val_if_fail (index,                0);
	g_return_val_if_fail (NODE_IS_INDEX(index), 0);

	if (NODE_INDEX_GET_INTERFACE (index) -> nodeid)
			return NODE_INDEX_GET_INTERFACE (index) -> nodeid (index);

	return 0;
}

guint
node_index_index                                 ( NodeIndex *index )
{
	g_return_val_if_fail(index != NULL , 0);
	g_return_val_if_fail(NODE_IS_INDEX(index) , 0);
	if(NODE_INDEX_GET_INTERFACE(index)->index )
			return NODE_INDEX_GET_INTERFACE(index)->index(index);
	return 0;
}
guint
node_index_subindex                              ( NodeIndex *index )
{
	g_return_val_if_fail(index != NULL , 0);
	g_return_val_if_fail(NODE_IS_INDEX(index) , 0);
	if(NODE_INDEX_GET_INTERFACE(index)->subindex )
			return NODE_INDEX_GET_INTERFACE(index)->subindex(index);
	return 0;
}

guint
node_index_value32                                ( NodeIndex *index )
{
	g_return_val_if_fail(index != NULL , 0);
	g_return_val_if_fail(NODE_IS_INDEX(index) , 0);
	if(NODE_INDEX_GET_INTERFACE(index)->value32 )
			return NODE_INDEX_GET_INTERFACE(index)->value32(index);
	return 0;
}

GValue* node_index_value (NodeIndex* index)
{
	g_return_val_if_fail (index,                NULL);
	g_return_val_if_fail (NODE_IS_INDEX(index), NULL);

	if (NODE_INDEX_GET_INTERFACE (index) -> value)
		return NODE_INDEX_GET_INTERFACE (index) -> value (index);

	return 0;
}

gboolean node_index_set_value32 (NodeIndex* index, guint value)
{
	g_return_val_if_fail (index,                FALSE);
	g_return_val_if_fail (NODE_IS_INDEX(index), FALSE);

	if (NODE_INDEX_GET_INTERFACE (index) -> set_value32)
			return NODE_INDEX_GET_INTERFACE (index) -> set_value32 (index, value);

	return FALSE;
}

gboolean
node_index_set_valueStr                             ( NodeIndex *index, const gchar *str_val )
{
	g_return_val_if_fail(index != NULL , FALSE);
	g_return_val_if_fail(NODE_IS_INDEX(index) , FALSE);
	if(NODE_INDEX_GET_INTERFACE(index)->set_value_str )
		return NODE_INDEX_GET_INTERFACE(index)->set_value_str(index,str_val);
	return FALSE;
}

gboolean
node_index_set_value                             ( NodeIndex *index, const GValue *value  )
{
	g_return_val_if_fail(index != NULL , FALSE);
	g_return_val_if_fail(NODE_IS_INDEX(index) , FALSE);
	if(NODE_INDEX_GET_INTERFACE(index)->set_value )
		return NODE_INDEX_GET_INTERFACE(index)->set_value(index,value);
	return FALSE;
}

GType
node_index_value_type                            ( NodeIndex *index )
{
	g_return_val_if_fail(index != NULL , G_TYPE_NONE);
	g_return_val_if_fail(NODE_IS_INDEX(index) , G_TYPE_NONE);
	if(NODE_INDEX_GET_INTERFACE(index)->get_value_type )
		return NODE_INDEX_GET_INTERFACE(index)->get_value_type(index);
	return G_TYPE_NONE;
}


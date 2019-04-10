/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @defgroup MktLibrary
 * @defgroup NodeIndex
 * @ingroup  NodeIndex
 * @{
 * @file  mkt-measurement.h	Measurement interface model header
 * @brief This is Measurement interface model object header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 * $Id: $ $URL: $
 */
#ifndef NODE_INDEX_H_
#define NODE_INDEX_H_


#include <mktlib.h>
#include <mktbus.h>


G_BEGIN_DECLS

#define NODE_TYPE_INDEX                  (node_index_get_type ())
#define NODE_INDEX(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj),NODE_TYPE_INDEX, NodeIndex))
#define NODE_IS_INDEX(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj),NODE_TYPE_INDEX))
#define NODE_INDEX_GET_INTERFACE(inst)   (G_TYPE_INSTANCE_GET_INTERFACE ((inst), NODE_TYPE_INDEX, NodeIndexInterface))


typedef struct _NodeIndexInterface   NodeIndexInterface;
typedef struct _NodeIndex            NodeIndex;



struct _NodeIndexInterface
{
	GTypeInterface            parent_iface;
	const gchar*            (*indexid)                                   ( NodeIndex *self );
	const gchar*            (*parametername)                             ( NodeIndex *self );
	guint                   (*objecttype)                                ( NodeIndex *self );
	guint                   (*datatype)                                  ( NodeIndex *self );
	const gchar*            (*accesstype)                                ( NodeIndex *self );
	const gchar*            (*defaultvalue)                              ( NodeIndex *self );
	guint                   (*pdomapping)                                ( NodeIndex *self );
	guint                   (*lowlimit)                                  ( NodeIndex *self );
	guint                   (*highlimit)                                 ( NodeIndex *self );
	guint                   (*nodeid)                                    ( NodeIndex *self );
	guint                   (*index)                                     ( NodeIndex *self );
	guint                   (*subindex)                                  ( NodeIndex *self );
	guint                   (*value32)                                   ( NodeIndex *self );
	GValue*                 (*value)                                     ( NodeIndex *self );
	gboolean                (*set_value32)                               ( NodeIndex *self , guint value );
	gboolean                (*set_value_str)                             ( NodeIndex *self , const gchar  *value );
	gboolean                (*set_value)                                 ( NodeIndex *self , const GValue *value );
	GType                   (*get_value_type)                            ( NodeIndex *self );

};




GType                   node_index_get_type                              ( void ) G_GNUC_CONST;

const gchar*            node_index_indexid                               ( NodeIndex *index );
const gchar*            node_index_parametername                         ( NodeIndex *index );
guint                   node_index_objecttype                            ( NodeIndex *index );
guint                   node_index_datatype                              ( NodeIndex *index );
const gchar*            node_index_accesstype                            ( NodeIndex *index );
const gchar*            node_index_defaultvalue                          ( NodeIndex *index );
guint                   node_index_pdomapping                            ( NodeIndex *index );
guint                   node_index_lowlimit                              ( NodeIndex *index );
guint                   node_index_highlimit                             ( NodeIndex *index );
guint                   node_index_nodeid                                ( NodeIndex *index );
guint                   node_index_index                                 ( NodeIndex *index );
guint                   node_index_subindex                              ( NodeIndex *index );
guint                   node_index_value32                               ( NodeIndex *index );
GValue*                 node_index_value                                 ( NodeIndex *index );
gboolean                node_index_set_value32                           ( NodeIndex *index, guint value);
gboolean                node_index_set_valueStr                          ( NodeIndex *index, const gchar *str_val );
gboolean                node_index_set_value                             ( NodeIndex *index, const GValue *value  );
GType                   node_index_value_type                            ( NodeIndex *index );




G_END_DECLS

#endif /* NODE_INDEX_H_ */

/** @} */

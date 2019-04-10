/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * node-index.c
 * Copyright (C) 2014 doseus <doseus@doseus-ThinkPad-T430s>
 *
 * largdm is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * largdm is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "node-index-object.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _NodeIndexObjectPrivate {
    gchar *  id;
    guint    node;
    guint    index;
    guint    subindex;
    gchar *  parameter_name;
    guint    object_type;
    guint    data_type;
    gchar *  access;
    gchar *  default_value;
    guint    pdo_mapping;
    guint    low;
    guint    high;
    GType    g_type;
    GValue * internal_value;
    GString *string_value;
    guint    toggle_byte;
    guint    len;
    gchar *  status;
};

enum {
    NODE_INDEX_READ  = 1 << 0,
    NODE_INDEX_WRITE = 1 << 0,
};

enum {
    PROP_0,
    PROP_INDEX_OBJECT_ID,
    PROP_PARAMETER_NAME,
    PROP_OBJECT_TYPE,
    PROP_DATA_TYPE,
    PROP_ACCESS_TYPE,
    PROP_DEFAULT_VALUE,
    PROP_PDO_MAPPING,
    PROP_LOW_LIMIT,
    PROP_HIGH_LIMIT,
    PROP_NODE_ID,
    PROP_INDEX,
    PROP_SUBINDEX

};

#define NODE_TRACE_FILE "/var/log/candaemon.log"

char *node_debug_get_rt() {
    mktTime_t rT;
    rT = mktNow();
    return mktTimeStr(&rT);
}

void node_trace_va(const char *format, va_list args) {
    FILE *f = fopen(NODE_TRACE_FILE, "a");
    if (f != NULL) {
        fprintf(f, "%s ", node_debug_get_rt());
        vfprintf(f, format, args);
        fflush(f);
        fclose(f);
    }
}

void node_trace(const char *format, ...) {
    va_list args;
    va_start(args, format);
    node_trace_va(format, args);
    va_end(args);
}

static const gchar *node_index_object_get_indexid(NodeIndex *index) {
    g_return_val_if_fail(index != NULL, NULL);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), NULL);
    return NODE_INDEX_OBJECT(index)->priv->id;
}
static const gchar *node_index_object_get_parametername(NodeIndex *index) {
    g_return_val_if_fail(index != NULL, NULL);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), NULL);
    return NODE_INDEX_OBJECT(index)->priv->parameter_name;
}

static guint node_index_object_get_objecttype(NodeIndex *index) {
    g_return_val_if_fail(index != NULL, 0);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), 0);
    return NODE_INDEX_OBJECT(index)->priv->object_type;
}

static guint node_index_object_get_datatype(NodeIndex *index) {
    g_return_val_if_fail(index != NULL, 0);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), 0);
    return NODE_INDEX_OBJECT(index)->priv->data_type;
}

static const gchar *node_index_object_get_accesstype(NodeIndex *index) {
    g_return_val_if_fail(index != NULL, NULL);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), NULL);
    return NODE_INDEX_OBJECT(index)->priv->access;
}

static const gchar *node_index_object_get_defaultvalue(NodeIndex *index) {
    g_return_val_if_fail(index != NULL, NULL);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), NULL);
    return NODE_INDEX_OBJECT(index)->priv->default_value;
}

static guint node_index_object_get_pdomapping(NodeIndex *index) {
    g_return_val_if_fail(index != NULL, 0);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), 0);
    return NODE_INDEX_OBJECT(index)->priv->pdo_mapping;
}

static guint node_index_object_get_lowlimit(NodeIndex *index) {
    g_return_val_if_fail(index != NULL, 0);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), 0);
    return NODE_INDEX_OBJECT(index)->priv->low;
}

static guint node_index_object_get_highlimit(NodeIndex *index) {
    g_return_val_if_fail(index != NULL, 0);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), 0);
    return NODE_INDEX_OBJECT(index)->priv->high;
}
static guint node_index_object_get_nodeid(NodeIndex *index) {
    g_return_val_if_fail(index != NULL, 0);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), 0);
    return NODE_INDEX_OBJECT(index)->priv->node;
}
static guint node_index_object_get_index(NodeIndex *index) {
    g_return_val_if_fail(index != NULL, 0);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), 0);
    return NODE_INDEX_OBJECT(index)->priv->index;
}
static guint node_index_object_get_subindex(NodeIndex *index) {
    g_return_val_if_fail(index != NULL, 0);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), 0);
    return NODE_INDEX_OBJECT(index)->priv->subindex;
}

static guint node_index_object_get_value32(NodeIndex *index) {
    g_return_val_if_fail(index != NULL, 0);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), 0);
    NodeIndexObject *oindex = NODE_INDEX_OBJECT(index);
    switch (oindex->priv->g_type) {
    case G_TYPE_BOOLEAN:
        break;

    case G_TYPE_CHAR:
        return (guint)g_value_get_int(oindex->priv->internal_value);
    case G_TYPE_INT:
        return (guint)g_value_get_int(oindex->priv->internal_value);
    case G_TYPE_UCHAR:
        return (guint)g_value_get_uint(oindex->priv->internal_value);
    case G_TYPE_UINT:
        return (guint)g_value_get_uint(oindex->priv->internal_value);
    case G_TYPE_DOUBLE:
        return (guint)g_value_get_double(oindex->priv->internal_value);
    case G_TYPE_STRING:
        if (g_value_get_string(oindex->priv->internal_value) == NULL)
            return 0;
        else
            return (guint)g_utf8_strlen(g_value_get_string(oindex->priv->internal_value), 4095);
    default:
        return 0;
    }
    return 0;
    // return NODE_INDEX_OBJECT(index)->priv->subindex;
}

static GValue* node_index_object_get_value (NodeIndex* index)
{
	NodeIndexObject* oindex;
	GValue*          value;

	g_return_val_if_fail (index,                       NULL);
	g_return_val_if_fail (NODE_IS_INDEX_OBJECT(index), NULL);

	oindex = NODE_INDEX_OBJECT (index);

	if (! oindex -> priv -> internal_value)
		return NULL;

	value = mkt_value_new (oindex -> priv -> internal_value -> g_type);

	g_value_copy (oindex->priv->internal_value, value);

	return value;
}

static gboolean node_index_object_set_value32 (NodeIndex* index, guint value)
{
	NodeIndexObject* oindex;

	g_return_val_if_fail (index,                       FALSE);
	g_return_val_if_fail (NODE_IS_INDEX_OBJECT(index), FALSE);

	oindex = NODE_INDEX_OBJECT (index);

	switch (oindex -> priv -> g_type) {
		case G_TYPE_BOOLEAN:
			g_value_set_boolean (oindex->priv->internal_value, (gboolean)value);
			break;

		case G_TYPE_CHAR:
			g_value_set_int (oindex->priv->internal_value, (gchar)value);
			return TRUE;

		case G_TYPE_INT:
			g_value_set_int (oindex->priv->internal_value, (gint)value);
			return TRUE;

		case G_TYPE_UCHAR:
			g_value_set_uint (oindex->priv->internal_value, (guchar)value);
			return TRUE;

		case G_TYPE_UINT:
			g_value_set_uint (oindex->priv->internal_value, (guint)value);
			return TRUE;

		case G_TYPE_DOUBLE:
			g_value_set_double (oindex->priv->internal_value, (gdouble)value);
			return TRUE;

		case G_TYPE_STRING:
			return FALSE;

		default:
			return FALSE;
	}

	return FALSE;
}

static gboolean node_index_object_set_valueStr (NodeIndex* index, const gchar* value)
{
	NodeIndexObject* oindex;

	g_return_val_if_fail (index,                       FALSE);
	g_return_val_if_fail (NODE_IS_INDEX_OBJECT(index), FALSE);

	oindex = NODE_INDEX_OBJECT (index);

	return mkt_set_gvalue_from_string (oindex->priv->internal_value, value);
}

static gboolean node_index_object_set_value (NodeIndex* index, const GValue* value)
{
	NodeIndexObject* oindex;

	g_return_val_if_fail (index,                       FALSE);
	g_return_val_if_fail (NODE_IS_INDEX_OBJECT(index), FALSE);
	g_return_val_if_fail (value,                       FALSE);

	oindex = NODE_INDEX_OBJECT (index);

	if (g_value_type_compatible (value->g_type, oindex->priv->internal_value->g_type)) {
		g_value_copy (value, oindex->priv->internal_value);
		return TRUE;
	}

	if (g_value_type_transformable (value->g_type, oindex->priv->internal_value->g_type)) {
		g_value_transform (value, oindex->priv->internal_value);
		return TRUE;
	}

	return FALSE;
}

static GType node_index_object_value_type(NodeIndex *index) {
    g_return_val_if_fail(index != NULL, G_TYPE_NONE);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), G_TYPE_NONE);
    NodeIndexObject *oindex = NODE_INDEX_OBJECT(index);
    if (!oindex->priv->internal_value) return G_TYPE_NONE;
    return oindex->priv->internal_value->g_type;
}

static void node_index_object_init_index_interface(NodeIndexInterface *iface) {
    iface->indexid        = node_index_object_get_indexid;
    iface->parametername  = node_index_object_get_parametername;
    iface->objecttype     = node_index_object_get_objecttype;
    iface->datatype       = node_index_object_get_datatype;
    iface->accesstype     = node_index_object_get_accesstype;
    iface->defaultvalue   = node_index_object_get_defaultvalue;
    iface->pdomapping     = node_index_object_get_pdomapping;
    iface->lowlimit       = node_index_object_get_lowlimit;
    iface->highlimit      = node_index_object_get_highlimit;
    iface->nodeid         = node_index_object_get_nodeid;
    iface->index          = node_index_object_get_index;
    iface->subindex       = node_index_object_get_subindex;
    iface->value          = node_index_object_get_value;
    iface->value32        = node_index_object_get_value32;
    iface->set_value32    = node_index_object_set_value32;
    iface->set_value_str  = node_index_object_set_valueStr;
    iface->set_value      = node_index_object_set_value;
    iface->get_value_type = node_index_object_value_type;
}

G_DEFINE_TYPE_WITH_CODE(NodeIndexObject, node_index_object, G_TYPE_OBJECT, G_IMPLEMENT_INTERFACE(NODE_TYPE_INDEX, node_index_object_init_index_interface))

static void node_index_object_init(NodeIndexObject *node_index) {
    node_index->priv                 = G_TYPE_INSTANCE_GET_PRIVATE(node_index, NODE_TYPE_INDEX_OBJECT, NodeIndexObjectPrivate);
    node_index->priv->node           = 0;
    node_index->priv->index          = 0;
    node_index->priv->subindex       = 0;
    node_index->priv->id             = NULL;
    node_index->priv->parameter_name = NULL;
    node_index->priv->object_type    = 0;
    node_index->priv->data_type      = 0;
    node_index->priv->access         = NULL;
    node_index->priv->default_value  = NULL;
    node_index->priv->pdo_mapping    = 0;
    node_index->priv->low            = 0;
    node_index->priv->high           = 0;
    node_index->priv->internal_value = mkt_value_new(G_TYPE_INT);
    node_index->priv->g_type         = G_TYPE_UINT;
    node_index->priv->string_value   = NULL;
    node_index->priv->status         = NULL;
}

static void node_index_object_finalize(GObject *object) {
    /* TODO: Add deinitalization code here */
    NodeIndexObject *index = NODE_INDEX_OBJECT(object);
    // g_debug("node_index_object_finalize %x", node_index_index(NODE_INDEX(index)));
    if (index->priv->id) g_free(index->priv->id);
    if (index->priv->parameter_name) g_free(index->priv->parameter_name);
    if (index->priv->access) g_free(index->priv->access);
    if (index->priv->default_value) g_free(index->priv->default_value);
    if (index->priv->internal_value) mkt_value_free(index->priv->internal_value);
    if (index->priv->string_value) g_string_free(index->priv->string_value, TRUE);
    G_OBJECT_CLASS(node_index_object_parent_class)->finalize(object);
}
static void node_index_init_data_type(NodeIndexObject *index) {
    // 0x0001= 	1 - 1 bit boolean
    // 0x0002= 	1 - 8 bit signed integer
    // 0x0003=  1 - 16 bit signed integer
    // 0x0004=  1 - 32 bit signed integer
    // 0x0005=  1 - 8 bit unsigned integer
    // 0x0006= 	1 - 16 bit unsigned integer
    // 0x0007= 	1 - 32 bit unsigned integer
    // 0x0008=	1 - single precision floating point
    // 0x0009= 	1 - visible string
    // 0x000A= 	1 - octet string  z.Z wird nicht unterstürtzt.
    // 0x000B= 	0 - date
    // 0x000C= 	0 - time of day
    // 0x000D= 	0 - time difference
    // 0x000E= 	0 - bit string
    // 0x000F= 	0 - domain
    // 0x0020= 	1 - PDO CommPar
    // 0x0021= 	1 - PDO mapping
    // 0x0022= 	1 - SDO parameter
    switch (index->priv->data_type) {
    case 0x1:
        index->priv->g_type = G_TYPE_BOOLEAN;
        break;
    case 0x2:
        index->priv->g_type = G_TYPE_INT;
        break;
    case 0x3:
        index->priv->g_type = G_TYPE_INT;
        break;
    case 0x4:
        index->priv->g_type = G_TYPE_INT;
        break;
    case 0x5:
        index->priv->g_type = G_TYPE_UINT;
        break;
    case 0x6:
        index->priv->g_type = G_TYPE_UINT;
        break;
    case 0x7:
        index->priv->g_type = G_TYPE_UINT;
        break;
    case 0x8:
        index->priv->g_type = G_TYPE_DOUBLE;
        break;
    case 0x9:
        index->priv->g_type = G_TYPE_STRING;
        break;
    default:
        index->priv->g_type = G_TYPE_INT;
        break;
    }
    if (index->priv->internal_value) mkt_value_free(index->priv->internal_value);
    index->priv->internal_value = mkt_value_new(index->priv->g_type);
}

static void node_index_init_object_type(NodeIndexObject *index) {
    //  0x07 = var
    //  0x08 = array
    //  0x09 = record
}

static void node_index_object_init_address (NodeIndexObject* index)
{
	gchar* p = strstr (index->priv->id, "sub");

	if (p) {
		gchar  i [10];
		size_t l;

		memset(i, 0, 10);

		l = (size_t) (p - index->priv->id);

		if (l > 9)
			l = 9;

		strncpy (i, index->priv->id, l);

		index -> priv -> index    = (guint) g_ascii_strtoull (index->priv->id, NULL, 16);
		p                        += 3;
		index -> priv -> index    = (guint) g_ascii_strtoull (i, NULL, 16);
		index -> priv -> subindex = (guint) g_ascii_strtoull (p, NULL, 16);
	}
	else {
		index -> priv -> index    = (guint) g_ascii_strtoull (index->priv->id, NULL, 16);
		index -> priv -> subindex = 0;
	}
}

static void node_index_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
	g_return_if_fail(NODE_IS_INDEX_OBJECT(object));
	NodeIndexObject *index = NODE_INDEX_OBJECT(object);
	switch (prop_id) {
	case PROP_INDEX_OBJECT_ID:
		if (index->priv->id) g_free(index->priv->id);
		index->priv->id = g_value_dup_string(value);
		node_index_object_init_address(index);
		break;
	case PROP_PARAMETER_NAME:
		if (index->priv->parameter_name) g_free(index->priv->parameter_name);
		index->priv->parameter_name = g_value_dup_string(value);
		break;
	case PROP_OBJECT_TYPE:
		index->priv->object_type = g_value_get_uint(value);
		node_index_init_object_type(index);
		break;
	case PROP_DATA_TYPE:
		index->priv->data_type = g_value_get_uint(value);
		node_index_init_data_type(index);
		break;
	case PROP_ACCESS_TYPE:
		if (index->priv->access) g_free(index->priv->access);
		index->priv->access = g_value_dup_string(value);
		break;
	case PROP_DEFAULT_VALUE:
		if (index->priv->default_value) g_free(index->priv->default_value);
		index->priv->default_value = g_value_dup_string(value);
		break;
	case PROP_PDO_MAPPING:
		index->priv->pdo_mapping = g_value_get_uint(value);
		break;
	case PROP_LOW_LIMIT:
		index->priv->low = g_value_get_uint(value);
		break;
	case PROP_HIGH_LIMIT:
		index->priv->high = g_value_get_uint(value);
		break;
	case PROP_NODE_ID:
		index->priv->node = g_value_get_uint(value);
		break;
	case PROP_INDEX:
		index->priv->index = g_value_get_uint(value);
		break;
	case PROP_SUBINDEX:
		index->priv->subindex = g_value_get_uint(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void node_index_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(NODE_IS_INDEX_OBJECT(object));
    NodeIndexObject *index = NODE_INDEX_OBJECT(object);
    switch (prop_id) {
    case PROP_INDEX_OBJECT_ID:
        g_value_set_string(value, index->priv->id);
        break;
    case PROP_PARAMETER_NAME:
        g_value_set_string(value, index->priv->parameter_name);
        break;
    case PROP_OBJECT_TYPE:
        g_value_set_uint(value, index->priv->object_type);
        break;
    case PROP_DATA_TYPE:
        g_value_set_uint(value, index->priv->data_type);
        break;
    case PROP_ACCESS_TYPE:
        g_value_set_string(value, index->priv->access);
        break;
    case PROP_DEFAULT_VALUE:
        g_value_set_string(value, index->priv->default_value);
        break;
    case PROP_PDO_MAPPING:
        g_value_set_uint(value, index->priv->pdo_mapping);
        break;
    case PROP_LOW_LIMIT:
        g_value_set_uint(value, index->priv->low);
        break;
    case PROP_HIGH_LIMIT:
        g_value_set_uint(value, index->priv->high);
        break;
    case PROP_NODE_ID:
        g_value_set_uint(value, index->priv->node);
        break;
    case PROP_INDEX:
        g_value_set_uint(value, index->priv->index);
        break;
    case PROP_SUBINDEX:
        g_value_set_uint(value, index->priv->subindex);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void node_index_object_class_init(NodeIndexObjectClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    g_type_class_add_private(klass, sizeof(NodeIndexObjectPrivate));

    object_class->finalize     = node_index_object_finalize;
    object_class->set_property = node_index_object_set_property;
    object_class->get_property = node_index_object_get_property;

    g_object_class_override_property(object_class, PROP_INDEX_OBJECT_ID, "indexid");
    g_object_class_override_property(object_class, PROP_PARAMETER_NAME, "parametername");
    g_object_class_override_property(object_class, PROP_OBJECT_TYPE, "objecttype");
    g_object_class_override_property(object_class, PROP_DATA_TYPE, "datatype");
    g_object_class_override_property(object_class, PROP_ACCESS_TYPE, "accesstype");
    g_object_class_override_property(object_class, PROP_DEFAULT_VALUE, "defaultvalue");
    g_object_class_override_property(object_class, PROP_PDO_MAPPING, "pdomapping");
    g_object_class_override_property(object_class, PROP_LOW_LIMIT, "lowlimit");
    g_object_class_override_property(object_class, PROP_HIGH_LIMIT, "highlimit");
    g_object_class_override_property(object_class, PROP_NODE_ID, "nodeid");
    g_object_class_override_property(object_class, PROP_INDEX, "index");
    g_object_class_override_property(object_class, PROP_SUBINDEX, "subindex");
}

gchar *node_index_object_dup_str(NodeIndexObject *index) {
    GString *string = g_string_new("");
    g_string_append_printf(string, "[%s]\n", index->priv->id);
    g_string_append_printf(string, "ParameterName=%s\n", index->priv->parameter_name);
    g_string_append_printf(string, "ObjectType=%x\n", index->priv->object_type);
    g_string_append_printf(string, "DataType=%x\n", index->priv->data_type);
    g_string_append_printf(string, "AccessType=%s\n", index->priv->access);
    g_string_append_printf(string, "DefaultValue=%s\n", index->priv->default_value);
    g_string_append_printf(string, "PDOMapping=%u\n", index->priv->pdo_mapping);
    g_string_append_printf(string, "LowLimit=%u\n", index->priv->low);
    g_string_append_printf(string, "HighLimit=%u\n", index->priv->high);
    g_string_append_printf(string, "id=%u\n", index->priv->node);
    g_string_append_printf(string, "index=%u\n", index->priv->index);
    g_string_append_printf(string, "subindex=%u\n", index->priv->subindex);
    if (index->priv->status) g_string_append_printf(string, "ScanStatus=%s\n", index->priv->status);
    gchar *ret = string->str;
    g_string_free(string, FALSE);
    return ret;
}

void node_object_test_print(NodeIndexObject *index) {
    gchar *params         = node_index_object_dup_str(index);
    gchar *internal_value = mkt_value_stringify(index->priv->internal_value);
    g_free(params);
    g_free(internal_value);
}

static gboolean node_index_init_wdata_package(NodeIndexObject *index, unsigned char *wdata) {
    int count = 0;
    wdata[0]  = index->priv->toggle_byte;
    if (index->priv->string_value == NULL) return FALSE;
    if (index->priv->string_value->len < 1) return FALSE;
    gchar *p = index->priv->string_value->str;
    for (count = 1; count < 8 && (p - index->priv->string_value->str) < index->priv->string_value->len; p++, count++) {
        wdata[count] = *p;
    }
    count--;
    switch (count) {
    case 0:
        wdata[0] += (0x0E) + (0x01);
        break;
    case 1:
        wdata[0] += (0x0C) + (0x01);
        break;
    case 2:
        wdata[0] += (0x0A) + (0x01);
        break;
    case 3:
        wdata[0] += (0x08) + (0x01);
        break;
    case 4:
        wdata[0] += (0x06) + (0x01);
        break;
    case 5:
        wdata[0] += (0x04) + (0x01);
        break;
    case 6:
        wdata[0] += (0x02) + (0x01);
        break;
    default:
        break;
    }
    index->priv->string_value = g_string_erase(index->priv->string_value, 0, count < 8 ? count : 7);
    return TRUE;
}

static gboolean node_index_init_rdata_package(NodeIndexObject *index, unsigned char *rdata, unsigned char *wdata) {
    int count = 0;
    wdata[0]  = index->priv->toggle_byte;
    int lang  = rdata[0] & 0x0E;
    switch (lang) {
    case 0x00:
        lang = 7;
        break;
    case 0x02:
        lang = 6;
        break;
    case 0x04:
        lang = 5;
        break;
    case 0x06:
        lang = 4;
        break;
    case 0x08:
        lang = 3;
        break;
    case 0x0A:
        lang = 2;
        break;
    case 0x0C:
        lang = 1;
        break;
    case 0x0E:
        lang = 0;
        break;
    default:
        lang = 0;
        break;
    }

    for (count = 1; count < lang + 1; count++) index->priv->string_value = g_string_append_unichar(index->priv->string_value, rdata[count]);
    if (lang < 7 || (rdata[0] & (0x1)) || (index->priv->len <= index->priv->string_value->len)) {
        gchar *escape = g_strescape(index->priv->string_value->str, NULL);
        g_value_set_string(index->priv->internal_value, escape);
        g_free(escape);
        return FALSE;
    }
    return TRUE;
}

gboolean node_index_object_toggle(NodeIndexObject *index, unsigned char *rdata, unsigned char *wdata) {
    g_return_val_if_fail(index, FALSE);
    g_return_val_if_fail(rdata, FALSE);
    g_return_val_if_fail(wdata, FALSE);
    g_return_val_if_fail(index->priv->g_type == G_TYPE_STRING, FALSE);
    memset(wdata, 0, sizeof(char) * 8);

    switch (rdata[0]) {
    case 0x41:
        if (index->priv->string_value) g_string_free(index->priv->string_value, TRUE);
        index->priv->string_value = g_string_new("");
        index->priv->toggle_byte  = 0x60;
        index->priv->len          = rdata[4] + (rdata[5] << 8);
        if (index->priv->len > 127) {
            node_trace("LEN=%d\n", index->priv->len);
        }
        if (index->priv->len < 1) {
            g_value_set_string(index->priv->internal_value, index->priv->string_value->str);
            return FALSE;
        }
        wdata[0] = index->priv->toggle_byte;
        break;
    case 0x60:
        index->priv->toggle_byte = 0x00;
        index->priv->len         = 0;
        if (index->priv->string_value) g_string_free(index->priv->string_value, TRUE);
        index->priv->string_value = g_string_new(g_value_get_string(index->priv->internal_value));
        if (index->priv->string_value->len < 1) return FALSE;
        if (!node_index_init_wdata_package(index, wdata)) return FALSE;
        break;
    default:
        if ((rdata[0] < 0x20)) {
            if (!node_index_init_rdata_package(index, rdata, wdata)) return FALSE;
        } else if ((rdata[0] & (0x20))) {

            if (!node_index_init_wdata_package(index, wdata)) return FALSE;
        }
        break;
    }
    index->priv->toggle_byte ^= 0x10;
    return TRUE;
}

gchar *node_index_dup_value(NodeIndexObject *index) {
    g_return_val_if_fail(index, NULL);
    g_return_val_if_fail(NODE_IS_INDEX_OBJECT(index), NULL);
    return mkt_value_stringify(index->priv->internal_value);
}

/*
// Wurde nur in 'node_device_object_look_for_eds' benutzt. Diese Funktion ist
// aber gestrichen.
void node_index_object_status(NodeIndexObject *index, const gchar *format, ...) {
    g_return_if_fail(index != NULL);
    g_return_if_fail(NODE_IS_INDEX_OBJECT(index));
    va_list args;
    gchar * msg;
    va_start(args, format);
    msg = g_strdup_vprintf(format, args);
    va_end(args);
    if (index->priv->status) g_free(index->priv->status);
    index->priv->status = msg;
}
*/

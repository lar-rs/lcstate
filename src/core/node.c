/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-measurement-data.c
 * Copyright (C) LAR
 *
 */

#include "node-object.h"
#include "node-index-object.h"
#include "node-device-object.h"
#include "node-control-app-object.h"

// GS: Erforderlich. Sonst für gibt es für
// GTypeCValue      _cvalue = { 0, };
// die Meldung: error: variable ‘_cvalue’ has initializer but incomplete type
#include "gobject/gvaluecollector.h"

#include "../config.h"


#include <glib/gi18n-lib.h>

enum
{
	PROP_0,
	PROP_NODE_ID,
	PROP_NODE_DEVICE,
	PROP_NODE_COFIGURATION,
	PROP_NODE_VENDOR_NAME,
	PROP_NODE_VENDOE_NUMBER,
	PROP_NODE_PRODUCT_NAME,
	PROP_NODE_PRODUCT_NUMBER,
	PROP_NODE_REVISION_NUMBER,
	PROP_NODE_ORDER_CODE
};


//     Die Klasse 'NodeObject' ist Basisklasse für dir Klassen
// 'NodeAnalogObject', 'NodeMotor3Object', 'NodeMotorObject',
// 'NodeDigitalObject' und 'NodeAnalogExtObject'.
//     Die Liste 'GList* lindex' und die Streuwerttabelle 'GHashTable* indexes'
// enthalten Objekte der Klasse 'NodeIndexObject'.
struct _NodeObjectPrivate
{
	gchar*      node_configuration;
	gchar*      vendor_name;
	guint       vendor_number;
	gchar*      product_name;
	guint       product_number;
	guint       revision_number;
	gchar*      order_code;
	GHashTable* indexes;
	GList*      lindex;
	guint       reset_couner;
	guint       reseted_tag;
};


G_DEFINE_TYPE_WITH_PRIVATE (NodeObject, node_object, NODES_TYPE_OBJECT_SKELETON);


static NodeIndexObject* node_object_index_read_value (NodeObject* object, const gchar* index_id)
{
	const gchar* address;
	GObject*     device;
	GObject*     index;

	g_return_val_if_fail (object,                 NULL);
	g_return_val_if_fail (index_id,               NULL);
	g_return_val_if_fail (NODE_IS_OBJECT(object), NULL);

	index = g_hash_table_lookup (object->priv->indexes, index_id);

	if (! index) {
		g_warning ("Index %s not found", index_id);
		return NULL;
	}

	address = nodes_simple_get_device (nodes_object_get_simple (NODES_OBJECT (object)));
	device  = G_OBJECT(g_dbus_object_manager_get_object(node_control_app_device_manager(),address));

	if (! device)
		return NULL;

	if (node_device_object_read_index (NODE_DEVICE_OBJECT(device), NODE_INDEX(index)))
		return NODE_INDEX_OBJECT (index);

	return NULL;
}

static gchar* node_object_index_value (NodeObject* node_obj, const gchar* index_id)
{
	NodeIndexObject* index = NODE_INDEX_OBJECT(node_object_index_read_value (node_obj, index_id));

	if (index)
		return node_index_dup_value (index);

	return NULL;
}

static const gchar*
node_object_index_gtype ( NodeObject *node_obj , const gchar *index_id )
{
	g_return_val_if_fail(node_obj != NULL , NULL);
	g_return_val_if_fail(NODE_IS_OBJECT(node_obj) , NULL);
	GObject *index = g_hash_table_lookup(node_obj->priv->indexes,index_id);
	if(!index)return NULL;
	GType type = node_index_value_type(NODE_INDEX(index));
	return g_type_name(type);
}


static gboolean
node_object_set_index_value ( NodeObject *node_obj , const gchar *index_id, const gchar *value )
{
	g_return_val_if_fail(node_obj != NULL , FALSE);
	g_return_val_if_fail(NODE_IS_OBJECT(node_obj) , FALSE);
	GObject *index = g_hash_table_lookup(node_obj->priv->indexes,index_id);
	if(!index)return FALSE;
	if(!node_index_set_valueStr(NODE_INDEX(index),value))	return FALSE;
	const gchar *address = nodes_simple_get_device(nodes_object_get_simple(NODES_OBJECT(node_obj)));
	GObject *device = G_OBJECT(g_dbus_object_manager_get_object(node_control_app_device_manager(),address));
	if(!device) return FALSE;

	if(node_device_object_write_index(NODE_DEVICE_OBJECT(device),NODE_INDEX(index)))
	{
		return TRUE;
	}
	return FALSE;
}


static GParamSpec* find_interface_pspec (const gchar* pname)
{
	guint       ni;
	GType*      iface;
	int         i;
	GParamSpec* pspec = NULL;

	iface = g_type_interfaces (NODE_TYPE_INDEX_OBJECT, &ni);

	for (i=0; i<ni; i++) {
		gpointer itable = g_type_default_interface_ref (iface [i]);

		if (itable) {
			pspec = g_object_interface_find_property (itable, pname);

			g_type_default_interface_unref (itable);

			if (pspec)
				break;
		}
	}

	g_free (iface);

	return pspec;
}


// Vormals: mkt_utils_keyfile_create_parameter
//
//    Diese Funktion erzeugt einen Array von 'GParameter' Objekten. Jeder Para-
// meter hat einen der Namen
// - indexid    parametername objecttype datatype accesstype defaultvalue
// - pdomapping lowlimit      highlimit  nodeid   index      subindex
// Diese Namen sind die Namen der Eigenschaften der Klasse 'NodeIndexObject'.
// Gesetzt werden nur Namen, die sich aus einem kleingeschriebenen Schlüssel
// aus der Gruppe 'group' aus der Datei mit dem Namen aus 'eds_fspec' ergeben.
// Der Wert jedes 'GParameter' Objektes wird aus dem zum Schlüssel gehörenden
// Wert ermittelt.
//    Die ersten 'start' Elemente des Arrays bleiben frei (== NULL).
static GParameter* keyfile_create_parameter (const gchar* eds_fspec, const gchar* group, gsize start, gsize* plen)
{
	GError*     error   = NULL;
	GKeyFile*   keyfile = g_key_file_new ();
	gsize       len;
	int         npar    = start;
	GParameter* param   = NULL;

	* plen  = 0;
	keyfile = g_key_file_new ();

	if (! keyfile)
		return NULL;

	if (! g_key_file_load_from_file (keyfile, eds_fspec, G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS, &error)) {
		if (error) {
			g_warning    ("Key file load from file %s error : %s ", eds_fspec, error->message);
			g_error_free (error);
		}
		else
			g_warning ("Key file load from file %s error : unknown ", eds_fspec);
	}
	else {
		gchar** keys = g_key_file_get_keys (keyfile, group, &len, &error);

		if (!keys || error) {
			if (error) {
				g_warning    ("Key file sort keys  error : %s ", error->message);
				g_error_free (error);
			}
			else
				g_warning ("Key file sort keys  error : unknown ");
		}
		else {
			int i;

			param = calloc (sizeof(GParameter), start+len);

			if (param) {
				// Traversiert alle Schlüssel der vorgegebenen Gruppe.
				for (i=0; i<len && keys[i]; i++) {
					gchar* val;

					error = NULL;
					val   = g_key_file_get_value (keyfile, group, keys[i], &error);

					if (! val)
						g_warning ( "Key file get value error : %s ",error!=NULL?error->message:"unknown");

					else {
						GParamSpec* pspec;
						GString*    string = g_string_new (keys [i]);

						string = g_string_ascii_down (string);

						//    Sucht für den aktuellen Schlüssel eine Eigenschaft
						// der Klasse 'NodeIndexObject' deren Name der kleinge-
						// schriebene Name des Schlüssels ist. Ein Objekt vom
						// Typ 'GParamSpec', das zu dem Namen auch den Typ lie-
						// fert, wird zurückgegeben.
						//    Die Klasse 'NodeIndexObject' hat folgende Eigen-
						// schaften:
						// - indexid    parametername objecttype datatype
						// - accesstype defaultvalue  pdomapping lowlimit
						// - highlimit  nodeid        index      subindex
						pspec = find_interface_pspec (string -> str);

						if (pspec) {
							GValue* value = mkt_value_new (pspec -> value_type);

							if (! mkt_set_gvalue_from_string (value, val)) {
								value = NULL;
								g_warning ("Object value %s:%s transform failed ", pspec->name, val?val:"NULL");
							}
							else {
								// Hier ist ein 'GParamSpec' Objekt gefunden
								// worden, das eine Eigenschaft bezeichnet, des-
								// sen Name der kleingeschriebene Name des aktu-
								// ellen Schlüssels ist und das den dazugehöri-
								// gen Typ liefert.
								param [npar] . name = pspec -> name;

								g_value_init (&param[npar].value, value->g_type);
								g_value_copy (value, &param[npar].value);

								npar++;
							}

							mkt_value_free(value);
						}

						g_free        (val);
						g_string_free (string,TRUE);
					}
				}

				if (! npar) {
					g_free (param);
					param = NULL;
				}
				else
				* plen = npar;
			}

			if (keys)
				g_strfreev (keys);
		}
	}

	if (keyfile)
		g_key_file_free (keyfile);

	return param;
}


static void unset_parameter (GParameter* src, gsize len)
{
	int i;

	if (!src || !len)
		return;

	for (i=0; i<len; i++)
		if (src[i].value.g_type > G_TYPE_NONE)
			g_value_unset (& src [i] . value);
}


//    Diese Funktion schreibt zwei 'GParameter' Objekte in die ersten beiden
// Elemente des Arrays 'params'. Die Parameter bekommen die Werte:
//    Name: "indexid" "nodeid"
//    Typ:  gpointer  gint
//    Wert: group     0
gboolean new_index_objekt_parameter (const gchar* group, GParameter* params)
{
	GTypeCValue      _cvalue = { 0, };
	GTypeValueTable* _vtab;
	gchar*           error   = NULL;
	const gchar*     name;
	GParamSpec*      pspec;

	name  = "indexid";
	pspec = find_interface_pspec (name);

	if (!pspec)
		g_warning ("%s: object class `%s' has no property named `%s'", G_STRFUNC, g_type_name (NODE_TYPE_INDEX_OBJECT), name);

	else {
		params [0] . name           = pspec -> name;
		_vtab                       = g_type_value_table_peek (pspec -> value_type);
		params [0] . value . g_type = pspec -> value_type;
		_cvalue . v_pointer         = (gpointer) group;

		error = _vtab -> collect_value (&params[0].value, 1, &_cvalue, 0);

		if (error) {
			g_warning     ("%s: %s", G_STRFUNC, error);
			g_free        (error);
			g_value_unset (& params [0] . value);
		}
		else {

			name = "nodeid";

			memset (&_cvalue, 0, sizeof _cvalue);
			pspec = find_interface_pspec (name);

			if (! pspec)
				g_warning ("%s: object class `%s' has no property named `%s'", G_STRFUNC, g_type_name (NODE_TYPE_INDEX_OBJECT), name);

			else {
				params [1] . name           = pspec -> name;
				_vtab                       = g_type_value_table_peek (pspec -> value_type);
				params [1] . value . g_type = pspec -> value_type;
				_cvalue . v_int             = 0;

				error = _vtab -> collect_value (&params[1].value, 1, &_cvalue, 0);

				if (error) {
					g_warning     ("%s: %s", G_STRFUNC, error);
					g_free        (error);
					g_value_unset (& params [1] . value);
				}
			}
		}
	}

	return TRUE;
}


// mkt_utils_keyfile_new_objectv
static GObject* new_index_objekt_key_v (const gchar* eds_fspec, const gchar* group)
{
	gsize       param_count = 0;
	GObject*    object      = NULL;
	GParameter* user_param  = NULL;

	//    'keyfile_create_parameter' erzeugt einen Array von 'GParameter' Objek-
	// ten. Jeder Parameter hat einen der Namen
	// - parametername objecttype datatype  accesstype defaultvalue
	// - pdomapping    lowlimit   highlimit index      subindex
	// Diese Namen sind die Namen der Eigenschaften der Klasse
	// 'NodeIndexObject' mit Ausnahme von "indexid" und "nodeid". Gesetzt werden
	// nur Namen, die sich aus einem kleingeschriebenen Schlüssel aus der Gruppe
	// 'group' aus der Datei mit dem Namen aus 'eds_fspec' ergeben. Der Wert je-
	// des 'GParameter' Objektes wird aus dem zum Schlüssel gehörenden Wert er-
	// mittelt.
	//    Die ersten 2 Elemente des Arrays bleiben frei (== NULL).
	//    Parameter mit den Namen der Eigenschaften "indexid" und "nodeid" wer-
	// den in der Funktion 'new_index_objekt_parameter' erzeugt.
	user_param = keyfile_create_parameter (eds_fspec, group, 2, &param_count);

	//    Diese Funktion schreibt zwei 'GParameter' Objekte in die ersten beiden
	// Elemente des Arrays 'params'. Die Parameter bekommen die Werte:
	//    Name: "indexid" "nodeid"
	//    Typ:  gpointer  gint
	//    Wert: group     0
	if (user_param && new_index_objekt_parameter (group, user_param)) {
		//    'g_object_newv' erzeugt hier ein Objekt vom Typ 'NodeIndexObject'.
		// Die Eigenschaft "indexid" wird auf den Namen der Gruppe gesetzt.
		// Die Eigenschaft "nodeid"  wird auf 0 gesetzt. Die weiteren Eigen-
		// schaften "parametername" und "objecttype" werden immer in der .eds-
		// Datei gefunden und gesetzt; die Eigenschaften "objecttype",
		// "datatype", "accesstype", "defaultvalue", "pdomapping", "lowlimit",
		// "highlimit", "index" und "subindex" werden manchmal gesetzt, manchmal
		// nicht.
		object = g_object_newv (NODE_TYPE_INDEX_OBJECT, param_count, user_param);
		unset_parameter (user_param, param_count);
	}

	if (user_param)
		g_free (user_param);

	return object;
}


//     Einziger Aufruf dieser Funktion in dieser Datei in
// 'node_object_set_property' mit 'prop_id' == 'PROP_NODE_COFIGURATION', also
// wenn die Eigenschaft "node-configuration" gesetzt wird.
//
//     Diese Funktion liest die .eds-Datei ein, deren Pfad in
// 'node->priv->node_configuration' steht.
//
//     Eine .eds-Datei ist eine Textdatei, die folgendermaßen gegliedert ist:
// sie besteht aus benannten Gruppen, die mit dem in '[]' geklammerten Gruppen-
// namen anfangen. Die Bedeutung der Zeilen vor dem ersten Gruppennamen ist un-
// klar, wahrscheinlich werden die Zeilen ignoriert. Nach einem Gruppennamen
// folgen Schlüssel-Wert Paare, wobei der Schlüssel links steht und von einem
// '=' gefolgt wird. Hinter dem '=' steht der Wert. Ob Leerzeichen ggf. igno-
// riert werden, ist unklar. Zeilen, die mit einem '#' anfangen, werden igno-
// riert. Ob ein '#' innerhalb einer Zeile eine Auswirkung hat, ist unklar.
//
//     'node_object_load_indexes_table' generiert für jede Gruppe aus der .eds-
// Datei, die den Schlüssel "DataType" konvertierbar auf 'guint' und den Schlüs-
// sel "AccessType" (die Konvertierung auf 'gchar*' ist immer möglich) enthält
// ein Objekt vom Typ 'NodeIndexObject'.
//     In diesem Objekt wird die Eigenschaft "indexid" auf den Namen der Gruppe
// gesetzt. In diesem Namen sind die Werte für "index" und "subindex" unterge-
// bracht. Wenn z. B. "indexid" den Wert 1018sub1 hat, so wird der Wert für
// "index" auf 0x1018 und der Wert für "subindex" auf 1 gesetzt. Für Werte ohne
// "sub" wird "subindex" auf 0 gesetzt. Die Eigenschaft "nodeid"  wird auf 0 ge-
// setzt. Die weiteren Eigenschaften "parametername" und "objecttype" werden im-
// mer in der .eds-Datei gefunden und gesetzt; die Eigenschaften "objecttype",
// "datatype", "accesstype", "defaultvalue", "pdomapping", "lowlimit" und
// "highlimit" werden manchmal gesetzt, manchmal nicht.
//     Die Adresse jedes dieser Objekte wird in der Liste 'node->priv->lindex'
// und unter dem Schlüssel des Gruppennamens, aus dem dies Objekt generiert wur-
// de, in der Streuwerttabelle 'GHashTable node->priv->indexes' abgelegt.
static void node_object_load_indexes_table (NodeObject* node)
{
	GKeyFile* keyfile;  // Bekommt die Werte der aktuellen .eds-Datei zugewisen.
	GError*   error = NULL;

	g_return_if_fail (node -> priv -> node_configuration);
	g_return_if_fail (g_file_test (node->priv->node_configuration, G_FILE_TEST_EXISTS));

	if (node -> priv -> indexes)
		g_hash_table_destroy (node -> priv -> indexes);

	node -> priv -> indexes = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_object_unref);

	if (node -> priv -> lindex)
		g_list_free (node -> priv -> lindex);

	keyfile = g_key_file_new ();

	//    'node->priv->node_configuration' (Wert der Eigenschaft
	// "node-configuration") ist gerade neu gesetzt worden, und zwar auf einen
	// der Werte
	// - "/etc/candaemon/eds/Scan.eds"
	// - "/etc/candaemon/eds/AnalogExt.eds"
	// - "/etc/candaemon/eds/Digital.eds"
	// - "/etc/candaemon/eds/lar_analognode_v15.eds"
	// - "/etc/candaemon/eds/lar_doppelmotornode_v12.eds"
	// - "/etc/candaemon/eds/lar_doppelmotornode_v13.eds"
	if (! g_key_file_load_from_file (keyfile, node->priv->node_configuration, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error)) {
		g_warning ("Key file load from file %s error : %s ", node->priv->node_configuration, error?error->message:"unknown");

		if (error)
			g_error_free (error);
	}
	else {
		gsize   glen   = 0;
		gchar** groups = g_key_file_get_groups(keyfile,&glen);

		if (glen>0 && groups) {
			gint i = 0 ;

			// Traversiere alle Gruppen aus der .eds-Datei.
			for (i=0; i<glen && groups[i]; i++) {
				gboolean is_index = TRUE;
				gchar*   p        = groups [i];
				int      j        = 0;

				for (; j<4 && *p; j++, p++)
					if (! g_ascii_isxdigit (*p))
						is_index = FALSE;

				// Next check group parameter ..
				if (is_index) {
					GValue* data_type = mkt_keyfile_get (node->priv->node_configuration, groups[i], "DataType",   G_TYPE_UINT);
					GValue* accept    = mkt_keyfile_get (node->priv->node_configuration, groups[i], "AccessType", G_TYPE_STRING);

					is_index = data_type && accept;

					if (data_type)
						mkt_value_free (data_type);

					if (accept)
						mkt_value_free (accept);
				}

				if (is_index) {
					//    'new_index_objekt_key_v' erzeugt ein Objekt vom Typ
					// 'NodeIndexObject'. Die Werte der Eigenschaften s. o.
					GObject* index = new_index_objekt_key_v (node->priv->node_configuration, groups[i]);

					//     'node_index_indexid' ruft die Funktion, deren Adresse
					// in dem Element 'indexid' der Schnittstelle
					// 'NodeIndexInterface' steht, auf. Da es sich um ein Objekt
					// der Klasse 'NodeIndexObject' handelt, wurde diese Adresse
					// in 'node_index_object_init_index_interface' auf die Funk-
					// tion 'node_index_object_get_indexid' gesetzt.
					// 'node_index_object_get_indexid' nun liefert das Element
					// 'id' aus dem privaten Teil 'NodeIndexObjectPrivate' der
					// Klasse 'NodeIndexObject'.
					//     Das Element 'id' aus 'NodeIndexObjectPrivate' wird
					// auf den Wert der Eigenschaft "indexid" gesetzt.
					//     Zusammenfassend gilt:
					// 'node_index_indexid(NODE_INDEX(index))' liefert
					// 'groups [i]'.
					//
					//     Die Adresse des 'NodeIndexObject' Objektes wird in
					// der Liste 'node->priv->lindex' und unter dem Schlüssel
					// 'groups [i]' in der Streuwerttabelle 'node->priv->lindex'
					// agelegt.
					g_hash_table_insert (node->priv->indexes, (gpointer)node_index_indexid(NODE_INDEX(index)), (gpointer)index);

					node->priv->lindex = g_list_append (node->priv->lindex,index);
				}
			}

			g_strfreev (groups);
		}
	}
}

static gboolean
node_object_get_indexed_value (NodesSimple *interface,
		                     GDBusMethodInvocation *invocation,
							 const gchar *index,
							 gpointer user_data)
{
	NodeObject  *data = NODE_OBJECT(user_data);
	gchar *value = node_object_index_value(data,index);
	if(value==NULL)
	{
		 g_dbus_method_invocation_return_dbus_error (invocation,
		                                                  "com.lar.nodes.simple.GetIndexValue",
		                                                  "Index value not found");
		return TRUE;
	}
	const gchar *type  = node_object_index_gtype(data,index);
	g_dbus_method_invocation_return_value (invocation,g_variant_new ("(s,s)", value,type));
	g_free(value);
	return TRUE;
}


static gboolean
node_object_set_indexed_value (NodesSimple *interface,
		                       GDBusMethodInvocation *invocation,
							   const gchar *index,
							   const gchar *value,
							   gpointer user_data)
{
	NodeObject  *data = NODE_OBJECT(user_data);
	gboolean done  = node_object_set_index_value(data,index,value);
	g_dbus_method_invocation_return_value (invocation,g_variant_new ("(b)", done));
	return TRUE;
}

static gboolean
node_object_get_indexes (NodesSimple *interface,
		                 GDBusMethodInvocation *invocation,
						 gpointer user_data)
{
	NodeObject  *data = NODE_OBJECT(user_data);
	GString *str = g_string_new("");
	GList *l = NULL;
	for(l=data->priv->lindex;l!=NULL;l=l->next)
	{
		if(l->data && NODE_IS_INDEX_OBJECT(l->data))
		{
			g_string_append_printf(str,"%s%s",str->len>1?",":"",node_index_indexid(NODE_INDEX(l->data)));
		}
	}
	g_dbus_method_invocation_return_value (invocation,g_variant_new ("(s)", str->str));

	return TRUE;
}

static gboolean
node_object_read_all (NodesSimple *interface,
		              GDBusMethodInvocation *invocation,
					  const gchar *index,
					  gpointer user_data)
{
	 g_dbus_method_invocation_return_dbus_error (invocation,
			                                                  "com.lar.nodes.simple.ReadAll",
			                                                  "Method not ready programmed.");

	return TRUE;
}

static void node_object_simple_change_id (GObject* object, GParamSpec* pspec, gpointer user_data)
{
	guint       id;
	GList*      l = NULL;
	NodeObject* node_object;

	id          = nodes_simple_get_node_id (NODES_SIMPLE (object));
	node_object = NODE_OBJECT              (user_data);

	for (l=node_object->priv->lindex; l; l=l->next)
		if (l->data && NODE_IS_INDEX_OBJECT(l->data))
			g_object_set (l->data, "nodeid", id, NULL);
}


static void
node_object_init (NodeObject *node_object)
{
	NodeObjectPrivate *priv      = node_object_get_instance_private (node_object);
	priv->vendor_name = NULL;
	priv->product_name= NULL;
	priv->order_code  = NULL;
	priv->node_configuration= NULL;
	priv->indexes = g_hash_table_new_full(g_str_hash,g_str_equal,NULL,g_object_unref);
	node_object->priv   = priv;
	/* TODO: Add initialization code here */
}

static void
node_object_constructed ( GObject *object )
{
	if(G_OBJECT_CLASS (node_object_parent_class)->constructed)G_OBJECT_CLASS (node_object_parent_class)->constructed (object);
	NodeObject  *data = NODE_OBJECT(object);
	NodesSimple *simple = nodes_simple_skeleton_new();
	nodes_object_skeleton_set_simple(NODES_OBJECT_SKELETON(object),simple);
	g_signal_connect(simple,"handle-get-index-value", G_CALLBACK(node_object_get_indexed_value), data);
	g_signal_connect(simple,"handle-set-index-value", G_CALLBACK(node_object_set_indexed_value), data);
	g_signal_connect(simple,"handle-get-index",       G_CALLBACK(node_object_get_indexes),       data);
	g_signal_connect(simple,"handle-read-all",        G_CALLBACK(node_object_read_all),          data);
	g_signal_connect(simple,"notify::node-id",        G_CALLBACK(node_object_simple_change_id),  data);

	if(data->priv->node_configuration)
	{
		GValue *description = mkt_keyfile_get(data->priv->node_configuration,"FileInfo","Description",G_TYPE_STRING);
		if(description)
		{
			nodes_simple_set_name(simple,g_value_get_string(description));
			mkt_value_free(description);
		}
	}

	//     GS: Stand vorher in 'node_object_set_property' im case
	// 'PROP_NODE_COFIGURATION'.
	//     Die Funktion 'node_object_load_indexes_table' liest die .eds-Datei
	// ein, deren Pfad jetzt in 'node->priv->node_configuration' steht und gene-
	// riert nach der dort vorgefundenen Beschreibung eine Reihe von Objekten
	// der Klasse 'NodeIndexObject', die in der Streuwerttabelle
	// 'GHashTable node->priv->indexes' und in der Liste 'node->priv->lindex'
	// gespeichert werden.
	node_object_load_indexes_table (data);
	g_object_unref                 (simple);
}

static void
node_object_finalize (GObject *object)
{
	NodeObject *data = NODE_OBJECT(object);
	if(data->priv->indexes) g_hash_table_destroy(data->priv->indexes);
	G_OBJECT_CLASS (node_object_parent_class)->finalize (object);
}




static void
node_object_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (NODE_IS_OBJECT (object));
	NodeObject *data = NODE_OBJECT(object);
	switch (prop_id)
	{
	//    Die Eigenschaft "node-configuration" wird nur in der Datei
	// 'can-daemon/src/node-device-object.c' in den Funktionen
	// 'node_device_object_create_scan_node' mit dem Aufruf
	// 'NodesObjectSkeleton *node     = NODES_OBJECT_SKELETON(g_object_new(NODE_TYPE_OBJECT, "g-object-path", "/com/lar/nodes/Scan", "node-configuration", etc_path, NULL));'
	// und 'node_device_object_create_new_node' mit dem Aufruf
	// 'NodesObjectSkeleton *node = NODES_OBJECT_SKELETON(g_object_new(node_type, "g-object-path", path, "node-configuration", eds_config, NULL));'
	// gesetzt.
	//    Im ersten Fall ist der Wert dabei "/etc/candaemon/eds/Scan.eds".
	// Dieser Wert ist zwar von hinten durch die Brust ins Auge teilweise (näm-
	// lich "/usr/etc") im Makefile festgelegt, aber er ist trotzdem konstant.
	//    Im zweiten Fall ist der Wert dabei auf einen der Werte
	// - "/etc/candaemon/eds/AnalogExt.eds"
	// - "/etc/candaemon/eds/Digital.eds"
	// - "/etc/candaemon/eds/lar_analognode_v15.eds"
	// - "/etc/candaemon/eds/lar_doppelmotornode_v12.eds"
	// - "/etc/candaemon/eds/lar_doppelmotornode_v13.eds"
	// - "/etc/candaemon/eds/Scan.eds"                    | Aufgerufen mit NODE_TYPE_OBJECT
	// gesetzt.
	case PROP_NODE_COFIGURATION:
		if(data->priv->node_configuration!= NULL)g_free(data->priv->node_configuration);
		data->priv->node_configuration = g_value_dup_string(value);
		// Früher stand hier die Funktion 'node_object_load_indexes_table'.
		// Jetzt steht sie in 'node_object_constructed'.
		break;
	case PROP_NODE_VENDOR_NAME:
		if(data->priv->vendor_name!= NULL)g_free(data->priv->vendor_name);
		data->priv->vendor_name = g_value_dup_string(value);
		break;
	case PROP_NODE_VENDOE_NUMBER:
		data->priv->vendor_number = g_value_get_uint(value);
		break;
	case PROP_NODE_PRODUCT_NAME:
		if(data->priv->product_name!= NULL)g_free(data->priv->product_name);
		data->priv->product_name = g_value_dup_string(value);
		break;
	case PROP_NODE_PRODUCT_NUMBER:
		data->priv->product_number = g_value_get_uint(value);
		break;
	case PROP_NODE_REVISION_NUMBER:
		data->priv->revision_number = g_value_get_uint(value);
		break;
	case PROP_NODE_ORDER_CODE:
		if(data->priv->order_code!= NULL)g_free(data->priv->order_code);
		data->priv->order_code = g_value_dup_string(value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
node_object_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (NODE_IS_OBJECT (object));
	NodeObject *data = NODE_OBJECT(object);
	switch (prop_id)
	{
	case PROP_NODE_COFIGURATION:
		g_value_set_string(value , data->priv->node_configuration);
		break;
	case PROP_NODE_VENDOR_NAME:
		g_value_set_string(value , data->priv->vendor_name);
		break;
	case PROP_NODE_VENDOE_NUMBER:
		g_value_set_uint(value , data->priv->vendor_number);
		break;
	case PROP_NODE_PRODUCT_NAME:
		g_value_set_string(value , data->priv->product_name);
		break;
	case PROP_NODE_PRODUCT_NUMBER:
		g_value_set_uint(value , data->priv->product_number);
		break;
	case PROP_NODE_REVISION_NUMBER:
		g_value_set_uint(value , data->priv->revision_number);
		break;
	case PROP_NODE_ORDER_CODE:
		g_value_set_string(value , data->priv->order_code);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
node_object_class_init (NodeObjectClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	//MktDbusObjectClass* parent_class = MKT_DBUS_OBJECT_CLASS (klass);
	object_class->finalize     = node_object_finalize;
	object_class->set_property = node_object_set_property;
	object_class->get_property = node_object_get_property;
	object_class->constructed  = node_object_constructed;

	g_object_class_install_property (object_class,PROP_NODE_ID,
			g_param_spec_int  ("node-id",
					"Node id",
					"Node id",
					0,G_MAXINT32,0,
					G_PARAM_WRITABLE | G_PARAM_READABLE  ));
	g_object_class_install_property (object_class,PROP_NODE_COFIGURATION,
			g_param_spec_string  ("node-configuration",
					"Node configuration ",
					"Node configuration ",
					"/etc/candaemon/file.conf",
					G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY ));

}

const gchar*
node_object_get_eds_path                  ( NodeObject *object )
{
	g_return_val_if_fail(object !=NULL , NULL );
	g_return_val_if_fail(NODE_IS_OBJECT(object) , NULL );
	return object->priv->node_configuration;
}

gint
sort_on_index (gconstpointer a,gconstpointer  b )
{
	g_return_val_if_fail(a !=NULL , 0 );
	g_return_val_if_fail(b !=NULL , 0 );
	if(node_index_index(NODE_INDEX(a))>node_index_index(NODE_INDEX(b))) return  1;
	if(node_index_index(NODE_INDEX(a))<node_index_index(NODE_INDEX(b))) return -1;
	return 0;
}

/*
// Wurde nur in 'node_device_object_look_for_eds' aufgerufen. Die Funktion wurde
// gestrichen.
void node_object_dup_to_file (NodeObject* object, const gchar* adjust, gboolean done)
{
	GString*     file_path;
	guint        id;
	GList*       indexes;
	GList*       l;
	NodesSimple* simple;
	GString*     string;

	g_return_if_fail (object);
	g_return_if_fail (NODE_IS_OBJECT (object));

	string  = g_string_new            ("");
	indexes = g_hash_table_get_values (object -> priv -> indexes);
	simple  = nodes_object_get_simple (NODES_OBJECT (object));
	id      = 1;

	if (simple)
		id = nodes_simple_get_node_id (simple);

	indexes = g_list_sort (indexes, sort_on_index);

	g_string_append_printf (string, "[FileInfo]\n");
	g_string_append_printf (string, "Created:%s\n",  market_db_get_date_sqlite_format(market_db_time_now()));
	g_string_append_printf (string, "AdjustTo:%s\n", adjust);
	g_string_append_printf (string, "NodeId:%0x\n",  id);
	g_string_append_printf (string, "\n\n");

	l = NULL;

	for (l=indexes; l; l=l->next) {
		gchar* indexDup = node_index_object_dup_str (NODE_INDEX_OBJECT (l -> data));

		g_string_append_printf (string, "%s\n", indexDup);
		g_free                 (indexDup);
	}

	g_list_free (indexes);

	file_path = g_string_new ("/var/log/tera");

	g_mkdir_with_parents   (file_path->str, 0755);
	g_string_append_printf (file_path,      "/%0x-scan-%s.%s", id, done?"done":"fail", adjust);

	file_path = g_string_ascii_down (file_path);

	g_file_set_contents (file_path->str, string->str, string->len, NULL);
	g_string_free       (string,         TRUE);
	g_string_free       (file_path,      TRUE);
}
*/


/*
// Wurde nur in 'node_device_object_scann_idle' und
// 'node_device_object_look_for_eds' sowie in der schon lange nicht mehr genutz-
// ten Funktion 'node_device_object_scan' aufgerufen. Beide ersteren Funktionen
// wurden gestrichen.
GList* node_object_childs_index (NodeObject* node)
{
	return node -> priv -> lindex;
}
*/

GValue* node_object_read_value (NodeObject* object, const gchar* index_id)
{
	NodeIndex* index = NODE_INDEX (node_object_index_read_value (object, index_id));

	if (index)
		return node_index_value (index);

	return NULL;
}

GValue*
node_object_read_value_type_transform( NodeObject *object, const gchar *index_id , GType type)
{
	GValue *value      = mkt_value_new(type);
	GValue *read_val   = node_object_read_value(object,index_id);
	gboolean done = FALSE;
	if(read_val)
	{
		if(g_value_type_compatible(read_val->g_type,type))
		{
			g_value_copy(read_val,value);
			done = TRUE;
		}
		else if(g_value_type_transformable (read_val->g_type, type))
		{
			g_value_transform (read_val, value);
			done = TRUE;
		}
		mkt_value_free(read_val);
	}
	if(!done )
	{
		g_warning("node object read index %s with type transform fail",index_id);
		mkt_value_free(value);	value = NULL;
	}
	return value;
}



gboolean
node_object_write_value              ( NodeObject *object, const gchar *index_id , const GValue *value)
{
	g_return_val_if_fail(object !=NULL , FALSE );
	g_return_val_if_fail(index_id !=NULL , FALSE );
	g_return_val_if_fail(NODE_IS_OBJECT(object) , FALSE );
	g_return_val_if_fail(value !=NULL , FALSE );
	GObject *index = g_hash_table_lookup(object->priv->indexes,index_id);
	if(!index)
	{
		g_warning("Index %s not found",index_id);
		return FALSE;
	}
	if(!node_index_set_value(NODE_INDEX(index),value))
	{
		g_warning("Value can not set");
		return FALSE;
	}
	const gchar *address = nodes_simple_get_device(nodes_object_get_simple(NODES_OBJECT(object)));
	GObject *device = G_OBJECT(g_dbus_object_manager_get_object(node_control_app_device_manager(),address));
	if(!device)return FALSE;
	if(node_device_object_write_index(NODE_DEVICE_OBJECT(device),NODE_INDEX(index)))
	{
		//g_debug("Write index done ");
		return TRUE;
	}
	return FALSE;

}

NodeIndex*
node_object_lookup_index             ( NodeObject *object, const gchar *index_id )
{
	g_return_val_if_fail(object !=NULL , FALSE );
	g_return_val_if_fail(index_id !=NULL , FALSE );
	g_return_val_if_fail(NODE_IS_OBJECT(object) , FALSE );
	NodeIndex *index = NODE_INDEX(g_hash_table_lookup(object->priv->indexes,index_id));
	return index;
}



static gboolean
node_object_reseted_callback ( gpointer user_data )
{
//	gs: wird nicht benutzt.
//	NodeObject *node = NODE_OBJECT(user_data);
	NodesSimple *simple = nodes_object_get_simple( NODES_OBJECT(user_data));
	guint count = nodes_simple_get_reset_count(simple);
	nodes_simple_set_reset_count(simple,(count+1));
	nodes_simple_emit_reseted(simple,nodes_simple_get_reset_count(simple));

	return FALSE;
}

void
node_object_reseted                  ( NodeObject *object )
{
	g_return_if_fail(object !=NULL );
	g_return_if_fail(NODE_IS_OBJECT(object));
	if(object->priv->reseted_tag == 0)
		object->priv->reseted_tag = g_timeout_add_seconds(1,node_object_reseted_callback,object);
}

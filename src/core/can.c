/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-measurement-data.c
 * Copyright (C) LAR
 *
 */

#include <fcntl.h>
#include <glib/gstdio.h>
#include <mktbus.h>
#include <mktlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "node-analog-object.h"
#include "node-analogext-object.h"
#include "node-control-app-object.h"
#include "node-device-object.h"
#include "node-digital-object.h"
#include "node-motor-object.h"
#include "node-motor3-object.h"

#include "../config.h"
#include <glib/gi18n-lib.h>


#if GLIB_CHECK_VERSION(2, 31, 7)
	static GRecMutex init_rmutex;
	#define MUTEX_LOCK()   g_rec_mutex_lock   (&init_rmutex)
	#define MUTEX_UNLOCK() g_rec_mutex_unlock (&init_rmutex)
#else
	static GStaticRecMutex init_mutex = G_STATIC_REC_MUTEX_INIT;
	#define MUTEX_LOCK()   g_static_rec_mutex_lock   (&init_mutex)
	#define MUTEX_UNLOCK() g_static_rec_mutex_unlock (&init_mutex)
#endif

enum {PROP_0, PROP_NODE_BAUTRATE, PROP_NODE_PATH, PROP_NODE_FLAG};

struct _NodeDeviceObjectPrivate {
	NodeObject* scan;
	// - Wird initial (in 'node_device_object_init') auf NULL gesetzt.
	// - Wird in 'node_device_object_create_scan_node' mit
	// 'device -> priv -> scan = NODE_OBJECT (g_object_new (NODE_TYPE_OBJECT, "g-object-path", "/com/lar/nodes/Scan", "node-configuration", etc_path, NULL));'
	// angelegt.

	gboolean is_open;
	// Initial undefiniert. Wird in 'node_device_object_open' beim Start der
	// Funktion auf FALSE, bei Erfolg dann auf TRUE gesetzt.
	// In 'node_device_object_scan', in 'node_device_object_read_index' und in
	// 'node_device_object_write_index' wird als Voraussetzung
	// 'g_return_val_if_fail (device->priv->is_open, FALSE);' angegeben. D. h.:
	// 'node_device_object_scan', 'node_device_object_read_index' und
	// 'node_device_object_write_index' können nur funktionieren, wenn
	// 'node_device_object_open' erfolgreich durchlaufen wurde, also wenn das
	// can-Gerät geöffnet ist.

	gint fd;
	// - Wird initial (in 'node_device_object_init') auf 0 gesetzt.
	// - In 'node_device_object_open' wird das can-Gerät geöffnet und 'fd' auf
	//   den resultierenden Dateibeschreiber gesetzt.
	// - In 'node_read_system_message', int 'node_device_object_read_index' und
	//   in 'node_device_object_write_index' wird mit 'read' von 'fd' gelesen.
	// - In 'node_device_object_read_index', in 'node_device_object_write_index'
	//   und in 'node_device_process_index' wird mit 'write' auf 'fd' geschrie-
	//   ben.
	// - In 'node_device_object_finalize' wird der Dateibeschreiber 'fd' mit
	//   'close' geschlossen.

	GList* noden;
	// - Wird initial (in 'node_device_object_init') auf NULL gesetzt.

	guint read_idle;
};

G_DEFINE_TYPE_WITH_PRIVATE (NodeDeviceObject, node_device_object, CANDEVICE_TYPE_OBJECT_SKELETON);

// Ein Objekt wird in 'node_device_object_scan' allokiert.
typedef struct IterateData {
	NodeDeviceObject* device;
	NodeObject*       scan;
	guint             id;

	GList*            list_to_read;
	// Wird in 'node_device_object_scan' mit
	// 'data -> list_to_read = node_object_childs_index (device -> priv -> scan);'
	// initialisiert. 'list_to_read' bekommt damit alle Objekte der Klasse
	// 'NodeIndexObject' die in den privaten Elementen 'GList* lindex' und
	// 'GHashTable* indexes' abgespeichert sind und die relevaten Einträge aus
	// der Datei "Scan.eds" enthalten.
	// Nachdem 'list_to_read' so initialisiert worden ist, wird mit
	// 'g_timeout_add_full (G_PRIORITY_DEFAULT, 20, node_device_object_scann_idle, device, node_device_object_scann_idle_destroy);'
	// ein Intervallaufruf von 'node_device_object_scann_idle' eingerichtet.

	gint              fd;
	NodeIndex*        index;
}

// Diese Funktion (node_device_object_create_new_node) wird nur in dieser Datei
// in der Funktion 'node_device_create_new_node_from_eds' aufgerufen.

// eds_config =                                        | node_type =         | path =
//    "/etc/candaemon/eds/AnalogExt.eds"               | NodeAnalogExtObject | /com/lar/nodes/Analogext1
//    "/etc/candaemon/eds/Digital.eds"                 | NodeDigitalObject   | /com/lar/nodes/Digital1
//                                                                           | /com/lar/nodes/Digital2 ?warum?
//    "/etc/candaemon/eds/lar_analognode_v15.eds"      | NodeAnalogObject    | /com/lar/nodes/Analog1
//    "/etc/candaemon/eds/lar_doppelmotornode_v12.eds" | NodeMotor3Object    | /com/lar/nodes/Doppelmotor1 |
//    "/etc/candaemon/eds/lar_doppelmotornode_v13.eds" | NodeMotor3Object    | /com/lar/nodes/Doppelmotor2 | Welche von beiden Dateien?
//    "/etc/candaemon/eds/Scan.eds"                    | NodeObject          | /com/lar/nodes/Scann1       | Warum finde ich Scan1?
//
//    "/etc/candaemon/eds/Scan.eds"                    | NodeObject          | /com/lar/nodes/Scan      !Erzeugt in 'node_device_object_create_scan_node'

// Warum 'node_device_object_create_new_node' kein Objekt aus
// "/etc/candaemon/eds/Scan.eds" erzeugt ist nicht klar. Es wird aber ein sol-
// ches Objekt in der obigen Funktion 'node_device_object_create_scan_node'
// erzeugt. In dem Fall stammt die Eigenschaft "g-object-path" nicht aus der
// .eds-Datei, sondern wird mit "/com/lar/nodes/Scan" vorgegeben.

//     Hier wird ein Objekt einer von 'NodeObject' abgeleiteten Klasse erzeugt.
// Das gesamte Objekt wurde in einer .eds-Datei spezifiziert. Bevor diese Funk-
// tion aufgerufen wurde, wurde das 'GType' Objekt der Klasse des zu erzeugenden
// Objektes bereits aus der .eds-Datei ermittelt. Der Name der Klasse steht in
// dem Wert zum Schlüssel "TypeInfo" in der Gruppe "FileInfo". Gleichfalls im
// vorab wurde der dbus-Pfad ermittelt. Der Pfad fängt stets mit
// "/com/lar/nodes/" an. Dann folgt der Wert des Schlüssels "Name" in der Gruppe
// "FileInfo". Zum Schluss folgt noch eine ganze Zahl >= 1.
//
//     Beim Erzeugen werden die Eigenschaften "g-object-path" und
// "node-configuration" gesetzt. "g-object-path" ist zuständig für die Identifi-
// zierung durch den dbus, "node-configuration" ist eine Eigenschaft, die den
// Namen der .eds-Datei trägt. Wenn diese Eigenschaft gesetzt wird, erfolgt die
// weitere Initialisierung des Objektes.
//     Für jede Gruppe aus der .eds-Datei, die den Schlüssel "DataType" konver-
// tierbar auf 'guint' und den Schlüssel "AccessType" (die Konvertierung auf
// 'gchar*' ist immer möglich) enthält, wird ein Objekt vom Typ
// 'NodeIndexObject' erzeugt.
//     In diesem Objekt wird die Eigenschaft "indexid" auf den Namen der Gruppe
// gesetzt. In diesem Namen sind die Werte für "index" und "subindex" unterge-
// bracht. Wenn z. B. "indexid" den Wert 1018sub1 hat, so wird der Wert für
// "index" auf 0x1018 und der Wert für "subindex" auf 1 gesetzt. Für Werte ohne
// "sub" wird "subindex" auf 0 gesetzt. Die Eigenschaft "nodeid" wird auf 0 ge-
// setzt. Die weiteren Eigenschaften "parametername" und "objecttype" werden im-
// mer in der .eds-Datei gefunden und gesetzt; die Eigenschaften "objecttype",
// "datatype", "accesstype", "defaultvalue", "pdomapping", "lowlimit",
// "highlimit", "index" und "subindex" werden manchmal gesetzt, manchmal nicht.
//     Die Adresse jedes dieser Objekte wird in der Liste 'node->priv->lindex'
// mit 'NodeObject node' und unter dem Schlüssel des Gruppennamens, aus dem dies
// Objekt erzeugt wurde, in der Streuwerttabelle
// 'GHashTable node->priv->indexes' mit 'NodeObject node' abgelegt.
//
//     Nachdem das Objekt komplett (inklusive aller dbus Initialisierungen) fer-
// tiggestellt wurde, wird seine Adresse an die Liste 'noden' des privaten Teils
// 'NodeDeviceObjectPrivate' der Klasse 'NodeDeviceObject' angehängt.
/*
// Wurde nur aus 'node_device_create_new_node_from_eds' aufgerufen. Die Funktion
// ist aber gestrichen.
static gboolean node_device_object_create_new_node (NodeDeviceObject* device, GType node_type, const gchar* eds_config, guint id, const gchar* path)
{
	NodesObjectSkeleton* node = NODES_OBJECT_SKELETON (g_object_new (node_type, "g-object-path", path, "node-configuration", eds_config, NULL));

	nodes_simple_set_device             (nodes_object_get_simple(NODES_OBJECT(node)), g_dbus_object_get_object_path(G_DBUS_OBJECT(device)));
	nodes_simple_set_node_id            (nodes_object_get_simple(NODES_OBJECT(node)), id);
	g_dbus_object_manager_server_export (G_DBUS_OBJECT_MANAGER_SERVER(node_control_app_nodes_manager()), G_DBUS_OBJECT_SKELETON(node));
	g_object_unref                      (node);

	device -> priv -> noden = g_list_append (device->priv->noden, node);

	g_message (
		"Connect new node %s ID:%x",
		nodes_simple_get_name    (nodes_object_get_simple (NODES_OBJECT (node))),
		nodes_simple_get_node_id (nodes_object_get_simple (NODES_OBJECT (node)))
	);

	return TRUE;
}
*/

/*
// Wurde nur aus 'node_device_create_new_node_from_eds' aufgerufen. Die Funktion
// ist aber gestrichen.
// 'name' stammt aus Datei eds_config, Gruppe "FileInfo", Schlüssel "Name"
static gchar* create_new_node_path (const gchar* name)
{
    gint fre_addr = 0;

    for (fre_addr=1; fre_addr<125; fre_addr++) {
        gchar*       full_path = g_strdup_printf                  ("/com/lar/nodes/%s%d", name, fre_addr);
        GDBusObject* node      = g_dbus_object_manager_get_object (node_control_app_nodes_manager(), full_path);

        if (! node)
			return full_path;

        g_free (full_path);
    }

    return NULL;
}
*/

// Diese Funktion ('node_device_create_new_node_from_eds') wird nur in dieser
// Datei in der Funktion 'node_device_object_look_for_eds' aufgerufen.

// eds_config =                                        | node_type =         | path =
//    "/etc/candaemon/eds/AnalogExt.eds"               | NodeAnalogExtObject | /com/lar/nodes/Analogext1
//    "/etc/candaemon/eds/Digital.eds"                 | NodeDigitalObject   | /com/lar/nodes/Digital1
//                                                                           | /com/lar/nodes/Digital2 ?warum?
//    "/etc/candaemon/eds/lar_analognode_v15.eds"      | NodeAnalogObject    | /com/lar/nodes/Analog1
//    "/etc/candaemon/eds/lar_doppelmotornode_v12.eds" | NodeMotor3Object    | /com/lar/nodes/Doppelmotor1 |
//    "/etc/candaemon/eds/lar_doppelmotornode_v13.eds" | NodeMotor3Object    | /com/lar/nodes/Doppelmotor2 | Welche von beiden Dateien?
//    "/etc/candaemon/eds/Scan.eds"                    | NodeObject          | /com/lar/nodes/Scann1       | Warum finde ich Scan1?
//
//    "/etc/candaemon/eds/Scan.eds"                    | NodeObject          | /com/lar/nodes/Scan      !Erzeugt in 'node_device_object_create_scan_node'

// Warum 'node_device_object_create_new_node' kein Objekt aus
// "/etc/candaemon/eds/Scan.eds" erzeugt ist nicht klar. Es wird aber ein sol-
// ches Objekt in der obigen Funktion 'node_device_object_create_scan_node'
// erzeugt. In dem Fall stammt die Eigenschaft "g-object-path" nicht aus der
// .eds-Datei, sondern wird mit "/com/lar/nodes/Scan" vorgegeben.

//     Hier wird eine .eds-Datei vorgegeben. Wenn sich in dieser Datei eine
// Gruppe "FileInfo" befindet, die die Schlüssel "TypeInfo" und "Name" enthält,
// wobei der Wert von "TypeInfo" mit 'g_type_from_name' in einen Typindex des
// Typs 'GType' konvertieren lässt und der erhaltene Typ eine von 'NodeObject'
// abgeleitete Klasse bezeichnet, wird hier ein Objekt dieser Klasse erzeugt.
//     Der Wert des Schlüssels "Name" wird benutzt, um den dbus-Pfad des Objek-
// tes zu erzeugen. Der Pfad ergibt sich aus "/com/lar/nodes/" gefolgt von dem
// Wert des Schlüssels "Name" gefolgt von einer ganzen Zahl >= 1.

//     Beim Erzeugen dieses Objektes werden die Eigenschaften "g-object-path"
// und "node-configuration" gesetzt. "g-object-path" ist zuständig für die Iden-
// tifizierung durch den dbus, "node-configuration" ist eine Eigenschaft, die
// den Namen der .eds-Datei trägt. Wenn diese Eigenschaft gesetzt wird, erfolgt
// die weitere Initialisierung des Objektes.
//     Für jede Gruppe aus der .eds-Datei, die den Schlüssel "DataType" konver-
// tierbar auf 'guint' und den Schlüssel "AccessType" (die Konvertierung auf
// 'gchar*' ist immer möglich) enthält, wird ein Objekt vom Typ
// 'NodeIndexObject' erzeugt.
//     In diesem Objekt wird die Eigenschaft "indexid" auf den Namen der Gruppe
// gesetzt. In diesem Namen sind die Werte für "index" und "subindex" unterge-
// bracht. Wenn z. B. "indexid" den Wert 1018sub1 hat, so wird der Wert für
// "index" auf 0x1018 und der Wert für "subindex" auf 1 gesetzt. Für Werte ohne
// "sub" wird "subindex" auf 0 gesetzt. Die Eigenschaft "nodeid" wird auf 0 ge-
// setzt. Die weiteren Eigenschaften "parametername" und "objecttype" werden im-
// mer in der .eds-Datei gefunden und gesetzt; die Eigenschaften "objecttype",
// "datatype", "accesstype", "defaultvalue", "pdomapping", "lowlimit",
// "highlimit", "index" und "subindex" werden manchmal gesetzt, manchmal nicht.
//     Die Adresse jedes dieser Objekte wird in der Liste 'node->priv->lindex'
// mit 'NodeObject node' und unter dem Schlüssel des Gruppennamens, aus dem dies
// Objekt erzeugt wurde, in der Streuwerttabelle
// 'GHashTable node->priv->indexes' mit 'NodeObject node' abgelegt.
//
//     Nachdem das Objekt komplett (inklusive aller dbus Initialisierungen) fer-
// tiggestellt wurde, wird seine Adresse an die Liste 'noden' des privaten Teils
// 'NodeDeviceObjectPrivate' der Klasse 'NodeDeviceObject' angehängt.
//
//     Wenn eine neues Objekt (eine neue Node) erzeugt worden ist, so wird TRUE
// zurückgeliefert, sonst FALSE.
/*
// Wurde nur aus 'node_device_object_look_for_eds' aufgerufen. Die Funktion ist
// aber gestrichen.
static gboolean node_device_create_new_node_from_eds (NodeDeviceObject* device, NodeObject* scan, const gchar* eds_config)
{
	guint        id;
	gboolean     is_created;
	GValue*      name_value;
	NodesSimple* simple;
	GValue*      typename;

	g_return_val_if_fail (device,     FALSE);
	g_return_val_if_fail (eds_config, FALSE);

	simple = nodes_object_get_simple  (NODES_OBJECT(scan));
	id     = nodes_simple_get_node_id (simple);

	// eds_config =
	//    "/etc/candaemon/eds/AnalogExt.eds"
	//    "/etc/candaemon/eds/Digital.eds"
	//    "/etc/candaemon/eds/lar_analognode_v15.eds"
	//    "/etc/candaemon/eds/lar_doppelmotornode_v12.eds"
	//    "/etc/candaemon/eds/lar_doppelmotornode_v13.eds"
	//    "/etc/candaemon/eds/Scan.eds"
	//
	// In der Gruppe (Sektion) "FileInfo" unter dem Schlüssel "TypeInfo" in der
	// .eds-Datei steht der hier verwendete Klassenname, z. B.:
	// 'NodeAnalogObject' in der Datei 'lar_analognode_v15.eds'.
	typename   = mkt_keyfile_get (eds_config, "FileInfo", "TypeInfo", G_TYPE_STRING);
	name_value = mkt_keyfile_get (eds_config, "FileInfo", "Name",     G_TYPE_STRING);
	is_created = FALSE;

	if (typename && name_value) {
		// 'otype' bekommt den Typindex zu dem Typnamen, der durch den Schlüssel
		// "TypeInfo" in der Gruppe "FileInfo" aus der Datei 'eds_config' abge-
		// rufen wurde.
		GType otype = g_type_from_name (g_value_get_string (typename));

		if (g_type_is_a (otype, NODE_TYPE_OBJECT)) {
			//     'otype', der Typindex zu dem Typnamen, der durch den Schlüs-
			// sel "TypeInfo" in der Gruppe "FileInfo" aus der Datei
			// 'eds_config' abgerufen wurde, bezeichnet eine von 'NodeObject'
			// abgeleiteten Klasse.
			//     Mögliche Klassen sind: 'NodeAnalogObject',
			// 'NodeMotor3Object', 'NodeMotorObject', 'NodeDigitalObject' und
			// 'NodeAnalogExtObject'.
			//     Da der Klassenname "NodeMotorObject" in keiner .eds-Datei
			// vorkommt, wird ein solches Objekt nicht erzeugt.
			//     Der dazugehörige Wert von "Name" in der selben Gruppe ist:
			// für 'NodeAnalogObject' "Analog", for 'NodeMotor3Object'
			// "Doppelmotor", für 'NodeDigitalObject' "Digital" und für
			// 'NodeAnalogExtObject' "Analogext".
			//     Der Klassenname "NodeMotor3Object" kommt in den beiden .eds-
			// Dateien "lar_doppelmotornode_v12.eds" und
			// "lar_doppelmotornode_v13.eds", jeweils mit dem selben Namen
			// "Doppelmotor", vor.

			// 'name' stammt aus Datei eds_config, Gruppe "FileInfo", Schlüssel "Name"
			const gchar* name = g_value_get_string (name_value);

			if (id > 0 && name) {
				gchar* full_path = create_new_node_path (name);

				if (full_path && g_type_is_a(otype, NODE_TYPE_OBJECT)) {
					is_created = node_device_object_create_new_node (device, otype, eds_config, id, full_path);
					g_free (full_path);
				}
			}
		}
		else
			g_warning ("Unknown object type name %s", g_value_get_string(typename));
	}

	if (typename)
		mkt_value_free (typename);

	if (name_value)
		mkt_value_free (name_value);

	return is_created;
}
*/

/*
// Wurde nur in 'node_device_object_look_for_eds' benutzt. Diese Funktion ist
// aber gestrichen.
static gboolean check_eds_index (const gchar* path, NodeIndex* index)
{
	GValue*     check_value = node_index_value   (index);
	const char* group       = node_index_indexid (index);
	GKeyFile*   keyfile;
	GError*     error       = NULL;
	gboolean    result      = FALSE;

	g_return_val_if_fail (check_value, FALSE);
	g_return_val_if_fail (path,        FALSE);
	g_return_val_if_fail (group,       FALSE);

	keyfile = g_key_file_new();

	if (g_key_file_load_from_file (keyfile, path, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error)) {
		gchar* val = g_key_file_get_value (keyfile, group, "DefaultValue", &error);

		if (val) {
			GValue* check = mkt_value_new (check_value -> g_type);

			if (mkt_set_gvalue_from_string (check, val)) {
				// GS: Auf gint geändert (war vorher guint), damit sind Tests möglich.
				gint ret = mkt_value_equal (check_value, check);

				if (g_strcmp0 ("1018sub3", group))
					result = ret == 0;
				else
//					result = ret >= 0;
					result = TRUE;  // GS: Da 'ret' ein 'guint' war, war ret >= 0 immer TRUE.
			}

			mkt_value_free (check);
			g_free         (val);
		}
	}
	else {
		g_warning ("Key file load from file %s error : %s ", path, error != NULL ? error->message : "unknown");

		if (error)
			g_error_free (error);
	}

	g_key_file_free (keyfile);
	mkt_value_free  (check_value);

	return result;
}
*/

/*
// Wurde nur in 'node_device_object_init_new_node' benutzt. Diese Funktion ist
// aber gestrichen.
//static gboolean node_device_object_look_for_eds (NodeDeviceObject* device, NodeObject* scan)
static gboolean node_device_object_look_for_eds (IterateData* data)
{
	gboolean     accept    = TRUE;
	GDir*        dir;
	GError*      error     = NULL;
	gchar*       check_dir = "/etc/candaemon/eds";
	const gchar* name      = NULL;

	dir = g_dir_open (check_dir, 0, NULL);

	if (!dir || error) {
		check_dir = "/usr/etc/candaemon/eds";
		dir       = g_dir_open (check_dir, 0, &error);

		if (!dir || error) {
			g_critical   ("open eds files dir - %s", error->message);
			g_error_free (error);

			return FALSE;
		}
	}

	while ((name = g_dir_read_name (dir))) {
		if (name && g_str_has_suffix(name, ".eds")) {

			// check_dir = "/etc/candaemon/eds"
			// name =
			//    "AnalogExt.eds"
			//    "Digital.eds"
			//    "lar_analognode_v15.eds"
			//    "lar_doppelmotornode_v12.eds"
			//    "lar_doppelmotornode_v13.eds"
			//    "Scan.eds"
			gchar* eds_conf = g_build_path ("/", check_dir, name, NULL);
			// eds_conf =
			//    "/etc/candaemon/eds/AnalogExt.eds"
			//    "/etc/candaemon/eds/Digital.eds"
			//    "/etc/candaemon/eds/lar_analognode_v15.eds"
			//    "/etc/candaemon/eds/lar_doppelmotornode_v12.eds"
			//    "/etc/candaemon/eds/lar_doppelmotornode_v13.eds"
			//    "/etc/candaemon/eds/Scan.eds"

			if (g_file_test (eds_conf, G_FILE_TEST_IS_REGULAR)) {
				guint  check_count;
				guint  cont_done = 0;
				GList* l         = NULL;

				// Ergibt 5. Es wurden 5 'NodeIndexObject' Objekte erzeugt. Die
				// Eigenschaft 'indexid' und weitere haben folgende Werte
				//     indexid   | parametername | objecttype | datatype | accesstype | pdomapping | lowlimit | highlimit
				//     ----------+-------------- +------------+----------+------------+------------+----------+----------
				//     1000      | DeviceType    | 0x7        | 0x0007   | ro         | 0          |-         |-
				//     1018sub0  | nrOfEntries   | 0x7        | 0x0005   | ro         | 0          | 1        | 4
				//     1018sub1  | Vendor ID     | 0x7        | 0x0007   | ro         | 0          |-         |-
				//     1018sub2  | Product Code  | 0x7        | 0x0007   | ro         | 0          |-         |-
				//     1018sub3  | Revision Nr   | 0x7        | 0x0007   | ro         | 0          |-         |-
				// 'lowlimit' und 'highlimit' sind nur für das Objekt mit
				// 'indexid' = "1018sub0" definiert.
				check_count = g_list_length (node_object_childs_index (NODE_OBJECT (data -> scan)));

				for (l=node_object_childs_index(NODE_OBJECT(data->scan)); l; l=l->next) {
					//     Alle Objekt in der Liste 'l' sind vom Typ
					// 'NodeIndexObject'. Mit 'node_index_value' wird die Funk-
					// tion, deren Adresse in dem Element 'value' des privaten
					// Teils 'NodeIndexObjectPrivate' der Klasse
					// 'NodeIndexObject' abgelegt ist. 'value' wurde in
					// 'node_index_object_init_index_interface' auf
					// 'node_index_object_get_value' gesetzt.
					//      'node_index_object_get_value' liefert eine 'GValue'
					// Kopie dws Elements 'GValue* internal_value' des privaten
					// Teils 'NodeIndexObjectPrivate' der Klasse
					// 'NodeIndexObject'
					if (check_eds_index (eds_conf, NODE_INDEX(l->data))) {
						node_index_object_status (NODE_INDEX_OBJECT(l->data), "done");
						cont_done++;
					}
					else {
						accept = FALSE;
						node_index_object_status (NODE_INDEX_OBJECT(l->data), "value deviation[check wrong eds file]");
					}
				}

				if (check_count == cont_done) {
					node_device_create_new_node_from_eds (data->device, data->scan, eds_conf);
					node_object_dup_to_file              (data->scan,   name,       TRUE);

					accept = TRUE;

					break;
				}
				else
					node_object_dup_to_file (data->scan, name, FALSE);
			}

			g_free (eds_conf);
		}
	}

	g_dir_close (dir);

	return accept;
}
*/

static void print_reset_message_debug (canmsg_t* rxmsg)
{
    g_return_if_fail(rxmsg != NULL);
    gchar *path = g_build_path("/", g_get_home_dir(), "node-reset-debug.log", NULL);
    FILE * f    = fopen(path, "a+");
    if (f != NULL) {
        fprintf(f, "%s RESET_ID(%lx):  [%lx,%x,%x,%x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x]\n", market_db_get_date_sqlite_format(market_db_time_now()), rxmsg->id & 0x07f, rxmsg->id, rxmsg->length,
            rxmsg->flags, rxmsg->cob, rxmsg->data[0], rxmsg->data[1], rxmsg->data[2], rxmsg->data[3], rxmsg->data[4], rxmsg->data[5], rxmsg->data[6], rxmsg->data[7]);
        fflush(f);
        fclose(f);
    }
    g_free(path);
}

static void broadcast_node_reset (NodeDeviceObject* device, guint node)
{
	CandeviceSimple* simple;

	simple = candevice_object_get_simple (CANDEVICE_OBJECT (device));

	candevice_simple_set_reseted_node  (simple, node);
	candevice_simple_emit_node_reseted (simple, node);
}

static void node_read_system_message (NodeDeviceObject* device)
{
	enum {NODE_COUNT = 125};

	char     noden [NODE_COUNT];
	guint    retry = 0;
	canmsg_t rxmsg;

	memset (noden, 0, NODE_COUNT);

	// GS: 'MUTEX_LOCK' wird in 'node_device_object_read_index' und in
	// 'node_device_object_write_index' aufgerufen. Warum nicht hier?
	MUTEX_LOCK();

	//     GS: Die Längenangabe von 1 an dieser Stelle verwundert. Trotzdem wird
	// die Struktur 'rxmsg' vollständig gefüllt. Wenn man aber, wie für 'read'
	// spezifiziert, 'sizeof rxmsg' einsetzt, so wird zwar auch die ganze Struk-
	// tur gefüllt, aber trotzdem 1 zurückgegeben.
	while (0 < read (device->priv->fd, &rxmsg, 1) && retry <= 60) {
		gulong node = rxmsg.id & 0x07f;

		// g_debug (
		// 	"RESET:%lx,%x,%x,%x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x,%.2x\n",
		// 	rxmsg.id,      rxmsg.length,  rxmsg.flags,   rxmsg.cob,
		// 	rxmsg.data[0], rxmsg.data[1], rxmsg.data[2], rxmsg.data[3], rxmsg.data[4], rxmsg.data[5], rxmsg.data[6], rxmsg.data[7]
		// );

		if (rxmsg.id > 0x700 && rxmsg.id < 0x800)
			if (node<NODE_COUNT && !noden[node]) {
				// print_reset_message_debug (&rxmsg);
				broadcast_node_reset      (device, node);

				noden [node] = node;
			}
	}

	MUTEX_UNLOCK();
}

static gboolean node_device_read_idle (gpointer data)
{
	NodeDeviceObject* device = NODE_DEVICE_OBJECT (data);
	node_read_system_message (device);
	return TRUE;
}

/*
// Wurde nur in 'node_device_object_scann_idle' benutzt. Diese Funktion ist aber
// gestrichen.
static gboolean node_device_object_init_new_node (IterateData* data)
{
	g_message ("New Node %d", nodes_simple_get_node_id(nodes_object_get_simple(NODES_OBJECT(data->scan))));

	if (! node_device_object_look_for_eds (data))
		g_warning ("EDS configuration file not found ");

	return TRUE;
}
*/

//     Diese Funktion wird aufgerufen, nachdem bereits eine SDO Anfrage an den
// Server (die Node) gesendet worden ist. Die gesendete Nachricht sah folgender-
// maßen aus:
//  - . data [0] = 0x40;                                       // Start des Uploads (Lesen aus einem Objektverzeichnis), n, e und s sind nicht gesetzt.
//  - . id       = (node_index_nodeid(index) & 0x7F) | 0x600;  // COB-ID dieses Kanals: 0x600 + Node ID für den Empfang
//  - . flags    = 0;
//  - . cob      = 0;
//  - . length   = 8;

//  - . data [1] = (char) (node_index_index    (index) & 0xff);  // index, low  byte
//  - . data [2] = (char) (node_index_index    (index) >> 8);    // index, high byte
//  - . data [3] = (char) (node_index_subindex (index));         // subindex
//  - . data [4] = 0;
//  - . data [5] = 0;
//  - . data [6] = 0;
//  - . data [7] = 0;
//
//     Mögliche Werte für "nodeid" (geliefert von 'node_index_nodeid'), "index"
// (geliefert von 'node_index_index'), "subindex" (geliefert von
// 'node_index_subindex') und "indexid" waren:
//  - nodeid | index  | subindex | indexid
//    -------+--------+----------+--------
//    1      | 0x1000 | 0        | 1000
//    2      | 0x1000 | 0        | 1000
//    3      | 0x1000 | 0        | 1000
//    4      | 0x1000 | 0        | 1000
//    4      | 0x1018 | 0        | 1018sub0
//    4      | 0x1018 | 1        | 1018sub1
//    4      | 0x1018 | 2        | 1018sub2
//    4      | 0x1018 | 3        | 1018sub3
//    5      | 0x1000 | 0        | 1000
//    6      | 0x1000 | 0        | 1000
//    7      | 0x1000 | 0        | 1000
//    8      | 0x1000 | 0        | 1000
//    9      | 0x1000 | 0        | 1000
//    10     | 0x1000 | 0        | 1000
//    11     | 0x1000 | 0        | 1000
//    12     | 0x1000 | 0        | 1000
//    13     | 0x1000 | 0        | 1000
//    14     | 0x1000 | 0        | 1000
//    15     | 0x1000 | 0        | 1000
//    16     | 0x1000 | 0        | 1000
//    17     | 0x1000 | 0        | 1000
//    18     | 0x1000 | 0        | 1000
//    18     | 0x1018 | 0        | 1018sub0
//    18     | 0x1018 | 1        | 1018sub1
//    18     | 0x1018 | 2        | 1018sub2
//    18     | 0x1018 | 3        | 1018sub3
//    19     | 0x1000 | 0        | 1000
//    20     | 0x1000 | 0        | 1000
//    20     | 0x1018 | 0        | 1018sub0
//    20     | 0x1018 | 1        | 1018sub1
//    20     | 0x1018 | 2        | 1018sub2
//    20     | 0x1018 | 3        | 1018sub3
//    21     | 0x1000 | 0        | 1000
//    22     | 0x1000 | 0        | 1000
//    23     | 0x1000 | 0        | 1000
//    24     | 0x1000 | 0        | 1000
//    24     | 0x1018 | 0        | 1018sub0
//    24     | 0x1018 | 1        | 1018sub1
//    24     | 0x1018 | 2        | 1018sub2
//    24     | 0x1018 | 3        | 1018sub3
//    25     | 0x1000 | 0        | 1000
//    25     | 0x1018 | 0        | 1018sub0
//    25     | 0x1018 | 1        | 1018sub1
//    25     | 0x1018 | 2        | 1018sub2
//    25     | 0x1018 | 3        | 1018sub3
//
//
//     Hier kommen 5 mögliche 'NodeIndexObject' Objekte 'index' vor. Die Eigen-
// schaft 'indexid' und weitere können folgende Werte haben:
//     indexid   | parametername | objecttype | datatype | accesstype | pdomapping | lowlimit | highlimit
//     ----------+-------------- +------------+----------+------------+------------+----------+----------
//     1000      | DeviceType    | 0x7        | 0x0007   | ro         | 0          |-         |-
//     1018sub0  | nrOfEntries   | 0x7        | 0x0005   | ro         | 0          | 1        | 4
//     1018sub1  | Vendor ID     | 0x7        | 0x0007   | ro         | 0          |-         |-
//     1018sub2  | Product Code  | 0x7        | 0x0007   | ro         | 0          |-         |-
//     1018sub3  | Revision Nr   | 0x7        | 0x0007   | ro         | 0          |-         |-
// 'lowlimit' und 'highlimit' sind nur für das Objekt mit 'indexid' = "1018sub0"
// definiert.

enum {PROCESS_INDEX_DONE, PROCESS_INDEX_RESTART_TIMER, PROCESS_INDEX_BUSY, PROCESS_INDEX_NODE_RESETED};

// NodeObject* scan
// guint       id
// GList*      list_to_read
// gint        fd
// NodeIndex*  index
static gint node_device_process_index (IterateData* data, canmsg_t* rxmsg)
{
	g_return_val_if_fail (rxmsg, FALSE);

	if (rxmsg->id < 0x700 && rxmsg->id > 0x580) {
		enum {
			CAN_DATA_LONG  = 1,
			CAN_DATA_ANY   = 2,

			CAN_COUNT_TYPE_4B =  0,
			CAN_COUNT_TYPE_3B =  4,
			CAN_COUNT_TYPE_2B =  8,
			CAN_COUNT_TYPE_1B = 12
		};


		canmsg_t cm;
		guint    typ;   // Message Typ e,b,w,t,q und s
		guint    node;  // Node Adresse
		guint    oi;    // Objectbaum Index
		guint    si;    // Sub Index
		guint    value;

// - Byte 1 3 bits: ccs=1
// -        1 bit:  reserved(=0)
// -        2 bits: n
// -        1 bit:  e
// -        1 bit:  s
// - Byte 2-3:      index
// - Byte 4:        subindex
// - Byte 5-8:      data
//
// ccs:      ist der Client-Befehlsspezifizierer der SDO-Übertragung
//           0: SDO-Segment-Download
//           1: Start des Downloads
//           2: Start des Uploads
//           3: SDO-Segment-Upload
//           4: Abbruch eines SDO-Transfers
//           5: SDO Block Upload
//           6: SDO Block Download
// n:        Anzahl der Bytes im Datenteil der Nachricht, die keine Daten ent-
//           halten, nur gültig, wenn e und s gesetzt sind
// e:        gesetzt:       beschleunigte Übertragung, alle ausgetauschten Daten
//                          sind in der Nachricht enthalten.
//           nicht gesetzt: segmentierte Übertragung, die Daten passen nicht in
//                          eine Nachricht und es werden mehrere Nachrichten
//                          verwendet.
// s:        gesetzt:       zeigt an, dass die Datengröße in n (wenn e gesetzt
//                          ist) oder im Datenteil der Nachricht angegeben ist
		typ  = rxmsg -> data [0];                            // Message Typ e,b,w,t,q und s
		node = rxmsg -> id & 0x07f;                          // Node Adresse
		oi   = rxmsg -> data [1] + (rxmsg -> data [2] << 8); // Objectbaum Index
		si   = rxmsg -> data [3];                            // Sub Index

		if (node_index_nodeid(data->index) != node) {
			g_warning("Unknown package runs on CAN-Bus -  ID:%x index=%x sub=%x", node, oi, si);
			return PROCESS_INDEX_BUSY;
		}

		memset (&cm, 0, sizeof(canmsg_t));

		cm.id = (node_index_nodeid(data->index) & 0x7F) + 0x600;

		switch (typ & 0xE0) {
			case 0x0:
			case 0x10:
				if (node_index_object_toggle (NODE_INDEX_OBJECT(data->index), &rxmsg->data[0], &cm.data[0])) {
					cm.flags  = 0;
					cm.cob    = 0;
					cm.length = 8;

					if (write (data->fd, &cm, 1) > 0)
						return PROCESS_INDEX_RESTART_TIMER;
				}

				return PROCESS_INDEX_DONE;

			case 0x20:
				if (node_index_object_toggle (NODE_INDEX_OBJECT(data->index), &rxmsg->data[0], &cm.data[0])) {
					cm.flags  = 0;
					cm.cob    = 0;
					cm.length = 8;

					if (write (data->fd, &cm, 1) > 0)
						return PROCESS_INDEX_RESTART_TIMER;
				}

				return PROCESS_INDEX_DONE;

			case 0x40:
				if (typ & CAN_DATA_ANY) {
					switch (CAN_TYPED_DATA_LEN (typ)) {
						case CAN_COUNT_TYPE_1B:
							value = rxmsg -> data [4];
							node_index_set_value32 (data->index, value);
							return PROCESS_INDEX_DONE;

						case CAN_COUNT_TYPE_2B:
							value = rxmsg->data[4] + (rxmsg->data[5] << 8);
							node_index_set_value32 (data->index, value);
							return PROCESS_INDEX_DONE;

						case CAN_COUNT_TYPE_3B:
							value = rxmsg->data[4] + (rxmsg->data[5] << 8) + (rxmsg->data[6] << 16);
							node_index_set_value32 (data->index, value);
							return PROCESS_INDEX_DONE;

						case CAN_COUNT_TYPE_4B:
							value = rxmsg->data[4] + (rxmsg->data[5] << 8) + (rxmsg->data[6] << 16) + (rxmsg->data[7] << 24);
							node_index_set_value32 (data->index, value);
							return PROCESS_INDEX_DONE;

						default:
							return PROCESS_INDEX_BUSY;
					}
				}
				else if ((typ & CAN_DATA_LONG)) {
					if (node_index_object_toggle (NODE_INDEX_OBJECT(data->index), &rxmsg->data[0], &cm.data[0])) {
						cm.flags  = 0;
						cm.cob    = 0;
						cm.length = 8;

						if (write (data->fd, &cm, 1) > 0)
							return PROCESS_INDEX_RESTART_TIMER;

						return PROCESS_INDEX_BUSY;
					}
				}

				return PROCESS_INDEX_DONE;

			case 0x60:
			case 0x70:
				if (
					(node_index_datatype (NODE_INDEX (data->index)) == 0x9 || ((typ & CAN_DATA_LONG) && !(typ & CAN_DATA_ANY)))
					&&
					(node_index_object_toggle (NODE_INDEX_OBJECT (data->index), &rxmsg->data[0], &cm.data[0]))
				) {
					cm.flags  = 0;
					cm.cob    = 0;
					cm.length = 8;

					if (write (data->fd, &cm, 1) > 0)
						return PROCESS_INDEX_RESTART_TIMER;
				}

				return PROCESS_INDEX_DONE;
		}

		#undef CAN_TYPED_DATA_LEN
	}
	else if (rxmsg->id > 0x700) {
		guint node = rxmsg->id & 0x07f;

		if (node > 0 && node < 124) {
			print_reset_message_debug (rxmsg);
			return PROCESS_INDEX_NODE_RESETED;
		}

		return PROCESS_INDEX_DONE;
	}

	return PROCESS_INDEX_BUSY;
}

enum {READ_INDEX_DONE, READ_INDEX_FAILED, READ_INDEX_NODE_RESETED};

static gint read_index (IterateData* data, unsigned long* idFromCan)
{
	canmsg_t cm;
	gint     result = READ_INDEX_FAILED;

	g_return_val_if_fail (data->index, FALSE);

	MUTEX_LOCK();

//  GS: Beschreibung aus "https://wikivividly.com/wiki/CANopen"
//
//	Servicedatenobjekt (SDO) -Protokoll
//
//     Nicht zu verwechseln mit Service Data Objects, einer Daten-als-Service-
// Abstraktion.
//
//     Das SDO-Protokoll dient zum Setzen und Lesen von Werten aus dem Objekt-
// verzeichnis eines Gerätes. Das Gerät, auf dessen Objektverzeichnis zugegrif-
// fen wird, ist der SDO-Server und das Gerät, das auf das Objektverzeichnis zu-
// greift, ist der SDO-Client. Die Kommunikation wird immer vom SDO-Client ini-
// tiiert. In der CANopen-Terminologie wird die Kommunikation vom SDO-Server
// aus betrachtet, so dass ein Lesen aus einem Objektverzeichnis als SDO-Upload
// und ein Schreiben in ein Objektverzeichnis als SDO-Download bezeichnet wird.
//     Da die Objektverzeichniswerte größer als die 8-Byte-Grenze eines CAN-Rah-
// mens sein können, implementiert das SDO-Protokoll die Segmentierung und De-
// segmentierung längerer Nachrichten. Tatsächlich gibt es zwei dieser Protokol-
// le: SDO Download / Upload und SDO Block Download / Upload. Die SDO-Blocküber-
// tragung ist eine neuere Ergänzung des Standards, die es ermöglicht, große Da-
// tenmengen mit etwas weniger Protokollaufwand zu übertragen.
//     Die COB-IDs der jeweiligen SDO-Übertragungsnachrichten von Client zum
// Server und vom Server zum Client können im Objektverzeichnis eingestellt wer-
// den. Bis zu 128 SDO-Server können im Objektverzeichnis unter den Adressen
// 0x1200 -0x127F eingerichtet werden. Auf die gleiche Weise können die SDO-
// Client-Verbindungen des Geräts mit Variablen bei 0x1280 - 0x12FF konfiguriert
// werden. Der vordefinierte Satz von Verbindungen legt des weiteren einen SDO-
// Kanal fest, der auch unmittelbar nach dem Hochfahren (vor dem Zustand "be-
// triebsbereit") zum Konfigurieren des Geräts verwendet werden kann. Die COB-
// IDs dieses Kanals sind 0x600 + Node ID für den Empfang und 0x580 + Node ID
// für die Übertragung.
//     Um einen Download zu initiieren, sendet der SDO-Client die folgenden Da-
// ten in einer CAN-Nachricht mit der 'receive' COB-ID des SDO-Kanals:
// - Byte 1 3 bits: ccs=1
// -        1 bit:  reserved(=0)
// -        2 bits: n
// -        1 bit:  e
// -        1 bit:  s
// - Byte 2-3:      index
// - Byte 4:        subindex
// - Byte 5-8:      data
//
// ccs:      ist der Client-Befehlsspezifizierer der SDO-Übertragung
//           0: SDO-Segment-Download
//           1: Start des Downloads
//           2: Start des Uploads
//           3: SDO-Segment-Upload
//           4: Abbruch eines SDO-Transfers
//           5: SDO Block Upload
//           6: SDO Block Download
// n:        Anzahl der Bytes im Datenteil der Nachricht, die keine Daten ent-
//           halten, nur gültig, wenn e und s gesetzt sind
// e:        gesetzt:       beschleunigte Übertragung, alle ausgetauschten Daten
//                          sind in der Nachricht enthalten.
//           nicht gesetzt: segmentierte Übertragung, die Daten passen nicht in
//                          eine Nachricht und es werden mehrere Nachrichten
//                          verwendet.
// s:        gesetzt:       zeigt an, dass die Datengröße in n (wenn e gesetzt
//                          ist) oder im Datenteil der Nachricht angegeben ist
// index:    ist der Objektverzeichnisindex der Daten, auf die zugegriffen wer-
//           den soll
// Subindex: ist der Subindex der Objektverzeichnisvariablen
// data:     enthält die Daten, die im Falle einer beschleunigten Übertragung
//           hochgeladen werden (e ist gesetzt) oder die Größe der zu ladenden
//           Daten (s ist gesetzt, e ist nicht gesetzt)

//     Bei dem Aufruf aus der Funktion 'node_device_object_scann_idle' kommen 5
// mögliche 'NodeIndexObject' Objekte 'data->index' vor. Die Eigenschaft 'indexid' und
// weitere können folgende Werte haben:
//     indexid   | parametername | objecttype | datatype | accesstype | pdomapping | lowlimit | highlimit
//     ----------+-------------- +------------+----------+------------+------------+----------+----------
//     1000      | DeviceType    | 0x7        | 0x0007   | ro         | 0          |-         |-
//     1018sub0  | nrOfEntries   | 0x7        | 0x0005   | ro         | 0          | 1        | 4
//     1018sub1  | Vendor ID     | 0x7        | 0x0007   | ro         | 0          |-         |-
//     1018sub2  | Product Code  | 0x7        | 0x0007   | ro         | 0          |-         |-
//     1018sub3  | Revision Nr   | 0x7        | 0x0007   | ro         | 0          |-         |-
// 'lowlimit' und 'highlimit' sind nur für das Objekt mit 'indexid' = "1018sub0"
// definiert.
	memset (&cm, 0, sizeof(canmsg_t));

	cm . flags    = 0;
	cm . cob      = 0;
	cm . id       = (node_index_nodeid(data->index) & 0x7F) | 0x600;  // COB-ID dieses Kanals: 0x600 + Node ID für den Empfang
	cm . length   = 8;

	cm . data [0] = 0x40;                                         // Start des Uploads (Lesen aus einem Objektverzeichnis), n, e und s sind nicht gesetzt.
	cm . data [1] = (char) (node_index_index    (data->index) & 0xff);  // index, low  byte
	cm . data [2] = (char) (node_index_index    (data->index) >> 8);    // index, high byte
	cm . data [3] = (char) (node_index_subindex (data->index));         // subindex
	cm . data [4] = 0;
	cm . data [5] = 0;
	cm . data [6] = 0;
	cm . data [7] = 0;

	//     GS: Ich konnte an dieser Stelle Abfragen von 24 verschiedene
	// 'NodeIndex'-Objekten feststellen, allerdings mit wechselnden Eigenschaf-
	// ten. In der nachfolgenden Tabelle bezeichnet "Anzahl" die Anzahl der Auf-
	// rufe mit einer identischen Kombination der nachfolgenden Werte, "Datei"
	// bezeichnet die Quelldatei, aus der der Aufruf erfolgte, "Zeile" bezeich-
	// net die Zeilennummer in der Quelldatei, wo der Aufruf erfolgte und "Node"
	// bezeichnet den Zeiger auf das hier verwendete 'NodeIndex'-Objekt.
	// ,--------+----------------------+-------+----------+---------+--------+-------+----------.  ,--------+----------------------+-------+----------+----------+--------+-------+----------.  ,--------+----------------------+-------+----------+----------+--------+-------+----------.
	// | Anzahl | Datei                | Zeile | Node     | indexid | nodeid | index | subindex |  | Anzahl | Datei                | Zeile | Node     | indexid  | nodeid | index | subindex |  | Anzahl | Datei                | Zeile | Node     | indexid  | nodeid | index | subindex |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 01     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD38F8 | 1000     | 1C     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3A00 | 1018sub2 | 1C     | 1018  | 2        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 02     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD38F8 | 1000     | 1D     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3A58 | 1018sub3 | 04     | 1018  | 3        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 03     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD38F8 | 1000     | 1E     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3A58 | 1018sub3 | 12     | 1018  | 3        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 04     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD38F8 | 1000     | 1F     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3A58 | 1018sub3 | 14     | 1018  | 3        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 05     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD38F8 | 1000     | 20     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3A58 | 1018sub3 | 18     | 1018  | 3        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 06     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD38F8 | 1000     | 21     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3A58 | 1018sub3 | 19     | 1018  | 3        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 07     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD38F8 | 1000     | 22     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3A58 | 1018sub3 | 1C     | 1018  | 3        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 08     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD38F8 | 1000     | 23     | 1000  | 0        |  | 1      | node-object.c        | 1168  | 08F18CA0 | 6411sub0 | 1C     | 6411  | 0        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 09     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD38F8 | 1000     | 24     | 1000  | 0        |  | 1      | node-object.c        | 1168  | 08DD33D0 | 6300sub1 | 18     | 6300  | 1        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 0A     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD38F8 | 1000     | 25     | 1000  | 0        |  | 1      | node-object.c        | 1168  | 08E10858 | 6300sub1 | 19     | 6300  | 1        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 0B     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3950 | 1018sub0 | 04     | 1018  | 0        |  | 585    | node-object.c        | 1168  | 08D27A58 | 6101sub3 | 04     | 6101  | 3        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 0C     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3950 | 1018sub0 | 12     | 1018  | 0        |  | 1496   | node-object.c        | 1168  | 08DCFA18 | 6100sub1 | 18     | 6100  | 1        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 0D     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3950 | 1018sub0 | 14     | 1018  | 0        |  | 585    | node-object.c        | 1168  | 08D27AB0 | 6101sub4 | 04     | 6101  | 4        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 0E     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3950 | 1018sub0 | 18     | 1018  | 0        |  | 3792   | node-object.c        | 1168  | 08D2ADB0 | 6111sub1 | 04     | 6111  | 1        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 0F     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3950 | 1018sub0 | 19     | 1018  | 0        |  | 1148   | node-object.c        | 1168  | 08D279A8 | 6101sub1 | 04     | 6101  | 1        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 10     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3950 | 1018sub0 | 1C     | 1018  | 0        |  | 1147   | node-object.c        | 1168  | 08D27A00 | 6101sub2 | 04     | 6101  | 2        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 11     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD39A8 | 1018sub1 | 04     | 1018  | 1        |  | 4      | node-object.c        | 1168  | 08D31608 | 6000sub1 | 12     | 6000  | 1        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 12     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD39A8 | 1018sub1 | 12     | 1018  | 1        |  | 1      | node-object.c        | 1168  | 08D317C0 | 6010sub1 | 12     | 6010  | 1        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 13     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD39A8 | 1018sub1 | 14     | 1018  | 1        |  | 2      | node-object.c        | 1168  | 08D89450 | 6000sub1 | 14     | 6000  | 1        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 14     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD39A8 | 1018sub1 | 18     | 1018  | 1        |  | 1149   | node-object.c        | 1168  | 08CF31A8 | 6010sub1 | 04     | 6010  | 1        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 15     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD39A8 | 1018sub1 | 19     | 1018  | 1        |  | 10     | node-object.c        | 1168  | 08D92BC8 | 6201sub2 | 14     | 6201  | 2        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 16     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD39A8 | 1018sub1 | 1C     | 1018  | 1        |  | 9      | node-object.c        | 1168  | 08D92A10 | 6200sub2 | 14     | 6200  | 2        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 17     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3A00 | 1018sub2 | 04     | 1018  | 2        |  | 2      | node-object.c        | 1168  | 08D92D68 | 6201sub6 | 14     | 6201  | 6        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 18     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3A00 | 1018sub2 | 12     | 1018  | 2        |  | 8      | node-object.c        | 1168  | 08D5EDA8 | 6201sub2 | 12     | 6201  | 2        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 19     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3A00 | 1018sub2 | 14     | 1018  | 2        |  | 2      | node-object.c        | 1168  | 08D5EF08 | 6201sub6 | 12     | 6201  | 6        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 1A     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3A00 | 1018sub2 | 18     | 1018  | 2        |  | 7      | node-object.c        | 1168  | 08D5B7D0 | 6200sub2 | 12     | 6200  | 2        |
	// | 1      | node-device-object.c | 710   | 08CD38F8 | 1000    | 1B     | 1000  | 0        |  | 1      | node-device-object.c | 710   | 08CD3A00 | 1018sub2 | 19     | 1018  | 2        |  `--------+----------------------+-------+----------+----------+--------+-------+----------'
	// `--------+----------------------+-------+----------+---------+--------+-------+----------'  `--------+----------------------+-------+----------+----------+--------+-------+----------'
	//     Hier fällt auf, dass bei den Aufrufen dieser Funktion aus dieser Da-
	// tei, hier Zeile 710, das ist von der Funktion
	// 'node_device_object_scann_idle', die Werte von "indexid" (und damit auch
	// von "index" und "subindex") jeweils strikt einem Objekt zugeordnet sind,
	// sich der Wert von "nodeid" aber ändert.
	//     Bei den Aufrufen dieser Funktion aus 'node-object.c', hier Zeile
	// 1168, das ist aus der Funktion 'node_object_read_value', sind die Objekte
	// (keines der Objekte aus den Aufrufen dieser Funktion aus dieser Datei von
	// der Funktion 'node_device_object_scann_idle') durch "indexid" nicht genau
	// zugeordnet, d. h. zwei verschiedenen Objekten kann die selbe "indexid"
	// zugeordnet sein. Auch "nodeid" und "indexid" zusammen definieret ein
	// 'NodeIndex'-Objekt in diesen Fällen nicht eindeutig.
	if (write (data->fd, &cm, 1) > 0) {
		GTimer* timer = g_timer_new();

		g_timer_start (timer);

		while (READ_INDEX_FAILED == result) {
			gulong   microseconds = 0;
			gdouble  sec          = g_timer_elapsed (timer, &microseconds);
			canmsg_t rxmsg;

			//     GS: Die Längenangabe von 1 an dieser Stelle verwundert.
			// Trotzdem wird die Struktur 'rxmsg' vollständig gefüllt. Wenn man
			// aber, wie für 'read' spezifiziert, 'sizeof rxmsg' einsetzt, so
			// wird zwar auch die ganze Struktur gefüllt, aber trotzdem 1 zu-
			// rückgegeben.
			//     GS: Die beiden Websites
			// 'https://de.wikipedia.org/wiki/Can4linux' und
			// 'https://wikivividly.com/wiki/CANopen' schreiben in einem (bei
			// beiden identischen und ansonsten nichtssagenden) Beispielprogramm
			// hinter der 'write' Anweisung in schlechtem deutsch:
			// "/* ! count enthält Anzahl Frames, nicht Byte */".
			if (read (data->fd, &rxmsg, 1) > 0) {
				gint ret = node_device_process_index (data, &rxmsg);

				switch (ret) {
					case PROCESS_INDEX_DONE:
						result = READ_INDEX_DONE;
						break;

					case PROCESS_INDEX_RESTART_TIMER:
						g_timer_reset (timer);
						break;

					case PROCESS_INDEX_NODE_RESETED:
						result = READ_INDEX_NODE_RESETED;
						* idFromCan = rxmsg . id;
						break;
				}
			}
			else if (sec > 1.0 || microseconds > 200000)
				break;

			g_usleep (2);
		}

		g_timer_destroy (timer);
	}

	MUTEX_UNLOCK();

	return result;
}

//     Die Funktion 'node_object_childs_index' liefert die aus der .eds-Datei
// "Scan.eds" generierten 'NodeIndexObject'-Objekte. Diese .eds-Datei legt die
// Werte für die folgenden Eigenschaften fest:
//     indexid   | parametername | objecttype | datatype | accesstype | pdomapping | lowlimit | highlimit
//     ----------+-------------- +------------+----------+------------+------------+----------+----------
//     1000      | DeviceType    | 0x7        | 0x0007   | ro         | 0          |-         |-
//     1018sub0  | nrOfEntries   | 0x7        | 0x0005   | ro         | 0          | 1        | 4
//     1018sub1  | Vendor ID     | 0x7        | 0x0007   | ro         | 0          |-         |-
//     1018sub2  | Product Code  | 0x7        | 0x0007   | ro         | 0          |-         |-
//     1018sub3  | Revision Nr   | 0x7        | 0x0007   | ro         | 0          |-         |-
// 'lowlimit' und 'highlimit' sind nur für das Objekt mit 'indexid' =
// "1018sub0" definiert.
/*
// Wurde nur in 'node_device_object_scan' benutzt. Diese Funktion ist aber ge-
// strichen.
static gboolean node_device_object_scann_idle (gpointer dataArg)
{
	IterateData* data = (IterateData*) dataArg;

	if (! data -> scan) {
		g_warning ("Can bus scan object fail.. ");
		return FALSE;
	}

	g_return_val_if_fail (data->device,                FALSE);
	g_return_val_if_fail (data->device->priv->is_open, FALSE);

	if (data->list_to_read && data->list_to_read->data && NODE_IS_INDEX(data->list_to_read->data)) {
		unsigned long idFromCan;

		// GS: Bei meinen Tests wurde hier nie gelesen. Nur bei Nodereset?
		node_read_system_message (data -> device);

		data -> index = NODE_INDEX (data -> list_to_read -> data);
		//     In der Liste 'data->list_to_read' befinden sich 5
		// 'NodeIndexObject' Objekte. Die Eigenschaft 'indexid' und weitere ha-
		// ben folgende Werte:
		//     indexid   | parametername | objecttype | datatype | accesstype | pdomapping | lowlimit | highlimit
		//     ----------+-------------- +------------+----------+------------+------------+----------+----------
		//     1000      | DeviceType    | 0x7        | 0x0007   | ro         | 0          |-         |-
		//     1018sub0  | nrOfEntries   | 0x7        | 0x0005   | ro         | 0          | 1        | 4
		//     1018sub1  | Vendor ID     | 0x7        | 0x0007   | ro         | 0          |-         |-
		//     1018sub2  | Product Code  | 0x7        | 0x0007   | ro         | 0          |-         |-
		//     1018sub3  | Revision Nr   | 0x7        | 0x0007   | ro         | 0          |-         |-
		// 'lowlimit' und 'highlimit' sind nur für das Objekt mit 'indexid' = "1018sub0"
		// definiert.

		switch (read_index (data, &idFromCan)) {
			case READ_INDEX_NODE_RESETED:
				broadcast_node_reset (data->device, idFromCan & 0x07f);
				// GS: Hier wurde KEIN 'break' vergessen. Das ist Absicht.

			case READ_INDEX_DONE:
				data->list_to_read = data->list_to_read->next;
				// "node_id" bleibt gleich, nächstes 'NodeIndexObject'
				return TRUE;

			case READ_INDEX_FAILED:
				g_warning("Index %s can not read", node_index_indexid(NODE_INDEX(data->list_to_read->data)));
				// "node_id" wird um 1 erhöht, 'NodeIndexObject' bleibt gleich
				break;
		}
	}
	else
		node_device_object_init_new_node (data);

	data -> list_to_read = node_object_childs_index (data -> scan);

	// Die höchste beobachtete Zahl war 0x25, d. h. 37
	if (data->id > 36)
		return FALSE;

	data -> id ++;
	nodes_simple_set_node_id (nodes_object_get_simple (NODES_OBJECT(data->scan)), data->id);

	return TRUE;
}
*/

static void node_device_object_scann_idle_destroy (gpointer user_data)
{
	IterateData*     data   = (IterateData*) user_data;
	CandeviceSimple* simple = candevice_object_get_simple (CANDEVICE_OBJECT (data->device));

	candevice_simple_set_scanned (simple, TRUE);
	//     Setzt die Eigenschaft "Scanned" im dbus-Objekt
	// "com.lar.candevice.simple". Wird nicht mehr abgefragt.

	service_simple_set_done (tera_service_get_simple(), TRUE);
	//     Setzt die Eigenschaft "Done" im dbus-Objekt "com.lar.service.simple".
	// Wird in der Funktion 'get_service_object_finish_callback' in der Datei
	// 'mktbus/src/tera-client-object.c' abgefragt.

	service_simple_emit_initialized (tera_service_get_simple(), TRUE);
	//     Dieses Signal wird in der Funktion
	// 'get_service_object_finish_callback' in der Datei
	// 'mktbus/src/tera-client-object.c' angeschlossen.

	/*
	if (data -> device -> priv -> read_idle)
		g_source_remove (data -> device -> priv -> read_idle);

	data -> device -> priv -> read_idle = g_timeout_add (100, node_device_read_idle, data->device);
	*/

	free (data);
}

gboolean node_device_init_ultra_nodes(NodeDeviceObject *device){

   // Create Analog node id2

    NodesObjectSkeleton *node = NODES_OBJECT_SKELETON(g_object_new(NODE_TYPE_ANALOG_OBJECT,
    "g-object-path","/com/lar/nodes/Analog1", "node-configuration",SYSCONFDIR"/candaemon/eds/lar_analognode_v15.eds", NULL));
    nodes_simple_set_device(nodes_object_get_simple(NODES_OBJECT(node)), g_dbus_object_get_object_path(G_DBUS_OBJECT(device)));
    nodes_simple_set_node_id(nodes_object_get_simple(NODES_OBJECT(node)), 0x4);
    g_dbus_object_manager_server_export(G_DBUS_OBJECT_MANAGER_SERVER(node_control_app_nodes_manager()), G_DBUS_OBJECT_SKELETON(node));
    device->priv->noden = g_list_append(device->priv->noden, g_object_ref(node));
    g_object_unref(node);

    node = NODES_OBJECT_SKELETON(g_object_new(NODE_TYPE_MOTOR3_OBJECT,
    "g-object-path","/com/lar/nodes/Doppelmotor1", "node-configuration",SYSCONFDIR"/candaemon/eds/lar_doppelmotornode_v13.eds", NULL));
    nodes_simple_set_device(nodes_object_get_simple(NODES_OBJECT(node)), g_dbus_object_get_object_path(G_DBUS_OBJECT(device)));
    nodes_simple_set_node_id(nodes_object_get_simple(NODES_OBJECT(node)),0x12);
    g_dbus_object_manager_server_export(G_DBUS_OBJECT_MANAGER_SERVER(node_control_app_nodes_manager()), G_DBUS_OBJECT_SKELETON(node));
    device->priv->noden = g_list_append(device->priv->noden, g_object_ref(node));
    g_object_unref(node);

	node = NODES_OBJECT_SKELETON(g_object_new(NODE_TYPE_MOTOR3_OBJECT,
    "g-object-path","/com/lar/nodes/Doppelmotor2", "node-configuration",SYSCONFDIR"/candaemon/eds/lar_doppelmotornode_v13.eds", NULL));
    nodes_simple_set_device(nodes_object_get_simple(NODES_OBJECT(node)), g_dbus_object_get_object_path(G_DBUS_OBJECT(device)));
    nodes_simple_set_node_id(nodes_object_get_simple(NODES_OBJECT(node)),0x14);
    g_dbus_object_manager_server_export(G_DBUS_OBJECT_MANAGER_SERVER(node_control_app_nodes_manager()), G_DBUS_OBJECT_SKELETON(node));
    device->priv->noden = g_list_append(device->priv->noden, g_object_ref(node));
    g_object_unref(node);

    node = NODES_OBJECT_SKELETON(g_object_new(NODE_TYPE_DIGITAL_OBJECT,
    "g-object-path","/com/lar/nodes/Digital1", "node-configuration",SYSCONFDIR"/candaemon/eds/Digital.eds", NULL));
    nodes_simple_set_device(nodes_object_get_simple(NODES_OBJECT(node)), g_dbus_object_get_object_path(G_DBUS_OBJECT(device)));
    nodes_simple_set_node_id(nodes_object_get_simple(NODES_OBJECT(node)),0x18);
    g_dbus_object_manager_server_export(G_DBUS_OBJECT_MANAGER_SERVER(node_control_app_nodes_manager()), G_DBUS_OBJECT_SKELETON(node));
    device->priv->noden = g_list_append(device->priv->noden, g_object_ref(node));
    g_object_unref(node);

    node = NODES_OBJECT_SKELETON(g_object_new(NODE_TYPE_DIGITAL_OBJECT,
    "g-object-path","/com/lar/nodes/Digital2", "node-configuration",SYSCONFDIR"/candaemon/eds/Digital.eds", NULL));
    nodes_simple_set_device(nodes_object_get_simple(NODES_OBJECT(node)), g_dbus_object_get_object_path(G_DBUS_OBJECT(device)));
    nodes_simple_set_node_id(nodes_object_get_simple(NODES_OBJECT(node)),0x19);
    g_dbus_object_manager_server_export(G_DBUS_OBJECT_MANAGER_SERVER(node_control_app_nodes_manager()), G_DBUS_OBJECT_SKELETON(node));
    device->priv->noden = g_list_append(device->priv->noden, g_object_ref(node));
    g_object_unref(node);

    node = NODES_OBJECT_SKELETON(g_object_new(NODE_TYPE_ANALOGEXT_OBJECT,
    "g-object-path","/com/lar/nodes/Analogext", "node-configuration",SYSCONFDIR"/candaemon/eds/AnalogExt.eds", NULL));
    nodes_simple_set_device(nodes_object_get_simple(NODES_OBJECT(node)), g_dbus_object_get_object_path(G_DBUS_OBJECT(device)));
    nodes_simple_set_node_id(nodes_object_get_simple(NODES_OBJECT(node)),0x1c);
    g_dbus_object_manager_server_export(G_DBUS_OBJECT_MANAGER_SERVER(node_control_app_nodes_manager()), G_DBUS_OBJECT_SKELETON(node));
    device->priv->noden = g_list_append(device->priv->noden, g_object_ref(node));
    service_simple_set_done(tera_service_get_simple(), TRUE);
    service_simple_emit_initialized(tera_service_get_simple(), TRUE);
    g_object_unref(node);

    if(device->priv->read_idle) g_source_remove(device->priv->read_idle);
    device->priv->read_idle = g_timeout_add_seconds(20, node_device_read_idle,device);

    return TRUE;
}

/*
// Wurde nur in 'node_control_app_initialize_all' benutzt. Diese Funktion ist
// aber gestrichen.
gboolean node_device_object_scan (NodeDeviceObject* device)
{
	IterateData* data;

	g_return_val_if_fail (device,                        FALSE);
	g_return_val_if_fail (NODE_IS_DEVICE_OBJECT(device), FALSE);
	g_return_val_if_fail (device->priv->is_open,         FALSE);

	g_message ("Initialize node type : %s ", g_type_name(NODE_TYPE_DIGITAL_OBJECT));
	g_message ("Initialize node type : %s ", g_type_name(NODE_TYPE_ANALOG_OBJECT));
	g_message ("Initialize node type : %s ", g_type_name(NODE_TYPE_MOTOR_OBJECT));
	g_message ("Initialize node type : %s ", g_type_name(NODE_TYPE_MOTOR3_OBJECT));
	g_message ("Initialize node type : %s ", g_type_name(NODE_TYPE_ANALOGEXT_OBJECT));

	if (! device -> priv -> scan) {
		g_warning ("Can bus scan object fail.. ");
		return FALSE;
	}

	nodes_simple_set_node_id (nodes_object_get_simple (NODES_OBJECT(device->priv->scan)), 1);
	//     Die Funktion 'node_object_childs_index' liefert die aus der .eds-Da-
	// tei "Scan.eds" generierten 'NodeIndexObject'-Objekte. Diese .eds-Datei
	// legt die Werte für die folgenden Eigenschaften fest:
	//     indexid   | parametername | objecttype | datatype | accesstype | pdomapping | lowlimit | highlimit
	//     ----------+-------------- +------------+----------+------------+------------+----------+----------
	//     1000      | DeviceType    | 0x7        | 0x0007   | ro         | 0          |-         |-
	//     1018sub0  | nrOfEntries   | 0x7        | 0x0005   | ro         | 0          | 1        | 4
	//     1018sub1  | Vendor ID     | 0x7        | 0x0007   | ro         | 0          |-         |-
	//     1018sub2  | Product Code  | 0x7        | 0x0007   | ro         | 0          |-         |-
	//     1018sub3  | Revision Nr   | 0x7        | 0x0007   | ro         | 0          |-         |-
	// 'lowlimit' und 'highlimit' sind nur für das Objekt mit 'indexid' =
	// "1018sub0" definiert.

	data = (IterateData*) malloc (sizeof *data);

	data -> device       = device;
	data -> scan         = device -> priv -> scan;
	data -> id           = 1;
	data -> list_to_read = node_object_childs_index (device -> priv -> scan);
	data -> fd           = device -> priv -> fd;
	data -> index        = NULL;

	g_timeout_add_full (G_PRIORITY_DEFAULT, 20, node_device_object_scann_idle, data, node_device_object_scann_idle_destroy);

	return TRUE;
}
*/

gboolean node_device_object_open (NodeDeviceObject* device)
{
	gint         flag;
	const gchar* path;

	g_return_val_if_fail (device,                        FALSE);
	g_return_val_if_fail (NODE_IS_DEVICE_OBJECT(device), FALSE);

	device -> priv -> is_open = FALSE;

	path = candevice_simple_get_path (candevice_object_get_simple (CANDEVICE_OBJECT (device)));
	flag = candevice_simple_get_flag (candevice_object_get_simple (CANDEVICE_OBJECT (device)));

	if (access (path, 0)) {
		candevice_simple_set_error_message (candevice_object_get_simple(CANDEVICE_OBJECT(device)), "Can device driver fail");
		g_critical                         ("Can device file %s not found",                        path);
	}
	else
	{
		if(device->priv->fd>0) close(device->priv->fd);
		device -> priv -> fd = open (path, flag);
	}
	if (device->priv->fd <= 0) {
		candevice_simple_set_error_message (candevice_object_get_simple(CANDEVICE_OBJECT(device)), "Can device can not open");
		g_critical                         ("Can device file %s can not open",                     path);
		return FALSE;
	}

	if (node_device_object_init_can (device))
		device -> priv -> is_open = TRUE;

	candevice_simple_set_is_open (candevice_object_get_simple(CANDEVICE_OBJECT(device)), device->priv->is_open);

	return device -> priv -> is_open;
}

static gboolean node_device_object_open_device (CandeviceSimple* interface, GDBusMethodInvocation* invocation, gpointer user_data)
{
	gboolean is_open = node_device_object_open (NODE_DEVICE_OBJECT (user_data));
	g_dbus_method_invocation_return_value (invocation, g_variant_new("(b)", is_open));
	return TRUE;
}

static void node_device_object_init (NodeDeviceObject* node_device_object)
{
	NodeDeviceObjectPrivate* priv = node_device_object_get_instance_private (node_device_object);
	priv -> scan  = NULL;
	priv -> fd    = 0;
	priv -> noden = NULL;
	node_device_object -> priv = priv;
}

static void node_device_object_constructed (GObject* object)
{
	NodeDeviceObject* device = NODE_DEVICE_OBJECT (object);
	CandeviceSimple*  simple = candevice_simple_skeleton_new();

	candevice_object_skeleton_set_simple (CANDEVICE_OBJECT_SKELETON(device), simple);
	candevice_simple_set_path            (simple, "/dev/can0");
	candevice_simple_set_flag            (simple, O_RDWR | O_NONBLOCK);
	candevice_simple_set_bautrate        (simple, 125);
	candevice_simple_set_scanned         (simple, FALSE);
	g_signal_connect                     (simple, "handle-open", G_CALLBACK(node_device_object_open_device), device);
	g_object_unref                       (simple);
//	node_device_object_create_scan_node  (device);
}

static void node_device_object_finalize(GObject *object) {
    NodeDeviceObject *data = NODE_DEVICE_OBJECT(object);
    if (data->priv->fd > 0) close(data->priv->fd);
    G_OBJECT_CLASS(node_device_object_parent_class)->finalize(object);
}

static void node_device_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(NODE_IS_DEVICE_OBJECT(object));
    // NodeDeviceObject *data = NODE_DEVICE_OBJECT(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void node_device_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(NODE_IS_DEVICE_OBJECT(object));
    // NodeDeviceObject *data = NODE_DEVICE_OBJECT(object);
    switch (prop_id) {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void node_device_object_class_init (NodeDeviceObjectClass* klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->finalize     = node_device_object_finalize;
	object_class->set_property = node_device_object_set_property;
	object_class->get_property = node_device_object_get_property;
	object_class->constructed  = node_device_object_constructed;
}

gboolean node_device_object_init_can(NodeDeviceObject *device) {
    g_return_val_if_fail(device != NULL, FALSE);
    g_return_val_if_fail(NODE_IS_DEVICE_OBJECT(device), FALSE);

    /*if(mktCheckPanelPCType()==PANEL_PC_CELERON) Nur für alte Celeron PC notwendig
    {
            usleep(200);
            printf("init Can4linux device for Celeron PC\n");
            device->cmd.cmd = CMD_STOP;
            if(ioctl(device->fd,CAN_IOCTL_COMMAND,&device->cmd) <= 0)return FALSE;
            usleep(200);
            Config_par_t   cfg;
            cfg.target =   CONF_TIMING;
            cfg.val1   =   device->baudrate;
            if(ioctl(device->fd,CAN_IOCTL_CONFIG,&cfg) <= 0)         return FALSE;
            usleep(200);
            device->cmd.cmd = CMD_RESET;
            if(ioctl(device->fd,CAN_IOCTL_COMMAND,&device->cmd) <= 0)return FALSE;
            usleep(200);
            device->cmd.cmd = CMD_START;
            if(ioctl(device->fd,CAN_IOCTL_COMMAND,&device->cmd) <= 0)return FALSE;
            usleep(900);
            node_trace("Initialize Can4linux done\n");
    }*/
    return TRUE;
}

gboolean node_device_object_read_index (NodeDeviceObject* device, NodeIndex* index)
{
	IterateData   data;
	unsigned long idFromCan;

	g_return_val_if_fail (device,                FALSE);
	g_return_val_if_fail (device->priv->is_open, FALSE);

	node_read_system_message (device);

	memset (&data, 0, sizeof data);
	data . fd    = device -> priv -> fd;
	data . index = index;

	switch (read_index (&data, &idFromCan)) {
		case READ_INDEX_DONE:
			return TRUE;

		case READ_INDEX_FAILED:
			return FALSE;

		case READ_INDEX_NODE_RESETED:
			broadcast_node_reset (device, idFromCan & 0x07f);
			return TRUE;
			// GS: Ich glaube nicht, dass das hier so gemeint war. Diese Rückga-
			// be wurde erst auffällig, als ich die Funktion 'read_index' einge-
			// führt hatte. Trotz des Nodereset wurde hier TRUE zurückgegeben
			// also ändere ich erstmal nicht.
	}

	return FALSE;
}



void node_device_clean_report(NodeDeviceObject *device) {
    GDir *  dir;
    GError *error    = NULL;
    gchar * cleanDir = "/var/log/tera";
    dir              = g_dir_open(cleanDir, 0, NULL);
    if (dir == NULL || error != NULL) {
        const gchar *name = NULL;
        while ((name = g_dir_read_name(dir))) {
            // g_debug("check EDS FILE %s",name);
            gchar *isNeedle = g_strstr_len(name, 256, "-scan-");
            if (isNeedle) {
                gchar *toRemove = g_build_path("/", cleanDir, name, NULL);
                g_remove(toRemove);
                g_free(toRemove);
                g_free(isNeedle);
            }
        }

        g_dir_close(dir);
    }
}

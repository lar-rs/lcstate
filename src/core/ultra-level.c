/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-security-data.c
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 *
mkt-security-data.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mkt-security-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "security-application-object.h"

#include <market-time.h>
#include <mktbus.h>
#include <mktlib.h>

#include "../config.h"
#include <glib/gi18n-lib.h>

enum
{
    PROP_0,
};

struct _SecurityApplicationObjectPrivate
{
	GDBusObjectManagerServer* security_manager;
	GDBusObjectManagerServer* users_manager;
	SecurityObjectSkeleton*   device_guard;
	SecurityDevice*           guard;
	GSettings*                security_settings;
	GCancellable*             check_errors_cancable;
	GCancellable*             check_limits_cancable;
	GCancellable*             check_messages_cancable;
	LarpcDevice*              pc_device;
	guint                     pending_logout;
};

G_DEFINE_TYPE_WITH_PRIVATE(SecurityApplicationObject, security_application_object, TERA_TYPE_SERVICE_OBJECT);

// ------------------------------ Check errors
// -------------------------------------

static gboolean security_run_wait_error_idle(gpointer data);

static void check_errors_warning_ready_callback(GSList *models, gpointer user_data)
{
    SecurityApplicationObject *app = SECURITY_APPLICATION_OBJECT(user_data);
    GSList *list = models;
    guint warnung = 0;
    if (list)
        warnung = g_slist_length(list);

    security_device_set_warnung(app->priv->guard, warnung > 0);
    security_device_set_warnings(app->priv->guard, warnung);
    GSList *l = NULL;
    GString *str = g_string_new("");
    for (l = models; l != NULL; l = l->next)
        g_string_append_printf(str, "%sE%d", str->len > 1 ? " " : "", mkt_error_number(MKT_ERROR(l->data)));

    security_device_set_warnung_str(app->priv->guard, str->str);
    g_string_free(str, TRUE);
    g_timeout_add_seconds(2, security_run_wait_error_idle, app);
}

static void check_errors_critical_ready_callback(GSList *models, gpointer user_data)
{
    SecurityApplicationObject *app = SECURITY_APPLICATION_OBJECT(user_data);
    GSList *list = models;
    guint criticals = 0;

    if (list)
        criticals = g_slist_length(list);
    security_device_set_critical(app->priv->guard, criticals > 0);
    security_device_set_criticals(app->priv->guard, criticals);
    GSList *l = NULL;
    GString *str = g_string_new("");
    for (l = models; l != NULL; l = l->next)
        g_string_append_printf(str, "%sE%d", str->len > 1 ? " " : "", mkt_error_number(MKT_ERROR(l->data)));
    security_device_set_critical_str(app->priv->guard, str->str);
    g_string_free(str, TRUE);

    mkt_model_look(MKT_TYPE_ERROR_MESSAGE, app->priv->check_errors_cancable, check_errors_warning_ready_callback, app,
                   "select * from %s where error_pending = 1 and error_type = %d ORDER BY error_number ASC",
                   g_type_name(MKT_TYPE_ERROR_MESSAGE), MKT_ERROR_WARNING);
}

gboolean security_run_wait_error_idle(gpointer data)
{
    SecurityApplicationObject *app = SECURITY_APPLICATION_OBJECT(data);
    mkt_model_look(MKT_TYPE_ERROR_MESSAGE, app->priv->check_errors_cancable, check_errors_critical_ready_callback, app,
                   "select * from %s where error_pending = 1 and error_type = %d ORDER BY error_number ASC",
                   g_type_name(MKT_TYPE_ERROR_MESSAGE), MKT_ERROR_CRITICAL);
    return FALSE;
}

// ------------------------------ Check errors
// -------------------------------------
static gboolean security_run_wait_limit_idle(gpointer data);

static void check_limit_ready_callback(GSList *models, gpointer user_data)
{
    SecurityApplicationObject *app = SECURITY_APPLICATION_OBJECT(user_data);
    guint limits = g_slist_length(models);
    security_device_set_limits(app->priv->guard, limits);
    GSList *l = NULL;
    GString *str = g_string_new("");
    for (l = models; l != NULL; l = l->next)
        g_string_append_printf(
            str, "%s%s%s", str->len > 1 ? " " : "", mkt_limit_name(MKT_LIMIT(l->data)), mkt_limit_pending(MKT_LIMIT(l->data)) > 0 ? "max" : "min");

    // g_debug("LIMIT:%s",str->str);
    security_device_set_limits_str(app->priv->guard, str->str);
    g_string_free(str, TRUE);
    g_timeout_add_seconds(2, security_run_wait_limit_idle, app);
}

gboolean security_run_wait_limit_idle(gpointer data)
{
    SecurityApplicationObject *app = SECURITY_APPLICATION_OBJECT(data);
    mkt_model_look(MKT_TYPE_LIMIT_MESSAGE, app->priv->check_limits_cancable, check_limit_ready_callback, app, "select * from %s where limit_pending <> 0 and "
                                                                                                              "limit_activated = 1 ORDER BY limit_name ASC;",
                   g_type_name(MKT_TYPE_LIMIT_MESSAGE));
    return FALSE;
}

//-------------------------security message status status
//-----------------------------------------------


static gboolean security_run_wait_device_status_idle(gpointer data);

static void check_all_device_status_ready_callback(GSList *models, gpointer user_data)
{
    SecurityApplicationObject *app = SECURITY_APPLICATION_OBJECT(user_data);
    GSList *list = models;
    GString *string = g_string_new("");
    for (list = models; list != NULL; list = list->next)
    {
        g_string_append_printf(string, "%s%s", mkt_status_signification(MKT_STATUS(list->data)), string->len > 0 ? " " : "");
    }
    security_device_set_control_status(app->priv->guard, string->str);
    g_string_free(string, TRUE);
    g_timeout_add_seconds(2, security_run_wait_device_status_idle, app);
}

gboolean security_run_wait_device_status_idle(gpointer data)
{
    SecurityApplicationObject *app = SECURITY_APPLICATION_OBJECT(data);
    mkt_model_look(MKT_TYPE_STATUS_MODEL, app->priv->check_limits_cancable, check_all_device_status_ready_callback, app,
                   "select * from %s where status_active = 1", g_type_name(MKT_TYPE_STATUS_MODEL));
    return FALSE;
}

static void secutity_check_all(SecurityApplicationObject *app)
{
    g_cancellable_cancel(app->priv->check_errors_cancable);
    g_cancellable_cancel(app->priv->check_limits_cancable);
    g_cancellable_reset(app->priv->check_errors_cancable);
    g_cancellable_reset(app->priv->check_limits_cancable);
    g_timeout_add_seconds(2, security_run_wait_error_idle, app);
    g_timeout_add_seconds(2, security_run_wait_limit_idle, app);
    g_timeout_add_seconds(2, security_run_wait_device_status_idle, app);
}

// Aktualisiert die Eigenschaften "level2", "level3", "level4" und "level5" des
// privaten 'SecurityDevice'-Elementes 'guard' in Abhängigkeit der Eigenschaft
// "level" desselben Objektes.
static void security_application_update_level (SecurityDevice* guard)
{
	// Liest die Eigenschaft "level" des privaten 'SecurityDevice'-Elementes
	// 'guard'.
    guint level = security_device_get_level (guard);

	// Setzt die Eigenschaft "level5" des privaten 'SecurityDevice'-Elementes
	// 'guard'
    security_device_set_level5 (guard, level>=5);
    security_device_set_level4 (guard, level>=4);
    security_device_set_level3 (guard, level>=3);
    security_device_set_level2 (guard, level>=2);
}

// Das Einfügen eines / vor dem Kommentarzeichen /* setzt den Level auf 5.
/*
#include <stdio.h>
static guint getLevel (void)
{
	static char file [] = "/home/ultra/usb.txt";

	int   ch;
	FILE* fp;

	fp = fopen (file, "r");

	if (! fp)
		return 0;

	ch = fgetc (fp);
	fclose (fp);

	switch (ch) {
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
	}

	return 0;
}

typedef struct checkUsb_data {
	SecurityApplicationObject* security_application;
	LarpcDevice*               pcDev;
	guint                      level;
}
checkUsb_data;

static void security_system_level_changed(LarpcDevice *pc_device, GParamSpec *pspec, SecurityApplicationObjectPrivate *security_application);
static SecurityApplicationObject*        security_application = NULL;
static SecurityApplicationObjectPrivate* priv                 = NULL;
static SecurityApplicationObjectPrivate* prv                  = NULL;

static gboolean checkUsb (gpointer dataArg)
{
	guint          level;
	checkUsb_data* data;

	data  = dataArg;
	level = getLevel ();

	if (level != data->level) {
		data -> level = level;
		security_system_level_changed (data->pcDev, NULL, prv);
	}

	return TRUE;
}

static guint myLarpc_device_get_level (LarpcDevice* pcDev)
{
	static checkUsb_data data = {NULL, NULL, 0};

	if (! data . pcDev) {
		data . pcDev = pcDev;
		data . level = larpc_device_get_level (pcDev);;

		checkUsb      (& data);
		g_timeout_add (300, checkUsb, &data);
	}

	return data . level;
}
#define larpc_device_get_level(pcDev)  ((priv ? (prv = priv) : (prv = security_application->priv)), myLarpc_device_get_level(pcDev))
//*/

static gboolean security_manager_logout_callback (SecurityDevice* device, GDBusMethodInvocation* invocation, gpointer user_data)
{
	guint                      level;
	SecurityApplicationObject* security_application = SECURITY_APPLICATION_OBJECT(user_data);
	guint                      security_level;
	gboolean                   updated;

	security_device_set_user (security_application->priv->guard, "unknown");

	level          = larpc_device_get_level    (security_application -> priv -> pc_device);
	security_level = security_device_get_level (security_application -> priv -> guard);
	updated        = level != security_level;

	security_device_set_level (security_application->priv->guard, level);

	if (updated)
		security_application_update_level (security_application -> priv -> guard);

	security_device_emit_system_logout    (security_application->priv->guard);
	g_dbus_method_invocation_return_value (invocation, g_variant_new("(b)", TRUE));

	return TRUE;
}

// Wird nach dem Einrichten der 'MktUserObject'-Objekte bei der Initialisierung
// oder wenn sich die Eigenschaft "level" des privaten 'LarpcDevice'-Objektes
// ändert aufgerufen.
// Setzt bei allen 'MktUserObject'-Objekten aus 'users_manager' das private Ele-
// ment 'level_stick' auf die Benutzerebene aus dem privaten 'LarpcDevice'-Ele-
// ment 'pc_device'.
static void security_system_level_set_all_user (SecurityApplicationObjectPrivate* priv)
{
	// Liest aus dem privaten 'LarpcDevice'-Element 'pc_device' die Benutzerebe-
	// ne.
    guint  level = larpc_device_get_level            (                       priv -> pc_device);

	// Alle 'MktUserObject'-Objekte aus 'users_manager'.
    GList* users = g_dbus_object_manager_get_objects (G_DBUS_OBJECT_MANAGER (priv -> users_manager));
    GList* l     = NULL;

    for (l=users; l; l=l->next)
		// Setzt das private Element 'level_stick' des 'MktUserObject'-Objektes
		// auf die Benutzerebene aus 'pc_device'.
        mkt_user_object_level_stick (MKT_USER_OBJECT(l->data), level);

    g_list_free (users);
}

// Wird nach dem Einrichten der 'MktUserObject'-Objekte bei der Initialisierung
// oder wenn sich die Eigenschaft "level" des privaten 'LarpcDevice'-Objektes
// ändert aufgerufen.
static void security_system_update_level (LarpcDevice* pc_device, SecurityApplicationObjectPrivate* priv)
{
	// Liest aus dem privaten 'LarpcDevice'-Element 'pc_device' die Benutzerebe-
	// ne.
	guint    level          = larpc_device_get_level    (pc_device);

	// Liest die Eigenschaft "level" des privaten 'SecurityDevice'-Elementes
	// 'guard', also die Ebene des z. Zt. angemeldeten Benutzers.
	guint    security_level = security_device_get_level (priv->guard);

	// Die Benutzerebene aus 'pc_device' (Die Ebene des eingesteckten USB-
	// Sticks) unterscheidet sich von der Benutzerebene aus 'guard' (Die Ebene
	// des z. Zt. angemeldeten Benutzers).
	// Wenn 'updated' müssen die Eigenschaften "level", "level2", "level3",
	// "level4" und "level5" des privaten 'SecurityDevice'-Elementes 'guard' ge-
	// setzt werden.
	gboolean updated        = level != security_level;

	// Die Benutzerebene aus 'pc_device' (Die Ebene des eingesteckten USB-
	// Sticks) ist kleiner als die Benutzerebene aus 'guard' (Die Ebene des z.
	// Zt. angemeldeten Benutzers).
	// Wenn 'logout' wird das Signal "system-logout" des privaten
	// 'SecurityDevice'-Elementes 'guard' gesendet.
	gboolean logout         = level <  security_level;

	// Setzt die Eigenschaft "security-usb" des privaten 'SecurityDevice'-Ele-
	// mentes 'guard' auf 'level > 2'. Falls TRUE, wird in der Klasse
	// 'LgdmSidebar' die Ikone "usb-lock" geladen.
	security_device_set_security_usb   (priv->guard, level > 2);

	// Setzt bei allen 'MktUserObject'-Objekten aus dem privaten Element
	// 'users_manager' das private Element 'level_stick' auf die Benutzerebene
	// aus dem privaten 'LarpcDevice'-Element 'pc_device'.
	security_system_level_set_all_user (priv);

	security_device_set_level (priv->guard, level);

	if (updated)
		security_application_update_level  (priv -> guard);

	if (logout)
		security_device_emit_system_logout (priv -> guard);
}

static gboolean security_system_user_logout (gpointer dataArg)
{
	guint                             level;
	SecurityApplicationObjectPrivate* priv = dataArg;

	priv -> pending_logout = 0;

	security_device_set_user (priv->guard, "unknown");

	level = larpc_device_get_level (priv->pc_device);

	security_device_set_level          (priv->guard, level);
	security_application_update_level  (priv->guard);
	security_device_emit_system_logout (priv->guard);

	return FALSE;
}

// Wenn sich die Eigenschaft "level" des privaten 'LarpcDevice'-Objektes ändert.
static void security_system_level_changed (LarpcDevice* pc_device, GParamSpec* pspec, SecurityApplicationObjectPrivate* priv)
{
	if (priv -> pending_logout) {
		g_source_remove (priv -> pending_logout);
		priv -> pending_logout = 0;
	}

	security_system_update_level (pc_device, priv);
}

// 1. Setzt die Eigenschaft "login" des ausgewählten 'UsersUser'-Objektes auf
//    TRUE.
// 2. Setzt die Eigenschaft "user" des privaten 'SecurityDevice'-Elementes
//    'guard' auf den Namen des ausgewählten 'UsersUser'-Objektes.
// 3. Liest aus dem privaten 'LarpcDevice'-Element 'pc_device' die Benutzerebe-
//    ne und vergleicht diese mit der Ebene des ausgewählten 'UsersUser'-Objek-
//    tes.
//    - Wenn die ausgewählte Ebene größer ist als die Benutzerebene aus
//      'pc_device' (es ist ein Login mit Passwort erfolgt) wird die Eigenschaft
//      "level" des privaten 'SecurityDevice'-Elementes 'guard' auf die ausge-
//      wählte Ebene gesetzt.
//    - Wenn die ausgewählte Ebene kleiner als die oder gleich der Benutzerebene
//      aus 'pc_device' ist (es ist ein Login mit USB-Stick erfolgt) wird die
//      Eigenschaft "level" des privaten 'SecurityDevice'-Elementes 'guard' auf
//      die Ebene aus 'pc_device' gesetzt.
//      DAS HEISST: ES WIRD IMMER DIE EBENE DES LEVELE-STICKS AKTIVIERT, UNAB-
//      HÄNGING DAVON, WELCHE EBENE DER BENUTZER AUSWÄHLT.
// 4. Aktualisiert die Eigenschaften "level2", "level3", "level4" und "level5"
//    des privaten 'SecurityDevice'-Elementes 'guard' in Abhängigkeit der Eigen-
//    schaft "level" (die oben soeben gesetzt wurde) desselben Objektes.
// 5. Sendet das Signal "system-login" des privaten 'SecurityDevice'-Elementes
//    'guard'.
static void security_system_user_login (UsersUser* user, gboolean auto_logout, SecurityApplicationObjectPrivate* priv)
{
	guint level;

	if (priv -> pending_logout) {
		g_source_remove (priv -> pending_logout);
		priv -> pending_logout = 0;
	}

	if (auto_logout)
		priv -> pending_logout = g_timeout_add_seconds (2700, security_system_user_logout, priv);

	security_device_set_user (priv->guard, users_user_get_name(user));
	// Setzt die Eigenschaft "user" des privaten 'SecurityDevice'-Elementes
	// 'guard' auf den Namen des ausgewählten 'UsersUser'-Objektes.

	// Ebene des ausgewählten 'UsersUser'-Objektes.
	level = users_user_get_level (user);

	// Liest aus dem privaten 'LarpcDevice'-Element 'pc_device' die Benutzerebe-
	// ne und vergleicht diese mit der Ebene des ausgewählten 'UsersUser'-Objek-
	// tes.
	if (larpc_device_get_level(priv->pc_device) < level)
		// Wenn die ausgewählte Ebene größer ist als die Benutzerebene aus
		// 'pc_device' (es ist ein Login mit Passwort erfolgt) wird die Eigen-
		// schaft "level" des privaten 'SecurityDevice'-Elementes 'guard' auf
		// die ausgewählte Ebene gesetzt.
		security_device_set_level (priv->guard, level);
	else
		// Wenn die ausgewählte Ebene kleiner als die oder gleich der Benutzere-
		// bene aus 'pc_device' ist (es ist ein Login mit USB-Stick erfolgt)
		// wird die Eigenschaft "level" des privaten 'SecurityDevice'-Elementes
		// 'guard' auf die Ebene aus 'pc_device' gesetzt.
		// DAS HEISST: ES WIRD IMMER DIE EBENE DES LEVELE-STICKS AKTIVIERT, UN-
		// ABHÄNGING DAVON, WELCHE EBENE DER BENUTZER AUSWÄHLT.
		security_device_set_level (priv->guard, larpc_device_get_level(priv->pc_device));

	// Aktualisiert die Eigenschaften "level2", "level3", "level4" und "level5"
	// des privaten 'SecurityDevice'-Elementes 'guard' in Abhängigkeit der Ei-
	// genschaft "level" (die oben soeben gesetzt wurde) desselben Objektes.
	security_application_update_level (priv -> guard);
	security_device_emit_system_login (priv -> guard);
	// Sendet das Signal "system-login" des privaten 'SecurityDevice'-Elementes
	// 'guard'.
}

static gboolean security_ceck_done_cb(gpointer user_data) {
    service_simple_set_done(tera_service_get_simple(), TRUE);
    service_simple_emit_initialized(tera_service_get_simple(), TRUE);
    return FALSE;
}

/* Erzeugt wird ein 'MktUserObject'-Objekt. Die Klasse 'MktUserObject' ist von
-- 'UsersObjectSkeleton' abgeleitet. 'UsersObjectSkeleton' enthält eine Schnitt-
-- stellenklasse 'UsersUser' mit der Schnittstelle 'UsersUserIface' =
-- 'UsersUserInterface'.
--
-- 'MktUserObject' hat  | entspricht hier                    | mit den Werten
-- folgende Eigenschaf- | dem Argument                       |
-- ten:                 |                                    |
--     "user-level"     | 'userLevel'                        | 1       | 2          |
--     "user-name"      | 'userName' wird zu                 | "guest" | "operator" | ""
--                      | "/com/lar/tera/security/"+userName |         |            |
*/
static void setUser (SecurityApplicationObject* security_application, const gchar* dbusName, guint userLevel, const gchar* userName, const gchar* userdefPw)
{
	GObject*                  object;
	gchar*                    object_path;
	UsersUser*                user;
	GDBusObjectManagerServer* users_manager;

	object_path   = g_strdup_printf       ("%s/%s",              TERA_USERS_MANAGER, dbusName);
	object        = g_object_new          (MKT_TYPE_USER_OBJECT, "g-object-path", object_path, "user-level", userLevel, "user-name", userName, "user-default-password", userdefPw, NULL);
	user          = users_object_get_user (USERS_OBJECT (object));
	users_manager = security_application -> priv -> users_manager;

	g_dbus_object_manager_server_export (users_manager,  G_DBUS_OBJECT_SKELETON(object));
	g_signal_connect                    (user, "login",  G_CALLBACK(security_system_user_login),  security_application->priv);
	g_signal_connect                    (user, "logout", G_CALLBACK(security_system_user_logout), security_application->priv);
	g_object_unref                      (object);
	g_free                              (object_path);
}

static void security_application_object_initialize (SecurityApplicationObject* security_application)
{
	gchar*         device_name;
	MktUserObject* object;
	gchar*         object_path;

	security_application -> priv -> pc_device      = mkt_pc_manager_client_get_device ();
	security_application -> priv -> device_guard   = security_object_skeleton_new     (TERA_SECURITY_GUARD);
	security_application -> priv -> guard          = security_device_skeleton_new     ();
	security_application -> priv -> pending_logout = 0;

	security_device_set_busy            (security_application->priv->guard,        FALSE);
	security_object_skeleton_set_device (security_application->priv->device_guard, security_application->priv->guard);
	mkt_model_exec_async                (MKT_TYPE_STATUS_MODEL, NULL, NULL, NULL, "UPGRADE %s status_active  = 0 WHERE status_active = 1;", g_type_name(MKT_TYPE_STATUS_MODEL));
	g_signal_connect                    (security_application->priv->guard,        "handle-logout", G_CALLBACK(security_manager_logout_callback), security_application);
	security_device_set_red_button      (security_application->priv->guard,        FALSE);
	security_device_set_green_button    (security_application->priv->guard,        FALSE);

	device_name = g_settings_get_string (security_application->priv->security_settings, "device-serial");
	security_device_set_device_name     (security_application->priv->guard,            device_name);
	g_free                              (device_name);

	g_dbus_object_manager_server_export (security_application->priv->security_manager, G_DBUS_OBJECT_SKELETON(security_application->priv->device_guard));
	secutity_check_all                  (security_application);

	gboolean auto_start = g_settings_get_boolean (security_application->priv->security_settings, "auto-start");
	security_device_set_autostart (security_application->priv->guard, auto_start);

	if (security_device_get_autostart (security_application -> priv -> guard))
		security_device_set_online (security_application -> priv -> guard, TRUE);

	setUser (security_application, "guest",    1, "Measurement Overview", NULL);
	setUser (security_application, "operator", 2, "Operator Level",       "lar");
	setUser (security_application, "expert",   3, "Expert Level",         NULL);
	setUser (security_application, "service",  4, "Service",              NULL);

	security_system_update_level (security_application->priv->pc_device, security_application->priv);
	g_signal_connect             (security_application->priv->pc_device, "notify::level", G_CALLBACK(security_system_level_changed), security_application->priv);
	// Wenn sich die Eigenschaft "level" des privaten 'LarpcDevice'-Objektes än-
	// dert.

	g_timeout_add_seconds (1, security_ceck_done_cb, security_application);

//	#define TEST_MODE
	#ifdef TEST_MODE
		/*
		object_path = g_strdup_printf("%s/level3", TERA_USERS_MANAGER);
		object = MKT_USER_OBJECT(
			g_object_new(MKT_TYPE_USER_OBJECT, "g-object-path", object_path, "user-level", 3, "user-name", "TESTLEVEL3", "user-autologin", TRUE, NULL));
		g_dbus_object_manager_server_export(security_application->priv->users_manager, G_DBUS_OBJECT_SKELETON(object));
		g_signal_connect(users_object_get_user(USERS_OBJECT(object)), "login", G_CALLBACK(security_system_user_login), security_application);
		g_signal_connect(users_object_get_user(USERS_OBJECT(object)), "logout", G_CALLBACK(security_system_user_logout), security_application);
		g_object_unref(object);
		g_free(object_path);

		object_path = g_strdup_printf("%s/level4", TERA_USERS_MANAGER);
		object = MKT_USER_OBJECT(
			g_object_new(MKT_TYPE_USER_OBJECT, "g-object-path", object_path, "user-level", 4, "user-name", "TESTLEVEL4", "user-autologin", TRUE, NULL));
		g_dbus_object_manager_server_export(security_application->priv->users_manager, G_DBUS_OBJECT_SKELETON(object));
		g_signal_connect(users_object_get_user(USERS_OBJECT(object)), "login", G_CALLBACK(security_system_user_login), security_application);
		g_signal_connect(users_object_get_user(USERS_OBJECT(object)), "logout", G_CALLBACK(security_system_user_logout), security_application);
		g_object_unref(object);
		g_free(object_path);
		*/

		object_path = g_strdup_printf("%s/level5", TERA_USERS_MANAGER);
		object = MKT_USER_OBJECT(
			g_object_new(MKT_TYPE_USER_OBJECT, "g-object-path", object_path, "user-level", 5, "user-name", "TEST LEVEL5", "user-autologin", TRUE, NULL));
		g_dbus_object_manager_server_export(security_application->priv->users_manager, G_DBUS_OBJECT_SKELETON(object));
		g_signal_connect(users_object_get_user(USERS_OBJECT(object)), "login", G_CALLBACK(security_system_user_login), security_application);
		g_signal_connect(users_object_get_user(USERS_OBJECT(object)), "logout", G_CALLBACK(security_system_user_logout), security_application);
		g_object_unref(object);
		g_free(object_path); 
	#endif
}

static void security_application_object_init(SecurityApplicationObject *security_application_object) {
    SecurityApplicationObjectPrivate *priv = security_application_object_get_instance_private(security_application_object);
    priv->security_manager = NULL;
    priv->security_settings = g_settings_new("com.lar.tera.security");
    priv->check_errors_cancable = g_cancellable_new();
    priv->check_limits_cancable = g_cancellable_new();
    priv->check_messages_cancable = g_cancellable_new();
    security_application_object->priv = priv;
}

static void security_application_object_finalize(GObject *object) {
    SecurityApplicationObject *security_application = SECURITY_APPLICATION_OBJECT(object);
    if (security_application->priv->security_manager)
        g_object_unref(security_application->priv->security_manager);
    G_OBJECT_CLASS(security_application_object_parent_class)->finalize(object);
}

static void security_application_object_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    g_return_if_fail(SECURITY_IS_APPLICATION_OBJECT(object));
    // SecurityApplicationObject *data = SECURITY_APPLICATION_OBJECT(object);
    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void security_application_object_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    g_return_if_fail(SECURITY_IS_APPLICATION_OBJECT(object));
    // SecurityApplicationObject *data = SECURITY_APPLICATION_OBJECT(object);
    switch (prop_id)
    {

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void security_application_object_activated(TeraServiceObject *service) {
    SecurityApplicationObject *security_application = SECURITY_APPLICATION_OBJECT(service);
    security_application->priv->security_manager = g_dbus_object_manager_server_new(TERA_SECURITY_MANAGER);
    g_dbus_object_manager_server_set_connection(security_application->priv->security_manager, tera_service_dbus_connection());
    security_application->priv->users_manager = g_dbus_object_manager_server_new(TERA_USERS_MANAGER);
    g_dbus_object_manager_server_set_connection(security_application->priv->users_manager, tera_service_dbus_connection());
    security_application_object_initialize(security_application);
}

static void security_application_object_class_init(SecurityApplicationObjectClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    TeraServiceObjectClass *app_class = TERA_SERVICE_OBJECT_CLASS(klass);
    object_class->finalize = security_application_object_finalize;
    object_class->set_property = security_application_object_set_property;
    object_class->get_property = security_application_object_get_property;
    app_class->activated = security_application_object_activated;
}

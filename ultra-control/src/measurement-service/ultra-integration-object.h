/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @file  ultra-integration-object.h	Valve object header
 * @brief Pump object header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 * @author A.Smolkov
 * $Id: $ $URL: $
 */

#ifndef _ULTRA_INTEGRATION_OBJECT_H_
#define _ULTRA_INTEGRATION_OBJECT_H_

#include <glib.h>
#include <gio/gio.h>
#include <mktbus.h>
#include <mktlib.h>

G_BEGIN_DECLS

#define ULTRA_TYPE_INTEGRATION_OBJECT (ultra_integration_object_get_type())
#define ULTRA_INTEGRATION_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), ULTRA_TYPE_INTEGRATION_OBJECT, UltraIntegrationObject))
#define ULTRA_INTEGRATION_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), ULTRA_TYPE_INTEGRATION_OBJECT, UltraIntegrationObjectClass))
#define ULTRA_IS_INTEGRATION_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ULTRA_TYPE_INTEGRATION_OBJECT))
#define ULTRA_IS_INTEGRATION_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), ULTRA_TYPE_INTEGRATION_OBJECT))
#define ULTRA_INTEGRATION_OBJECT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), ULTRA_TYPE_INTEGRATION_OBJECT, UltraIntegrationObjectClass))

typedef struct _UltraIntegrationObjectClass UltraIntegrationObjectClass;
typedef struct _UltraIntegrationObject UltraIntegrationObject;
typedef struct _UltraIntegrationObjectPrivate UltraIntegrationObjectPrivate;

struct _UltraIntegrationObjectClass
{
	IntegrationObjectSkeletonClass parent_class;
};

struct _UltraIntegrationObject
{
	IntegrationObjectSkeleton parent_instance;
	UltraIntegrationObjectPrivate *priv;
};

GType ultra_integration_object_get_type(void) G_GNUC_CONST;

void ultra_integration_server_run(GDBusConnection *connection);
GDBusObjectManager *ultra_integration_server(void);

GList *ultra_integrations();
IntegrationObject *ultra_integration(const gchar *patch);

gboolean ultra_integration_object_analyze(UltraIntegrationObject *integration, guint64 creator, guint trigger, gboolean is_tic);
gboolean ultra_integration_object_justifying_run(UltraIntegrationObject *integration);
gboolean ultra_integration_object_integrating_run(UltraIntegrationObject *integration);
gboolean ultra_integration_object_calculate_justification(UltraIntegrationObject *integration);
gboolean ultra_integration_object_calculate_integration(UltraIntegrationObject *integration);
void ultra_integration_object_analyse_break(UltraIntegrationObject *integration);

void ultra_integration_stop(void);

IntegrationObject *ultra_integration_get_ndir1();
IntegrationObject *ultra_integration_get_ndir2();
IntegrationObject *ultra_integration_get_tnb();
IntegrationObject *ultra_integration_get_codo();

G_END_DECLS

#endif /* _ULTRA_INTEGRATION_OBJECT_H_ */

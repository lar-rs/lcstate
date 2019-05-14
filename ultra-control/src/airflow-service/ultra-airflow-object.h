/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @defgroup UltimateLibrary
 * @defgroup UltraAirflowObject
 * @ingroup  UltraAirflowObject
 * @{
 * @file  ultra-airflow-object.h	AXIS object header
 * @brief This is AXIS object header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 * $Id: $ $URL: $
 */
#ifndef _ULTRA_AIRFLOW_OBJECT_H_
#define _ULTRA_AIRFLOW_OBJECT_H_

#include <mktlib.h>
#include <mktbus.h>


G_BEGIN_DECLS

#define ULTRA_TYPE_AIRFLOW_OBJECT             (ultra_airflow_object_get_type ())
#define ULTRA_AIRFLOW_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_AIRFLOW_OBJECT, UltraAirflowObject))
#define ULTRA_AIRFLOW_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  ULTRA_TYPE_AIRFLOW_OBJECT, UltraAirflowObjectClass))
#define ULTRA_IS_AIRFLOW_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_AIRFLOW_OBJECT))
#define ULTRA_IS_AIRFLOW_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  ULTRA_TYPE_AIRFLOW_OBJECT))
#define ULTRA_AIRFLOW_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  ULTRA_TYPE_AIRFLOW_OBJECT, UltraAirflowObjectClass))

typedef struct _UltraAirflowObjectClass         UltraAirflowObjectClass;
typedef struct _UltraAirflowObject              UltraAirflowObject;
typedef struct _UltraAirflowObjectPrivate       UltraAirflowObjectPrivate;


struct _UltraAirflowObjectClass
{
	AirflowObjectSkeletonClass                  parent_class;
};

struct _UltraAirflowObject
{
	AirflowObjectSkeleton                       parent_instance;
	UltraAirflowObjectPrivate                   *priv;
};

GType                   ultra_airflow_object_get_type                 ( void ) G_GNUC_CONST;

G_END_DECLS

#endif /* _ULTRA_AIRFLOW_OBJECT_H_ */
/** @} */

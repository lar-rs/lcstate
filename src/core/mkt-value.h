/*
 * mkt-value.h
 *
 *  Created on: 02.05.2013
 *      Author: sascha
 */

#ifndef MKT_VALUE_H_
#define MKT_VALUE_H_


#include <glib.h>
#include <glib-object.h>



typedef struct _MktParameter MktParameter;


#define MKT_TIMEZONE_INVALID (2*12*60*60)

#define MKT_TYPE_TIME (mkt_time_get_type())
#define MKT_TYPE_TIMESTAMP (mkt_timestamp_get_type())
#define MKT_VALUE_HOLDS_TIME(value)            G_VALUE_HOLDS(value, MKT_TYPE_TIME)
#define MKT_VALUE_HOLDS_TIMESTAMP(value)       G_VALUE_HOLDS(value, MKT_TYPE_TIMESTAMP)

enum 
{
	MKT_PARAMETER_PROPERTY = 1 < 0,
	MKT_PARAMETER_VALUE    = 1 < 1,
	MKT_PARAMETER_SIGNAL   = 1 < 2
};



struct _MktParameter
{
	gchar    *name;
	GValue   *value;
	GValue   *old;
	guint     flag;
	GObject  *instance;
	gchar    *select;  //GType:id:prop
};



/**
 * MktTime:
 * @hour:
 * @minute:
 * @second:
 * @fraction:
 * @timezone:
 */
typedef struct {
	gushort hour;
	gushort minute;
	gushort second;
	gulong  fraction;
	glong   timezone;	/* # of seconds to the east UTC */
} MktTime;

/**
 * MktTimestamp:
 * @year: representation of the date
 * @month: month representation of the date, as a number between 1 and 12
 * @day: day representation of the date, as a number between 1 and 31
 * @hour:
 * @minute:
 * @second:
 * @fraction:
 * @timezone:
 */
typedef struct {
	gshort  year;
	gushort month;
	gushort day;
	gushort hour;
	gushort minute;
	gushort second;
	gulong  fraction;
	glong   timezone;	/* # of seconds to the east UTC */
} MktTimestamp;


#define mkt_value_isa(value, type) (G_VALUE_HOLDS(value, type))

GValue *               mkt_value_new                          ( GType type );
void                   mkt_value_free                         ( GValue *value );

gboolean               mkt_set_gvalue_from_string             ( GValue *value, const gchar *as_string);
gchar *                mkt_value_stringify                    ( const GValue *value );
const gchar *          mkt_value_stringify_static             ( const GValue *value );
const gchar *          mkt_value_stringify_double             ( gdouble value );
const gchar *          mkt_value_stringify_double_enUS        ( gdouble value );
gchar *                mkt_value_dup_double                   ( gdouble value );
gchar *                mkt_value_dup_double_enUS              ( gdouble value );
//gchar *                mkt_value_dup_stringify_double         ( gdouble value );
//gchar *                mkt_value_dup_stringify_double_enUS    ( gdouble value );


gboolean               mkt_strv_equal0                        ( gchar **a, gchar **b );
gboolean               mkt_value_g_value_equal                ( const GValue *a, const GValue *b );
gint                   mkt_value_equal                        ( const GValue *a, const GValue *b );

void                   mkt_list_free_full                     ( GList *list, GDestroyNotify  free_func );
void                   mkt_slist_free_full                    ( GSList *list, GDestroyNotify  free_func );
gboolean               mkt_value_string_digit                 ( const gchar *value );


const MktTime*         mkt_value_get_time                      ( const GValue *value );
void                   mkt_value_set_time                      ( GValue *value, const MktTime *val );
const MktTimestamp*    mkt_value_get_timestamp                 ( const GValue *value );
void                   mkt_value_set_timestamp                 ( GValue *value, const MktTimestamp *val );

GType                  mkt_time_get_type (void) G_GNUC_CONST;
gpointer               mkt_time_copy (gpointer boxed);
void                   mkt_time_free (gpointer boxed);
gboolean               mkt_time_valid (const MktTime *time);
gboolean               mkt_parse_iso8601_time (MktTime *timegda, const gchar *value);

GType                  mkt_timestamp_get_type (void) G_GNUC_CONST;
gpointer               mkt_timestamp_copy (gpointer boxed);
void                   mkt_timestamp_free (gpointer boxed);
gboolean               mkt_timestamp_valid (const MktTimestamp *timestamp);
gboolean               mkt_parse_iso8601_timestamp (MktTimestamp *timestamp, const gchar *value);

MktParameter*          mkt_parameter_new                      ( GObject  *instance , const gchar *name , GType type ,guint flag );
void                   mkt_parameter_free                     ( MktParameter *parameter );
void                   mkt_parameter_set_select               ( MktParameter *parameter, const gchar *select );

#endif /* MKT_VALUE_H_ */

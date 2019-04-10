/**
 * @defgroup time
 * @ingroup market
 * @brief   Module description
 * @ingroup time
 * @{
 * @file  market-system.h
 * @brief time header file description
 *
 * (c) 2008 - 2013 LAR Process Analysers AG - All rights reserved.
 *
 * @author system
 *
 * $Id: $ $URL: $
 */

#ifndef __MARKET_TIME_H_INCLUDED
#define __MARKET_TIME_H_INCLUDED (1)

// #undef __need_timespec
// #define __need_timespec

#include <time.h>
#include <stdlib.h>
#include <glib.h>


#define TimeStrClockTemplate "1970-01-01T00:00:00"
enum {TimeStrClockSize = sizeof TimeStrClockTemplate, TimeStrClockLen = TimeStrClockSize - 1};

#define MARKET_TIME_FORMAT_FULL "%Y-%m-%dT%H:%M:%S"
#define MARKET_TIME_FORMAT_FILENAME "%Y-%m-%dT%H-%M-%S"


//
// time - do not use the underlying structure to stay compatible to future revisions
//

/**
 * this version uses 32 bit of unix time in seconds since 1970 and 32 bit tv_nsec for nanoseconds
 **/ 
typedef struct timespec mktTime_t;


/**
 * Defining days and time of day for repetive actions
 **/ 

/**
 * the current market time
 **/
mktTime_t mktNow(void);

/* Liefert eine Zeichenkette, die den formatierten Zeitstempel enthält.
-- Zu den bereits bekannten Formatbeschreibern von 'strftime' wird '%N'
-- für 9 Stellen Nanosekunden hinzugefügt.
-- Wenn der Platz von 256 Bytes (255 Zeichen + Nullbyte) nicht reicht,
-- wird NULL zurückgeliefert. */
const char* formattedTime (const char* format, const mktTime_t* tStamp);

/**
 * @return the UTC date and time in the format "2009-02-13T23:24:58.123456789"
 *
 * @date	the date/time you want, NULL returns the current date/time
 **/
char *mktTimeStr(const mktTime_t *date);

/**
 * @return the UTC date and time in the format "2011-05-12T14:26:58"
 *
 * @date	the date/time you want, NULL returns the current date/time
 **/
char *mktTimeStrClock       (const mktTime_t *date);
char* mktTimeStrClockDHM    (const mktTime_t* date);
char* mktTimeStrClockDMHM   (const mktTime_t* date);
char* mktTimeStrClockHMDMY  (const mktTime_t* date);

char *mktTimeStrFile(const mktTime_t *date);
/**
 * time comparision
 *
 * @return <0, if lhs < rhs
 *         =0, if lhs = rhs
 *         >0, if lhs > rhs
 **/
int mktTime_compare (const mktTime_t* lhs, const mktTime_t* rhs);

/**
 * time substraction
 *
 * @return the difference left - right time
 *
 * Note: This function is legacent, please prefer mktTimeDiff
 **/
mktTime_t mktTime_diff( mktTime_t *lhs, mktTime_t *rhs );

/**
 * time substraction
 *
 * @return the difference left - right time
 *
 **/
mktTime_t mktTimeDiff( mktTime_t lhs, mktTime_t rhs );

/**
 * convert mktTime to sec
 *
 * @return the seconds
 **/
double mktTime2sec( mktTime_t t);

/**
 * @return the given date/time in seconds omitting leading zeroes
 *
 * The format may be look like "0.123456789" 
 *
 * @date	the date/time you want, NULL returns the distance to the last call
 **/
char *mktTime_diffStr(const mktTime_t *date);



const char*      market_db_get_date                         ( gdouble tval );
const char*      market_db_get_date_format                  ( gdouble tval, const char *format );
const char*      market_db_get_date_file                    ( gdouble tval  );
const char*      market_db_get_date_string                  ( gdouble tval  );
const char*      market_db_get_date_dmy                     ( gdouble tval );
const char*      market_db_get_date_dmhm                    ( gdouble tval );
const char*      market_db_get_date_ydmhm                   ( gdouble tval );
const char*      market_db_get_date_hmydm                   ( gdouble tval );
const char*      market_db_get_date_hms                     ( gdouble tval );

double           market_db_time_now                         (  );
double           market_db_date_from_dmy                    ( int d , int m , int y );
double           market_db_data_curr_day                    ( gdouble tval );

const char*      market_db_get_date_lar_format              ( gdouble tval );
const char*      market_db_get_date_sqlite_format           ( gdouble tval );
const char*      market_db_get_date_sqlite_short_format     ( gdouble tval );
char*            market_db_get_date_sqlite_dup_short_format ( gdouble tval );

/* Soll mit Hilfe einer Sprachidentifikationszeichenkette
-- ("de_DE" o. ä.) ein kurzes, übliches Datum liefern. Dies muss an den
-- Sprachsupport angebunden werden. Z. Zt. werden deutsche Datumswerte
-- geliefert. */
const char*      mktHumanDate                               ( const char* lang, mktTime_t* t );
const char*      mktHumanTime                               ( const char* lang, mktTime_t* t );
const char*      mktHumanShortTime                          ( const char* lang, mktTime_t* t );
const char*      mktHumanDateShortTime                      ( const char* lang, mktTime_t* t );
const char*      market_db_human_date                       ( const char* lang, double t );
const char*      market_db_human_time                       ( const char* lang, double t );
const char*      market_db_human_short_time                 ( const char* lang, double t );
const char*      market_db_human_date_short_time            ( const char* lang, double t );

double           market_db_local_date_from_dmy_hms          ( int D, int M, int Y, int h, int m, int s);

/* Das Datum des Rechners wird neu gesetzt. Dabei wird 'date' als
-- 'localtime' interpretiert. 'dtString' wird auf den Datumsstring ge-
-- setzt, der an das Linuxprogramm 'date' übergeben worden ist. Das kann
-- für die Fehlersuche interessant sein. 'dtString' darf NULL sein. */
gboolean         setDate                                    ( double      date, char*  dtString);

#endif

/**
 * @file market-time.c	time
 *
 * (c) 2008 LAR Process Analysers AG - All rights reserved.
 *
 * @author C.Vogt
 **/

#define _XOPEN_SOURCE 600

#define _DEFAULT_SOURCE
#include "market-time.h"

// standard headers

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#undef __USE_BSD
#define __USE_BSD
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
//#define __USE_POSIX199309 	/* clock_gettime CLOCK_REALTIME need it */
#undef __USE_BSD
#include <glib.h>
#include <time.h>

#define HUMAN_DATE_FORMAT_GERMAN "%d.%m.%Y"
#define HUMAN_TIME_FORMAT_GERMAN "%H:%M:%S"
#define HUMAN_SHORT_TIME_FORMAT_GERMAN "%H:%M"
#define HUMAN_DATE_SHORT_TIME_FORMAT_GERMAN HUMAN_DATE_FORMAT_GERMAN " " HUMAN_SHORT_TIME_FORMAT_GERMAN

enum { BUF_LEN = 255, BUF_SIZE = BUF_LEN + 1 };

/**
 * the current market time
 **/
mktTime_t mktNow(void) {
    mktTime_t r;

    clock_gettime(CLOCK_REALTIME, &r);

    return (r);
}

int mktTime_compare(const mktTime_t *lhs, const mktTime_t *rhs) {
    if (lhs->tv_sec < rhs->tv_sec) {
        return (-1);
    }
    if (lhs->tv_sec > rhs->tv_sec) {
        return (1);
    }
    return (lhs->tv_nsec - rhs->tv_nsec);
}

static int oddPercent(const char *format, const char *percent) {
    if (format >= percent) return -1;

    for (;;) {
        if ('%' != percent[0]) return 0;

        if (format == percent) return 1;

        percent--;

        if ('%' != percent[0]) return 1;

        if (format == percent) return 0;

        percent--;
    }
}

/* Liefert eine Zeichenkette, die den formatierten Zeitstempel enthält.
-- Zu den bereits bekannten Formatbeschreibern von 'strftime' wird '%N'
-- für 9 Stellen Nanosekunden hinzugefügt.
-- Wenn der Platz von 256 Bytes (255 Zeichen + Nullbyte) nicht reicht,
-- wird NULL zurückgeliefert.
-- Es wäre eine gute Idee, die übrigen Funktionen nach und nach auf die-
-- se Funktion zurückzuführen. Im Moment traue ich mich nicht, da bei
-- Änderungen Fehler auftreten und diese Funktionen gut getestet sind. */
const char *formattedTime(const char *format, const mktTime_t *tStamp) {
    enum { SIZE = 256 };

    static char  buffer[SIZE << 2];
    static char  temp[SIZE];
    static char *nextResult = buffer;

    const char *fmtPtr    = format;
    size_t      freeSpace = sizeof buffer - (nextResult - buffer);
    const char *nano      = strstr(fmtPtr, "%N");
    char *      resPtr    = nextResult;
    char *      result    = nextResult;
    struct tm * timeinfo  = localtime(&tStamp->tv_sec);
    size_t      written;

    if (freeSpace < 256) {
        nextResult = buffer;
        freeSpace  = SIZE;
        resPtr     = nextResult;
        result     = nextResult;
    }

    result[0] = 0;

    while (nano) {
        if (freeSpace - 1 < nano - format) return NULL;

        if (oddPercent(fmtPtr, nano)) {
            strncpy(temp, fmtPtr, nano - fmtPtr);
            temp[nano - fmtPtr] = 0;

            written = strftime(resPtr, freeSpace, temp, timeinfo);

            if (!written) return NULL;

            fmtPtr = nano + 2;
            freeSpace -= written;
            resPtr += written;

            if (freeSpace < 10) return NULL;

            written = sprintf(resPtr, "%09lu", tStamp->tv_nsec);

            freeSpace -= written;
            resPtr += written;
        }

        nano = strstr(nano + 2, "%N");
    }

    if (fmtPtr[0]) {
        written = strftime(resPtr, freeSpace, fmtPtr, timeinfo);

        if (!written) return NULL;

        freeSpace -= written;
        resPtr += written;
    }

    nextResult = resPtr + 1;

    return result;
}

/**
 * @return the UTC date and time in the format "2009-02-13T23:24:58.123456789"
 *
 * @date	the date/time you want, NULL returns the current date/time
 **/
char *mktTimeStr(const mktTime_t *date) {
    mktTime_t   now;
    struct tm * timeinfo;
    static char buf[31];

    if (NULL == date) {
        now = mktNow();
    } else {
        now = *date;
    }
    timeinfo = gmtime(&now.tv_sec);
    strftime(buf, 30, "%Y-%m-%dT%H:%M:%S", timeinfo);
    sprintf(buf + 19, ".%.9lu", now.tv_nsec); // FIX: was %.9u

    return (buf);
}
#define TimeStrClockSize 64
/**
 * @return the UTC date and time in the format "2011-05-12T14:26:58"
 *
 * @date	the date/time you want, NULL returns the current date/time
 **/
char *mktTimeStrClock(const mktTime_t *date) {
    static char buf[TimeStrClockSize];

    mktTime_t  now;
    struct tm *timeinfo;

    if (!date)
        now = mktNow();
    else
        now = *date;

    timeinfo = gmtime(&now.tv_sec);

    strftime(buf, TimeStrClockLen, "%Y-%m-%dT%H:%M:%S", timeinfo);

    return buf;
}

char *mktTimeStrClockDHM(const mktTime_t *date) {
    static char buf[TimeStrClockSize];

    mktTime_t  now;
    struct tm *timeinfo;

    if (!date)
        now = mktNow();
    else
        now = *date;

    timeinfo = gmtime(&now.tv_sec);

    strftime(buf, TimeStrClockLen, "%d %H:%M", timeinfo);

    return buf;
}

char *mktTimeStrClockDMHM(const mktTime_t *date) {
    static char buf[TimeStrClockSize];

    mktTime_t  now;
    struct tm *timeinfo;

    if (!date)
        now = mktNow();
    else
        now = *date;

    timeinfo = gmtime(&now.tv_sec);

    strftime(buf, TimeStrClockLen, "%d %b %H:%M", timeinfo);

    return buf;
}

char *mktTimeStrClockHMDMY(const mktTime_t *date) {
    static char buf[TimeStrClockSize];

    mktTime_t  now;
    struct tm *timeinfo;

    if (!date)
        now = mktNow();
    else
        now = *date;

    timeinfo = gmtime(&now.tv_sec);

    strftime(buf, TimeStrClockLen, "%H:%M %d.%m.%y", timeinfo);

    return buf;
}

char *mktTimeStrFile(const mktTime_t *date) {
    mktTime_t   now;
    struct tm * timeinfo;
    static char buf[31];

    if (NULL == date) {
        now = mktNow();
    } else {
        now = *date;
    }
    timeinfo = gmtime(&now.tv_sec);
    strftime(buf, 30, "%Y-%m-%dT%H-%M-%S", timeinfo);
    //	sprintf(buf+19,".%.9lu",now.tv_nsec);	// FIX: was %.9u

    return (buf);
}

/**
 * time substraction
 *
 * @return the difference left - right time
 *
 * Note: This function is legacent, please prefer mktTimeDiff
 **/
mktTime_t mktTime_diff(mktTime_t *lhs, mktTime_t *rhs) {
    mktTime_t ret;
    ret.tv_sec  = lhs->tv_sec - rhs->tv_sec;
    ret.tv_nsec = lhs->tv_nsec - rhs->tv_nsec;
    if (0 > ret.tv_nsec) {
        ret.tv_sec -= 1;
        ret.tv_nsec += 1000000000;
    }
    return (ret);
}

/**
 * time substraction
 *
 * @return the difference left - right time
 *
 **/
mktTime_t mktTimeDiff(mktTime_t lhs, mktTime_t rhs) {
    mktTime_t ret;
    ret.tv_sec  = lhs.tv_sec - rhs.tv_sec;
    ret.tv_nsec = lhs.tv_nsec - rhs.tv_nsec;
    if (0 > ret.tv_nsec) {
        ret.tv_sec -= 1;
        ret.tv_nsec += 1000000000;
    }
    return (ret);
}

/**
 * convert mktTime to sec
 *
 * @return the seconds
 **/
double mktTime2sec(mktTime_t t) {
    return (t.tv_nsec / 1000000000. + t.tv_sec);
}

static mktTime_t market_db_time_to_mktTime(double t) {
    mktTime_t result;

    result.tv_sec  = (long)t;
    result.tv_nsec = (long)((t - (double)result.tv_sec) * 1E9);

    return result;
}

/**
 * @return the given time in seconds omitting leading zeroes
 *
 * The format may be look like "0.123456789"
 *
 * @date	the date/time you want, NULL returns the distance to the last
 *call
 **/
char *mktTime_diffStr(const mktTime_t *date) {
    static mktTime_t last = {0, 0};
    mktTime_t        now, diff;
    struct tm *      timeinfo;
    static char      buf[31];
    int              n;

    if (NULL == date) {
        now  = mktNow();
        diff = mktTimeDiff(now, last);
        last = now;
    } else {
        diff = *date;
    }
    timeinfo = localtime(&diff.tv_sec);
    n        = strftime(buf, 30, "%s", timeinfo);
    sprintf(buf + n, ".%.9lu", diff.tv_nsec);

    return (buf);
}

double market_db_time_now() {
    return mktTime2sec(mktNow());
}

/* m (month) [ 0 - 11 ]
 * d (day)   [ 1 - 31 ]
 * y (year)  [ 1971 - 2037]
 */

double market_db_date_from_dmy(int d, int m, int y) {
    struct tm timeinfo;
    double    ret;
    // mkt_debug("d=%d;m=%d;y=%d",d,m,y);
    timeinfo.tm_sec  = 1;
    timeinfo.tm_min  = 0;
    timeinfo.tm_hour = 0;
    timeinfo.tm_mon  = m;
    timeinfo.tm_mday = d;
    timeinfo.tm_year = (y - 1900);
    ret              = timegm(&timeinfo);
    return ret;
}

double market_db_data_curr_day(double tval) {
    struct tm *timeinfo;
    mktTime_t  time = mktNow();
    time.tv_sec     = (long int)tval;
    timeinfo        = localtime(&time.tv_sec);
    // printf("TEST DATE(%d : %d :
    // %d)\n",timeinfo->tm_mday,timeinfo->tm_mon,timeinfo->tm_year);
    return market_db_date_from_dmy(timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year + 1900);
}

const char *market_db_get_date(double tval) {
    mktTime_t time = market_db_time_to_mktTime(tval);
    return mktTimeStr(&time);
}

const char *market_db_get_date_format(double tval, const char *format) {
    mktTime_t time = mktNow();
    time.tv_sec    = (long int)tval;
    struct tm * timeinfo;
    static char buf[60];
    timeinfo = localtime(&time.tv_sec);
    strftime(buf, sizeof(buf), format, timeinfo);
    return (buf);
}

const char *market_db_get_date_file(double tval) {
    mktTime_t time = mktNow();
    time.tv_sec    = (long int)tval;
    struct tm * timeinfo;
    static char buf[20];
    timeinfo = localtime(&time.tv_sec);
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H-%M-%S", timeinfo);
    return (buf);
}

const char *market_db_get_date_dmy(double tval) {
    mktTime_t time = mktNow();
    time.tv_sec    = (long int)tval;
    struct tm * timeinfo;
    static char buf[20];
    timeinfo = localtime(&time.tv_sec);
    strftime(buf, sizeof(buf), "%Y-%m-%d", timeinfo);
    return (buf);
}

const char *market_db_get_date_dmhm(double tval) {
    mktTime_t time = mktNow();
    time.tv_sec    = (long int)tval;
    struct tm * timeinfo;
    static char buf[20];
    timeinfo = localtime(&time.tv_sec);
    strftime(buf, sizeof(buf), "%d.%m %H:%M", timeinfo);
    return (buf);
}

const char *market_db_get_date_ydmhm(double tval) {
    mktTime_t time = mktNow();
    time.tv_sec    = (long int)tval;
    struct tm * timeinfo;
    static char buf[20];
    timeinfo = localtime(&time.tv_sec);
    strftime(buf, sizeof(buf), "%Y.%d.%m %H:%M", timeinfo);
    return (buf);
}

const char *market_db_get_date_hmydm(double tval) {
    mktTime_t time = mktNow();
    time.tv_sec    = (long int)tval;
    struct tm * timeinfo;
    static char buf[20];
    timeinfo = localtime(&time.tv_sec);
    strftime(buf, sizeof(buf), "%Y.%m.%d %H:%M", timeinfo);
    return (buf);
}
const char *market_db_get_date_string(double tval) {
    mktTime_t time = mktNow();
    time.tv_sec    = (long int)tval;
    struct tm * timeinfo;
    static char buf[20];
    timeinfo = localtime(&time.tv_sec);
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", timeinfo);
    return (buf);
}

const char *market_db_get_date_hms(double tval) {
    mktTime_t time = mktNow();
    time.tv_sec    = (long int)tval;
    struct tm * timeinfo;
    static char buf[20];
    timeinfo = localtime(&time.tv_sec);
    strftime(buf, sizeof(buf), "%H:%M:%S", timeinfo);
    return (buf);
}

const char *market_db_get_date_lar_format(double tval) {
    mktTime_t time = mktNow();
    time.tv_sec    = (long int)tval;
    struct tm * timeinfo;
    static char buf[20];
    timeinfo = localtime(&time.tv_sec);
    strftime(buf, sizeof(buf), "%d.%m.%Y-%H:%M:%S", timeinfo);
    return (buf);
}

const char *market_db_get_date_sqlite_format(gdouble tval) {
    mktTime_t time = mktNow();
    time.tv_sec    = (long int)tval;
    struct tm * timeinfo;
    static char buf[40];
    timeinfo = localtime(&time.tv_sec);
    GDoubleIEEE754 vt;
    vt.v_double = tval;
    strftime(buf, sizeof(buf), "%Y.%m.%dT%H:%M:%S", timeinfo);
    sprintf(buf + 19, ":%.9u", vt.mpn.mantissa_low);
    return (buf);
}

const char *market_db_get_date_sqlite_short_format(gdouble tval) {
    mktTime_t time = mktNow();
    time.tv_sec    = (long int)tval;
    struct tm * timeinfo;
    static char buf[40];
    timeinfo = localtime(&time.tv_sec);
    strftime(buf, sizeof(buf), "%Y.%m.%dT%H:%M", timeinfo);
    return (buf);
}

char *market_db_get_date_sqlite_dup_short_format(gdouble tval) {
    mktTime_t time = mktNow();
    time.tv_sec    = (long int)tval;
    struct tm *timeinfo;
    char       buf[40];
    timeinfo = localtime(&time.tv_sec);
    strftime(buf, sizeof(buf), "%Y.%m.%dT%H:%M", timeinfo);
    return g_strdup(buf);
}

/* Die hier folgenden Zeitformatfunktionen treten immer Paarweise auf,
-- d. h. einmal im CamelCase-Format mit dem zweiten Argument vom Typ
-- 'mktTime_t*' und einmal im '_'-Format mit dem zweiten Argument vom
-- Typ 'double'. Da diese Funktionen strukturell alle gleich sind und
-- sich nur in den Namen und im Format unterscheiden, bietet sich eine
-- vereinheitlichung mit Makros an.
*/

#define FormattetTime(ccaseName, uscoreName, format)        \
    const char *ccaseName(const char *lang, mktTime_t *t) { \
        return formattedTime(format, t);                    \
    }                                                       \
                                                            \
    const char *uscoreName(const char *lang, double t) {    \
        mktTime_t time = market_db_time_to_mktTime(t);      \
        return ccaseName(lang, &time);                      \
    }

FormattetTime(mktHumanDate, market_db_human_date, HUMAN_DATE_FORMAT_GERMAN) FormattetTime(mktHumanTime, market_db_human_time, HUMAN_TIME_FORMAT_GERMAN)
    FormattetTime(mktHumanShortTime, market_db_human_short_time, HUMAN_SHORT_TIME_FORMAT_GERMAN)
        FormattetTime(mktHumanDateShortTime, market_db_human_date_short_time, HUMAN_DATE_SHORT_TIME_FORMAT_GERMAN)

            double market_db_local_date_from_dmy_hms(int D, int M, int Y, int h, int m, int s) {
    struct tm timeinfo;
    time_t    time;

    timeinfo.tm_sec  = s;
    timeinfo.tm_min  = m;
    timeinfo.tm_hour = h;
    timeinfo.tm_mon  = M - 1;
    timeinfo.tm_mday = D;
    timeinfo.tm_year = Y - 1900;

    time = mktime(&timeinfo);

    return time;
}

/*static void dupWithoutWarning (int fd)
{
        if (dup (fd))
                return;
}
#define dup dupWithoutWarning

static void child (int fd_chl)
{
        int            rc;
        struct termios term_settings;

        rc = tcgetattr (fd_chl, &term_settings);

        if (rc)
                exit (1);

        cfmakeraw (&term_settings);
        tcsetattr (fd_chl, TCSANOW, &term_settings);

        // Standardeingabe, -ausgabe und -fehler werden auf das Pseudoterminal
gesetzt.
        close (0); // Schließe Standardeingabe (aktuelles Terminal)
        close (1); // Schließe Standardausgabe (aktuelles Terminal)
        close (2); // Schließe Standardfehler (aktuelles Terminal)

        dup (fd_chl); // Pseudoterminal wird Standardeingabe (0)
        dup (fd_chl); // Pseudoterminal wird Standardausgabe (1)
        dup (fd_chl); // Pseudoterminal wird Standardfehler (2)

        // Jetzt ist der Originaldeskriptor nutzlos.
        close (fd_chl);

        // Mache den Prozess zum neuen Sitzungsleiter.
        setsid();

        // Als Sitzungsleiter wird das Kontrollterminal passive Seite des
        // Pseudoterminals. (Erforderlich für Programme wie die Shell um
        // ihre Ausgaben richtig zu handhaben.)
        ioctl  (0, TIOCSCTTY, 1);
        execlp ("/bin/su", "/bin/su", (char*)NULL);
}

/ * 'readFromRoot' liefert <0 bei Fehler, 0 bei Erfolg. * /
static int readFromRoot (int fd_par, char* buffer) {
        size_t pos = 0;

        while (pos < BUF_LEN) {
                int            count;
                fd_set         fd_in;
                int            rc;
                struct timeval timeout;

                for (count=0; count<8; count++) {
                        FD_ZERO         (&fd_in);
                        FD_SET  (fd_par, &fd_in);

                        timeout.tv_sec  = 0;
                        timeout.tv_usec = 0;

                        // 'select' funktionierte bei meinen Tests nicht so, wie
an-
                        // genommen. Wartezeiten unter einer Sekunde wurden als
0
                        // interpretiert. Mithilfe von 'usleep' und dieser
Schleife
                        // konnte ich letztlich eine brauchbare Wartezeit von
64ms
                        // erzeugen.
                        rc = select (fd_par+1, &fd_in, NULL, NULL, &timeout);

                        if (rc < 0)
                                return rc;

                        if (rc)
                                break;

                        usleep (8000);
                }

                if (rc <= 0)
                        return rc;

                rc = read (fd_par, &buffer[pos], BUF_LEN-pos);

                if (rc < 0)
                        return 0;

                pos          += rc;
                buffer [pos]  = 0;
        }

        return 0;
}
#define readFromRoot(fd_par, buffer) \
        do {\
                if (readFromRoot (fd_par, buffer))\
                        return FALSE;\
        }\
        while (0)

static void writeWithoutWarning (int fd, char* buffer, size_t toWrite)
{
        if (write (fd, buffer, toWrite))
                return;
}
#define write writeWithoutWarning

int parent (double date, int fd_par, char* dtString)
{
        char      buffer [BUF_SIZE];
        struct tm lT;
        int       rc;
        time_t    t;

        t  = (time_t) date;
        lT = * localtime (&t);

        readFromRoot (fd_par,   buffer);           // Lesen der Frage nach dem
Passwort.
        write        (fd_par,   "14lar99!\n", 9);  // Senden des Passwortes.
        readFromRoot (fd_par,   buffer);           // Lesen einer Returntaste
(Echo der Passworteingabe).
        sprintf      (dtString, "%d-%d-%d %d:%d:%d", lT.tm_year+1900,
lT.tm_mon+1, lT.tm_mday, lT.tm_hour, lT.tm_min, lT.tm_sec);

        rc = sprintf (buffer, "date -s '@%ld' && echo gelungen || echo
mißlungen\n", (long)date);

        write        (fd_par, buffer, rc);        // Senden des Datumskommandos.
        readFromRoot (fd_par, buffer);            // Lesen der Antwort

        rc = !! strstr (buffer, "\ngelungen\n");  // Ermitteln des Erfolges

        if (rc) {
                rc = sprintf (buffer, "hwclock --systohc && echo gelungen ||
echo mißlungen\n");

                write        (fd_par, buffer, rc);        // Senden des
Speicherkommandos.
                readFromRoot (fd_par, buffer);            // Lesen der Antwort

                rc = !! strstr (buffer, "\ngelungen\n");  // Ermitteln des
Erfolges
        }

        write        (fd_par, "exit\n", 5);  // Schließen der root-shell.
        readFromRoot (fd_par, buffer);       // Lesen der Antwort. (Das Echo,
also exit.)

        return rc;
}

gboolean _setDate (double date, char* dtString)
{
        char  dt [] = "YYYY-mm-dd HH:MM:SS";
        int   fd_par;
        int   fd_chl;
        pid_t pid;
        int   rc;

        fd_par = posix_openpt (O_RDWR);

        if (fd_par < 0)
                return FALSE;

        rc = grantpt (fd_par);

        if (rc)
                return FALSE;

        rc = unlockpt (fd_par);

        if (rc)
                return FALSE;

        // Öffnen der Kinddatei
        fd_chl = open (ptsname(fd_par), O_RDWR);
        pid    = fork ();

        switch (pid) {
                case -1:
                        return FALSE;

                case 0:
                        // Kindprozess: Elterdatei wird geschlossen
                        close (fd_par);
                        child (fd_chl);
                        // Die Funktion 'child' endet mit 'execlp', kehrt also
nicht zurück.
        }

        // Elterprozess: Kinddatei wird geschlossen
        close  (fd_chl);

        if (! dtString)
                dtString = dt;

        return parent (date, fd_par, dtString);
}*/

gboolean setDate(double date, char *dtString) {
    char buffer[BUF_SIZE];

    if (dtString) {
        struct tm lT;
        time_t    t;

        t  = (time_t)date;
        lT = *localtime(&t);

        sprintf(dtString, "%d-%d-%d %d:%d:%d", lT.tm_year + 1900, lT.tm_mon + 1, lT.tm_mday, lT.tm_hour, lT.tm_min, lT.tm_sec);
    }

    sprintf(buffer, "sudo date -s '@%ld'", (long)date);

    if (system(buffer)) return FALSE;

    return !system("sudo hwclock --systohc");
}

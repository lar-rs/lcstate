/*
 * mkt-value.c
 *
 *  Created on: 02.05.2013
 *      Author: sascha
 */

#include "mkt-value.h"
#include <ctype.h>
#include <fcntl.h>
#include <locale.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define l_g_value_unset(val)                     \
    G_STMT_START {                               \
        if (G_IS_VALUE(val)) g_value_unset(val); \
    }                                            \
    G_STMT_END

static GRecMutex init_rmutex;
#define MUTEX_LOCK() g_rec_mutex_lock(&init_rmutex)
#define MUTEX_UNLOCK() g_rec_mutex_unlock(&init_rmutex)

gboolean mkt_value_string_digit(const gchar *value) {
    // g_debug("%s",value);
    gchar *p    = (gchar *)value;
    gint   koma = 0;
    while ((*p != '\n' && *p != '\0')) {
        if (!g_ascii_isdigit(*p)) {
            // g_printf("|%c|",*p);
            if ((*p == '.' && koma == 0))
                koma++;
            else
                return FALSE;
        }
        p++;
    }
    // g_printf("\n");
    return TRUE;
}

GValue *mkt_value_new(GType type) {
    GValue *value;
    value = calloc(1, sizeof(GValue));
    if (type > G_TYPE_NONE)
        g_value_init(value, type);
    else
        value->g_type = type;
    return value;
}

void mkt_value_free(GValue *value) {
    if (!value) return;
    l_g_value_unset(value);
    g_free(value);
}

gboolean mkt_set_gvalue_from_string(GValue *value, const gchar *as_string) {
    gboolean retval;
    gchar *  endptr[1];
    gdouble  dvalue;
    glong    lvalue;
    gulong   ulvalue;
    GType    type;

    g_return_val_if_fail(value, FALSE);
    g_return_val_if_fail(as_string != NULL, FALSE);
    // g_debug("transform value type %s string %s",g_type_name(value->g_type),as_string);
    if (!G_IS_VALUE(value)) {
        g_warning("Can't set value for a G_TYPE_INVALID GValue");
        return FALSE;
    } else if (value->g_type <= G_TYPE_NONE) {
        g_warning("Can't set value for a NULL GValue");
        return FALSE;
    }
    // TEST:g_debug("transfor value type %s vrom str=%s",g_type_name(value->g_type),as_string);
    type = G_VALUE_TYPE(value);
    g_value_reset(value);

    /* custom transform function */
    retval = FALSE;
    if (type == G_TYPE_BOOLEAN) {
        if ((as_string[0] == 't') || (as_string[0] == 'T')) {
            g_value_set_boolean(value, TRUE);
            retval = TRUE;
        } else if ((as_string[0] == 'f') || (as_string[0] == 'F')) {
            g_value_set_boolean(value, FALSE);
            retval = TRUE;
        } else {
            gint i;
            i = atoi(as_string); /* Flawfinder: ignore */
            g_value_set_boolean(value, i ? TRUE : FALSE);
            retval = TRUE;
        }
    } else if (type == G_TYPE_INT64) {
        gint64 i64;
        if (g_str_has_prefix(as_string, "0x"))
            i64 = g_ascii_strtoll(as_string, endptr, 16);
        else
            i64 = g_ascii_strtoll(as_string, endptr, 10);
        if (*as_string != '\0' && **endptr == '\0') {
            g_value_set_int64(value, i64);
            retval = TRUE;
        }
    } else if (type == G_TYPE_UINT64) {
        guint64 ui64;
        if (g_str_has_prefix(as_string, "0x"))
            ui64 = g_ascii_strtoull(as_string, endptr, 16);
        else
            ui64 = g_ascii_strtoull(as_string, endptr, 10);
        if (*as_string != '\0' && **endptr == '\0') {
            g_value_set_uint64(value, ui64);
            retval = TRUE;
        }
    } else if (type == G_TYPE_INT) {
        if (g_str_has_prefix(as_string, "0x"))
            lvalue = strtol(as_string, endptr, 16);
        else
            lvalue = strtol(as_string, endptr, 10);
        if (*as_string != '\0' && **endptr == '\0') {
            g_value_set_int(value, (gint32)lvalue);
            retval = TRUE;
        }
    } else if (type == G_TYPE_UINT) {
        if (g_str_has_prefix(as_string, "0x"))
            ulvalue = strtoul(as_string, endptr, 16);
        else
            ulvalue = strtoul(as_string, endptr, 10);
        if (*as_string != 0 && **endptr == 0) {
            g_value_set_uint(value, (guint32)ulvalue);
            retval = TRUE;
        }
    } else if (type == G_TYPE_CHAR) {
        if (g_str_has_prefix(as_string, "0x"))
            lvalue = strtol(as_string, endptr, 16);
        else
            lvalue = strtol(as_string, endptr, 10);
        if (*as_string != 0 && **endptr == 0) {
#if GLIB_CHECK_VERSION(2, 31, 7)
            g_value_set_schar(value, (gint)lvalue);
#else
            g_value_set_char(value, (gchar)lvalue);
#endif
            retval = TRUE;
        }
    } else if (type == G_TYPE_UCHAR) {
        if (g_str_has_prefix(as_string, "0x"))
            ulvalue = strtoul(as_string, endptr, 16);
        else
            ulvalue = strtoul(as_string, endptr, 10);
        if (*as_string != 0 && **endptr == 0) {
            g_value_set_uchar(value, (guchar)ulvalue);
            retval = TRUE;
        }
    } else if (type == G_TYPE_FLOAT) {
        dvalue = g_ascii_strtod(as_string, endptr);
        if (*as_string != '\0' && **endptr == '\0') {
            g_value_set_float(value, (gfloat)dvalue);
            retval = TRUE;
        }
    } else if (type == G_TYPE_DOUBLE) {
        dvalue = g_ascii_strtod(as_string, endptr);
        if (*as_string != '\0' && **endptr == '\0') {
            g_value_set_double(value, dvalue);
            retval = TRUE;
        }
    } else if (type == G_TYPE_GTYPE) {
        GType gt;
        gt = g_type_from_name(as_string);
        if (gt != G_TYPE_INVALID) {
            g_value_set_gtype(value, gt);
            retval = TRUE;
        }
    } else if (type == G_TYPE_ULONG) {
        gulong ulvalue;
        if (g_str_has_prefix(as_string, "0x"))
            ulvalue = strtoul(as_string, endptr, 16);
        else
            ulvalue = strtoul(as_string, endptr, 10);
        if (*as_string != 0 && **endptr == 0) {
            g_value_set_ulong(value, ulvalue);
            retval = TRUE;
        }
    } else if (type == G_TYPE_LONG) {
        glong lvalue;
        if (g_str_has_prefix(as_string, "0x"))
            lvalue = strtol(as_string, endptr, 16);
        else
            lvalue = strtol(as_string, endptr, 10);
        if (*as_string != 0 && **endptr == 0) {
            g_value_set_long(value, lvalue);
            retval = TRUE;
        }
    }

    if (!retval && g_value_type_transformable(G_TYPE_STRING, type)) {
        /* use the GLib type transformation function */
        GValue *string;

        string = calloc(1, sizeof(GValue));
        g_value_init(string, G_TYPE_STRING);
        g_value_set_string(string, as_string);

        g_value_transform(string, value);
        mkt_value_free(string);

        retval = TRUE;
    }

    return retval;
}

gchar *mkt_value_stringify(const GValue *value) {
    if (!value) return g_strdup("NULL");
    if (G_VALUE_TYPE(value) == G_TYPE_BOOLEAN) return g_value_get_boolean(value) ? g_strdup("1") : g_strdup("0");

    if (g_value_type_transformable(G_VALUE_TYPE(value), G_TYPE_STRING)) {
        GValue *string;
        gchar * str;
        string            = mkt_value_new(G_TYPE_STRING);
        gchar *old_locale = NULL;
        if (G_VALUE_HOLDS_FLOAT(value) || G_VALUE_HOLDS_DOUBLE(value)) {
            old_locale = g_strdup(setlocale(LC_ALL, NULL));
            setlocale(LC_ALL, "en_US.UTF-8");
        }
        g_value_transform(value, string);
        str = g_value_dup_string(string);
        mkt_value_free(string);
        if (old_locale) {
            setlocale(LC_ALL, old_locale);
            g_free(old_locale);
        }
        return str;
    } else
        return g_strdup("");
}

const gchar *mkt_value_stringify_static(const GValue *value) {
    MUTEX_LOCK();
    static gchar *internal_value = NULL;
    if (internal_value) g_free(internal_value);
    if (!value)
        internal_value = g_strdup("NULL");
    else {
        if (G_VALUE_TYPE(value) == G_TYPE_BOOLEAN) {
            internal_value = g_value_get_boolean(value) ? g_strdup("1") : g_strdup("0");
        } else if (G_VALUE_TYPE(value) == G_TYPE_DOUBLE) {
            if (fabs(g_value_get_double(value)) < 1) {
                internal_value = g_strdup_printf("%.3f", g_value_get_double(value));
            } else if (fabs(g_value_get_double(value)) < 10) {
                internal_value = g_strdup_printf("%.2f", g_value_get_double(value));
            } else if (fabs(g_value_get_double(value)) < 100) {
                internal_value = g_strdup_printf("%.1f", g_value_get_double(value));
            } else {
                internal_value = g_strdup_printf("%.0f", g_value_get_double(value));
            }
        } else if (g_value_type_transformable(G_VALUE_TYPE(value), G_TYPE_STRING)) {
            GValue *string;
            string = mkt_value_new(G_TYPE_STRING);
            g_value_transform(value, string);
            internal_value = g_value_dup_string(string);
            mkt_value_free(string);
        } else {
            internal_value = g_strdup("");
        }
    }
    MUTEX_UNLOCK();
    return internal_value;
}

gchar *mkt_value_dup_double(gdouble value) {
    gchar *  text_value = NULL;
    gboolean is_0       = FALSE;
    ;
    if (fabs(value) < 0.001) {
        text_value = g_strdup_printf("%.4f", value);
    } else if (fabs(value) < 0.01) {
        text_value = g_strdup_printf("%.3f", value);
    } else if (fabs(value) < 2) {
        text_value = g_strdup_printf("%.3f", value);
    } else if (fabs((value)) < 10) {
        text_value = g_strdup_printf("%.2f", (value));
    } else if (fabs((value)) < 100) {
        text_value = g_strdup_printf("%.1f", (value));
    } else {
        text_value = g_strdup_printf("%.0f", (value));
        is_0       = TRUE;
    }
    if (!is_0) {
        gchar *p = NULL;
        p        = text_value;
        p += strlen(p) - 1;
        for (; ((*(p - 1) != '.') && (*(p - 1) != ',') && (*p == '0')); p--) *p = '\0';
    }
    return text_value;
}

const gchar *mkt_value_stringify_double(gdouble value) {
    MUTEX_LOCK();
    static gchar internal_value[65];
    memset(internal_value, 0, sizeof(internal_value));
    gchar *  temp_value = NULL;
    gboolean is_0       = FALSE;
    ;
    if (fabs(value) < 0.001) {
        temp_value = g_strdup_printf("%.4f", value);
    } else if (fabs(value) < 1) {
        temp_value = g_strdup_printf("%.3f", value);
    } else if (fabs((value)) < 10) {
        temp_value = g_strdup_printf("%.2f", (value));
    } else if (fabs((value)) < 100) {
        temp_value = g_strdup_printf("%.1f", (value));
        is_0       = TRUE;
    } else {
        temp_value = g_strdup_printf("%.0f", (value));
        is_0       = TRUE;
    }
    strncpy(internal_value, temp_value, 64);
    g_free(temp_value);
    if (!is_0) {
        gchar *p = NULL;
        p        = internal_value;
        p += strlen(p) - 1;
        for (; ((*(p - 1) != '.') && (*(p - 1) != ',') && (*p == '0')); p--) *p = '\0';
    }
    MUTEX_UNLOCK();
    return internal_value;
}

const gchar *mkt_value_stringify_double_enUS(gdouble value) {
    MUTEX_LOCK();
    static gchar internal_value[65];
    gchar *      old_locale = g_strdup(setlocale(LC_ALL, NULL));
    setlocale(LC_ALL, "en_US.UTF-8");
    memset(internal_value, 0, sizeof(internal_value));
    gchar *  temp_value = NULL;
    gboolean is_0       = FALSE;
    ;
    if (fabs(value) < 0.001) {
        temp_value = g_strdup_printf("%.4f", value);
    } else if (fabs(value) < 1) {
        temp_value = g_strdup_printf("%.3f", value);
    } else if (fabs((value)) < 10) {
        temp_value = g_strdup_printf("%.2f", (value));
    } else if (fabs((value)) < 100) {
        temp_value = g_strdup_printf("%.1f", (value));
        is_0       = TRUE;
    } else {
        temp_value = g_strdup_printf("%.0f", (value));
        is_0       = TRUE;
    }
    strncpy(internal_value, temp_value, 64);
    g_free(temp_value);
    if (!is_0) {
        gchar *p = NULL;
        p        = internal_value;
        p += strlen(p) - 1;
        for (; ((*(p - 1) != '.') && (*(p - 1) != ',') && (*p == '0')); p--) *p = '\0';
    }
    if (old_locale) {
        setlocale(LC_ALL, old_locale);
        g_free(old_locale);
    }
    MUTEX_UNLOCK();
    return internal_value;
}

gchar *mkt_value_dup_double_enUS(gdouble value) {
    MUTEX_LOCK();
    gchar *internal_value = NULL;
    gchar *old_locale     = g_strdup(setlocale(LC_ALL, NULL));
    setlocale(LC_ALL, "en_US.UTF-8");
    internal_value = mkt_value_dup_double(value);
    if (old_locale) {
        setlocale(LC_ALL, old_locale);
        g_free(old_locale);
    }
    MUTEX_UNLOCK();
    return internal_value;
}

// MktAtom service functionen

gboolean mkt_atom_strv_equal0(gchar **a, gchar **b) {
    gboolean ret = FALSE;
    guint    n;
    if (a == NULL && b == NULL) {
        ret = TRUE;
        goto out;
    }
    if (a == NULL || b == NULL) goto out;
    if (g_strv_length(a) != g_strv_length(b)) goto out;
    for (n = 0; a[n] != NULL; n++)
        if (g_strcmp0(a[n], b[n]) != 0) goto out;
    ret = TRUE;
out:
    return ret;
}
/*
 *
 For GLib version at 2.30

static gboolean
_g_variant_equal0 (GVariant *a, GVariant *b)
{
  gboolean ret = FALSE;
  if (a == NULL && b == NULL)
    {
      ret = TRUE;
      goto out;
    }
  if (a == NULL || b == NULL)
    goto out;
  ret = g_variant_equal (a, b);
out:
  return ret;
}
*/

gboolean mkt_value_g_value_equal(const GValue *a, const GValue *b) {
    gboolean ret = FALSE;
    // TEST:g_debug("Value is equal type a=%s  type b=%s",G_VALUE_TYPE_NAME(a),G_VALUE_TYPE_NAME(b));
    // TEST:g_debug("%s == %s",mkt_value_stringify(a),mkt_value_stringify(b));
    g_return_val_if_fail(G_VALUE_TYPE(a) == G_VALUE_TYPE(b), FALSE);
    switch (G_VALUE_TYPE(a)) {
    case G_TYPE_BOOLEAN:
        ret = (g_value_get_boolean(a) == g_value_get_boolean(b));
        break;
    case G_TYPE_CHAR:
        ret = (g_value_get_schar(a) == g_value_get_schar(b));
        break;
    case G_TYPE_UCHAR:
        ret = (g_value_get_uchar(a) == g_value_get_uchar(b));
        break;
    case G_TYPE_INT:
        ret = (g_value_get_int(a) == g_value_get_int(b));
        break;
    case G_TYPE_UINT:
        ret = (g_value_get_uint(a) == g_value_get_uint(b));
        break;
    case G_TYPE_INT64:
        ret = (g_value_get_int64(a) == g_value_get_int64(b));
        break;
    case G_TYPE_UINT64:
        ret = (g_value_get_uint64(a) == g_value_get_uint64(b));
        break;
    case G_TYPE_DOUBLE:
        ret = (g_value_get_double(a) == g_value_get_double(b));
        break;
    case G_TYPE_STRING:
        ret = (g_strcmp0(g_value_get_string(a), g_value_get_string(b)) == 0);
        break;
    /*   For GLib version at 2.30
     *   case G_TYPE_VARIANT:
            ret = _g_variant_equal0 (g_value_get_variant (a), g_value_get_variant (b));
            break;
      */
    default:
        if (G_VALUE_TYPE(a) == G_TYPE_STRV) {
            ret = mkt_atom_strv_equal0(g_value_get_boxed(a), g_value_get_boxed(b));
        } else {
            g_warning("_g_value_equal() does not handle type %s", g_type_name(G_VALUE_TYPE(a)));
            g_assert(0);
        }
        break;
    }
    return ret;
}

gint mkt_value_equal(const GValue *a, const GValue *b) {
    gint ret = 0;
    // TEST:g_debug("Value is equal type a=%s  type b=%s",G_VALUE_TYPE_NAME(a),G_VALUE_TYPE_NAME(b));
    // TEST:g_debug("%s == %s",mkt_value_stringify(a),mkt_value_stringify(b));
    g_return_val_if_fail(G_VALUE_TYPE(a) == G_VALUE_TYPE(b), FALSE);
    switch (G_VALUE_TYPE(a)) {
    case G_TYPE_BOOLEAN:
        ret = (g_value_get_boolean(a) != g_value_get_boolean(b));
        break;
    case G_TYPE_CHAR:
        if ((g_value_get_schar(a) > g_value_get_schar(b)))
            ret = 1;
        else if ((g_value_get_schar(a) < g_value_get_schar(b)))
            ret = -1;
        break;
    case G_TYPE_UCHAR:
        if ((g_value_get_uchar(a) > g_value_get_uchar(b)))
            ret = 1;
        else if ((g_value_get_uchar(a) < g_value_get_uchar(b)))
            ret = -1;
        break;
    case G_TYPE_INT:
        if ((g_value_get_int(a) > g_value_get_int(b)))
            ret = 1;
        else if ((g_value_get_int(a) < g_value_get_int(b)))
            ret = -1;
        break;
    case G_TYPE_UINT:
        if ((g_value_get_uint(a) > g_value_get_uint(b)))
            ret = 1;
        else if ((g_value_get_uint(a) < g_value_get_uint(b)))
            ret = -1;
        break;
    case G_TYPE_INT64:
        if ((g_value_get_int64(a) > g_value_get_int64(b)))
            ret = 1;
        else if ((g_value_get_int64(a) < g_value_get_int64(b)))
            ret = -1;
        break;
    case G_TYPE_UINT64:
        if ((g_value_get_uint64(a) > g_value_get_uint64(b)))
            ret = 1;
        else if ((g_value_get_uint64(a) < g_value_get_uint64(b)))
            ret = -1;
        break;
    case G_TYPE_DOUBLE:
        if ((g_value_get_double(a) > g_value_get_double(b)))
            ret = 1;
        else if ((g_value_get_double(a) < g_value_get_double(b)))
            ret = -1;
        break;
    case G_TYPE_STRING:
        ret = (g_strcmp0(g_value_get_string(a), g_value_get_string(b)));
        break;
    /*   For GLib version at 2.30
     *   case G_TYPE_VARIANT:
    ret = _g_variant_equal0 (g_value_get_variant (a), g_value_get_variant (b));
    break;
     */
    default:
        if (G_VALUE_TYPE(a) == G_TYPE_STRV) {
            ret = mkt_atom_strv_equal0(g_value_get_boxed(a), g_value_get_boxed(b));
        } else {
            g_warning("_g_value_equal() does not handle type %s", g_type_name(G_VALUE_TYPE(a)));
            g_assert(0);
        }
        break;
    }
    return ret;
}

void mkt_list_free_full(GList *list, GDestroyNotify free_func) {
    if (list) {
        g_list_foreach(list, (GFunc)free_func, NULL);
        g_list_free(list);
    }
}

void mkt_slist_free_full(GSList *list, GDestroyNotify free_func) {
    if (list) {
        g_slist_foreach(list, (GFunc)free_func, NULL);
        g_slist_free(list);
    }
}

/*
 * Register the MktTime type in the GType system
 */

static void time_to_string(const GValue *src, GValue *dest) {
    MktTime *gdatime;
    GString *string;

    g_return_if_fail(G_VALUE_HOLDS_STRING(dest) && MKT_VALUE_HOLDS_TIME(src));

    gdatime = (MktTime *)mkt_value_get_time((GValue *)src);

    string = g_string_new("");
    g_string_append_printf(string, "%02u:%02u:%02u", gdatime->hour, gdatime->minute, gdatime->second);
    if (gdatime->fraction != 0) g_string_append_printf(string, ".%lu", gdatime->fraction);

    if (gdatime->timezone != MKT_TIMEZONE_INVALID) g_string_append_printf(string, "%+02d", (int)gdatime->timezone / 3600);

    g_value_take_string(dest, string->str);
    g_string_free(string, FALSE);
}

/* Transform a String GValue to a MktTime from a string like "12:30:15+01" */
static void string_to_time(const GValue *src, GValue *dest) {
    /* FIXME: add more checks*/
    MktTime *    timegda;
    const gchar *as_string;
    const gchar *ptr;

    g_return_if_fail(G_VALUE_HOLDS_STRING(src) && MKT_VALUE_HOLDS_TIME(dest));

    as_string = g_value_get_string(src);
    if (!as_string) return;
    timegda = calloc(1, sizeof(MktTime));

    /* hour */
    timegda->timezone = MKT_TIMEZONE_INVALID;
    ptr               = as_string;
    if ((*ptr >= '0') && (*ptr <= '9') && (*(ptr + 1) >= '0') && (*(ptr + 1) <= '9'))
        timegda->hour = (*ptr - '0') * 10 + *(ptr + 1) - '0';
    else {
        g_free(timegda);
        return;
    }

    /* minute */
    ptr += 2;
    if (!*ptr) {
        g_free(timegda);
        return;
    }
    if (*ptr == ':') ptr++;
    if ((*ptr >= '0') && (*ptr <= '9') && (*(ptr + 1) >= '0') && (*(ptr + 1) <= '9'))
        timegda->minute = (*ptr - '0') * 10 + *(ptr + 1) - '0';
    else {
        g_free(timegda);
        return;
    }

    /* second */
    ptr += 2;
    timegda->second = 0;
    if (!*ptr) {
        if ((timegda->hour <= 24) && (timegda->minute <= 60)) mkt_value_set_time(dest, timegda);
        g_free(timegda);
        return;
    }
    if (*ptr == ':') ptr++;
    if ((*ptr >= '0') && (*ptr <= '9') && (*(ptr + 1) >= '0') && (*(ptr + 1) <= '9')) timegda->second = (*ptr - '0') * 10 + *(ptr + 1) - '0';

    /* extra */
    ptr += 2;
    if (!*ptr) {
        if ((timegda->hour <= 24) && (timegda->minute <= 60) && (timegda->second <= 60)) mkt_value_set_time(dest, timegda);
        g_free(timegda);
        return;
    }

    if (*ptr == '.') {
        gulong fraction = 0;

        ptr++;
        while (*ptr && (*ptr >= '0') && (*ptr <= '9')) {
            fraction = fraction * 10 + *ptr - '0';
            ptr++;
        }
    }

    if ((*ptr == '+') || (*ptr == '-')) {
        glong sign        = (*ptr == '+') ? 1 : -1;
        timegda->timezone = 0;
        while (*ptr && (*ptr >= '0') && (*ptr <= '9')) {
            timegda->timezone = timegda->timezone * 10 + sign * ((*ptr) - '0');
            ptr++;
        }
        timegda->timezone *= 3600;
    }

    /* checks */
    if ((timegda->hour <= 24) || (timegda->minute <= 60) || (timegda->second <= 60)) mkt_value_set_time(dest, timegda);
    g_free(timegda);
}

GType mkt_time_get_type(void) {
    static GType type = 0;

    if (G_UNLIKELY(type == 0)) {
        type = g_boxed_type_register_static("MktTime", (GBoxedCopyFunc)mkt_time_copy, (GBoxedFreeFunc)mkt_time_free);

        g_value_register_transform_func(G_TYPE_STRING, type, string_to_time);
        g_value_register_transform_func(type, G_TYPE_STRING, time_to_string);
    }

    return type;
}

/**
 * mkt_time_copy:
 *
 * Returns: (transfer full):
 */
gpointer mkt_time_copy(gpointer boxed) {

    MktTime *src  = (MktTime *)boxed;
    MktTime *copy = NULL;

    g_return_val_if_fail(src, NULL);

    copy           = calloc(1, sizeof(MktTime));
    copy->hour     = src->hour;
    copy->minute   = src->minute;
    copy->second   = src->second;
    copy->fraction = src->fraction;
    copy->timezone = src->timezone;

    return copy;
}

void mkt_time_free(gpointer boxed) { g_free(boxed); }

/**
 * mkt_time_valid:
 * @time: a #MktTime value to check if it is valid
 *
 * Returns: #TRUE if #MktTime is valid; %FALSE otherwise.
 *
 * Since: 4.2
 */
gboolean mkt_time_valid(const MktTime *time) {
    g_return_val_if_fail(time, FALSE);

    if (time->hour > 24 || time->minute > 59 || time->second > 59)
        return FALSE;
    else
        return TRUE;
}

/*
 * Register the MktTimestamp type in the GType system
 */
/* Transform a String GValue to a MktTimestamp from a string like "2003-12-13 13:12:01.12+01" */
static void string_to_timestamp(const GValue *src, GValue *dest) {
    /* FIXME: add more checks*/
    MktTimestamp timestamp;

    g_return_if_fail(G_VALUE_HOLDS_STRING(src) && MKT_VALUE_HOLDS_TIMESTAMP(dest));

    if (!mkt_parse_iso8601_timestamp(&timestamp, g_value_get_string(src))) g_warning("Can't convert '%s' to a timestamp", g_value_get_string(src));
    mkt_value_set_timestamp(dest, &timestamp);
}

static void timestamp_to_string(const GValue *src, GValue *dest) {
    MktTimestamp *timestamp;
    GString *     string;

    g_return_if_fail(G_VALUE_HOLDS_STRING(dest) && MKT_VALUE_HOLDS_TIMESTAMP(src));

    timestamp = (MktTimestamp *)mkt_value_get_timestamp((GValue *)src);

    string = g_string_new("");
    g_string_append_printf(string, "%04u-%02u-%02u %02u:%02u:%02u", timestamp->year, timestamp->month, timestamp->day, timestamp->hour, timestamp->minute, timestamp->second);
    if (timestamp->fraction > 0) g_string_append_printf(string, ".%lu", timestamp->fraction);
    if (timestamp->timezone != MKT_TIMEZONE_INVALID) g_string_append_printf(string, "%+02d", (int)timestamp->timezone / 3600);
    g_value_take_string(dest, string->str);
    g_string_free(string, FALSE);
}

GType mkt_timestamp_get_type(void) {
    static GType type = 0;

    if (G_UNLIKELY(type == 0)) {
        type = g_boxed_type_register_static("MktTimestamp", (GBoxedCopyFunc)mkt_timestamp_copy, (GBoxedFreeFunc)mkt_timestamp_free);

        g_value_register_transform_func(G_TYPE_STRING, type, string_to_timestamp);
        g_value_register_transform_func(type, G_TYPE_STRING, timestamp_to_string);
    }

    return type;
}

/**
 * mkt_timestamp_copy:
 *
 * Returns: (transfer full):
 */
gpointer mkt_timestamp_copy(gpointer boxed) {
    MktTimestamp *src = (MktTimestamp *)boxed;
    MktTimestamp *copy;

    g_return_val_if_fail(src, NULL);

    copy           = calloc(1, sizeof(MktTimestamp));
    copy->year     = src->year;
    copy->month    = src->month;
    copy->day      = src->day;
    copy->hour     = src->hour;
    copy->minute   = src->minute;
    copy->second   = src->second;
    copy->fraction = src->fraction;
    copy->timezone = src->timezone;

    return copy;
}

void mkt_timestamp_free(gpointer boxed) { g_free(boxed); }

/**
 * mkt_timestamp_valid:
 * @timestamp: a #MktTimestamp value to check if it is valid
 *
 * Returns: #TRUE if #MktTimestamp is valid; %FALSE otherwise.
 *
 * Since: 4.2
 */
gboolean mkt_timestamp_valid(const MktTimestamp *timestamp) {
    g_return_val_if_fail(timestamp, FALSE);

    GDate *gdate;

    /* check the date part */
    gdate = g_date_new_dmy((GDateDay)timestamp->day, (GDateMonth)timestamp->month, (GDateYear)timestamp->year);
    if (!gdate) return FALSE;
    if (!g_date_valid(gdate)) {
        g_date_free(gdate);
        return FALSE;
    }
    g_date_free(gdate);

    /* check the time part */
    if (timestamp->hour > 24 || timestamp->minute > 59 || timestamp->second > 59)
        return FALSE;
    else
        return TRUE;
}

/**
 * mkt_value_get_time:
 * @value: a #GValue whose value we want to get.
 *
 * Returns: (transfer none): the value stored in @value.
 */
const MktTime *mkt_value_get_time(const GValue *value) {
    g_return_val_if_fail(value, NULL);
    g_return_val_if_fail(mkt_value_isa(value, MKT_TYPE_TIME), NULL);
    return (const MktTime *)g_value_get_boxed(value);
}

/**
 * mkt_value_set_time:
 * @value: a #GValue that will store @val.
 * @val: value to be stored in @value.
 *
 * Stores @val into @value.
 */
void mkt_value_set_time(GValue *value, const MktTime *val) {
    g_return_if_fail(value);
    g_return_if_fail(val);

    l_g_value_unset(value);
    g_value_init(value, MKT_TYPE_TIME);
    g_value_set_boxed(value, val);
}

/**
 * mkt_value_get_timestamp:
 * @value: a #GValue whose value we want to get.
 *
 * Returns: (transfer none): the value stored in @value.
 */
const MktTimestamp *mkt_value_get_timestamp(const GValue *value) {
    g_return_val_if_fail(value, NULL);
    g_return_val_if_fail(mkt_value_isa(value, MKT_TYPE_TIMESTAMP), NULL);
    return (const MktTimestamp *)g_value_get_boxed(value);
}

/**
 * mkt_value_set_timestamp:
 * @value: a #GValue that will store @val.
 * @val: value to be stored in @value.
 *
 * Stores @val into @value.
 */
void mkt_value_set_timestamp(GValue *value, const MktTimestamp *val) {
    g_return_if_fail(value);
    g_return_if_fail(val);

    l_g_value_unset(value);
    g_value_init(value, MKT_TYPE_TIMESTAMP);
    g_value_set_boxed(value, val);
}

static gboolean _parse_iso8601_date(GDate *gdate, const gchar *value, char **out_endptr) {
    GDateYear         year;
    GDateMonth        month;
    GDateDay          day;
    unsigned long int tmp;
    char *            endptr;

    g_date_clear(gdate, 1);

    if (!isdigit(*value)) return FALSE;
    tmp = strtoul(value, &endptr, 10);
    if (tmp <= G_MAXUINT16)
        year = tmp;
    else
        return FALSE;
    if (*endptr != '-') return FALSE;

    value = endptr + 1;
    if (!isdigit(*value)) return FALSE;
    tmp   = strtoul(value, &endptr, 10);
    month = tmp > 0 ? (tmp <= G_DATE_DECEMBER ? tmp : G_DATE_BAD_MONTH) : G_DATE_BAD_MONTH;
    if (month == G_DATE_BAD_MONTH) return FALSE;
    if (*endptr != '-') return FALSE;

    value = endptr + 1;
    if (!isdigit(*value)) return FALSE;
    tmp = strtoul(value, &endptr, 10);
    day = tmp > 0 ? (tmp <= G_MAXUINT8 ? tmp : G_DATE_BAD_DAY) : G_DATE_BAD_DAY;
    if (day == G_DATE_BAD_DAY) return FALSE;

    if (g_date_valid_dmy(day, month, year)) {
        g_date_set_dmy(gdate, day, month, year);
        *out_endptr = endptr;
        return TRUE;
    } else {
        memset(gdate, 0, sizeof(GDate));
        return FALSE;
    }
}

/**
 * mkt_parse_iso8601_date:
 * @gdate: a pointer to a #GDate structure which will be filled
 * @value: a string
 *
 * Extracts date parts from @value, and sets @gdate's contents
 *
 * Accepted date format is "YYYY-MM-DD" (more or less than 4 digits for years and
 * less than 2 digits for month and day are accepted). Years must be in the 1-65535 range,
 * a limitation imposed by #GDate.
 *
 * Returns: %TRUE if no error occurred
 */
gboolean mkt_parse_iso8601_date(GDate *gdate, const gchar *value) {
    g_return_val_if_fail(gdate, FALSE);

    char *endptr;
    if (!value) return FALSE;

    if (!_parse_iso8601_date(gdate, value, &endptr)) return FALSE;
    if (*endptr) return FALSE;
    return TRUE;
}

static gboolean _parse_iso8601_time(MktTime *timegda, const gchar *value, char **out_endptr) {
    unsigned long int tmp;
    char *            endptr;

    memset(timegda, 0, sizeof(MktTime));
    timegda->timezone = MKT_TIMEZONE_INVALID;

    if (!isdigit(*value)) return FALSE;
    tmp = strtoul(value, &endptr, 10);
    if (tmp <= 23)
        timegda->hour = tmp;
    else
        return FALSE;
    if (*endptr != ':') return FALSE;

    value = endptr + 1;
    if (!isdigit(*value)) return FALSE;
    tmp = strtoul(value, &endptr, 10);
    if (tmp < 60)
        timegda->minute = tmp;
    else
        return FALSE;
    if (*endptr != ':') return FALSE;

    value = endptr + 1;
    if (!isdigit(*value)) return FALSE;
    tmp = strtoul(value, &endptr, 10);
    if (tmp < 60)
        timegda->second = tmp;
    else
        return FALSE;

    if (*endptr && (*endptr != '.') && (*endptr != '+') && (*endptr != '-')) {
        *out_endptr = endptr;
        return TRUE; /* end of the parsing */
    }

    if (*endptr == '.') {
        value = endptr + 1;
        if (!isdigit(*value)) return FALSE;
        tmp = strtoul(value, &endptr, 10);
        if (tmp < G_MAXULONG)
            timegda->fraction = tmp;
        else
            return FALSE;
    }
    if ((*endptr == '+') || (*endptr == '-')) {
        long int stmp;
        value = endptr;
        stmp  = strtol(value, &endptr, 10);
        if ((stmp >= -24) && (stmp <= 24))
            timegda->timezone = stmp * 60 * 60;
        else
            return FALSE;
    }

    *out_endptr = endptr;
    return TRUE;
}

/**
 * mkt_parse_iso8601_time:
 * @timegda: a pointer to a #MktTime structure which will be filled
 * @value: a string
 *
 * Extracts time parts from @value, and sets @timegda's contents
 *
 * Accepted date format is "HH:MM:SS[.ms][TZ]" where TZ is +hour or -hour
 *
 * Returns: TRUE if no error occurred
 */
gboolean mkt_parse_iso8601_time(MktTime *timegda, const gchar *value) {
    g_return_val_if_fail(timegda, FALSE);

    char *endptr;
    if (!value) return FALSE;

    if (!_parse_iso8601_time(timegda, value, &endptr)) return FALSE;
    if (*endptr) return FALSE;
    return TRUE;
}

/**
 * mkt_parse_iso8601_timestamp:
 * @timestamp: a pointer to a #MktTimeStamp structure which will be filled
 * @value: a string
 *
 * Extracts date and time parts from @value, and sets @timestamp's contents
 *
 * Accepted date format is "YYYY-MM-DD HH:MM:SS[.ms][TZ]" where TZ is +hour or -hour
 *
 * Returns: TRUE if no error occurred
 */
gboolean mkt_parse_iso8601_timestamp(MktTimestamp *timestamp, const gchar *value) {
    g_return_val_if_fail(timestamp, FALSE);

    gboolean retval = TRUE;
    char *   endptr;
    GDate    gdate;
    MktTime  timegda;

    memset(timestamp, 0, sizeof(MktTimestamp));
    memset(&timegda, 0, sizeof(MktTime));
    timegda.timezone = MKT_TIMEZONE_INVALID;

    if (!value) return FALSE;

    /* date part */
    if (!_parse_iso8601_date(&gdate, value, &endptr)) {
        retval = FALSE;
        goto out;
    }
    timestamp->year  = g_date_get_year(&gdate);
    timestamp->month = g_date_get_month(&gdate);
    timestamp->day   = g_date_get_day(&gdate);

    /* separator */
    if (!*endptr) goto out;

    if (*endptr != ' ') {
        retval = FALSE;
        goto out;
    }
    value = endptr + 1;
    if (!*value) goto out;

    /* time part */
    if (!_parse_iso8601_time(&timegda, value, &endptr) || *endptr) retval = FALSE;
out:
    timestamp->hour     = timegda.hour;
    timestamp->minute   = timegda.minute;
    timestamp->second   = timegda.second;
    timestamp->fraction = timegda.fraction;
    timestamp->timezone = timegda.timezone;

    return retval;
}

/**
 *
 *
 *
 *
 *
 *
 */
MktParameter *mkt_parameter_new(GObject *instance, const gchar *name, GType type, guint flag) {
    MktParameter *par = g_malloc0(sizeof(MktParameter));
    if (name)
        par->name = g_strdup(name);
    else
        par->name = NULL;
    par->value    = mkt_value_new(type);
    par->flag     = flag;
    par->instance = instance;
    par->old      = NULL;
    par->select   = NULL;
    if (instance) {
        GParamSpec *pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(instance), name);
        if (pspec) {
            par->old = mkt_value_new(type);
            g_object_get_property(instance, name, par->old);
        }
    }
    return par;
}

/**
 *
 *
 *
 *
 *
 *
 */
void mkt_parameter_free(MktParameter *parameter) {
    if (parameter) {
        if (parameter->name) g_free(parameter->name);
        if (parameter->select) g_free(parameter->select);
        if (parameter->value) mkt_value_free(parameter->value);
        if (parameter->old) mkt_value_free(parameter->old);
        g_free(parameter);
    }
}

void mkt_parameter_set_select(MktParameter *parameter, const gchar *select) {
    g_return_if_fail(parameter != NULL);
    g_return_if_fail(select != NULL);
    if (parameter->select) g_free(parameter->select);
    parameter->select = g_strdup(select);
}

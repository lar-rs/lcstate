/*
 * mkt-string.h
 *
 *  Created on: 28.11.2011
 *      Author: sascha
 */

#ifndef GL_STRING_H_
#define GL_STRING_H_
#include <glib.h>






gchar**  gl_strsplit_set (const gchar* string,const gchar *delimiters,const gchar *strsep,gint max_tokens);

gchar**  gl_strvadd_part          ( const gchar **strv , const gchar *part );
gchar**  gl_strvadd_part_and_free ( gchar **strv , const gchar *part );

void     gl_str_delete_eof        (const char *str);
gboolean gl_str_is_comment        (const char *str);

#endif /* MKTSTRING_H_ */

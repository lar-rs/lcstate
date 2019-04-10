/*
 * mkt-string.c
 *
 *  Created on: 28.11.2011
 *      Author: sascha
 */

#include "gl-string.h"
#include <stdlib.h>
#include <string.h>


gchar**  gl_strsplit_set (const gchar* string,const gchar *delimiters,const gchar *strsep,gint max_tokens)
{
	char  delim_table[256];
	char  str_table[256];
	int   n_tokens;
	const char *s;
	const char *current;
	const char *str_token = NULL;
	char  **result        = NULL;
	char  *token;
	if(string == NULL)
	{
		g_critical("ERROR: function mktStrsplit_set string == NULL\n");
		return NULL;
	}
	if(delimiters == NULL)
	{
		g_critical("ERROR: function mktStrsplit_set delimiters == NULL\n");
		return NULL;
	}
	if (max_tokens < 1)
		max_tokens = 60;


	if (*string == '\0')
	{
		result    = calloc( sizeof(char *) , 1 );
		result[0] = NULL;
		return result;
	}

	result = calloc(sizeof(char *), max_tokens+1);
	int count = 0;
	for(;count < max_tokens;count++)result[count]=NULL;
	memset (delim_table, 0, sizeof (delim_table));
	memset (str_table, 0, sizeof (str_table));
	for (s = delimiters; *s != '\0'; ++s)
		delim_table[*(unsigned char *)s] = 1;
	if(strsep != NULL)
	{
		for (s = strsep; *s != '\0'; ++s)
			str_table[*(unsigned char *)s] = 1;
	}
	n_tokens = 0;
	s = current = string;
	while (*s != '\0')
	{
		if (delim_table[*(unsigned char *)s] && n_tokens + 1 < max_tokens)
		{
			if(str_token != NULL)
			{

				token = g_strndup(current, str_token - current);
			}
			else
			{
				token = g_strndup(current, s - current);
			}
			str_token = NULL;
			if(result) result[n_tokens] = token;
			++n_tokens;
			current = s + 1;
		}
		if(str_table[*(unsigned char *)s]&& n_tokens + 1 < max_tokens)
		{
			++s;
			current = s;
			while(!str_table[*(unsigned char *)s])
			{
				if(*s == '\0') break;
				++s;
			}
			str_token = s;

		}
		if(*s != '\0')++s;
	}
	if(str_token != NULL)
		token = g_strndup(current, str_token - current);
	else
		token = g_strndup(current, s - current);
	result[n_tokens] = token;
	++n_tokens;
	result[n_tokens] = NULL;
	return result;
}

gchar**  gl_strvadd_part ( const gchar **strv , const gchar *part )
{
	g_return_val_if_fail (strv != NULL,NULL) ;
	g_return_val_if_fail (part != NULL,NULL) ;
	gint i;
	gint len = g_strv_length((gchar**)strv)+1;
	gchar **ret = calloc(sizeof(char *), len+1);
	g_return_val_if_fail (ret != NULL,NULL) ;
	for( i=0; strv[i]!= NULL; i++ )ret[i] = strdup(strv[i]);
	ret[i] = strdup(part);ret[i+1] = NULL;
	return ret;
}

gchar**
gl_strvadd_part_and_free ( gchar **strv , const gchar *part )
{
	g_return_val_if_fail (part != NULL,strv) ;
	gint i = 0;
	gint len = 1;
	if(strv != NULL)len += g_strv_length((gchar**)strv);
	gchar **ret = calloc(sizeof(char *), len+1);
	g_return_val_if_fail (ret != NULL,strv) ;
	for( i=0; strv!=NULL &&  strv[i]!= NULL; i++ ) ret[i] = strdup(strv[i]);
	if(strv != NULL) g_strfreev(strv);
	ret[i] = strdup(part);ret[i+1] = NULL;
	return ret;
}





void
gl_str_delete_eof   (const char *str)
{
	char *p = NULL;
	if(str != NULL)
	{
		p = (char *) str ;
		for(;*p!='\0';p++)
		{
			if(*p=='\n')*p = '\0';
		}
	}
}


gboolean
gl_str_is_comment(const char* string)
{
	char *p = (char*) string;
	if(p != NULL)
	{
		for(;*p!='\0';p++)
		{
			if(*p>32)
			{
				if(*p=='#') return 1;
				else return 0;
			}
		}
		return 0;
	}
	return -1;

}



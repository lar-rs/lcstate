/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * gl-translation.c
 * Copyright (C) A.Smolkov 2011 <asmolkov@lar.com>
 * 
 * gl-translation.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gl-translation.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "gl-translation.h"
#include "gl-connection.h"
#include "gl-widget-option.h"
#include "gl-level-manager.h"

#include <market-translation.h>
#include <mkt-collector.h>

#include <glib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
//#include "lar-localize.h"

const gchar* __GL_TR_TEST__ = NULL;

#define GL_TRANSLATION_MAX_LANGUAGE 10
#define GL_TRANSLATION_PROGRAM_LANGUAGE "default"

#define GL_TRANSLATION_DEFAULT_LANGUAGE "English"
#define GL_TRANSLATION_DEFAULT_LCID     "en_GB"

struct _GlTranslationPrivate
{
	gchar*       language;
	gint         language_nummer;
	gchar*       filepath;
	GKeyFile    *key_file;
	gint         tanslation_type;
	gchar       *last_text;
	guint        file_type;
	GScanner    *scanner;
	GHashTable  *translate_table;
	gchar      **head;
	gboolean     loaded;
	gchar       *language_str;

	GList       *translate_widgets;

	gchar*       static_on;
};


#define GL_TRANSLATE_MAIN_GROUP      "GuiTranslate"
#define GL_TRANSLATE_DEFAULT_FORMAT  "#FORMAT:name"


#define GL_TRANSLATION_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_TRANSLATION, GlTranslationPrivate))


enum
{
	PROP_0,
	PROP_LANGUAGE,
	PROP_FILEPATH,
};


enum
{
	GL_TRANSLATION_TRANSLATE,
	GL_TRANSLATION_LAST_SIGNAL
};


static guint gl_translation_signals[GL_TRANSLATION_LAST_SIGNAL] = { 0 };


static void  gl_translation_parameters_language_set_combobox_from_item   ( GlTranslation *trans,  GtkWidget *widget );
static void  gl_translation_sett_all_widget_from_parameter               ( GlTranslation *trans );

//FIXME: send change language signal
/*gboolean
gl_translation_change_language_notify   ( GlBinding *binding ,GlTranslation *translation )
{
	//TEST:g_debug("gl_translation_change_language_notify");
	g_return_val_if_fail(GL_IS_BINDING(binding),FALSE);
	g_return_val_if_fail(translation !=NULL,FALSE);
	const mktItem_t *item = gl_binding_get_input_item(binding);
	g_return_val_if_fail(item != NULL,FALSE);
	if(item->type != MKT_ITEM_TYPE_string32)
	{
		g_warning ( "plugin set translate language : item  incorrect type");
		return FALSE;
	}
	market_translation_set_language(item->value.string32);
	g_signal_emit(translation,gl_translation_signals[GL_TRANSLATION_TRANSLATE],0);
	return TRUE;
}*/


G_DEFINE_TYPE (GlTranslation, gl_translation, MKT_TYPE_ATOM);

static void
gl_translation_init (GlTranslation *object)
{
	/* TODO: Add initialization code here */
	object->priv = G_TYPE_INSTANCE_GET_PRIVATE(object,GL_TYPE_TRANSLATION,GlTranslationPrivate);
	object->priv->loaded    = FALSE;
	//FIXME: connect model signal handler.
	//gl_connection_connect_binding_signal("parameter___language",G_CALLBACK(gl_translation_change_language_notify),object);
}

static void
gl_translation_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	//GlTranslation *transl = GL_TRANSLATION(object);
	//g_free(translanguage);
	G_OBJECT_CLASS (gl_translation_parent_class)->finalize (object);
}

static void
gl_translation_init_market_translation ( GlTranslation *trans )
{

	//g_debug("gl_translation_init_market_translation 1");
	if(!market_translation_is_loaded())
		market_translation_init(trans->priv->filepath);

	mktList *keys = market_translation_get_keys();
	mktList *l = NULL;
	for(l = keys;l!=NULL;l= l-> next)
	{
		if(l->data )
		{
			const gchar *key =  (const gchar *) l->data;
			//g_debug("gl_translation_init_market_translation Create binding %s",key);
			GlBinding *binding = gl_connection_get_binding(key);
			if(binding == NULL)
			{
				binding = gl_binding_new(key);
				gl_connection_add_binding(binding);
			}
			if(binding)
			{
				gl_binding_add_translation(GL_BINDING(binding),trans);
			}
		}
	}
	if(keys)mktListFree(keys);
}

static void
gl_translation_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_TRANSLATION (object));
	GlTranslation *trans = GL_TRANSLATION(object);
	//g_debug("GL_TRANSLATION:set_property\n");
	switch (prop_id)
	{
	case PROP_LANGUAGE:
		/* TODO: Add setter for "language" property here */
//		g_signal_emit(trans,gl_translation_signals[GL_TRANSLATION_TRANSLATE],0);
		break;
	case PROP_FILEPATH:
		g_free(trans->priv->filepath);
		trans->priv->filepath = g_value_dup_string(value);
		gl_translation_init_market_translation(trans);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_translation_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (GL_IS_TRANSLATION (object));
	GlTranslation *trans = GL_TRANSLATION(object);
	//g_debug("GL_TRANSLATION:get_property\n");
	switch (prop_id)
	{
	case PROP_LANGUAGE:
		g_value_set_string(value,(const gchar *)trans->priv->language);
		break;
	case PROP_FILEPATH:
		g_value_set_string(value,(const gchar *)trans->priv->filepath);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_translation_class_init (GlTranslationClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (GlTranslationPrivate));

	object_class->finalize     = gl_translation_finalize;
	object_class->set_property = gl_translation_set_property;
	object_class->get_property = gl_translation_get_property;

	klass->translate           = NULL;

	GParamSpec *pspec;
	pspec = g_param_spec_string ("filepath",
			"File path",
			"Set/Get translatin file path",
			"/lar/var/translation.csv",
			G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
	g_object_class_install_property (object_class,
			PROP_FILEPATH,pspec);
	pspec = g_param_spec_string ("language",
			"Language",
			"Set/Get language",
			"en-uk",
			G_PARAM_READABLE | G_PARAM_WRITABLE );
	g_object_class_install_property (object_class,
			PROP_LANGUAGE,pspec);

	gl_translation_signals[GL_TRANSLATION_TRANSLATE] =
			g_signal_new ("translate",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET (GlTranslationClass, translate),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

GlTranslation*
gl_translation_new ( const gchar *id , const gchar *path )
{
	return GL_TRANSLATION(mkt_atom_object_new(GL_TYPE_TRANSLATION,MKT_ATOM_PN_ID,id,"filepath",path,NULL));
}


const gchar*
gl_translation_get_default_text(GlTranslation *translation , const  gchar *group, const  gchar *name)
{
	//g_debug("gl_translation_get_default_text ID %s LANGUAGE %s",name,translation->priv->language);
	GHashTable *table = NULL;
	if(!translation->priv->loaded)return NULL;
	if(name == NULL) return NULL;
	table = ( GHashTable* ) g_hash_table_lookup(translation->priv->translate_table,(gconstpointer)translation->priv->language);
	if(table != NULL)
	{
		//g_debug("table found language %s ",translation->priv->language);
		const gchar *text = NULL;
		text =(const gchar *) g_hash_table_lookup(table,(gconstpointer)name);
		if(text != NULL )
		{
			//TEST:g_debug("find id %s text=%s",name,text);
			return text;
		}
		else
		{
			//TEST:g_debug ( "text not found ");
		}
	}
	return NULL;
}


const gchar*
gl_translation_get_text ( GlTranslation *translation ,const gchar *module,const  gchar *id)
{
	g_return_val_if_fail(translation != NULL,NULL);
	g_return_val_if_fail(GL_IS_TRANSLATION(translation),NULL);
	g_return_val_if_fail(id != NULL,NULL);
	g_return_val_if_fail(translation->priv->language!= NULL,NULL);
	const gchar  *group = module!=NULL?module:GL_TRANSLATE_MAIN_GROUP;
	return gl_translation_get_default_text(translation,group,  id);
}

gboolean
gl_translation_parameters_change_language(GtkWidget *widget,gpointer data)
{
	static gchar *language = NULL;
	if(language) g_free(language);
	language = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));
	//FIXME: set new language.
	GValue value =  {0};
	g_value_init(&value,G_TYPE_STRING);
	g_value_set_string(&value,language);
	g_value_unset(&value);
	//FIX:Binding gl_connection_change_value_item(connection,item,TRUE);
	return TRUE;
}
void
gl_translation_sett_all_widget_from_parameter ( GlTranslation *trans )
{
	g_return_if_fail(trans != NULL);
	g_return_if_fail(GL_IS_TRANSLATION(trans));
	GList *curr = trans->priv->translate_widgets;
	while(curr != NULL)
	{
		if(curr -> data && GTK_IS_COMBO_BOX(curr -> data) )
		{
			gl_translation_parameters_language_set_combobox_from_item(trans,GTK_WIDGET(curr -> data));
		}
		curr = curr->next;
	}
}

void
gl_translation_parameters_language_set_combobox_from_item( GlTranslation *trans,  GtkWidget *widget)
{
	//FIXME: init translation combobox new language.
	//mktItem_t *item = mktSubscriptionLookupInputItem(subscription0071,"parameter___language");
	//g_return_if_fail( item != NULL);
	g_return_if_fail( widget != NULL );
	g_return_if_fail( GTK_IS_COMBO_BOX(widget));

	/*GtkTreeModel *treemodel = gtk_combo_box_get_model(GTK_COMBO_BOX(widget));
	GtkTreeIter iter ;
	gint count = 0;
	gboolean valid = gtk_tree_model_get_iter_first(treemodel,&iter);
	while(valid)
	{
		gchar *text = NULL;
		gtk_tree_model_get(treemodel,&iter,0,&text,-1);
		if(text!=NULL)
		{
			if(0==g_strcmp0(text , item->value.string32))
			{
				gtk_signal_handler_block_by_func(GTK_OBJECT(widget), GTK_SIGNAL_FUNC(gl_translation_parameters_change_language), trans);
				gtk_combo_box_set_active(GTK_COMBO_BOX(widget),count);
				gtk_signal_handler_unblock_by_func(GTK_OBJECT(widget), GTK_SIGNAL_FUNC(gl_translation_parameters_change_language), trans);
				break;
			}
			g_free(text);
		}
		valid =  gtk_tree_model_iter_next ( treemodel , &iter );
		count++;
	}*/
}

GtkWidget*
gl_translation_get_combobox    ( GlTranslation *translation )
{

	GtkWidget *combobox = gtk_combo_box_new_text();
	mktList *languages  = market_translation_get_languages();
	mktList *l = NULL;
	for (l=languages ; l != NULL ; l=l->next)
	{
		if( l->data )
			gtk_combo_box_append_text(GTK_COMBO_BOX(combobox),(const gchar*) l->data);
	}
	translation->priv->translate_widgets = g_list_append(translation->priv->translate_widgets,combobox);
	gl_translation_parameters_language_set_combobox_from_item( translation, combobox);
	g_signal_connect(  combobox ,"changed" , G_CALLBACK(gl_translation_parameters_change_language),translation );
	mktListFreeFull(languages,free);
	return combobox;
}


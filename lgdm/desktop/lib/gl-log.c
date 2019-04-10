/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * dbusexample
 * Copyright (C) sascha 2012 <sascha@sascha-ThinkPad-X61>
 * 
dbusexample is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * dbusexample is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl-log.h"
#include "gl-indicate.h"
#include "gl-connection.h"
#include "gl-plugin.h"
#include "gl-action-widget.h"
#include "gl-translation.h"
#include "gl-date-time-window.h"
#include "gl-widget-option.h"
#include "gl-level-manager.h"

#include <market-translation.h>
#include <mkt-collector.h>
#include <mkt-error.h>
#include <mkt-log.h>
#include <market-time.h>
#include <mkt-error-message.h>
#include <mkt-utils.h>
#include <mkt-dbus.h>
#include "../lgdm-status.h"
//#include "market-db.h"



enum
{
	GL_SYSTEM_LOG_OK,
	GL_SYSTEM_LOG_WARNING,
	GL_SYSTEM_LOG_CRITICAL,
	GL_SYSTEM_LOG_UNKNOWN,
	GL_SYSTEM_LOG_LAST
};


enum
{
	GL_SYSTEM_LOG_TYPE_SYSTEM,
	GL_SYSTEM_LOG_TYPE_ERRORS,
	GL_SYSTEM_LOG_TYPE_LIMIT,
	GL_SYSTEM_LOG_TYPE_MEASUREMENT,
	GL_SYSTEM_LOG_TYPE_CALIBRATE,
	GL_SYSTEM_LOG_TYPE_SERVICE_LOG,
	GL_SYSTEM_LOG_TYPE_OPERATOR,
	GL_SYSTEM_LOG_TYPE_UNKNOWN,
	GL_SYSTEM_LOG_TYPE_LAST,

};

typedef struct
{
	gint text;
	gint color;

}GlLogColumnType;


struct _GlLogPrivate
{
	GtkWidget   *treeview;
	GlIndicate  *door_indicate;
	GlIndicate  *remote_indicate;
	GlIndicate  *system_state_indicate;
	gchar       *door_icons;
	gchar       *remote_control_icons;
	gchar       *state_icons[GL_SYSTEM_LOG_LAST];
	gchar       *log_rsql;
	GtkWidget   *log_tree;
	GtkWidget   *notebook;
	gboolean     loaded;
	GtkTreeIter  log_book_iter[MKT_LOG_MESSAGE_LAST];
	gint         idle_tag;
	guint        error_update;
	guint        update_tag;
	//sqlite3_stmt *res;
	gboolean     log_book_loaded;
	gint         load_limit;
	gchar        error_type;
	gchar       *date;
	gboolean     errors_async;
	gboolean     reload_errors_async;
	gboolean     logbook_async;
	guint        last_offset;
	gboolean     need_reload;
	gint         type_sort;
};

#define GL_LOG_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_LOG, GlLogPrivate))



//parameter___drag_and_drop

G_DEFINE_TYPE (GlLog, gl_log, GL_TYPE_PLUGIN);


enum
{
	GL_LOG_PROP0,
	GL_LOG_XML_PATH,
	GL_LOG_DRAG_AND_DROP
};

enum
{
	GL_LOG_LAST_SIGNAL
};


//static guint gl_log_signals[GL_LOG_LAST_SIGNAL] = { 0 };


static void
gl_log_need_data_reload ( GlLog *log )
{
	log->priv->need_reload = TRUE;
	log->priv->last_offset = 0;

}


static void
gl_log_reload_errors_tree_view (GSList *errors, GlLog *log  )
{
	log->priv->reload_errors_async = FALSE;
	if( errors )
	{
		if(log->priv->treeview==NULL)
			log->priv->treeview= mkt_window_find_widget(MKT_WINDOW(log),"gl_system_log_tree");

		g_return_if_fail(log->priv->treeview != NULL);
		GList *list;
		GList* columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(log->priv->treeview));
		list = columns;
		while(list!= NULL)
		{
			gtk_tree_view_remove_column(GTK_TREE_VIEW(log->priv->treeview),GTK_TREE_VIEW_COLUMN( list->data));
			list = list->next;
		}
		g_list_free(columns);
		gint count = 0;
		GtkTreeViewColumn *col;
		GtkCellRenderer   *renderer;

		gint column = 0;

		col = gtk_tree_view_column_new();
		gtk_tree_view_column_set_title(col,_TR_("TRANSLATE_static_ErrorLogsErrno","Err no."));
		gtk_tree_view_column_set_alignment(col,0.1);
		gtk_tree_view_column_set_sort_column_id(col,count);
		renderer = gtk_cell_renderer_text_new();
		gtk_tree_view_column_pack_start(col,renderer,TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(log->priv->treeview),col);
		gtk_tree_view_column_set_fixed_width(col,200);
		gtk_tree_view_column_set_attributes( col, renderer,"text", column,
				"background", column+1,
				NULL );
		column +=2;


		col = gtk_tree_view_column_new();
		gtk_tree_view_column_set_title(col,_TR_("TRANSLATE_static_ErrorLogsDiscription","Description"));
		gtk_tree_view_column_set_alignment(col,0.1);
		gtk_tree_view_column_set_sort_column_id(col,count);
		renderer = gtk_cell_renderer_text_new();
		gtk_tree_view_column_pack_start(col,renderer,TRUE);
		gtk_tree_view_append_column(GTK_TREE_VIEW(log->priv->treeview),col);
		gtk_tree_view_column_set_fixed_width(col,200);
		gtk_tree_view_column_set_attributes( col, renderer,"text", column,
				"background", column+1,
				NULL );
		column +=2;


		GtkTreeStore       *treestore;
		treestore = gtk_tree_store_new( 4,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);
		gtk_tree_view_set_model(GTK_TREE_VIEW(log->priv->treeview),GTK_TREE_MODEL(treestore));

		GtkTreeIter iter;
		gboolean found_errors = FALSE;
		gboolean critical     = FALSE;
		gboolean warning     = FALSE;
		log->priv->error_type = -1;
		g_debug("test here 1");
		GSList *l = NULL;
		for(l= errors ;l!=NULL;l=l->next)
		{
			g_debug("Add error # %d",mkt_error_number(MKT_ERROR(l->data)));

			gchar *errNo = g_strdup_printf("E%d",mkt_error_number(MKT_ERROR(l->data)));
			if(errNo==NULL)continue;
			gtk_tree_store_append(treestore,&iter,NULL);
			gtk_tree_store_set(treestore,&iter,0,errNo,-1);
			g_free(errNo);
			const gchar *color = NULL;
			if(log->priv->error_type<mkt_error_state(MKT_ERROR(l->data)))
				log->priv->error_type = mkt_error_state(MKT_ERROR(l->data));

			switch ( mkt_error_state(MKT_ERROR(l->data)))
			{
			case MKT_ERROR_WARNING:color= "yellow";warning = TRUE;break;
			case MKT_ERROR_SWITCHABLE:color= "orange";warning = TRUE;break;
			case MKT_ERROR_CRITICAL:color= "red";critical = TRUE;break;
			default: color= "yellow";warning = TRUE;break;
			}
			gtk_tree_store_set(treestore,&iter,1,color,-1);
			gint column = 2;
			found_errors = TRUE;
			const gchar *description = mkt_error_description(MKT_ERROR(l->data));
			gtk_tree_store_set(treestore,&iter,column,description?description:"unknown description",-1);
			gtk_tree_store_set(treestore,&iter,column+1,color,-1);
		}
		if(found_errors)
		{
			if(critical)
			{
				//DEL(04.06.13): gl_plugin_set_action_widget_icon(GL_PLUGIN(log),"critical");
				gl_indicate_set_indicate_profile(log->priv->system_state_indicate,log->priv->state_icons[GL_SYSTEM_LOG_CRITICAL]);
				gl_indicate_start(log->priv->system_state_indicate);
				//mkIset(gui_subscription__ERROR_CRITICAL,1);
			}
			else if ( warning )
			{
				gl_indicate_set_indicate_profile(log->priv->system_state_indicate,log->priv->state_icons[GL_SYSTEM_LOG_WARNING]);
				//DEL(04.06.13):gl_plugin_set_action_widget_icon(GL_PLUGIN(log),"warning");
				gl_indicate_start(log->priv->system_state_indicate);
				//mkIset(gui_subscription__ERROR,1);
			}
			else
			{
				gl_indicate_set_indicate_profile(log->priv->system_state_indicate,log->priv->state_icons[GL_SYSTEM_LOG_OK]);
				gl_indicate_start(log->priv->system_state_indicate);
			}

		}
		else
		{
			gl_indicate_set_indicate_profile(log->priv->system_state_indicate,log->priv->state_icons[GL_SYSTEM_LOG_OK]);
			gl_indicate_start(log->priv->system_state_indicate);
			//DEL(04.06.13):gl_plugin_set_action_widget_icon(GL_PLUGIN(log),"default");
		}
		g_object_unref(treestore);
	}
}

static void
mkt_log_reload_errors_start (  GlLog *log  )
{
	if(!log->priv->reload_errors_async)
	{
		if(mkt_model_select_async(MKT_TYPE_ERROR,(MktModelAsyncCallback )gl_log_reload_errors_tree_view,
				log,"select * from $tablename where error_pending = 1") )
		{
			log->priv->reload_errors_async = TRUE;
		}
	}
}

static void
gl_log_system_tree_create_head ( GlLog *log )
{
	if(log->priv->log_tree==NULL)
		log->priv->log_tree= mkt_window_find_widget(MKT_WINDOW(log),"gl_system_main_log_tree");
	g_return_if_fail(log->priv->log_tree != NULL);
	GList *list;
	GList *columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(log->priv->log_tree));
	list = columns;
	while(list!= NULL)
	{
		gtk_tree_view_remove_column(GTK_TREE_VIEW(log->priv->log_tree),GTK_TREE_VIEW_COLUMN( list->data));
		list = list->next;
	}
	g_list_free(columns);
	GtkTreeViewColumn *col;
	GtkCellRenderer   *renderer;
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col,_TR_("TRANSLATE_column_DataBase_LogTree_type","Type"));
	gtk_tree_view_column_set_alignment(col,0.1);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col,renderer,TRUE);
	gtk_tree_view_column_add_attribute(col,renderer,"text",0);
	gtk_tree_view_append_column(GTK_TREE_VIEW(log->priv->log_tree),col);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col,_TR_("TRANSLATE_column_DataBase_LogTree_time","Time"));
	gtk_tree_view_column_set_alignment(col,0.1);
	gtk_tree_view_column_set_sort_column_id(col,1);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col,renderer,TRUE);
	gtk_tree_view_column_add_attribute(col,renderer,"text",1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(log->priv->log_tree),col);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(col,_TR_("TRANSLATE_column_DataBase_LogTree_message","Message"));
	gtk_tree_view_column_set_sort_column_id(col,2);
	gtk_tree_view_column_set_alignment(col,0.1);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col,renderer,TRUE);
	gtk_tree_view_column_add_attribute(col,renderer,"text",2);
	gtk_tree_view_append_column(GTK_TREE_VIEW(log->priv->log_tree),col);

	GtkTreeStore *treestore;
	treestore = gtk_tree_store_new(4,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,	G_TYPE_STRING );
	gtk_tree_view_set_model(GTK_TREE_VIEW(log->priv->log_tree),GTK_TREE_MODEL(treestore));
	g_object_unref( treestore );
	//TEST:g_debug("Test .. end....................................");

}


static void
gl_logs_start_on_indicate_clicked ( GlIndicate *indicate , GlLog *log )
{
	//TEST:g_debug("gl_logs_start_on_indicate_clicked");
	g_return_if_fail(log!=NULL);
	g_return_if_fail(GL_IS_LOG(log));
	if(log->priv->error_type>=0)
	{
		mkt_log_reload_errors_start(log);
		GtkWidget *widget = mkt_window_find_widget(MKT_WINDOW(log),"TimeDateLogBook");
		if(widget)gtk_widget_hide(widget);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(log->priv->notebook),0);
	}
	else
	{
		GtkWidget *widget = mkt_window_find_widget(MKT_WINDOW(log),"TimeDateLogBook");
		if(widget)gtk_widget_show(widget);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(log->priv->notebook),1);
	}
	gl_plugin_open(GL_PLUGIN(log));
}

static void
gl_log_system_tree_add_data (GSList *models ,  GlLog *log )
{
	log->priv->logbook_async = FALSE;
	if(models)
	{
		if(log->priv->last_offset == 0)
		{
			gl_log_system_tree_create_head(log);
		}
		GSList *l = NULL;
		for(l=models;l!=NULL;l=l->next)
		{
			const gchar *tname = NULL;
			gint type = mkt_log_type(MKT_LOG(l->data));
			gint iter = type;
			switch (type  )
			{
			case MKT_LOG_STATE_SYSTEM:
				tname = _TR_("TRANSLATE_type_DataBase_LogTree_messageSystem","System");
				break;
			case MKT_LOG_MESSAGE_ERRORS:
				tname = _TR_("TRANSLATE_type_DataBase_LogTree_messageErrors","Errors");
				break;
			case MKT_LOG_MESSAGE_LIMIT:
				tname = _TR_("TRANSLATE_type_DataBase_LogTree_messageLimit","Limit");
				break;
			case MKT_LOG_MESSAGE_MEASUREMENT:
				tname = _TR_("TRANSLATE_type_DataBase_LogTree_messageMeasurementDaten","Measurement Daten");
				break;
			case MKT_LOG_MESSAGE_CALIBRATE:
				tname = _TR_("TRANSLATE_type_DataBase_LogTree_messageCalibrateDaten","Calibrate Daten");
				break;
			case MKT_LOG_MESSAGE_SERVICE_LOG:
				tname = _TR_("TRANSLATE_type_DataBase_LogTree_messageServiceLog","Service Log");
				break;
			case MKT_LOG_MESSAGE_OPERATOR:
				tname = _TR_("TRANSLATE_type_DataBase_LogTree_messageOperatorsLog","Operators Log");
				break;
			case MKT_LOG_MESSAGE_UNKNOWN:
				tname = _TR_("TRANSLATE_type_DataBase_LogTree_messageUnknown","Unknown");
				break;
			default:
				tname = _TR_("TRANSLATE_type_DataBase_LogTree_messageUnknown","Unknown");
				iter = MKT_LOG_MESSAGE_UNKNOWN;
				break;
			}
			GtkTreeStore   *treestore     = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(log->priv->log_tree)));
			if(iter >= 0 && tname != NULL && iter < MKT_LOG_MESSAGE_LAST )
			{
				if(iter<=MKT_LOG_MESSAGE_LIMIT || gl_level_manager_is_tru_user(GUI_USER_SUPER_OPERATOR_TYPE))
				{
					GtkTreeIter  child;
					if(!gtk_tree_store_iter_is_valid(GTK_TREE_STORE(treestore),&log->priv->log_book_iter[iter]))
					{
						gtk_tree_store_append(GTK_TREE_STORE(treestore),&log->priv->log_book_iter[iter],NULL);
						gtk_tree_store_set(GTK_TREE_STORE(treestore),&log->priv->log_book_iter[iter],0,tname,-1);
					}
					gtk_tree_store_append(GTK_TREE_STORE(treestore),&child,&log->priv->log_book_iter[iter]);
					gtk_tree_store_set(GTK_TREE_STORE(treestore),&child,1,market_db_get_date_hmydm(mkt_log_changed(l->data)),2,mkt_log_message(l->data),-1);
				}
			}
		}
		log->priv->last_offset = log->priv->last_offset + g_slist_length(models);
	}
	else
	{
		g_debug("Model not find .... ");
		log->priv->need_reload = FALSE;
	}

}

static void
gl_data_base_change_search_time ( GlLog *log )
{
	MktWindow *td_win = MKT_WINDOW(mkt_collector_get_atom_static("com.lar.GlDateTimeWindow.LogBookTiming"));
	if(td_win == NULL )return ;
	if(log->priv->date)g_free(log->priv->date);
	log->priv->date = NULL;
	log->priv->date = g_strdup_printf("log_changed > %f and log_changed < %f",gl_date_time_window_get_from(GL_DATE_TIME_WINDOW(td_win)),gl_date_time_window_get_to(GL_DATE_TIME_WINDOW(td_win)));
	gl_log_need_data_reload(log);
}

static void
gl_log_change_data_time ( GlDateTimeWindow *td_win , GlLog *log)
{
	GtkWidget *from = mkt_window_find_widget(MKT_WINDOW(log),"GlLogbookSearchDateFrom");
	GtkWidget *to = mkt_window_find_widget(MKT_WINDOW(log),"GlLogbookSearchDateto");
	gboolean is_set = gl_date_time_window_is_set(td_win);
	if(from != NULL)
	{
		if(is_set)gtk_label_set_text(GTK_LABEL(from),market_db_get_date_hmydm(gl_date_time_window_get_from(td_win)));
		else gtk_label_set_text(GTK_LABEL(from),"--:--");
	}
	if(to != NULL)
	{
		if(is_set)gtk_label_set_text(GTK_LABEL(to),market_db_get_date_hmydm(gl_date_time_window_get_to(td_win)));
		else gtk_label_set_text(GTK_LABEL(to),"--:--");
	}
	gl_data_base_change_search_time(log);

}

static void
gl_log_analyse_errors (GSList *errors , GlLog *log )
{
	log->priv->errors_async = FALSE;
	log->priv->error_type = -1;
	guint error_level = GL_SYSTEM_LOG_OK;
	if(errors)
	{
		GSList *l = NULL;
		for(l= errors ;l!=NULL;l=l->next)
		{
			if(log->priv->error_type< mkt_error_state(MKT_ERROR(l->data)))
				log->priv->error_type= mkt_error_state(MKT_ERROR(l->data));

			switch ( mkt_error_state(MKT_ERROR(l->data)))
			{
			case MKT_ERROR_WARNING:
				if(error_level<GL_SYSTEM_LOG_WARNING) error_level = GL_SYSTEM_LOG_WARNING;
				break;
			case MKT_ERROR_SWITCHABLE:
				if(error_level<GL_SYSTEM_LOG_WARNING) error_level = GL_SYSTEM_LOG_WARNING;
				break;
			case MKT_ERROR_CRITICAL:
				if(error_level<GL_SYSTEM_LOG_CRITICAL) error_level = GL_SYSTEM_LOG_CRITICAL;
				break;
			default:
				if(error_level<GL_SYSTEM_LOG_WARNING) error_level = GL_SYSTEM_LOG_WARNING;
				break;
			}
		}
		gl_indicate_set_indicate_profile(log->priv->system_state_indicate,log->priv->state_icons[error_level]);
		gl_indicate_start(log->priv->system_state_indicate);
	}
	else
	{
		gl_indicate_set_indicate_profile(log->priv->system_state_indicate,log->priv->state_icons[GL_SYSTEM_LOG_OK]);
		gl_indicate_start(log->priv->system_state_indicate);
	}
}


static void
mkt_log_check_errors_start ( GlLog *log )
{
	if(!log->priv->errors_async)
	{
		if(mkt_model_select_async(MKT_TYPE_ERROR,(MktModelAsyncCallback )gl_log_analyse_errors,
				log,"select * from $tablename where error_pending = 1") )
		{
			log->priv->errors_async = TRUE;
		}
	}
}



static gboolean
_log_update_cb(gpointer user_data )
{
	GlLog *log = GL_LOG(user_data);
	mkt_log_check_errors_start(log);
	return TRUE;
}

static void
_log_update_cb_destroy (gpointer      user_data)
{
	GlLog *log = GL_LOG(user_data);
	log->priv->update_tag = 0;
}




static void
mkt_log_reload_start ( GlLog *log )
{

	if(!log->priv->logbook_async)
	{

		if(log->priv->type_sort >-1 )
		{
			//	messages = mkt_model_select ( MKT_TYPE_LOG,"select * from $tablename  where log_type = %d %s LIMIT 30 OFFSET %d;",log->priv->type_sort,log->priv->load_limit);
			if( log->priv->type_sort >= MKT_LOG_MESSAGE_UNKNOWN )
			{
				if(mkt_model_select_async(MKT_TYPE_LOG,(MktModelAsyncCallback )gl_log_system_tree_add_data,
						log,"select * from $tablename  where log_type < 0 or log_type > 0 %s LIMIT 200 OFFSET %d;",log->priv->date?log->priv->date:"",log->priv->last_offset) )
				{
					log->priv->logbook_async = TRUE;
				}
			}
			else
			{
				if(mkt_model_select_async(MKT_TYPE_LOG,(MktModelAsyncCallback )gl_log_system_tree_add_data,
						log,"select * from $tablename  where log_type = %d and %s LIMIT 200 OFFSET %d;",log->priv->type_sort,
						log->priv->date?log->priv->date:"",log->priv->last_offset) )
				{
					log->priv->logbook_async = TRUE;
				}
			}
		}
		else
		{
			if(mkt_model_select_async(MKT_TYPE_LOG,(MktModelAsyncCallback )gl_log_system_tree_add_data,
					log,"select * from $tablename  where %s LIMIT 200 OFFSET %d;",log->priv->date?log->priv->date:"",log->priv->last_offset) )
			{
				log->priv->logbook_async = TRUE;
			}
		}
	}
}


static void
gl_log_check_reload (GlLog *log)
{
	//g_debug("Check reload .... need %d async=%d",log->priv->need_reload,log->priv->logbook_async);
	if(log->priv->need_reload && !log->priv->logbook_async)
	{
		mkt_log_reload_start(log);
	}
}

gboolean
__log_book_load_cb ( gpointer data)
{
	GlLog *log = GL_LOG(data);
	gl_log_check_reload(log);
	return  TRUE;
}

void
__log_book_load_cb_destroy (gpointer      user_data)
{
	GlLog *log = GL_LOG(user_data);
	log->priv->idle_tag     = 0;
}



static void
gl_log_open( MktWindow *win , gpointer data )
{
	g_return_if_fail ( win != NULL );
	g_return_if_fail ( MKT_IS_WINDOW(win) );
	GlLog *log = GL_LOG(win);
	GtkWidget *type = mkt_window_find_widget(win,"SortTypeComboBox");
	if(type ) gtk_combo_box_set_active(GTK_COMBO_BOX(type),log->priv->type_sort+1);
	MktWindow *td_win = MKT_WINDOW(mkt_collector_get_atom_static ( "com.lar.GlDateTimeWindow.LogBookTiming"));
   	if(td_win)
   	{
   		if(!gl_date_time_window_is_set(GL_DATE_TIME_WINDOW(td_win)))
   		{
   			gl_date_time_set_current(GL_DATE_TIME_WINDOW(td_win));
   		}
   	}
   	if(log->priv->idle_tag==0)
   	{
   		log->priv->idle_tag = g_timeout_add_full( G_PRIORITY_DEFAULT ,500,__log_book_load_cb,log,__log_book_load_cb_destroy);
   	}
}

void
gl_log_close( MktWindow *win , gpointer data )
{
	g_return_if_fail ( win != NULL );
	g_return_if_fail ( MKT_IS_WINDOW(win) );
	GlLog *log = GL_LOG(win);
	if(log->priv->idle_tag!=0)	g_source_remove(log->priv->idle_tag);
	MktWindow *td_win = MKT_WINDOW(mkt_collector_get_atom_static ( "com.lar.GlDateTimeWindow.LogBookTiming"));
	mkt_window_hide(td_win);
}


static void
gl_log_init (GlLog *gl_log)
{
    GlLogPrivate *priv = GL_LOG_GET_PRIVATE(gl_log);
    gl_log->priv = priv;
    gl_log->priv->door_icons = g_strdup_printf("%s,%s,%s,%s,%s","/lar/gui/Alerts-iconsmall0.png","/lar/gui/Alerts-iconsmall1.png",
    		                                   "/lar/gui/Alerts-iconsmall2.png","/lar/gui/Alerts-iconsmall3.png","/lar/gui/Alerts-iconsmall4.png");
    gl_log->priv->remote_control_icons = g_strdup_printf("%s,%s,%s,%s,%s,%s","/lar/gui/netz0.png","/lar/gui/netz1.png",
        		                                         "/lar/gui/netz2.png","/lar/gui/netz3.png","/lar/gui/netz4.png","/lar/gui/netz3.png");

    gl_log->priv->state_icons[GL_SYSTEM_LOG_OK]       = g_strdup("/lar/gui/security-high.png");
    gl_log->priv->state_icons[GL_SYSTEM_LOG_WARNING]  = g_strdup("/lar/gui/security-medium.png");
    gl_log->priv->state_icons[GL_SYSTEM_LOG_CRITICAL] = g_strdup("/lar/gui/security-low.png");
    gl_log->priv->notebook                            = NULL;
    gl_log->priv->log_tree                            = NULL;
    gl_log->priv->idle_tag                            = 0;
    gl_log->priv->log_book_loaded                     = FALSE;
    gl_log->priv->type_sort                           = -1;
    gl_log->priv->date                                = NULL;
    gl_log->priv->need_reload                         = TRUE;
    gl_log->priv->last_offset                         = 0;
    gl_log->priv->errors_async                        = FALSE;
    gl_log->priv->logbook_async                       = FALSE;
    gl_log->priv->reload_errors_async                 = FALSE;
    //gl_log->priv->log_rsql                            = g_strdup_printf("select * from $tablename");


    GtkWidget *indicate_label = NULL;
    gl_log->priv->door_indicate = gl_indicate_new("com.lar.GlIndicate.DoorWatcher",GL_INDICATE_NO_ASK);
   	gl_indicate_set_indicate_profile(gl_log->priv->door_indicate,gl_log->priv->door_icons);
   	indicate_label = gtk_label_new(_TR_("TRANSLATE_static_OpenDor","Warning door open")); // FIX: Add translate to widget
   	gl_widget_option_set_name(G_OBJECT(indicate_label),"TRANSLATE_com.lar.GlIndicate.DoorWatcher_IndicateDescription");
   	gl_binding_insert_widget(indicate_label);
   	gtk_misc_set_alignment(GTK_MISC(indicate_label),0.01,0.5);
   	gtk_widget_modify_font(GTK_WIDGET(indicate_label),pango_font_description_from_string("Oreal 8"));
   	gtk_widget_show(indicate_label);
   //MemoryTestRem:
   	//gl_connection_connect_binding_signal("history___errorState",G_CALLBACK(gl_log_system_change_error_state),gl_log);
   	gl_indicate_set_indicate_box(gl_log->priv->door_indicate,indicate_label);
  /* 	GlBinding *binding = GL_BINDING(mkt_collector_get_atom_static("control_gui__door_open"));
   	if(binding )
   	{
   		g_signal_connect(binding ,"incoming-item",G_CALLBACK(gl_log_system_change_open_door),gl_log);
   	}
   	if( mkIget(control_gui__door_open) )
   	{
   		gl_indicate_start(gl_log->priv->door_indicate);
   	}
   	else
   	{
    	gl_indicate_stop(gl_log->priv->door_indicate);
   	}*/

   	gl_log->priv->remote_indicate = gl_indicate_new("com.lar.GlIndicate.RemouteControl",GL_INDICATE_NO_ASK);
    gl_indicate_set_indicate_profile(gl_log->priv->remote_indicate,gl_log->priv->remote_control_icons);
    indicate_label = gtk_label_new(_TR_("TRANSLATE_static_RCActive","Warning remote control active")); // FIX: Add translate to widget
	gl_widget_option_set_name(G_OBJECT(indicate_label),"TRANSLATE_com.lar.GlIndicate.RemouteControl_IndicateDescription");
    gtk_misc_set_alignment(GTK_MISC(indicate_label),0.01,0.5);
    gtk_widget_modify_font(GTK_WIDGET(indicate_label),pango_font_description_from_string("Oreal 8"));
    gtk_widget_show(indicate_label);
    gl_indicate_set_indicate_box(gl_log->priv->remote_indicate,indicate_label);
    /* gl_connection_connect_binding_signal("pc_rc__active",G_CALLBACK(gl_log_system_change_rc_active),gl_log);
      if( mkIget(pc_rc__active) )
    {
    	gl_indicate_start(gl_log->priv->remote_indicate);
    }
    else
    {
    	gl_indicate_stop(gl_log->priv->remote_indicate);
    }*/

    gl_log->priv->system_state_indicate = gl_indicate_new("com.lar.GlIndicate.SystemCheck",GL_INDICATE_NO_ASK);
   	gl_indicate_set_indicate_profile(gl_log->priv->system_state_indicate,gl_log->priv->state_icons[GL_SYSTEM_LOG_CRITICAL]);
   //	indicate_label = gtk_label_new(mkIget(history___internalControlState)); // FIX: Add translate to widget
   //	gl_widget_option_set_name(G_OBJECT(indicate_label),"control_subscription__internalControlState");
   //	gl_binding_insert_widget(indicate_label);
   	gtk_misc_set_alignment(GTK_MISC(indicate_label),0.01,0.5);
   	gtk_widget_modify_font(GTK_WIDGET(indicate_label),pango_font_description_from_string("Oreal 8"));
   	gtk_widget_show(indicate_label);
   	gl_indicate_set_indicate_box(gl_log->priv->system_state_indicate,indicate_label);
   	gl_indicate_start(gl_log->priv->system_state_indicate);
   	g_signal_connect(gl_log->priv->system_state_indicate,"click_start",G_CALLBACK(gl_logs_start_on_indicate_clicked),gl_log);

   	MktWindow *td_win = MKT_WINDOW(mkt_collector_get_atom_static ( "com.lar.GlDateTimeWindow.LogBookTiming"));
   	g_signal_connect(td_win,"changed-date-time",G_CALLBACK(gl_log_change_data_time),gl_log);
   	g_signal_connect ( MKT_WINDOW(gl_log), "window_show",G_CALLBACK(gl_log_open),NULL);
   	g_signal_connect ( MKT_WINDOW(gl_log), "window_hide",G_CALLBACK(gl_log_close),NULL);
	/* TODO: Add initialization code here */
}

static void
gl_log_finalize (GObject *object)
{
	GlLog *log = GL_LOG(object);
	if(log->priv->door_icons) g_free(log->priv->door_icons);
	if(log->priv->remote_control_icons) g_free(log->priv->remote_control_icons);
	g_free(log->priv->state_icons[GL_SYSTEM_LOG_OK]);
	g_free(log->priv->state_icons[GL_SYSTEM_LOG_WARNING]);
	g_free(log->priv->state_icons[GL_SYSTEM_LOG_CRITICAL]);
	gl_indicate_stop(log->priv->door_indicate);
	gl_indicate_stop(log->priv->remote_indicate);
	gl_indicate_stop(log->priv->system_state_indicate);

	G_OBJECT_CLASS (gl_log_parent_class)->finalize (object);
}



void
gl_log_set_property(  GObject        *object,
		guint           prop_id,
		const GValue   *value,
		GParamSpec     *pspec)
{
	//GlLog *log = GL_LOG(object);
	switch(prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}
void
gl_log_get_property(  GObject        *object,
		guint           prop_id,
		GValue         *value,
		GParamSpec     *pspec)
{
	//GlLog *log = GL_LOG(object);
	switch(prop_id)
	{

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object,prop_id,pspec);
		break;
	}
}





void
gl_log_system_init_notebook ( MktWindow *window, GtkWidget *notebook )
{
	//TEST:g_debug("gl_log_system_init_notebook for %s %s",gl_widget_option_get_name(notebook),G_OBJECT_TYPE_NAME(notebook));
	g_return_if_fail(window != NULL );
	g_return_if_fail(GL_IS_LOG(window));
	g_return_if_fail(notebook != NULL );
	g_return_if_fail (GTK_IS_NOTEBOOK(notebook));
	GL_LOG(window)->priv->notebook = notebook;
}



void
gl_log_system_realize_system_errors ( MktWindow *window, GtkWidget *tree )
{
	//TEST:g_debug( "gl_log_system_realize_system_errors for %s %s",gl_widget_option_get_name(tree),G_OBJECT_TYPE_NAME(tree));
	g_return_if_fail(window != NULL );
	g_return_if_fail(GL_IS_LOG(window));
	g_return_if_fail(tree != NULL );
	g_return_if_fail (GTK_IS_TREE_VIEW(tree));
	GL_LOG(window)->priv->treeview = tree;
}

void
gl_log_system_realize_system_log (  MktWindow *window, GtkWidget *tree )
{
	//TEST:g_debug( "gl_log_system_realize_system_log for %s %s",gl_widget_option_get_name(tree),G_OBJECT_TYPE_NAME(tree));
	g_return_if_fail(window != NULL );
	g_return_if_fail(GL_IS_LOG(window));
	g_return_if_fail(tree != NULL);
	g_return_if_fail (GTK_IS_TREE_VIEW(tree));
	GL_LOG(window)->priv->log_tree = tree;
	gl_log_system_tree_create_head(GL_LOG(window));
}
gboolean
gl_logs_load_menu ( GlPlugin *plugin )
{
	g_return_val_if_fail(plugin!= NULL ,FALSE);
	g_return_val_if_fail(GL_IS_LOG(plugin) ,FALSE);

	GtkWidget *notebook = mkt_window_find_widget(MKT_WINDOW(plugin),"gl_log_system_notebook");
	gl_log_system_init_notebook(MKT_WINDOW(plugin),notebook);

	GtkWidget *logtree = mkt_window_find_widget(MKT_WINDOW(plugin),"gl_system_main_log_tree");
	gl_log_system_realize_system_log(MKT_WINDOW(plugin),logtree);
	mkt_log_check_errors_start(GL_LOG(plugin));
	if(GL_LOG(plugin)->priv->update_tag==0)
		GL_LOG(plugin)->priv->update_tag = g_timeout_add_full( G_PRIORITY_DEFAULT ,900,_log_update_cb,plugin,_log_update_cb_destroy);

	return TRUE;
}

static void
gl_log_class_init (GlLogClass *klass)
{
	GObjectClass*   object_class  = G_OBJECT_CLASS (klass);
	//MktWindowClass *mktdraw_class = MKT_WINDOW_CLASS(klass);
	GlPluginClass* parent_class   = GL_PLUGIN_CLASS(klass);
	g_type_class_add_private (klass, sizeof (GlLogPrivate));
	object_class->finalize         = gl_log_finalize;
	parent_class->plugin_load_menu = gl_logs_load_menu;
}

gboolean
save_log_data_async_done_callback ( gpointer data)
{
	g_debug("save_log_data_async_done_callback");
	GtkWidget *widget  = mkt_window_find_widget(MKT_WINDOW(data),"SaveLogButton");
	gtk_widget_set_sensitive(widget,TRUE);
	return FALSE;
}

void
__save_log_data_async   (GType type ,guint64 id ,gpointer user_data )
{
	GlLog *log = GL_LOG(user_data);
	//
	//FIXME:Use PC interface for devicename read.
	gchar *path     = g_strdup_printf("/media/usb0/%s","noname");
	g_debug("async save run make patch %s",path);

	mkt_make_dir(path);
	//MktWindow *td_win = MKT_WINDOW(mkt_collector_get_atom_static ( "com.lar.GlDateTimeWindow.LogBookTiming"));
	//MARKET_TIME_FORMAT_FILENAME
	gdouble td = market_db_time_now();
	gchar *filepath = g_strdup_printf("%s/logbook-%s.csv",path,market_db_get_date_format(td,MARKET_TIME_FORMAT_FILENAME));
	FILE *file  = fopen(filepath,"w+");
	if(file)
	{
		GSList *datas = NULL;

		if(log->priv->type_sort >-1 )
		{
			if( log->priv->type_sort >= MKT_LOG_MESSAGE_UNKNOWN )
				datas = mkt_model_select(MKT_TYPE_LOG_MESSAGE,"select * from $tablename where log_type < 0 or log_type < %d and %s",log->priv->type_sort,log->priv->date?log->priv->date:"");
			else
				datas = mkt_model_select(MKT_TYPE_LOG_MESSAGE,"select * from $tablename where log_type = %d and %s",log->priv->type_sort,log->priv->date?log->priv->date:"");
		}
		else
		{
			datas = mkt_model_select(MKT_TYPE_LOG_MESSAGE,"select * from $tablename where %s",log->priv->date?log->priv->date:"");
		}
		fprintf(file,"timestamp,message\n");
		GSList *l = NULL;
		for(l=datas;l!=NULL;l=l->next)
		{
			if(0>fprintf(file,"%s,%s\n",market_db_get_date_string(mkt_log_changed(MKT_LOG(l->data))),mkt_log_message(MKT_LOG(l->data))))
				break;
			g_usleep(4000);
		}
		if(datas) mkt_slist_free_full(datas,g_object_unref);
		fclose(file);
	}
	g_free(path);
	g_free(filepath);
	g_timeout_add(10,save_log_data_async_done_callback,user_data);
}


gboolean
gl_log_save_log_clicked ( GtkWidget *widget ,gpointer user_data )
{
	gtk_widget_set_sensitive(widget,FALSE);
	mkt_model_runs_async ( MKT_TYPE_LOG_MESSAGE,0, __save_log_data_async, user_data);
	return TRUE;
}

gboolean
gl_log_show_error_log_clicked(GtkWidget *widget ,gpointer user_data)
{
	//TEST:g_debug("gl_log_show_error_log_clicked");
	g_return_val_if_fail(user_data!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_LOG(user_data),FALSE);
	GlLog *log = GL_LOG(user_data);
	if(log->priv->notebook && GTK_IS_NOTEBOOK(log->priv->notebook))
	{
		GtkWidget *widget = mkt_window_find_widget(MKT_WINDOW(log),"TimeDateLogBook");
		if(widget)gtk_widget_hide(widget);
		mkt_log_reload_errors_start(log);
		//const gchar *error_state = mkIget(history___errorState);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(log->priv->notebook),0);
	}
	return TRUE;
}

gboolean
gl_log_show_system_log_clicked(GtkWidget *widget ,gpointer user_data)
{
	//TEST:g_debug("gl_log_show_system_log_clicked");
	g_return_val_if_fail(user_data!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_LOG(user_data),FALSE);
	GlLog *log = GL_LOG(user_data);
	if(log->priv->notebook && GTK_IS_NOTEBOOK(log->priv->notebook))
	{
		GtkWidget *widget = mkt_window_find_widget(MKT_WINDOW(log),"TimeDateLogBook");
		if(widget)gtk_widget_show(widget);
		gtk_notebook_set_current_page(GTK_NOTEBOOK(log->priv->notebook),1);
	}
	return TRUE;
}



void
gl_log_system_sort_on_type_changed_signal_cb ( GtkWidget *widget , GlLog *log)
{
	g_return_if_fail(log != NULL );
	g_return_if_fail(GL_IS_LOG(log));
	g_return_if_fail(widget != NULL );
	g_return_if_fail(GTK_IS_COMBO_BOX(widget));
	log->priv->type_sort = gtk_combo_box_get_active(GTK_COMBO_BOX(widget))-1;
	gl_log_need_data_reload(log);
}

void
gl_log_system_set_date_clicked ( GtkWidget *widget , GlLog *log )
{
	g_return_if_fail(log != NULL );
	g_return_if_fail(GL_IS_LOG(log));
	g_return_if_fail(widget != NULL );
	g_return_if_fail(GTK_IS_BUTTON(widget));
	MktWindow *td_win = MKT_WINDOW(mkt_collector_get_atom_static ( "com.lar.GlDateTimeWindow.LogBookTiming"));
	mkt_window_show(td_win);
}


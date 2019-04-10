/*
 * gl-pligin-CalibrationPlotPlot.c
 *
 *  Created on: 05.04.2013
 *      Author: sascha
 */


#include "gl-user-question.h"



struct _GlUserQuestionPrivate
{
	gboolean  user_answer;

};


#define GL_USER_QUESTION_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GL_TYPE_USER_QUESTION, GlUserQuestionPrivate))




G_DEFINE_TYPE (GlUserQuestion, gl_user_question, MKT_TYPE_WINDOW);


enum
{
	USER_ANSWER,
	LAST_SIGNAL
};


static guint user_question[LAST_SIGNAL] = { 0 };

void
gl_user_question_start ( MktWindow *window , gpointer data )
{
	g_return_if_fail(window!=NULL);
	g_return_if_fail(GL_IS_USER_QUESTION(window));
	GlUserQuestion *uq = GL_USER_QUESTION(window);
	uq->priv->user_answer = FALSE;
}

static void
gl_user_question_init (GlUserQuestion *object)
{
	GlUserQuestionPrivate *priv = GL_USER_QUESTION_GET_PRIVATE(object);
	object->priv   = priv;
	object->priv->user_answer = FALSE;
	g_signal_connect(object,"window_show",G_CALLBACK(gl_user_question_start),NULL);

}

static void
gl_user_question_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */
	//GlUserQuestion *cal = GL_USER_QUESTION(object);
	G_OBJECT_CLASS (gl_user_question_parent_class)->finalize (object);
}

static void
gl_user_question_class_finalize (GlUserQuestionClass *object)
{
	/* TODO: Add deinitalization code here */
	g_warning("gl_plugin_test_class_finalize\n");
	//G_OBJECT_CLASS (gl_plugin_test_parent_class)->finalize (object);
}


static void
gl_user_question_expose ( MktWindow *window )
{
	GtkWidget* widget = mkt_window_find_widget(window,mkt_atom_get_id(MKT_ATOM(window)));
	if (widget != NULL)
	{
		//g_debug("Widget2 set color ");
		GdkColor color;
		gdk_color_parse ("yellow", &color);
		gtk_widget_modify_bg (widget, GTK_STATE_NORMAL, &color);
	}

}

static void
gl_user_question_class_init (GlUserQuestionClass *klass)
{
	GObjectClass*   object_class    = G_OBJECT_CLASS (klass);
	MktWindowClass* parent_class    = MKT_WINDOW_CLASS(klass);
	object_class->finalize          = gl_user_question_finalize;
	parent_class->expose            = gl_user_question_expose;

	g_type_class_add_private (klass, sizeof (GlUserQuestionPrivate));

	user_question[USER_ANSWER] = g_signal_new ("user-answered",
					G_TYPE_FROM_CLASS (klass),
					G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
					G_STRUCT_OFFSET ( GlUserQuestionClass, user_answered),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


void
gl_user_question_answer_ok_cb ( GtkWidget *widget ,GlUserQuestion *object)
{
	g_return_if_fail(widget!=NULL);
	g_return_if_fail(object!=NULL);
	g_return_if_fail(GL_IS_USER_QUESTION(object));
	object->priv->user_answer = TRUE;
	g_signal_emit(object,user_question[USER_ANSWER],0);
	mkt_window_hide(MKT_WINDOW(object));
}

void
gl_user_question_answer_cancel_cb ( GtkWidget *widget ,GlUserQuestion *object)
{
	g_return_if_fail(widget!=NULL);
	g_return_if_fail(object!=NULL);
	g_return_if_fail(GL_IS_USER_QUESTION(object));
	object->priv->user_answer = FALSE;
	g_signal_emit(object,user_question[USER_ANSWER],0);
	mkt_window_hide(MKT_WINDOW(object));
}


gboolean
gl_user_question_get_answer          ( GlUserQuestion *uq )
{
	g_return_val_if_fail(uq!=NULL,FALSE);
	g_return_val_if_fail(GL_IS_USER_QUESTION(uq),FALSE);
	return uq->priv->user_answer;
}

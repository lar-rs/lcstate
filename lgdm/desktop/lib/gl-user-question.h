/*
 * gl-user-question.h
 *
 *  Created on: 19.04.2013
 *      Author: sascha
 */



#ifndef GL_USER_QUESTION_H_
#define GL_USER_QUESTION_H_

#include "mkt-window.h"

#define GL_TYPE_USER_QUESTION             (gl_user_question_get_type ())
#define GL_USER_QUESTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_USER_QUESTION, GlUserQuestion))
#define GL_USER_QUESTION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_USER_QUESTION, GlUserQuestionClass))
#define GL_IS_USER_QUESTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_USER_QUESTION))
#define GL_IS_USER_QUESTION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_USER_QUESTION))
#define GL_USER_QUESTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_USER_QUESTION, GlUserQuestionClass))

typedef struct _GlUserQuestionClass  GlUserQuestionClass;
typedef struct _GlUserQuestion       GlUserQuestion;
typedef struct _GlUserQuestionPrivate GlUserQuestionPrivate;





struct _GlUserQuestionClass
{
	MktWindowClass          parent_class;
	void                  (*user_answered)       ( GlUserQuestion *uq );
};

struct _GlUserQuestion
{
	MktWindow                parent_instance;
	GlUserQuestionPrivate   *priv;

};


GType                   gl_user_question_get_type            ( void ) G_GNUC_CONST;

gboolean                gl_user_question_get_answer          ( GlUserQuestion *uq );

#endif /* GL_USER_QUESTION_H_ */

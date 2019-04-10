/*
 * gtklarcontrolbox.h
 *
 *  Created on: 12.10.2010
 *      Author: asmolkov
 */

#ifndef GTKLARCONTROLBOX_H_
#define GTKLARCONTROLBOX_H_

#include <gtk/gtk.h>
#include <glib.h>
#include "mkt-window.h"


G_BEGIN_DECLS


#define GL_CONTROL_BOX_TYPE				(gl_control_box_get_type())
#define GL_CONTROL_BOX(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj),GL_CONTROL_BOX_TYPE,  GlControlBox))
#define GL_CONTROL_BOX_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass) ,GL_CONTROL_BOX_TYPE,  GlControlBoxClass))
#define GL_IS_CONTROL_BOX(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj),GL_CONTROL_BOX_TYPE))
#define GL_IS_CONTROL_BOX_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass) ,GL_CONTROL_BOX_TYPE))

typedef struct _GlControlBox			GlControlBox;
typedef struct _GlControlBoxClass		GlControlBoxClass;
typedef struct _GlControlBoxPrivate     GlControlBoxPrivate;


typedef enum {
  CONTROL_BOX_OPEN_IMAGE,
  CONTROL_BOX_CLOSE_IMAGE,
  CONTROL_BOX_LEFT_IMAGE,
  CONTROL_BOX_RIGHT_IMAGE,
  CONTROL_BOX_LAST_IMAGE
}GlControBoxImageType;

typedef enum {
  CONTROL_BOX_HBOX_DIRECTION,
  CONTROL_BOX_VBOX_DIRECTION,
  CONTROL_BOX_LAST_DIRECTION,

}GlControBoxDirectionType;




typedef enum
{
	CONTROL_BOX_LAST_OPERATION_NULL,
	CONTROL_BOX_LAST_OPERATION_CLICKED,
	CONTROL_BOX_LAST_OPERATION_LAST



}GlControlBoxLastOperationWidget;

struct _GlControlBox
{
	MktWindow             mkt_window;
	GlControlBoxPrivate  *priv;
	GtkWidget            *window;
	GtkWidget            *viewport;
	GtkWidget            *mhbox;
	GtkWidget            *ochbox;
	GtkWidget            *ihbox;
	GtkWidget            *cbOC;
	GtkWidget            *inLink;
	GtkWidget            *inRecht;
	GtkWidget            *Image[CONTROL_BOX_LAST_IMAGE];

	GtkWidget            *lastSignal[CONTROL_BOX_LAST_OPERATION_LAST];

	GtkWidget            *keytaste;

	gint                  max_widget;

	GString              *openCB;
	GString              *closeCB;

	gint                  maxWidth;
	gint                  maxHigth;
	gint                  aButtonLen;
	gchar                *place;
	gboolean              can_close;
};

struct _GlControlBoxClass
{
	MktWindowClass              parent_class;
	void (*controlbox ) (GlControlBox *controlBox);

};

GtkType		        gl_control_box_get_type                 ( void );

MktAtom*	        gl_control_box_new_full                 ( const gchar *id, GlControBoxDirectionType type, guint x, guint y, guint width , guint height , guint pos );


//  --------------------  Image Options -------------------------------------------------------------------------
gboolean            gl_control_box_set_images_from_files    ( GlControlBox *ctrlbox,GlControBoxImageType _type,char *file );

//  -------------------- Control Widgets-------------------------------------------------------------------------

gboolean            gl_control_box_add_action               ( GlControlBox *ctrlbox,MktWindow *action,gboolean expand,gboolean fill,gint padding );
void                gl_control_box_set_max_size             ( GlControlBox *ctrlbox,gint width,gint higth );

gboolean            gl_control_box_refresh                  ( GlControlBox *ctrlbox );
gboolean            gl_control_box_close_refresh            ( GlControlBox *ctrlbox );

void                gl_control_box_can_close                ( GlControlBox *ctrlbox,gboolean can_close );
void                gl_control_box_set_max_widgets          ( GlControlBox *ctrlbox,gint  max );

gboolean            gl_control_box_close                    ( GlControlBox *ctrlbox );
gboolean            gl_control_box_open                     ( GlControlBox *ctrlbox );


G_END_DECLS


#endif /* GTKLARCONTROLBOX_H_ */

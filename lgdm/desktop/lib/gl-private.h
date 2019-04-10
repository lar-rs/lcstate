



#ifndef GUIPRIVATE_H_
#define GUIPRIVATE_H_

#include <glib.h>
#include "gl-system.h"




enum
{
	GUI_ITEMS_USER_CHANGED          = 1 << 1,
	GUI_ITEMS_CONNECTED             = 1 << 2,
	GUI_ITEMS_IS_XML_ID             = 1 << 3,
	GUI_ITEMS_IS_HAVE_IDLE          = 1 << 4,
	GUI_ITEMS_IS_TESTED             = 1 << 5,
	GUI_ITEMS_IS_HAS_RTV            = 1 << 6,
	GUI_ITEMS_IS_REF                = 1 << 7,
	GUI_ITEMS_IS_CALPAR             = 1 << 8,
	GUI_ITEMS_IS_TRANSLATION        = 1 << 9,

	GUI_ITEMS_HARDWARE              = 1 << 15,
};

enum
{
	GUI_SUBSCRIPT_LOCK              = 1 << 1,
};

extern  gboolean gui_private_ipc_stop;
extern  gboolean gui_private_ipc_wait;

extern  gboolean gui_private_all_control_disable;



/*
enum
{

	#define GL_EMPTY   		      0
#define GL_USER_CHANGED	        = 1 << 1
#define GL_ITEM_COMMING         = 1 << 2
#define GL_WIDGETS_NOT_FIND     1 << 3
#define GL_NOT_USED_5         8
#define GL_NOT_USED_6         16
#define GL_NOT_USED_7         32
#define GL_NOT_USED_8         64
#define GL_NOT_USED_9         128
#define GL_NOT_USED_10        256
#define GL_NOT_USED_11        512
#define GL_NOT_USED_12        1024
#define GL_NOT_USED_13        2048
#define GL_NOT_USED_14        4096
#define GL_NOT_USED_15        8192
#define GL_NOT_USED_16        16384
#define GL_NOT_USED_17        32768
#define GL_NOT_USED_18        65536
#define GL_NOT_USED_19        131072
#define GL_NOT_USED_20        262144
#define GL_NOT_USED_21        524288
*/


#define  GL_PRIVATE_DEFAULD_FIELD   "main_box"
/*
gboolean                   gl_private_set_user(GuiUserAutorization user);
GuiUserAutorization        gl_private_get_user();
gboolean                   gl_private_is_tru_user(GuiUserType user);
void                       gl_private_user_autorization_test();

gboolean                   gl_private_get_item_to_widget();
void                       gl_private_item_to_widget_on();
void                       gl_private_item_to_widget_off();

char*                      gl_private_get_autorisation_name();
char*                      gl_private_get_user_name        (GuiUserType user);
GuiUserType                gl_private_get_user_from_name   (char *user_name);
GuiUserType                gl_private_get_user_acces_type  (GuiPrivateUserType);
*/
#endif /*GUIPRIVATE_H_*/

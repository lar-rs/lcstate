/*
 * gui-private.c
 *
 *  Created on: 03.11.2010
 *      Author: asmolkov
 *      TODO: Auf GLib umsteigen (strcmp -> g_strcmp). ..
 */



#include "gl-private.h"
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>


gboolean gui_private_ipc_stop  = FALSE;
gboolean gui_private_ipc_wait  = FALSE;





// internal control
/*
gboolean gui_private_all_control_disable = FALSE;


static GuiUserAutorization __gui_system_user = GUI_USER_AUTORIZATION_DEVICE;

static gboolean __item_to_widget_entered = FALSE;


char*    gl_private_get_autorisation_name(GuiUserAutorization autorization)
{
	static gchar user_name[128];
	memset(user_name,0,128);
	switch(autorization)
	{
	case GUI_USER_AUTORIZATION_UNKNOWN  : g_stpcpy(user_name,"unknown") ;break;
	case GUI_USER_AUTORIZATION_DEVICE   : g_stpcpy(user_name,"device")  ;break;
	case GUI_USER_AUTORIZATION_OPERATOR : g_stpcpy(user_name,"operator");break;
	case GUI_USER_AUTORIZATION_SERVICE  : g_stpcpy(user_name,"service") ;break;
	case GUI_USER_AUTORIZATION_ROOT     : g_stpcpy(user_name,"root")    ;break;
	default                             : g_stpcpy(user_name,"unknown") ;break;

	}
	return user_name;
}

GuiUserType gl_private_get_user_from_name(char *user_name)
{
	if(!user_name) return GUI_USER_DEVICE;
	if(0==g_strcmp0(user_name,"operator"))     return GUI_USER_OPERATOR;
	else if(0==g_strcmp0(user_name,"service")) return GUI_USER_SERVICE;
	else if(0==g_strcmp0(user_name,"root"))    return GUI_USER_ROOT;

	return GUI_USER_DEVICE;
}

char*    gl_private_get_user_name(GuiUserType user)
{
	static char user_name[128];
	memset(user_name,0,128);
	switch(user)
	{
	case GUI_USER_DEVICE       : g_stpcpy(user_name,"device")  ;break;
	case GUI_USER_OPERATOR     : g_stpcpy(user_name,"operator");break;
	case GUI_USER_SERVICE      : g_stpcpy(user_name,"service") ;break;
	case GUI_USER_ROOT         : g_stpcpy(user_name,"root")    ;break;
	default                    : g_stpcpy(user_name,"unknown") ;break;

	}
	return user_name;
}

gboolean gl_private_get_item_to_widget()
{
	return __item_to_widget_entered;
}


void gl_private_item_to_widget_on()
{
	__item_to_widget_entered = TRUE;
}
void gl_private_item_to_widget_off()
{
	__item_to_widget_entered = FALSE;
}

gboolean  gl_private_set_user(GuiUserAutorization user)
{
	__gui_system_user = user;
	return TRUE;
}
GuiUserAutorization  gl_private_get_user()
{
	return __gui_system_user;

}


gboolean  gl_private_is_tru_user(GuiUserType user)
{
	if(user &  gl_private_get_user()) return TRUE;

	return FALSE;
}


GuiUserType  gl_private_get_user_acces_type(GuiPrivateUserType utype)
{
	switch(utype)
	{
		case GUI_USER_DEVICE_TYPE       : return GUI_USER_DEVICE;
		case GUI_USER_OPERATOR_TYPE     : return GUI_USER_OPERATOR;
		case GUI_USER_SERVICE_TYPE		: return GUI_USER_SERVICE;
		case GUI_USER_ROOT_TYPE    		: return GUI_USER_ROOT;
		default                    		: return 0;

	}


}


void gui_private_user_autorization_test()
{
	/*int userNub     = 0;userNub     |= GUI_USER_AUTORIZATION_NUB;
	int userService = 0;userService |= GUI_USER_AUTORIZATION_SERVICE;
	int userRoot    = 0;userRoot    |= GUI_USER_AUTORIZATION_ROOT;


	printf("Un=%d,Us=%d,Ur=%d\n",GUI_USER_NUB,GUI_USER_SERVICE,GUI_USER_ROOT);
	printf("Nub=%d,Service=%d,Root=%d\n",GUI_USER_AUTORIZATION_NUB,GUI_USER_AUTORIZATION_SERVICE,GUI_USER_AUTORIZATION_ROOT);
	if(GUI_USER_NUB     & userNub) printf("NUB-NUB OK\n");else printf("NUB-NUB ERR\n");
	if(GUI_USER_SERVICE & userNub) printf("NUB-Service ERR\n");else printf("NUB-Service OK\n");
	if(GUI_USER_ROOT    & userNub) printf("NUB-ROOT ERR\n");else printf("NUB-ROOT OK\n");

	if(GUI_USER_NUB & userService) printf("Service NUB OK\n");else printf("Service NUB ERR\n");
	if(GUI_USER_SERVICE & userService) printf("Service-Service OK\n");else printf("Service-Service ERR\n");
	if(GUI_USER_ROOT & userService) printf("Service-ROOT ERR\n");else printf("Service-ROOT OK\n");

	if(GUI_USER_NUB & userRoot) printf("ROOT -NUB OK\n");else printf("ROOT- NUB ERR\n");
	if(GUI_USER_SERVICE & userRoot) printf("ROOT-Service OK\n");else printf("ROOT-Service ERR\n");
	if(GUI_USER_ROOT & userRoot) printf("ROOT-ROOT OK\n");else printf("ROOT-ROOT ERR\n");*/
//}


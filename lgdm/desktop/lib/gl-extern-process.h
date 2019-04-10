/*
 * gui-extern-process.c
 *
 *  Created on: 20.01.2011
 *      Author: asmolkov
 */

#ifndef GUIEXTERNPROCESS_C_
#define GUIEXTERNPROCESS_C_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include "gtk/gtk.h"




typedef struct
{
	char  name[256];
	char  program[256];
	pid_t pid;
}GuiExternProcess_t;



gboolean gui_extern_process_is_exec(char *name);
gboolean gui_extern_process_init(char *name,char *program);
gboolean gui_extern_process_stop(char *name);
pid_t    gui_extern_process_start(char *name,char *ar, ...);
gboolean gui_extern_process_delete_all();

GuiExternProcess_t* gui_extern_process_get_process(char *name);

gboolean            gui_extern_process_delete_all();





#endif /* GUIEXTERNPROCESS_C_ */

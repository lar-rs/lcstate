/*
 * gui-extern-process.c
 *
 *  Created on: 20.01.2011
 *      Author: asmolkov
 */


#include "gl-extern-process.h"
#include <string.h>


GList *GuiExternP = NULL;


GuiExternProcess_t*
gui_extern_process_get_process(char *name)
{
	GList *curr = GuiExternP;
	if(name == NULL) return NULL;
	while(curr != NULL)
	{
		GuiExternProcess_t *process = (GuiExternProcess_t *) curr->data;
		if(NULL != process)
		{
			if( (0== strcmp(process->name,name)))
			{
				return process;
			}
		}
		curr = curr->next;
	}
	return NULL;
}
gboolean
gui_extern_process_is_run(char *name)
{
	GList *curr = GuiExternP;
	if(name == NULL) return FALSE;
	while(curr != NULL)
	{
		GuiExternProcess_t *process = (GuiExternProcess_t *) curr->data;
		if(NULL != process)
		{
			if( (0== strcmp(process->name,name)) && (process->pid != 0) )
			{
				return TRUE;
			}
		}
		curr = curr->next;
	}
	return FALSE;
}

gboolean
gui_extern_process_delete_all()
{
	GList *curr = GuiExternP;
	while(curr != NULL)
	{
		GuiExternProcess_t *process = (GuiExternProcess_t *) curr->data;
		if(NULL != process)
		{
			if((process->pid != 0) )
			{
				gui_extern_process_stop(process->name);
			}
		}
		curr = curr->next;
	}
	curr = GuiExternP;
	while(curr != NULL)
	{
		if(curr->data != NULL)g_free(curr->data);
		curr = curr->next;
	}
	g_list_free(GuiExternP);
	GuiExternP = NULL;
	return TRUE;
}


gboolean
gui_extern_process_is_exec(char *name)
{
	GList *curr = GuiExternP;
	if(name == NULL) return FALSE;
	while(curr != NULL)
	{
		GuiExternProcess_t *process = (GuiExternProcess_t *) curr->data;
		if(NULL != process)
		{
			if( (0== strcmp(process->name,name)) )
			{
				return TRUE;
			}
		}
		curr = curr->next;
	}
	return FALSE;
}


gboolean
gui_extern_process_init(char *name,char *program)
{
	if(name == NULL)    return FALSE;
	if(program == NULL) return FALSE;
	GuiExternProcess_t *process  = NULL;
	if(!gui_extern_process_is_exec(name))
	{
		process  = g_malloc0(sizeof(GuiExternProcess_t));
		if(process == NULL) return FALSE;
		memset(process->name,0,256);
		strcpy(process->name,name );
		memset(process->program,0,256);
		strcpy(process->program,program);
	}
	else
	{
		process = gui_extern_process_get_process(name);
		if(process == NULL) return FALSE;
		memset(process->program,0,256);
		strcpy(process->program,program);
	}
	GuiExternP = g_list_append(GuiExternP,process);
	return TRUE;
}

gboolean
gui_extern_process_stop(char *name)
{
	if(name == NULL) return FALSE;
	GuiExternProcess_t *process = gui_extern_process_get_process(name);
	if(process == NULL) return FALSE;
	if(process-> pid == 0) return FALSE;
	int status;
	if(!kill(	process->pid , SIGKILL))
	{
		waitpid(process->pid,&status,0);
		process->pid = 0;
	}
	else
	{
		process->pid = 0;
		fprintf(stderr,"an error occurred in kill( process:%s pid=%d;signal:SIGTERM);\n",process->name,process->pid);
		return FALSE;
	}
	return TRUE;

}
pid_t
gui_extern_process_start(char *name,char *ar, ...)
{
	if(name == NULL) return FALSE;
	if(gui_extern_process_is_run(name)) return FALSE;
	char **argv = &ar;
	printf("gui_extern_process_start\n");
	GuiExternProcess_t *process = gui_extern_process_get_process(name);
	int coun = 0;
	for(;argv[coun] != NULL;coun++)printf("TEST:%s\n",argv[coun]);
	if(process == NULL) return FALSE;
	printf("gui_extern_process_start 22\n");
	process->pid = fork();
	if(process->pid != 0)
	{
		printf("Start programm %s pid %d\n",process->name,process->pid);
		return process->pid;
	}
	else
	{
		execvp(process->program,argv);
		abort();
	}
	return process->pid;
}







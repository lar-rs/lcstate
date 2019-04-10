/*
 * gui-control.c
 *
 *  Created on: 28.03.2011
 *      Author: asmolkov
 */



#include "gl-control.h"

#include <string.h>

GList             *__all_operation__   = NULL;
GlOperations_t    *__last_operation__  = NULL;


static gboolean
___internal_operation_run(gpointer data)
{
	if(data == NULL) return FALSE;
	GlOperations_t *op = (GlOperations_t*) data;
	//TEST:g_debug("___internal_operation_run %s",op->name);
	gboolean not_done = FALSE;
	GList *curr = op -> op_func;
	int count = 0;
	while(curr != NULL)
	{
		if(count > 1000) return FALSE;
		if( curr -> data != NULL)
		{
			GuiControlFunk_t* op_func = (GuiControlFunk_t*) curr -> data;
			if(!op_func -> done )
			{
				op_func -> done = op_func -> func (op->data);
				not_done = TRUE;
				break;
			}
		}
		count++;
		curr = curr -> next;
	}
	return not_done;
}

static void
___internal_operation_done(gpointer data)
{
	//TEST:printf("___internal_operation_done \n");
	if(data != NULL)
	{
		GlOperations_t *op = (GlOperations_t*) data;
		GList *curr = op -> op_func_stop;
		int count = 0;
		while(curr != NULL)
		{
			if(count > 1000) return ;
			if( curr -> data != NULL)
			{
				GuiControlFunk_t* op_func_stop = (GuiControlFunk_t*) curr -> data;
				if(!op_func_stop -> done )
				{
					op_func_stop -> func (op->data);
				}
			}
			count++;
			curr = curr -> next;
		}
		op -> done = TRUE;
		op -> run  = FALSE;
		op -> tag  = 0;
	}
	printf("___internal_operation_done end\n");
}


GlOperations_t*
gl_control_operation_get_operation( char *name)
{
	//TEST:g_debug ("get operation %s start",name);
	GList *curr = __all_operation__;
	while(curr != NULL)
	{
		if(curr -> data != NULL)
		{
			GlOperations_t *op = (GlOperations_t*) curr->data;
			if(op->name != NULL )
			{
				if(0==g_strcmp0(op->name,name)){  return op; }
			}
		}
		curr = curr -> next;
	}
	//TEST:g_debug ("get operation %s end NULL",name);
	return NULL;
}

gboolean
gl_control_operations_is_run()
{
	GList *curr = __all_operation__;
	while(curr != NULL)
	{
		if(curr -> data != NULL)
		{
			GlOperations_t *op = (GlOperations_t*) curr->data;
			if(op->run )return TRUE;
		}
		curr = curr -> next;
	}
	return FALSE;
}

gboolean
gl_control_operation_name_is_run(char *name)
{
	GList *curr = __all_operation__;
	while(curr != NULL)
	{
		if(curr -> data != NULL)
		{
			GlOperations_t *op = (GlOperations_t*) curr->data;
			if(op->name != NULL )
			{
				if(0==strcmp(op->name,name)) return op->run;
			}
		}
		curr = curr -> next;
	}
	return FALSE;
}

gboolean
gl_control_operation_name_is_done(char *name)
{
	GList *curr = __all_operation__;
	while(curr != NULL)
	{
		if(curr -> data != NULL)
		{
			GlOperations_t *op = (GlOperations_t*) curr->data;
			if(op->name != NULL )
			{
				if(0==strcmp(op->name,name)) return op->done;
			}
		}
		curr = curr -> next;
	}
	return FALSE;
}

gboolean
gl_control_operation_stop_last()
{
	if(__last_operation__)
	{
		gl_control_operation_stop(__last_operation__);
	}
	return TRUE;
}

gboolean
gl_control_operation_stop_all ()
{
	GList *curr = __all_operation__;
	while(curr != NULL)
	{
		if(curr -> data != NULL)
		{
			GlOperations_t *op = (GlOperations_t*) curr->data;
			gl_control_operation_stop(op);
		}
		curr = curr -> next;
	}
	return TRUE;
}

gboolean
gl_control_operation_name_stop    (char *name)
{
	GList *curr = __all_operation__;
	while(curr != NULL)
	{
		if(curr -> data != NULL)
		{
			GlOperations_t *op = (GlOperations_t*) curr->data;
			if(op->name == NULL )
			{
				if(0==strcmp(op->name,name))
				{
					gl_control_operation_stop(op);
					return TRUE;
				}
			}
		}
		curr = curr -> next;
	}
	return FALSE;
}

gboolean
gl_control_operation_name_run     (char *name)
{
	GList *curr = __all_operation__;
	while(curr != NULL)
	{
		if(curr -> data != NULL)
		{
			GlOperations_t *op = (GlOperations_t*) curr->data;
			if(op->name == NULL )return FALSE;
			if(0==strcmp(op->name,name))
			{
				gl_control_operation_run(op);
				return TRUE;
			}
		}
		curr = curr -> next;
	}
	return FALSE;

}


GlOperations_t*
gl_control_operation_new(char *name,gpointer data)
{
	//TEST:g_debug ("new operation %s start",name);
	GlOperations_t *op = NULL;
	if(!(op = gl_control_operation_get_operation(name)))
	{
		op = g_malloc(sizeof(GlOperations_t));
		op -> done    = FALSE;
		op -> name    = NULL;
		op -> run     = FALSE;
		op -> op_func = NULL;
		op -> op_func_stop = NULL;
		op -> data    = data;
		op -> tag     = 0;
		op -> timeout = 100;

		if(name) op -> name = g_strdup(name);
		__all_operation__  = g_list_append(__all_operation__,op);
	}
	else
	{
		op -> done    = FALSE;
		op -> run     = FALSE;
		op -> data    = data;
	}
	//TEST:g_debug ("new operation %s end",name);
	return op;
}

gboolean
gl_control_operation_add_run_func(GlOperations_t *op,GtkFunction func)
{
	if(func == NULL) return FALSE;
	GuiControlFunk_t *fg = NULL;
	fg = g_malloc(sizeof(GuiControlFunk_t));
	fg -> func  = func;
	fg -> done  = FALSE;
	fg -> run   = FALSE;
	op -> op_func = g_list_append(op -> op_func,fg);
	return TRUE;
}

gboolean
gl_control_operation_add_stop_func(GlOperations_t *op,GtkFunction func)
{
	if(func == NULL) return FALSE;
	GuiControlFunk_t *fg = NULL;
	fg = g_malloc(sizeof(GuiControlFunk_t));
	fg -> func  = func;
	fg -> done  = FALSE;
	fg -> run   = FALSE;
	op -> op_func_stop = g_list_append(op -> op_func_stop,fg);
	return TRUE;
}

void
gl_control_operation_free(GlOperations_t *op)
{
	if(op == __last_operation__) __last_operation__ = NULL;
	gl_control_operation_stop(op);
	GList *curr = op -> op_func;
	while(curr != NULL)
	{
		if(curr -> data != NULL)
			g_free(curr -> data);
		curr = curr -> next;
	}
	g_list_free(op -> op_func); op -> op_func = NULL;
	curr = op -> op_func_stop;
	while(curr != NULL)
	{
		if(curr -> data != NULL)
			g_free(curr -> data);
		curr = curr -> next;
	}
	g_list_free(op -> op_func_stop); op -> op_func_stop = NULL;
	GList *del = NULL;
	curr = __all_operation__;
	while(curr != NULL)
	{
		if(curr -> data == op)
		{
			del = curr;
		}
		curr = curr -> next;
	}
	if(del != NULL)__all_operation__ = 	g_list_delete_link(__all_operation__,del);
	if(op -> name != NULL) g_free(op -> name);
	g_free(op);

}

gboolean
gl_control_operation_stop ( GlOperations_t *op )
{
	if( op == NULL) return FALSE;
	//TEST:g_debug("Stop op %s",op->name);
	if(op->tag)	gtk_timeout_remove (op->tag);
	op->tag = 0;
	return TRUE;
}

gboolean
gl_control_operation_run ( GlOperations_t *op )
{
	if( op == NULL) return FALSE;
	gl_control_operation_stop(op);
	__last_operation__ = op;
	GList *curr = op -> op_func;
	while(curr != NULL)
	{
		if( curr -> data != NULL)
		{
			GuiControlFunk_t* op_func = (GuiControlFunk_t*) curr -> data;
			op_func -> done  = FALSE;
			op_func -> run   = FALSE;
		}
		curr = curr -> next;
	}
	op->tag  = gtk_timeout_add_full(op->timeout,___internal_operation_run,NULL,op,
			                                    ___internal_operation_done);
	return TRUE;
}

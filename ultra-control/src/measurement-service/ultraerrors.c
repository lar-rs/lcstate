/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ultraerrors.c
 * Copyright (C) LAR 2017
 *

 */


#include <glib.h>
#include "ultraerrors.h"



GQuark UltraErrorsQuark( )
{
	static GQuark error;
	if (!error)
		error = g_quark_from_static_string("ultra-error");
	return error;
}
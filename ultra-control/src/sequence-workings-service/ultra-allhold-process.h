/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mkt-measurement-data.c
 * Copyright (C) sascha 2013 <sascha@sascha-VirtualBox>
 * 
mkt-measurement-data.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * mkt-measurement-data.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __ULTRA_PROCESS_ALLHOLD_H_
#define __ULTRA_PROCESS_ALLHOLD_H_


#include <mktlib.h>
#include <mktbus.h>
#include <ultimate-library.h>
#include "ultra-sequence-process.h"


G_BEGIN_DECLS



#define ULTRA_TYPE_ALLHOLD_PROCESS             (ultra_allhold_process_get_type ())
#define ULTRA_ALLHOLD_PROCESS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_ALLHOLD_PROCESS, UltraAllholdProcess))
#define ULTRA_ALLHOLD_PROCESS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  ULTRA_TYPE_ALLHOLD_PROCESS, UltraAllholdProcessClass))
#define ULTRA_IS_ALLHOLD_PROCESS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_ALLHOLD_PROCESS))
#define ULTRA_IS_ALLHOLD_PROCESS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  ULTRA_TYPE_ALLHOLD_PROCESS))
#define ULTRA_ALLHOLD_PROCESS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  ULTRA_TYPE_ALLHOLD_PROCESS, UltraAllholdProcessClass))

typedef struct _UltraAllholdProcessClass         UltraAllholdProcessClass;
typedef struct _UltraAllholdProcess              UltraAllholdProcess;
typedef struct _UltraAllholdProcessPrivate       UltraAllholdProcessPrivate;


struct _UltraAllholdProcessClass
{
	UltraSequenceProcessClass                                   parent_class;

};

struct _UltraAllholdProcess
{
	UltraSequenceProcess                                        parent_instance;
	UltraAllholdProcessPrivate                                  *priv;
};

GType                   ultra_allhold_process_get_type                 ( void ) G_GNUC_CONST;


G_END_DECLS

#endif /* _ULTRA_HARDWARE_OBJECT_H_ */

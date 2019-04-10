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

#ifndef __ULTRA_CONTROL_APP_OBJECT_H_
#define __ULTRA_CONTROL_APP_OBJECT_H_


#include <mktlib.h>
#include <mktbus.h>


G_BEGIN_DECLS



void                     node_control_app_new                                    (  GDBusConnection *connection  );

GDBusObjectManager*      node_control_app_device_manager                         ( void );
GDBusObjectManager*      node_control_app_nodes_manager                          ( void );


G_END_DECLS

#endif /* _ULTRA_CONTROL_APP_OBJECT_H_ */

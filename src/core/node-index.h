/*
 * node-index.h
 * Copyright (C) 2019
 *
 */

#ifndef _NODE_INDEX_H_
#define _NODE_INDEX_H_

#include <glib-object.h>

#include "node-device-object.h"



typedef struct _NodeValue NodeValue;



NodeValue*  node_value_read        (guint id, guint index, guint subindex );
gboolean    node_value_write_uint  (guint value);

#endif


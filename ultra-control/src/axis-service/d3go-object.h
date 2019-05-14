/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @defgroup XYSystem
 * @ingroup  XYSystem
 * @{
 * @file  D3GO-object.h	D3GO object header
 * @brief This is D3GO object header file.
 * 	 Copyright (C) LAR 2016
 *
 * @author A.Smolkov  <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */
#ifndef _D3GO_OBJECT_H_
#define _D3GO_OBJECT_H_

#include "move-object.h"
#include <mktbus.h>
#include <mktlib.h>

G_BEGIN_DECLS

#define D3GO_TYPE (D3GO_get_type())
#define D3GO_OBJECT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), D3GO_TYPE, D3GO))
#define D3GO_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), D3GO_TYPE, D3GOClass))
#define D3GO_IS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), D3GO_TYPE))
#define D3GO_IS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), D3GO_TYPE))
#define D3GO_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), D3GO_TYPE, D3GOClass))

typedef struct _D3GOClass   D3GOClass;
typedef struct _D3GO        D3GO;
typedef struct _D3GOPrivate D3GOPrivate;

struct _D3GOClass {
    MOVEClass parent_class;
};

struct _D3GO {
    MOVE         parent_instance;
    D3GOPrivate *priv;
};

GType D3GO_get_type(void) G_GNUC_CONST;

MOVE *d3go_new(AxisObject *axis, guint part, guint to_pos, guint move_parameter, guint repeat);
G_END_DECLS

#endif /* _D3GO_OBJECT_H_ */
/** @} */

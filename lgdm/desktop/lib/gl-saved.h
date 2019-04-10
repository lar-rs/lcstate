/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/**
 * @defgroup GlLibrary
 * @defgroup GlSaved
 * @ingroup  GlSaved
 * @{
 * @file  mkt-saved.h	Saved interface model header
 * @brief This is Saved interface model object header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 * $Id: $ $URL: $
 */
#ifndef GL_SAVED_H_
#define GL_SAVED_H_


#include "mkt-model.h"

G_BEGIN_DECLS

#define GL_TYPE_SAVED                  (gl_saved_get_type ())
#define GL_SAVED(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj),GL_TYPE_SAVED, GlSaved))
#define GL_IS_SAVED(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj),GL_TYPE_SAVED))
#define GL_SAVED_GET_INTERFACE(inst)   (G_TYPE_INSTANCE_GET_INTERFACE ((inst), GL_TYPE_SAVED, GlSavedInterface))


typedef struct _GlSavedInterface GlSavedInterface;
typedef struct _GlSaved GlSaved;


struct _GlSavedInterface
{
	GTypeInterface parent_iface;
	const gchar*            (*saved_path)                           ( GlSaved *self );
	const gchar*            (*saved_window)                         ( GlSaved *self );
	const gchar*            (*saved_widget)                         ( GlSaved *self );
};

GType                   gl_saved_get_type                          (void) G_GNUC_CONST;

const gchar*            gl_saved_get_path                          ( GlSaved *saved );
const gchar*            gl_saved_get_window                        ( GlSaved *saved );
const gchar*            gl_saved_get_widget                        ( GlSaved *saved );




G_END_DECLS

#endif /* GL_SAVED_H_ */

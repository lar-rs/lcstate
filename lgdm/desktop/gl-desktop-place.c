/*
 * @ingroup GlDesktopPlace
 * @{
 * @file  gl-desktop-place.c	generated object file
 * @brief generated object file
 *
 *
 *  Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "gl-desktop-place.h"
#include "gl-desktop-action.h"

#include "../config.h"
#include <glib/gi18n-lib.h>

// static GlDesktopPlace *__gui_process_desktop = NULL;

struct _GlDesktopPlacePrivate {
  GtkGrid *noob_actions;
  guint level;

  GHashTable *actions;
};

enum {
  GL_DESKTOP_PLACE_PROP_NULL,
  GL_DESKTOP_PLACE_PROP_LEVEL,

};

enum { GL_DESKTOP_PLACE_LAST_SIGNAL };

// static guint gl_desktop_place_signals[GL_DESKTOP_PLACE_LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE(GlDesktopPlace, gl_desktop_place, GTK_TYPE_BOX);

static void gl_desktop_place_change_level(GlDesktopPlace *desktop) {}

static void gl_desktop_place_init(GlDesktopPlace *gl_desktop_place) {
  g_return_if_fail(gl_desktop_place != NULL);
  g_return_if_fail(GL_IS_DESKTOP_PLACE(gl_desktop_place));
  gl_desktop_place->priv =
      gl_desktop_place_get_instance_private(gl_desktop_place);
  gtk_widget_init_template(GTK_WIDGET(gl_desktop_place));
  gl_desktop_place->priv->level = 0;
  gl_desktop_place->priv->actions = g_hash_table_new(g_str_hash, g_str_equal);
}

static void gl_desktop_place_finalize(GObject *object) {
  GlDesktopPlace *gl_desktop_place = GL_DESKTOP_PLACE(object);
  if (gl_desktop_place->priv->actions)
    g_hash_table_destroy(gl_desktop_place->priv->actions);

  G_OBJECT_CLASS(gl_desktop_place_parent_class)->finalize(object);
}

static void gl_desktop_place_set_property(GObject *object, guint prop_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {
  g_return_if_fail(GL_IS_DESKTOP_PLACE(object));
  GlDesktopPlace *gl_desktop_place = GL_DESKTOP_PLACE(object);
  switch (prop_id) {
  case GL_DESKTOP_PLACE_PROP_LEVEL:
    gl_desktop_place->priv->level = g_value_get_uint(value);
    gl_desktop_place_change_level(gl_desktop_place);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void gl_desktop_place_get_property(GObject *object, guint prop_id,
                                          GValue *value, GParamSpec *pspec) {
  g_return_if_fail(GL_IS_DESKTOP_PLACE(object));
  GlDesktopPlace *gl_desktop_place = GL_DESKTOP_PLACE(object);
  switch (prop_id) {
  case GL_DESKTOP_PLACE_PROP_LEVEL:
    g_value_set_uint(value, gl_desktop_place->priv->level);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void gl_desktop_place_class_init(GlDesktopPlaceClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
  object_class->finalize = gl_desktop_place_finalize;
  object_class->set_property = gl_desktop_place_set_property;
  object_class->get_property = gl_desktop_place_get_property;

  gtk_widget_class_set_template_from_resource(
      widget_class, "/lgdm/ui/layout/desktop-actions.ui");
  gtk_widget_class_bind_template_child_private(widget_class, GlDesktopPlace,
                                               noob_actions);

  g_object_class_install_property(
      object_class, GL_DESKTOP_PLACE_PROP_LEVEL,
      g_param_spec_uint("desktop-level", "Desktop level", "Desktop level", 0, 4,
                        0, G_PARAM_WRITABLE | G_PARAM_READABLE));
  /*
  gtk_widget_class_bind_template_child_private (widget_class, GlDesktopPlace,
  example_object);
  gtk_widget_class_bind_template_callback (widget_class,
  example_signal_callback);*/
}

gboolean gl_desktop_place_check_action(GlDesktopPlace *desktop,
                                       const gchar *action_name) {
  gpointer action_ptr =
      g_hash_table_lookup(desktop->priv->actions, (gconstpointer)action_name);
  return action_ptr != NULL;
}

static gboolean desktop_place_add_to_noobs(GlDesktopPlace *desktop,
                                           GtkWidget *action) {
  gint top = 0;
  gint left = 0;
  for (top = 0; top < 20; top++) {
    for (left = 0; left < 3; left++) {
      GtkWidget *widget =
          gtk_grid_get_child_at(desktop->priv->noob_actions, left, top);
      if (widget == NULL) {
        gtk_grid_attach(desktop->priv->noob_actions, action, left, top, 1, 1);
        return TRUE;
      }
    }
  }
  return FALSE;
}

gboolean gl_desktop_place_add_action(GlDesktopPlace *desktop, GtkWidget *action,
                                     guint x_pos, guint y_pos, guint level) {
  return desktop_place_add_to_noobs(desktop, action);
}

/** @} */

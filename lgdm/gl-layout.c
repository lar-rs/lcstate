/*
 * file  gl-desktop-action.c	LGDM desktop action button
 * brief LGDM desktop action button.
 * Copyright (C) LAR 2014-2019
 *
 * author A.Smolkov <asmolkov@lar.com>
 *
 */

#include "gl-layout.h"

#include "gl-layout-manager.h"
#include <glib.h>
#include <glib/gnode.h>
#include <mktbus.h>
#include <mktlib.h>
#include <string.h>

#include "../config.h"
#include <glib/gi18n-lib.h>

// static GlLayout *__gui_process_desktop = NULL;

struct _GlLayoutPrivate {
  gchar *id;
  gchar *name;
  guint close_level;
  gboolean activate;
  GlLayout *parent;
  GtkWidget *layout_manager;
  GList *child_layouts;
  GList *contaiments_layouts;
  GList *child_dialogs;
  guint last_state;
};

enum {
  GL_LAYOUT_PROP_NULL,
  GL_LAYOUT_ID,
  GL_LAYOUT_NAME,
  GL_LAYOUT_MENU,
  GL_LAYOUT_MANAGER,
  GL_LAYOUT_PARENT,
};

enum {
  GL_LAYOUT_ADD_MANAGER,
  GL_LAYOUT_ACTIVATED,
  GL_LAYOUT_STATE,
  GL_LAYOUT_LAST_SIGNAL
};

static guint gl_layout_signals[GL_LAYOUT_LAST_SIGNAL] = {0};

G_DEFINE_TYPE_WITH_PRIVATE(GlLayout, gl_layout, GTK_TYPE_SCROLLED_WINDOW);

static void gl_layout_init(GlLayout *layout) {
  g_return_if_fail(layout != NULL);
  g_return_if_fail(GL_IS_LAYOUT(layout));
  layout->priv = gl_layout_get_instance_private(layout);
  layout->priv->id = g_strdup("com.lar.Layout.Unknown");
  layout->priv->name = NULL;
  layout->priv->parent = NULL;
  layout->priv->layout_manager = NULL;
  layout->priv->contaiments_layouts = NULL;
  layout->priv->child_layouts = NULL;
  layout->priv->close_level = 0;
}

static void gl_layout_finalize(GObject *object) {
  GlLayout *layout = GL_LAYOUT(object);
  if (layout->priv->id)
    g_free(layout->priv->id);
  if (layout->priv->name)
    g_free(layout->priv->name);
  // if(layout->priv->child_dialogs)
  // g_list_free_full(layout->priv->child_dialogs,(GDestroyNotify)
  // gtk_widget_destroy);
  if (layout->priv->child_dialogs)
    g_list_free(layout->priv->child_dialogs);
  if (layout->priv->child_layouts)
    g_list_free(layout->priv->child_layouts);
  G_OBJECT_CLASS(gl_layout_parent_class)->finalize(object);
}

static void gl_layout_activate_real(GlLayout *layout) {
  //	g_debug("Activate real layout %s",layout->priv->id);
}

static void gl_layout_deactivate_real(GlLayout *layout) {
  //	g_debug("Deactivate layout %s",layout->priv->id);
}

static void destroy_child_layout(GlLayout *child, GlLayout *layout) {
  if (layout->priv->child_layouts != NULL) {
    layout->priv->child_layouts =
        g_list_remove(layout->priv->child_layouts, child);
  }
}

static void gl_layout_insert_childs_layout(GlLayout *layout, GlLayout *child) {
  layout->priv->child_layouts =
      g_list_append(layout->priv->child_layouts, child);
  g_signal_connect(child, "destroy", G_CALLBACK(destroy_child_layout), layout);
}

static void gl_layout_set_property(GObject *object, guint prop_id,
                                   const GValue *value, GParamSpec *pspec) {
  g_return_if_fail(GL_IS_LAYOUT(object));
  GlLayout *layout = GL_LAYOUT(object);
  switch (prop_id) {
  case GL_LAYOUT_ID:
    if (layout->priv->id)
      g_free(layout->priv->id);
    layout->priv->id = g_value_dup_string(value);
    break;
  case GL_LAYOUT_NAME:
    if (layout->priv->name)
      g_free(layout->priv->name);
    layout->priv->name = g_value_dup_string(value);
    break;
  case GL_LAYOUT_MANAGER:
    layout->priv->layout_manager = g_value_get_object(value);
    // g_signal_emit (layout, gl_layout_signals[GL_LAYOUT_ADD_MANAGER],
    // 0,layout->priv->layout_manager);
    break;
  case GL_LAYOUT_PARENT:
    layout->priv->parent = g_value_get_object(value);
    if (layout->priv->parent) {
      if (!gtk_widget_get_parent(GTK_WIDGET(layout)))
        gl_layout_manager_default_add_layout(layout);
      gl_layout_insert_childs_layout(layout->priv->parent, layout);
    }
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void gl_layout_get_property(GObject *object, guint prop_id,
                                   GValue *value, GParamSpec *pspec) {
  g_return_if_fail(GL_IS_LAYOUT(object));
  GlLayout *layout = GL_LAYOUT(object);
  switch (prop_id) {
  case GL_LAYOUT_ID:
    g_value_set_string(value, layout->priv->id);
    break;
  case GL_LAYOUT_NAME:
    g_value_set_string(value, layout->priv->name);
    break;
  case GL_LAYOUT_MANAGER:
    g_value_set_object(value, layout->priv->layout_manager);
    break;
  case GL_LAYOUT_PARENT:
    g_value_set_object(value, layout->priv->parent);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void gl_layout_class_init(GlLayoutClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  // GtkWidgetClass       *widget_class     =  GTK_WIDGET_CLASS (klass);
  object_class->finalize = gl_layout_finalize;
  object_class->set_property = gl_layout_set_property;
  object_class->get_property = gl_layout_get_property;
  klass->activate = gl_layout_activate_real;
  klass->deactivate = gl_layout_deactivate_real;

  g_object_class_install_property(
      object_class, GL_LAYOUT_ID,
      g_param_spec_string(
          "layout-id", "Layout id", "Layout identification name", "Application",
          G_PARAM_WRITABLE | G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property(
      object_class, GL_LAYOUT_NAME,
      g_param_spec_string("layout-name", "Layout user display name",
                          "Layout user display name", "Unknown name",
                          G_PARAM_WRITABLE | G_PARAM_READABLE));
  g_object_class_install_property(
      object_class, GL_LAYOUT_MENU,
      g_param_spec_object("layout-menu", "Layout menu info", "Layout menu info",
                          GTK_TYPE_MENU, G_PARAM_WRITABLE | G_PARAM_READABLE));
  g_object_class_install_property(
      object_class, GL_LAYOUT_MANAGER,
      g_param_spec_object("layout-manager", "Layout manager object",
                          "Layout manager object", GL_TYPE_LAYOUT_MANAGER,
                          G_PARAM_WRITABLE | G_PARAM_READABLE));
  g_object_class_install_property(
      object_class, GL_LAYOUT_PARENT,
      g_param_spec_object("layout-parent", "Layout parent object",
                          "Layout parent object", GL_TYPE_LAYOUT,
                          G_PARAM_WRITABLE | G_PARAM_READABLE |
                              G_PARAM_CONSTRUCT));
  gl_layout_signals[GL_LAYOUT_ADD_MANAGER] = g_signal_new(
      "layout-add-manager", G_TYPE_FROM_CLASS(klass),
      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION, 0, NULL, NULL,
      g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, GTK_TYPE_WIDGET);
  gl_layout_signals[GL_LAYOUT_ACTIVATED] =
      g_signal_new("layout-activated", G_TYPE_FROM_CLASS(klass),
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION, 0, NULL, NULL,
                   g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
  gl_layout_signals[GL_LAYOUT_STATE] =
      g_signal_new("layout-state", G_TYPE_FROM_CLASS(klass),
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION, 0, NULL, NULL,
                   g_cclosure_marshal_VOID__UINT, G_TYPE_NONE, 1, G_TYPE_UINT);
}

const gchar *gl_layout_get_id(GlLayout *layout) {
  g_return_val_if_fail(layout != NULL, NULL);
  g_return_val_if_fail(GL_IS_LAYOUT(layout), NULL);
  return layout->priv->id;
}
const gchar *gl_layout_get_name(GlLayout *layout) {
  g_return_val_if_fail(layout != NULL, NULL);
  g_return_val_if_fail(GL_IS_LAYOUT(layout), NULL);
  return layout->priv->name;
}

void gl_layout_set_name(GlLayout *layout, const gchar *name) {
  g_return_if_fail(layout != NULL);
  g_return_if_fail(GL_IS_LAYOUT(layout));
  g_object_set(layout, "layout-name", name, NULL);
}

void gl_layout_set_close_level(GlLayout *layout, guint level) {
  g_return_if_fail(layout != NULL);
  g_return_if_fail(GL_IS_LAYOUT(layout));
  layout->priv->close_level = level;
}
guint gl_layout_get_close_level(GlLayout *layout) {
  g_return_val_if_fail(layout != NULL, 0);
  g_return_val_if_fail(GL_IS_LAYOUT(layout), 0);
  return layout->priv->close_level;
}

gboolean gl_layout_get_activate(GlLayout *layout) {
  g_return_val_if_fail(layout != NULL, FALSE);
  g_return_val_if_fail(GL_IS_LAYOUT(layout), FALSE);
  return layout->priv->activate;
}

void gl_layout_activate(GlLayout *layout) {
  g_return_if_fail(layout != NULL);
  g_return_if_fail(GL_IS_LAYOUT(layout));
  layout->priv->activate = TRUE;
  /*GSList *l = NULL;
  for(l=layout->priv->contaiments_layouts;l!=NULL;l=l->next)
  {
          gl_layout_activate(GL_LAYOUT(l->data));
  }*/
  if (GL_LAYOUT_GET_CLASS(layout)->activate)
    GL_LAYOUT_GET_CLASS(layout)->activate(layout);

  gtk_widget_show_all(GTK_WIDGET(layout));
  g_signal_emit(layout, gl_layout_signals[GL_LAYOUT_ACTIVATED], 0);
}

void gl_layout_deactivate(GlLayout *layout) {
  g_return_if_fail(layout != NULL);
  g_return_if_fail(GL_IS_LAYOUT(layout));
  layout->priv->activate = FALSE;
  gl_layout_close_dialogs(layout);
  if (GL_LAYOUT_GET_CLASS(layout)->deactivate)
    GL_LAYOUT_GET_CLASS(layout)->deactivate(layout);
  // gtk_widget_hide(GTK_WIDGET(layout));
}

void gl_layout_add_manager(GlLayout *layout, GtkWidget *manager) {
  g_return_if_fail(layout != NULL);
  g_return_if_fail(GL_IS_LAYOUT(layout));
  g_return_if_fail(layout->priv->layout_manager == NULL);
  g_return_if_fail(manager != NULL);
  g_object_set(layout, "layout-manager", manager, NULL);
}
GtkWidget *gl_layout_get_manager(GlLayout *layout) {
  g_return_val_if_fail(layout != NULL, NULL);
  g_return_val_if_fail(GL_IS_LAYOUT(layout), NULL);
  return layout->priv->layout_manager;
}

GlLayout *gl_layout_get_parent(GlLayout *layout) {
  g_return_val_if_fail(layout != NULL, NULL);
  g_return_val_if_fail(GL_IS_LAYOUT(layout), NULL);
  return layout->priv->parent;
}

GlLayout *gl_layout_for_widget(GtkWidget *widget) {
  GtkWidget *parent = gtk_widget_get_parent(widget);
  while (parent != NULL) {
    if (GL_IS_LAYOUT(parent))
      break;
    parent = gtk_widget_get_parent(parent);
  }
  return (parent != NULL && GL_IS_LAYOUT(parent)) ? GL_LAYOUT(parent) : NULL;
}

GList *gl_layout_get_children(GlLayout *layout) {
  g_return_val_if_fail(layout != NULL, NULL);
  g_return_val_if_fail(GL_IS_LAYOUT(layout), NULL);
  return layout->priv->child_layouts;
}

void gl_layout_add_child_layout(GlLayout *layout, GlLayout *child) {
  g_return_if_fail(layout != NULL);
  g_return_if_fail(GL_IS_LAYOUT(layout));
  g_return_if_fail(child != NULL);
  g_return_if_fail(GL_IS_LAYOUT(child));
  g_object_set(child, "layout-parent", layout, NULL);
}

static void destroy_dialog_widget(GtkWidget *dialog, GlLayout *layout) {
  if (layout->priv->child_dialogs != NULL) {
    layout->priv->child_dialogs =
        g_list_remove(layout->priv->child_dialogs, dialog);
  }
}

void gl_layout_add_dialog(GlLayout *layout, GtkWidget *dialog) {
  g_return_if_fail(layout != NULL);
  g_return_if_fail(GL_IS_LAYOUT(layout));
  g_return_if_fail(dialog != NULL);
  g_return_if_fail(GTK_IS_WIDGET(dialog));
  layout->priv->child_dialogs =
      g_list_append(layout->priv->child_dialogs, dialog);
  g_signal_connect(dialog, "destroy", G_CALLBACK(destroy_dialog_widget),
                   layout);
}

void gl_layout_close_dialogs(GlLayout *layout) {
  g_return_if_fail(layout != NULL);
  g_return_if_fail(GL_IS_LAYOUT(layout));
  GList *l = NULL;
  for (l = layout->priv->child_dialogs; l != NULL; l = l->next) {
    if (GTK_IS_WIDGET(l->data))
      gtk_widget_hide(GTK_WIDGET(l->data));
  }
}

void gl_layout_change_state(GlLayout *layout, guint state) {
  g_return_if_fail(layout != NULL);
  g_return_if_fail(GL_IS_LAYOUT(layout));
  layout->priv->last_state = state;
  g_signal_emit(layout, gl_layout_signals[GL_LAYOUT_STATE], 0, state);
}

guint gl_layout_last_state(GlLayout *layout) {
  g_return_val_if_fail(layout != NULL, 0);
  g_return_val_if_fail(GL_IS_LAYOUT(layout), 0);
  return layout->priv->last_state;
}

static gboolean __binding_LEVEL2(GBinding *binding, const GValue *from_value,
                                 GValue *to_value, gpointer user_data) {
  g_return_val_if_fail(from_value != NULL, FALSE);
  g_return_val_if_fail(to_value != NULL, FALSE);
  guint level = g_value_get_uint(from_value);
  g_value_set_boolean(to_value, level >= 2);
  return TRUE;
}
static gboolean __binding_LEVEL3(GBinding *binding, const GValue *from_value,
                                 GValue *to_value, gpointer user_data) {
  g_return_val_if_fail(from_value != NULL, FALSE);
  g_return_val_if_fail(to_value != NULL, FALSE);
  guint level = g_value_get_uint(from_value);
  g_value_set_boolean(to_value, level >= 3);
  return TRUE;
}

static gboolean __binding_LEVEL4(GBinding *binding, const GValue *from_value,
                                 GValue *to_value, gpointer user_data) {
  g_return_val_if_fail(from_value != NULL, FALSE);
  g_return_val_if_fail(to_value != NULL, FALSE);
  guint level = g_value_get_uint(from_value);
  g_value_set_boolean(to_value, level >= 4);
  return TRUE;
}

static gboolean __binding_LEVEL5(GBinding *binding, const GValue *from_value,
                                 GValue *to_value, gpointer user_data) {
  g_return_val_if_fail(from_value != NULL, FALSE);
  g_return_val_if_fail(to_value != NULL, FALSE);
  g_return_val_if_fail(G_VALUE_HOLDS_UINT(from_value), FALSE);
  guint level = g_value_get_uint(from_value);
  g_value_set_boolean(to_value, level >= 5);
  return TRUE;
}

void gl_layout_LEVEL2_sensitive(GtkWidget *widget) {
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_WIDGET(widget));
  g_object_bind_property_full(TERA_GUARD(), "level", widget, "sensitive",
                              G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE,
                              __binding_LEVEL2, NULL, NULL, NULL);
}
void gl_layout_LEVEL3_sensitive(GtkWidget *widget) {
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_WIDGET(widget));
  g_object_bind_property_full(TERA_GUARD(), "level", widget, "sensitive",
                              G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE,
                              __binding_LEVEL3, NULL, NULL, NULL);
}

void gl_layout_LEVEL4_sensitive(GtkWidget *widget) {
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_WIDGET(widget));
  g_object_bind_property_full(TERA_GUARD(), "level", widget, "sensitive",
                              G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE,
                              __binding_LEVEL4, NULL, NULL, NULL);
}

void gl_layout_LEVEL5_sensitive(GtkWidget *widget) {
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_WIDGET(widget));
  g_object_bind_property_full(TERA_GUARD(), "level", widget, "sensitive",
                              G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE,
                              __binding_LEVEL5, NULL, NULL, NULL);
}

void gl_layout_LEVEL2_visible(GtkWidget *widget) {
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_WIDGET(widget));
  g_object_bind_property_full(TERA_GUARD(), "level", widget, "visible",
                              G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE,
                              __binding_LEVEL2, NULL, NULL, NULL);
}

void gl_layout_LEVEL3_visible(GtkWidget *widget) {
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_WIDGET(widget));
  g_object_bind_property_full(TERA_GUARD(), "level", widget, "visible",
                              G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE,
                              __binding_LEVEL3, NULL, NULL, NULL);
}

void gl_layout_LEVEL4_visible(GtkWidget *widget) {
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_WIDGET(widget));
  g_object_bind_property_full(TERA_GUARD(), "level", widget, "visible",
                              G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE,
                              __binding_LEVEL4, NULL, NULL, NULL);
}

void gl_layout_LEVEL5_visible(GtkWidget *widget) {
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_WIDGET(widget));
  g_object_bind_property_full(TERA_GUARD(), "level", widget, "visible",
                              G_BINDING_DEFAULT | G_BINDING_SYNC_CREATE,
                              __binding_LEVEL5, NULL, NULL, NULL);
}

void gl_layout_realize_LEVEL2_sensitive(GtkWidget *widget, gpointer data) {
  gl_layout_LEVEL2_sensitive(widget);
}

void gl_layout_realize_LEVEL3_sensitive(GtkWidget *widget, gpointer data) {
  gl_layout_LEVEL3_sensitive(widget);
}

void gl_layout_realize_LEVEL4_sensitive(GtkWidget *widget, gpointer data) {
  gl_layout_LEVEL4_sensitive(widget);
}
void gl_layout_realize_LEVEL5_sensitive(GtkWidget *widget, gpointer data) {
  gl_layout_LEVEL5_sensitive(widget);
}

void gl_layout_realize_LEVEL2_visible(GtkWidget *widget, gpointer data) {
  gl_layout_LEVEL2_visible(widget);
}
void gl_layout_realize_LEVEL3_visible(GtkWidget *widget, gpointer data) {
  gl_layout_LEVEL3_visible(widget);
}
void gl_layout_realize_LEVEL4_visible(GtkWidget *widget, gpointer data) {
  gl_layout_LEVEL4_visible(widget);
}
void gl_layout_realize_LEVEL5_visible(GtkWidget *widget, gpointer data) {
  gl_layout_LEVEL5_visible(widget);
}

/*
static gint
layout_compare_layout_name        (gconstpointer  a, gconstpointer  b)
{
        g_debug("layout_compare_layout_name %s == %s
",gl_layout_get_id(GL_LAYOUT(a)),(const gchar*)b);

        return g_strcmp0(gl_layout_get_id(GL_LAYOUT(a)),(const gchar*)b);
}


GlLayout*
gl_layout_find_child                ( GlLayout *layout , const gchar *id )
{
        g_return_val_if_fail(layout!=NULL,NULL);
        g_return_val_if_fail(GL_IS_LAYOUT(layout),NULL);
        GSList *l =
g_list_find_custom(layout->priv->child_layouts,id,layout_compare_layout_name);
        if(l==NULL)return NULL;
        return GL_LAYOUT(l->data);
}
*/

void gl_layout_button_clicked_signal_cb(GlLayout *layout, GtkButton *button) {
  g_return_if_fail(layout != NULL);
  g_return_if_fail(GL_IS_LAYOUT(layout));
  g_return_if_fail(button != NULL);
  g_return_if_fail(GTK_IS_BUTTON(button));
  const gchar *widget_name = gtk_widget_get_name(GTK_WIDGET(button));
  gl_layout_manager_default_activate_named(widget_name);
}

void gl_layout_activate_list_box_signal_cb(GlLayout *layout, GtkListBoxRow *row,
                                           GtkListBox *box) {
  g_return_if_fail(layout != NULL);
  g_return_if_fail(GL_IS_LAYOUT(layout));
  g_return_if_fail(row != NULL);
  g_return_if_fail(GTK_IS_LIST_BOX_ROW(row));
  const gchar *widget_name = gtk_widget_get_name(GTK_WIDGET(row));
  // g_debug("Activate layout=%s widget name =%s",layout_name,widget_name);
  gl_layout_manager_default_activate_named(widget_name);
}

/** @} */

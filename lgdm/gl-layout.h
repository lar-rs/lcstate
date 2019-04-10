/**
 * file  gl-layout.h object header
 *
 * Copyright (C) LAR 2013
 *
 * author A.Smolkov <asmolkov@lar.com>
 *
 */

#ifndef __GL_LAYOUT_H__
#define __GL_LAYOUT_H__
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GL_TYPE_LAYOUT (gl_layout_get_type())
#define GL_LAYOUT(obj)                                                         \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GL_TYPE_LAYOUT, GlLayout))
#define GL_LAYOUT_CLASS(klass)                                                 \
  (G_TYPE_CHECK_CLASS_CAST((klass), GL_TYPE_LAYOUT, GlLayoutClass))
#define GL_IS_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GL_TYPE_LAYOUT))
#define GL_IS_LAYOUT_CLASS(klass)                                              \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GL_TYPE_LAYOUT))
#define GL_LAYOUT_GET_CLASS(obj)                                               \
  (G_TYPE_INSTANCE_GET_CLASS((obj), GL_TYPE_LAYOUT, GlLayoutClass))

typedef struct _GlLayout GlLayout;
typedef struct _GlLayoutClass GlLayoutClass;
typedef struct _GlLayoutPrivate GlLayoutPrivate;

struct _GlLayoutClass {
  GtkScrolledWindowClass parent_class;
  void (*activate)(GlLayout *layout);
  void (*deactivate)(GlLayout *layout);
  GtkMenu *(*menu)(GlLayout *layout);
};

struct _GlLayout {
  GtkScrolledWindow layout;
  GlLayoutPrivate *priv;
};

enum {
  GL_LAYOUT_ACTIVATE_UP,
  GL_LAYOUT_ACTIVATE_LEFT,
  GL_LAYOUT_ACTIVATE_RIGHT,
  GL_LAYOUT_DEACTIVATE_LEFT,
  GL_LAYOUT_DEACTIVATE_RIGHT,
  GL_LAYOUT_DEACTIVATE_DOWN,
};

GType gl_layout_get_type(void);

const gchar *gl_layout_get_id(GlLayout *layout);
const gchar *gl_layout_get_name(GlLayout *layout);
void gl_layout_set_name(GlLayout *layout, const gchar *name);
void gl_layout_set_close_level(GlLayout *layout, guint level);
guint gl_layout_get_close_level(GlLayout *layout);

gboolean gl_layout_get_activate(GlLayout *layout);
void gl_layout_activate(GlLayout *layout);
void gl_layout_deactivate(GlLayout *layout);
void gl_layout_add_contaiment_layout(GlLayout *layout, GlLayout *contaiment);
void gl_layout_add_child_layout(GlLayout *layout, GlLayout *child);
void gl_layout_add_manager(GlLayout *layout, GtkWidget *manager);
GlLayout *gl_layout_get_parent(GlLayout *layout);
GList *gl_layout_get_children(GlLayout *layout);
GlLayout *gl_layout_for_widget(GtkWidget *widget);

GtkWidget *gl_layout_get_manager(GlLayout *layout);
GtkMenu *gl_layout_get_menu(GlLayout *layout);
GlLayout *gl_layout_find_child(GlLayout *layout, const gchar *id);

gboolean gl_layout_activate_from_name(GlLayout *layout, const gchar *name);

void gl_layout_close_dialogs(GlLayout *layout);
void gl_layout_add_dialog(GlLayout *layout, GtkWidget *dialog);
void gl_layout_change_state(GlLayout *layout, guint state);
guint gl_layout_last_state(GlLayout *layout);

void gl_layout_activate_list_box_signal_cb(GlLayout *layout, GtkListBoxRow *row,
                                           GtkListBox *box);
void gl_layout_button_clicked_signal_cb(GlLayout *layout, GtkButton *button);

void gl_layout_widget_change_state(GtkWidget *widget, GtkStateFlags flags,
                                   GlLayout *layout);

void gl_layout_LEVEL2_sensitive(GtkWidget *widget);
void gl_layout_LEVEL3_sensitive(GtkWidget *widget);
void gl_layout_LEVEL4_sensitive(GtkWidget *widget);
void gl_layout_LEVEL5_sensitive(GtkWidget *widget);

void gl_layout_LEVEL2_visible(GtkWidget *widget);
void gl_layout_LEVEL3_visible(GtkWidget *widget);
void gl_layout_LEVEL4_visible(GtkWidget *widget);
void gl_layout_LEVEL5_visible(GtkWidget *widget);

void gl_layout_realize_LEVEL2_sensitive(GtkWidget *widget, gpointer data);
void gl_layout_realize_LEVEL3_sensitive(GtkWidget *widget, gpointer data);
void gl_layout_realize_LEVEL4_sensitive(GtkWidget *widget, gpointer data);
void gl_layout_realize_LEVEL5_sensitive(GtkWidget *widget, gpointer data);

void gl_layout_realize_LEVEL2_visible(GtkWidget *widget, gpointer data);
void gl_layout_realize_LEVEL3_visible(GtkWidget *widget, gpointer data);
void gl_layout_realize_LEVEL4_visible(GtkWidget *widget, gpointer data);
void gl_layout_realize_LEVEL5_visible(GtkWidget *widget, gpointer data);

//-------------------------------progress bar control
//----------------------------------------------

G_END_DECLS
#endif /* __GL_LAYOUT_H__ */

/** @} */

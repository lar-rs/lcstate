/*
 * gl-curve.h
 *
 *  Created on: 08.02.2012
 *      Author: sascha
 */

#ifndef GL_CURVE_H_
#define GL_CURVE_H_


#include <gtk/gtk.h>


G_BEGIN_DECLS

#define GL_TYPE_CURVE                  (gl_curve_get_type ())
#define GL_CURVE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GL_TYPE_CURVE, GlCurve))
#define GL_CURVE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GL_TYPE_CURVE, GlCurveClass))
#define GL_IS_CURVE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GL_TYPE_CURVE))
#define GL_IS_CURVE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GL_TYPE_CURVE))
#define GL_CURVE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GL_TYPE_CURVE, GlCurveClass))


typedef struct _GlCurve	        GlCurve;
typedef struct _GlCurveClass	GlCurveClass;
typedef struct _GlCurvePrivate  GlCurvePrivate;

typedef enum
{
  GL_CURVE_TYPE_LINEAR,       /* linear interpolation */
  GL_CURVE_TYPE_SPLINE,       /* spline interpolation */
  GL_CURVE_TYPE_FREE          /* free form curve */
} GlCurveType;


typedef enum
{
  GL_CURVE_SCALE_NONE,
  GL_CURVE_SCALE_EXPAND,
  GL_CURVE_SCALE_FREE
} GlCurveScale;


GType gl_curve_type_get_scale   (void) G_GNUC_CONST;
#define GL_TYPE_CURVE_SCALE     (gl_curve_type_get_type ())

GType gl_curve_type_get_type    (void) G_GNUC_CONST;
#define GL_TYPE_CURVE_TYPE      (gl_curve_type_get_type ())

struct _GlCurve
{
  GtkDrawingArea    graph;
  GlCurvePrivate   *priv;
  gint              cursor_type;

  GdkPixmap *pixmap;
  GlCurveType curve_type;
  gint height;                  /* (cached) graph height in pixels */
  gint grab_point;              /* point currently grabbed */
  gint last;

  /* (cached) curve points: */
  gint num_points;
  GdkPoint *point;

  /* control points: */
  gint num_ctlpoints;           /* number of control points */
  gfloat (*ctlpoint)[2];        /* array of control points */
};

struct _GlCurveClass
{
  GtkDrawingAreaClass parent_class;

  void (* gl_curve_type_changed)  (GlCurve *curve);
  void (* gl_curve_scale_changed) (GlCurve *curve);

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};


GType		gl_curve_get_type	(void) G_GNUC_CONST;
GtkWidget*	gl_curve_new		(void);
void		gl_curve_reset		(GlCurve *curve);
void		gl_curve_set_gamma	(GlCurve *curve, gfloat gamma_);
void		gl_curve_set_range	(GlCurve *curve,
					 gfloat min_x, gfloat max_x,
					 gfloat min_y, gfloat max_y);
void		gl_curve_get_vector	(GlCurve *curve,
					 int veclen, gfloat vector[]);
void		gl_curve_set_vector	(GlCurve *curve,
					 int veclen, gfloat vector[]);

gboolean    gl_curve_set_next_point         (GlCurve *c, GdkPoint *p);

void		gl_curve_set_curve_type         (GlCurve *curve, GlCurveType  type);
void        gl_curve_set_curve_scale        (GlCurve *c,     GlCurveScale new_scale);

G_END_DECLS

#endif /* GL_CURVE_H_ */

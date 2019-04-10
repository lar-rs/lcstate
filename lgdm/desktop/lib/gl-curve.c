/* GTK - The GIMP Toolkit
 * Copyright (C) 1997 David Mosberger
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */


#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gl-curve.h"
#include <gtk/gtk.h>
#include <gtk/gtkprivate.h>


#define P_(String) (String)

/* not really I18N-related, but also a string marker macro */
#define I_(string) g_intern_static_string (string)


#define RADIUS		3	/* radius of the control points */
#define MIN_DISTANCE	8	/* min distance between control points */

#define GRAPH_MASK	(GDK_EXPOSURE_MASK |		\
		GDK_POINTER_MOTION_MASK |	\
		GDK_POINTER_MOTION_HINT_MASK |	\
		GDK_ENTER_NOTIFY_MASK |	\
		GDK_BUTTON_PRESS_MASK |	\
		GDK_BUTTON_RELEASE_MASK |	\
		GDK_BUTTON1_MOTION_MASK)

enum {
	PROP_0,
	PROP_GL_CURVE_TYPE,
	PROP_GL_CURVE_SCALE,
	PROP_GL_MIN_X,
	PROP_GL_MAX_X,
	PROP_GL_MIN_Y,
	PROP_GL_MAX_Y
};


struct _GlCurvePrivate
{
	GdkColor             punkt_color;

	gdouble              min_x;
	gdouble              max_x;
	gdouble              min_y;
	gdouble              max_y;

	GlCurveScale         curve_scale;
	gdouble              scaleXmin;
	gdouble              scaleXmax;
	gdouble              scaleYmin;
	gdouble              scaleYmax;
};



static GtkDrawingAreaClass *parent_class = NULL;


enum
{
	GL_CURVE_SIGNAL_TYPE_CHANGE,
	GL_CURVE_SIGNAL_SCALE_CHANGE,
	LAST_SIGNAL
};

static guint gl_curve_signals[LAST_SIGNAL] = { 0 };


GType
gl_curve_type_get_type (void)
{
	static GType etype = 0;
	if (G_UNLIKELY(etype == 0)) {
		static const GEnumValue values[] = {
				{ GL_CURVE_TYPE_LINEAR, "GL_CURVE_TYPE_LINEAR", "linear" },
				{ GL_CURVE_TYPE_SPLINE, "GL_CURVE_TYPE_SPLINE", "spline" },
				{ GL_CURVE_TYPE_FREE, "GL_CURVE_TYPE_FREE", "free" },
				{ 0, NULL, NULL }
		};
		etype = g_enum_register_static (g_intern_static_string ("GlCurveType"), values);
	}
	return etype;
}

GType
gl_curve_type_get_scalling (void)
{
	static GType etype = 0;
	if (G_UNLIKELY(etype == 0)) {
		static const GEnumValue values[] = {
				{ GL_CURVE_SCALE_NONE,    "GL_CURVE_SCALE_NONE",    "not scale" },
				{ GL_CURVE_SCALE_EXPAND,  "GL_CURVE_SCALE_EXPAND",  "expand scale" },
				{ GL_CURVE_SCALE_FREE,    "GL_CURVE_SCALE_FREE",    "free scale" },
				{ 0, NULL, NULL }
		};
		etype = g_enum_register_static (g_intern_static_string ("GlCurveScale"), values);
	}
	return etype;
}


/* forward declarations: */
static void gl_curve_class_init   (GlCurveClass *class);
static void gl_curve_init         (GlCurve      *curve);
static void gl_curve_get_property  (GObject              *object,
		guint                 param_id,
		GValue               *value,
		GParamSpec           *pspec);
static void gl_curve_set_property  (GObject              *object,
		guint                 param_id,
		const GValue         *value,
		GParamSpec           *pspec);
static void gl_curve_finalize     (GObject       *object);
static gint gl_curve_graph_events (GtkWidget     *widget,
		GdkEvent      *event,
		GlCurve      *c);
static void gl_curve_size_graph   (GlCurve      *curve);

GType
gl_curve_get_type (void)
{
	static GType curve_type = 0;

	if (!curve_type)
	{
		const GTypeInfo curve_info =
		{
				sizeof (GlCurveClass),
				NULL,		/* base_init */
				NULL,		/* base_finalize */
				(GClassInitFunc) gl_curve_class_init,
				NULL,		/* class_finalize */
				NULL,		/* class_data */
				sizeof (GlCurve),
				0,		/* n_preallocs */
				(GInstanceInitFunc) gl_curve_init,
		};

		curve_type = g_type_register_static (GTK_TYPE_DRAWING_AREA, I_("GlCurve"),
				&curve_info, 0);
	}
	return curve_type;
}

static void
gl_curve_class_init (GlCurveClass *class)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);
	g_type_class_add_private (class, sizeof (GlCurvePrivate));
	parent_class = g_type_class_peek_parent (class);

	gobject_class->finalize = gl_curve_finalize;

	gobject_class->set_property = gl_curve_set_property;
	gobject_class->get_property = gl_curve_get_property;

	g_object_class_install_property (gobject_class,
			PROP_GL_CURVE_TYPE,
			g_param_spec_enum ("curve-type",
					P_("Curve type"),
					P_("Is this curve linear, spline interpolated, or free-form"),
					GL_TYPE_CURVE_TYPE,
					GL_CURVE_TYPE_FREE,
					GTK_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
			PROP_GL_CURVE_SCALE,
			g_param_spec_enum ("curve-scale",
					P_("Curve scale"),
					P_("Is this curve scale allow"),
					GL_TYPE_CURVE_SCALE,
					GL_CURVE_SCALE_FREE,
					GTK_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
			PROP_GL_MIN_X,
			g_param_spec_float ("min-x",
					P_("Minimum X"),
					P_("Minimum possible value for X"),
					-G_MAXFLOAT,
					G_MAXFLOAT,
					0.0,
					GTK_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
			PROP_GL_MAX_X,
			g_param_spec_float ("max-x",
					P_("Maximum X"),
					P_("Maximum possible X value"),
					-G_MAXFLOAT,
					G_MAXFLOAT,
					1.0,
					GTK_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
			PROP_GL_MIN_Y,
			g_param_spec_float ("min-y",
					P_("Minimum Y"),
					P_("Minimum possible value for Y"),
					-G_MAXFLOAT,
					G_MAXFLOAT,
					0.0,
					GTK_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
			PROP_GL_MAX_Y,
			g_param_spec_float ("max-y",
					P_("Maximum Y"),
					P_("Maximum possible value for Y"),
					-G_MAXFLOAT,
					G_MAXFLOAT,
					1.0,
					GTK_PARAM_READWRITE));

	gl_curve_signals[GL_CURVE_SIGNAL_TYPE_CHANGE] =
			g_signal_new (I_("curve-type-changed"),
					G_OBJECT_CLASS_TYPE (gobject_class),
					G_SIGNAL_RUN_FIRST,
					G_STRUCT_OFFSET (GlCurveClass, gl_curve_type_changed),
					NULL, NULL,
					g_cclosure_marshal_VOID__VOID,
					G_TYPE_NONE, 0);
	gl_curve_signals[GL_CURVE_SIGNAL_TYPE_CHANGE] =
				g_signal_new (I_("curve-scale-changed"),
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (GlCurveClass,gl_curve_scale_changed),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE, 0);
}

static void
gl_curve_init (GlCurve *curve)
{
	gint old_mask;
	curve->priv = G_TYPE_INSTANCE_GET_PRIVATE(curve,GL_TYPE_CURVE,GlCurvePrivate);
	curve->cursor_type = GDK_TOP_LEFT_ARROW;
	curve->pixmap = NULL;
	curve->curve_type = GL_CURVE_TYPE_FREE;
	curve->height = 0;
	curve->grab_point = -1;

	curve->num_points = 0;
	curve->point = NULL;

	curve->num_ctlpoints = 0;
	curve->ctlpoint = NULL;

	curve->priv->min_x = 0.0;
	curve->priv->max_x = 1.0;
	curve->priv->min_y = 0.0;
	curve->priv->max_y = 1.0;

	curve->priv->curve_scale = GL_CURVE_SCALE_FREE;

	curve->priv->scaleXmin = curve->priv->min_x;
	curve->priv->scaleXmax = curve->priv->max_x;
	curve->priv->scaleYmin = curve->priv->min_y;
	curve->priv->scaleYmax = curve->priv->max_y;

	old_mask = gtk_widget_get_events (GTK_WIDGET (curve));
	gtk_widget_set_events (GTK_WIDGET (curve), old_mask | GRAPH_MASK);
	g_signal_connect (curve, "event",
			G_CALLBACK (gl_curve_graph_events), curve);
	gl_curve_size_graph (curve);
}

static void
gl_curve_set_property (GObject              *object,
		guint                 prop_id,
		const GValue         *value,
		GParamSpec           *pspec)
{
	GlCurve *curve = GL_CURVE (object);

	switch (prop_id)
	{
	case PROP_GL_CURVE_TYPE:
		gl_curve_set_curve_type (curve, g_value_get_enum (value));
		break;
	case PROP_GL_CURVE_SCALE:
		gl_curve_set_curve_scale (curve, g_value_get_enum (value));
		break;
	case PROP_GL_MIN_X:
		gl_curve_set_range (curve, g_value_get_float (value), curve->priv->max_x,
				curve->priv->min_y, curve->priv->max_y);
		break;
	case PROP_GL_MAX_X:
		gl_curve_set_range (curve, curve->priv->min_x, g_value_get_float (value),
				curve->priv->min_y, curve->priv->max_y);
		break;
	case PROP_GL_MIN_Y:
		gl_curve_set_range (curve, curve->priv->min_x, curve->priv->max_x,
				g_value_get_float (value), curve->priv->max_y);
		break;
	case PROP_GL_MAX_Y:
		gl_curve_set_range (curve, curve->priv->min_x, curve->priv->max_x,
				curve->priv->min_y, g_value_get_float (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gl_curve_get_property (GObject              *object,
		guint                 prop_id,
		GValue               *value,
		GParamSpec           *pspec)
{
	GlCurve *curve = GL_CURVE (object);

	switch (prop_id)
	{
	case PROP_GL_CURVE_TYPE:
		g_value_set_enum (value, curve->curve_type);
		break;
	case PROP_GL_CURVE_SCALE:
		g_value_set_enum (value, curve->priv->curve_scale);
		break;
	case PROP_GL_MIN_X:
		g_value_set_float (value, curve->priv->min_x);
		break;
	case PROP_GL_MAX_X:
		g_value_set_float (value, curve->priv->max_x);
		break;
	case PROP_GL_MIN_Y:
		g_value_set_float (value, curve->priv->min_y);
		break;
	case PROP_GL_MAX_Y:
		g_value_set_float (value, curve->priv->max_y);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static int
project (gfloat value, gfloat min, gfloat max, int norm)
{
	return (norm - 1) * ((value - min) / (max - min)) + 0.5;
}

static gfloat
unproject (gint value, gfloat min, gfloat max, int norm)
{
	return value / (gfloat) (norm - 1) * (max - min) + min;
}

/* Solve the tridiagonal equation system that determines the second
   derivatives for the interpolation points.  (Based on Numerical
   Recipies 2nd Edition.) */
static void
spline_solve (int n, gfloat x[], gfloat y[], gfloat y2[])
{
	gfloat p, sig, *u;
	gint i, k;

	u = g_malloc ((n - 1) * sizeof (u[0]));

	y2[0] = u[0] = 0.0;	/* set lower boundary condition to "natural" */

	for (i = 1; i < n - 1; ++i)
	{
		sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
		p = sig * y2[i - 1] + 2.0;
		y2[i] = (sig - 1.0) / p;
		u[i] = ((y[i + 1] - y[i])
				/ (x[i + 1] - x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1]));
		u[i] = (6.0 * u[i] / (x[i + 1] - x[i - 1]) - sig * u[i - 1]) / p;
	}

	y2[n - 1] = 0.0;
	for (k = n - 2; k >= 0; --k)
		y2[k] = y2[k] * y2[k + 1] + u[k];

	g_free (u);
}

static gfloat
spline_eval (int n, gfloat x[], gfloat y[], gfloat y2[], gfloat val)
{
	gint k_lo, k_hi, k;
	gfloat h, b, a;

	/* do a binary search for the right interval: */
	k_lo = 0; k_hi = n - 1;
	while (k_hi - k_lo > 1)
	{
		k = (k_hi + k_lo) / 2;
		if (x[k] > val)
			k_hi = k;
		else
			k_lo = k;
	}

	h = x[k_hi] - x[k_lo];
	g_assert (h > 0.0);

	a = (x[k_hi] - val) / h;
	b = (val - x[k_lo]) / h;
	return a*y[k_lo] + b*y[k_hi] +
			((a*a*a - a)*y2[k_lo] + (b*b*b - b)*y2[k_hi]) * (h*h)/6.0;
}

static void
gl_curve_interpolate (GlCurve *c, gint width, gint height)
{
	gfloat *vector;
	int i;

	vector = g_malloc (width * sizeof (vector[0]));

	gl_curve_get_vector (c, width, vector);

	c->height = height;
	if (c->num_points != width)
	{
		c->num_points = width;
		g_free (c->point);
		c->point = g_malloc (c->num_points * sizeof (c->point[0]));
	}

	for (i = 0; i < width; ++i)
	{
		c->point[i].x = RADIUS + i;
		c->point[i].y = RADIUS + height
				- project (vector[i], c->priv->min_y, c->priv->max_y, height);
	}

	g_free (vector);
}

static void
gl_curve_draw (GlCurve *c, gint width, gint height)
{
	GtkStateType state;
	GtkStyle *style;
	gint i;

	if (!c->pixmap)
		return;

	if (c->height != height || c->num_points != width)
		gl_curve_interpolate (c, width, height);

	state = GTK_STATE_NORMAL;
	if (!GTK_WIDGET_IS_SENSITIVE(GTK_WIDGET (c)))
		state = GTK_STATE_INSENSITIVE;

	style = GTK_WIDGET (c)->style;

	/* clear the pixmap: */
	gtk_paint_flat_box (style, c->pixmap, GTK_STATE_NORMAL, GTK_SHADOW_ETCHED_OUT,
			NULL, GTK_WIDGET (c), "curve_bg",
			0, 0, width + RADIUS * 2, height + RADIUS * 2);
	/* draw the grid lines: (XXX make more meaningful) */

	for (i = 0; i < 6; i++)
	{
		gdk_draw_line (c->pixmap, style->dark_gc[state],
				RADIUS, i * (height / 4.0) + RADIUS,
				width + RADIUS, i * (height / 4.0) + RADIUS);
		gdk_draw_line (c->pixmap, style->dark_gc[state],
				i * (width / 4.0) + RADIUS, RADIUS,
				i * (width / 4.0) + RADIUS, height + RADIUS);
	}

	gdk_draw_points (c->pixmap, style->fg_gc[state], c->point, c->num_points);

	gdk_draw_drawable (GTK_WIDGET (c)->window, style->fg_gc[state], c->pixmap,
			0, 0, 0, 0, width + RADIUS * 2, height + RADIUS * 2);
}

static gint
gl_curve_graph_events (GtkWidget *widget, GdkEvent *event, GlCurve *c)
{
	GdkCursorType new_type = c->cursor_type;
	gint i, src, dst;
	GdkEventMotion *mevent;
	GtkWidget *w;
	gint tx, ty;
	gint cx, x, y, width, height;
	gint closest_point = 0;
	gfloat  min_x;
	guint distance;
	gint x1, x2, y1, y2;
	gint retval = FALSE;

	w = GTK_WIDGET (c);
	width = w->allocation.width - RADIUS * 2;
	height = w->allocation.height - RADIUS * 2;

	if ((width < 0) || (height < 0))
		return FALSE;

	/*  get the pointer position  */
	gdk_window_get_pointer (w->window, &tx, &ty, NULL);
	x = CLAMP ((tx - RADIUS), 0, width-1);
	y = CLAMP ((ty - RADIUS), 0, height-1);

	min_x = c->priv->min_x;

	distance = ~0U;
	for (i = 0; i < c->num_ctlpoints; ++i)
	{
		cx = project (c->ctlpoint[i][0], min_x, c->priv->max_x, width);
		if ((guint) abs (x - cx) < distance)
		{
			distance = abs (x - cx);
			closest_point = i;
		}
	}

	switch (event->type)
	{
	case GDK_CONFIGURE:
		if (c->pixmap)
			g_object_unref (c->pixmap);
		c->pixmap = NULL;
		/* fall through */
	case GDK_EXPOSE:
		if (!c->pixmap)
			c->pixmap = gdk_pixmap_new (w->window,
					w->allocation.width,
					w->allocation.height, -1);
		gl_curve_draw (c, width, height);
		break;

	case GDK_BUTTON_PRESS:
		gtk_grab_add (widget);

		new_type = GDK_TCROSS;

		switch (c->curve_type)
		{
		case GL_CURVE_TYPE_LINEAR:
		case GL_CURVE_TYPE_SPLINE:
			if (distance > MIN_DISTANCE)
			{
				/* insert a new control point */
				if (c->num_ctlpoints > 0)
				{
					cx = project (c->ctlpoint[closest_point][0], min_x,
							c->priv->max_x, width);
					if (x > cx)
						++closest_point;
				}
				++c->num_ctlpoints;
				c->ctlpoint =
						g_realloc (c->ctlpoint,
								c->num_ctlpoints * sizeof (*c->ctlpoint));
				for (i = c->num_ctlpoints - 1; i > closest_point; --i)
					memcpy (c->ctlpoint + i, c->ctlpoint + i - 1,
							sizeof (*c->ctlpoint));
			}
			c->grab_point = closest_point;
			c->ctlpoint[c->grab_point][0] =
					unproject (x, min_x, c->priv->max_x, width);
			c->ctlpoint[c->grab_point][1] =
					unproject (height - y, c->priv->min_y, c->priv->max_y, height);

			gl_curve_interpolate (c, width, height);
			break;

		case GL_CURVE_TYPE_FREE:
			c->point[x].x = RADIUS + x;
			c->point[x].y = RADIUS + y;
			c->grab_point = x;
			c->last = y;
			break;
		}
		gl_curve_draw (c, width, height);
		retval = TRUE;
		break;

		case GDK_BUTTON_RELEASE:
			gtk_grab_remove (widget);

			/* delete inactive points: */
			if (c->curve_type != GL_CURVE_TYPE_FREE)
			{
				for (src = dst = 0; src < c->num_ctlpoints; ++src)
				{
					if (c->ctlpoint[src][0] >= min_x)
					{
						memcpy (c->ctlpoint + dst, c->ctlpoint + src,
								sizeof (*c->ctlpoint));
						++dst;
					}
				}
				if (dst < src)
				{
					c->num_ctlpoints -= (src - dst);
					if (c->num_ctlpoints <= 0)
					{
						c->num_ctlpoints = 1;
						c->ctlpoint[0][0] = min_x;
						c->ctlpoint[0][1] = c->priv->min_y;
						gl_curve_interpolate (c, width, height);
						gl_curve_draw (c, width, height);
					}
					c->ctlpoint =
							g_realloc (c->ctlpoint,
									c->num_ctlpoints * sizeof (*c->ctlpoint));
				}
			}
			new_type = GDK_FLEUR;
			c->grab_point = -1;
			retval = TRUE;
			break;

		case GDK_MOTION_NOTIFY:
			mevent = (GdkEventMotion *) event;

			switch (c->curve_type)
			{
			case GL_CURVE_TYPE_FREE:
				if (c->grab_point != -1)
				{
					if (c->grab_point > x)
					{
						x1 = x;
						x2 = c->grab_point;
						y1 = y;
						y2 = c->last;
					}
					else
					{
						x1 = c->grab_point;
						x2 = x;
						y1 = c->last;
						y2 = y;
					}

					if (x2 != x1)
						for (i = x1; i <= x2; i++)
						{
							c->point[i].x = RADIUS + i;
							c->point[i].y = RADIUS +
									(y1 + ((y2 - y1) * (i - x1)) / (x2 - x1));
						}
					else
					{
						c->point[x].x = RADIUS + x;
						c->point[x].y = RADIUS + y;
					}
					c->grab_point = x;
					c->last = y;
					gl_curve_draw (c, width, height);
				}
				if (mevent->state & GDK_BUTTON1_MASK)
					new_type = GDK_TCROSS;
				else
					new_type = GDK_PENCIL;
				break;
			default:
				break;
			}
			if (new_type != (GdkCursorType) c->cursor_type)
			{
				GdkCursor *cursor;

				c->cursor_type = new_type;

				cursor = gdk_cursor_new_for_display (gtk_widget_get_display (w),
						c->cursor_type);
				gdk_window_set_cursor (w->window, cursor);
				gdk_cursor_unref (cursor);
			}
			retval = TRUE;
			break;

			default:
				break;
	}

	return retval;
}

void
gl_curve_set_curve_type (GlCurve *c, GlCurveType new_type)
{
	gfloat rx, dx;
	gint x, i;

	if (new_type != c->curve_type)
	{
		gint width, height;

		width  = GTK_WIDGET (c)->allocation.width - RADIUS * 2;
		height = GTK_WIDGET (c)->allocation.height - RADIUS * 2;

		if (new_type == GL_CURVE_TYPE_FREE)
		{
			gl_curve_interpolate (c, width, height);
			c->curve_type = new_type;
		}
		else if (c->curve_type == GL_CURVE_TYPE_FREE)
		{
			g_free (c->ctlpoint);
			c->num_ctlpoints = 9;
			c->ctlpoint = g_malloc (c->num_ctlpoints * sizeof (*c->ctlpoint));

			rx = 0.0;
			dx = (width - 1) / (gfloat) (c->num_ctlpoints - 1);

			for (i = 0; i < c->num_ctlpoints; ++i, rx += dx)
			{
				x = (int) (rx + 0.5);
				c->ctlpoint[i][0] =
						unproject (x, c->priv->min_x, c->priv->max_x, width);
				c->ctlpoint[i][1] =
						unproject (RADIUS + height - c->point[x].y,
								c->priv->min_y, c->priv->max_y, height);
			}
			c->curve_type = new_type;
			gl_curve_interpolate (c, width, height);
		}
		else
		{
			c->curve_type = new_type;
			gl_curve_interpolate (c, width, height);
		}
		g_signal_emit (c, gl_curve_signals[GL_CURVE_SIGNAL_TYPE_CHANGE], 0);
		g_object_notify (G_OBJECT (c), "curve-type");
		gl_curve_draw (c, width, height);
	}
}

void
gl_curve_set_curve_scale (GlCurve *c, GlCurveScale new_scale)
{


	if (new_scale != c->priv->curve_scale)
	{
		gint width, height;

		width  = GTK_WIDGET (c)->allocation.width - RADIUS * 2;
		height = GTK_WIDGET (c)->allocation.height - RADIUS * 2;
		c->priv->curve_scale = new_scale;

		g_signal_emit (c, gl_curve_signals[GL_CURVE_SIGNAL_SCALE_CHANGE], 0);
		g_object_notify (G_OBJECT (c), "curve-scale");
		gl_curve_draw (c, width, height);
	}
}

static void
gl_curve_size_graph (GlCurve *curve)
{
	gint width, height;
	gfloat aspect;
	GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (curve));

	width  = (curve->priv->max_x - curve->priv->min_x) + 1;
	height = (curve->priv->max_y - curve->priv->min_y) + 1;
	aspect = width / (gfloat) height;
	if (width > gdk_screen_get_width (screen) / 4)
		width  = gdk_screen_get_width (screen) / 4;
	if (height > gdk_screen_get_height (screen) / 4)
		height = gdk_screen_get_height (screen) / 4;

	if (aspect < 1.0)
		width  = height * aspect;
	else
		height = width / aspect;

	gtk_widget_set_size_request (GTK_WIDGET (curve),
			width + RADIUS * 2,
			height + RADIUS * 2);
}

static void
gl_curve_reset_vector (GlCurve *curve)
{
	g_free (curve->ctlpoint);

	curve->num_ctlpoints = 2;
	curve->ctlpoint = g_malloc (2 * sizeof (curve->ctlpoint[0]));
	curve->ctlpoint[0][0] = curve->priv->min_x;
	curve->ctlpoint[0][1] = curve->priv->min_y;
	curve->ctlpoint[1][0] = curve->priv->max_x;
	curve->ctlpoint[1][1] = curve->priv->max_y;

	if (curve->pixmap)
	{
		gint width, height;

		width = GTK_WIDGET (curve)->allocation.width - RADIUS * 2;
		height = GTK_WIDGET (curve)->allocation.height - RADIUS * 2;

		if (curve->curve_type == GL_CURVE_TYPE_FREE)
		{
			curve->curve_type = GL_CURVE_TYPE_LINEAR;
			gl_curve_interpolate (curve, width, height);
			curve->curve_type = GL_CURVE_TYPE_FREE;
		}
		else
			gl_curve_interpolate (curve, width, height);
		gl_curve_draw (curve, width, height);
	}
}

void
gl_curve_reset (GlCurve *c)
{
	GlCurveType old_type;

	old_type = c->curve_type;
	c->curve_type = GL_CURVE_TYPE_SPLINE;
	gl_curve_reset_vector (c);

	if (old_type != GL_CURVE_TYPE_SPLINE)
	{
		g_signal_emit (c, gl_curve_signals[GL_CURVE_SIGNAL_TYPE_CHANGE], 0);
		g_object_notify (G_OBJECT (c), "curve-type");
	}
}

void
gl_curve_set_gamma (GlCurve *c, gfloat gamma)
{
	gfloat x, one_over_gamma, height;
	GlCurveType old_type;
	gint i;

	if (c->num_points < 2)
		return;

	old_type = c->curve_type;
	c->curve_type = GL_CURVE_TYPE_FREE;

	if (gamma <= 0)
		one_over_gamma = 1.0;
	else
		one_over_gamma = 1.0 / gamma;
	height = c->height;
	for (i = 0; i < c->num_points; ++i)
	{
		x = (gfloat) i / (c->num_points - 1);
		c->point[i].x = RADIUS + i;
		c->point[i].y =
				RADIUS + (height * (1.0 - pow (x, one_over_gamma)) + 0.5);
	}

	if (old_type != GL_CURVE_TYPE_FREE)
		g_signal_emit (c, gl_curve_signals[GL_CURVE_SIGNAL_TYPE_CHANGE], 0);

	gl_curve_draw (c, c->num_points, c->height);
}

void
gl_curve_set_range (GlCurve *curve,
		gfloat    min_x,
		gfloat    max_x,
		gfloat    min_y,
		gfloat    max_y)
{
	g_object_freeze_notify (G_OBJECT (curve));
	if (curve->priv->min_x != min_x) {
		curve->priv->min_x = min_x;
		g_object_notify (G_OBJECT (curve), "min-x");
	}
	if (curve->priv->max_x != max_x) {
		curve->priv->max_x = max_x;
		g_object_notify (G_OBJECT (curve), "max-x");
	}
	if (curve->priv->min_y != min_y) {
		curve->priv->min_y = min_y;
		g_object_notify (G_OBJECT (curve), "min-y");
	}
	if (curve->priv->max_y != max_y) {
		curve->priv->max_y = max_y;
		g_object_notify (G_OBJECT (curve), "max-y");
	}
	g_object_thaw_notify (G_OBJECT (curve));

	gl_curve_size_graph (curve);
	gl_curve_reset_vector (curve);
}

void
gl_curve_set_vector (GlCurve *c, int veclen, gfloat vector[])
{
	GlCurveType old_type;
	gfloat rx, dx, ry;
	gint i, height;
	GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (c));

	old_type = c->curve_type;
	c->curve_type = GL_CURVE_TYPE_FREE;

	if (c->point)
		height = GTK_WIDGET (c)->allocation.height - RADIUS * 2;
	else
	{
		height = (c->priv->max_y - c->priv->min_y);
		if (height > gdk_screen_get_height (screen) / 4)
			height = gdk_screen_get_height (screen) / 4;

		c->height = height;
		c->num_points = veclen;
		c->point = g_malloc (c->num_points * sizeof (c->point[0]));
	}
	rx = 0;
	dx = (veclen - 1.0) / (c->num_points - 1.0);

	for (i = 0; i < c->num_points; ++i, rx += dx)
	{
		ry = vector[(int) (rx + 0.5)];
		if (ry > c->priv->max_y) ry = c->priv->max_y;
		if (ry < c->priv->min_y) ry = c->priv->min_y;
		c->point[i].x = RADIUS + i;
		c->point[i].y =
				RADIUS + height - project (ry, c->priv->min_y, c->priv->max_y, height);
	}
	if (old_type != GL_CURVE_TYPE_FREE)
	{
		g_signal_emit (c, gl_curve_signals[GL_CURVE_SIGNAL_TYPE_CHANGE], 0);
		g_object_notify (G_OBJECT (c), "curve-type");
	}

	gl_curve_draw (c, c->num_points, height);
}

gboolean
gl_curve_set_next_point (GlCurve *c, GdkPoint *p)
{
	static int next_point = 0;
	GlCurveType old_type;
	gint i, height;
	GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (c));
	i = 0;
	old_type = c->curve_type;
	c->curve_type = GL_CURVE_TYPE_FREE;

	if (c->point)
		height = GTK_WIDGET (c)->allocation.height - RADIUS * 2;
	else
	{
		height = (c->priv->max_y - c->priv->min_y);
		if (height > gdk_screen_get_height (screen) / 4)
			height = gdk_screen_get_height (screen) / 4;

		c->height = height;
		c->num_points = height;
		c->point = g_malloc (c->num_points * sizeof (c->point[0]));
	}
	if(next_point >= c->num_points ){  next_point = 0; return FALSE;}

	if (p->y > c->priv->max_y) p->y = c->priv->max_y;
	if (p->y < c->priv->min_y) p->y = c->priv->min_y;


	c->point[next_point].x =    RADIUS + i;
	c->point[next_point].y =	RADIUS + height - project (p->y, c->priv->min_y, c->priv->max_y, height);

	if (old_type != GL_CURVE_TYPE_FREE)
	{
		g_signal_emit (c, gl_curve_signals[GL_CURVE_SIGNAL_TYPE_CHANGE], 0);
		g_object_notify (G_OBJECT (c), "curve-type");
	}
	next_point++;
	gl_curve_draw (c, c->num_points, height);
	return TRUE;
}


void
gl_curve_get_vector (GlCurve *c, int veclen, gfloat vector[])
{
	gfloat rx, ry, dx, dy, min_x, delta_x, *mem, *xv, *yv, *y2v, prev;
	gint dst, i, x, next, num_active_ctlpoints = 0, first_active = -1;

	min_x = c->priv->min_x;

	if (c->curve_type != GL_CURVE_TYPE_FREE)
	{
		/* count active points: */
		prev = min_x - 1.0;
		for (i = num_active_ctlpoints = 0; i < c->num_ctlpoints; ++i)
			if (c->ctlpoint[i][0] > prev)
			{
				if (first_active < 0)
					first_active = i;
				prev = c->ctlpoint[i][0];
				++num_active_ctlpoints;
			}

		/* handle degenerate case: */
		if (num_active_ctlpoints < 2)
		{
			if (num_active_ctlpoints > 0)
				ry = c->ctlpoint[first_active][1];
			else
				ry = c->priv->min_y;
			if (ry < c->priv->min_y) ry = c->priv->min_y;
			if (ry > c->priv->max_y) ry = c->priv->max_y;
			for (x = 0; x < veclen; ++x)
				vector[x] = ry;
			return;
		}
	}

	switch (c->curve_type)
	{
	case GL_CURVE_TYPE_SPLINE:
		mem = g_malloc (3 * num_active_ctlpoints * sizeof (gfloat));
		xv  = mem;
		yv  = mem + num_active_ctlpoints;
		y2v = mem + 2*num_active_ctlpoints;

		prev = min_x - 1.0;
		for (i = dst = 0; i < c->num_ctlpoints; ++i)
			if (c->ctlpoint[i][0] > prev)
			{
				prev    = c->ctlpoint[i][0];
				xv[dst] = c->ctlpoint[i][0];
				yv[dst] = c->ctlpoint[i][1];
				++dst;
			}

		spline_solve (num_active_ctlpoints, xv, yv, y2v);

		rx = min_x;
		dx = (c->priv->max_x - min_x) / (veclen - 1);
		for (x = 0; x < veclen; ++x, rx += dx)
		{
			ry = spline_eval (num_active_ctlpoints, xv, yv, y2v, rx);
			if (ry < c->priv->min_y) ry = c->priv->min_y;
			if (ry > c->priv->max_y) ry = c->priv->max_y;
			vector[x] = ry;
		}

		g_free (mem);
		break;

	case GL_CURVE_TYPE_LINEAR:
		dx = (c->priv->max_x - min_x) / (veclen - 1);
		rx = min_x;
		ry = c->priv->min_y;
		dy = 0.0;
		i  = first_active;
		for (x = 0; x < veclen; ++x, rx += dx)
		{
			if (rx >= c->ctlpoint[i][0])
			{
				if (rx > c->ctlpoint[i][0])
					ry = c->priv->min_y;
				dy = 0.0;
				next = i + 1;
				while (next < c->num_ctlpoints
						&& c->ctlpoint[next][0] <= c->ctlpoint[i][0])
					++next;
				if (next < c->num_ctlpoints)
				{
					delta_x = c->ctlpoint[next][0] - c->ctlpoint[i][0];
					dy = ((c->ctlpoint[next][1] - c->ctlpoint[i][1])
							/ delta_x);
					dy *= dx;
					ry = c->ctlpoint[i][1];
					i = next;
				}
			}
			vector[x] = ry;
			ry += dy;
		}
		break;

	case GL_CURVE_TYPE_FREE:
		if (c->point)
		{
			rx = 0.0;
			dx = c->num_points / (double) veclen;
			for (x = 0; x < veclen; ++x, rx += dx)
				vector[x] = unproject (RADIUS + c->height - c->point[(int) rx].y,
						c->priv->min_y, c->priv->max_y,
						c->height);
		}
		else
			memset (vector, 0, veclen * sizeof (vector[0]));
		break;
	}
}

GtkWidget*
gl_curve_new (void)
{
	return g_object_new (GL_TYPE_CURVE, NULL);
}

static void
gl_curve_finalize (GObject *object)
{
	GlCurve *curve;

	g_return_if_fail (GL_IS_CURVE (object));

	curve = GL_CURVE (object);
	if (curve->pixmap)
		g_object_unref (curve->pixmap);
	g_free (curve->point);
	g_free (curve->ctlpoint);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}


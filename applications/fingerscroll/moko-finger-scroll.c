
#include "moko-finger-scroll.h"

G_DEFINE_TYPE (MokoFingerScroll, moko_finger_scroll, GTK_TYPE_SCROLLED_WINDOW)
#define FINGER_SCROLL_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_FINGER_SCROLL, MokoFingerScrollPrivate))
typedef struct _MokoFingerScrollPrivate MokoFingerScrollPrivate;

struct _MokoFingerScrollPrivate {
	MokoFingerScrollMode mode;
	gdouble x;
	gdouble y;
	gboolean enabled;
	gboolean clicked;
	gboolean moved;
	GTimeVal click_start;
	gdouble vmin;
	gdouble vmax;
	guint sps;
	gdouble vel_x;
	gdouble vel_y;
};

enum {
	PROP_ENABLED = 1,
	PROP_MODE,
	PROP_VELOCITY_MIN,
	PROP_VELOCITY_MAX,
	PROP_SPS,
};

static gboolean
moko_finger_scroll_button_press_cb (GtkWidget *widget, GdkEventButton *event,
				    MokoFingerScroll *scroll)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);

	if ((!priv->enabled) || (priv->clicked) || (event->button != 1))
		return FALSE;

	g_get_current_time (&priv->click_start);
	priv->x = (gint)event->x;
	priv->y = (gint)event->y;
	priv->moved = FALSE;
	priv->clicked = TRUE;
	
	return TRUE;
}

static void
moko_finger_scroll_scroll (MokoFingerScroll *scroll, gdouble x, gdouble y)
{
	gdouble h, v;
	GtkAdjustment *hadjust = gtk_scrolled_window_get_hadjustment (
		GTK_SCROLLED_WINDOW (scroll));
	GtkAdjustment *vadjust = gtk_scrolled_window_get_vadjustment (
		GTK_SCROLLED_WINDOW (scroll));
	
	h = MIN (hadjust->upper - hadjust->page_size,
		MAX (hadjust->lower, gtk_adjustment_get_value (hadjust) - x));
	v = MIN (vadjust->upper - vadjust->page_size,
		MAX (vadjust->lower, gtk_adjustment_get_value (vadjust) - y));

	gtk_adjustment_set_value (hadjust, h);
	gtk_adjustment_set_value (vadjust, v);
}

static gboolean
moko_finger_scroll_timeout (MokoFingerScroll *scroll)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);
	
	if ((!priv->clicked) || (!priv->enabled) ||
	    (priv->mode != MOKO_FINGER_SCROLL_MODE_ACCEL)) return FALSE;
	
	moko_finger_scroll_scroll (scroll, priv->vel_x, priv->vel_y);
	
	return TRUE;
}

static gboolean
moko_finger_scroll_motion_notify_cb (GtkWidget *widget, GdkEventMotion *event,
				     MokoFingerScroll *scroll)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);
	gint dnd_threshold;
	gdouble x, y;

	if ((!priv->enabled) || (!priv->clicked)) return FALSE;

	g_object_get (G_OBJECT (gtk_settings_get_default ()),
		"gtk-dnd-drag-threshold", &dnd_threshold, 0);
	x = event->x - priv->x;
	y = event->y - priv->y;
	
	if ((!priv->moved) && (
	     (ABS (x) > dnd_threshold) || (ABS (y) > dnd_threshold))) {
		priv->moved = TRUE;
		if (priv->mode == MOKO_FINGER_SCROLL_MODE_ACCEL) {
			g_timeout_add (priv->sps,
				(GSourceFunc)moko_finger_scroll_timeout,
				scroll);
		}
	}
	
	if (priv->moved) {
		switch (priv->mode) {
		    case MOKO_FINGER_SCROLL_MODE_PUSH :
			moko_finger_scroll_scroll (scroll, x, y);
			priv->x = event->x;
			priv->y = event->y;
			break;
		    case MOKO_FINGER_SCROLL_MODE_ACCEL :
			priv->vel_x = (((x > 0) ? -1 : 1) *
				(ABS (x) /
				 (gdouble)widget->allocation.width) *
				(priv->vmax-priv->vmin));
			priv->vel_y = (((y > 0) ? -1 : 1) *
				(ABS (y) /
				 (gdouble)widget->allocation.height) *
				(priv->vmax-priv->vmin));
			break;
		    default :
			break;
		}
	}
	
	return TRUE;
}

static gboolean
moko_finger_scroll_button_release_cb (GtkWidget *widget, GdkEventButton *event,
				      MokoFingerScroll *scroll)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (scroll);
	GTimeVal current;
		
	if ((!priv->enabled) || (event->button != 1))
		return FALSE;

	g_get_current_time (&current);

	if ((!priv->moved) &&
	    ((current.tv_sec > priv->click_start.tv_sec) ||
	    (current.tv_usec - priv->click_start.tv_usec > 500))) {
		/* Send synthetic click event */
		event->type = GDK_BUTTON_PRESS;
		gtk_widget_event (widget, (GdkEvent *)event);
		priv->clicked = FALSE;
		event->type = GDK_BUTTON_RELEASE;
		gtk_widget_event (widget, (GdkEvent *)event);
	}
	priv->clicked = FALSE;

	return TRUE;
}

static void
moko_finger_scroll_add (GtkContainer *container,
			GtkWidget    *child)
{
	GTK_CONTAINER_CLASS (moko_finger_scroll_parent_class)->add (
		container, child);

	g_signal_connect (G_OBJECT (child), "button-press-event",
		G_CALLBACK (moko_finger_scroll_button_press_cb), container);
	g_signal_connect (G_OBJECT (child), "button-release-event",
		G_CALLBACK (moko_finger_scroll_button_release_cb), container);
	g_signal_connect (G_OBJECT (child), "motion-notify-event",
		G_CALLBACK (moko_finger_scroll_motion_notify_cb), container);
}

static void
moko_finger_scroll_remove (GtkContainer *container,
			   GtkWidget    *child)
{
	GTK_CONTAINER_CLASS (moko_finger_scroll_parent_class)->remove (
		container, child);
	
	g_signal_handlers_disconnect_by_func (child, 
		moko_finger_scroll_button_press_cb, container);
	g_signal_handlers_disconnect_by_func (child, 
		moko_finger_scroll_button_release_cb, container);
	g_signal_handlers_disconnect_by_func (child, 
		moko_finger_scroll_motion_notify_cb, container);
}

static void
moko_finger_scroll_get_property (GObject * object, guint property_id,
				 GValue * value, GParamSpec * pspec)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (object);

	switch (property_id) {
	    case PROP_ENABLED :
		g_value_set_boolean (value, priv->enabled);
		break;
	    case PROP_MODE :
		g_value_set_int (value, priv->mode);
		break;
	    case PROP_VELOCITY_MIN :
		g_value_set_double (value, priv->vmin);
		break;
	    case PROP_VELOCITY_MAX :
		g_value_set_double (value, priv->vmax);
		break;
	    case PROP_SPS :
		g_value_set_uint (value, priv->sps);
		break;
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
moko_finger_scroll_set_property (GObject * object, guint property_id,
				 const GValue * value, GParamSpec * pspec)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (object);

	switch (property_id) {
	    case PROP_ENABLED :
		priv->enabled = g_value_get_boolean (value);
		break;
	    case PROP_MODE :
		priv->mode = g_value_get_int (value);
		break;
	    case PROP_VELOCITY_MIN :
		priv->vmin = g_value_get_double (value);
		break;
	    case PROP_VELOCITY_MAX :
		priv->vmax = g_value_get_double (value);
		break;
	    case PROP_SPS :
		priv->sps = g_value_get_uint (value);
		break;
	    default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
moko_finger_scroll_dispose (GObject * object)
{
	if (G_OBJECT_CLASS (moko_finger_scroll_parent_class)->dispose)
		G_OBJECT_CLASS (moko_finger_scroll_parent_class)->
			dispose (object);
}

static void
moko_finger_scroll_finalize (GObject * object)
{
	G_OBJECT_CLASS (moko_finger_scroll_parent_class)->finalize (object);
}

static void
moko_finger_scroll_class_init (MokoFingerScrollClass * klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

	g_type_class_add_private (klass, sizeof (MokoFingerScrollPrivate));

	object_class->get_property = moko_finger_scroll_get_property;
	object_class->set_property = moko_finger_scroll_set_property;
	object_class->dispose = moko_finger_scroll_dispose;
	object_class->finalize = moko_finger_scroll_finalize;
	
	container_class->add = moko_finger_scroll_add;
	container_class->remove = moko_finger_scroll_remove;

	g_object_class_install_property (
		object_class,
		PROP_ENABLED,
		g_param_spec_boolean (
			"enabled",
			"Enabled",
			"Enable or disable finger-scroll.",
			TRUE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (
		object_class,
		PROP_MODE,
		g_param_spec_int (
			"mode",
			"Scroll mode",
			"Change the finger-scrolling mode.",
			MOKO_FINGER_SCROLL_MODE_PUSH,
			MOKO_FINGER_SCROLL_MODE_ACCEL,
			MOKO_FINGER_SCROLL_MODE_ACCEL,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (
		object_class,
		PROP_VELOCITY_MIN,
		g_param_spec_double (
			"velocity_min",
			"Minimum scroll velocity",
			"Minimum distance the child widget should scroll "
				"per 'frame', in pixels.",
			0, G_MAXDOUBLE, 1,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (
		object_class,
		PROP_VELOCITY_MAX,
		g_param_spec_double (
			"velocity_max",
			"Maximum scroll velocity",
			"Minimum distance the child widget should scroll "
				"per 'frame', in pixels.",
			0, G_MAXDOUBLE, 10,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (
		object_class,
		PROP_SPS,
		g_param_spec_uint (
			"sps",
			"Scrolls per second",
			"Amount of scroll events to generate per second.",
			0, G_MAXUINT, 15,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
moko_finger_scroll_init (MokoFingerScroll * self)
{
	MokoFingerScrollPrivate *priv = FINGER_SCROLL_PRIVATE (self);
	
	priv->x = 0;
	priv->y = 0;
	priv->moved = FALSE;
	priv->clicked = FALSE;	

	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (self),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
}

GtkWidget *
moko_finger_scroll_new (void)
{
	return g_object_new (MOKO_TYPE_FINGER_SCROLL, NULL);
}

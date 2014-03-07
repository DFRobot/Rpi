/*
 by: lisper <leyapin@gmail.com>
 created: 2013-10-29
 this program is writen by C with gtk3 and cairo

*/


#include <gtk/gtk.h>
#include <string.h>

#define CAIRO_HAS_PNG_FUNCTIONS

GSList *draw_list = NULL;
cairo_surface_t *image ;

int win_width;
int win_height;

int old_x;
int old_y;

int px, py;

int ctrl_key_state;

char *name = NULL;

GdkRGBA draw_color = {1, 0, 0, 1};
GdkRGBA bg_color = {1, 1, 0.6, 1};
gint draw_width = 4;

/* Surface to store current scribbles */
static cairo_surface_t *surface = NULL;


//
void close_window (void) {
	if (surface)
		cairo_surface_destroy (surface);
	gtk_main_quit ();
}

//
static void clear_surface (void) {
	cairo_t *cr;
	cr = cairo_create (surface);
	cairo_set_source_rgba (cr, bg_color.red, bg_color.green, bg_color.blue, bg_color.alpha); //1 1  0.5
	cairo_paint (cr);
	cairo_destroy (cr);
}

static void draw_logo (void) {
	cairo_t *cr;
	cr = cairo_create (surface);

	cairo_set_source_surface (cr, image, 0, 0);
	cairo_paint (cr);
	cairo_destroy (cr);
}


static void color_button_cb (GObject *o, GParamSpec *pspect,  gpointer data) {
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(o), &draw_color);
	//  g_print ("color changed: %g %g %g %g\n", rgba.red, rgba.green, rgba.blue, rgba.alpha);
}


static void bg_button_cb (GObject *o, GParamSpec *pspect,  gpointer data) {
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(o), &bg_color);
	clear_surface ();
	//  g_print ("color changed: %g %g %g %g\n", rgba.red, rgba.green, rgba.blue, rgba.alpha);
}


static void full_cb (GtkWidget* widget, gpointer data) {

}

static void scale_size_cb  (GtkAdjustment *get, gpointer data) {
	g_print ("->%f\n", gtk_adjustment_get_value (get));
	draw_width = gtk_adjustment_get_value (get);
}

static void clear_button_cb (GtkWidget*widget, gpointer data) {
	clear_surface ();
	gtk_widget_queue_draw (GTK_WIDGET(data));
}

int str_end_with (char* str, char* data) {
        int leng1 = strlen (str)-1;
        //g_print ("--<%c,%c\n", data[j], str[i]);
        int leng2 = strlen (data)-1;
        int i=leng1, j=leng2;
        for (; i>= 0 && j>= 0; i--, j--) {
                if (data[j] != str[i]) {
                        break;
                }
        }
        if (j < 0) {
                return TRUE;
        }
        else {
                return FALSE;
        }
}

void save_image (GtkWidget* widget) {
	GtkWidget *save_dialog;

	save_dialog = gtk_file_chooser_dialog_new ("Save File",
	                widget,
	                GTK_FILE_CHOOSER_ACTION_SAVE,
	                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
	                NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (save_dialog), TRUE);

	if (gtk_dialog_run (GTK_DIALOG (save_dialog)) == GTK_RESPONSE_ACCEPT) {
                         char namebuffer[100];
		char *filename;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (save_dialog));
		strcpy (namebuffer, filename);
		if (!str_end_with(namebuffer, ".png"))
                        strcat (namebuffer, ".png");
		cairo_surface_write_to_png(surface, namebuffer);
		g_free (filename);
	}
	gtk_widget_destroy (save_dialog);
}

/* Create a new surface of the appropriate size to store our scribbles */
static gboolean configure_event_cb (
        GtkWidget *widget,
        GdkEventConfigure *event,
        gpointer data) {

	g_print ("helo");
	if (surface)
		cairo_surface_destroy (surface);
//cairo_scale (surface, gtk_widget_get_allocated_width (widget),
//	                  gtk_widget_get_allocated_height (widget));


	surface = gdk_window_create_similar_surface (
	                  gtk_widget_get_window (widget),
	                  CAIRO_CONTENT_COLOR,
	                  gtk_widget_get_allocated_width (widget),
	                  gtk_widget_get_allocated_height (widget));

	//g_print ("in configure\n");
	/* Initialize the surface to white */
	static int first_time = 1;
	if (first_time == 1)
		draw_logo ();
	else
		clear_surface ();

	first_time++;

	win_width = gtk_widget_get_allocated_width (widget);
	win_height = gtk_widget_get_allocated_height (widget);

	/* We've handled the configure event, no need for further processing. */
	return TRUE;
}

/* Redraw the screen from the surface. Note that the ::draw
 * signal receives a ready-to-be-used cairo_t that is already
 * clipped to only draw the exposed areas of the widget
 */
static gboolean draw_cb (GtkWidget *widget,
                         cairo_t   *cr,
                         gpointer   data) {
	cairo_set_source_surface (cr, surface, 0, 0);
	cairo_paint (cr);

	return FALSE;
}


/* Draw a rectangle on the surface at the given position */
static void draw_brush (GtkWidget *widget,
                        gdouble    x,
                        gdouble    y) {
	cairo_t *cr;

	/* Paint to the surface, where we store our state */
	cr = cairo_create (surface);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND); //
	cairo_set_source_rgba (cr, draw_color.red, draw_color.green, draw_color.blue, draw_color.alpha);
	cairo_set_line_width (cr, draw_width);
	//cairo_rectangle (cr, x, y, 30, 30);
	cairo_move_to (cr, old_x, old_y);
	cairo_line_to (cr, x, y);
	// cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

	//cairo_fill (cr);
	cairo_stroke (cr);
	cairo_destroy (cr);

	/* Now invalidate the affected region of the drawing area. */
	//gtk_widget_queue_draw_area (widget, 0, 0, win_width, win_height);
	gtk_widget_queue_draw_area (widget, MIN(x, px)-draw_width, MIN(y, py)-draw_width, ABS(x-px)+draw_width*2, ABS(y-py)+draw_width*2);
	old_x = x;
	old_y = y;
	px = x;
	py = y;
}

/* Handle button press events by either drawing a rectangle
 * or clearing the surface, depending on which button was pressed.
 * The ::button-press signal handler receives a GdkEventButton
 * struct which contains this information.
 */
static gboolean button_press_event_cb (GtkWidget      *widget,
                                       GdkEventButton *event,
                                       gpointer        data) {

	/* paranoia check, in case we haven't gotten a configure event */
	if (surface == NULL)
		return FALSE;

	old_x = event->x;
	old_y = event->y;
	px = event->x;
	py = event->y;

//	if (event->button == GDK_BUTTON_PRIMARY && (ctrl_key_state == 0)) {
//		draw_brush (widget, event->x, event->y);
//	}
	if (event->button == GDK_BUTTON_SECONDARY) {
		if (!(event->state & GDK_BUTTON1_MASK)) {
			//clear_surface ();
			//gtk_widget_queue_draw (widget);
		}
	} else if (event->button == GDK_BUTTON_MIDDLE) {
		close_window ();
	}



	/* We've handled the event, stop processing */
	return TRUE;
}

/* Handle motion events by continuing to draw if button 1 is
 * still held down. The ::motion-notify signal handler receives
 * a GdkEventMotion struct which contains this information.
 */
//
static gboolean motion_notify_event_cb (GtkWidget      *widget,
                                        GdkEventMotion *event,
                                        gpointer        data) {
	//ctrl_key_state = 1;

	/* paranoia check, in case we haven't gotten a configure event */
	if (surface == NULL)
		return FALSE;

	if (event->state & GDK_BUTTON1_MASK && ctrl_key_state == 0 && !(event->state & GDK_BUTTON3_MASK)) {
		draw_brush (widget, event->x, event->y);
		//	g_print ("%f %f	%f %f\n", event->x, event->y, event->x_root, event->y_root);
	} else if (event->state & GDK_BUTTON1_MASK && ((ctrl_key_state == 1) || (event->state & GDK_BUTTON3_MASK))) {
		//gtk_window_move (GTK_WINDOW (data), );
		//g_print ("%d\n", __LINE__);
		gtk_window_move (GTK_WINDOW (data), event->x_root-px, event->y_root-py);
	}

	/* We've handled it, stop processing */
	return TRUE;
}

void redo_draw () {
	cairo_restore (surface);
	if (draw_list) {

	}
}

void undo_draw () {

}

void get_draw () {
	cairo_save (surface);
	//list = g_list_prepend(list, sur)
}

gboolean on_key_press_event (GtkWidget *window, GdkEventKey *event, gpointer data) {
	//g_print ("%x  ", event->keyval);
//	g_print ("%x ", event->state);
//	g_print ("%s\n", gdk_keyval_name (event->keyval));

//	g_print ("%x %x %x\n", event->keyval, event->state,  GDK_CONTROL_MASK);
	if (event->state & GDK_CONTROL_MASK) {
		ctrl_key_state = 1;
	}

	switch (event->keyval) {
	case 0xffe3:
	case 0xffe4:
		ctrl_key_state = 1;
		break;
	case GDK_KEY_q:
	case GDK_KEY_Q:
		if (ctrl_key_state == 1)
			close_window();
		break;
	case GDK_KEY_x:
	case GDK_KEY_X:
		if (ctrl_key_state == 1)
			gtk_window_resize (GTK_WINDOW (data), 290, 290);
	case GDK_KEY_z:
	case GDK_KEY_Z:
		redo_draw ();
		break;
	case GDK_KEY_y:
	case GDK_KEY_Y:
		undo_draw ();
		break;
	case GDK_KEY_p:
	case GDK_KEY_P:
		get_draw ();
		break;

	case GDK_KEY_s:
	case GDK_KEY_S:
		save_image(window);
		break;
	}

	// g_printf ("%c, %d\n", event->keyval, event->keyval);
}

//
gboolean on_key_release_event (GtkWidget *window, GdkEventKey *event, gpointer data) {
	ctrl_key_state = 0;

	// g_printf ("%c, %d\n", event->keyval, event->keyval);
}

int main (int argc, char *argv[]) {
	GtkWidget *window;	// main window
//	GtkWidget *frame;	// frame
	GtkWidget *da;		// drawing area
	GtkWidget *box;
	GtkWidget *button_box;
	GtkWidget *scale_size;
	GtkWidget*clear_button;
	GtkWidget* quit_button;
	//GtkWidget* full_button; ////

	gtk_init (&argc, &argv);	//init gtk

        char namebuffer[100];
        char iconbuffer[100];

	int length = strlen (argv[0]);

	strcpy (namebuffer, argv[0]);

	for (int i=length; namebuffer[i] != '/'; i--) {
		namebuffer[i] = '\0';
	}
	strcpy (iconbuffer, namebuffer);
	strcat (namebuffer, "logo.png");
	printf (namebuffer);
	image = cairo_image_surface_create_from_png (namebuffer);

        strcat (iconbuffer, "logo.png");
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_window_set_icon (GTK_WINDOW(window), gdk_pixbuf_new_from_file (iconbuffer,NULL));
	//gtk_window_set_icon (GTK_WINDOW(window), gdk_pixbuf_new_from_file (namebuffer,NULL));

	gtk_window_set_title (GTK_WINDOW (window), "DFRobot");
//	gtk_container_set_border_width (GTK_CONTAINER (window), 1);
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
	gtk_window_fullscreen(window);
	gtk_window_set_keep_above (GTK_WINDOW (window), TRUE);
//	frame = gtk_frame_new (NULL);
//	gtk_frame_set_label (GTK_FRAME (frame), get_name ());
//	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

	GtkWidget *color_button = gtk_color_button_new ();
	GtkWidget *bg_button = gtk_color_button_new ();

        clear_button = gtk_button_new_with_label("clear");
	//full_button = gtk_button_new_with_label("+");   ////
	quit_button = gtk_button_new_with_label("X");

	gtk_color_button_set_rgba (color_button, &draw_color);
	gtk_color_button_set_rgba (bg_button, &bg_color);
	//gtk_window_set_skip_taskbar_hint (GTK_WINDOW(window), TRUE);
	//gtk_window_set_urgency_hint (GTK_WINDOW(window), FALSE);
	// gtk_window_set_accept_focus (GTK_WINDOW (window), TRUE);


	gtk_window_set_opacity (GTK_WINDOW (window), 0.9);
	da = gtk_drawing_area_new ();

	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 3);
	button_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);

	GtkWidget* adj = gtk_adjustment_new(draw_width, 1, 50.0, 1.0, 1.0, 0.0);
	g_signal_connect (adj, "value-changed", G_CALLBACK (scale_size_cb), NULL);

	scale_size = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, adj);

	gtk_box_pack_start (GTK_BOX (box), da, TRUE, TRUE, 0);

	gtk_box_pack_start (GTK_BOX (box), button_box, FALSE, TRUE, 1);
	gtk_box_pack_start(GTK_BOX (button_box), color_button, FALSE, TRUE, 1);
	gtk_box_pack_start(GTK_BOX (button_box), bg_button, FALSE, TRUE, 1);
	//gtk_box_pack_start(GTK_BOX (button_box), full_button, FALSE, TRUE, 1);
	gtk_box_pack_start(GTK_BOX (button_box), clear_button, FALSE, TRUE, 1);
	gtk_box_pack_start(GTK_BOX (button_box), quit_button, FALSE, TRUE, 1);
        gtk_box_pack_start(GTK_BOX (button_box), scale_size, TRUE, TRUE, 1);


	gtk_container_add (GTK_CONTAINER (window), box);
	/* set a minimum size */

	gtk_widget_set_size_request (da, 20, 20);
	gtk_window_set_default_size (GTK_WINDOW (window), 290, 290);
	//gtk_container_add (GTK_CONTAINER (frame), da);
	g_signal_connect (window, "destroy", G_CALLBACK (close_window), NULL);
	/* Signals used to handle the backing surface */
	g_signal_connect (da, "draw", G_CALLBACK (draw_cb), NULL);
	g_signal_connect (da,"configure-event",	G_CALLBACK (configure_event_cb), NULL);

	/* Event signals */

	g_signal_connect (da, "motion-notify-event",
	                  G_CALLBACK (motion_notify_event_cb), (gpointer)window);
	g_signal_connect (da, "button-press-event",
	                  G_CALLBACK (button_press_event_cb), NULL);

	g_signal_connect (window, "key-press-event", G_CALLBACK (on_key_press_event), (gpointer) window);
	g_signal_connect (window, "key-release-event", G_CALLBACK (on_key_release_event), NULL);
	g_signal_connect (color_button, "notify::color", G_CALLBACK (color_button_cb), NULL);
	g_signal_connect (bg_button, "notify::color", G_CALLBACK (bg_button_cb), NULL);
	g_signal_connect (clear_button, "clicked", G_CALLBACK (clear_button_cb), da);
	g_signal_connect (quit_button, "clicked", G_CALLBACK (gtk_main_quit), NULL);
	//g_signal_connect (scale_size, "value-changed", G_CALLBACK (scale_size_cb), NULL);     //error

	gtk_widget_set_events (da, gtk_widget_get_events (da)
	                       | GDK_BUTTON_PRESS_MASK
	                       | GDK_POINTER_MOTION_MASK
	                      );


	gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
	gtk_widget_show_all (window);

	gtk_main ();

	return 0;
}

/*	by lisper <leyapin@gmail.com>
 *	create:	2014-02-21
 *	use Respberry Pi control led by i2c
 *	the program is writen by c with gtk+-3.0
 *
 */

#include <gtk/gtk.h>

GtkWidget* open_entry;
GtkWidget* close_entry;

//
static void switch_button_cb () {
	static int state = FALSE;
	g_print ("%d\n", state = !state);
	if (state) {
		g_print (gtk_entry_get_text(GTK_ENTRY(open_entry)));
		system (gtk_entry_get_text(GTK_ENTRY(open_entry)));
	} else {
		g_print (gtk_entry_get_text(GTK_ENTRY(close_entry)));
		system (gtk_entry_get_text(GTK_ENTRY(close_entry)));
	}
}


//
int main (int argc, char **argv) {

	/*-- CSS ------------------*/
	GtkCssProvider *provider;
	GdkDisplay *display;
	GdkScreen *screen;
	/*---------------------------*/
	GtkWidget* window;
	GtkWidget* switch_button;

	GtkWidget* title_label;
	GtkWidget* box;
	GtkWidget* end_label;

	gtk_init (&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER (window), 3);
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	open_entry = gtk_entry_new();
	close_entry = gtk_entry_new();
	gtk_entry_set_text (open_entry, "i2cset -y 1 0x20 0x06 0x0");
	gtk_entry_set_text (close_entry, "i2sget -y 1 0x20 0x06 0xff");
	title_label = gtk_label_new("Switch Test");

	end_label = gtk_label_new("From DFRobot");
	switch_button = gtk_switch_new();


	gtk_box_pack_start(GTK_BOX(box), title_label, FALSE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(box), open_entry, TRUE, TRUE, 1);
	gtk_box_pack_start(GTK_BOX(box), close_entry, TRUE, TRUE, 1);
	gtk_box_pack_start(GTK_BOX(box), switch_button, TRUE, TRUE, 1);
	gtk_box_pack_start(GTK_BOX(box), end_label, FALSE, TRUE, 3);

	gtk_container_add(GTK_CONTAINER(window), box);

	g_signal_connect (switch_button, "notify::active", G_CALLBACK (switch_button_cb), NULL);

	/*---------------- CSS ------------------------------------------------------*/
	provider = gtk_css_provider_new ();
	display = gdk_display_get_default ();
	screen = gdk_display_get_default_screen (display);
	gtk_style_context_add_provider_for_screen (screen, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	gsize bytes_written, bytes_read;

	const gchar* home = "./picss.css";
	GError *error = 0;
	gtk_css_provider_load_from_path (provider,
			g_filename_to_utf8(home, strlen(home), &bytes_read, &bytes_written, &error),
			NULL);
	g_object_unref (provider);

	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}

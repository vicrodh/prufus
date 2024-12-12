#include <gtk/gtk.h>

static void
print_hello (GtkWidget *widget,
             gpointer   data)
{
  g_print ("Creating booteable USB....\n");
  sleep(2);
  g_print ("Success\n");
  g_print ("You can disconnect your USB now\n");
}

static void
activate (GtkApplication *app,
          gpointer        user_data)
{
  GtkWidget *window;
  GtkWidget *button;
  GtkWidget *box;
  GtkWidget* title;
  GtkWidget* description;
  GtkWidget* header_box;
  GtkWidget* action_box;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "prufus");
  gtk_window_set_default_size (GTK_WINDOW (window), 500, 650);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign (box, GTK_ALIGN_FILL);
  gtk_widget_set_valign (box, GTK_ALIGN_FILL);

  header_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign (header_box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (header_box, GTK_ALIGN_CENTER);

  action_box= gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign (action_box, GTK_ALIGN_END);
  gtk_widget_set_valign (action_box, GTK_ALIGN_END);


  gtk_window_set_child (GTK_WINDOW (window), box);


  //create UI
  button = gtk_button_new_with_label ("Create booteable USB");

  title = gtk_label_new("prufus");
  PangoAttrList *const Attrs = pango_attr_list_new();
  PangoAttribute *const SizeAttr = pango_attr_size_new(40 * PANGO_SCALE);
  pango_attr_list_insert(Attrs, SizeAttr);
  gtk_label_set_attributes((GtkLabel*)title,Attrs);

  description = gtk_label_new("Create booteable USB from .iso images");

  g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
  
  gtk_box_append (GTK_BOX (box), header_box);

  gtk_box_append (GTK_BOX (header_box), title);
  gtk_box_append (GTK_BOX (header_box), description);
  gtk_box_append (GTK_BOX (action_box), button);
  gtk_box_append (GTK_BOX (box), action_box);

  gtk_window_present (GTK_WINDOW (window));
}

int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.prufus", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}


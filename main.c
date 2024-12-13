#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

typedef struct Disk{
  char name[40];
  char device[20];
  char size[20];
} Disk;


char* devices[10];

char buffer_disk_name[128];

char buffer_disk_full_name[60*10];

GtkWidget* selected_iso_label;
GtkWidget* disk_label;

Disk disks[10];
int disk_counter = 0;
#define NAME    1
#define DEVICE  2
#define SIZE    3
#define DEVICE_COUNT    4

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
select_file_result(GObject      *source_object,
         GAsyncResult *res,
         gpointer      user_data)
{
  GFile* my_iso = gtk_file_dialog_open_finish((GtkFileDialog*)source_object,res,NULL);
  char* my_iso_path = g_file_get_path(my_iso);
  g_print("chossed\n");
  g_print(my_iso_path);
  gtk_label_set_text((GtkLabel*)selected_iso_label,my_iso_path);

}

static void
choose_iso(GtkWidget *widget,
             gpointer   data)
{
  gtk_file_dialog_set_title((GtkFileDialog*)data,"prufus");
  gtk_file_dialog_open((GtkFileDialog*)data, NULL, NULL, select_file_result ,NULL);
  g_print ("Choose your iso\n");
}

static void
activate (GtkApplication *app,
          gpointer        user_data)
{
  GtkWidget *window;
  GtkWidget *button;
  GtkWidget *choose_iso_button;
  GtkWidget *box;
  GtkWidget* title;
  GtkWidget* description;
  GtkWidget* header_box;
  GtkWidget* action_box;
  GtkWidget* select_iso_box;

  GtkWidget* select_iso_drop_down;

  GtkWidget* separator;

  

  GtkFileDialog* choose_iso_dialog;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "prufus");
  gtk_window_set_default_size (GTK_WINDOW (window), 500, 650);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_widget_set_halign (box, GTK_ALIGN_FILL);
  gtk_widget_set_valign (box, GTK_ALIGN_FILL);

  header_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign (header_box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (header_box, GTK_ALIGN_CENTER);

  action_box= gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign (action_box, GTK_ALIGN_END);
  gtk_widget_set_valign (action_box, GTK_ALIGN_END);

  select_iso_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_halign (select_iso_box, GTK_ALIGN_FILL);
  gtk_widget_set_valign (select_iso_box, GTK_ALIGN_FILL);

  gtk_window_set_child (GTK_WINDOW (window), box);

  
  separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);


  choose_iso_dialog = gtk_file_dialog_new();

  select_iso_drop_down = gtk_drop_down_new(NULL, NULL);
  gtk_drop_down_set_factory(GTK_DROP_DOWN(select_iso_drop_down), NULL);

  selected_iso_label = gtk_label_new("None .iso selected");
  
  //disk_label = gtk_label_new(buffer_disk_name);

  //create UI
  button = gtk_button_new_with_label ("Create booteable USB");
  choose_iso_button = gtk_button_new_with_label ("Select .iso");

  title = gtk_label_new("prufus");
  PangoAttrList *const Attrs = pango_attr_list_new();
  PangoAttribute *const SizeAttr = pango_attr_size_new(40 * PANGO_SCALE);
  pango_attr_list_insert(Attrs, SizeAttr);
  gtk_label_set_attributes((GtkLabel*)title,Attrs);

  description = gtk_label_new("Create booteable USB from .iso images");

  int offset = 0;
  for (int i = 0; i < disk_counter; i++) {
    devices[i] = buffer_disk_full_name + offset;
    offset += 60;
  }
  GtkWidget* devices_drop_down = gtk_drop_down_new_from_strings((const char* const*)devices);





  g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
  g_signal_connect (choose_iso_button, "clicked", G_CALLBACK (choose_iso), choose_iso_dialog);
  
  gtk_box_append (GTK_BOX (box), header_box);

  gtk_box_append (GTK_BOX (header_box), title);
  gtk_box_append (GTK_BOX (header_box), description);
  gtk_box_append (GTK_BOX (action_box), button);
  
  gtk_box_append (GTK_BOX (select_iso_box), choose_iso_button);
  gtk_box_append (GTK_BOX (select_iso_box), selected_iso_label);

  gtk_box_append (GTK_BOX (box), select_iso_box);

  
  GtkWidget* box_disk = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); 
  
  GtkWidget* separator_disk1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  GtkWidget* box_disk_label = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); 
  gtk_box_append (GTK_BOX (box_disk_label), separator_disk1);

  //gtk_box_append (GTK_BOX (box_disk_label), disk_label);
  GtkWidget* separator_disk2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_append (GTK_BOX (box_disk_label), separator_disk2);


  GtkWidget* disk_button = gtk_button_new_with_label("Change destination USB");
  gtk_box_append (GTK_BOX (box_disk), disk_button);
  gtk_box_append (GTK_BOX (box_disk), box_disk_label);


  gtk_box_append (GTK_BOX (box), box_disk);
  gtk_box_append (GTK_BOX (box), devices_drop_down);

  //gtk_box_append (GTK_BOX (box), select_iso_drop_down);
  gtk_box_append (GTK_BOX (box), action_box);

  gtk_window_set_resizable(GTK_WINDOW(window),FALSE);
  gtk_window_present (GTK_WINDOW (window));
}

void get_string(int type){
    char buffer[256];
    memset(buffer,0,sizeof(buffer));
    int counter = 0;
    int local_disk_counter = 0;

    
    int offset = 0;
    for(int i = 0 ; ;i++){
      if(buffer_disk_name[i] == '\0')
        break;
      if(buffer_disk_name[i] != '\n'){
        buffer[counter] = buffer_disk_name[i];
        counter++; 
      }else {
        if(type == NAME){
          memcpy(disks[local_disk_counter].name, buffer, counter);
          offset += counter;
        }else if(type == DEVICE){
          memcpy(disks[local_disk_counter].device, buffer, counter);
        }else if(type == SIZE){
          memcpy(disks[local_disk_counter].size, buffer, counter);
        }
        local_disk_counter++;
        if(type == DEVICE_COUNT){
          disk_counter++;
        }
        counter = 0;
      }
    }
  
}

void get_string_from_file(FILE *script_file) {
  memset(buffer_disk_name, 0, sizeof(buffer_disk_name));
  char buffer[128];
  memset(buffer,0,sizeof(buffer));
  int offset = 0;
  while (fgets(buffer, sizeof(buffer), script_file) !=
         NULL) {

    int len = strlen(buffer);
    memcpy(&buffer_disk_name[offset], buffer, len);
    offset += len;
  }
  
}

void get_usb_disk(){
    FILE *script_file;
    script_file = popen("./get_device_name.sh", "r");

    get_string_from_file(script_file); 
    get_string(NAME);

    pclose(script_file);
  

    script_file = popen("./get_devices.sh", "r");

    get_string_from_file(script_file); 
    get_string(DEVICE);
    pclose(script_file);

    script_file = popen("./get_sizes.sh", "r");

    get_string_from_file(script_file); 
    get_string(SIZE);

    get_string(DEVICE_COUNT);

    pclose(script_file);
     

    for(int i = 0; i< disk_counter; i++){
      printf("%s %s %s\n",disks[i].name, disks[i].size, disks[i].device);
    }
  

    int offset = 0;
    for(int i = 0; i < disk_counter; i++){
      sprintf(buffer_disk_full_name+offset, "%s %s", disks[i].name,disks[i].size);
      offset += 60;
    }


}

int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  get_usb_disk();


  app = gtk_application_new ("org.gtk.prufus", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}


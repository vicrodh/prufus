#include "gio/gio.h"
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include <gtk/gtk.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>


typedef struct Disk{
  char name[40];
  char device[20];
  char size[20];
} Disk;


char* devices_info[10];

char buffer_disk_name[128];

char buffer_disks_info[60*10];

GtkWidget* selected_iso_label;
GtkWidget* disk_label;
GtkWidget* devices_drop_down;
GtkWidget *window;
GtkWidget *status_label;

Disk disks[10];
Disk valid_disks[10];

pid_t make_usb_pid;

int disk_counter = 0;
#define NAME    1
#define DEVICE  2
#define SIZE    3
#define DEVICE_COUNT    4

#define DISKS_INFO_OFFSET 60

#define START '1'
#define TEMP '2'
#define MOUNT '3'
#define COPY '4'
#define COPY_BIG '5'
#define SYNC '6'
#define CLEAN '7'
#define SUCCESS '8'

bool can_update_status = true;

typedef struct MakeUSB{
  char* device;
  char iso_path[100];
} MakeUSB ;

static MakeUSB make_usb_data;


void * update_status(){
    usleep(500000) ;//wait for status file
    while (can_update_status == true) {
      int status_file_descriptor = open("/tmp/prufus/status", O_RDONLY);
      if (status_file_descriptor == -1) {
        g_print("Error open status file\n");
        perror("Status:");
      }
      char status;
      read(status_file_descriptor, &status, 1);
      //printf("%c\n",status);
      close(status_file_descriptor);

      switch (status) {
      case START: {
          gtk_label_set_text((GtkLabel*)status_label,"Start");
        break;
      }
      case TEMP: {
          gtk_label_set_text((GtkLabel*)status_label,"Temporal files");
        break;
      }
      case MOUNT: {
          gtk_label_set_text((GtkLabel*)status_label,"Mounting");
        break;
      }
      case COPY: {
          gtk_label_set_text((GtkLabel*)status_label,"Copying files");
        break;
      }
      case COPY_BIG: {
          gtk_label_set_text((GtkLabel*)status_label,"Copying files....");
        break;
      }
      case SYNC: {
          gtk_label_set_text((GtkLabel*)status_label,"Syncronizing disks......");
        break;
      }
      case CLEAN: {
          gtk_label_set_text((GtkLabel*)status_label,"Cleaning");
        break;
      }
      case SUCCESS: {
          gtk_label_set_text((GtkLabel*)status_label,"Success! you can disconnect you USB");
          can_update_status = false;
        break;
      }
      }

      usleep(500000);
    }


    printf("Finished status update\n");
}

static void
begin_usb_creation(GObject *source_object, GAsyncResult *res, gpointer user_data)
{

  int result = gtk_alert_dialog_choose_finish(user_data, res, NULL);
  if (result == 0) {
    guint select_device_index =
        gtk_drop_down_get_selected(GTK_DROP_DOWN(devices_drop_down));
    g_print("Formating....\n");
    GError *error_open = NULL;
    char *command[] = {"/root/prufus/simulate.sh", make_usb_data.iso_path,
                       valid_disks[select_device_index].device, NULL};

    char *env[] = {NULL};
    gboolean result = g_spawn_async(
        //NULL, command, env, G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL,
        NULL, command, env, G_SPAWN_SEARCH_PATH | G_SPAWN_CHILD_INHERITS_STDIN,
        NULL, NULL, &make_usb_pid, &error_open);
    if (!result) {
      g_print("can't execute\n");
      if (error_open != NULL) {
        g_print("error executing %s\n", error_open->message);
        g_error_free(error_open);
      }
    }
    can_update_status = true; 
    pthread_t thread;
    pthread_create( &thread,NULL,update_status,NULL);
    

  } else {
    g_print("Choose other option\n");
  }
  
}

static void cancel(GtkWidget *widget, gpointer data)
{
  g_print("cancel!\n");

  kill(make_usb_pid,9);

  can_update_status = false;

  gtk_label_set_text(GTK_LABEL(status_label),"Canceled!");
}

static void make_usb (GtkWidget *widget, gpointer data)
{
  

  guint select_device_index = 
    gtk_drop_down_get_selected(GTK_DROP_DOWN(devices_drop_down));

  GtkAlertDialog *write_usb_warning = gtk_alert_dialog_new(
      "WARNING! All data will be lost\n ISO Image: %s\n USB: %s %s",
      make_usb_data.iso_path,valid_disks[select_device_index].name, 
      valid_disks[select_device_index].size);
  
  const char * const dialog_buttons[] = {"I'm sure", "Cancel",NULL};

  gtk_alert_dialog_set_buttons(write_usb_warning,dialog_buttons);



  gtk_alert_dialog_choose(write_usb_warning,
      GTK_WINDOW(window),NULL, begin_usb_creation, write_usb_warning);


}

static void
select_file_result(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  GFile* my_iso = gtk_file_dialog_open_finish((GtkFileDialog*)source_object,res,NULL);
  char* my_iso_path = g_file_get_path(my_iso);
  int iso_path_len = strlen(my_iso_path);

  //clean iso path firstly
  memset(make_usb_data.iso_path,0,sizeof(make_usb_data.iso_path));

  memcpy(make_usb_data.iso_path, my_iso_path, iso_path_len);
  g_print("chossed\n");
  g_print(make_usb_data.iso_path);
  g_print("\n");
  gtk_label_set_text((GtkLabel*)selected_iso_label,my_iso_path);

}

static void choose_iso(GtkWidget *widget, gpointer data)
{
  gtk_file_dialog_set_title((GtkFileDialog*)data,"prufus");
  gtk_file_dialog_open((GtkFileDialog*)data, NULL, NULL, select_file_result ,NULL);
  g_print ("Choose your iso\n");
}

static void create_user_interface (GtkApplication *app, gpointer user_data)
{
  GtkWidget *create_usb_button;
  GtkWidget *choose_iso_button;
  GtkWidget *box;
  GtkWidget* title;
  GtkWidget* description;
  GtkWidget* header_box;
  GtkWidget* action_box;
  GtkWidget* select_iso_box;

  GtkWidget* select_iso_drop_down;

  GtkWidget* separator;

  GtkWidget* cancel_button; 

  GtkFileDialog* choose_iso_dialog;

  GtkFileFilter* iso_filter = gtk_file_filter_new();
  gtk_file_filter_add_suffix(iso_filter, "iso");

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "prufus");
  gtk_window_set_default_size (GTK_WINDOW (window), 500, 650);

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_widget_set_halign (box, GTK_ALIGN_FILL);
  gtk_widget_set_valign (box, GTK_ALIGN_FILL);
  gtk_widget_set_hexpand(box,FALSE);

  header_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign (header_box, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (header_box, GTK_ALIGN_CENTER);

  action_box= gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign (action_box, GTK_ALIGN_END);
  gtk_widget_set_valign (action_box, GTK_ALIGN_START);
  gtk_widget_set_vexpand(action_box,TRUE);

  select_iso_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_halign (select_iso_box, GTK_ALIGN_FILL);
  gtk_widget_set_valign (select_iso_box, GTK_ALIGN_FILL);


  
  separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);


  choose_iso_dialog = gtk_file_dialog_new();
  gtk_file_dialog_set_default_filter(choose_iso_dialog,iso_filter);

  select_iso_drop_down = gtk_drop_down_new(NULL, NULL);
  gtk_drop_down_set_factory(GTK_DROP_DOWN(select_iso_drop_down), NULL);

  selected_iso_label = gtk_label_new("None .iso selected");
  
  status_label = gtk_label_new("Nothing to do");
  
  //disk_label = gtk_label_new(buffer_disk_name);

  //create UI
  create_usb_button = gtk_button_new_with_label ("Create booteable USB");
  choose_iso_button = gtk_button_new_with_label ("Select .iso");

  cancel_button = gtk_button_new_with_label ("Cancel");

  title = gtk_label_new("prufus");
  PangoAttrList *const Attrs = pango_attr_list_new();
  PangoAttribute *const SizeAttr = pango_attr_size_new(40 * PANGO_SCALE);
  pango_attr_list_insert(Attrs, SizeAttr);
  gtk_label_set_attributes((GtkLabel*)title,Attrs);

  description = gtk_label_new("Create booteable USB from .iso images");


  //create drop down with devices names and sizes
  int offset = 0;
  for (int i = 0; i < disk_counter; i++) {
    devices_info[i] = buffer_disks_info + offset;
    offset += DISKS_INFO_OFFSET;//it's a constant offset defined
  }

  devices_drop_down = gtk_drop_down_new_from_strings((const char* const*)devices_info);



  g_signal_connect (cancel_button, "clicked", G_CALLBACK (cancel), NULL);
  g_signal_connect (create_usb_button, "clicked", G_CALLBACK (make_usb), NULL);
  g_signal_connect (choose_iso_button, "clicked", G_CALLBACK (choose_iso), choose_iso_dialog);
  
  gtk_box_append (GTK_BOX (box), header_box);

  gtk_box_append (GTK_BOX (header_box), title);
  gtk_box_append (GTK_BOX (header_box), description);
  gtk_box_append (GTK_BOX (action_box), create_usb_button);
  gtk_box_append (GTK_BOX (action_box), cancel_button);
  
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
  //gtk_box_append (GTK_BOX (box_disk), disk_button);
  gtk_box_append (GTK_BOX (box_disk), box_disk_label);


  gtk_box_append (GTK_BOX (box), box_disk);
  gtk_box_append (GTK_BOX (box), devices_drop_down);

  //gtk_box_append (GTK_BOX (box), select_iso_drop_down);
  gtk_box_append (GTK_BOX (box), action_box);
  
  GtkWidget* status_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_widget_set_halign (status_box, GTK_ALIGN_START);
  gtk_widget_set_valign (status_box, GTK_ALIGN_END);

  gtk_box_append (GTK_BOX (status_box), status_label);

  gtk_box_append (GTK_BOX (box), status_box);


  
  gtk_window_set_child (GTK_WINDOW (window), box);

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

void get_usb_disks(){
    FILE *script_file;
    script_file = popen("/root/prufus/get_device_name.sh", "r");

    get_string_from_file(script_file); 
    get_string(NAME);

    pclose(script_file);
  

    script_file = popen("/root/prufus/get_devices.sh", "r");

    get_string_from_file(script_file); 
    get_string(DEVICE);
    pclose(script_file);

    script_file = popen("/root/prufus/get_sizes.sh", "r");

    get_string_from_file(script_file); 
    get_string(SIZE);

    get_string(DEVICE_COUNT);

    pclose(script_file);
     

    for(int i = 0; i< disk_counter; i++){
      printf("%s %s %s\n",disks[i].name, disks[i].size, disks[i].device);
    }
    
    //filters disks
    int valid_disks_counter = 0;
    bool has_m2 = false;

    for(int i = 0; i< disk_counter; i++){
      if(strncmp(disks[i].device, "/dev/nv",7) == 0){
        has_m2 = true;
      }
    }
    
    memset(&valid_disks,0,sizeof(valid_disks));


    //if has m2 disk we suppose we have USBs in /dev/sd* unless some other SATA disk
    if (has_m2) {

      for (int i = 0; i < disk_counter; i++) {
        if (strncmp(disks[i].device, "/dev/sd", 7) == 0) {
          Disk* valid_disk = &disks[i];
          memcpy(&valid_disks[valid_disks_counter], valid_disk, sizeof(Disk));
          valid_disks_counter++;
          printf("Valid disk: %s\n",disks[i].device); 
        }
      }
    }else{//not tested but if not has m2 we suppose the main disk it's /dev/sda
      for (int i = 0; i < disk_counter; i++) {
        if (strncmp(disks[i].device, "/dev/sda", 7) != 0) {
          Disk* valid_disk = &disks[i];
          memcpy(&valid_disks[valid_disks_counter], valid_disk, sizeof(Disk));
          valid_disks_counter++;
          printf("Valid disk: %s\n",disks[i].device); 
        }
      }

    }

    printf("Valid disks %d\n",valid_disks_counter); 

    for(int i = 0; i< valid_disks_counter; i++){
      printf("%s %s %s\n",valid_disks[i].name, 
          valid_disks[i].size, valid_disks[i].device);
    }

    disk_counter = valid_disks_counter;

    int offset = 0;
    for(int i = 0; i < disk_counter; i++){
      sprintf(buffer_disks_info+offset, "%s %s", valid_disks[i].name,valid_disks[i].size);
      offset += DISKS_INFO_OFFSET;
    }


}

int main(int arguments_count, char **arguments_value)
{
  GtkApplication *prufus_application;

  int status;

  get_usb_disks();

  prufus_application = 
    gtk_application_new ("org.gtk.prufus", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect(prufus_application, "activate", 
      G_CALLBACK (create_user_interface), NULL);

  status = g_application_run(G_APPLICATION (prufus_application), 
      arguments_count, arguments_value);

  g_object_unref (prufus_application);

  return status;
}


#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include "user_interface.h"
#include "prufus.h"


char* devices_info[10];

pid_t make_usb_pid;

int disk_counter = 0;

bool can_update_status = true;

bool can_update_working_status = true;

static MakeUSB make_usb_data;


void * update_working_label(){
  while(can_update_working_status == true){
    gtk_label_set_text(GTK_LABEL(working_label),"..");
    usleep(500000) ;
    gtk_label_set_text(GTK_LABEL(working_label),"...");
    usleep(500000) ;
    gtk_label_set_text(GTK_LABEL(working_label),"....");
    usleep(500000) ;
    gtk_label_set_text(GTK_LABEL(working_label),".....");
    usleep(500000) ;
  }
  gtk_label_set_text(GTK_LABEL(working_label),"");
  g_print("Finish working label update\n");
}

void * update_status(){

  can_update_working_status = true;
  pthread_t working_thread;
  pthread_create(&working_thread, NULL, update_working_label, NULL);

  usleep(500000); // wait for status file
  while (can_update_status == true) {
    int status_file_descriptor = open("/tmp/prufus/status", O_RDONLY);
    if (status_file_descriptor == -1) {
      g_print("Error open status file\n");
      perror("Status:");
    }
    char status;
    read(status_file_descriptor, &status, 1);
    // printf("%c\n",status);
    close(status_file_descriptor);

    switch (status) {
    case FORMAT: {
      gtk_label_set_text((GtkLabel *)status_label, "Formating");
      break;
    }
    case TEMP: {
      gtk_label_set_text((GtkLabel *)status_label, "Creating temporal files");
      break;
    }
    case MOUNT: {
      gtk_label_set_text((GtkLabel *)status_label, "Mounting");
      break;
    }
    case COPY: {
      gtk_label_set_text((GtkLabel *)status_label, "Copying files");
      break;
    }
    case COPY_BIG: {
      gtk_label_set_text((GtkLabel *)status_label, "Copying files");
      break;
    }
    case SYNC: {
      gtk_label_set_text((GtkLabel *)status_label, "Syncronizing disks");
      break;
    }
    case CLEAN: {
      gtk_label_set_text((GtkLabel *)status_label, "Cleaning");
      break;
    }
    case SUCCESS: {
      gtk_label_set_text((GtkLabel *)status_label,
                         "Success! you can disconnect you USB");
      can_update_status = false;
      can_update_working_status = false;
      break;
    }
    }

    usleep(500000);
    }

    can_update_status = false;
    can_update_working_status = false;

    gtk_alert_dialog_show(success_alert,GTK_WINDOW(window));
  
    gtk_widget_set_sensitive(create_usb_button, TRUE);
    gtk_widget_set_sensitive(choose_iso_button, TRUE);
    gtk_button_set_label(GTK_BUTTON(create_usb_button), "Create booteable USB");

    printf("Finished status update\n");
}

static void
begin_usb_creation(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  
  gtk_widget_set_sensitive(create_usb_button, FALSE);
  gtk_button_set_label(GTK_BUTTON(create_usb_button), "Working");
  gtk_widget_set_sensitive(choose_iso_button, FALSE);
  gtk_widget_set_sensitive(cancel_button, TRUE);


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

void cancel(GtkWidget *widget, gpointer data)
{
  g_print("cancel!\n");

  kill(make_usb_pid,9);

  can_update_status = false;

  gtk_label_set_text(GTK_LABEL(status_label),"Canceled!");
  gtk_widget_set_sensitive(cancel_button, FALSE);
}

void make_usb(GtkWidget *widget, gpointer data)
{
  

  guint select_device_index = 
    gtk_drop_down_get_selected(GTK_DROP_DOWN(devices_drop_down));

  write_usb_warning = gtk_alert_dialog_new(
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

void choose_iso(GtkWidget *widget, gpointer data)
{
  gtk_file_dialog_set_title((GtkFileDialog*)data,"prufus");
  gtk_file_dialog_open((GtkFileDialog*)data, NULL, NULL, select_file_result ,NULL);
  g_print ("Choose your iso\n");
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


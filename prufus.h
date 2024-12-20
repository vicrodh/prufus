#ifndef _PRUFUS_H_
#define _PRUFUS_H_

#include <gtk/gtk.h>

#define NAME    1
#define DEVICE  2
#define SIZE    3
#define DEVICE_COUNT    4

#define DISKS_INFO_OFFSET 60

#define FORMAT '1'
#define TEMP '2'
#define MOUNT '3'
#define COPY '4'
#define COPY_BIG '5'
#define SYNC '6'
#define CLEAN '7'
#define SUCCESS '8'

void cancel(GtkWidget *widget, gpointer data);
void make_usb (GtkWidget *widget, gpointer data);
void choose_iso(GtkWidget *widget, gpointer data);

typedef struct MakeUSB{
  char* device;
  char iso_path[100];
} MakeUSB ;

extern char buffer_disks_info[];

extern char* devices_info[];

extern int disk_counter;

#endif

/*
 * testlibusb.c
 *
 * Test suite program shamelessly stolen from libusb.
 * Modified by Joachim Nilsson <jocke()vmlinux!org>
 *
 */

#ident "$Id$"

#include <argp.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <usb.h>

#include "usbmisc.h"
#include "usbext.h"


const char *argp_program_version = "$Id$";
const char *argp_program_bug_address = "<joachim.nilsson@afconsult.com>";

static char doc[] =
  "short program to show the use of argp\nThis program does little";

static char args_doc[] = "[SHOW|RESET|STATUS]";

/* initialise an argp_option struct with the options we except */
static struct argp_option options[] =
  {
    {"verbose", 'v', 0,         0, "Produce verbose output" },
    {"device",  'D', "PATH",    0, "Operate on this device, /proc/bus/usb/BBB/DDD ,instead of $DEVICE" },
    {"find",    'd', "VID/PID", 0, "Operate on a list of devices matching VendorID/DeviceID"},
    { 0 }
  };

/* Used by `main' to communicate with `parse_opt'. */
struct arguments
{
      char *cmd[1];
      int silent, verbose;
      int vid, pid;
      char *path;
};

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
   char *tmp;
   /* Get the INPUT argument from `argp_parse', which we
      know is a pointer to our arguments structure. */
   struct arguments *args = state->input;

   switch (key)
   {
      case 'q': case 's':
         args->silent = 1;
         break;

      case 'v':
         args->verbose ++;
         usb_set_debug (args->verbose);
         break;

      case 'd':
#if 0
         /* Either VID/PID or only a VID */
         tmp = strtok (arg, "/\0");
         if (tmp)
         {
            /* Convert one record, abort on error. */
            if (1 != sscanf (tmp, "0x%X", &args->vid))
            {
               errx (EINVAL, "%s is not a valid VID.", arg);
            }

            /* PID is actually optional. */
            tmp = strtok (NULL, "\0");
            if (tmp)
            {
               /* Convert one record, abort on error. */
               if (1 != sscanf (tmp, "0x%X", &args->pid))
               {
                  errx (EINVAL, "%s is not a valid PID.", arg);
               }
            }
            else
            {
               printf ("No PID given, will result in listing ALL devices for that vendor.\n");
            }
         }
         else
         {
            errx (EINVAL, "%s is not a valid VID/PID pair.", arg);
         }
#else
         /* Alternative implementation, stolen from lsusb */
         tmp = strchr (arg, '/');
         if (tmp) *tmp++ = 0;
         args->vid = strtoul (arg, NULL, 0);
         if (tmp) args->pid = strtoul (tmp, NULL, 0);
#endif
         break;

      case 'D':
         args->path = arg;
         break;

      case ARGP_KEY_ARG:
         if (state->arg_num >= 1)
            /* Too many arguments. */
            argp_usage (state);

         args->cmd[state->arg_num] = arg;

         break;

      case ARGP_KEY_END:
         break;
         if (state->arg_num < 1)
            /* Not enough arguments. */
            argp_usage (state);
         break;

      default:
         return ARGP_ERR_UNKNOWN;
   }

   return 0;
}

void print_endpoint(struct usb_endpoint_descriptor *endpoint)
{
  printf("      bEndpointAddress: %02xh\n", endpoint->bEndpointAddress);
  printf("      bmAttributes:     %02xh\n", endpoint->bmAttributes);
  printf("      wMaxPacketSize:   %d\n", endpoint->wMaxPacketSize);
  printf("      bInterval:        %d\n", endpoint->bInterval);
  printf("      bRefresh:         %d\n", endpoint->bRefresh);
  printf("      bSynchAddress:    %d\n", endpoint->bSynchAddress);
}

void print_altsetting(struct usb_interface_descriptor *interface)
{
  int i;

  printf("    bInterfaceNumber:   %d\n", interface->bInterfaceNumber);
  printf("    bAlternateSetting:  %d\n", interface->bAlternateSetting);
  printf("    bNumEndpoints:      %d\n", interface->bNumEndpoints);
  printf("    bInterfaceClass:    %d\n", interface->bInterfaceClass);
  printf("    bInterfaceSubClass: %d\n", interface->bInterfaceSubClass);
  printf("    bInterfaceProtocol: %d\n", interface->bInterfaceProtocol);
  printf("    iInterface:         %d\n", interface->iInterface);

  for (i = 0; i < interface->bNumEndpoints; i++)
    print_endpoint(&interface->endpoint[i]);
}

void print_interface(struct usb_interface *interface)
{
  int i;

  for (i = 0; i < interface->num_altsetting; i++)
    print_altsetting(&interface->altsetting[i]);
}

void print_configuration(struct usb_config_descriptor *config)
{
  int i;

  printf("  wTotalLength:         %d\n", config->wTotalLength);
  printf("  bNumInterfaces:       %d\n", config->bNumInterfaces);
  printf("  bConfigurationValue:  %d\n", config->bConfigurationValue);
  printf("  iConfiguration:       %d\n", config->iConfiguration);
  printf("  bmAttributes:         %02xh\n", config->bmAttributes);
  printf("  MaxPower:             %d\n", config->MaxPower);

  for (i = 0; i < config->bNumInterfaces; i++)
    print_interface(&config->interface[i]);
}

int print_device_orig(struct usb_device *dev, int level, int verbose)
{
  usb_dev_handle *udev;
  char description[256];
  char string[256];
  int ret, i;

  udev = usb_open(dev);
  if (udev)
  {
     i = snprintf(description, sizeof(description), "%04X/%04X",
                  dev->descriptor.idVendor, dev->descriptor.idProduct);

     ret = usb_get_string_simple(udev, dev->descriptor.iManufacturer, string, sizeof(string));
     if (ret > 0)
     {
        ret = snprintf(&description[i], sizeof(description) - i, " - %s", string);
        i += ret;
     }

     ret = usb_get_string_simple(udev, dev->descriptor.iProduct, string, sizeof(string));
     if (ret > 0)
     {
        ret = snprintf(&description[i], sizeof(description) - i, " %s", string);
        i += ret;
     }

     printf("%.*sDev #%d: %s\n", level * 2, "                    ", dev->devnum,
            description);
  }
  if (udev && verbose) {
    if (dev->descriptor.iSerialNumber) {
      ret = usb_get_string_simple(udev, dev->descriptor.iSerialNumber, string, sizeof(string));
      if (ret > 0)
        printf("%.*s  - Serial Number: %s\n", level * 2,
               "                    ", string);
    }
  }

  if (udev)
    usb_close(udev);

  if (verbose) {
    if (!dev->config) {
      printf("  Couldn't retrieve descriptors\n");
      return 0;
    }

    for (i = 0; i < dev->descriptor.bNumConfigurations; i++)
      print_configuration(&dev->config[i]);
  } else {
    for (i = 0; i < dev->num_children; i++)
       print_device_orig(dev->children[i], level + 1, verbose);
  }

  return 0;
}

int print_device(struct usb_device *dev, int level, int verbose)
{
  usb_dev_handle *udev;
  char description[256];
  char string[256];
  int ret, i;

  udev = usb_open(dev);
  if (udev)
  {
     i = snprintf(description, sizeof(description), "ID:%04X/%04X/%04X",
                  dev->descriptor.idVendor, dev->descriptor.idProduct,dev->descriptor.bcdDevice);

     ret = usb_get_string_simple(udev, dev->descriptor.iManufacturer, string, sizeof(string));
     if (ret > 0)
     {
        ret = snprintf(&description[i], sizeof(description) - i, " %s", string);
        i += ret;
     }

     ret = usb_get_string_simple(udev, dev->descriptor.iProduct, string, sizeof(string));
     if (ret > 0)
     {
        ret = snprintf(&description[i], sizeof(description) - i, " - %s", string);
        i += ret;
     }
#if 0
     printf("%.*sDev #%d: Bus %s Device %s %s\n", level * 2, "                    ",
            //dev->bus ? "dev->bus->dirname" : "(NULL)", "dev->filename",
            "BBB", "DDD",
            dev->devnum,
            description);

     printf ("Bus: %s Device: %s Dev#%d %s\n", dev->bus->dirname, dev->filename,
             dev->devnum,
             description);
#else
     {
#ifdef LIBUSB_HAS_GET_DRIVER_NP
        char buf[64];
        int bInterfaceNumber, result;

        bInterfaceNumber = dev->config->interface->altsetting[0].bInterfaceNumber;
        result = usb_get_driver_np(udev, bInterfaceNumber, buf, 64);
        if (!result)
           sprintf (string, "Driver:%s ", buf);
        else
           string[0] = 0;
#else
        string[0] = 0;
#endif
     }
     printf ("%s/%s/%s Dev:%d %s%s\n", PATH_USBFS,
             dev->bus->dirname, dev->filename,
             dev->devnum, string, description);
#endif
  }
  if (udev && verbose) {
     if (dev->descriptor.iSerialNumber) {
        ret = usb_get_string_simple(udev, dev->descriptor.iSerialNumber, string, sizeof(string));
        if (ret > 0)
           printf("%.*s  Serial Number:        %s\n", level * 2, "                    ", string);
     }
  }

  if (udev)
    usb_close(udev);

  if (verbose) {
    if (!dev->config) {
      printf("  Couldn't retrieve descriptors\n");
      return 0;
    }

    for (i = 0; i < dev->descriptor.bNumConfigurations; i++)
       print_configuration(&dev->config[i]);
  } else {
//    for (i = 0; i < dev->num_children; i++)
//       print_device(dev->children[i], level + 1, verbose);
  }

  return 0;
}

void print_devices (int verbose)
{
  struct usb_bus *bus;

  for (bus = usb_busses; bus; bus = bus->next)
  {
     if (bus->root_dev && !verbose)
        print_device(bus->root_dev, 0, verbose);
     else {
        struct usb_device *dev;

        for (dev = bus->devices; dev; dev = dev->next)
           print_device(dev, 0, verbose);
     }
  }
}

/* Clone dev and add head */
int list_add_clone (struct usb_device **list, struct usb_device *dev)
{
   struct usb_device *new = calloc (1, sizeof(struct usb_device));

   if (!new)
   {
      errx (ENOMEM, "Yikes! No memory ... bailing out.");
   }

   /* Found a match, clone and put it on the list */
   bcopy (dev, new, sizeof (struct usb_device));
   new->prev = new->next = NULL;

   if (!*list)
   {
      *list = new;
   }
   else
   {
      new->next     = *list;
      (*list)->prev = new;
      *list         = new;
   }

   return 0;
}

struct usb_device *find_devices (int vid, int pid, int did)
{
  struct usb_bus *bus;
  struct usb_device *head = NULL;

  usb_find_busses();
  usb_find_devices();

  //printf ("%s() - Searching for 0x%04X/0x%04X\n", __FUNCTION__, vid, pid);

  for (bus = usb_busses; bus; bus = bus->next)
  {
     struct usb_device *dev;

     for (dev = bus->devices; dev; dev = dev->next)
     {
        if (dev->descriptor.idVendor == vid && dev->descriptor.idProduct == pid)
        {
           list_add_clone (&head, dev);
        }
     }
  }

  return head;
}

struct usb_device *locate_device (char *path)
{
   struct usb_device *dev = NULL;

   list_add_clone (&dev, get_usb_device (path));

   return dev;
}

int reset (struct usb_device *list, int verbose)
{
   int result = 0;
   struct usb_dev_handle *udev;

   while (list)
   {
      udev = usb_claim_device (list);
      if (!udev)
      {
         fprintf (stderr, "Failed claiming device: %s\n", usb_strerror());
      }
      else
      {
         //print_device (dev, 0, verbose);
         printf ("Resetting ... ");
         if ((result = usb_reset (udev)))
         {
            perror ("Failed");
         }
         else
         {
            printf ("OK\n");
         }

         usb_release_device (udev);
      }

      list = list->next;
   }

   return result;
}


/* Query device status using simple control ep */
int status (struct usb_device *list, int verbose)
{
   int result;
   struct usb_dev_handle *udev;

   while (list)
   {
      udev = usb_claim_device (list);
      if (!udev)
      {
         fprintf (stderr, "Failed claiming device: %s\n", usb_strerror());
      }
      else
      {
         result = usb_control_msg(udev, USB_TYPE_VENDOR | USB_RECIP_DEVICE,
                                  USB_REQ_GET_STATUS, 0, 0, NULL, 0, 5000 );
         if (result)
            printf("usb_control_msg() returned %d (%s)\n", result, usb_strerror());
         result = usb_control_msg(udev, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,
                                  USB_REQ_GET_STATUS, 0, 0, NULL, 0, 5000 );
         if (result)
            printf("usb_control_msg() returned %d (%s)\n", result, usb_strerror());

         usb_release_device (udev);
      }

      list = list->next;
   }

   return 0;
}

/* Display device information */
int display (struct usb_device *list, int verbose)
{
   while (list)
   {
      print_device (list, 0, verbose);
      list = list->next;
   }

   return 0;
}

int list_free (struct usb_device *list)
{
   struct usb_device *dev;

   while (list)
   {
      dev = list;
      list = dev->next;

      free (dev);
   }

   return 0;
}

typedef enum {DISPLAY = 0, STATUS, RESET} op_t;

typedef struct {
  char *command;
  op_t  cmd;
} cmd_t;

cmd_t command_map[] = {
   {"DISPLAY", DISPLAY},
   {"SHOW", DISPLAY},
   {"STATUS", STATUS},
   {"RESET", RESET},
};

#define ARRAY_SIZE(a) sizeof((a)) / sizeof((a)[0])

int map_command_to_cmd (char *command)
{
  int i;

  for (i = 0; i < ARRAY_SIZE(command_map); i++)
    {
      if (!strcasecmp (command_map[i].command, command))
        return command_map[i].cmd;
    }

  return -1;
}



int main (int argc, char **argv)
{
   int cmd;
   struct usb_device *list;
   struct arguments arg;
   /* Our argp parser. */
   static struct argp argp = { options, parse_opt, args_doc, doc };

   /* Default values. */
   arg.silent  = 0;
   arg.verbose = 0;
   arg.vid     = 0;
   arg.pid     = 0;
   arg.path    = getenv ("DEVICE");
   arg.cmd[0]  = "DISPLAY";

   /* Parse our arguments; every option seen by `parse_opt' will
      be reflected in `arguments'. */
   argp_parse (&argp, argc, argv, 0, 0, &arg);

   usb_init();

   cmd = map_command_to_cmd (arg.cmd[0]);
   if (cmd < 0)
   {
      err(EINVAL, "No such command, reverint to display device.");
   }

   usb_find_busses();
   usb_find_devices();

   //print_devices ();
   //find_device (atoi(argv[1]), atoi(argv[2]), 0);
   //list = find_devices (0xE6E6, 0x201, 0);
   if (arg.path)
   {
      list = locate_device (arg.path);
   }
   else if (arg.vid)
   {
      list = find_devices (arg.vid, arg.pid, 0);
   }
   else // (!arg.path && !arg.vid)
   {
      errx(EINVAL, "You must specify a device path or VID[/PID] device match.");
   }

   switch (cmd)
   {
      case STATUS:
         status (list, arg.verbose);
         break;

      case RESET:
         reset (list, arg.verbose);
         break;

      case DISPLAY:
      default:
         /* Read usb_device_descriptor and print it out. */
         display (list, arg.verbose);
         break;
   }

   list_free (list);

   return 0;
}


/**
 * Local Variables:
 *  c-file-style: "ellemtel"
 *  indent-tabs-mode: nil
 * End:
 */

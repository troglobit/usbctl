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
#include <string.h>
#include <usb.h>

const char *argp_program_version = "$Id$";
const char *argp_program_bug_address = "<joachim.nilsson@afconsult.com>";

static char doc[] =
  "short program to show the use of argp\nThis program does little";

static char args_doc[] = "COMMAND";

int verbose = 0;

/* initialise an argp_option struct with the options we except */
static struct argp_option options[] =
  {
    {"verbose", 'v', 0,      0, "Produce verbose output" },
    {"device",  'D', "PATH", 0, "Use this device, instead of $DEVICE" },
    { 0 }
  };

/* Used by `main' to communicate with `parse_opt'. */
struct arguments
{
  char *cmd[1];
  int silent, verbose;
  char *device_path;
};

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the INPUT argument from `argp_parse', which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'q': case 's':
      arguments->silent = 1;
      break;

    case 'v':
      arguments->verbose = 1;
      break;

    case 'D':
      arguments->device_path = arg;
      break;

    case ARGP_KEY_ARG:
      if (state->arg_num >= 1)
        /* Too many arguments. */
        argp_usage (state);

      arguments->cmd[state->arg_num] = arg;

      break;

    case ARGP_KEY_END:
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

int print_device_orig(struct usb_device *dev, int level)
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
      print_device(dev->children[i], level + 1);
  }

  return 0;
}

int print_device(struct usb_device *dev, int level)
{
  usb_dev_handle *udev;
  char description[256];
  char string[256];
  int ret, i;

  udev = usb_open(dev);
  if (udev)
  {
     i = snprintf(description, sizeof(description), "[%04X/%04X]",
                  dev->descriptor.idVendor, dev->descriptor.idProduct);

     ret = usb_get_string_simple(udev, dev->descriptor.iManufacturer, string, sizeof(string));
     if (ret > 0)
     {
        ret = snprintf(&description[i], sizeof(description) - i, " %s", string);
        i += ret;
     }

     ret = usb_get_string_simple(udev, dev->descriptor.iProduct, string, sizeof(string));
     if (ret > 0)
     {
        ret = snprintf(&description[i], sizeof(description) - i, " :: %s", string);
        i += ret;
     }
#if 0
     printf("%.*sDev #%d: Bus %s Device %s %s\n", level * 2, "                    ",
            //dev->bus ? "dev->bus->dirname" : "(NULL)", "dev->filename",
            "BBB", "DDD",
            dev->devnum,
            description);
#else
     printf ("Bus: %s Device: %s Dev#%d %s\n", dev->bus->dirname, dev->filename,
             dev->devnum,
             description);
#endif
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
      print_device(dev->children[i], level + 1);
  }

  return 0;
}

void print_devices (void)
{
  struct usb_bus *bus;

  usb_find_busses();
  usb_find_devices();

  for (bus = usb_busses; bus; bus = bus->next) {
    if (bus->root_dev && !verbose)
      print_device(bus->root_dev, 0);
    else {
      struct usb_device *dev;

      for (dev = bus->devices; dev; dev = dev->next)
        print_device(dev, 0);
    }
  }
}

struct usb_device *find_devices (int vid, int pid, int did)
{
  struct usb_bus *bus;
  struct usb_device *head = NULL, *curr;

  usb_find_busses();
  usb_find_devices();

  printf ("%s() - Searching for 0x%04X/0x%04X\n", __FUNCTION__, vid, pid);

  for (bus = usb_busses; bus; bus = bus->next)
  {
     struct usb_device *dev;

     for (dev = bus->devices; dev; dev = dev->next)
     {
        if (dev->descriptor.idVendor == vid && dev->descriptor.idProduct == pid)
        {
           struct usb_device *new = calloc (1, sizeof(struct usb_device));

           /* Found a match, clone and put it on the list */
           bcopy (dev, new, sizeof (struct usb_device));
           new->prev = new->next = NULL;

           if (!head)
           {
              head = new;
              curr = head;
           }
           else
           {
              new->prev  = curr;
              curr->next = new;
              curr       = new;
           }
        }
     }
  }

  return head;
}


int do_usb_reset (struct usb_device *dev)
{
   int result = -1;
   usb_dev_handle *udev;

   udev = usb_open(dev);
   if (udev)
   {
      result = usb_reset (udev);

      usb_close(udev);
   }
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

int main (int argc, char **argv)
{
   int cmd, result;
   struct usb_device *list, *dev;
   struct arguments arguments;

   /* Default values. */
   arguments.silent = 0;
   arguments.verbose = 0;
   arguments.device_path = getenv ("DEVICE");

   /* Parse our arguments; every option seen by `parse_opt' will
      be reflected in `arguments'. */
   argp_parse (&argp, argc, argv, 0, 0, &arguments);
   verbose = arguments.verbose;

   usb_init();

   //print_devices ();
   verbose = 1;
   //find_device (atoi(argv[1]), atoi(argv[2]), 0);
   list = find_devices (0xE6E6, 0x201, 0);
   dev = list;
   while (dev)
   {
      print_device (dev, 0);
      printf ("Resetting!\n");
      if (do_usb_reset (dev))
      {
         perror ("Failed USB device reset");
      }
      dev = dev->next;
   }
   while (list)
   {
      dev = list;
      list = dev->next;

      free (dev);
   }

   return 0;
}


/**
 * Local Variables:
 *  c-file-style: "ellemtel"
 *  indent-tabs-mode: nil
 *  //compile-command: "gcc -o usbctl usbctl.c -L. -lnsl -lm -lc -lusb"
 * End:
 */

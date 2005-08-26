/* usbext.c  --  Extensions to LibUSB.
 *
 * Copyright (C) 2005  Joachim Nilsson <jocke()vmlinux!org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef _USBEXT_H
#define _USBEXT_H

#include <usb.h>
#include <sys/ioctl.h>

#define INTERFACE_NUMBER(dev) \
  (dev)->config->interface->altsetting[0].bInterfaceNumber

struct usb_ioctl {
        int ifno;       /* interface 0..N ; negative numbers reserved */
        int ioctl_code; /* MUST encode size + direction of data so the
                         * macros in <asm/ioctl.h> give correct values */
        void *data;     /* param buffer (in, or out) */
};

/* Actually the usb_dev_handle from libusb.  Renamed here to avoid clash */
struct usb_dev_handle_ext {
  int fd;

  struct usb_bus *bus;
  struct usb_device *device;

  int config;
  int interface;
  int altsetting;

  /* Added by RMT so implementations can store other per-open-device data */
  void *impl_info;
};

extern int usb_debug;


#define IOCTL_USB_IOCTL         _IOWR('U', 18, struct usb_ioctl)
#define IOCTL_USB_CONNECT       _IO('U', 23)

struct usb_dev_handle *usb_claim_device (struct usb_device *dev);
int usb_release_device (struct usb_dev_handle *udev);
int usb_reattach_kernel_driver_np(usb_dev_handle *udev, int interface);

#endif /* _USBEXT_H */

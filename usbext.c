/*****************************************************************************/
/*
 *      usbext.c  --  Extensions to LibUSB.
 *
 *      Copyright (C) 2005  Joachim Nilsson <jocke()vmlinux!org>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 */

/*****************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "error.h"
#include "usbext.h"

struct usb_dev_handle *usb_claim_device (struct usb_device *dev)
{
   int result;
   struct usb_dev_handle *udev;

   udev = usb_open(dev);
   if (udev)
   {
#ifdef LIBUSB_HAS_GET_DRIVER_NP
      result = usb_detach_kernel_driver_np (udev, INTERFACE_NUMBER(dev));
      if (result) goto exit;
#endif
      result = usb_claim_interface (udev, INTERFACE_NUMBER(dev));
      if (result) goto exit;

      return udev;
   }
  exit:
   usb_close(udev);

   return NULL;
}

int usb_release_device (struct usb_dev_handle *udev)
{
   usb_release_interface (udev, INTERFACE_NUMBER(usb_device(udev)));
   usb_reattach_kernel_driver_np (udev, INTERFACE_NUMBER(usb_device(udev)));

   return usb_close(udev);
}

/* Reattach kernel driver. */
int usb_reattach_kernel_driver_np(usb_dev_handle *udev, int interface)
{
   struct usb_dev_handle_ext *dev = (void *)udev;

   struct usb_ioctl command;
   int ret;

   command.ifno = interface;
   command.ioctl_code = IOCTL_USB_CONNECT;
   command.data = NULL;

   ret = ioctl(dev->fd, IOCTL_USB_IOCTL, &command);
   if (ret)
   {
      USB_ERROR_STR(-errno, "could not reattach kernel driver to interface %d: %s",
                    interface, strerror(errno));
   }

   return 0;
}

/**
 * Local Variables:
 *  c-file-style: "ellemtel"
 *  indent-tabs-mode: nil
 * End:
 */

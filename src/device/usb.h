#ifndef NANOTUBE_USB_H
#define NANOTUBE_USB_H

#include "../queue.h"

#include <usbdrvce.h>

//todo: remove
#define ETHERNET_MTU 1500

enum device_types {
	DEVICE_NONE,
	DEVICE_CDC_ECM,
	DEVICE_CDC_EEM,
	DEVICE_CDC_EEM_HOST
};

typedef struct {
	uint8_t type;
	usb_device_t dev;
	usb_endpoint_t in;
	usb_endpoint_t out;
	queue_t queue;
} netif_state_t;

usb_error_t usb_handle_connect(usb_device_t dev);
usb_error_t usb_handle_disconnect(usb_device_t dev);

#endif //NANOTUBE_USB_H

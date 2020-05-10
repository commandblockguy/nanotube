#ifndef NANOTUBE_USB_H
#define NANOTUBE_USB_H

#include "../queue.h"
#include "ecm.h"

#include <usbdrvce.h>

#define USB_CS_INTERFACE_DESCRIPTOR 0x24

#define NEXT_DESCRIPTOR(desc) (usb_descriptor_t *) (((uint8_t *) (desc) + (desc)->bLength))

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
} usb_class_specific_descriptor_t;

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
	uint8_t mac_index;
	uint24_t mtu;
	void *read_data;
	//todo: remove
	bool halted;
} netif_state_t;

void netif_init_common(struct netif *netif);

usb_error_t usb_handle_connect(usb_device_t dev);
usb_error_t usb_handle_disconnect(usb_device_t dev);

usb_error_t usb_handle_event(usb_event_t event, void *event_data, usb_callback_data_t *callback_data);

#endif //NANOTUBE_USB_H

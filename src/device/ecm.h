#ifndef NANOTUBE_ECM_H
#define NANOTUBE_ECM_H

#include <usbdrvce.h>
#include "../lwIP/include/lwip/err.h"
#include "../lwIP/include/lwip/netif.h"

/* For USB CDC Ethernet Control Model devices */

#define ECM_SUBCLASS 6

#define CDC_ETHERNET_NETWORKING_SUBTYPE 0x0F
#define CDC_UNION_SUBTYPE 6

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;
    uint8_t  iMACAddress;
    uint32_t bmEthernetStatistics;
    uint16_t wMaxSegmentSize;
    uint16_t wNumberMCFilters;
    uint8_t  bNumberPowerFilters;
} cdc_ethernet_descriptor_t;

typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;
    uint8_t  bControlInterface;
    uint8_t  bSubordinateInterfaces[1];
} cdc_union_descriptor_t;

usb_error_t ecm_read_callback(usb_endpoint_t pEndpoint, usb_transfer_status_t status, size_t size, void *pVoid);
usb_error_t ecm_write_callback(usb_endpoint_t pEndpoint, usb_transfer_status_t status, size_t size, void *pVoid);

static err_t ecm_netif_output(struct netif *netif, struct pbuf *p);

err_t ecm_init_netif(struct netif *netif);

enum ECM_PACKET_FILTER {
	PACKET_TYPE_MULTICAST     = (1 << 4),
	PACKET_TYPE_BROADCAST     = (1 << 3),
	PACKET_TYPE_DIRECTED      = (1 << 2),
	PACKET_TYPE_ALL_MULTICAST = (1 << 1),
	PACKET_TYPE_PROMISCUOUS   = (1 << 0),
};

usb_error_t ecm_set_packet_filer(usb_device_t dev, uint8_t filter);

cdc_ethernet_descriptor_t *get_ether_desc(usb_interface_descriptor_t *int_desc, usb_descriptor_t *end);
uint8_t *get_subordinate_ints(usb_interface_descriptor_t *int_desc, usb_descriptor_t *end, uint8_t *length);

#endif //NANOTUBE_ECM_H

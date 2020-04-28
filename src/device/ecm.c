#include <string.h>
#include <usbdrvce.h>
#include "../lwIP/include/lwip/err.h"
#include "../lwIP/include/lwip/netif.h"
#include "../lwIP/include/lwip/etharp.h"
#include "../lwIP/include/lwip/ip4_addr.h"

#include "ecm.h"
#include "usb.h"
#include "../queue.h"
#include "../lwIP/include/lwip/snmp.h"
#include "../log.h"

#define ETHERNET_MTU 1518

extern uint8_t eth_data[ETHERNET_MTU];

//todo: remove
void clear_halt(struct netif *netif) {
    netif_state_t *state = netif->state;
    if(state->halted) {
        usb_error_t error;
        error = usb_ClearEndpointHalt(state->in);
        if(error) custom_printf("Error %u clearing ep halt\n", error);
        error = usb_ScheduleTransfer(state->in, eth_data, sizeof(eth_data), ecm_read_callback, netif);
        if(error) {
            custom_printf("Error %u rescheduling transfer\n", error);
        } else {
            state->halted = false;
        }
    }
}

/* For USB CDC Ethernet Control Model devices */

usb_error_t ecm_read_callback(usb_endpoint_t pEndpoint, usb_transfer_status_t status, size_t size, void *pVoid) {
	struct pbuf *p;
	struct netif *netif = pVoid;
	netif_state_t *state = netif->state;
	uint16_t type = ((struct eth_hdr*)eth_data)->type;
	//mainlog("got packet\n");
	/* Ignore IPv6 and ARRIS router broadcasts */
	if(type != PP_HTONS(ETHTYPE_IPV6) && type != 0x9988 && size) {
		p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
		if(p != NULL) {

			log_packet(eth_data, size, true);

			/* Copy ethernet frame into pbuf */
			pbuf_take(p, eth_data, size);

			/* Put in a queue which is processed in main loop */
			if(!queue_add(&state->queue, p)) {
				/* queue is full -> packet loss */
				mainlog("packet queue full - dropping\n");
				pbuf_free(p);
			}
		}
	}
    if(status) {
        custom_printf("Transfer returned status %02x\n", status);
        if(status & USB_TRANSFER_FAILED) {
            state->halted = true;
            return USB_IGNORE;
        }
    }
    usb_ScheduleTransfer(pEndpoint, eth_data, sizeof(eth_data), ecm_read_callback, netif);
    return USB_SUCCESS;
}

usb_error_t ecm_write_callback(usb_endpoint_t pEndpoint, usb_transfer_status_t status, size_t size, void *pVoid) {
	struct pbuf *p = pVoid;
	//mainlog("sent packet\n");
	if(size != p->tot_len) {
        custom_printf("Transfer sent %u bytes, expected %u (status: %u)\n", size, p->tot_len, status);
	}
	pbuf_free(p);
	return USB_SUCCESS;
}

static err_t ecm_netif_output(struct netif *netif, struct pbuf *p) {
	netif_state_t *state = netif->state;
//	int unicast;
//	size_t transferred;
	LINK_STATS_INC(link.xmit);
	/* Update SNMP stats (only if you use SNMP) */
	MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);
//	unicast = ((((uint8_t *) p->payload)[0] & 0x01) == 0);
//	if(unicast) {
//		MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
//	} else {
//		MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
//	}

	pbuf_ref(p);

    log_packet(p->payload, p->tot_len, false);

	usb_ScheduleTransfer(state->out, p->payload, p->tot_len, ecm_write_callback, p);

	return ERR_OK;
}

uint8_t wchar_hex_to_int(wchar_t wchar) {
    if(wchar < 'A') {
        return wchar - '0';
    } else {
        return wchar - 'A' + 0x0A;
    }
}

err_t ecm_get_mac(usb_device_t dev, uint8_t index, uint8_t *addr) {
    struct {
        uint8_t  bLength;
        uint8_t  bDescriptorType;
        wchar_t  str[2 * NETIF_MAX_HWADDR_LEN];
    } str_desc;
    usb_error_t error;
    uint8_t i;

    error = usb_GetStringDescriptor(dev, index, 0x0409, (usb_string_descriptor_t*)&str_desc, sizeof(str_desc), NULL);
    if(error) {
        custom_printf("Error %u when getting MAC address\n", error);
        return ERR_IF;
    }

    mainlog("got MAC address ");
    for(i = 0; i < NETIF_MAX_HWADDR_LEN; i++) {
        uint8_t upper = wchar_hex_to_int(str_desc.str[2 * i]);
        uint8_t lower = wchar_hex_to_int(str_desc.str[2 * i + 1]);
        addr[i] = (upper << 4) | lower;
        custom_printf("%x%x", upper, lower);
    }
    mainlog("\n");

    return ERR_OK;
}

err_t ecm_init_netif(struct netif *netif) {
	netif_state_t *state = netif->state;
	uint8_t mac[6];
	mainlog("ecm netif init called\n");
	netif->linkoutput = ecm_netif_output;
	netif->output = etharp_output;
	ecm_get_mac(state->dev, state->mac_index, mac);
#if LWIP_IPV6
	netif->output_ip6 = ethip6_output;
#endif
	netif->mtu = ETHERNET_MTU;
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6;
	memcpy(netif->hwaddr, mac, ETH_HWADDR_LEN);
	netif->hwaddr_len = ETH_HWADDR_LEN;

	netif->name[0] = 'e';
	netif->name[1] = 't';

	queue_init(&state->queue);

	ecm_set_packet_filer(state->dev, PACKET_TYPE_BROADCAST | PACKET_TYPE_DIRECTED);

	state->halted = false;
	usb_ScheduleTransfer(state->in, eth_data, sizeof(eth_data), ecm_read_callback, netif);

	netif_init_common(netif);

	return ERR_OK;
}

usb_error_t ecm_set_packet_filer(usb_device_t dev, uint8_t filter){
	usb_error_t err;
	usb_control_setup_t setup = {0x21, 0x43, 0, 1, 0};
	setup.wValue = filter;
	err = usb_DefaultControlTransfer(dev, &setup, NULL, 10, NULL);
	if(err) {
        custom_printf("error %u on filter set\n", err);
	}

	mainlog("set filter\n");
	return err;
}

cdc_ethernet_descriptor_t *get_ether_desc(usb_interface_descriptor_t *int_desc, usb_descriptor_t *end) {
    usb_descriptor_t *current_desc;
    for(current_desc = NEXT_DESCRIPTOR(int_desc);
        current_desc < end && current_desc->bDescriptorType != USB_INTERFACE_DESCRIPTOR;
        current_desc = NEXT_DESCRIPTOR(current_desc)) {

        if(current_desc->bDescriptorType == USB_CS_INTERFACE_DESCRIPTOR) {
            if(((usb_class_specific_descriptor_t*)current_desc)->bDescriptorSubtype == CDC_ETHERNET_NETWORKING_SUBTYPE) {
                return (cdc_ethernet_descriptor_t*)current_desc;
            }
        }
    }

    return NULL;
}

uint8_t *get_subordinate_ints(usb_interface_descriptor_t *int_desc, usb_descriptor_t *end, uint8_t *length) {
    usb_descriptor_t *current_desc;
    for(current_desc = NEXT_DESCRIPTOR(int_desc);
        current_desc < end && current_desc->bDescriptorType != USB_INTERFACE_DESCRIPTOR;
        current_desc = NEXT_DESCRIPTOR(current_desc)) {

        if(current_desc->bDescriptorType == USB_CS_INTERFACE_DESCRIPTOR) {
            if(((usb_class_specific_descriptor_t*)current_desc)->bDescriptorSubtype == CDC_UNION_SUBTYPE) {
                cdc_union_descriptor_t *union_desc = (cdc_union_descriptor_t*)current_desc;
                *length = union_desc->bLength - 4;
                return union_desc->bSubordinateInterfaces;
            }
        }
    }

    *length = 0;
    return NULL;
}

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

//todo: remove
extern uint8_t mac_addr[6];
extern uint8_t eth_data[ETHERNET_MTU + 18];

/* For USB CDC Ethernet Control Model devices */

usb_error_t ecm_read_callback(usb_endpoint_t pEndpoint, usb_transfer_status_t status, size_t size, void *pVoid) {
	struct pbuf *p;
	struct netif *netif = pVoid;
	netif_state_t *state = netif->state;
	uint16_t type = ((struct eth_hdr*)eth_data)->type;
	//mainlog("got packet");
	/* Ignore IPv6 and ARRIS router broadcasts */
	if(type != PP_HTONS(ETHTYPE_IPV6) && type != 0x9988) {
		p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
		if(p != NULL) {

			log_packet(eth_data, size, true);

			/* Copy ethernet frame into pbuf */
			pbuf_take(p, eth_data, size);

			/* Put in a queue which is processed in main loop */
			if(!queue_add(&state->queue, p)) {
				/* queue is full -> packet loss */
				mainlog("packet queue full - dropping");
				pbuf_free(p);
			}
		}
	}
	usb_ScheduleTransfer(pEndpoint, eth_data, sizeof(eth_data), ecm_read_callback, netif);
	return USB_SUCCESS;
}

usb_error_t ecm_write_callback(usb_endpoint_t pEndpoint, usb_transfer_status_t status, size_t size, void *pVoid) {
	struct pbuf *p = pVoid;
	//mainlog("sent packet");
	if(size != p->tot_len) {
        custom_printf("Transfer sent %u bytes, expected %u (status: %u)", size, p->tot_len, status);
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

err_t ecm_init_netif(struct netif *netif) {
    ip4_addr_t ip, netmask, gateway;
	netif_state_t *state = netif->state;
	mainlog("ecm netif init called");
	netif->linkoutput = ecm_netif_output;
	netif->output = etharp_output;
#if LWIP_IPV6
	netif->output_ip6 = ethip6_output;
#endif
	netif->mtu = ETHERNET_MTU;
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6;
	//todo: properly get this
//	mac_addr = //ecm_get_mac_address(state->dev);
	memcpy(netif->hwaddr, mac_addr, ETH_HWADDR_LEN);
	netif->hwaddr_len = ETH_HWADDR_LEN;
	//todo: remove / use dhcp
    IP4_ADDR(&ip, 192,168,0,6);
    IP4_ADDR(&netmask, 255,255,255,0);
    IP4_ADDR(&gateway, 192,168,0,1);
    netif_set_addr(netif, &ip, &netmask, &gateway);

	queue_init(&state->queue);

	ecm_set_packet_filer(state->dev, PACKET_TYPE_MULTICAST | PACKET_TYPE_BROADCAST | PACKET_TYPE_DIRECTED);

	usb_ScheduleTransfer(state->in, eth_data, sizeof(eth_data), ecm_read_callback, netif);

	return ERR_OK;
}

usb_error_t ecm_set_packet_filer(usb_device_t dev, uint8_t filter){
	usb_error_t err;
	usb_control_setup_t setup = {0x21, 0x43, 0, 1, 0};
	setup.wValue = filter;
	err = usb_DefaultControlTransfer(dev, &setup, NULL, 10, NULL);
	if(err) {
        custom_printf("error %u on filter set", err);
	}

	mainlog("set filter");
	return err;
}

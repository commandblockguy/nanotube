#include "usb.h"
#include "../lwIP/include/lwip/api.h"
#include "../lwIP/include/lwip/netif.h"
#include "../lwIP/include/lwip/dhcp.h"
#include "ecm.h"
#include <stdlib.h>
#include <debug.h>

//todo: remove
#define CONFIGURATION 2
#define EP_IN 0x81
#define EP_OUT 0x02

usb_error_t usb_handle_connect(usb_device_t dev) {
#if !LWIP_DHCP
    // todo: handle
    ip4_addr_t ip = IPADDR4_INIT_BYTES(192,168,0,6);
    ip4_addr_t netmask = IPADDR4_INIT_BYTES(255,255,255,0);
    ip4_addr_t gateway = IPADDR4_INIT_BYTES(192,168,0,1);
#endif
	struct netif *netif = NULL;
	netif_state_t *state = NULL;
	uint8_t buf[256];
	size_t len;
	usb_error_t err;
	err_t error;
	uint8_t type;

	mainlog("Device connected.\n");
	usb_ResetDevice(dev);
	usb_WaitForEvents();

	netif = malloc(sizeof(struct netif));

	if(!netif) {
		mainlog("couldn't allocate netif\n");
		return USB_SUCCESS;
	}

	usb_SetDeviceData(dev, netif);

	state = malloc(sizeof(netif_state_t));

	if(!state) {
		mainlog("couldn't allocate netif state\n");
		return USB_SUCCESS;
	}

	//todo: figure out what to do here
	len = usb_GetConfigurationDescriptorTotalLength(dev, CONFIGURATION - 1);
	err = usb_GetDescriptor(dev, USB_CONFIGURATION_DESCRIPTOR, CONFIGURATION - 1, buf, len, NULL);
	if(err) custom_printf("Error %u on config get\n", err);

    custom_printf("got config descriptor %p (%u)\n", buf, len);

	err = usb_SetConfiguration(dev, (usb_configuration_descriptor_t *) buf, len);
	if(err) custom_printf("Error %u on config set\n", err);

	mainlog("set config\n");

	// todo: this is very bad
	usb_SetInterface(dev, (usb_interface_descriptor_t * )(&buf[0x39]), len - 0x39);

	mainlog("set interface\n");

	state->in = usb_GetDeviceEndpoint(dev, EP_IN);
    custom_printf("got in endpoint: %p\n", state->in);
	state->out = usb_GetDeviceEndpoint(dev, EP_OUT);
    custom_printf("got out endpoint: %p\n", state->out);

	if(state->in && state->out)
		mainlog("got endpoints\n");
	else {
		mainlog("error: couldn't get endpoints\n");
		return USB_SUCCESS;
	}

    //todo: determine device type
    type = DEVICE_CDC_ECM;

    state->type = type;
    state->dev = dev;

	//todo: get actual function
#if LWIP_DHCP
	netif_add(netif, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY, state, ecm_init_netif, netif_input);
#else
    netif_add(netif, &ip, &netmask, &gateway, state, ecm_init_netif, netif_input);
#endif

	//todo: where should this go?
	netif->hostname = "ti84pce";

#if LWIP_IPV6
	netif_create_ip6_linklocal_address(netif, 1);
	netif.ip6_autoconfig_enabled = 1;
#endif
	if(!netif_default) {
		netif_set_default(netif);
	}
	netif_set_up(netif);

	//todo: figure out how to do this correctly
	netif_set_link_up(netif);

	mainlog("interface set up\n");

#if LWIP_DHCP
    /* Start DHCP */
    error = dhcp_start(netif);
    if(error) {
        custom_printf("error in dhcp start: %u\n", error);
    }
    mainlog("dhcp started\n");
#endif

	return USB_SUCCESS;
}

usb_error_t usb_handle_disconnect(usb_device_t dev) {
	struct netif *netif = usb_GetDeviceData(dev);
	mainlog("device disconnected\n");
	netif_remove(netif);
	free(netif->state);
	free(netif);
	return USB_SUCCESS;
}

#include "usb.h"
#include "../lwIP/include/lwip/api.h"
#include "../lwIP/include/lwip/netif.h"
#include "../lwIP/include/lwip/dhcp.h"
#include "ecm.h"
#include <stdlib.h>

// todo: associate netifs with device so that we can free them on disconnect / error

#if !LWIP_DHCP
extern ip4_addr_t ip, netmask, gateway;
#endif

#define MAX_CONFIG_LENGTH 256

void netif_init_common(struct netif *netif) {
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
}

usb_interface_descriptor_t *get_interface_descriptor(usb_descriptor_t *start, usb_descriptor_t *end, uint8_t n) {
    usb_descriptor_t *current_desc;

    for(current_desc = start; current_desc < end; current_desc = NEXT_DESCRIPTOR(current_desc)) {
        if(current_desc->bDescriptorType == USB_INTERFACE_DESCRIPTOR) {
            usb_interface_descriptor_t *int_desc = (usb_interface_descriptor_t *) current_desc;
            if(int_desc->bInterfaceNumber == n) return int_desc;
        }
    }

    return NULL;
}

void
get_endpoints(usb_interface_descriptor_t *int_desc, usb_descriptor_t *end, uint8_t *in, uint8_t *out, uint8_t *inter) {
    usb_descriptor_t *current_desc;
    for(current_desc = NEXT_DESCRIPTOR(int_desc);
        current_desc < end && current_desc->bDescriptorType != USB_INTERFACE_DESCRIPTOR;
        current_desc = NEXT_DESCRIPTOR(current_desc)) {

        if(current_desc->bDescriptorType == USB_ENDPOINT_DESCRIPTOR) {
            usb_endpoint_descriptor_t *ep_desc = (usb_endpoint_descriptor_t *) current_desc;

            if(ep_desc->bmAttributes == USB_BULK_TRANSFER) {
                if(ep_desc->bEndpointAddress & 0x80) {
                    *in = ep_desc->bEndpointAddress;
                } else {
                    *out = ep_desc->bEndpointAddress;
                }
            } else if(ep_desc->bmAttributes == USB_INTERRUPT_TRANSFER && ep_desc->bEndpointAddress & 0x80) {
                *inter = ep_desc->bEndpointAddress;
            }
        }
    }
}

void
get_eps_any_setting(usb_device_t dev, usb_descriptor_t *start_desc, usb_descriptor_t *config_end, uint8_t interface,
                    uint8_t *ret_in, uint8_t *ret_out) {
    usb_descriptor_t *int_search_pos;
    uint8_t in = 0, out = 0, inter = 0;
    for(int_search_pos = (usb_descriptor_t *) start_desc;;) {
        usb_interface_descriptor_t *int_desc;
        // This crashes the compiler if I put it on the same line as above.
        int_desc = get_interface_descriptor(int_search_pos, config_end, interface);
        if(!int_desc) break;

        custom_printf("Sub: int %u, setting %u\n", interface, int_desc->bAlternateSetting);

        get_endpoints(int_desc, config_end, &in, &out, &inter);

        custom_printf("Eps: in %02x, out %02x, inter %02x\n", in, out, inter);

        if(in && out) {
            usb_error_t error;
            if(interface > 0) {
                custom_printf("Setting interface %p with length %u\n", int_desc,
                              (uint8_t *) config_end - (uint8_t *) int_desc);
                error = usb_SetInterface(dev, int_desc, (uint8_t *) config_end - (uint8_t *) int_desc);
                if(error) {
                    custom_printf("Error %u when setting interface\n", error);
                    return;
                }
            }
            *ret_in = in;
            *ret_out = out;
            return;
        }

        int_search_pos = NEXT_DESCRIPTOR(int_desc);
    }
}

bool process_config_descriptor(usb_device_t dev, uint8_t config_num) {
    uint8_t conf_desc_buf[MAX_CONFIG_LENGTH];
    usb_configuration_descriptor_t *conf_desc = (usb_configuration_descriptor_t *) conf_desc_buf;
    usb_descriptor_t *config_end;
    size_t config_length;
    usb_error_t error;
    uint8_t current_int;
    bool netif_found = false;

    custom_printf("processing configuration %u\n", config_num);

    config_length = usb_GetConfigurationDescriptorTotalLength(dev, config_num - 1);

    if(config_length > MAX_CONFIG_LENGTH) {
        custom_printf("Config desc too long (%u)\n", config_length);
        return false;
    }

    error = usb_GetDescriptor(dev, USB_CONFIGURATION_DESCRIPTOR,
                              config_num - 1, conf_desc_buf, config_length, NULL);
    if(error) {
        custom_printf("error %u getting config descriptor\n", error);
        return false;
    }

    error = usb_SetConfiguration(dev, conf_desc, config_length);
    if(error) {
        custom_printf("error %u setting config\n", error);
        return false;
    }
    mainlog("set config\n");

    config_end = (usb_descriptor_t *) (conf_desc_buf + config_length);

    custom_printf("Config: starts %p, length %u, ends %p\n", conf_desc, config_length, config_end);

    for(current_int = 0; current_int < conf_desc->bNumInterfaces; current_int++) {
        usb_descriptor_t *search_pos;
        custom_printf("Finding interface %u\n", current_int);
        for(search_pos = (usb_descriptor_t *) conf_desc;;) {
            cdc_ethernet_descriptor_t *eth_desc;
            usb_interface_descriptor_t *int_desc;
            // This crashes the compiler if I put it on the same line as above.
            int_desc = get_interface_descriptor(search_pos, config_end, current_int);
            if(!int_desc) break;

            custom_printf("%p: Interface %u, setting %u\n", int_desc, int_desc->bInterfaceNumber,
                          int_desc->bAlternateSetting);

            eth_desc = get_ether_desc(int_desc, config_end);
            if(eth_desc) {
                uint8_t in = 0, out = 0, inter = 0;
                uint8_t num_subs, current_sub;
                uint8_t *subs;
                netif_state_t *state;
                struct netif *netif;

                custom_printf("Has ether descriptor\n");

                subs = get_subordinate_ints(int_desc, config_end, &num_subs);
                for(current_sub = 0; current_sub < num_subs; current_sub++) {
                    custom_printf("Getting subordinate %u\n", subs[current_sub]);
                    get_eps_any_setting(dev, (usb_descriptor_t *) conf_desc, config_end, subs[current_sub], &in, &out);
                }
                get_endpoints(int_desc, config_end, &in, &out, &inter);

                if(!in || !out) {
                    search_pos = NEXT_DESCRIPTOR(int_desc);
                    continue;
                }

                custom_printf("Endpoints found: in %02x, out %02x, int: %02x\n", in, out, inter);

                if(int_desc->bAlternateSetting) {
                    custom_printf("Setting interface %p with length %u\n", int_desc,
                                  (uint8_t *) config_end - (uint8_t *) int_desc);
                    error = usb_SetInterface(dev, int_desc, (uint8_t *) config_end - (uint8_t *) int_desc);
                    if(error) {
                        custom_printf("Error %u when setting interface\n", error);
                        break;
                    }
                }

                netif = malloc(sizeof(struct netif));
                if(!netif) {
                    mainlog("Failed to malloc netif\n");
                    break;
                }
                state = malloc(sizeof(netif_state_t));
                if(!state) {
                    mainlog("Failed to malloc state\n");
                    free(netif);
                    break;
                }

                state->dev = dev;
                state->in = usb_GetDeviceEndpoint(dev, in);
                state->out = usb_GetDeviceEndpoint(dev, out);
                if(!state->in || !state->out) {
                    free(netif);
                    free(state);
                }

                state->type = DEVICE_CDC_ECM;
                state->mac_index = eth_desc->iMACAddress;
                state->mtu = eth_desc->wMaxSegmentSize;

#if LWIP_DHCP
                netif_add(netif, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY, state, ecm_init_netif, netif_input);
#else
                netif_add(netif, &ip, &netmask, &gateway, state, ecm_init_netif, netif_input);
#endif

                netif_found = true;

                break;
            }

            search_pos = NEXT_DESCRIPTOR(int_desc);
        }
    }

    return netif_found;
}

usb_error_t usb_handle_event(usb_event_t event, void *event_data, usb_callback_data_t *callback_data) {
    if(event == USB_DEVICE_CONNECTED_EVENT) {
        return usb_handle_connect(event_data);
    } else if(event == USB_DEVICE_DISCONNECTED_EVENT) {
        return usb_handle_disconnect(event_data);
    }
    return USB_SUCCESS;
}

usb_error_t usb_handle_connect(usb_device_t dev) {
    usb_device_descriptor_t dev_desc;
    uint8_t config;
    usb_error_t error;

    mainlog("Device connected\n");

    if(!(usb_GetDeviceFlags(dev) & USB_IS_ENABLED)) {
        usb_ResetDevice(dev);
        usb_WaitForEvents();
        custom_printf("Device reset\n");
    }

    error = usb_GetDescriptor(dev, USB_DEVICE_DESCRIPTOR, 0, &dev_desc, sizeof(dev_desc), NULL);
    if(error) {
        custom_printf("error %u getting device descriptor\n", error);
        // todo: fix
        dev_desc.bNumConfigurations = 2;
        //return USB_SUCCESS;
    }

    error = usb_GetConfiguration(dev, &config);
    if(error) {
        custom_printf("error %u when getting current config\n", error);
        // todo: fix
        config = 0;
        //return USB_SUCCESS;
    }
    custom_printf("device has config %u\n", config);
    if(config) {
        process_config_descriptor(dev, config);
        return USB_SUCCESS;
    }

    for(config = 1; config <= dev_desc.bNumConfigurations; config++) {
        process_config_descriptor(dev, config);
    }

    return USB_SUCCESS;
}

usb_error_t usb_handle_disconnect(usb_device_t dev) {
    mainlog("device disconnected\n");
    return USB_SUCCESS;
}

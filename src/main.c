#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>
#include <string.h>

#include <keypadc.h>
#include <usbdrvce.h>
#include <fontlibc.h>
#include <fileioc.h>
#include <graphx.h>

#include "lwIP/include/lwip/init.h"
#include "lwIP/include/lwip/netif.h"
#include "lwIP/include/lwip/dhcp.h"
#include "lwIP/include/lwip/timeouts.h"
#include "lwIP/include/lwip/etharp.h"
#include "lwIP/include/lwip/snmp.h"
#include "lwIP/include/lwip/apps/httpd.h"

//temp globals
usb_device_t dev;
usb_endpoint_t in, out;
uint24_t pnum;

#define CONFIGURATION 2
#define EP_IN 0x81
#define EP_OUT 0x02
#define ETHERNET_MTU 1500

#define PACKET_CAPTURE 1

uint8_t mac_addr[6] = {0xA0, 0xCE, 0xC8, 0xE0, 0x0F, 0x35};

uint8_t eth_data[ETHERNET_MTU + 18]; // verify this, lol
uint8_t mac_send_buffer[ETHERNET_MTU + 18];

struct pbuf *pb;
struct netif netif;

char tmpstr[50];

usb_error_t eth_mac_irq(usb_endpoint_t pEndpoint, usb_transfer_status_t status, size_t i, void *pVoid);

void mainlog(char *str) {
	ti_var_t logfile;
	dbg_sprintf(dbgout, "%s\n", str);
	fontlib_DrawString(str);
	fontlib_DrawString("\n");
	logfile = ti_Open("ETHLOG", "a");
	ti_Write(str, strlen(str), 1, logfile);
	ti_PutC('\n', logfile);
	if(kb_IsDown(kb_KeyMode)) {
		ti_SetArchiveStatus(true, logfile);
		ti_Close(logfile);
		*(uint8_t *) 1 = 1; // crash
	}
	ti_Close(logfile);
}

void schedule_read(void) {
	usb_ScheduleTransfer(in, eth_data, sizeof(eth_data), eth_mac_irq, NULL);
	mainlog("scheduled read");
}

static usb_error_t handle_usb_event(usb_event_t event, void *event_data,
                                    usb_callback_data_t *callback_data) {
	if(event == USB_DEVICE_CONNECTED_EVENT) {
		uint8_t buf[256];
		size_t len;
		usb_control_setup_t setup = {0x21, 0x43, 0x1e, 1, 0};
		usb_endpoint_t ep0;
		usb_error_t err;

		mainlog("Device connected.");
		dev = event_data;
		usb_ResetDevice(dev);
		usb_WaitForEvents();

		len = usb_GetConfigurationDescriptorTotalLength(dev, CONFIGURATION - 1);
		err = usb_GetDescriptor(dev, USB_CONFIGURATION_DESCRIPTOR, CONFIGURATION - 1, buf, len, NULL);
		if(err) dbg_sprintf(dbgout, "Error %u on config get\n", err);

		dbg_sprintf(dbgout, "got config descriptor %p (%u)\n", buf, len);

		err = usb_SetConfiguration(dev, (usb_configuration_descriptor_t *) buf, len);
		if(err) dbg_sprintf(dbgout, "Error %u on config set\n", err);

		mainlog("set config");

		usb_SetInterface(dev, (usb_interface_descriptor_t * )(&buf[0x39]), len - 0x39);

		mainlog("set interface");

		in = usb_GetDeviceEndpoint(dev, EP_IN);
		dbg_sprintf(dbgout, "got in endpoint: %p\n", in);
		out = usb_GetDeviceEndpoint(dev, EP_OUT);
		dbg_sprintf(dbgout, "got out endpoint: %p\n", out);

		ep0 = usb_GetDeviceEndpoint(dev, 0);
		dbg_sprintf(dbgout, "got endpoint 0: %p\n", ep0);

		mainlog("got endpoints");

		if(!in || !out || !ep0) return USB_SUCCESS;

		err = usb_ControlTransfer(ep0, &setup, NULL, 1, NULL);
		if(err) dbg_sprintf(dbgout, "Error %u on filter set\n", err);

		mainlog("set filter");

		schedule_read();

	} else if(event == USB_DEVICE_DISCONNECTED_EVENT) {
		mainlog("device disconnected");
		netif_set_down(&netif);
	}
	return USB_SUCCESS;
}

usb_error_t eth_mac_irq(usb_endpoint_t pEndpoint, usb_transfer_status_t status, size_t size, void *pVoid) {
	/* Service MAC IRQ here */
	/* Allocate pbuf from pool (avoid using heap in interrupts) */
	char filename[9];
	ti_var_t slot;
	struct pbuf *p;
	p = pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
	if(p != NULL) {

#if PACKET_CAPTURE
		sprintf(filename, "P%05uRX", pnum);
		slot = ti_Open(filename, "w");
		if(slot) {
			ti_Write(eth_data, size, 1, slot);
			ti_SetArchiveStatus(true, slot);
			ti_Close(slot);
		}
		sprintf(tmpstr, "Read %u got %u bytes.", pnum, size);
		mainlog(tmpstr);
#endif

		/* Copy ethernet frame into pbuf */
		pbuf_take(p, eth_data, size);
		//todo: fix
		pb = p;
		/* Put in a queue which is processed in main loop */
		//if(!queue_try_put(&queue, p)) {
		//    /* queue is full -> packet loss */
		//    pbuf_free(p);
		//}
		pnum++;
	}
	return USB_SUCCESS;
}

static err_t netif_output(struct netif *netif, struct pbuf *p) {
	int unicast;
	char filename[9];
	size_t transferred;
	ti_var_t slot;
	LINK_STATS_INC(link.xmit);
	/* Update SNMP stats (only if you use SNMP) */
	MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);
	unicast = ((((uint8_t *) p->payload)[0] & 0x01) == 0);
	if(unicast) {
		MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
	} else {
		MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
	}

	pbuf_copy_partial(p, mac_send_buffer, p->tot_len, 0);

#ifdef PACKET_CAPTURE
	sprintf(filename, "P%05uTX", pnum);
	slot = ti_Open(filename, "w");
	if(slot) {
		ti_Write(mac_send_buffer, p->tot_len, 1, slot);
		ti_SetArchiveStatus(true, slot);
		ti_Close(slot);
	}
#endif

	/* Start MAC transmit here */

	usb_Transfer(out, mac_send_buffer, p->tot_len, 5, &transferred);
	sprintf(tmpstr, "Transmit %u sent %u bytes of %u", pnum, transferred, p->tot_len);
	mainlog(tmpstr);
	pnum++;
	if(transferred != p->tot_len) {
		//todo: error
	}

	return ERR_OK;
}

static err_t netif_init_2(struct netif *netif) {
	mainlog("netif init called");
	netif->linkoutput = netif_output;
	netif->output = etharp_output;
	//netif->output_ip6 = ethip6_output;
	netif->mtu = ETHERNET_MTU;
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6;
	MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, 100000000);
	memcpy(netif->hwaddr, mac_addr, ETH_HWADDR_LEN);
	netif->hwaddr_len = ETH_HWADDR_LEN;
	return ERR_OK;
}

void nt_process(void) {
	/* Check for received frames, feed them to lwIP */
	//todo: fix
	struct pbuf *p = pb;// = queue_try_get(&queue);
	pb = NULL;
	if(p != NULL) {
		int unicast;
		mainlog("got a packet");
		LINK_STATS_INC(link.recv);

		/* Update SNMP stats (only if you use SNMP) */
		MIB2_STATS_NETIF_ADD(netif, ifinoctets, p->tot_len);
		unicast = ((((uint8_t *) p->payload)[0] & 0x01) == 0);
		if(unicast) {
			MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
		} else {
			MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
		}

		if(netif.input(p, &netif) != ERR_OK) {
			pbuf_free(p);
		}
	}

	/* Cyclic lwIP timers check */
	sys_check_timeouts();
	usb_HandleEvents();
}

void main(void) {
	uint24_t error;
	fontlib_font_t *font;

	ti_CloseAll();
	ti_Delete("ETHLOG");

	gfx_Begin();
	gfx_FillScreen(gfx_black);

	font = fontlib_GetFontByIndex("GOHUFONT", 1);
	if(font) {
		fontlib_SetFont(font, 0);
	} else {
		goto exit;
	}
	fontlib_SetWindowFullScreen();
	fontlib_SetCursorPosition(0, 0);
	fontlib_SetTransparency(false);
	fontlib_SetBackgroundColor(gfx_black);
	fontlib_SetForegroundColor(gfx_white);
	fontlib_SetNewlineOptions(FONTLIB_ENABLE_AUTO_WRAP | FONTLIB_PRECLEAR_NEWLINE | FONTLIB_AUTO_SCROLL);

	mainlog("started");

	usb_Init(handle_usb_event, NULL, NULL, USB_DEFAULT_INIT_FLAGS);

	mainlog("usb initialized");

	while(!dev) {
		kb_Scan();
		if(kb_IsDown(kb_KeyClear)) goto exit;
		usb_HandleEvents();
	}

	lwip_init();

	mainlog("lwIP initialized");

	netif_add(&netif, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY, NULL, netif_init_2, netif_input);
	netif.name[0] = 'e';
	netif.name[1] = '0';
	//netif_create_ip6_linklocal_address(&netif, 1);
	//netif.ip6_autoconfig_enabled = 1;
	//netif_set_status_callback(&netif, netif_status_callback);
	netif_set_default(&netif);
	netif_set_up(&netif);

	netif_set_link_up(&netif);

	mainlog("interface set up");

	/* Start DHCP and HTTPD */
	error = dhcp_start(&netif);
	if(error) {
		sprintf(tmpstr, "error in dhcp start: %u", error);
	}

	mainlog("dhcp started");

	httpd_init();

	mainlog("webserver initialized");

	kb_Scan();
	while(!kb_IsDown(kb_KeyClear)) {
		nt_process();

		/* your application goes here */
	}

	exit:
	usb_Cleanup();
	gfx_End();
}

#include <stddef.h>
#include <stdint.h>

#include <stdio.h>

#include <keypadc.h>
#include <usbdrvce.h>
#include <fileioc.h>
#include <graphx.h>

#ifdef GRAPHICS
#include <fontlibc.h>
#endif

#include "lwIP/include/lwip/netif.h"
#include "lwIP/include/lwip/apps/httpd.h"
#include "device/usb.h"
#include "log.h"
#include "nanotube.h"
#include "lwIP/include/lwip/apps/tftp_server.h"
#include "tftp.h"
#include "lwIP/apps/tcpecho_raw/echo.h"
#include "lwIP/include/lwip/apps/http_client.h"

#define ETHERNET_MTU 1500

//todo: remove
const uint8_t mac_addr[6] = {0xA0, 0xCE, 0xC8, 0xE0, 0x0F, 0x35};

uint8_t eth_data[ETHERNET_MTU + 18];

//will either save space or look cool
//uint8_t *eth_data = gfx_vram;

static usb_error_t handle_usb_event(usb_event_t event, void *event_data,
                                    usb_callback_data_t *callback_data) {
	if(event == USB_DEVICE_CONNECTED_EVENT) {
		usb_handle_connect(event_data);
	} else if(event == USB_DEVICE_DISCONNECTED_EVENT) {
		usb_handle_disconnect(event_data);
	}
	return USB_SUCCESS;
}

void httpc_transfer_callback(void *arg, httpc_result_t httpc_result, u32_t rx_content_len, u32_t srv_res, err_t err) {
    custom_printf("HTTP callback: res %u, length %u, status %u, err %u\n", httpc_result, (uint24_t)rx_content_len, (uint24_t)srv_res, err);
}

err_t header_callback(httpc_state_t *connection, void *arg, struct pbuf *hdr, u16_t hdr_len,
                     u32_t content_len) {
    uint24_t print_length = hdr_len + content_len;
    if(print_length > 400) print_length = 400;
    custom_printf("Header callback: header len %u, content length %u, data:\n", hdr_len, (uint24_t)content_len);
    custom_printf("%.*s\n", print_length, hdr->payload);
    return ERR_OK;
}

err_t http_data_callback(void *arg, struct tcp_pcb *tpcb,
                         struct pbuf *p, err_t err) {
    uint24_t print_length = p->len;
    if(print_length > 400) print_length = 400;
    custom_printf("HTTP data callback: err %u, len %u, data:\n", err, p->len, print_length, p->payload);
    custom_printf("%.*s\n", print_length, p->payload);
    return ERR_OK;
}

const httpc_connection_t httpcConnection = {
    0,
    0,
    false,
    httpc_transfer_callback,
    header_callback
};

const ip4_addr_t http_server = IPADDR4_INIT_BYTES(199,15,107,244);

void main(void) {
#ifdef GRAPHICS
	fontlib_font_t *font;
#endif

	if(!log_init()) {
	    os_SetCursorPos(0, 0);
	    os_PutStrFull("No logfile.");
	    return;
	}

#ifdef GRAPHICS
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
#endif

	nt_init();
	mainlog("nt_init called\n");

	usb_Init(handle_usb_event, NULL, NULL, USB_DEFAULT_INIT_FLAGS);
	mainlog("usb initialized\n");

	//todo: determine whether I can remove this
	//wait for interface
	while(!netif_default) {
		kb_Scan();
		if(kb_IsDown(kb_KeyClear)) goto exit;
		usb_HandleEvents();
	}
	mainlog("got netif\n");

	//httpd_init();
	//mainlog("webserver initialized\n");

	//tftp_init(&tftpContext);
	//mainlog("tftp initialized\n");

	//echo_init();
	//mainlog("tcpecho initialized\n");

    httpc_get_file(&http_server, 80, "/test.html", &httpcConnection, http_data_callback, NULL, NULL);
	mainlog("requested page over HTTP\n");

	/* Main loop */
	do {
		kb_Scan();
		nt_process();
		/* You would put other stuff here, presumably */
	} while(!kb_IsDown(kb_KeyClear));

	exit:
    tftp_cleanup();
	usb_Cleanup();
	ti_CloseAll();
#ifdef GRAPHICS
	gfx_End();
#endif
}

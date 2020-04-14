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

#define ETHERNET_MTU 1500

//todo: remove
uint8_t mac_addr[6] = {0xA0, 0xCE, 0xC8, 0xE0, 0x0F, 0x35};

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
	mainlog("nt_init called");

	usb_Init(handle_usb_event, NULL, NULL, USB_DEFAULT_INIT_FLAGS);
	mainlog("usb initialized");

	//todo: determine whether I can remove this
	//wait for interface
	while(!netif_default) {
		kb_Scan();
		if(kb_IsDown(kb_KeyClear)) goto exit;
		usb_HandleEvents();
	}
	mainlog("got netif");

	httpd_init();
	mainlog("webserver initialized");

	tftp_init(&tftpContext);

	echo_init();
	mainlog("tcpecho initialized");

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

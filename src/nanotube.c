#include <usbdrvce.h>
#include "device/usb.h"
#include "lwIP/include/lwip/netif.h"
#include "lwIP/include/lwip/timeouts.h"
#include "lwIP/include/lwip/arch/time.h"
#include "lwIP/include/lwip/init.h"

void nt_init(void) {
	mainlog("starting\n");
	lwip_init();

	mainlog("lwIP initialized\n");
	timer_init();
}

void nt_process(void) {
	struct netif *netif;

	NETIF_FOREACH(netif) {
		netif_state_t *state = netif->state;
		struct pbuf *p = queue_get(&state->queue);

		/* Check for received frames, feed them to lwIP */
		if(p != NULL) {
//		    int unicast;
            //mainlog("got a packet\n");
            LINK_STATS_INC(link.recv);

//            /* Update SNMP stats (only if you use SNMP) */
//            MIB2_STATS_NETIF_ADD(netif, ifinoctets, p->tot_len);
//            unicast = ((((uint8_t *) p->payload)[0] & 0x01) == 0);
//            if(unicast) {
//                MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
//            } else {
//                MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
//            }

			if(netif->input(p, netif) != ERR_OK) {
				pbuf_free(p);
			}
			//mainlog("packet processed\n");
		}
	}

	/* Cyclic lwIP timers check */
	sys_check_timeouts();
	usb_HandleEvents();
}
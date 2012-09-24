#include "contiki.h"
#include "contiki-net.h"
#include "dev/leds.h"
#include "hrclock.h"

#include <AppHardwareApi.h>

PROCESS(sync_master, "sync_master");
AUTOSTART_PROCESSES(&sync_master);

#define SEND_INTERVAL 500

PROCESS_THREAD(sync_master, ev, data)
{
  static struct etimer send_et;
  static struct rtimer rt;

  PROCESS_BEGIN();

  //Backoff
  MAC_vPibSetMinBe(pvAppApiGetMacHandle(), 0);
  MAC_vPibSetMaxCsmaBackoffs(pvAppApiGetMacHandle(), 0);

  static uip_ipaddr_t addr;
  uiplib_ipaddrconv("ff02::1", &addr); 

  while(1) {
    etimer_set(&send_et, SEND_INTERVAL);
    PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_TIMER);
    *UIP_ICMP6_TIMESTAMP = clock_hrtime();
    uip_icmp6_send(&addr, ICMP6_TIMESYNC, 1, sizeof(hrclock_t));
    leds_toggle(LEDS_ALL);
  }

  PROCESS_END();
}

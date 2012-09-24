#include "contiki.h"
#include "contiki-net.h"
#include "dev/leds.h"
#include "hrclock.h"
#include "cfs.h"

PROCESS(synced_thread, "synced_thread");
AUTOSTART_PROCESSES(&synced_thread);

static int fd;
static char buf[512];

void toggle(struct rtimer *t, void *ptr)
{
  leds_toggle(LEDS_ALL);
  cfs_write(fd, buf, sizeof(buf));
  //geschriebene Daten SIchern
  cfs_close(fd);
  fd = cfs_open("/synctest.txt", CFS_READ | CFS_WRITE);
  clock_delay(1);
  rtimer_set(t, t->time + 1000, 0, toggle, NULL);
}

PROCESS_THREAD(synced_thread, ev, data)
{
  static struct uip_udp_conn *udpconn;
  static struct rtimer rt;

  PROCESS_BEGIN();

  //Backoff
  MAC_vPibSetMinBe(pvAppApiGetMacHandle(), 0);
  MAC_vPibSetMaxCsmaBackoffs(pvAppApiGetMacHandle(), 0);

  //wird benötigt um stack zu initialisieren?!
  udpconn = udp_broadcast_new(UIP_HTONS(10000), NULL);
  
  clock_delay(CLOCK_SECOND);
  //beim ersten aufruf wird SD karte initialisiert
  fd = cfs_open("/synctest.txt", CFS_READ | CFS_WRITE);
  int i;
  for (i = 0; i < sizeof(buf); i++)
    buf[i] = 'a';

  while(!clock_synced())
  {
    process_poll(&synced_thread);
    PROCESS_YIELD();
  }


  rtimer_init();
  clock_time_t time = clock_synced_hrtime()/1000;
  time += 1000 - time%1000;
  rtimer_set(&rt, time, 0, toggle, NULL); 

  while(1){
    PROCESS_YIELD();
  }
  PROCESS_END();
}

/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A simple power saving MAC protocol based on X-MAC [SenSys 2006]
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#include "dev/leds.h"
#include "dev/radio.h"
#include "dev/watchdog.h"
#include "lib/random.h"
#include "net/netstack.h"
#include "net/mac/xmac.h"
#include "net/rime.h"
#include "net/rime/timesynch.h"
#include "sys/compower.h"
#include "sys/pt.h"
#include "sys/rtimer.h"

#include "contiki-conf.h"

#ifdef EXPERIMENT_SETUP
#include "experiment-setup.h"
#endif

#include <string.h>

#ifndef WITH_ACK_OPTIMIZATION
#define WITH_ACK_OPTIMIZATION        0
#endif
#ifndef WITH_ENCOUNTER_OPTIMIZATION
#define WITH_ENCOUNTER_OPTIMIZATION  1
#endif
#ifndef WITH_STREAMING
#define WITH_STREAMING               1
#endif
#ifndef WITH_STROBE_BROADCAST
#define WITH_STROBE_BROADCAST        0
#endif

struct announcement_data {
  uint16_t id;
  uint16_t value;
};

/* The maximum number of announcements in a single announcement
   message - may need to be increased in the future. */
#define ANNOUNCEMENT_MAX 10

/* The structure of the announcement messages. */
struct announcement_msg {
  uint16_t num;
  struct announcement_data data[ANNOUNCEMENT_MAX];
};

/* The length of the header of the announcement message, i.e., the
   "num" field in the struct. */
#define ANNOUNCEMENT_MSG_HEADERLEN (sizeof (uint16_t))

#define DISPATCH          0
#define TYPE_STROBE       0x10
/* #define TYPE_DATA         0x11 */
#define TYPE_ANNOUNCEMENT 0x12
#define TYPE_STROBE_ACK   0x13

struct xmac_hdr {
  uint8_t dispatch;
  uint8_t type;
};

#define MAX_STROBE_SIZE 50

#ifdef XMAC_CONF_ON_TIME
#define DEFAULT_ON_TIME (XMAC_CONF_ON_TIME)
#else
#define DEFAULT_ON_TIME (RTIMER_ARCH_SECOND / 160)
#endif

#ifdef XMAC_CONF_OFF_TIME
#define DEFAULT_OFF_TIME (XMAC_CONF_OFF_TIME)
#else
#define DEFAULT_OFF_TIME (RTIMER_ARCH_SECOND / NETSTACK_RDC_CHANNEL_CHECK_RATE - DEFAULT_ON_TIME)
#endif

#define DEFAULT_PERIOD (DEFAULT_OFF_TIME + DEFAULT_ON_TIME)

#define WAIT_TIME_BEFORE_STROBE_ACK RTIMER_ARCH_SECOND / 1000

/* On some platforms, we may end up with a DEFAULT_PERIOD that is 0
   which will make compilation fail due to a modulo operation in the
   code. To ensure that DEFAULT_PERIOD is greater than zero, we use
   the construct below. */
#if DEFAULT_PERIOD == 0
#undef DEFAULT_PERIOD
#define DEFAULT_PERIOD 1
#endif

/* The cycle time for announcements. */
#define ANNOUNCEMENT_PERIOD 4 * CLOCK_SECOND

/* The time before sending an announcement within one announcement
   cycle. */
#define ANNOUNCEMENT_TIME (random_rand() % (ANNOUNCEMENT_PERIOD))

#define DEFAULT_STROBE_WAIT_TIME (5 * DEFAULT_ON_TIME / 8)

struct xmac_config xmac_config = {
  DEFAULT_ON_TIME,
  DEFAULT_OFF_TIME,
  4 * DEFAULT_ON_TIME + DEFAULT_OFF_TIME,
  DEFAULT_STROBE_WAIT_TIME
};

#include <stdio.h>
static struct rtimer rt;
static struct pt pt;

static volatile uint8_t xmac_is_on = 0;

static volatile unsigned char waiting_for_packet = 0;
static volatile unsigned char someone_is_sending = 0;
static volatile unsigned char we_are_sending = 0;
static volatile unsigned char radio_is_on = 0;

#undef LEDS_ON
#undef LEDS_OFF
#undef LEDS_TOGGLE

#define LEDS_ON(x) leds_on(x)
#define LEDS_OFF(x) leds_off(x)
#define LEDS_TOGGLE(x) leds_toggle(x)
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINTDEBUG(...) printf(__VA_ARGS__)
#else
#undef LEDS_ON
#undef LEDS_OFF
#undef LEDS_TOGGLE
#define LEDS_ON(x)
#define LEDS_OFF(x)
#define LEDS_TOGGLE(x)
#define PRINTF(...)
#define PRINTDEBUG(...)
#endif

#if XMAC_CONF_ANNOUNCEMENTS
/* Timers for keeping track of when to send announcements. */
static struct ctimer announcement_cycle_ctimer, announcement_ctimer;

static int announcement_radio_txpower;
#endif /* XMAC_CONF_ANNOUNCEMENTS */

/* Flag that is used to keep track of whether or not we are listening
   for announcements from neighbors. */
static uint8_t is_listening;

#if XMAC_CONF_COMPOWER
static struct compower_activity current_packet;
#endif /* XMAC_CONF_COMPOWER */

#if WITH_ENCOUNTER_OPTIMIZATION

#include "lib/list.h"
#include "lib/memb.h"

struct encounter {
  struct encounter *next;
  rimeaddr_t neighbor;
  rtimer_clock_t time;
};

#define MAX_ENCOUNTERS 4
LIST(encounter_list);
MEMB(encounter_memb, struct encounter, MAX_ENCOUNTERS);
#endif /* WITH_ENCOUNTER_OPTIMIZATION */

static uint8_t is_streaming;
static rimeaddr_t is_streaming_to, is_streaming_to_too;
static rtimer_clock_t stream_until;
#define DEFAULT_STREAM_TIME (RTIMER_ARCH_SECOND)

#ifndef MIN
#define MIN(a, b) ((a) < (b)? (a) : (b))
#endif /* MIN */

struct seqno {
  rimeaddr_t sender;
  uint8_t seqno;
};

#define MAX_SEQNOS 8
static struct seqno received_seqnos[MAX_SEQNOS];


/*---------------------------------------------------------------------------*/
static void
on(void)
{
  if(xmac_is_on && radio_is_on == 0) {
    radio_is_on = 1;
    NETSTACK_RADIO.on();
    LEDS_ON(LEDS_RED);
  }
}
/*---------------------------------------------------------------------------*/
static void
off(void)
{
  if(xmac_is_on && radio_is_on != 0 && is_listening == 0 &&
     is_streaming == 0) {
    radio_is_on = 0;
    NETSTACK_RADIO.off();
    LEDS_OFF(LEDS_RED);
  }
}
/*---------------------------------------------------------------------------*/
static char powercycle(struct rtimer *t, void *ptr);
static void
schedule_powercycle(struct rtimer *t, rtimer_clock_t time)
{
  int r;
  if(xmac_is_on) {
    r = rtimer_set(t, RTIMER_TIME(t) + time, 1,
		   (void (*)(struct rtimer *, void *))powercycle, NULL);
    if(r) {
      PRINTF("schedule_powercycle: could not set rtimer\n");
    }
  }
}
static void
powercycle_turn_radio_off(void)
{
  if(we_are_sending == 0 &&
     waiting_for_packet == 0) {
    off();
  }
#if XMAC_CONF_COMPOWER
  compower_accumulate(&compower_idle_activity);
#endif /* XMAC_CONF_COMPOWER */
}
static void
powercycle_turn_radio_on(void)
{
  if(we_are_sending == 0 &&
     waiting_for_packet == 0) {
    on();
  }
}
static char
powercycle(struct rtimer *t, void *ptr)
{
  if(is_streaming) {
    if(!RTIMER_CLOCK_LT(RTIMER_NOW(), stream_until)) {
      is_streaming = 0;
      rimeaddr_copy(&is_streaming_to, &rimeaddr_null);
      rimeaddr_copy(&is_streaming_to_too, &rimeaddr_null);
    }
  }

  PT_BEGIN(&pt);

  while(1) {
    /* Only wait for some cycles to pass for someone to start sending */
    if(someone_is_sending > 0) {
      someone_is_sending--;
    }

    /* If there were a strobe in the air, turn radio on */
    powercycle_turn_radio_on();
    schedule_powercycle(t, xmac_config.on_time);
    PT_YIELD(&pt);

    if(xmac_config.off_time > 0 && !NETSTACK_RADIO.receiving_packet()) {
      powercycle_turn_radio_off();
      if(waiting_for_packet != 0) {
	waiting_for_packet++;
	if(waiting_for_packet > 2) {
	  /* We should not be awake for more than two consecutive
	     power cycles without having heard a packet, so we turn off
	     the radio. */
	  waiting_for_packet = 0;
	  powercycle_turn_radio_off();
	}
      }
      schedule_powercycle(t, xmac_config.off_time);
      PT_YIELD(&pt);
    }
  }

  PT_END(&pt);
}
/*---------------------------------------------------------------------------*/
#if XMAC_CONF_ANNOUNCEMENTS
static int
parse_announcements(const rimeaddr_t *from)
{
  /* Parse incoming announcements */
  struct announcement_msg adata;
  int i;

  memcpy(&adata, packetbuf_dataptr(), MIN(packetbuf_datalen(), sizeof(adata)));

  /*  printf("%d.%d: probe from %d.%d with %d announcements\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	 from->u8[0], from->u8[1], adata->num);*/
  /*  for(i = 0; i < packetbuf_datalen(); ++i) {
    printf("%02x ", ((uint8_t *)packetbuf_dataptr())[i]);
  }
  printf("\n");*/

  for(i = 0; i < adata.num; ++i) {
    /*   printf("%d.%d: announcement %d: %d\n",
	  rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	  adata->data[i].id,
	  adata->data[i].value);*/

    announcement_heard(from,
		       adata.data[i].id,
		       adata.data[i].value);
  }
  return i;
}
/*---------------------------------------------------------------------------*/
static int
format_announcement(char *hdr)
{
  struct announcement_msg adata;
  struct announcement *a;

  /* Construct the announcements */
  /*  adata = (struct announcement_msg *)hdr;*/

  adata.num = 0;
  for(a = announcement_list();
      a != NULL && adata.num < ANNOUNCEMENT_MAX;
      a = list_item_next(a)) {
    adata.data[adata.num].id = a->id;
    adata.data[adata.num].value = a->value;
    adata.num++;
  }

  memcpy(hdr, &adata, sizeof(struct announcement_msg));

  if(adata.num > 0) {
    return ANNOUNCEMENT_MSG_HEADERLEN +
      sizeof(struct announcement_data) * adata.num;
  } else {
    return 0;
  }
}
#endif /* XMAC_CONF_ANNOUNCEMENTS */
/*---------------------------------------------------------------------------*/
#if WITH_ENCOUNTER_OPTIMIZATION
static void
register_encounter(const rimeaddr_t *neighbor, rtimer_clock_t time)
{
  struct encounter *e;

  /* If we have an entry for this neighbor already, we renew it. */
  for(e = list_head(encounter_list); e != NULL; e = list_item_next(e)) {
    if(rimeaddr_cmp(neighbor, &e->neighbor)) {
      e->time = time;
      break;
    }
  }
  /* No matching encounter was found, so we allocate a new one. */
  if(e == NULL) {
    e = memb_alloc(&encounter_memb);
    if(e == NULL) {
      /* We could not allocate memory for this encounter, so we just drop it. */
      return;
    }
    rimeaddr_copy(&e->neighbor, neighbor);
    e->time = time;
    list_add(encounter_list, e);
  }
}
#endif /* WITH_ENCOUNTER_OPTIMIZATION */
/*---------------------------------------------------------------------------*/
static int
detect_ack(void)
{
#define INTER_PACKET_INTERVAL              RTIMER_ARCH_SECOND / 5000
#define ACK_LEN 3
#define AFTER_ACK_DETECTECT_WAIT_TIME      RTIMER_ARCH_SECOND / 1000
  rtimer_clock_t wt;
  uint8_t ack_received = 0;
  
  wt = RTIMER_NOW();
  leds_on(LEDS_GREEN);
  while(RTIMER_CLOCK_LT(RTIMER_NOW(), wt + INTER_PACKET_INTERVAL)) { }
  leds_off(LEDS_GREEN);
  /* Check for incoming ACK. */
  if((NETSTACK_RADIO.receiving_packet() ||
      NETSTACK_RADIO.pending_packet() ||
      NETSTACK_RADIO.channel_clear() == 0)) {
    int len;
    uint8_t ackbuf[ACK_LEN + 2];
    
    wt = RTIMER_NOW();
    while(RTIMER_CLOCK_LT(RTIMER_NOW(), wt + AFTER_ACK_DETECTECT_WAIT_TIME)) { }
    
    len = NETSTACK_RADIO.read(ackbuf, ACK_LEN);
    if(len == ACK_LEN) {
      ack_received = 1;
    }
  }
  if(ack_received) {
    leds_toggle(LEDS_RED);
  }
  return ack_received;
}
/*---------------------------------------------------------------------------*/
static int
send_packet(void)
{
  rtimer_clock_t t0;
  rtimer_clock_t t;
  rtimer_clock_t encounter_time = 0;
  int strobes;
  int ret;
#if 0
  struct xmac_hdr *hdr;
#endif
  uint8_t got_strobe_ack = 0;
  uint8_t got_ack = 0;
  uint8_t strobe[MAX_STROBE_SIZE];
  int strobe_len, len;
  int is_broadcast = 0;
/*int is_reliable; */
  struct encounter *e;
  struct queuebuf *packet;
  int is_already_streaming = 0;
  uint8_t collisions;

  /* Create the X-MAC header for the data packet. */
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &rimeaddr_node_addr);
  if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), &rimeaddr_null)) {
    is_broadcast = 1;
    PRINTDEBUG("xmac: send broadcast\n");
  } else {
#if UIP_CONF_IPV6
    PRINTDEBUG("xmac: send unicast to %02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[0],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[1],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[2],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[3],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[4],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[5],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[6],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[7]);
#else
    PRINTDEBUG("xmac: send unicast to %u.%u\n",
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[0],
           packetbuf_addr(PACKETBUF_ADDR_RECEIVER)->u8[1]);
#endif /* UIP_CONF_IPV6 */
  }
/*  is_reliable = packetbuf_attr(PACKETBUF_ATTR_RELIABLE) ||
    packetbuf_attr(PACKETBUF_ATTR_ERELIABLE); */

  packetbuf_set_attr(PACKETBUF_ATTR_MAC_ACK, 1);
  len = NETSTACK_FRAMER.create();
  strobe_len = len + sizeof(struct xmac_hdr);
  if(len < 0 || strobe_len > (int)sizeof(strobe)) {
    /* Failed to send */
   PRINTF("xmac: send failed, too large header\n");
    return MAC_TX_ERR_FATAL;
  }
  memcpy(strobe, packetbuf_hdrptr(), len);
  strobe[len] = DISPATCH; /* dispatch */
  strobe[len + 1] = TYPE_STROBE; /* type */

  packetbuf_compact();
  packet = queuebuf_new_from_packetbuf();
  if(packet == NULL) {
    /* No buffer available */
    PRINTF("xmac: send failed, no queue buffer available (of %u)\n",
           QUEUEBUF_CONF_NUM);
    return MAC_TX_ERR;
  }

#if WITH_STREAMING
  if(is_streaming == 1 &&
     (rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
		   &is_streaming_to) ||
      rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
		   &is_streaming_to_too))) {
    is_already_streaming = 1;
  }
  if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) ==
     PACKETBUF_ATTR_PACKET_TYPE_STREAM) {
    is_streaming = 1;
    if(rimeaddr_cmp(&is_streaming_to, &rimeaddr_null)) {
      rimeaddr_copy(&is_streaming_to, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
    } else if(!rimeaddr_cmp(&is_streaming_to, packetbuf_addr(PACKETBUF_ADDR_RECEIVER))) {
      rimeaddr_copy(&is_streaming_to_too, packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
    }
    stream_until = RTIMER_NOW() + DEFAULT_STREAM_TIME;
  }
#endif /* WITH_STREAMING */

  off();

#if WITH_ENCOUNTER_OPTIMIZATION
  /* We go through the list of encounters to find if we have recorded
     an encounter with this particular neighbor. If so, we can compute
     the time for the next expected encounter and setup a ctimer to
     switch on the radio just before the encounter. */
  for(e = list_head(encounter_list); e != NULL; e = list_item_next(e)) {
    const rimeaddr_t *neighbor = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);

    if(rimeaddr_cmp(neighbor, &e->neighbor)) {
      rtimer_clock_t wait, now, expected;

      /* We expect encounters to happen every DEFAULT_PERIOD time
	 units. The next expected encounter is at time e->time +
	 DEFAULT_PERIOD. To compute a relative offset, we subtract
	 with clock_time(). Because we are only interested in turning
	 on the radio within the DEFAULT_PERIOD period, we compute the
	 waiting time with modulo DEFAULT_PERIOD. */

      now = RTIMER_NOW();
      wait = ((rtimer_clock_t)(e->time - now)) % (DEFAULT_PERIOD);
      if(wait < 2 * DEFAULT_ON_TIME) {
        wait = DEFAULT_PERIOD;
      }
      expected = now + wait - 2 * DEFAULT_ON_TIME;

#if WITH_ACK_OPTIMIZATION
      /* Wait until the receiver is expected to be awake */
      if(packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) !=
	 PACKETBUF_ATTR_PACKET_TYPE_ACK &&
	 is_streaming == 0) {
	/* Do not wait if we are sending an ACK, because then the
	   receiver will already be awake. */
	while(RTIMER_CLOCK_LT(RTIMER_NOW(), expected));
      }
#else /* WITH_ACK_OPTIMIZATION */
      /* Wait until the receiver is expected to be awake */
      while(RTIMER_CLOCK_LT(RTIMER_NOW(), expected));
#endif /* WITH_ACK_OPTIMIZATION */
    }
  }
#endif /* WITH_ENCOUNTER_OPTIMIZATION */

  /* By setting we_are_sending to one, we ensure that the rtimer
     powercycle interrupt do not interfere with us sending the packet. */
  we_are_sending = 1;
  
  t0 = RTIMER_NOW();
  strobes = 0;

  LEDS_ON(LEDS_BLUE);

  /* Send a train of strobes until the receiver answers with an ACK. */

  /* Turn on the radio to listen for the strobe ACK. */
  //  on();
  collisions = 0;
  if(!is_already_streaming) {
    watchdog_stop();
    got_strobe_ack = 0;
    t = RTIMER_NOW();
    for(strobes = 0, collisions = 0;
	got_strobe_ack == 0 && collisions == 0 &&
	  RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + xmac_config.strobe_time);
	strobes++) {

      while(got_strobe_ack == 0 &&
	    RTIMER_CLOCK_LT(RTIMER_NOW(), t + xmac_config.strobe_wait_time)) {
#if 0
	rtimer_clock_t now = RTIMER_NOW();

	/* See if we got an ACK */
	packetbuf_clear();
	len = NETSTACK_RADIO.read(packetbuf_dataptr(), PACKETBUF_SIZE);
	if(len > 0) {
	  packetbuf_set_datalen(len);
	  if(NETSTACK_FRAMER.parse() >= 0) {
	    hdr = packetbuf_dataptr();
	    if(hdr->dispatch == DISPATCH && hdr->type == TYPE_STROBE_ACK) {
	      if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
			      &rimeaddr_node_addr)) {
		/* We got an ACK from the receiver, so we can immediately send
		   the packet. */
		got_strobe_ack = 1;
		encounter_time = now;
	      } else {
		PRINTDEBUG("xmac: strobe ack for someone else\n");
	      }
	    } else /*if(hdr->dispatch == DISPATCH && hdr->type == TYPE_STROBE)*/ {
	      PRINTDEBUG("xmac: strobe from someone else\n");
	      collisions++;
	    }
	  } else {
	    PRINTF("xmac: send failed to parse %u\n", len);
	  }
	}
#endif /* 0 */
      }
      
      t = RTIMER_NOW();
            /* Send the strobe packet. */
      if(got_strobe_ack == 0 && collisions == 0) {

	if(is_broadcast) {
#if WITH_STROBE_BROADCAST
	  ret = NETSTACK_RADIO.send(strobe, strobe_len);
#else
	  /* restore the packet to send */
	  queuebuf_to_packetbuf(packet);
	  ret = NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
#endif
          off();
	} else {
#if 0
	  rtimer_clock_t wt;
#endif
          on();
	  ret = NETSTACK_RADIO.send(strobe, strobe_len);
#if 0
	  /* Turn off the radio for a while to let the other side
	     respond. We don't need to keep our radio on when we know
	     that the other side needs some time to produce a reply. */
	  off();
	  wt = RTIMER_NOW();
	  while(RTIMER_CLOCK_LT(RTIMER_NOW(), wt + WAIT_TIME_BEFORE_STROBE_ACK));
#endif /* 0 */
#if RDC_CONF_HARDWARE_ACK
          if(ret == RADIO_TX_OK) {
            got_strobe_ack = 1;
          } else {
            off();
          }
#else
          if(detect_ack()) {
            got_strobe_ack = 1;
          } else {
            off();
          }
#endif /* RDC_CONF_HARDWARE_ACK */

        }
      }
    }
  }

#if WITH_ACK_OPTIMIZATION
  /* If we have received the strobe ACK, and we are sending a packet
     that will need an upper layer ACK (as signified by the
     PACKETBUF_ATTR_RELIABLE packet attribute), we keep the radio on. */
  if(got_strobe_ack && (packetbuf_attr(PACKETBUF_ATTR_RELIABLE) ||
			packetbuf_attr(PACKETBUF_ATTR_ERELIABLE) ||
			packetbuf_attr(PACKETBUF_ATTR_PACKET_TYPE) ==
			PACKETBUF_ATTR_PACKET_TYPE_STREAM)) {
    on(); /* Wait for ACK packet */
    waiting_for_packet = 1;
  } else {
    off();
  }
#endif /* WITH_ACK_OPTIMIZATION */

  /* restore the packet to send */
  queuebuf_to_packetbuf(packet);
  queuebuf_free(packet);

  /* Send the data packet. */
  if((is_broadcast || got_strobe_ack || is_streaming) && collisions == 0) {
    ret = NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());

    if(!is_broadcast) {
#if RDC_CONF_HARDWARE_ACK
      if(ret == RADIO_TX_OK) {
        got_ack = 1;
      }
#else
      if(detect_ack()) {
        got_ack = 1;
      }
#endif /* RDC_CONF_HARDWARE_ACK */
    }
  }
  off();

#if WITH_ENCOUNTER_OPTIMIZATION
  if(got_strobe_ack && !is_streaming) {
    register_encounter(packetbuf_addr(PACKETBUF_ADDR_RECEIVER), encounter_time);
  }
#endif /* WITH_ENCOUNTER_OPTIMIZATION */
  watchdog_start();

  PRINTF("xmac: send (strobes=%u,len=%u,%s), done\n", strobes,
	 packetbuf_totlen(), got_strobe_ack ? "ack" : "no ack");

#if XMAC_CONF_COMPOWER
  /* Accumulate the power consumption for the packet transmission. */
  compower_accumulate(&current_packet);

  /* Convert the accumulated power consumption for the transmitted
     packet to packet attributes so that the higher levels can keep
     track of the amount of energy spent on transmitting the
     packet. */
  compower_attrconv(&current_packet);

  /* Clear the accumulated power consumption so that it is ready for
     the next packet. */
  compower_clear(&current_packet);
#endif /* XMAC_CONF_COMPOWER */

  we_are_sending = 0;

  LEDS_OFF(LEDS_BLUE);
  if(collisions == 0) {
    if(is_broadcast == 0 && got_ack == 0) {
      return MAC_TX_NOACK;
    } else {
      return MAC_TX_OK;
    }
  } else {
    someone_is_sending++;
    return MAC_TX_COLLISION;
  }

}
/*---------------------------------------------------------------------------*/
static void
qsend_packet(mac_callback_t sent, void *ptr)
{
  int ret;
  
  if(someone_is_sending) {
    PRINTF("xmac: should queue packet, now just dropping %d %d %d %d.\n",
	   waiting_for_packet, someone_is_sending, we_are_sending, radio_is_on);
    RIMESTATS_ADD(sendingdrop);
    ret = MAC_TX_COLLISION;
  } else {
    PRINTF("xmac: send immediately.\n");
    ret = send_packet();
  }

  mac_call_sent_callback(sent, ptr, ret, 1);
}
/*---------------------------------------------------------------------------*/
static void
qsend_list(mac_callback_t sent, void *ptr, struct rdc_buf_list *buf_list)
{
  if(buf_list != NULL) {
    queuebuf_to_packetbuf(buf_list->buf);
    qsend_packet(sent, ptr);
  }
}
/*---------------------------------------------------------------------------*/
static void
input_packet(void)
{
  struct xmac_hdr *hdr;

  if(NETSTACK_FRAMER.parse() >= 0) {
    hdr = packetbuf_dataptr();

    if(hdr->dispatch != DISPATCH) {
      someone_is_sending = 0;
      if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                                     &rimeaddr_node_addr) ||
	 rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                      &rimeaddr_null)) {
	/* This is a regular packet that is destined to us or to the
	   broadcast address. */

	/* We have received the final packet, so we can go back to being
	   asleep. */
	off();

        /* Check for duplicate packet by comparing the sequence number
           of the incoming packet with the last few ones we saw. */
        {
          int i;
          for(i = 0; i < MAX_SEQNOS; ++i) {
            if(packetbuf_attr(PACKETBUF_ATTR_PACKET_ID) == received_seqnos[i].seqno &&
               rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_SENDER),
                            &received_seqnos[i].sender)) {
              /* Drop the packet. */
              return;
            }
          }
          for(i = MAX_SEQNOS - 1; i > 0; --i) {
            memcpy(&received_seqnos[i], &received_seqnos[i - 1],
                   sizeof(struct seqno));
          }
          received_seqnos[0].seqno = packetbuf_attr(PACKETBUF_ATTR_PACKET_ID);
          rimeaddr_copy(&received_seqnos[0].sender,
                        packetbuf_addr(PACKETBUF_ADDR_SENDER));
        }

#if XMAC_CONF_COMPOWER
	/* Accumulate the power consumption for the packet reception. */
	compower_accumulate(&current_packet);
	/* Convert the accumulated power consumption for the received
	   packet to packet attributes so that the higher levels can
	   keep track of the amount of energy spent on receiving the
	   packet. */
	compower_attrconv(&current_packet);

	/* Clear the accumulated power consumption so that it is ready
	   for the next packet. */
	compower_clear(&current_packet);
#endif /* XMAC_CONF_COMPOWER */

	waiting_for_packet = 0;

        PRINTDEBUG("xmac: data(%u)\n", packetbuf_datalen());
	NETSTACK_MAC.input();
        return;
      } else {
        PRINTDEBUG("xmac: data not for us\n");
      }

    } else if(hdr->type == TYPE_STROBE) {
      someone_is_sending = 2;

      if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                      &rimeaddr_node_addr)) {
	/* This is a strobe packet for us. */

	/* If the sender address is someone else, we should
	   acknowledge the strobe and wait for the packet. By using
	   the same address as both sender and receiver, we flag the
	   message is a strobe ack. */
        waiting_for_packet = 1;
#if 0
	hdr->type = TYPE_STROBE_ACK;
	packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER,
			   packetbuf_addr(PACKETBUF_ADDR_SENDER));
	packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &rimeaddr_node_addr);
	packetbuf_compact();
	if(NETSTACK_FRAMER.create() >= 0) {
	  /* We turn on the radio in anticipation of the incoming
	     packet. */
	  someone_is_sending = 1;
	  waiting_for_packet = 1;
	  on();
	  NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
	  PRINTDEBUG("xmac: send strobe ack %u\n", packetbuf_totlen());
	} else {
	  PRINTF("xmac: failed to send strobe ack\n");
	}
#endif /* 0 */
      } else if(rimeaddr_cmp(packetbuf_addr(PACKETBUF_ADDR_RECEIVER),
                             &rimeaddr_null)) {
	/* If the receiver address is null, the strobe is sent to
	   prepare for an incoming broadcast packet. If this is the
	   case, we turn on the radio and wait for the incoming
	   broadcast packet. */
	waiting_for_packet = 1;
	on();
      } else {
        PRINTDEBUG("xmac: strobe not for us\n");
      }
      
      /* We are done processing the strobe and we therefore return
	 to the caller. */
      return;
#if XMAC_CONF_ANNOUNCEMENTS
    } else if(hdr->type == TYPE_ANNOUNCEMENT) {
      packetbuf_hdrreduce(sizeof(struct xmac_hdr));
      parse_announcements(packetbuf_addr(PACKETBUF_ADDR_SENDER));
#endif /* XMAC_CONF_ANNOUNCEMENTS */
    } else if(hdr->type == TYPE_STROBE_ACK) {
      PRINTDEBUG("xmac: stray strobe ack\n");
    } else {
      PRINTF("xmac: unknown type %u (%u/%u)\n", hdr->type,
             packetbuf_datalen(), len);
    }
  } else {
    PRINTF("xmac: failed to parse (%u)\n", packetbuf_totlen());
  }
}
/*---------------------------------------------------------------------------*/
#if XMAC_CONF_ANNOUNCEMENTS
static void
send_announcement(void *ptr)
{
  struct xmac_hdr *hdr;
  int announcement_len;

  /* Set up the probe header. */
  packetbuf_clear();
  hdr = packetbuf_dataptr();

  announcement_len = format_announcement((char *)hdr +
					 sizeof(struct xmac_hdr));

  if(announcement_len > 0) {
    packetbuf_set_datalen(sizeof(struct xmac_hdr) + announcement_len);
    hdr->dispatch = DISPATCH;
    hdr->type = TYPE_ANNOUNCEMENT;

    packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &rimeaddr_node_addr);
    packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &rimeaddr_null);
    packetbuf_set_attr(PACKETBUF_ATTR_RADIO_TXPOWER, announcement_radio_txpower);
    if(NETSTACK_FRAMER.create() >= 0) {
      NETSTACK_RADIO.send(packetbuf_hdrptr(), packetbuf_totlen());
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
cycle_announcement(void *ptr)
{
  ctimer_set(&announcement_ctimer, ANNOUNCEMENT_TIME,
	     send_announcement, NULL);
  ctimer_set(&announcement_cycle_ctimer, ANNOUNCEMENT_PERIOD,
	     cycle_announcement, NULL);
  if(is_listening > 0) {
    is_listening--;
    /*    printf("is_listening %d\n", is_listening);*/
  }
}
/*---------------------------------------------------------------------------*/
static void
listen_callback(int periods)
{
  is_listening = periods + 1;
}
#endif /* XMAC_CONF_ANNOUNCEMENTS */
/*---------------------------------------------------------------------------*/
void
xmac_set_announcement_radio_txpower(int txpower)
{
#if XMAC_CONF_ANNOUNCEMENTS
  announcement_radio_txpower = txpower;
#endif /* XMAC_CONF_ANNOUNCEMENTS */
}
/*---------------------------------------------------------------------------*/
static void
init(void)
{
  radio_is_on = 0;
  waiting_for_packet = 0;
  PT_INIT(&pt);
  rtimer_set(&rt, RTIMER_NOW() + xmac_config.off_time, 1,
	     (void (*)(struct rtimer *, void *))powercycle, NULL);

  xmac_is_on = 1;

#if WITH_ENCOUNTER_OPTIMIZATION
  list_init(encounter_list);
  memb_init(&encounter_memb);
#endif /* WITH_ENCOUNTER_OPTIMIZATION */

#if XMAC_CONF_ANNOUNCEMENTS
  announcement_register_listen_callback(listen_callback);
  ctimer_set(&announcement_cycle_ctimer, ANNOUNCEMENT_TIME,
	     cycle_announcement, NULL);
#endif /* XMAC_CONF_ANNOUNCEMENTS */
}
/*---------------------------------------------------------------------------*/
static int
turn_on(void)
{
  xmac_is_on = 1;
  rtimer_set(&rt, RTIMER_NOW() + xmac_config.off_time, 1,
	     (void (*)(struct rtimer *, void *))powercycle, NULL);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
turn_off(int keep_radio_on)
{
  xmac_is_on = 0;
  if(keep_radio_on) {
    return NETSTACK_RADIO.on();
  } else {
    return NETSTACK_RADIO.off();
  }
}
/*---------------------------------------------------------------------------*/
static unsigned short
channel_check_interval(void)
{
  return (1ul * CLOCK_SECOND * DEFAULT_PERIOD) / RTIMER_ARCH_SECOND;
}
/*---------------------------------------------------------------------------*/
const struct rdc_driver xmac_driver =
  {
    "X-MAC",
    init,
    qsend_packet,
    qsend_list,
    input_packet,
    turn_on,
    turn_off,
    channel_check_interval,
  };

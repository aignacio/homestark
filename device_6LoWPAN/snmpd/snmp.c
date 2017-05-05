/**
  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing,
  software distributed under the License is distributed on an
  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
  KIND, either express or implied.  See the License for the
  specific language governing permissions and limitations
  under the License.

 *******************************************************************************
 * @license This project is delivered under Apache 2.0 license.
 * @file snmp.c
 * @brief Main functions of SNMP port
 * @author Ânderson Ignácio da Silva
 * @date 12 Sept 2016
 * @see http://www.aignacio.com
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
// #include "apps/servreg-hack/servreg-hack.h"
#include "snmp.h"
#include "mibii.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "simple-udp.h"
#include "list.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "net/ip/uip-debug.h"
#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "sys/ctimer.h"
#include "homestark.h"

#if CONTIKI_TARGET_SRF06_CC26XX
#include "core/net/ipv6/uip-ds6-route.h"
#include "core/net/ipv6/sicslowpan.h"
#include "lib/newlib/syscalls.c" // Used on heap function, like malloc, free, calloc..
#include "button-sensor.h"
#include "batmon-sensor.h"
#include "board-peripherals.h"
#include "ti-lib.h"
#endif

#define printf6addr(addr) debug_snmp("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

#if !CONTIKI_TARGET_Z1
static struct etimer              update_snmp;
#endif
static struct ctimer              trap_timer;
static struct uip_udp_conn        *server_conn;
// static struct uip_udp_conn        *trap_conn;
static struct simple_udp_connection trap_conn;

uint8_t heartbeat_value = 0;
#if !CONTIKI_TARGET_Z1
char     global_ipv6_char[16],
         local_ipv6_char[16];
#endif
char device_hw[16];

PROCESS(snmp_main, "[SNMP] SNMPD - Agent V1");

static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen) {
  printf("Data received on port %d from port %d with length %d\n",
         receiver_port, sender_port, datalen);
}

static void init_trap(void) {
  // uip_ipaddr_t ipaddr;
  //
  // uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  // uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  // uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
  //
  // /* set server address */
  // uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 1);
  //
  // trap_conn = udp_new(&server_ipaddr, UIP_HTONS(0), NULL);
  // udp_bind(trap_conn, UIP_HTONS(162));
  static uip_ipaddr_t nms_addr;
  static uint16_t nms[] = {0xaaaa, 0, 0, 0, 0, 0, 0, 0x1};

  uip_ip6addr(&nms_addr, nms[0],
                         nms[1],
                         nms[2],
                         nms[3],
                         nms[4],
                         nms[5],
                         nms[6],
                         nms[7]);

  simple_udp_register(&trap_conn,
                      TRAP_SNMP_PORT,
                      &nms_addr,
                      TRAP_SNMP_PORT,
                      receiver);

  sprintf(device_hw,"%02X%02X%02X%02X%02X%02X%02X%02X",
          linkaddr_node_addr.u8[0],linkaddr_node_addr.u8[1],
          linkaddr_node_addr.u8[2],linkaddr_node_addr.u8[3],
          linkaddr_node_addr.u8[4],linkaddr_node_addr.u8[5],
          linkaddr_node_addr.u8[6],linkaddr_node_addr.u8[7]);
}

void cb_timer_trap_heartbeat(void *ptr){
  static uint8_t buf_trap[MAX_UDP_SNMP], len = 0;

  #ifdef DEBUG_SNMP_DECODING
  debug_snmp("Trap Heartbeat time expired!");
  #endif
  ctimer_reset(&trap_timer);

  uint8_t data_hw[16] = {device_hw[0],device_hw[1],
                        device_hw[2],device_hw[3],
                        device_hw[4],device_hw[5],
                        device_hw[6],device_hw[7],
                        device_hw[8],device_hw[9],
                        device_hw[10],device_hw[11],
                        device_hw[12],device_hw[13],
                        device_hw[14],device_hw[15]};

  len = snmp_encode_trap(buf_trap, TRAP_COLD_START, data_hw);

  // uip_ipaddr_copy(&trap_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
  // uip_udp_packet_sendto(trap_conn, &buf_trap, len,
  //                       &server_ipaddr, UIP_HTONS(TRAP_SNMP_PORT));
  // trap_addr = servreg_hack_lookup(190);

  // if(trap_addr != NULL) {
  //   debug_snmp("Sending unicast trap to ");
  //   uip_debug_ipaddr_print(trap_addr);
  uip_ipaddr_t addr;
  uip_ip6addr(&addr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0x0001);
  simple_udp_sendto(&trap_conn, buf_trap, len, &addr);
  // } else {
  //   printf("Service %d not found\n", 190);
  // }
}

void snmp_cb_data(void){
  static uint16_t len;
  static char buf[MAX_UDP_SNMP];
  memset(buf, 0, MAX_UDP_SNMP);

  if(uip_newdata()) {
    //#if !CONTIKI_TARGET_Z1
    //ctimer_stop(&trap_timer);
    //#endif

    len = uip_datalen();
    memcpy(buf, uip_appdata, len);
    #ifdef DEBUG_SNMP_DECODING
    debug_snmp("%u bytes from [", len);
    uip_debug_ipaddr_print(&UIP_IP_BUF->srcipaddr);
    printf("]:%u", UIP_HTONS(UIP_UDP_BUF->srcport));
    #endif
    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    server_conn->rport = UIP_UDP_BUF->srcport;
    snmp_t snmp_handle;
    if (snmp_decode_message(buf, &snmp_handle)){
      #ifdef DEBUG_SNMP_DECODING
      debug_snmp("New SNMP Request received!");
      #endif
      len = snmp_encode_message(&snmp_handle, buf);
      uip_udp_packet_send(server_conn, buf, len);
      uip_create_unspecified(&server_conn->ripaddr);
      server_conn->rport = 0;
    }
    else
      debug_snmp("Problem on SNMP Request received!");
  }
}

void snmp_init(void){
  process_start(&snmp_main, NULL);
}

int ipaddr_sprintf(char *buf, uint8_t buf_len, const uip_ipaddr_t *addr) {
  uint16_t a;
  uint8_t len = 0;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
        len += snprintf(&buf[len], buf_len - len, "::");
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        len += snprintf(&buf[len], buf_len - len, ":");
      }
      len += snprintf(&buf[len], buf_len - len, "%x", a);
    }
  }

  return len;
}

#if !CONTIKI_TARGET_Z1
static void print_ipv6_addr(const uip_ipaddr_t *ip_addr, char *ip_assign) {
    int i;
    for (i = 0; i < 16; i++) {
        // printf("%02x", ip_addr->u8[i]);
        *(ip_assign+i) = ip_addr->u8[i];
    }
}

void update_snmp_mib(void){
  heartbeat_value++;

  uint8_t oid_tree[2];
  char dado[MAX_STRINGS_LENGTH];
  uint8_t data_hw[4] = {device_hw[12],device_hw[13],
                        device_hw[14],device_hw[15]};

  /******************************* Type of device *****************************/
  oid_tree[0] = 25;
  oid_tree[1] = 1;
  sprintf(dado,"[%c%c%c%c]-type-%s",data_hw[0],data_hw[1],data_hw[2],data_hw[3],DEVICE_TYPE_STR);
  // debug_os("Dado de update: %s",dado);
  mib_ii_update_list(oid_tree,dado);

  /******************************** RSSI **************************************/
  oid_tree[0] = 25;
  oid_tree[1] = 2;
  int  def_rt_rssi = sicslowpan_get_last_rssi();
  sprintf(dado,"[%c%c%c%c]-RSSI-|%d",data_hw[0],data_hw[1],data_hw[2],data_hw[3],
  def_rt_rssi);
  mib_ii_update_list(oid_tree,dado);

  /*************************** Prefered IPv6 **********************************/
  char def_rt_str[64];
  oid_tree[0] = 4;
  oid_tree[1] = 21;
  memset(def_rt_str, 0, sizeof(def_rt_str));
  ipaddr_sprintf(def_rt_str, sizeof(def_rt_str), uip_ds6_defrt_choose());
  sprintf(dado,"[%c%c%c%c]-PRF-[%s]",data_hw[0],data_hw[1],data_hw[2],data_hw[3],
  def_rt_str);
  mib_ii_update_list(oid_tree,dado);

  /********************* Rank RPL e Parent Link Metric ************************/
  uint16_t rank_rpl = 0, link_metric_rpl = 0;
  rpl_parent_t *p = nbr_table_head(rpl_parents);
  rpl_instance_t *default_instance;
  default_instance = rpl_get_default_instance();
  while(p != NULL){
    if (p == default_instance->current_dag->preferred_parent) {
      rank_rpl = p->rank;
      link_metric_rpl = rpl_get_parent_link_metric(p);
      break;
    }
    else
    p = nbr_table_next(rpl_parents, p);
  }
  oid_tree[0] = 25;
  oid_tree[1] = 3;
  sprintf(dado,"[%c%c%c%c]-Rank-%5u",data_hw[0],data_hw[1],data_hw[2],data_hw[3],
  rank_rpl);
  mib_ii_update_list(oid_tree,dado);

  oid_tree[0] = 25;
  oid_tree[1] = 4;
  sprintf(dado,"[%c%c%c%c]-LM-%5u",data_hw[0],data_hw[1],data_hw[2],data_hw[3],
  link_metric_rpl);
  mib_ii_update_list(oid_tree,dado);

  /*********************** Global and Local IPv6 Address **********************/
  int i;
  uint8_t state;
  uip_ipaddr_t global_ipv6_address_node,
               local_ipv6_address_node;
  #ifdef DEBUG_SNMP_DECODING
  debug_snmp("Client IPv6 addresses: ");
  #endif
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
      (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      local_ipv6_address_node = uip_ds6_if.addr_list[i].ipaddr;
      if (i == 1)
        global_ipv6_address_node = uip_ds6_if.addr_list[i].ipaddr;
      else
        local_ipv6_address_node = uip_ds6_if.addr_list[i].ipaddr;
      #ifdef DEBUG_SNMP_DECODING
      printf6addr(&uip_ds6_if.addr_list[i].ipaddr);
      #endif
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE)
        uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
    }
  }

  print_ipv6_addr(&global_ipv6_address_node,&global_ipv6_char[0]);
  print_ipv6_addr(&local_ipv6_address_node,&local_ipv6_char[0]);

  oid_tree[0] = 4;
  oid_tree[1] = 2;
  sprintf(dado,"[%c%c%c%c]-Local-[%02x%02x::%02x%02x:%02x%02x:%02x%02x:%02x%02x]"
               ,data_hw[0],data_hw[1],data_hw[2],data_hw[3]
               ,local_ipv6_char[0]
               ,local_ipv6_char[1]
               ,local_ipv6_char[8]
               ,local_ipv6_char[9]
               ,local_ipv6_char[10]
               ,local_ipv6_char[11]
               ,local_ipv6_char[12]
               ,local_ipv6_char[13]
               ,local_ipv6_char[14]
               ,local_ipv6_char[15]);
  mib_ii_update_list(oid_tree,dado);
  oid_tree[0] = 4;
  oid_tree[1] = 20;
  sprintf(dado,"[%c%c%c%c]-Global-[%02x%02x::%02x%02x:%02x%02x:%02x%02x:%02x%02x]"
               ,data_hw[0],data_hw[1],data_hw[2],data_hw[3]
               ,global_ipv6_char[0]
               ,global_ipv6_char[1]
               ,global_ipv6_char[8]
               ,global_ipv6_char[9]
               ,global_ipv6_char[10]
               ,global_ipv6_char[11]
               ,global_ipv6_char[12]
               ,global_ipv6_char[13]
               ,global_ipv6_char[14]
               ,global_ipv6_char[15]);
  mib_ii_update_list(oid_tree,dado);

}
#endif

PROCESS_THREAD(snmp_main, ev, data){
  PROCESS_BEGIN();

  server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  udp_bind(server_conn, UIP_HTONS(DEFAULT_SNMP_PORT));
  debug_snmp("Listen port: %d, TTL=%u",DEFAULT_SNMP_PORT,server_conn->ttl);
  debug_snmp("Agent SNMPv1 active");

  init_trap();

  #if CONTIKI_TARGET_SRF06_CC26XX
  etimer_set(&update_snmp, TIME_UPDATE_SNMP);
  #endif

  // #if !CONTIKI_TARGET_Z1
  // Init the trap timer to maintain the hearbeat...
  ctimer_set(&trap_timer, TIME_TRAP_HEARTBEAT, cb_timer_trap_heartbeat, NULL);
  // #endif

  while(1){
    PROCESS_YIELD();
    if(ev == tcpip_event)
      snmp_cb_data();
    #if CONTIKI_TARGET_SRF06_CC26XX
    if (etimer_expired(&update_snmp)){
      etimer_reset(&update_snmp);
      update_snmp_mib();
      mib_ii_show();
    }
    #endif
  }
  PROCESS_END();
}

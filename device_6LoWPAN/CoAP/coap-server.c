/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "contiki-net.h"
#include "dev/leds.h"
#include "rest-engine.h"
// #include "board-peripherals.h"
// #include "rf-core/rf-ble.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "homestark.h"

/*---------------------------------------------------------------------------*/
/* Common resources */
extern resource_t res_hello,
                  res_switch,
                  res_light,
                  res_water;

/*---------------------------------------------------------------------------*/
const char *coap_server_not_found_msg = "Resource not found";
const char *coap_server_supported_msg = "Supported:"
                                        "text/plain,"
                                        "application/json,"
                                        "application/xml";
/*---------------------------------------------------------------------------*/
static void
start_board_resources(void) {
  rest_activate_resource(&res_hello, "test/hello");
  #ifdef SWITCH_DEVICE
  rest_activate_resource(&res_switch, "switch");
  #endif

  #ifdef LIGHT_DEVICE
  rest_activate_resource(&res_light, "light");
  #endif

  #ifdef WATER_DEVICE
  rest_activate_resource(&res_water, "water");
  #endif
}
/*---------------------------------------------------------------------------*/
PROCESS(coap_server_process, "CoAP Server");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(coap_server_process, ev, data) {
  PROCESS_BEGIN();

  debug_coap("Server CoAP\n");

  leds_init();

  rest_init_engine();

  start_board_resources();

  while(1) {
    PROCESS_WAIT_EVENT();
  }

  PROCESS_END();
}

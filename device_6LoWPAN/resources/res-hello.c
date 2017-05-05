#include "contiki.h"
#include "rest-engine.h"
#include "dev/leds.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if CONTIKI_TARGET_SRF06_CC26XX
#include "board-peripherals.h"
#endif

static void
res_get_handler(void *request, void *response, uint8_t *buffer,
                uint16_t preferred_size, int32_t *offset) {
  const char *len = NULL;
  /* Some data that has the length up to REST_MAX_CHUNK_SIZE. For more, see the chunk resource. */
  char const *const message = "Hello World! ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxy";
  int length = 12;

  /* The query string can be retrieved by rest_get_query() or parsed for its key-value pairs. */
  if(REST.get_query_variable(request, "cc", &len)) {


  }
  #if CONTIKI_TARGET_SRF06_CC26XX
    leds_toggle(LEDS_ALL);
    //leds_toggle((unsigned char)BOARD_LED_1);
    // leds_toggle(BOARD_LED_2);
  #else
    leds_toggle(LEDS_ALL);
  #endif
  memcpy(buffer, message, length);
   /* text/plain is the default, hence this option could be omitted. */
  REST.set_header_content_type(response, REST.type.TEXT_PLAIN);

  REST.set_header_etag(response, (uint8_t *)&length, 1);
  REST.set_response_payload(response, buffer, length);
}

RESOURCE(res_hello,
         "title=\"Hello world: ?cc=0..\";rt=\"Text\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

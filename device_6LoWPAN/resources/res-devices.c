#include "contiki.h"
#include "rest-engine.h"
#include "dev/leds.h"
#include "coap-server.h"
#include "homestark.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#if CONTIKI_TARGET_SRF06_CC26XX
#include "board-peripherals.h"
#endif
#include "sha256.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
//ADC Libs
#include "ti-lib.h"
#include "driverlib/aux_adc.h"
#include "driverlib/aux_wuc.h"

uint8_t switch_demo = 0,
        dimmer_value = 50;
uint16_t light_status = 500,
         water_level = 50;

uint16_t readADC(void ){
  uint16_t singleSample;

  ti_lib_aon_wuc_aux_wakeup_event(AONWUC_AUX_WAKEUP);
  while(!(ti_lib_aon_wuc_power_status_get() & AONWUC_AUX_POWER_ON))
  { }

  // Enable clock for ADC digital and analog interface (not currently enabled in driver)
  // Enable clocks
  ti_lib_aux_wuc_clock_enable(AUX_WUC_ADI_CLOCK | AUX_WUC_ANAIF_CLOCK | AUX_WUC_SMPH_CLOCK);
  while(ti_lib_aux_wuc_clock_status(AUX_WUC_ADI_CLOCK | AUX_WUC_ANAIF_CLOCK | AUX_WUC_SMPH_CLOCK) != AUX_WUC_CLOCK_READY)
  { }

  // printf("clock selected\r\n");

  // Connect AUX IO7 (DIO23, but also DP2 on XDS110) as analog input.
  AUXADCSelectInput(ADC_COMPB_IN_AUXIO7);
  // printf("input selected\r\n");

  // Set up ADC range
  // AUXADC_REF_FIXED = nominally 4.3 V
  AUXADCEnableSync(AUXADC_REF_FIXED,  AUXADC_SAMPLE_TIME_2P7_US, AUXADC_TRIGGER_MANUAL);
  // printf("init adc --- OK\r\n");

  //Trigger ADC converting
  AUXADCGenManualTrigger();
  // printf("trigger --- OK\r\n");

  //reading adc value
  singleSample = AUXADCReadFifo();

  // printf("%d mv on ADC\r\n",singleSample);

  //shut the adc down
  AUXADCDisable();
  // printf("disable --- OK\r\n");
  return singleSample;
}

void generateVerf(char *message, char *readySend){
  /************************* SHA-256 - Hash ***********************************/
  unsigned char buffer[50],
               hash[32];
  int idx;
  SHA256_CTX ctx;

  sha256_init(&ctx);
  sprintf(buffer,"%02X%02X&%s",linkaddr_node_addr.u8[6],linkaddr_node_addr.u8[7],message);
  sha256_update(&ctx,buffer,strlen((const char *)buffer));
  sha256_final(&ctx,hash);
  sprintf(readySend,"%x%x",hash[0],hash[31],message);
  /************************* SHA-256 - Hash ***********************************/
}

void generateHash(char *message, char *readySend){
  /************************* SHA-256 - Hash ***********************************/
  unsigned char buffer[50],
                hash[32];
  int idx;
  SHA256_CTX ctx;

  sha256_init(&ctx);

  // debug_os("Buffer a encriptar:%s",buffer);
  sprintf(buffer,"%02X%02X&%s",linkaddr_node_addr.u8[6],linkaddr_node_addr.u8[7],message);
  sha256_update(&ctx,buffer,strlen((const char *)buffer));
  sha256_final(&ctx,hash);

  // debug_os("Hash[data]: ");
  // for (idx=0; idx < 32; idx++)
  //    printf("%02x",hash[idx]);
  // printf("\n");

  sprintf(readySend,"h=%x%x&%s",hash[0],hash[31],message);
  // debug_os("Mensagem encriptada:%s",buffer);
  // readySend = &buffer;
  /************************* SHA-256 - Hash ***********************************/
}

/*********************************** SWITCH ***********************************/
static void
res_get_handler_switch(void *request, void *response, uint8_t *buffer,
                uint16_t preferred_size, int32_t *offset) {
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);

  char message[100],
       readySend[50];
  sprintf(message,"switch=%d",switch_demo);
  generateHash(&message,&readySend);
  #ifdef ENABLE_HASH_MESSAGES
  debug_os("Mensagem com hash a enviar:%s",readySend);
  #endif

  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%s", readySend);

    // REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
  // } else if(accept == REST.type.APPLICATION_JSON) {
  //   REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
  //   snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{\"SWITCH\":\"%d\"}",
  //            switch_demo);
  //
  //   REST.set_response_payload(response, buffer, strlen((char *)buffer));
  // } else if(accept == REST.type.APPLICATION_XML) {
  //   REST.set_header_content_type(response, REST.type.APPLICATION_XML);
  //   snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,
  //            "<switch val=\"%d\"/>", switch_demo);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    REST.set_response_payload(response, coap_server_supported_msg,
                              strlen(coap_server_supported_msg));
  }

}

static void
res_post_handler_switch(void *request, void *response, uint8_t *buffer,
                uint16_t preferred_size, int32_t *offset) {
  size_t len = 0,
         len2 = 0;
  const char *text = NULL,
             *text2 = NULL;
  char switch_value[10],
       hash_value[10];
  uint8_t  data_switch, verify = 0;

  memset(switch_value, 0, 10);
  memset(hash_value, 0, 10);

  len = REST.get_post_variable(request, "switch_value", &text);
  len2 = REST.get_post_variable(request, "h", &text2);

  if(len > 0 && len < 10 && len2 > 0 && len2 < 10) {
    memcpy(switch_value, text, len);
    memcpy(hash_value, text2, len2);
  }
  data_switch = atoi(switch_value);

  /************************* Verificando hash *********************************/
  char message[100],
       localHash[50],
       receivedHash[10];
  sprintf(message,"switch_value=%d",data_switch);
  generateVerf(&message,&localHash);

  sprintf(receivedHash,"%c%c%c%c",hash_value[0],hash_value[1],hash_value[2],hash_value[3]);
  #ifdef ENABLE_HASH_MESSAGES
  debug_os("Recebido: %s",receivedHash);
  debug_os("Gerado: %s",localHash);
  debug_os("Comparacao: %d",strstr(localHash,receivedHash));
  #endif
  if (strstr(localHash,receivedHash) != NULL)
    verify = 1;
  /************************* Verificando hash *********************************/

  if (verify){
    #ifdef ENABLE_HASH_MESSAGES
    debug_os("Mensagem integra!");
    #endif
    switch_demo = data_switch;
    if (switch_demo == 1)
      leds_on(LEDS_ALL);
    else
      leds_off(LEDS_ALL);
  }
  #ifdef ENABLE_HASH_MESSAGES
  else
    debug_os("Mensagem nao integra!");
  #endif
}
/*********************************** SWITCH ***********************************/

/*********************************** LIGHT ************************************/
static void
res_get_handler_light(void *request, void *response, uint8_t *buffer,
                uint16_t preferred_size, int32_t *offset) {
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);

  char message[100],
       readySend[50];

  uint16_t adcSample = readADC();
  // 2000lux/3600mV = 0.55lux/mV
  adcSample = adcSample*0.555;
  light_status = adcSample;
  sprintf(message,"light=%d",light_status);
  generateHash(&message,&readySend);
  #ifdef ENABLE_HASH_MESSAGES
  debug_os("Mensagem com hash a enviar:%s",readySend);
  #endif
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%s", readySend);
  //
  //   REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
  // } else if(accept == REST.type.APPLICATION_JSON) {
  //   REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
  //   snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{\"LIGHT\":\"%d\"}",
  //            light_status);

  //   REST.set_response_payload(response, buffer, strlen((char *)buffer));
  // } else if(accept == REST.type.APPLICATION_XML) {
  //   REST.set_header_content_type(response, REST.type.APPLICATION_XML);
  //   snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,
  //            "<light val=\"%d\"/>", light_status);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    REST.set_response_payload(response, coap_server_supported_msg,
                              strlen(coap_server_supported_msg));
  }

}

static void
res_post_handler_light(void *request, void *response, uint8_t *buffer,
                uint16_t preferred_size, int32_t *offset) {
  size_t len = 0,
         len2 = 0;
  const char *text = NULL,
             *text2 = NULL;
  char dimmer[10],
       hash_value[10];
  uint8_t  data_dimmer, verify = 0;

  memset(dimmer, 0, 10);
  memset(hash_value, 0, 10);

  len = REST.get_post_variable(request, "dimmer_value", &text);
  len2 = REST.get_post_variable(request, "h", &text2);

  if(len > 0 && len < 10 && len2 > 0 && len2 < 10) {
    memcpy(dimmer, text, len);
    memcpy(hash_value, text2, len2);
  }
  data_dimmer = atoi(dimmer);

  /************************* Verificando hash *********************************/
  char message[100],
       localHash[50],
       receivedHash[10];
  sprintf(message,"dimmer_value=%d",data_dimmer);
  generateVerf(&message,&localHash);

  sprintf(receivedHash,"%c%c%c%c",hash_value[0],hash_value[1],hash_value[2],hash_value[3]);
  #ifdef ENABLE_HASH_MESSAGES
  debug_os("Recebido: %s",receivedHash);
  debug_os("Gerado: %s",localHash);
  debug_os("Comparacao: %d",strstr(localHash,receivedHash));
  #endif
  if (strstr(localHash,receivedHash) != NULL)
    verify = 1;
  /************************* Verificando hash *********************************/

  if (verify) {
    #ifdef ENABLE_HASH_MESSAGES
    debug_os("Mensagem integra");
    #endif
    dimmer_value = atoi(dimmer);
  }
  #ifdef ENABLE_HASH_MESSAGES
  else
    debug_os("Mensagem nao integra");
  #endif
}
/*********************************** LIGHT ************************************/

/*********************************** WATER ************************************/
static void
res_get_handler_water(void *request, void *response, uint8_t *buffer,
                uint16_t preferred_size, int32_t *offset) {
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);

  char message[100],
       readySend[50];

  uint16_t adcSample = readADC();
  // 100%/3600mV = 0.0277%/mV
  adcSample = adcSample*0.0277;
  water_level = adcSample;
  
  sprintf(message,"water=%d",water_level);
  generateHash(&message,&readySend);
  #ifdef ENABLE_HASH_MESSAGES
  debug_os("Mensagem com hash a enviar:%s",readySend);
  #endif
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%s", readySend);
  //
  //   REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
  // } else if(accept == REST.type.APPLICATION_JSON) {
  //   REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
  //   snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{\"WATER\":\"%d\"}",
  //            water_level);
  //
  //   REST.set_response_payload(response, buffer, strlen((char *)buffer));
  // } else if(accept == REST.type.APPLICATION_XML) {
  //   REST.set_header_content_type(response, REST.type.APPLICATION_XML);
  //   snprintf((char *)buffer, REST_MAX_CHUNK_SIZE,
  //            "<light val=\"%d\"/>", water_level);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    REST.set_response_payload(response, coap_server_supported_msg,
                              strlen(coap_server_supported_msg));
  }

}

/*********************************** WATER ************************************/

RESOURCE(res_switch,
         "title=\"Switch: GET or POST\";rt=\"Text\"",
         res_get_handler_switch,
         res_post_handler_switch,
         NULL,
         NULL);

RESOURCE(res_light,
         "title=\"Light: GET or POST\";rt=\"Text\"",
         res_get_handler_light,
         res_post_handler_light,
         NULL,
         NULL);

RESOURCE(res_water,
         "title=\"Water: GET or POST\";rt=\"Text\"",
         res_get_handler_water,
         NULL,
         NULL,
         NULL);

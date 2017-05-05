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
 * @license This project is under APACHE 2.0 license.
 * @file snmp_asn1.c
 * @brief Encoding and decoding functions to SNMP agent
 * @author Ânderson Ignácio da Silva
 * @date 19 Sept 2016
 * @see http://www.aignacio.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#if CONTIKI_TARGET_SRF06_CC26XX
#include <math.h>
#endif
#include "snmp.h"
#include "mibii.h"

#if CONTIKI_TARGET_Z1
int pow(int base, int exp){
  if(exp < 0)
    return -1;

    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}
#endif

resp_con_t error_check_snmp(uint8_t *error_data){
    uint32_t error_status;

    decode_asn1_integer(error_data,&error_status);
    if (error_status != ERROR_NONE) {
      debug_snmp("There's some error STATUS in packet");
      return FAIL_CON;
    }
    decode_asn1_integer(error_data+3,&error_status);
    if (error_status != ERROR_NONE) {
      debug_snmp("There's some error INDEX in packet");
      return FAIL_CON;
    }
    return SUCCESS_CON;
}

resp_con_t decode_asn1_oct_str(uint8_t *data_encoded, uint8_t *oct_str){
  if (*data_encoded !=  (intptr_t)ASN1_PRIM_OCT_STR) {
    debug_snmp("The type of value passed is not an octet string!");
    return FAIL_CON;
  }

  uint8_t length = (intptr_t)*(data_encoded+1),
          index = 0;
  while (length) {
    *(oct_str+index) = (intptr_t)*(data_encoded+2+index);
    length--;
    index++;
  }

  *(oct_str+index) = '\0';
  // printf("\nEndereco----:> %d\n",oct_str);
  return SUCCESS_CON;
}

resp_con_t decode_asn1_integer(uint8_t *data_encoded, uint32_t *integer_value){
  uint8_t length = (intptr_t)*(data_encoded+1);
  // uint32_t integer_value;
  size_t i = 0;
  uint32_t aux;

  // Test if it's an integer value to be decoded
  if (*data_encoded != ASN1_PRIM_INTEGER){
    debug_snmp("The value is not integer!");
    return FAIL_CON;
  }

  for (i=1, *integer_value = 0; i <= length; i++){
    aux = *(data_encoded+1+i);
    *integer_value += aux*(pow(256,(length-i)));
    // debug_snmp("%lu * 256^%d = %lu",aux,(length-i),*integer_value);
  }
  return SUCCESS_CON;
}

resp_con_t snmp_decode_message(char *snmp_packet, snmp_t *snmp_handle){
  uint8_t buffer[50], aux;
  size_t i;

  #ifdef DEBUG_SNMP_DECODING
  // debug_snmp("Encoded SNMP packet:\n\t");
  // for (i=0, j=0; i < *(snmp_packet+1)+1; j++, i++){
  //   if (j > 7){
  //     j = 0;
  //     printf("\n\t");
  //   }
  //   printf("[%02x] ",*(snmp_packet+i));
  // }
  #endif

  if (!check_seq(*snmp_packet)){
    debug_snmp("Sequence initial of SNMP message error:%x",*snmp_packet);
    return FAIL_CON;
  }

  /************************ Check the SNMP version ****************************/
  for (i=0;i < *(snmp_packet+3)+2; i++)
    buffer[i] = *(snmp_packet+2+i);
  uint32_t SNMPv = 0;
  if (!decode_asn1_integer(buffer,&SNMPv)) return FAIL_CON;
  #ifdef DEBUG_SNMP_DECODING
  debug_snmp("Version SNMP:[1] OK");
  #endif
  if (SNMPv != SNMP_VERSION_1) {
    debug_snmp("SNMP version is different from v1:%lu",SNMPv);
    return FAIL_CON;
  }
  snmp_handle->snmp_version = SNMPv;

  /********************** Get the community string ****************************/
  for (i=0;i < *(snmp_packet+6)+2; i++)
  snmp_handle->community[i] = *(snmp_packet+5+i);
  snmp_handle->community[i] = '\0';
  aux = i;
  #ifdef DEBUG_SNMP_DECODING
  debug_snmp("Community String: ");
  for (i=0; i < aux; i++){
    if (i<2)
      printf("[%d]",snmp_handle->community[i]);
    else
      printf("[%c]",snmp_handle->community[i]);
  }
  #endif

  /************************** Get the request ID ******************************/
  aux = 5+snmp_handle->community[1]+2+2;
  for (i=0;i < *(snmp_packet+aux+1)+2; i++)
    snmp_handle->request_id_c[i] = *(snmp_packet+aux+i);
  snmp_handle->request_id_c[i] = '\0';
  aux = i;
  #ifdef DEBUG_SNMP_DECODING
  debug_snmp("Request ID: ");
  for (i=0; i < aux; i++){
    if (i<2)
      printf("[%d]",snmp_handle->request_id_c[i]);
    else
    printf("[%x]",snmp_handle->request_id_c[i]);
  }
  #endif

  /************************** Check for errors ********************************/
  aux = 5+(snmp_handle->community[1]+2)+2+(snmp_handle->request_id_c[1]+2);
  for (i=0;i < 6; i++)
    buffer[i] = *(snmp_packet+aux+i);
  buffer[i] = '\0';
  error_check_snmp(buffer);

  /**************************** Get the OID ***********************************/
  aux = 5+(snmp_handle->community[1]+2);
  aux += 2+(snmp_handle->request_id_c[1]+2)+10;
  for (i=0;i < *(snmp_packet+aux+1)+2; i++)
    snmp_handle->oid_encoded[i] = *(snmp_packet+aux+i);
  snmp_handle->oid_encoded[i] = '\0';
  aux = i;
  #ifdef DEBUG_SNMP_DECODING
  debug_snmp("OID: ");
  for (i=0; i < aux; i++){
    if (i <= 1)
      printf("[%d]",snmp_handle->oid_encoded[i]);
    else if (i == 2)
      printf("[%d.",snmp_handle->oid_encoded[i]);
    else
    printf("%d.",snmp_handle->oid_encoded[i]);
  }
  printf("]");
  #endif

  /************************** Get the PDU type ********************************/
  aux = 5+(snmp_handle->community[1]+2);
  snmp_handle->request_type  = *(snmp_packet+aux);
  snmp_handle->response_type = ASN1_CPX_GET_RESP;

  uint8_t string_value[MAX_OCTET_STRING];
  uint8_t status_mib2 = mib_ii_get_oid(snmp_handle->oid_encoded,&string_value[0]);

  switch (snmp_handle->request_type) {
    case ASN1_CPX_SEQUENCE:
    break;
    case ASN1_CPX_GET_REQ:
      aux = snmp_handle->oid_encoded[1]+1;
      if (snmp_handle->oid_encoded[aux] != 0   ||
          snmp_handle->oid_encoded[aux-3] != 1 ||
          snmp_handle->oid_encoded[aux-4] != 2 ||
          snmp_handle->oid_encoded[aux-5] != 1 ||
          snmp_handle->oid_encoded[aux-6] != 6 ||
          snmp_handle->oid_encoded[aux-7] != 0x2b){
        snmp_handle->oid_encoded[aux] = 1;
        status_mib2 = mib_ii_get_oid(snmp_handle->oid_encoded,&string_value[0]);
      }
      #ifdef DEBUG_SNMP_DECODING
      debug_snmp("GET Request PDU Type");
      #endif
      if (!status_mib2){
        #ifdef DEBUG_SNMP_DECODING
        debug_snmp("There isn't an value for that OID!");
        #endif
        snmp_handle->value[0] = 0x05;
        snmp_handle->value[1] = 0x00;
      }
      else {
        aux = strlen((const char*)string_value);
        snmp_handle->value[0] = ASN1_PRIM_OCT_STR;
        snmp_handle->value[1] = aux;

        for (i = 0; i < aux; i++)
          snmp_handle->value[2+i] = string_value[i];
        #ifdef DEBUG_SNMP_DECODING
        debug_snmp("String for OID: ");
        for (i=0; i < aux+2; i++){
          if (i == 0)
            printf("[%x]",snmp_handle->value[i]);
          else if (i == 1)
            printf("[%d][",snmp_handle->value[i]);
          else
            printf("%c",snmp_handle->value[i]);
        }
        printf("]");
        #endif
      }
    break;
    case ASN1_CPX_NEXT_REQ:
      // Let's check the last byte
      aux = snmp_handle->oid_encoded[1]+1;
      if (snmp_handle->oid_encoded[aux] == 0) {
        // We need to increment the OID for the snmpwalk... requisition
        if (snmp_handle->oid_encoded[aux-1] < 9) {
          snmp_handle->oid_encoded[aux-1] = snmp_handle->oid_encoded[aux-1]+1;
        }
        else
          snmp_handle->oid_encoded[aux] = 1; // Let's force not unknow value in the mib tree
        status_mib2 = mib_ii_get_oid(snmp_handle->oid_encoded,&string_value[0]);
      }
      else{
        if (snmp_handle->oid_encoded[aux-1] == 1 &&
            snmp_handle->oid_encoded[aux-2] == 2 &&
            snmp_handle->oid_encoded[aux-3] == 1 &&
            snmp_handle->oid_encoded[aux-4] == 6 &&
            snmp_handle->oid_encoded[aux-5] == 0x2b) {
          snmp_handle->oid_encoded[1] += 2;
          snmp_handle->oid_encoded[aux+1] = 1;
          snmp_handle->oid_encoded[aux+2] = 0;
          snmp_handle->oid_encoded[aux+3] = '\0';
        }
        // We need to set to the nearest OID for the snmpwalk... requisition, in this case .1.0
        status_mib2 = mib_ii_get_oid(snmp_handle->oid_encoded,&string_value[0]);
      }

      #ifdef DEBUG_SNMP_DECODING
      debug_snmp("GET NEXT Request PDU Type");
      #endif
      if (!status_mib2){
        #ifdef DEBUG_SNMP_DECODING
        debug_snmp("There isn't an value for that OID!");
        #endif
        snmp_handle->value[0] = 0x05;
        snmp_handle->value[1] = 0x00;
      }
      else {
        aux = strlen((const char*)string_value);
        snmp_handle->value[0] = ASN1_PRIM_OCT_STR;
        snmp_handle->value[1] = aux;

        for (i = 0; i < aux; i++)
          snmp_handle->value[2+i] = string_value[i];
        #ifdef DEBUG_SNMP_DECODING
        debug_snmp("String for OID: ");
        for (i=0; i < aux+2; i++){
          if (i == 0)
            printf("[%x]",snmp_handle->value[i]);
          else if (i == 1)
            printf("[%d][",snmp_handle->value[i]);
          else
            printf("%c",snmp_handle->value[i]);
        }
        printf("]");
        #endif
      }
    break;
    case ASN1_CPX_GET_RESP:
    break;
    case ASN1_CPX_SET_REQ:
    break;
    default:
      #ifdef DEBUG_SNMP_DECODING
      debug_snmp("The PDU type is not know");
      #endif
      return FAIL_CON;
    break;
  }

  #ifdef DEBUG_SNMP_DECODING
  printf("\n");
  #endif
  return SUCCESS_CON;
}

uint16_t snmp_encode_message(snmp_t *snmp_handle, char *data_encoded){
  uint8_t i, aux = 0, aux2 = 0;
  *data_encoded = ASN1_CPX_SEQUENCE;

  aux2 = 0;
  aux2 += 3+(snmp_handle->community[1]+2)+12;
  aux2 += (snmp_handle->request_id_c[1]+2);
  aux2 += (snmp_handle->oid_encoded[1]+2);
  aux2 += (snmp_handle->value[1]+2);
  *(data_encoded+1) = aux2;

  *(data_encoded+2) = ASN1_PRIM_INTEGER;
  *(data_encoded+3) = 0x01;
  switch (snmp_handle->snmp_version) {
    case SNMP_VERSION_1:
      *(data_encoded+4) = SNMP_VERSION_1;
    break;
    case SNMP_VERSION_2C:
      *(data_encoded+4) = SNMP_VERSION_2C;
    break;
    case SNMP_VERSION_3:
      *(data_encoded+4) = SNMP_VERSION_3;
    break;
    default:
      debug_snmp("Version SNMP not supported");
      return FAIL_CON;
    break;
  }

  for ( i = 0; i < snmp_handle->community[1]+2; i++)
    *(data_encoded+5+i) = snmp_handle->community[i];

  aux = 5+snmp_handle->community[1]+2;
  *(data_encoded+aux) = ASN1_CPX_GET_RESP;

  aux2 = 0;
  aux2 += (snmp_handle->request_id_c[1]+2)+10;
  aux2 += (snmp_handle->oid_encoded[1]+2);
  aux2 += (snmp_handle->value[1]+2);
  *(data_encoded+aux+1) = aux2;

  aux += 2;
  for ( i = 0; i < snmp_handle->request_id_c[1]+2; i++)
    *(data_encoded+aux+i) = snmp_handle->request_id_c[i];

  aux += snmp_handle->request_id_c[1]+2;

  if (snmp_handle->value[0] == ASN1_PRIM_NULL) {
    *(data_encoded+aux) = ASN1_PRIM_INTEGER;
    aux++;
    *(data_encoded+aux) = 0x01;
    aux++;
    *(data_encoded+aux) = ERROR_REQ_OID_NOT_FOUND;
    aux++;
    *(data_encoded+aux) = ASN1_PRIM_INTEGER;
    aux++;
    *(data_encoded+aux) = 0x01;
    aux++;
    *(data_encoded+aux) = ERROR_RESP_TOO_LARGE;
    aux++;
    *(data_encoded+aux) = ASN1_CPX_SEQUENCE;
    aux++;
    aux2 = 2;
    aux2 += (snmp_handle->oid_encoded[1]+2);
    aux2 += (snmp_handle->value[1]+2);
    *(data_encoded+aux) = aux2;
    aux++;
    *(data_encoded+aux) = ASN1_CPX_SEQUENCE;
    aux++;
    aux2 = 0;
    aux2 += (snmp_handle->oid_encoded[1]+2);
    aux2 += (snmp_handle->value[1]+2);
    *(data_encoded+aux) = aux2;
    aux++;
    for ( i = 0; i < snmp_handle->oid_encoded[1]+2; i++)
      *(data_encoded+aux+i) = snmp_handle->oid_encoded[i];
    aux += snmp_handle->oid_encoded[1]+2;
    *(data_encoded+aux) = ASN1_PRIM_NULL;
    aux++;
    *(data_encoded+aux) = 0x00;
  }
  else{
    *(data_encoded+aux) = ASN1_PRIM_INTEGER;
    aux++;
    *(data_encoded+aux) = 0x01;
    aux++;
    *(data_encoded+aux) = ERROR_NONE;
    aux++;
    *(data_encoded+aux) = ASN1_PRIM_INTEGER;
    aux++;
    *(data_encoded+aux) = 0x01;
    aux++;
    *(data_encoded+aux) = ERROR_NONE;
    aux++;
    *(data_encoded+aux) = ASN1_CPX_SEQUENCE;
    aux++;
    aux2 = 2;
    aux2 += (snmp_handle->oid_encoded[1]+2);
    aux2 += (snmp_handle->value[1]+2);
    *(data_encoded+aux) = aux2;
    aux++;
    *(data_encoded+aux) = ASN1_CPX_SEQUENCE;
    aux++;
    aux2 = 0;
    aux2 += (snmp_handle->oid_encoded[1]+2);
    aux2 += (snmp_handle->value[1]+2);
    *(data_encoded+aux) = aux2;
    aux++;
    for ( i = 0; i < snmp_handle->oid_encoded[1]+2; i++)
      *(data_encoded+aux+i) = snmp_handle->oid_encoded[i];
    aux += snmp_handle->oid_encoded[1]+2;
    for ( i = 0; i < snmp_handle->value[1]+2; i++)
      *(data_encoded+aux+i) = snmp_handle->value[i];
  }
  #ifdef DEBUG_SNMP_DECODING
  debug_snmp("Len of encoded packet: %d",*(data_encoded+1)+1);
  #endif
  return *(data_encoded+1)+2;
}

// Trap PDU EXAMPLE
// 0x30, 0x38, // Sequence type
// 0x02, 0x01, 0x00, // SNMP Version
// 0x04, 0x06, 0x70, 0x75, 0x62, 0x6c, 0x69, 0x63, // Community String
// 0xa4, 0x2b, // Type of PDU
// 0x06, 0x09, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x04, 0x01, 0x02, 0x15, // Enterprise
// 0x40, 0x04, 0x7f, 0x00, 0x00, 0x01, // IP address of the Agent 127.0.0.1
// 0x02, 0x01, 0x00, // Generic Trap Code - 0 Cold Start
// 0x02, 0x01, 0x00, // Specific code trap
// 0x43, 0x01, 0x00, // Timestamp
// 0x30, 0x0f, // Ver bind list
// 0x30, 0x0d, // Var bind vars
// 0x06, 0x08, 0x2b, 0x06, 0x01, 0x02, 0x01, 0x02, 0x01, 0x00, //oid
// 0x02, 0x01, 0x21
// +------------------------------------------------------------------------------------------------------------------------------------------+
// |                                                         TRAP SNMPv1 PDU                                                                  |
// +----+------+------------------------------------------------------------------------------------------------------------------------------+
// |PDU |Length|                                             Data of SNMP PDU                                                                 |
// |type| Data |                                                                                                                              |
// +----+------+----------+-------+-------+--------+------+---------------------------------------------------- ------------------------------+
// |0xA4|Length|Enterprise| Agent |Generic|Specific|Time  |                                VarBind List (Sequence)                            |
// |    | PDU  |   OID    |Address| Trap  |  Trap  |Stamp +------+--------+------------------------------+------------------------------+-----+
// |    |      |          |       | Type  | Number |      | 0x30 | Length |           Varbind 1          |         Varbind 22           | ... |
// |    |      |  (OID)   |       |       |        |      |      |        |          (Sequence)          |         (Sequence)           | ... |
// |    |      |          |       |       |        |      |      |        +----+-------+-------+---------+----+-------+-------+---------+-----+
// |    |      |          |       |       |        |      |      |        |0x30| Len 1 | OID 1 | Value 1 |0x30| Len 2 | OID 1 | Value 2 | ... |
// +----+------+----------+-------+-------+--------+------+------+--------+----+-------+-------+---------+----+-------+-------+---------+-----+
//                        |                                               |            |<-----Len 1----->|            |<-----Len 2----->|     |
//                        |                                               |<-----------------------------Length------------------------------>|
//                        |<--------------------------------------------Lenght PDU----------------------------------------------------------->|

uint16_t snmp_encode_trap(uint8_t *trap_pdu, uint8_t type_trap, uint8_t *device_hw){
  // uint8_t i;//, aux = 0, aux2 = 0;
  uint16_t length_trap = 0, aux = 0;

  *trap_pdu = ASN1_CPX_SEQUENCE;
  *(trap_pdu+1) = 55+1+2+12;

  // SNMP Version
  *(trap_pdu+2) = ASN1_PRIM_INTEGER;
  *(trap_pdu+3) = 0x01;
  *(trap_pdu+4) = SNMP_VERSION_1;

  // Comunity String - always "public"
  *(trap_pdu+5) = ASN1_PRIM_OCT_STR;
  *(trap_pdu+6) = 0x06;
  *(trap_pdu+7) = 0x70;
  *(trap_pdu+8) = 0x75;
  *(trap_pdu+9) = 0x62;
  *(trap_pdu+10) = 0x6c;
  *(trap_pdu+11) = 0x69;
  *(trap_pdu+12) = 0x63;

  // Type of PDU - Trap(0xa4)
  *(trap_pdu+13) = ASN1_CPX_TRAP;
  aux = 14;
  *(trap_pdu+aux) = 42+1+2+12;

  // Enterprise OID - 0x06, 0x09, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x04, 0x01, 0x02, 0x15
  aux++;
  *(trap_pdu+aux) = ASN1_PRIM_OID;
  // aux++;
  // *(trap_pdu+aux) = 0x09;
  // aux++;
  // *(trap_pdu+aux) = 0x2b;
  // aux++;
  // *(trap_pdu+aux) = 0x06;
  // aux++;
  // *(trap_pdu+aux) = 0x01;
  // aux++;
  // *(trap_pdu+aux) = 0x04;
  // aux++;
  // *(trap_pdu+aux) = 0x01;
  // aux++;
  // *(trap_pdu+aux) = 0x04;
  // aux++;
  // *(trap_pdu+aux) = 0x01;
  // aux++;
  // *(trap_pdu+aux) = 0x02;
  // aux++;
  // *(trap_pdu+aux) = 0x15;

  aux++;
  *(trap_pdu+aux) = 0x08;
  aux++;
  *(trap_pdu+aux) = 0x2b;
  aux++;
  *(trap_pdu+aux) = 0x06;
  aux++;
  *(trap_pdu+aux) = 0x01;
  aux++;
  *(trap_pdu+aux) = 0x04;
  aux++;
  *(trap_pdu+aux) = 0x01;
  aux++;
  *(trap_pdu+aux) = 0x82;
  aux++;
  *(trap_pdu+aux) = 0xFB;
  aux++;
  *(trap_pdu+aux) = 0x68;

  // IP Address of the agent, always 0.0.0.0 if we cannot send IPv6 in SNMPv1, in SNMPv2 the trap calls inform
  aux++;
  *(trap_pdu+aux) = ASN1_PRIM_IP_ADDRESS;
  aux++;
  *(trap_pdu+aux) = 4;
  aux++;
  *(trap_pdu+aux) = 0;
  aux++;
  *(trap_pdu+aux) = 0;
  aux++;
  *(trap_pdu+aux) = 0;
  aux++;
  *(trap_pdu+aux) = 0;

  // Generic Trap type
  aux++;
  *(trap_pdu+aux) = ASN1_PRIM_INTEGER;
  aux++;
  *(trap_pdu+aux) = 0x01;
  aux++;
  *(trap_pdu+aux) = type_trap;

  // Specific Trap Number - we don't use this
  aux++;
  *(trap_pdu+aux) = ASN1_PRIM_INTEGER;
  aux++;
  *(trap_pdu+aux) = 0x01;
  aux++;
  *(trap_pdu+aux) = 0x00;

  // Timestamp - we don't use this - default(0)
  aux++;
  *(trap_pdu+aux) = ASN1_PRIM_TIMESTAMP;
  aux++;
  *(trap_pdu+aux) = 0x01;
  aux++;
  *(trap_pdu+aux) = 0x00;

  // VarBind List - we don't use this - default(0)
  aux++;
  *(trap_pdu+aux) = ASN1_CPX_SEQUENCE;
  aux++;
  *(trap_pdu+aux) = 3+8+2+2+1+2+12;

  // VarBind List - we don't use this - default(0)
  aux++;
  *(trap_pdu+aux) = ASN1_CPX_SEQUENCE;
  aux++;
  *(trap_pdu+aux) = 3+8+2+1+2+12;

  // OID
  aux++;
  *(trap_pdu+aux) = ASN1_PRIM_OID;
  aux++;
  *(trap_pdu+aux) = 0x08;
  aux++;
  *(trap_pdu+aux) = 0x2b;
  aux++;
  *(trap_pdu+aux) = 0x06;
  aux++;
  *(trap_pdu+aux) = 0x01;
  aux++;
  *(trap_pdu+aux) = 0x02;
  aux++;
  *(trap_pdu+aux) = 0x01;
  aux++;
  *(trap_pdu+aux) = 0x02;
  aux++;
  *(trap_pdu+aux) = 0x01;
  aux++;
  *(trap_pdu+aux) = 0x00;

  // Value - Heartbeat
  aux++;
  *(trap_pdu+aux) = ASN1_PRIM_OCT_STR;
  aux++;
  *(trap_pdu+aux) = 0x10;

  size_t j;
  for (j=0; j < 16; j++) {
    aux++;
    *(trap_pdu+aux) = *(device_hw+j);
  }


  length_trap = 60+12;
  return length_trap;
}

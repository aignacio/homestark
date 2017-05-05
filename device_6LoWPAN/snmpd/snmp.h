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
 * @file snmp.h
 * @brief Header functions about SNMP implementation
 * @author Ânderson Ignácio da Silva
 * @date 12 Sept 2016
 * @see http://www.aignacio.com
 */

#ifndef __SNMP_H__
#define __SNMP_H__

#include <stdbool.h>
#include "simple-udp.h"
#include "clock.h"
#include "etimer.h"
#include "ctimer.h"
#include "list.h"
#include "net/ip/uip-debug.h"
#include "sys/ctimer.h"
#include "homestark.h"

/**
 * @brief Default SNMP Agent port
 */
#define DEFAULT_SNMP_PORT       161

/**
 * @brief Types of errors in SNMP PDU
 */
#define ERROR_NONE              0x00 /**  @brief No error occurred */
#define ERROR_RESP_TOO_LARGE    0x01 /**  @brief Response message too large to transpor */
#define ERROR_REQ_OID_NOT_FOUND 0x02 /**  @brief The name of the requested object was not found */
#define ERROR_DATA_TYPE_MATCH   0x03 /**  @brief A data type in the request did not match the data type in the SNMP agent */
#define ERROR_MAN_READ_ONLY     0x04 /**  @brief The SNMP manager attempted to set a read-only parameter */
#define ERROR_GENERAL           0x05 /**  @brief General Error (some error other than the ones listed above) */

/**
 * @brief Primitives data types of ANS.1 encoding
 */
#define ASN1_PRIM_INTEGER       0x02
#define ASN1_PRIM_OCT_STR       0x04
#define ASN1_PRIM_NULL          0x05
#define ASN1_PRIM_OID           0x06
#define ASN1_PRIM_IP_ADDRESS    0x40
#define ASN1_PRIM_TIMESTAMP     0x43

/**
 * @brief Max data types in each kind of variable
 */
// #define MAX_COMMUNITY_STRING 0x80 // 128 bytes
#define MAX_OCTET_STRING        0xFA // 250 bytes
#define MAX_OID_STRING          20   // 20 bytes - 20 levels in tree
#define MAX_UDP_SNMP            300

/**
 * @brief Complex data types of ANS.1 encoding
 */
#define ASN1_CPX_SEQUENCE       0x30
#define ASN1_CPX_GET_REQ        0xA0
#define ASN1_CPX_NEXT_REQ       0xA1
#define ASN1_CPX_GET_RESP       0xA2
#define ASN1_CPX_SET_REQ        0xA3
#define ASN1_CPX_TRAP           0xA4

#define MAX_OIDS                14             /** @brief Number max. of address that the device will answer about MIB Implementation */
#define MAX_STRINGS_LENGTH      100            /** @brief Max length of string in the OID Implementation */
#define TIME_UPDATE_SNMP        2*CLOCK_SECOND /** @brief Time to update the OIDs of MIB implementation */

#define TIME_TRAP_HEARTBEAT     10*CLOCK_SECOND /** @brief Define the time to send trap callback heartbeat message */
#define TRAP_COLD_START         0
#define TRAP_WARM_START         1
#define TRAP_LINK_DOWN          2
#define TRAP_LINK_UP            3
#define TRAP_AUTH_FAILURE       4
#define TRAP_NEIGHBOR_LOSS      5
#define TRAP_ENTERPRISE_SPEC    6
#define TRAP_SNMP_PORT          162

/** @brief value of the version field for the SNMPv1 */
#define SNMP_VERSION_1					0
/** @brief value of the version field for the SNMPv2c */
#define SNMP_VERSION_2C					1
/** @brief value of the version field for the SNMPv3 */
#define SNMP_VERSION_3					3
/** @brief Decode the initial sequence type */
#define check_seq(x) (x == ASN1_CPX_SEQUENCE ? 1 : 0)

//#define DEBUG_SNMP_DECODING /** @brief If defined, show decode SNMP messages */
#define DEBUG_SNMP          /** @brief Enable SNMP Debug message */

#ifdef DEBUG_SNMP
#define debug_snmp(fmt, args...) printf("\n[SNMP] "fmt, ##args)
#else
#define debug_snmp(fmt, ...)
#endif

/** @struct snmp_con_t
 *  @brief Struct of SNMP Connection
 *  @var snmp_con_t::udp_port
 *    Port of UDP NMS Connection
 *  @var snmp_con_t::udp_con
 *    Handle of UDP connection
 *  @var snmp_con_t::ipv6_nms
 *    Address IPV6 of NMS
 *  @var snmp_con_t::keep_alive
 *    Time of resend messages
 */
// typedef struct {
//   struct simple_udp_connection udp_con;
//   uint16_t udp_port;
//   uint16_t *ipv6_nms;
//   uint16_t keep_alive;
// } snmp_con_t;

/** @typedef resp_con_t
 *  @brief Type of errors in SNMP functions
 *  @var SUCCESS_CON::FAIL_CON
 *    Error to to process function
 *  @var SUCCESS_CON::SUCCESS_CON
 *    Sucess to process function
 *  @todo Implement more kind of errors
 */
typedef enum {
   FAIL_CON,
   SUCCESS_CON,
} resp_con_t;

/** @struct snmp_t
 *  @brief Struct for the SNMP Message
 *  @var snmp_t:snmp_version
 *    SNMP Version, only supports v1
 *  @var snmp_t:request_type
 *    Type of request to the SNMP Message
 *  @var snmp_t:response_type
 *    Type of the response type
 *  @var snmp_t:request_id_c
 *    Request string of the SNMP Message
 *  @var snmp_t:community
 *    Community string of request
 *  @var snmp_t:oid_encoded
 *    OID tree of the request
 *  @var snmp_t:value
 *    Value of the SNMP request message
 */
typedef struct {
   uint32_t        snmp_version;
   uint8_t         request_type;
   uint8_t         response_type;
   uint8_t         request_id_c[10];
   uint8_t         community[MAX_OCTET_STRING];
   uint8_t         oid_encoded[MAX_OID_STRING];
   uint8_t         value[MAX_OCTET_STRING];
} snmp_t;

/** @struct request
 *  @brief Struct for request of SNMP Packets
 *  @var snmp_t::pdu_request
 *    Request to be processed
 *  @var request::link
 *    Link to the next request
 */
struct request {
    snmp_t pdu_request;
    struct request *link;
}*request_first, *request_last;

/** @brief SNMP Decode Octet String
 *
 * 		Decode Octet String in ASN.1 format.
 *
 *  @param [in] data_encoded Data do decode
 *  @param [in] oct_str Variable the will receive the octet string
 *
 *  @retval SUCCESS_CON Success to decode string octet
 *  @retval FAIL_CON    Fail to decode octet string
 **/
resp_con_t decode_asn1_oct_str(uint8_t *data_encoded, uint8_t *oct_str);

/** @brief SNMP Decode Integer
 *
 * 		Decode ASN.1 integer(32 bit value).
 *
 *  @param [in] data_encoded Data do decode
 *  @param [in] integer_value Variable the will receive the integer 32-bit
 *
 *  @retval SUCCESS_CON Success to decode integer
 *  @retval FAIL_CON    Fail to decode integer
 **/
resp_con_t decode_asn1_integer(uint8_t *data_encoded, uint32_t *integer_value);

/** @brief Decode SNMP message
 *
 * 		Decode a SNMP message v1 and format to answer request.
 *
 *  @param [in] snmp_packet Data UDP - SNMP to decode
 *  @param [in] snmp_handle Struct that will receive the SNMP request messsage
 *
 *  @retval SUCCESS_CON Success to decode SNMP Message
 *  @retval FAIL_CON    Fail to decode SNMP Message
 **/
resp_con_t snmp_decode_message(char *snmp_packet, snmp_t *v);

/** @brief Encode SNMP message
 *
 * 		Encode a SNMP message and format to send the answer.
 *
 *  @param [in] snmp_handle Struct that will be encoded in the SNMP message format
 *  @param [in] data_encoded Variable that'll receive the encoded SNMP Message
 *
 *  @retval length Length of UDP packet encoded
 **/
uint16_t snmp_encode_message(snmp_t *snmp_handle, char *data_encoded);

/** @brief Convert IPv6 address in char format
 *
 * 		Format IPv6 address in string char variable.
 *
 *  @param [in] buf Variable that'll receive the ipv6 address decoded
 *  @param [in] buf_len Len of buf variable
 *  @param [in] addr Address IPv6 in uip_ipaddr_t format
 *
 *  @retval len Length of buf variable formated
 **/
int ipaddr_sprintf(char *buf, uint8_t buf_len, const uip_ipaddr_t *addr);

/** @brief SNMP Callback receive
 *
 * 		Receive in callback mode, any data from NMS of SNMP protocol.
 *
 *  @param [in] void No argument to pass
 *
 *  @retval void Doesn't return anything
 **/
void snmp_cb_data(void);

/** @brief SNMP Init function
 *
 * 		Init SNMP AGENT connection
 *
 *  @param [in] void No argument to pass
 *
 *  @retval void Not return argument
 **/
void snmp_init(void);

#if !CONTIKI_TARGET_Z1
/** @brief Update SNMP OIDs
 *
 * 		Update the OIDs of values from network
 *
 *  @param [in] void No argument to pass
 *
 *  @retval void Not return argument
 **/
void update_snmp_mib(void);
#endif

/** @brief Encode a SNMPv1 Trap message
 *
 * 		Encode a SNMPv1 Trap type message with AWGES PEN (48616) and heartbeat value
 *
 *  @param [in] trap_pdu Variable that'll receive the packet encoded
 *  @param [in] type_trap Type of trap that'll send ("coldStart","WarmStart"...)
 *  @param [in] device_hw Hardware Address
 *
 *  @retval len Length of encoded packet
 **/
uint16_t snmp_encode_trap(uint8_t *trap_pdu, uint8_t type_trap, uint8_t *device_hw);

#endif

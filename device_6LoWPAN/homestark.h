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
 * @license Este projeto está sendo liberado pela licença APACHE 2.0.
 * @file homestark.h
 * @brief Conjunto de protótipos e definiçoes de funções utilizadas
 * @author Ânderson Ignácio da Silva
 * @date 08 Set 2016
 * @see http://www.aignacio.com
 */

#ifndef HOMESTARK_H
#define HOMESTARK_H

/** @brief Defina isto para habilitar mensagens de conversão HASH para garantia de integridade */
//#define ENABLE_HASH_MESSAGES

#define DEBUG_OS
#ifdef DEBUG_OS
#define debug_os(fmt, args...) printf("\n[HomeStark] "fmt, ##args)
#else
#define debug_os(fmt, ...)
#endif

#define DEBUG_COAP
#ifdef DEBUG_OS
#define debug_coap(fmt, args...) printf("\n[CoAP] "fmt, ##args)
#else
#define debug_coap(fmt, ...)
#endif


// typedef enum {
//    water_device,
//    switch_device,
//    light_device,
//    servo_device,
//    smoke_device,
//    printer_device,
//    combo_switch,
//    combo_printer,
//    combo_water,
//    combo_servo
// } devices_t;
//
// devices_t thisDevice = switch_device;

// Define o tipo de dispositivo que será compilado
//#define WATER_DEVICE
#define SWITCH_DEVICE
//#define LIGHT_DEVICE
// #define SERVO_DEVICE
// #define SMOKE_DEVICE
// #define PRINTER_DEVICE
// #define COMBO_SWITCH
// #define COMBO_SERVO
#ifdef WATER_DEVICE
  #define DEVICE_TYPE_STR "device_water\0"
#endif

#ifdef SWITCH_DEVICE
  #define DEVICE_TYPE_STR "device_switch\0"
#endif

#ifdef LIGHT_DEVICE
  #define DEVICE_TYPE_STR "device_light\0"
#endif

#ifdef SERVO_DEVICE
  #define DEVICE_TYPE_STR "device_servo\0"
#endif

#ifdef SMOKE_DEVICE
  #define DEVICE_TYPE_STR "device_smoke\0"
#endif

#ifdef PRINTER_DEVICE
  #define DEVICE_TYPE_STR "device_printer\0"
#endif

#ifdef COMBO_SWITCH
  #define DEVICE_TYPE_STR "device_cswitch\0"
#endif

#ifdef COMBO_SERVO
  #define DEVICE_TYPE_STR "device_cservo\0"
#endif

#endif

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
 * @file mibii.c
 * @brief MIB II Implementation of homestark network
 * @author Ânderson Ignácio da Silva
 * @date 19 Sept 2016
 * @see http://www.aignacio.com
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snmp.h"
#include "mibii.h"

uint8_t  global_index = 0;
oid_data oid_list[MAX_OIDS];

resp_con_t mib_ii_check_oid(uint8_t *mib_oid, uint8_t *index){
  uint8_t i,
          mib_s[4];
  // A correct oid for this implementation must be iso.3.6.1.2.1.x.x.0
  mib_s[0] = *(mib_oid);
  mib_s[1] = *(mib_oid+1),
  mib_s[2] = *(mib_oid+2),
  mib_s[3] = *(mib_oid+3);

  #ifdef DEBUG_SNMP_DECODING
  debug_snmp("MIB to search: iso.3.6.1.2.1.[%d].[%d].[%d].[%d]",mib_s[0],mib_s[1],mib_s[2],mib_s[3]);
  #endif

  if (mib_s[2] != 0 || mib_s[3] != 0)
    return FAIL_CON;

  for (i = 0; i < MAX_OIDS; i++){
    if (oid_list[i].oid_tree[0] == mib_s[0] && oid_list[i].oid_tree[1] == mib_s[1]){
      *index = i;
      return SUCCESS_CON;
    }
    // debug_snmp("%s",mibii_tree[i]);
  }
  #ifdef DEBUG_SNMP_DECODING
  debug_snmp("MIB2 - There isn't OID mapped!");
  #endif
  return FAIL_CON;
}

resp_con_t mib_ii_get_oid(uint8_t *oid, uint8_t *oid_string){
  #if CONTIKI_TARGET_SRF06_CC26XX
    uint8_t index;
    if (!mib_ii_check_oid(oid+7,&index)) return FAIL_CON;

    char data[MAX_STRINGS_LENGTH];
    strcpy(data,oid_list[index].oid_value);

    uint8_t len = strlen(data),
            index2 = 0;
    while (index2 <= len) {
      *(oid_string+index2) = data[index2];
      index2++;
    }
    *(oid_string+index2) = '\0';

    #ifdef DEBUG_SNMP_DECODING
    debug_snmp("MIB2 Decode OID received:%s",data);
    #endif
    return SUCCESS_CON;
  #else
    uint8_t data[] = "z1_snmp\0";
    uint8_t len = 8,
            index2 = 0;
    while (index2 <= len) {
      *(oid_string+index2) = data[index2];
      index2++;
    }
    *(oid_string+index2) = '\0';
    #ifdef DEBUG_SNMP_DECODING
    debug_snmp("MIB2 Decode OID received:%s",(char *)oid_string);
    #endif
    return SUCCESS_CON;
  #endif
}

resp_con_t mib_ii_update_list(uint8_t *tree, char *value){
  uint8_t index_list;
  uint8_t tree_format[4];
  uint8_t mib1 = *tree;
  uint8_t mib2 = *(tree+1);

  tree_format[0] = mib1;
  tree_format[1] = mib2;
  tree_format[2] = 0;
  tree_format[3] = 0;

  // sprintf((void *)tree_format,"%c%c%c%c",mib1,mib2,mib3-0x30,mib4-0x30);
  if (!mib_ii_check_oid(tree_format, &index_list)) return FAIL_CON;
  sprintf(oid_list[index_list].oid_value,"%s",value);


  #ifdef DEBUG_SNMP_DECODING
  debug_snmp("Update MIB2 Indice:%d",index_list);
  debug_snmp("OID Tree: iso.3.6.1.2.1.%d.%d.0",oid_list[index_list].oid_tree[0],oid_list[index_list].oid_tree[1]);
  debug_snmp("OID Value:%s",oid_list[index_list].oid_value);
  #endif
  return SUCCESS_CON;
}

resp_con_t mib_ii_fill_list(uint8_t *oid_tree_var, const char *value){
  if (global_index == MAX_OIDS) return FAIL_CON;
  uint8_t index = global_index++;

  oid_list[index].oid_tree[0]  = *oid_tree_var;
  oid_list[index].oid_tree[1]  = *(oid_tree_var+1);
  strcpy(oid_list[index].oid_value,value);


  return SUCCESS_CON;
}

resp_con_t mib_ii_show(void){
  #ifdef DEBUG_SNMP_DECODING
  size_t i = 0;
    for (i=0; i < global_index; i++) {
      debug_snmp("Index:%d",i);
      debug_snmp("OID Tree: iso.3.6.1.2.1.%d.%d.0",oid_list[i].oid_tree[0],oid_list[i].oid_tree[1]);
      debug_snmp("OID Value:%s",oid_list[i].oid_value);
    }
  #endif
  return SUCCESS_CON;
}

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
 * @file mibii.h
 * @brief MIB II Headers and definitions
 * @author Ânderson Ignácio da Silva
 * @date 19 Sept 2016
 * @see http://www.aignacio.com
 */

#ifndef __MIBII_H__
#define __MIBII_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** @struct oid_data
 *  @brief Struct of OID data in MIB Implementation
 *  @var oid_data::oid_tree
 *    OID tree value in the MIB
 *  @var oid_data::oid_value
 *    OID Data in the tree MIB Implementation
 */
typedef struct {
    uint8_t oid_tree[2];
    char oid_value[MAX_STRINGS_LENGTH];
}oid_data;

/** @brief Check if exist OID
 *
 * 		Run in the MIB Structure to find OID available.
 *
 *  @param [in] mib_oid OID MIB tree string
 *  @param [in] index Index of the position of the MIB searched
 *
 *  @retval SUCCESS_CON Success to find OID value
 *  @retval FAIL_CON    Fail to find OID value
 **/
resp_con_t mib_ii_check_oid(uint8_t *mib_oid, uint8_t *index);

/** @brief Get OID Value in the MIB tree
 *
 * 		Search for OID data in the MIB tree of the OID passed.
 *
 *  @param [in] oid OID MIB tree string
 *  @param [in] oid_string String of the data in the OID-MIB
 *
 *  @retval SUCCESS_CON Success to get the OID Value
 *  @retval FAIL_CON    Fail to get the OID Value
 **/
resp_con_t mib_ii_get_oid(uint8_t *oid, uint8_t *oid_string);

/** @brief Update the MIB OID Tree
 *
 * 	 Search for OID initialized and update the data in the tree, we need to fill the OID first
 *
 *  @param [in] oid OID MIB tree string
 *  @param [in] oid_string String of the data in the OID-MIB
 *
 *  @retval SUCCESS_CON Success update the OID MIB tree
 *  @retval FAIL_CON    Fail to update the OID tree
 **/
resp_con_t mib_ii_update_list(uint8_t *tree, char *value);

/** @brief Init the MIB OID implementation
 *
 * 	 Initialize the MIB OID implementation structure
 *
 *  @param [in] oid_tree_var OID MIB tree string
 *  @param [in] value Value to insert into OID data
 *
 *  @retval SUCCESS_CON Success insert in the OID MIB tree
 *  @retval FAIL_CON    Fail to insert in the OID tree
 **/
resp_con_t mib_ii_fill_list(uint8_t *oid_tree_var, const char *value);

/** @brief List MIB OID values
 *
 * 	 List all MIB Implementation with OID tree and data
 *
 *  @param [in] void Without argument
 *
 *  @retval SUCCESS_CON Success to list OID tree
 *  @retval FAIL_CON    Fail to list OID tree
 **/
resp_con_t mib_ii_show(void);

#endif

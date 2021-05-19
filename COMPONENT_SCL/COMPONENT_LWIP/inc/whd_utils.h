/*
 * Copyright 2021, Cypress Semiconductor Corporation (an Infineon company)
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file whd_utils.h
 *
 * Utilities to help do specialized (not general purpose) WHD specific things
 */

#ifndef INCLUDED_WHD_UTILS_H_
#define INCLUDED_WHD_UTILS_H_

#include "whd_sdpcm.h"
#ifdef __cplusplus
extern "C" {
#endif

/*!
 ******************************************************************************
 * Convert an ipv4 string to a uint32_t.
 *
 * @param[in] ip4addr   : IP address in string format
 * @param[in] len       : length of the ip address string
 * @param[out] dest     : IP address format in uint32
 *
 * @return
 */
bool whd_str_to_ip(const char *ip4addr, size_t len, void *dest);

/*!
 ******************************************************************************
 * Print binary IPv4 address to a string.
 * String must contain enough room for full address, 16 bytes exact.
 * @param[in] ip4addr     : IPv4 address
 * @param[out] p          : ipv4 address in string format
 *
 * @return
 */
uint8_t whd_ip4_to_string(const void *ip4addr, char *p);
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif

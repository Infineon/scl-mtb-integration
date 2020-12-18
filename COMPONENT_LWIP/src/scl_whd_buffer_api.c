/*
 * Copyright 2018-2020 Cypress Semiconductor Corporation
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
 
 /** @file
 *  Provides the API translation from WHD Buffer APIs to SCL Buffer APIs
 */

#include "whd_buffer_api.h"
#include "scl_buffer_api.h" 

/******************************************************
 *               Function Definitions
 ******************************************************/

whd_result_t whd_host_buffer_get(whd_driver_t whd_driver, whd_buffer_t *buffer, whd_buffer_dir_t direction,
                                 uint16_t size, uint32_t wait) 
{
    scl_result_t ret_val;
    ret_val = scl_host_buffer_get(buffer, (scl_buffer_dir_t) direction, size, wait);
    if (ret_val != SCL_SUCCESS) {
        return WHD_BUFFER_ALLOC_FAIL;
    }
    return WHD_SUCCESS;
}

whd_result_t whd_buffer_release(whd_driver_t whd_driver, whd_buffer_t buffer, whd_buffer_dir_t direction) 
{
    scl_buffer_release(buffer, (scl_buffer_dir_t) direction);
    return WHD_SUCCESS;
}

uint8_t *whd_buffer_get_current_piece_data_pointer(whd_driver_t whd_driver, whd_buffer_t buffer) 
{
    return scl_buffer_get_current_piece_data_pointer(buffer);
}

uint16_t whd_buffer_get_current_piece_size(whd_driver_t whd_driver, whd_buffer_t buffer) 
{
    return scl_buffer_get_current_piece_size(buffer);
}

whd_result_t whd_buffer_set_size(whd_driver_t whd_driver, whd_buffer_t buffer, uint16_t size) 
{
    return scl_buffer_set_size(buffer,size);
}

whd_result_t whd_buffer_add_remove_at_front(whd_driver_t whd_driver, whd_buffer_t *buffer, int32_t add_remove_amount) 
{
    return scl_buffer_add_remove_at_front(buffer,add_remove_amount);
}
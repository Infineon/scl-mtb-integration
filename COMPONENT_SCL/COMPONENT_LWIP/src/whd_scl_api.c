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
 *  Provides the API translation from WHD APIs to SCL APIs
 */

#include "whd_wifi_api.h"
#include "cybsp_wifi.h"
#include "whd_events_int.h"
#include "scl_wifi_api.h"
#include "stdio.h"
#include "scl_ipc.h"
#include "cy_network_buffer.h"
#include "scl_buffer_api.h"
#include "cyabs_rtos.h"
#include "whd_cdc_bdc.h"
#include "whd_int.h"

/******************************************************
**         Variable definitions
*******************************************************/

typedef struct
{
    scl_bool_t event_set;
    whd_event_num_t events[SCL_MAX_EVENT_SUBSCRIPTION];
    whd_event_handler_t handler;
    void *handler_user_data;
    uint8_t ifidx;
} scl_whd_event_list_elem_t;

#define DEFAULT_IE_LEN          (1000)
scl_whd_event_list_elem_t scl_whd_event_list[SCL_EVENT_HANDLER_LIST_SIZE];

network_params_t network_parameter;
whd_interface_t whd_interface_for_rx_data;
void scl_scan_callback(scl_scan_result_t *result_ptr, void *user_data, scl_scan_status_t status);
whd_scan_result_callback_t whd_scan_callback;
uint8_t* scan_ie_ptr = NULL;
/******************************************************
 **               Function Declarations
 *******************************************************/

static void *scl_shim_event_handler_t(const scl_event_header_t *event_header,
                                     const uint8_t *event_data, void *handler_user_data);

static scl_result_t scl_whd_management_set_event_handler(const whd_event_num_t *event_nums,
                                                     whd_event_handler_t handler_func,
                                                     void *handler_user_data, uint16_t *event_index);
                                                     

/******************************************************
 *               Function Definitions
 ******************************************************/

uint32_t whd_wifi_set_ioctl_value(whd_interface_t ifp, uint32_t ioctl, uint32_t value)
{
    uint32_t ret_val;
    ret_val = scl_wifi_set_ioctl_value(ioctl,value);
    if (ret_val != SCL_SUCCESS) {
        return WHD_TIMEOUT;
    }
    return WHD_SUCCESS;
}

uint32_t whd_wifi_get_bss_info(whd_interface_t ifp, wl_bss_info_t *bi)
{
    uint32_t retval = CY_RSLT_SUCCESS;
    retval = scl_wifi_get_bss_info((scl_wl_bss_info_t *) bi);
    return retval;
}

uint32_t whd_wifi_is_ready_to_transceive(whd_interface_t ifp)
{
    whd_interface_for_rx_data = ifp;
    uint32_t is_wifi_ready;
    is_wifi_ready = scl_wifi_is_ready_to_transceive();
    return is_wifi_ready;
}

uint32_t whd_wifi_on(whd_driver_t whd_driver, whd_interface_t *ifpp)
{
    bool wifi_status;
	scl_init();
    wifi_status = scl_wifi_on();
    if (wifi_status == false) {
        return WHD_SDIO_BUS_UP_FAIL;
    }
    return WHD_SUCCESS;
}

uint32_t whd_wifi_join(whd_interface_t ifp, const whd_ssid_t *ssid, whd_security_t auth_type,
                              const uint8_t *security_key, uint8_t key_length)
{
    char dummy;
    uint32_t delay_timeout = 0;
    scl_result_t ret_val;

    ret_val = scl_wifi_join((const scl_ssid_t*) ssid, (scl_security_t) auth_type, security_key, key_length);
    
    if (ret_val == SCL_SUCCESS) {
        SCL_LOG(("wifi provisioning in progress\r\n"));
    }

    network_parameter.connection_status = SCL_NSAPI_STATUS_DISCONNECTED;

    //Get the network parameter from NP
    while ((network_parameter.connection_status != SCL_NSAPI_STATUS_GLOBAL_UP) && delay_timeout < NW_CONNECT_TIMEOUT) {
        ret_val = scl_get_nw_parameters(&network_parameter);
        Cy_SysLib_Delay(NW_DELAY_TIME_US/1000);
        delay_timeout++;
    }

    if (delay_timeout >= NW_CONNECT_TIMEOUT || ret_val != SCL_SUCCESS) {
        return WHD_TIMEOUT;
    }

    ret_val = scl_send_data(SCL_TX_CONNECTION_STATUS, &dummy, TIMER_DEFAULT_VALUE);
    if (ret_val != SCL_SUCCESS) {
        return WHD_TIMEOUT;
    }
    return CY_RSLT_SUCCESS;
}

uint32_t whd_wifi_leave(whd_interface_t ifp)
{
    scl_result_t ret_val;
    uint32_t delay_timeout = 0;

    ret_val = scl_wifi_leave();
    if (ret_val == SCL_ERROR) {
        return WHD_TIMEOUT;
    }

    // block till disconnected from network
    while ((network_parameter.connection_status != SCL_NSAPI_STATUS_DISCONNECTED) && delay_timeout < NW_DISCONNECT_TIMEOUT) {
        ret_val = scl_get_nw_parameters(&network_parameter);
        Cy_SysLib_Delay(NW_DELAY_TIME_US/1000);
        delay_timeout++;
    }

    if (delay_timeout >= NW_DISCONNECT_TIMEOUT) {
        return WHD_TIMEOUT;
    }
    
    return WHD_SUCCESS;
}

cy_rslt_t cybsp_wifi_init_primary(whd_interface_t* interface)
{
    bool ret_val;
    whd_interface_t ifp;
    /* creating the interface pointer */
    ifp = (whd_interface_t)malloc(sizeof(struct whd_interface));
    *interface = ifp;
    ifp->role = WHD_STA_ROLE;
    whd_interface_for_rx_data = ifp;
    scl_init();
    ret_val = scl_wifi_on();
    if (ret_val != true) {
        return WHD_SDIO_BUS_UP_FAIL;
    }
    return CY_RSLT_SUCCESS;
}

uint32_t whd_wifi_get_mac_address(whd_interface_t ifp, whd_mac_t *mac)
{
    if(scl_wifi_get_mac_address((scl_mac_t *)mac) == SCL_SUCCESS) {
        return WHD_SUCCESS;
    }
    return WHD_TIMEOUT;
}

uint32_t whd_wifi_register_multicast_address(whd_interface_t ifp, const whd_mac_t *mac)
{    
    if (scl_wifi_register_multicast_address((scl_mac_t *)mac) == SCL_SUCCESS) {
        return WHD_SUCCESS;
    }
    return WHD_TIMEOUT;
}

void whd_network_send_ethernet_data(whd_interface_t ifp, whd_buffer_t buffer)
{
    scl_tx_buf_t scl_tx_buffer_from_lwip;
    scl_tx_buffer_from_lwip.buffer = buffer;
    scl_tx_buffer_from_lwip.size = scl_buffer_get_current_piece_size(buffer);
    scl_network_send_ethernet_data(scl_tx_buffer_from_lwip);
    scl_buffer_release(buffer,SCL_NETWORK_TX);
}

whd_result_t whd_management_set_event_handler(whd_interface_t ifp, const whd_event_num_t *event_nums,
                                                     whd_event_handler_t handler_func,
                                                     void *handler_user_data, uint16_t *event_index)
{
    scl_result_t ret;
    /* Since the "whd_event_handler_t" contains ifp which is not reuired in COMPONENT_SCL, 
       we create a local event callback and register it with the COMPONENT_SCL */
    ret = scl_management_set_event_handler( (scl_event_num_t*) event_nums, scl_shim_event_handler_t, handler_user_data,event_index);
    if (ret != SCL_SUCCESS) {
        return WHD_TIMEOUT;
    }
    
    /* Register the event callback locally, this "handler_func" will be called when the scl_shim_event_handler_t is called */
    
    ret = scl_whd_management_set_event_handler(event_nums, handler_func, handler_user_data,event_index);
    if (ret != SCL_SUCCESS) {
        return WHD_TIMEOUT;
    }
    
    return WHD_SUCCESS;
}

void scl_network_process_ethernet_data(scl_buffer_t buffer)
{
    cy_network_process_ethernet_data(whd_interface_for_rx_data,buffer);
}

static void *scl_shim_event_handler_t(const scl_event_header_t *event_header,
                                     const uint8_t *event_data, void *handler_user_data)
{
    uint16_t i;
    uint16_t j;

    for (i = 0; i < (uint16_t)SCL_EVENT_HANDLER_LIST_SIZE; i++)
    {
        if (scl_whd_event_list[i].event_set)
        {
            for (j = 0; scl_whd_event_list[i].events[j] != WLC_E_NONE; ++j)
            {
                if (scl_whd_event_list[i].events[j] == event_header->event_type)
                {
                    /* Correct event type has been found - call the handler function and exit loop */
                    scl_whd_event_list[i].handler_user_data =
                        scl_whd_event_list[i].handler(whd_interface_for_rx_data, (whd_event_header_t*) event_header, event_data, handler_user_data);
                    break;
                }
            }
        }
    }
    return handler_user_data;
}

static uint8_t scl_whd_find_number_of_events(const scl_event_num_t *event_nums)
{
    uint8_t count = 0;

    while (*event_nums != SCL_WLC_E_NONE)
    {
        count++;
        event_nums++;

        if (count >= SCL_MAX_EVENT_SUBSCRIPTION)
            return 0;
    }
    return count + 1;
}

static scl_result_t scl_whd_management_set_event_handler(const whd_event_num_t *event_nums,
                                                     whd_event_handler_t handler_func,
                                                     void *handler_user_data, uint16_t *event_index)
{
    uint16_t entry = (uint16_t)0xFF;
    uint16_t i;
    uint8_t num_of_events;
    num_of_events = scl_whd_find_number_of_events((scl_event_num_t*) event_nums);

    if (num_of_events <= 1)
    {
        SCL_LOG( ("Exceeded the maximum event subscription/no event subscribed\n") );
        return WHD_UNFINISHED;
    }

    /* Find an existing matching entry OR the next empty entry */
    for (i = 0; i < (uint16_t)SCL_EVENT_HANDLER_LIST_SIZE; i++)
    {
        /* Find a matching event list OR the first empty event entry */
        if (!(memcmp(scl_whd_event_list[i].events, event_nums,
                     num_of_events * (sizeof(scl_event_num_t) ) ) ) )
        {
            /* Check if all the data already matches */
            if ( (scl_whd_event_list[i].handler           == handler_func) &&
                 (scl_whd_event_list[i].handler_user_data == handler_user_data) )
            {
                /* send back the entry where the handler is added */
                *event_index = i;
                return SCL_SUCCESS;
            }
        }
        else if ( (entry == (uint16_t)0xFF) && (scl_whd_event_list[i].event_set == SCL_FALSE) )
        {
            entry = i;
        }
    }

    /* Check if handler function was provided */
    if (handler_func != NULL)
    {
        /* Check if an empty entry was not found */
        if (entry == (uint16_t)0xFF)
        {
            SCL_LOG( ("Out of space in event handlers table - try increasing SCL_EVENT_HANDLER_LIST_SIZE\n") );
            return SCL_OUT_OF_EVENT_HANDLER_SPACE;
        }

        /* Add the new handler in at the free space */
        memcpy (scl_whd_event_list[entry].events, event_nums, num_of_events * (sizeof(scl_event_num_t) ) );
        scl_whd_event_list[entry].handler           = handler_func;
        scl_whd_event_list[entry].handler_user_data = handler_user_data;
        scl_whd_event_list[entry].event_set         = SCL_TRUE;
        /* send back the entry where the handler is added */
        *event_index = entry;
    }
    else
    {
        SCL_LOG(("Event handler callback function is NULL/not provided to register\n") );
        return WHD_BADARG;
    }
    return WHD_SUCCESS;
}

uint32_t whd_wifi_deregister_event_handler(whd_interface_t ifp, uint16_t event_index)
{
    if (event_index < SCL_EVENT_HANDLER_LIST_SIZE)
    {
        memset(scl_whd_event_list[event_index].events, 0xFF,
               (sizeof(scl_whd_event_list[event_index].events) ) );
        scl_whd_event_list[event_index].handler = NULL;
        scl_whd_event_list[event_index].handler_user_data = NULL;
        scl_whd_event_list[event_index].event_set = SCL_FALSE;
        return WHD_SUCCESS;
    }
    if (event_index == 0xFF)
    {
        return WHD_SUCCESS;
    }
    /* Invalid event index received to deregister the event handler */
    return WHD_BADARG;
}

void scl_scan_callback(scl_scan_result_t *result_ptr, void *user_data, scl_scan_status_t status)
{
    whd_scan_result_t **p_result_scan;
    uint32_t temp_var;
    p_result_scan = (whd_scan_result_t **) &temp_var;
    *p_result_scan = (whd_scan_result_t*) result_ptr;
    if (status != SCL_SCAN_COMPLETED_SUCCESSFULLY) {
        if (scan_ie_ptr == NULL) {
            scan_ie_ptr = malloc(DEFAULT_IE_LEN);
        }
        memcpy(scan_ie_ptr,(*p_result_scan)->ie_ptr,(*p_result_scan)->ie_len);
        (*p_result_scan)->ie_ptr = scan_ie_ptr;
        whd_scan_callback(p_result_scan, user_data, (whd_scan_status_t)status);
    }
    if (status == SCL_SCAN_COMPLETED_SUCCESSFULLY) {
        free(scan_ie_ptr);
        scan_ie_ptr = NULL;
        whd_scan_callback(NULL, user_data, (whd_scan_status_t)status);
    }
}
uint32_t whd_wifi_scan(whd_interface_t ifp,
                              whd_scan_type_t scan_type,
                              whd_bss_type_t bss_type,
                              const whd_ssid_t *optional_ssid,
                              const whd_mac_t *optional_mac,
                              const uint16_t *optional_channel_list,
                              const whd_scan_extended_params_t *optional_extended_params,
                              whd_scan_result_callback_t callback,
                              whd_scan_result_t *result_ptr,
                              void *user_data)
{
    scl_result_t ret_val;
    /* storing the callback */
    whd_scan_callback = callback;
    /* calling scl_wifi_scan */
    ret_val = scl_wifi_scan((scl_scan_type_t) scan_type, (scl_bss_type_t) bss_type, (scl_ssid_t*)optional_ssid, (scl_mac_t* )optional_mac, (uint16_t*) optional_channel_list, (scl_scan_extended_params_t* )optional_extended_params,(scl_scan_result_callback_t) scl_scan_callback,(scl_scan_result_t*) result_ptr, user_data);
    if (ret_val == SCL_SUCCESS) {
        return WHD_SUCCESS;
    }
    return WHD_TIMEOUT;
}

whd_interface_t whd_get_primary_interface(whd_driver_t whd_driver) {
    return (whd_interface_t)WHD_STA_ROLE;
}

whd_result_t whd_wifi_get_fwcap(whd_interface_t ifp, uint32_t *value)
{
    *value = (1 << WHD_FWCAP_SAE_EXT);
    return WHD_SUCCESS;
}

whd_result_t whd_wifi_send_auth_frame(whd_interface_t ifp, whd_auth_params_t *auth_params)
{
    return WHD_SUCCESS;
}

whd_result_t whd_wifi_set_auth_status(whd_interface_t ifp, whd_auth_req_status_t *params)
{
    return WHD_SUCCESS;
}

whd_result_t whd_wifi_set_pmk(whd_interface_t ifp, const uint8_t *security_key, uint8_t key_length)
{
    return WHD_SUCCESS;
}

whd_result_t whd_wifi_set_pmksa(whd_interface_t ifp, const pmkid_t *pmkid)
{
    return WHD_SUCCESS;
}

uint32_t whd_wifi_external_auth_request(whd_interface_t ifp,
                                               whd_auth_result_callback_t callback,
                                               void *result_ptr,
                                               void *user_data)
{
    return WHD_SUCCESS;
}

uint32_t whd_wifi_stop_external_auth_request(whd_interface_t ifp)
{
    return WHD_SUCCESS;
}

/* APIs to be supported in future */

uint32_t whd_wifi_get_channels(whd_interface_t ifp, whd_list_t *channel_list) {
    return WHD_WLAN_UNSUPPORTED;
}

cy_rslt_t cybsp_wifi_init_secondary(whd_interface_t* interface, whd_mac_t* mac_address)
{
    return CY_RSLT_SUCCESS;
}

uint32_t whd_wifi_get_ioctl_value(whd_interface_t ifp, uint32_t ioctl, uint32_t *value)
{
    return WHD_SUCCESS;
}

uint32_t whd_wifi_set_ioctl_buffer(whd_interface_t ifp, uint32_t ioctl, void *buffer, uint16_t buffer_length)
{
    return WHD_SUCCESS;
}

uint32_t whd_wifi_off(whd_interface_t ifp)
{
    return WHD_SUCCESS;
}

uint32_t whd_wifi_set_up(whd_interface_t ifp)
{
    return WHD_SUCCESS;
}

uint32_t whd_wifi_set_down(whd_interface_t ifp)
{
    return WHD_SUCCESS;
}



uint32_t whd_wifi_join_specific(whd_interface_t ifp, const whd_scan_result_t *ap, const uint8_t *security_key,
                                       uint8_t key_length)
{
    return WHD_SUCCESS;
}
uint32_t whd_wifi_get_ioctl_buffer(whd_interface_t ifp, uint32_t ioctl, uint8_t *out_buffer,
                                          uint16_t out_length)
{
    return WHD_SUCCESS;
}

uint32_t whd_wifi_unregister_multicast_address(whd_interface_t ifp, const whd_mac_t *mac)
{
    return WHD_SUCCESS;
}

uint32_t whd_wifi_stop_scan(whd_interface_t ifp)
{
    return WHD_SUCCESS;
}

cy_rslt_t cybsp_wifi_deinit(whd_interface_t interface)
{
    return WHD_SUCCESS;
}
whd_result_t whd_cdc_send_ioctl(whd_interface_t ifp, cdc_command_type_t type, uint32_t command,
                                whd_buffer_t send_buffer_hnd,
                                whd_buffer_t *response_buffer_hnd)
{
    return WHD_SUCCESS;
}
void *whd_cdc_get_ioctl_buffer(whd_driver_t whd_driver,
                               whd_buffer_t *buffer,
                               uint16_t data_length) 
{
	whd_buffer_t ret_val = NULL;
	return ret_val;
}
uint32_t whd_wifi_manage_custom_ie(whd_interface_t ifp, whd_custom_ie_action_t action,
                                          const uint8_t *oui, uint8_t subtype, const void *data,
                                          uint16_t length, uint16_t which_packets)
{
        return CY_RSLT_SUCCESS;
}
uint32_t whd_deinit(whd_interface_t ifp)
{
    return WHD_SUCCESS;
}
uint32_t whd_wifi_stop_ap(whd_interface_t ifp)
{
    return WHD_SUCCESS;
}
void *whd_cdc_get_iovar_buffer(whd_driver_t whd_driver,
                               whd_buffer_t *buffer,
                               uint16_t data_length,
                               const char *name)
{
    return NULL;
}
whd_result_t whd_cdc_send_iovar(whd_interface_t ifp, cdc_command_type_t type,
                                whd_buffer_t send_buffer_hnd,
                                whd_buffer_t *response_buffer_hnd)
{
    return WHD_SUCCESS;
}
uint32_t whd_wifi_get_ap_info(whd_interface_t ifp, whd_bss_info_t *ap_info, whd_security_t *security)
{
    return WHD_SUCCESS;
}
uint32_t whd_wifi_get_associated_client_list(whd_interface_t ifp, void *client_list_buffer,
                                                    uint16_t buffer_length)
{
    return WHD_SUCCESS;
}
uint32_t whd_wifi_init_ap(whd_interface_t ifp, whd_ssid_t *ssid, whd_security_t auth_type,
                                 const uint8_t *security_key, uint8_t key_length, uint8_t channel)
{
    return WHD_SUCCESS;
}
uint32_t whd_wifi_start_ap(whd_interface_t ifp)
{
    return WHD_SUCCESS;
}


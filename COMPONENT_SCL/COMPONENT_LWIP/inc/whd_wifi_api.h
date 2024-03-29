/*
 * Copyright 2020 Cypress Semiconductor Corporation
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
 *  Prototypes of functions for controlling the Wi-Fi system
 *
 *  This file provides prototypes for end-user functions which allow
 *  actions such as scanning for Wi-Fi networks, joining Wi-Fi
 *  networks, getting the MAC address, etc
 *
 */

#include "whd.h"
#include "whd_types.h"

#ifndef INCLUDED_WHD_WIFI_API_H
#define INCLUDED_WHD_WIFI_API_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
*             Function declarations
******************************************************/

/** @addtogroup wifi WHD Wi-Fi API
 *  APIs for controlling the Wi-Fi system
 *  @{
 */

/** @addtogroup wifimanagement WHD Wi-Fi Management API
 *  @ingroup wifi
 *  Initialisation and other management functions for WHD system
 *  @{
 */
/**
 * Turn on the Wi-Fi device
 *
 *  Initialise Wi-Fi platform
 *  Program various WiFi parameters and modes
 *
 *  @param  whd_driver        Pointer to handle instance of the driver
 *  @param   ifpp             Pointer to Pointer to handle instance of whd interface
 *
 *  @return WHD_SUCCESS if initialization is successful, error code otherwise
 */
extern uint32_t whd_wifi_on(whd_driver_t whd_driver, whd_interface_t *ifpp);

/**
 * Turn off the Wi-Fi device
 *
 *  - De-Initialises the required parts of the hardware platform
 *    i.e. pins for SDIO/SPI, interrupt, reset, power etc.
 *
 *  - De-Initialises the whd thread which arbitrates access
 *    to the SDIO/SPI bus
 *
 *  @param   ifp              Pointer to handle instance of whd interface
 *
 *  @return WHD_SUCCESS if deinitialization is successful, Error code otherwise
 */
extern uint32_t whd_wifi_off(whd_interface_t ifp);

/** Shutdown this instance of the wifi driver, freeing all used resources
 *
 *  @param   ifp              Pointer to handle instance of whd interface
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_deinit(whd_interface_t ifp);

/** Brings up the Wi-Fi core
 *
 *  @param   ifp              Pointer to handle instance of whd interface
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_set_up(whd_interface_t ifp);

/** Bring down the Wi-Fi core
 *
 *  WARNING / NOTE:
 *     This brings down the Wi-Fi core and existing network connections will be lost.
 *
 *  @param   ifp               Pointer to handle instance of whd interface
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_set_down(whd_interface_t ifp);

/** Creates a secondary interface
 *
 *  @param  whd_drv              Pointer to handle instance of the driver
 *  @param  mac_addr             MAC address for the interface
 *  @param  ifpp                 Pointer to the whd interface pointer
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_add_secondary_interface(whd_driver_t whd_drv, whd_mac_t *mac_addr, whd_interface_t *ifpp);
/**  @} */

/** @addtogroup wifijoin   WHD Wi-Fi Join, Scan and Halt API
 *  @ingroup wifi
 *  Wi-Fi APIs for join, scan & leave
 *  @{
 */

/** Scan result callback function pointer type
 *
 * @param result_ptr   A pointer to the pointer that indicates where to put the next scan result
 * @param user_data    User provided data
 * @param status       Status of scan process
 */
typedef void (*whd_scan_result_callback_t)(whd_scan_result_t **result_ptr, void *user_data, whd_scan_status_t status);

/** Initiates a scan to search for 802.11 networks.
 *
 *  This functions returns the scan results with limited sets of parameter in a buffer provided by the caller.
 *  It is also a blocking call. It is an simplified version of the whd_wifi_scan().
 *
 *  @param   ifp                       Pointer to handle instance of whd interface
 *  @param   scan_result               pointer to user requested records buffer.
 *  @param   count                     No of records user is interested in.
 *                                     If 0 return the total record count.
 *
 *  @note    When scanning specific channels, devices with a strong signal strength on nearby channels may be detected
 *
 *  @return record count or Error code
 */
extern uint32_t whd_wifi_scan_synch(whd_interface_t ifp,
                                    whd_sync_scan_result_t *scan_result,
                                    uint32_t count
                                    );

/** Initiates a scan to search for 802.11 networks.
 *
 *  The scan progressively accumulates results over time, and may take between 1 and 10 seconds to complete.
 *  The results of the scan will be individually provided to the callback function.
 *  Note: The callback function will be executed in the context of the WHD thread and so must not perform any
 *  actions that may cause a bus transaction.
 *
 *  @param   ifp                       Pointer to handle instance of whd interface
 *  @param   scan_type                 Specifies whether the scan should be Active, Passive or scan Prohibited channels
 *  @param   bss_type                  Specifies whether the scan should search for Infrastructure networks (those using
 *                                     an Access Point), Ad-hoc networks, or both types.
 *  @param   optional_ssid             If this is non-Null, then the scan will only search for networks using the specified SSID.
 *  @param   optional_mac              If this is non-Null, then the scan will only search for networks where
 *                                     the BSSID (MAC address of the Access Point) matches the specified MAC address.
 *  @param   optional_channel_list     If this is non-Null, then the scan will only search for networks on the
 *                                     specified channels - array of channel numbers to search, terminated with a zero
 *  @param   optional_extended_params  If this is non-Null, then the scan will obey the specifications about
 *                                     dwell times and number of probes.
 *  @param   callback                  The callback function which will receive and process the result data.
 *  @param   result_ptr                Pointer to a pointer to a result storage structure.
 *  @param   user_data                 user specific data that will be passed directly to the callback function
 *
 *  @note - When scanning specific channels, devices with a strong signal strength on nearby channels may be detected
 *        - Callback must not use blocking functions, nor use WHD functions, since it is called from the context of the
 *          WHD thread.
 *        - The callback, result_ptr and user_data variables will be referenced after the function returns.
 *          Those variables must remain valid until the scan is complete.
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_scan(whd_interface_t ifp,
                              whd_scan_type_t scan_type,
                              whd_bss_type_t bss_type,
                              const whd_ssid_t *optional_ssid,
                              const whd_mac_t *optional_mac,
                              const uint16_t *optional_channel_list,
                              const whd_scan_extended_params_t *optional_extended_params,
                              whd_scan_result_callback_t callback,
                              whd_scan_result_t *result_ptr,
                              void *user_data);

/** Abort a previously issued scan
 *
 *  @param   ifp           Pointer to handle instance of whd interface
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_stop_scan(whd_interface_t ifp);

/** Auth result callback function pointer type
 *
 * @param result_prt   A pointer to the pointer that indicates where to put the auth result
 * @param len          the size of result
 * @param status       Status of auth process
 * @param flag         flag of h2e will be indicated in auth request event, otherwise is NULL.
 * @param user_data    user specific data that will be passed directly to the callback function
 *
 */
typedef void (*whd_auth_result_callback_t)(void *result_ptr, uint32_t len, whd_auth_status_t status, uint8_t *flag,
                                           void *user_data);

/** Initiates SAE auth
 *
 *  The results of the auth will be individually provided to the callback function.
 *  Note: The callback function will be executed in the context of the WHD thread and so must not perform any
 *  actions that may cause a bus transaction.
 *
 *  @param   ifp                       Pointer to handle instance of whd interface
 *  @param   callback                  The callback function which will receive and process the result data.
 *  @param   result_ptr                      Pointer to a pointer to a result storage structure.
 *  @param   user_data                 user specific data that will be passed directly to the callback function
 *
 *  @note - Callback must not use blocking functions, nor use WHD functions, since it is called from the context of the
 *          WHD thread.
 *        - The callback, result_ptr and user_data variables will be referenced after the function returns.
 *          Those variables must remain valid until the scan is complete.
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_external_auth_request(whd_interface_t ifp,
                                               whd_auth_result_callback_t callback,
                                               void *result_ptr,
                                               void *user_data);

/** Abort authentication request
 *
 *  @param   ifp           Pointer to handle instance of whd interface
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_stop_external_auth_request(whd_interface_t ifp);

/** Joins a Wi-Fi network
 *
 *  Scans for, associates and authenticates with a Wi-Fi network.
 *  On successful return, the system is ready to send data packets.
 *
 *  @param   ifp           Pointer to handle instance of whd interface
 *  @param   ssid          A null terminated string containing the SSID name of the network to join
 *  @param   auth_type     Authentication type
 *  @param   security_key  A byte array containing either the cleartext security key for WPA/WPA2/WPA3 secured networks
 *  @param   key_length    The length of the security_key in bytes.
 *
 *  @note    In case of WPA3/WPA2 transition mode, the security_key value is WPA3 password.
 *
 *  @return  WHD_SUCCESS   when the system is joined and ready to send data packets
 *           Error code    if an error occurred
 */
extern uint32_t whd_wifi_join(whd_interface_t ifp, const whd_ssid_t *ssid, whd_security_t auth_type,
                              const uint8_t *security_key, uint8_t key_length);

/** Joins a specific Wi-Fi network
 *
 *  Associates and authenticates with a specific Wi-Fi access point.
 *  On successful return, the system is ready to send data packets.
 *
 *  @param   ifp           Pointer to handle instance of whd interface
 *  @param   ap            A pointer to a whd_scan_result_t structure containing AP details and
 *                         set ap.channel to 0 for unspecificed channel
 *  @param   security_key  A byte array containing either the cleartext security key for WPA/WPA2
 *                         secured networks
 *  @param   key_length    The length of the security_key in bytes.
 *
 *  @return  WHD_SUCCESS   when the system is joined and ready to send data packets
 *           Error code    if an error occurred
 */
extern uint32_t whd_wifi_join_specific(whd_interface_t ifp, const whd_scan_result_t *ap, const uint8_t *security_key,
                                       uint8_t key_length);

/**  @} */

/** @addtogroup wifiutilities   WHD Wi-Fi Utility API
 *  @ingroup wifi
 *  Allows WHD to perform utility operations
 *  @{
 */

/** Set the current channel on the WLAN radio
 *
 *  @note  On most WLAN devices this will set the channel for both AP *AND* STA
 *        (since there is only one radio - it cannot be on two channels simulaneously)
 *
 *  @param   ifp           Pointer to handle instance of whd interface
 *  @param   channel       The desired channel
 *
 *  @return  WHD_SUCCESS   if the channel was successfully set
 *           Error code    if the channel was not successfully set
 */
extern uint32_t whd_wifi_set_channel(whd_interface_t ifp, uint32_t channel);

/** Get the current channel on the WLAN radio
 *
 *  @note On most WLAN devices this will get the channel for both AP *AND* STA
 *       (since there is only one radio - it cannot be on two channels simulaneously)
 *
 *  @param   ifp            Pointer to handle instance of whd interface
 *  @param   channel        Pointer to receive the current channel
 *
 *  @return  WHD_SUCCESS    if the channel was successfully retrieved
 *           Error code     if the channel was not successfully retrieved
 */
extern uint32_t whd_wifi_get_channel(whd_interface_t ifp, uint32_t *channel);

/** Gets the supported channels
 *
 *  @param   ifp                 Pointer to handle instance of whd interface
 *  @param   channel_list        Buffer to store list of the supported channels
 *                               and max channel is WL_NUMCHANNELS
 *
 *  @return  WHD_SUCCESS         if the active connections was successfully read
 *           WHD_ERROR           if the active connections was not successfully read
 */
extern uint32_t whd_wifi_get_channels(whd_interface_t ifp, whd_list_t *channel_list);


/** Set the passphrase
 *
 *  @param   ifp            Pointer to handle instance of whd interface
 *  @param   security_key   The security key (passphrase) which is to be set
 *  @param   key_length     length of the key
 *
 *  @return  WHD_SUCCESS    when the key is set
 *           Error code     if an error occurred
 */
extern uint32_t whd_wifi_set_passphrase(whd_interface_t ifp, const uint8_t *security_key, uint8_t key_length);

/** Set the SAE password
 *
 *  @param   ifp            Pointer to handle instance of whd interface
 *  @param   security_key   The security key (password) which is to be set
 *  @param   key_length     length of the key
 *
 *  @return  WHD_SUCCESS    when the key is set
 *           Error code     if an error occurred
 */
extern uint32_t whd_wifi_sae_password(whd_interface_t ifp, const uint8_t *security_key, uint8_t key_length);

/** Enable WHD internal supplicant and set WPA2 passphrase in case of WPA3/WPA2 transition mode
 *
 *  @param   ifp                Pointer to handle instance of whd interface
 *  @param   security_key_psk   The security key (passphrase) which is to be set
 *  @param   psk_length         length of the key
 *  @param   auth_type          Authentication type: @ref WHD_SECURITY_WPA3_WPA2_PSK
 *
 *  @return  WHD_SUCCESS        when the supplicant variable and wpa2 passphrase is set
 *           Error code         if an error occurred
 */
extern uint32_t whd_wifi_enable_sup_set_passphrase(whd_interface_t ifp, const uint8_t *security_key_psk,
                                                   uint8_t psk_length, whd_security_t auth_type);


/** Enable WHD internal supplicant
 *
 *  @param   ifp            Pointer to handle instance of whd interface
 *  @param   auth_type      Authentication type
 *
 *  @return  WHD_SUCCESS    when the supplicant variable is set
 *           Error code     if an error occurred
 */
extern uint32_t whd_wifi_enable_supplicant(whd_interface_t ifp, whd_security_t auth_type);

/** Retrieve the latest RSSI value
 *
 *  @param   ifp           Pointer to handle instance of whd interface
 *  @param   rssi          The location where the RSSI value will be stored
 *
 *  @return  WHD_SUCCESS   if the RSSI was successfully retrieved
 *           Error code    if the RSSI was not retrieved
 */
extern uint32_t whd_wifi_get_rssi(whd_interface_t ifp, int32_t *rssi);


/**  @} */

/** @addtogroup wifisoftap     WHD Wi-Fi SoftAP API
 *  @ingroup wifi
 *  Wi-Fi APIs to perform SoftAP related functionalities
 *  @{
 */

/** Retrieves AP information
 *
 *  @param  ifp            Pointer to handle instance of whd interface
 *  @param  ap_info        Returns a whd_bss_info_t structure containing AP details
 *  @param  security       Authentication type
 *
 *  @return WHD_SUCCESS    if the AP info was successfully retrieved
 *          Error code     if the AP info was not successfully retrieved
 */
extern uint32_t whd_wifi_get_ap_info(whd_interface_t ifp, whd_bss_info_t *ap_info, whd_security_t *security);

/**  @} */


/** @addtogroup wifijoin   WHD Wi-Fi Join, Scan and Halt API
 *  @ingroup wifi
 *  Wi-Fi APIs for join, scan & leave
 *  @{
 */
/** Disassociates from a Wi-Fi network.
 *  Applicable only for STA role
 *
 *  @param   ifp           Pointer to handle instance of whd interface.
 *
 *  @return  WHD_SUCCESS   On successful disassociation from the AP
 *           Error code    If an error occurred
 */
extern uint32_t whd_wifi_leave(whd_interface_t ifp);
/**  @} */

/** @addtogroup wifiutilities   WHD Wi-Fi Utility API
 *  @ingroup wifi
 *  Allows WHD to perform utility operations
 *  @{
 */

/** Retrieves the current Media Access Control (MAC) address
 *  (or Ethernet hardware address) of the 802.11 device
 *
 *  @param   ifp           Pointer to handle instance of whd interface
 *  @param   mac           Pointer to a variable that the current MAC address will be written to
 *
 *  @return  WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_get_mac_address(whd_interface_t ifp, whd_mac_t *mac);

/** Get the BSSID of the interface
 *
 *  @param  ifp           Pointer to the whd_interface_t
 *  @param  bssid         Returns the BSSID address (mac address) if associated
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_get_bssid(whd_interface_t ifp, whd_mac_t *bssid);
/**  @} */

/** @addtogroup wifisoftap     WHD Wi-Fi SoftAP API
 *  @ingroup wifi
 *  Wi-Fi APIs to perform SoftAP related functionalities
 *  @{
 */

/** Initialises an infrastructure WiFi network (SoftAP)
 *
 *  @param   ifp           Pointer to handle instance of whd interface
 *  @param   ssid          A null terminated string containing the SSID name of the network to join
 *  @param   auth_type     Authentication type
 *  @param   security_key  A byte array containing the cleartext security key for the network
 *  @param   key_length    The length of the security_key in bytes.
 *  @param   channel       802.11 channel number
 *
 *  @return  WHD_SUCCESS   if successfully initialises an AP
 *           Error code    if an error occurred
 */
extern uint32_t whd_wifi_init_ap(whd_interface_t ifp, whd_ssid_t *ssid, whd_security_t auth_type,
                                 const uint8_t *security_key, uint8_t key_length, uint8_t channel);

/** Start the infrastructure WiFi network (SoftAP)
 *  using the parameter set by whd_wifi_init_ap() and optionaly by whd_wifi_manage_custom_ie()
 *
 *  @return  WHD_SUCCESS   if successfully creates an AP
 *           Error code    if an error occurred
 */
extern uint32_t whd_wifi_start_ap(whd_interface_t ifp);

/** Stops an existing infrastructure WiFi network
 *
 *  @param   ifp           Pointer to handle instance of whd interface
 *
 *  @return  WHD_SUCCESS   if the AP is successfully stopped or if the AP has not yet been brought up
 *           Error code    if an error occurred
 */
extern uint32_t whd_wifi_stop_ap(whd_interface_t ifp);


/** Get the maximum number of associations supported by AP interfaces
 *
 *  @param   ifp           Pointer to handle instance of whd interface
 *  @param   max_assoc     The maximum number of associations supported by Soft AP interfaces.
 *
 *  @return  WHD_SUCCESS   if the maximum number of associated clients was successfully read
 *           WHD_ERROR     if the maximum number of associated clients was not successfully read
 */
extern uint32_t whd_wifi_ap_get_max_assoc(whd_interface_t ifp, uint32_t *max_assoc);

/** Gets the current number of active connections
 *
 *  @param   ifp                 Pointer to handle instance of whd interface
 *  @param   client_list_buffer  Buffer to store list of associated clients
 *  @param   buffer_length       Length of client list buffer
 *
 *  @return  WHD_SUCCESS         if the active connections was successfully read
 *           WHD_ERROR           if the active connections was not successfully read
 */
extern uint32_t whd_wifi_get_associated_client_list(whd_interface_t ifp, void *client_list_buffer,
                                                    uint16_t buffer_length);

/** Deauthenticates a STA which may or may not be associated to SoftAP
 *
 * @param   ifp             Pointer to handle instance of whd interface
 * @param   mac             Pointer to a variable containing the MAC address to which the deauthentication will be sent
 *                          NULL mac address will deauthenticate all the associated STAs
 *
 * @param   reason          Deauthentication reason code
 *
 * @return  WHD_SUCCESS     On successful deauthentication of the other STA
 *          WHD_ERROR       If an error occurred
 */
extern uint32_t whd_wifi_deauth_sta(whd_interface_t ifp, whd_mac_t *mac, whd_dot11_reason_code_t reason);

/** Set the beacon interval.
 *
 *  Note that the value needs to be set before ap_start in order to beacon interval to take effect.
 *
 *  @param  ifp            Pointer to handle instance of whd interface
 *  @param  interval       Beacon interval in time units (Default: 100 time units = 102.4 ms)
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_ap_set_beacon_interval(whd_interface_t ifp, uint16_t interval);

/** Set the DTIM interval.
 *
 *  Note that the value needs to be set before ap_start in order to DTIM interval to take effect.
 *
 *  @param  ifp            Pointer to handle instance of whd interface
 *  @param  interval       DTIM interval, in unit of beacon interval
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_ap_set_dtim_interval(whd_interface_t ifp, uint16_t interval);
/**  @} */


/** @addtogroup wifipowersave   WHD Wi-Fi Power Save API
 *  @ingroup wifi
 *  Wi-Fi functions for WLAN low power modes
 *  @{
 */

/** Enables powersave mode on specified interface without regard for throughput reduction
 *
 *  This function enables (legacy) 802.11 PS-Poll mode and should be used
 *  to achieve the lowest power consumption possible when the Wi-Fi device
 *  is primarily passively listening to the network
 *
 *  @param  ifp            Pointer to handle instance of whd interface
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_enable_powersave(whd_interface_t ifp);

/** Enables powersave mode on specified interface while attempting to maximise throughput
 *
 *
 *  Network traffic is typically bursty. Reception of a packet often means that another
 *  packet will be received shortly afterwards (and vice versa for transmit).
 *
 *  In high throughput powersave mode, rather then entering powersave mode immediately
 *  after receiving or sending a packet, the WLAN chip waits for a timeout period before
 *  returning to sleep.
 *
 *  @param   ifp                    Pointer to handle instance of whd interface
 *  @param   return_to_sleep_delay  The variable to set return to sleep delay.
 *                                 return to sleep delay must be set to a multiple of 10 and not equal to zero.
 *
 *  @return  WHD_SUCCESS            if power save mode was successfully enabled
 *           Error code             if power save mode was not successfully enabled
 *
 */
extern uint32_t whd_wifi_enable_powersave_with_throughput(whd_interface_t ifp, uint16_t return_to_sleep_delay);

/** Get powersave mode on specified interface
 *
 *  @param  ifp            Pointer to handle instance of whd interface
 *  @param  value          Value of the current powersave state
 *                          PM1_POWERSAVE_MODE, PM2_POWERSAVE_MODE, NO_POWERSAVE_MODE
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_get_powersave_mode(whd_interface_t ifp, uint32_t *value);

/** Disables 802.11 power save mode on specified interface
 *
 *  @param   ifp               Pointer to handle instance of whd interface
 *
 *  @return  WHD_SUCCESS       if power save mode was successfully disabled
 *           Error code        if power save mode was not successfully disabled
 *
 */
extern uint32_t whd_wifi_disable_powersave(whd_interface_t ifp);
/** @} */

/** @addtogroup wifiutilities   WHD Wi-Fi Utility API
 *  @ingroup wifi
 *  Allows WHD to perform utility operations
 *  @{
 */

/** Set auth status used for External AUTH(SAE)
 *
 *  @param   ifp                    Pointer to handle instance of whd interface
 *  @param   params                 Pointer to Auth_Status structure
 *
 *  @return WHD_SUCCESS or Error code
 */
extern whd_result_t whd_wifi_set_auth_status(whd_interface_t ifp, whd_auth_req_status_t *params);

/** Set the PMK Key
 *
 *  @param   ifp            Pointer to handle instance of whd interface
 *  @param   security_key   The security key (PMK) which is to be set
 *  @param   key_length     length of the PMK(It must be 32 bytes)
 *
 *  @return  WHD_SUCCESS    when the key is set
 *           Error code     if an error occurred
 */
extern whd_result_t whd_wifi_set_pmk(whd_interface_t ifp, const uint8_t *security_key, uint8_t key_length);

/** Send a pre-prepared authentication frame
 *
 *  @param  ifp            Pointer to handle instance of whd interface
 *  @param  auth_params    pointer to a pre-prepared authentication frame structure
 *
 *  @return WHD_SUCCESS or Error code
 */
extern whd_result_t whd_wifi_send_auth_frame(whd_interface_t ifp, whd_auth_params_t *auth_params);

/** Set PMKID in Device (WLAN)
 *
 *  @param   ifp            Pointer to handle instance of whd interface
 *  @param   pmkid          Pointer to BSSID and PMKID(16 bytes)
 *
 *  @return whd_result_t
 */
extern whd_result_t whd_wifi_set_pmksa(whd_interface_t ifp, const pmkid_t *pmkid);

/** Get FW(chip) Capability
 *
 *  @param  ifp            Pointer to handle instance of whd interface
 *  @param  value          Enum value of the current FW capability, ex: sae or sae_ext or ...etc,
 *                         (enum value map to whd_fwcap_id_t)
 *  @return WHD_SUCCESS or Error code
 */
extern whd_result_t whd_wifi_get_fwcap(whd_interface_t ifp, uint32_t *value);

/** Registers interest in a multicast address
 *
 *  Once a multicast address has been registered, all packets detected on the
 *  medium destined for that address are forwarded to the host.
 *  Otherwise they are ignored.
 *
 *  @param  ifp              Pointer to handle instance of whd interface
 *  @param  mac              Ethernet MAC address
 *
 *  @return WHD_SUCCESS      if the address was registered successfully
 *          Error code       if the address was not registered
 */
extern uint32_t whd_wifi_register_multicast_address(whd_interface_t ifp, const whd_mac_t *mac);

/** Unregisters interest in a multicast address
 *
 *  Once a multicast address has been unregistered, all packets detected on the
 *  medium destined for that address are ignored.
 *
 *  @param  ifp              Pointer to handle instance of whd interface
 *  @param  mac              Ethernet MAC address
 *
 *  @return WHD_SUCCESS      if the address was unregistered successfully
 *          Error code       if the address was not unregistered
 */
extern uint32_t whd_wifi_unregister_multicast_address(whd_interface_t ifp, const whd_mac_t *mac);

/** Determines if a particular interface is ready to transceive ethernet packets
 *
 *  @param     ifp                    Pointer to handle instance of whd interface
 *
 *  @return    WHD_SUCCESS            if the interface is ready to transceive ethernet packets
 *             WHD_NOTFOUND           no AP with a matching SSID was found
 *             WHD_NOT_AUTHENTICATED  Matching AP was found but it won't let you authenticate.
 *                                    This can occur if this device is in the block list on the AP.
 *             WHD_NOT_KEYED          Device has authenticated and associated but has not completed the key exchange.
 *                                    This can occur if the passphrase is incorrect.
 *             Error code             if the interface is not ready to transceive ethernet packets
 */
extern uint32_t whd_wifi_is_ready_to_transceive(whd_interface_t ifp);

/* Action Frames */

/** Manage the addition and removal of custom IEs
 *
 *  @param  ifp            Pointer to handle instance of whd interface
 *  @param  action         the action to take (add or remove IE)
 *  @param  oui            the oui of the custom IE
 *  @param  subtype        the IE sub-type
 *  @param  data           a pointer to the buffer that hold the custom IE
 *  @param  length         the length of the buffer pointed to by 'data'
 *  @param  which_packets  A mask to indicate in which all packets this IE should be included. See whd_ie_packet_flag_t.
 *
 *  @return WHD_SUCCESS    if the custom IE action was successful
 *          Error code     if the custom IE action failed
 */
extern uint32_t whd_wifi_manage_custom_ie(whd_interface_t ifp, whd_custom_ie_action_t action,
                                          const uint8_t *oui, uint8_t subtype, const void *data,
                                          uint16_t length, uint16_t which_packets);

/**  @} */

/** @addtogroup wifiioctl   WHD Wi-Fi IOCTL Set/Get API
 *  @ingroup wifi
 *  Set and get IOCTL values
 *  @{
 */
/** Sends an IOCTL command - CDC_SET IOCTL value
 *
 *  @param  ifp            Pointer to handle instance of whd interface
 *  @param  ioctl          CDC_SET - To set the I/O control
 *  @param  value          Data value to be sent
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_set_ioctl_value(whd_interface_t ifp, uint32_t ioctl, uint32_t value);

/** Sends an IOCTL command - CDC_GET IOCTL value
 *
 *  @param  ifp            Pointer to handle instance of whd interface
 *  @param  ioctl          CDC_GET - To get the I/O control
 *  @param  value          Pointer to receive the data value
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_get_ioctl_value(whd_interface_t ifp, uint32_t ioctl, uint32_t *value);

/** Sends an IOCTL command - CDC_SET IOCTL buffer
 *
 *  @param  ifp            Pointer to handle instance of whd interface
 *  @param  ioctl          CDC_SET - To set the I/O control
 *  @param  buffer         Handle for a packet buffer containing the data value to be sent.
 *  @param  buffer_length  Length of buffer
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_set_ioctl_buffer(whd_interface_t ifp, uint32_t ioctl, void *buffer, uint16_t buffer_length);

/** Sends an IOCTL command - CDC_GET IOCTL buffer
 *
 *  @param  ifp           Pointer to handle instance of whd interface
 *  @param  ioctl         CDC_GET - To get the I/O control
 *  @param  out_buffer    Pointer to receive the handle for the packet buffer containing the response data value received
 *  @param  out_length    Length of out_buffer
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_get_ioctl_buffer(whd_interface_t ifp, uint32_t ioctl, uint8_t *out_buffer,
                                          uint16_t out_length);


/**  @} */

/** @addtogroup dbg  WHD Wi-Fi Debug API
 *  @ingroup wifi
 *  WHD APIs which allows debugging like, printing whd log information, getting whd stats, etc.
 *  @{
 */

/** Retrives the bss info
 *
 *  @param  ifp                  Pointer to handle instance of whd interface
 *  @param  bi                   A pointer to the structure wl_bss_info_t
 *
 *  @return WHD_SUCCESS or Error code
 */
extern uint32_t whd_wifi_get_bss_info(whd_interface_t ifp, wl_bss_info_t *bi);

/**  @} */
/**  @} */

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* ifndef INCLUDED_WHD_WIFI_H */

# SCL MTB Integration

SCL MTB Integration is a library which translates Wi-Fi Host Driver (WHD) APIs to Subsystems Communication Layer (SCL) APIs. 
Applications using WHD can use SCL with the help of SCL MTB Integration. The library APIs are thread-safe.

## Features and Functionality
The current implementation has the following features and functionality:
* STA mode. AP and connecurrent AP+STA modes are not supported
* lwIP network stack, FreeRTOS and Wi-Fi Connection Manager to join and communication with a network
* The following APIs are supported
```
    whd_wifi_set_ioctl_value
    whd_wifi_get_bss_info
    whd_wifi_is_ready_to_transceive
    whd_wifi_on
    whd_wifi_join
    cybsp_wifi_init_primary
    whd_wifi_get_mac_address
    whd_wifi_register_multicast_address
    whd_network_send_ethernet_data
    whd_management_set_event_handler
    whd_wifi_leave
    whd_wifi_deregister_event_handler
```



## Supported Platforms
This library and its features are supported on the following Cypress platforms:
* CYSBSYSKIT-01 Rapid IoT Connect Platform RP01 Feather Kit

## Dependent Libraries
This library depends on the following:
* [SCL Communication Layer](SCL)
* Wifi Connection Manager (https://github.com/cypresssemiconductorco/wifi-connection-manager/#latest-v1.X)

## Quick Start
* A set of pre-defined configuration files have been bundled with the wifi-mw-core library for FreeRTOS, lwIP, and mbed TLS. The developer is expected to review the configuration and make adjustments. See the "Quick Start" section in [README.md](https://github.com/cypresssemiconductorco/wifi-mw-core/blob/master/README.md)

* A set of COMPONENTS must be defined in the code example project's Makefile for this library. See the "Quick Start" section in [README.md](https://github.com/cypresssemiconductorco/wifi-mw-core/blob/master/README.md)

* Developer should set the following flag in FreeRTOSConfig to '0'. Other values lead to Tickless mode which is not supported

   ` #define configUSE_TICKLESS_IDLE  0`

## Additional Information
* [Wi-Fi Connection Manager RELEASE.md](./RELEASE.md)

* [Wi-Fi Connection Manager API Documentation](https://cypresssemiconductorco.github.io/wifi-connection-manager/api_reference_manual/html/index.html)

* [Connectivity Utilities API documentation - for cy-log details](https://cypresssemiconductorco.github.io/connectivity-utilities/api_reference_manual/html/group__logging__utils.html)

* [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)

* [Wi-Fi Connection Manager version](./version.txt)

* [ModusToolbox AnyCloud code examples](https://github.com/cypresssemiconductorco?q=mtb-example-anycloud%20NOT%20Deprecated)

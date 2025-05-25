/*
*        SPDX-License-Identifier: BSD-3-Clause
*
*        Copyright (c) 2025, Dennis B. Lewis
*        All rights reserved.
*        This file contains modifications to software originally licensed under the
*        BSD-3-Clause license by the Raspberry Pi Foundation.
*        See LEGAL.TXT in the root directory of this project for more details.
*/
#include "tusb.h"
#include "pico/unique_id.h"
#include "developer_tools.h" // needs ostrich.h for DEVELOPER_CONSOLE variable.
/*
Values below can be changed at:
pico-sdk\sdk\2.1.0\src\rp2_common\pico_stdio_usb\include\pico\stdio_usb.h

***(very useful keep this in mind that these will need to be changed)***

Change the values in this file: pico-sdk\sdk\2.1.0\lib\tinyusb\src\tusb_config.h

    #define CFG_TUD_CDC 3
    #define CFG_TUD_CDC_RX_BUFSIZE (4096 * 2) 
    #define CFG_TUD_CDC_TX_BUFSIZE (4096 * 2)

This will give us more than enough stdin buffer for complex ostrich bulk commands.
*/

/*
Checks if device is self powered is set:
if PICO_STDIO_USB_DEVICE_SELF_POWERED is not defined, 
set PICO_STDIO_USB_DEVICE_SELF_POWERED as true.
Perhaps when used with a battery or non USB power 
(I have noticed device works perfect either way)
*/ 
#ifndef PICO_STDIO_USB_DEVICE_SELF_POWERED
    #define PICO_STDIO_USB_DEVICE_SELF_POWERED (0)
#endif

/*
Capability to reset the device vial the vendor interface:
I have included a developer reset interface on one of the 
COMPORTS if device works without failure or soft errors.
(soft erros = core 1 error)
*/
#ifndef PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE  
    /* 
    needs something like driver or Windows OS 2.0 descriptor
    (someone can add this later if they so choose. not needed)
    */
    #define PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE (0)
#endif

/* 
Generic EMU is RP2350B and or RP2 device:
sets PICO_RP2040 as (0)
*/
#ifndef PICO_RP2040 
    #define PICO_RP2040 (0) 
#endif

/*
Sets the USB Device PID. Because this is a non-RP2040 device
this will default to USBD_PID (0x0009) you can include your
own creative PID number it will not change anything except 
how it is identified by yours and others computer(s). 
*/
#ifndef USBD_PID
    #if PICO_RP2040
        #define USBD_PID (0x000a)
    #else
        // example: 0xA1FA
        #define USBD_PID (0x0009)
    #endif
#endif

/*
USB device VID: this is currently set to Rasberry Pi
I will not touch this for LEGAL purposes and this
is out of scope for this project. please know that
this can be abstracted into a .h file and used for 
the USB DEVICE VENDOR IDENTIFICATON found in the 
ostrich.c file at the top. yes it will need to be 
8 bytes... further explanation found in ostrich.c
*/
#define USBD_VID (0x2E8A) // Raspberry Pi

#ifndef USBD_MANUFACTURER
    #define USBD_MANUFACTURER "Raspberry Pi"
#endif

/*
USB Device Product: this will display your product
name when plugged in for the first time 
(in the windows notification (bottom right)) change:
"Generic Emulator" -> "{product} Emulator"
Example: "Aetherion-2350 EMU"
*/
#ifndef USBD_PRODUCT
    #if DEVELOPER_CONSOLE
        #define USBD_PRODUCT "Generic Developer"
    #else
        #define USBD_PRODUCT "Generic Emulator"
    #endif
#endif

/*
If you intend to add an extra COMPORT you need to increase the number to accompdate this:
#define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + (n + 1) * TUD_CDC_DESC_LEN) (where n =  the current number)
If n = 2 it will only support 2 COMPORTS
(These values will be changed below)
*/
#ifndef USBD_DESC_LEN
    #define TUD_RPI_RESET_DESC_LEN  9
    #if !PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE
        #if DEVELOPER_CONSOLE
            //emulation, datalogging, dev-portal
            #define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + 3 * TUD_CDC_DESC_LEN)
        #else
            //emulation, datalogging
            #define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + 2 * TUD_CDC_DESC_LEN)
        #endif
    #else
        #if DEVELOPER_CONSOLE
            //emulation, datalogging, dev-portal, reset
            #define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + 4 * TUD_CDC_DESC_LEN + TUD_RPI_RESET_DESC_LEN)
        #else
            //emulation, datalogging, reset
            #define USBD_DESC_LEN (TUD_CONFIG_DESC_LEN + 3 * TUD_CDC_DESC_LEN + TUD_RPI_RESET_DESC_LEN)
        #endif
    #endif
#endif
/*
End Statement.
*/


/*
This sets if the device will be self powered as previously stated
on line 35:
This will govern over if the device is self powered via the ECU etc
*/
#if !PICO_STDIO_USB_DEVICE_SELF_POWERED
    #define USBD_CONFIGURATION_DESCRIPTOR_ATTRIBUTE (0)
    #define USBD_MAX_POWER_MA (250)
#else
    #define USBD_CONFIGURATION_DESCRIPTOR_ATTRIBUTE TUSB_DESC_CONFIG_ATT_SELF_POWERED
    #define USBD_MAX_POWER_MA (1)
#endif


/*
The code below i.e. USBD_ITF_CDC_x:
determines how many COMPORTS the device will have.
They need to be spaced apart in equality by 2
Example:
USBD_ITF_CDC_1              (0)
USBD_ITF_CDC_2              (2)
USBD_ITF_CDC_3              (4)
USBD_ITF_CDC_4              (6)
*/
#ifndef USBD_ITF_CDC_1
    #define USBD_ITF_CDC_1              (0)
#endif

#ifndef USBD_ITF_CDC_2
    #define USBD_ITF_CDC_2              (2)
#endif

#ifndef USBD_ITF_CDC_3
    #if DEVELOPER_CONSOLE
        #define USBD_ITF_CDC_3          (4)
    #endif
#endif
// End statement

/*
All USBD_CDCx_EP_CMD, OUT and IN are structured as follows:

    #define USBD_CDCx_EP_CMD        (0x85) <- the previous USBD_CDCx_EP_IN + 1 in HEX
    #define USBD_CDCx_EP_OUT        (0x06) <- the previous USBD_CDCx_EP_OUT + 2 in HEX
    #define USBD_CDCx_EP_IN         (0x86) <- the concurrent USBD_CDCx_EP_CMD + 1 in HEX
*/
#define USBD_CDC1_EP_CMD            (0x81)
#define USBD_CDC1_EP_OUT            (0x02)
#define USBD_CDC1_EP_IN             (0x82)

#define USBD_CDC2_EP_CMD            (0x83)
#define USBD_CDC2_EP_OUT            (0x04)
#define USBD_CDC2_EP_IN             (0x84)

#if DEVELOPER_CONSOLE
    #define USBD_CDC3_EP_CMD        (0x85)
    #define USBD_CDC3_EP_OUT        (0x06)
    #define USBD_CDC3_EP_IN         (0x86)
#endif


/*
If you need an comport to be added to CDC interfaces, you should increase the number.
Each CDC interface typically requires two interface numbers: 
one for the communication class interface (CMD) and another for the data class interface.
Try changing:
USBD_ITF_MAX                (n + 2) ("n" being the current number)
*/

#if !PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE
    #if DEVELOPER_CONSOLE
        //emulation, datalogging, dev-portal
        #define USBD_ITF_MAX       (6)
    #else
        // emulation, datalogging
        #define USBD_ITF_MAX       (4)
    #endif
#else
    #if DEVELOPER_CONSOLE
        // CDC_1 (0), CDC_2 (2), CDC_3 (4)... ect
        #define USBD_ITF_RPI_RESET (6)
        //emulation, datalogging, dev-portal, reset?
        #define USBD_ITF_MAX       (7)
    #else
        #define USBD_ITF_RPI_RESET (6)
        //emulation, datalogging, reset?
        #define USBD_ITF_MAX       (5)
    #endif
#endif
/*
End Statement.
*/

/*
This means that the largest control command (not data packets)
that the device will handle at once is 64 bytes.
(no need to change this)
*/
#define USBD_CDC_CMD_MAX_SIZE       (64) 
#define USBD_CDC_IN_OUT_MAX_SIZE    (64)   

// no need to change this
#define USBD_STR_0                  (0x00)

/*
USBD_STR_CDC_x structure:
USBD_STR_CDC_x + 1(HEX) of the previous USBD_STR_CDC_x
(must be added if you want to add a COMPORT)
*/
#define USBD_STR_MANUF              (0x01)
#define USBD_STR_PRODUCT            (0x02)
#define USBD_STR_SERIAL             (0x03)
#define USBD_STR_CDC_1              (0x04)
#define USBD_STR_CDC_2              (0x05)
#if DEVELOPER_CONSOLE
    #define USBD_STR_CDC_3          (0x06)
#endif

// someone will get around to this (but not needed)
#if PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE  
    #define USBD_STR_RPI_RESET      (0x07)
#endif

// Set up of the device descriptor type array.
static const tusb_desc_device_t usbd_desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200, // can be 0x0210 for Windows OS 2.0 descriptor
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = USBD_VID,
    .idProduct = USBD_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = USBD_STR_MANUF,
    .iProduct = USBD_STR_PRODUCT,
    .iSerialNumber = USBD_STR_SERIAL,
    .bNumConfigurations = 1,
};
// leave as is.
#define TUD_RPI_RESET_DESCRIPTOR(_itfnum, _stridx) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 0, TUSB_CLASS_VENDOR_SPECIFIC, RESET_INTERFACE_SUBCLASS, RESET_INTERFACE_PROTOCOL, _stridx,

/*
If you wish to add or remove a specific COMPORT the TUD_CDC_DESCRIPTOR()
and it contents need to be added here if you like things the way they are
leave as is.
*/
static const uint8_t usbd_desc_cfg[USBD_DESC_LEN] = {
    TUD_CONFIG_DESCRIPTOR(1, USBD_ITF_MAX, USBD_STR_0, USBD_DESC_LEN,
        USBD_CONFIGURATION_DESCRIPTOR_ATTRIBUTE, USBD_MAX_POWER_MA),

    TUD_CDC_DESCRIPTOR(USBD_ITF_CDC_1, USBD_STR_CDC_1, USBD_CDC1_EP_CMD,
        USBD_CDC_CMD_MAX_SIZE, USBD_CDC1_EP_OUT, USBD_CDC1_EP_IN, USBD_CDC_IN_OUT_MAX_SIZE),

    TUD_CDC_DESCRIPTOR(USBD_ITF_CDC_2, USBD_STR_CDC_2, USBD_CDC2_EP_CMD,
        USBD_CDC_CMD_MAX_SIZE, USBD_CDC2_EP_OUT, USBD_CDC2_EP_IN, USBD_CDC_IN_OUT_MAX_SIZE),

#if DEVELOPER_CONSOLE
    TUD_CDC_DESCRIPTOR(USBD_ITF_CDC_3, USBD_STR_CDC_3, USBD_CDC3_EP_CMD,
        USBD_CDC_CMD_MAX_SIZE, USBD_CDC3_EP_OUT, USBD_CDC3_EP_IN, USBD_CDC_IN_OUT_MAX_SIZE),
#endif

#if PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE
    TUD_RPI_RESET_DESCRIPTOR(USBD_ITF_RPI_RESET, USBD_STR_RPI_RESET)
#endif
};

static char usbd_serial_str[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];

static const char *const usbd_desc_str[] = {
    [USBD_STR_MANUF] = USBD_MANUFACTURER,
    [USBD_STR_PRODUCT] = USBD_PRODUCT,
    [USBD_STR_SERIAL] = usbd_serial_str,
    [USBD_STR_CDC_1] = "Generic RT-PROG",       // developer can change these values as they see fit.
    [USBD_STR_CDC_2] = "Generic DATALOG",   // developer can change these values as they see fit.
#if DEVELOPER_CONSOLE
    [USBD_STR_CDC_3] = "Dev-mode",
#endif
#if PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE
    [USBD_STR_RPI_RESET] = "Reset",
#endif
};
//***************************************************************************************************************************************************************************
//                                                  everything below remains unchanged. (correct me if I am wrong)
//***************************************************************************************************************************************************************************
const uint8_t *tud_descriptor_device_cb(void) {
    return (const uint8_t *)&usbd_desc_device;
}

const uint8_t *tud_descriptor_configuration_cb(__unused uint8_t index) {
    return usbd_desc_cfg;
}

const uint16_t *tud_descriptor_string_cb(uint8_t index, __unused uint16_t langid) {
#ifndef USBD_DESC_STR_MAX
#define USBD_DESC_STR_MAX (27)

#elif USBD_DESC_STR_MAX > 127 
#error USBD_DESC_STR_MAX too high (max is 127).
#elif USBD_DESC_STR_MAX < 17
#error USBD_DESC_STR_MAX too low (min is 17).
#endif
    static uint16_t desc_str[USBD_DESC_STR_MAX];

    if (!usbd_serial_str[0]) {
        pico_get_unique_board_id_string(usbd_serial_str, sizeof(usbd_serial_str));
    }

    uint8_t len;
    if (index == 0) {
        desc_str[1] = 0x0409;
        len = 1;
    } else {
        if (index >= sizeof(usbd_desc_str) / sizeof(usbd_desc_str[0])) {
            return NULL;
        }
        const char *str = usbd_desc_str[index];
        for (len = 0; len < USBD_DESC_STR_MAX - 1 && str[len]; ++len) {
            desc_str[1 + len] = str[len];
        }
    }

    desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (2 * len + 2));

    return desc_str;
}
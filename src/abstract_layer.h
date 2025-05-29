/*
*        SPDX-License-Identifier: BSD-3-Clause
*
*        Copyright (c) 2025, Dennis B. Lewis
*        All rights reserved.
*        This file contains modifications to software originally licensed under the
*        BSD-3-Clause license by the Raspberry Pi Foundation.
*        See LEGAL.TXT in the root directory of this project for more details.
*/
#ifndef ABSTRACT_LAYER_H
#define ABSTRACT_LAYER_H
#include "hardware/gpio.h"
/*
Ordered data lines are required for use with assembly programming.
Otherwise, if this is not time critical and you do not wish to use 
assembly you can move the data (D0 ... D7) and address lines 
(A0 ... A15) anywhere you so choose and initalize the pins in the c file.

the abstraction below can be used uncommented in abstract_layer.c
            (very useful when programming in ASM BTW)
*/
// const uint8_t TX   = 0;   // UART0 TX (Push COM data to ECU) (Datalog -> ECU)
// const uint8_t RX   = 1;   // UART0 RX (Push ECU data to COM) (ECU -> Datalog)
// const uint8_t D0   = 2;   // Data to chipset (RAM)(Enabled if HIGH)
// const uint8_t D1   = 3;   // Data to chipset (RAM)(Enabled if HIGH)
// const uint8_t D2   = 4;   // Data to chipset (RAM)(Enabled if HIGH)
// const uint8_t D3   = 5;   // Data to chipset (RAM)(Enabled if HIGH)
// const uint8_t D4   = 6;   // Data to chipset (RAM)(Enabled if HIGH)
// const uint8_t D5   = 7;   // Data to chipset (RAM)(Enabled if HIGH)
// const uint8_t D6   = 8;   // Data to chipset (RAM)(Enabled if HIGH)
// const uint8_t D7   = 9;   // Data to chipset (RAM)(Enabled if HIGH)
// const uint8_t A0   = 10;  // Address pin 0 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A1   = 11;  // Address pin 1 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A2   = 12;  // Address pin 2 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A3   = 13;  // Address pin 3 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A4   = 14;  // Address pin 4 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A5   = 15;  // Address pin 5 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A6   = 16;  // Address pin 6 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A7   = 17;  // Address pin 7 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A8   = 18;  // Address pin 8 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A9   = 19;  // Address pin 9 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A10  = 20;  // Address pin 10 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A11  = 21;  // Address pin 11 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A12  = 22;  // Address pin 12 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A13  = 23;  // Address pin 13 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A14  = 24;  // Address pin 14 to chipset (RAM)(Enabled if HIGH)
// const uint8_t A15  = 25;  // Address pin 16 to chipset (RAM)(Enabled if HIGH) ** Extra for 2^16 byte chipsets **
// const uint8_t OE   = 26;  // Output Enable (Enabled if LOW)(Disabled if HIGH)
// const uint8_t WE   = 27;  // Write Enable (Enabled if LOW) (Disabled if HIGH)                         
// const uint8_t CE   = 28;  // Chip  Enable (Enabled if LOW) (Disabled if HIGH)                        
// const uint8_t HSB  = 29;  // HSB (no need to implement) if implemented: will need to be added in the injection.pio
/*
HSB Duration Issues: Typically 10–20 ms on power down.
(probably do not need this if RP2 device is powered off anyways)
Data from SRAM is transferred to non-volatile memory automatically.
(is literally a monitoring pin for if the NVRAM powers down...
cant monitor anything if RP2 Device is also powered down and not writing.)
still going to add this one in but the problem is 20ms of power up
and 20ms of powerdown will be reserved for data aquisition.
therefore HSB becomes irrelevant as RP2 device will
either be dead during RAM writing or not booted yet...
if AutoStore is enabled: it has automatic storing upon power down.
*/

// // Available pins for use.
// const uint8_t GP30 = 30;  // Reserved for LEDs (check abstract_layer.c)
// const uint8_t GP31 = 31;  // Reserved for LEDs (check abstract_layer.c)
// const uint8_t GP32 = 32;  // Reserved for LEDs (check abstract_layer.c)
// const uint8_t GP33 = 33;  // Use for what you want!
// const uint8_t GP34 = 34;  // Use for what you want!
// const uint8_t GP35 = 35;  // Use for what you want!
// const uint8_t GP36 = 36;  // Use for what you want!
// const uint8_t GP37 = 37;  // Use for what you want!
// const uint8_t GP38 = 38;  // Use for what you want!
// const uint8_t GP39 = 39;  // Use for what you want!

// /*
// Extra ADC pins 
// */
// #define GP40_ADC0  40  // Extra ADC pin
// #define GP41_ADC1  41  // Extra ADC pin
// #define GP42_ADC2  42  // Extra ADC pin
// #define GP43_ADC3  43  // Extra ADC pin
// #define GP44_ADC4  44  // Extra ADC pin
// #define GP45_ADC5  45  // Extra ADC pin
// #define GP46_ADC6  46  // Extra ADC pin
// #define GP47_ADC7  47  // Extra ADC pin


/*
example datalog buffer, decode if you wish.
*/
// void post_data(){
//     uint8_t buffer[52] = {
//     //  ukn   ukn   ukn   ukn   ukn   ukn   ukn   ukn
//         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 8
//     //  ukn   ukn   ukn   ukn   ukn   ukn   ukn   ukn
//         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 16
//     //  ukn   ukn   ukn   ukn   ukn   ukn   ukn   ukn
//         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 24
//     //  ukn   ukn   ukn   ukn   ukn   ukn   ukn   ukn
//         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 32
//     //  ukn   ukn   ukn   ukn   ukn   ukn   ukn   ukn
//         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 40
//     //  ukn   ukn   ukn   ukn   ukn   ukn   ukn   ukn
//         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 48
//     //  ukn   ukn   ukn   ukn 
//         0x00, 0x00, 0x00, 0x00                           // 52
//     };
//     buffer[51] = checksum(buffer, 51); 
//     tud_cdc_n_write(1, buffer, sizeof(buffer));
//     tud_cdc_n_write_flush(1);
// }

/*
Abstractions of methods used to achieve data I/O
(I would probably call this in Ostrich.c in the 
ostrich_init function before the loop if used.)
Examples below to initialize this header file.
*/
#define USB_LED  (uint8_t[2]){30, GPIO_OUT}  // USB_LED: display when packets are sent over the USB
#define RW_LED   (uint8_t[2]){31, GPIO_OUT}  // read/write LED: show when read write operation happends
#define ERR_LED  (uint8_t[2]){32, GPIO_OUT}  // error/fault LED: displays if there is a fault 

void initialize_pins();
void deinitialize_pins();
void toggle_usb_led();
void toggle_err_led();
void toggle_rw_led();
#endif
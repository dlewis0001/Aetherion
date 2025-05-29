/*
*        SPDX-License-Identifier: BSD-3-Clause
*
*        Copyright (c) 2025, Dennis B. Lewis
*        All rights reserved.
*        This file contains modifications to software originally licensed under the
*        BSD-3-Clause license by the Raspberry Pi Foundation.
*        See LEGAL.TXT in the root directory of this project for more details.
*/
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "pico/bootrom.h"
#include "mutexes.h"
#include "developer_reset.h"
#include "developer_tools.h"

uint32_t* mutex_holder; // create a 32bit unsigned int for holding mutex owner dummy variable
// ensure that this is executing from RAM and not XIP on flash memory
/*
Obtains the binary in use mutex and holds it until device reset.
This ensures that both cores are locked and non-functioning until reset.
*/
void close_binary_in_use(){
    while (1){                                                                          // Sets loop until break.
        if (!mutex_try_enter(&tune_data.tune_flag, mutex_holder)){                      // Tries to obtain mutex (does not release)
            break;                                                                      // If mutex is obtained break.
        }                                                                               // Otherwise: keep trying to get mutex.        
    }
}

/*
Obtains the bool in use mutex and holds it until device reset.
This ensures that both cores are locked and non-functioning until reset.
*/
void close_bool_in_use(){
    while (1){                                                                          // Sets loop until break.
        if (!mutex_try_enter(&ostrich_usb.data_flag, mutex_holder)){                    // Tries to obtain mutex (does not release)
            break;                                                                      // If mutex is obtained break.
        }                                                                               // otherwise: keep trying to get mutex.
    }
}

/*
Performs a device reset after flashing pin 25:
END DEVELOPER: you will need to change pin 25 to some LED pin on your board.
If you wish to *see* a device reset take place with some LED. 
*/
void set_reset(uint8_t* command){
    if (!DEVELOPER_CONSOLE){return;}                                                    // Perform security check
    // Checks if default LED pin is defined
    #ifdef PICO_DEFAULT_LED_PIN  
        gpio_init(PICO_DEFAULT_LED_PIN);                                                // Initialize the pin if it is defined
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);                                   // Set to output
        for (int i = 0; i < 3; ++i) {                                                   // Iterate 3 times to signify reset is occuring
            gpio_put(PICO_DEFAULT_LED_PIN, 1);                                          // Turn on the LED
            sleep_ms(100);                                                              // Wait 100ms
            gpio_put(PICO_DEFAULT_LED_PIN, 0);                                          // Turn off the LED
            sleep_ms(100);                                                              // Wait 100ms
        }
    #endif
    reset_usb_boot(0, 0);  // Reset the device and bring up the MSD for reflash
}

/*
Flashes device clean: deleting everything!
Can be very useful when having flash memory errors when debugging.
END DEVELOPER: you will need to change pin 25 to some LED pin on your board.
If you wish to *see* a device reset take place with some LED. 
This does not flash clean and reboot if not running from RAM. (bottom of CMakeLists.txt)
*/
void set_clean(uint8_t* command){
    if (!DEVELOPER_CONSOLE){return;}                                                    // Perform security check
    uint flash_size_bytes;
    // check if the pico flash size is not defined
    #ifndef PICO_FLASH_SIZE_BYTES
    #warning PICO_FLASH_SIZE_BYTES not set, assuming 16MB
        flash_size_bytes = 16 * 1024 * 1024;
    #else
        flash_size_bytes = PICO_FLASH_SIZE_BYTES;                                       // If it is defined: set to board flash size 
    #endif
    close_bool_in_use();                                                                // Get bool mutex
    close_binary_in_use();                                                              // Get binary mutex and hold both until reset
    for (int i; i < 47; i++){                                                           // Iterate over all 47 pins (RP2350B)                                   flag (perhaps PIN_COUNT constant for later use when using multiple RP2 devices)
        gpio_deinit(i);                                                                 // Deinitalize all pins
    }
    // Check if the default LED pin is defined 
    #ifdef PICO_DEFAULT_LED_PIN  
        gpio_init(PICO_DEFAULT_LED_PIN);                                                // Initalize the LED pin
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);                                   // Set pin to output 
        for (int i = 0; i < 10; ++i){                                                   // Iterate 10 times
            gpio_put(PICO_DEFAULT_LED_PIN, 1);                                          // Voltage out on default pin
            sleep_ms(100);                                                              // Wait 100ms 
            gpio_put(PICO_DEFAULT_LED_PIN, 0);                                          // Set Voltage low
            sleep_ms(100);                                                              // Wait 100ms
        }                                                                               // Signifies the board is wiping flash
        flash_range_erase(0, flash_size_bytes);                                         // Erase the entire flash space
        static const uint8_t eyecatcher[FLASH_PAGE_SIZE] = "UhhDennis was here!";       // Make a cool eye catcher incase someone reads
        flash_range_program(0, eyecatcher, FLASH_PAGE_SIZE);                            // Program that eye catcher on the first page ;)
        reset_usb_boot(0, 0);                                                           // Reset the device and bring up the MSD for reflash
    #endif
}
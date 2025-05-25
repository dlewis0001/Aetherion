/*
*        SPDX-License-Identifier: BSD-3-Clause
*
*        Copyright (c) 2025, Dennis B. Lewis
*        All rights reserved.
*        This file contains modifications to software originally licensed under the
*        BSD-3-Clause license by the Raspberry Pi Foundation.
*        See LEGAL.TXT in the root directory of this project for more details.
*/

#include "pico/stdlib.h"
#include "abstract_layer.h"
#include "hardware/gpio.h"

uint8_t pins[47];     // You can log pins in this buffer to be later deinit.
uint8_t pin_amount;   // The amount of pins you wish to initalize or use size_of etc.

/*
Pin initialization: 
    General function to initalize GPIO pins IN or OUT.
    END DEVELOPER: Function contains nothing you will have
    to include pins you want.
*/
void initialize_pins(){
/*
You can include a script here to initalize your pins.
typical pin initialization may include pins used for:
[LEDs, MCU Alive, USB lights, Communication] 

And 

(Data, Address) if you choose to not use assembly.
Read (injection.c, injection.h and injection.pio)

Abstraction of code below:
*/
    // pin_amount = 47;
    // for (size_t pin = 0; pin < pin_amount; pin++){
    //     gpio_init((uint)pin);
    // }
}

/*
Pin Deinitialization:
    General function to deinitalize GPIO pins you have set.
    END DEVELOPER: Function contains nothing you will have
    to include pins you want.
*/
void deinitialize_pins(){
/*
Specifically used to deinitialize pins that where set.
Abstraction of code below:
*/ 
    // pin_amount = 47;
    // for (size_t pin = 0; pin < pin_amount; pin++){
    //     gpio_deinit((uint)pin);
    // }
}

/* 
Further pin abstraction can be found in:
"abstract_layer.h" with pin numbers and purposes
*/ 
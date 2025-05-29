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
bool usb_led = true;
bool err_led = true;
bool rw_led = true;
const uint8_t* all_pins[3] = {USB_LED,RW_LED,ERR_LED};
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
    uint8_t pin;
    uint8_t i_o;
    for (size_t i = 0; i < 3; i++){
        pin = all_pins[i][0];
        i_o = all_pins[i][1];
        gpio_init(pin);
        gpio_set_dir(pin, i_o);
    }
    for (uint8_t j = 0; j < 2; j++){
        toggle_rw_led();
        toggle_err_led();
        toggle_usb_led();        
    }
    animate();
}

// toggles the USB light on/off when called.
void toggle_usb_led(){
    usb_led = !usb_led;
    gpio_put(USB_LED[0], usb_led);
}

// toggles the error/fault light on/off when called.
void toggle_err_led(){
    err_led = !err_led;
    gpio_put(ERR_LED[0], err_led);
}

// toggles the read/write light on/off when called.
void toggle_rw_led(){
    rw_led = !rw_led;
    gpio_put(RW_LED[0], rw_led);
}

/*
Performs start up animation with LED lights.
EXAMPLE.
*/
void animate(){
    for (uint8_t i = 0; i < 10; i++){
        if (!(i % 2)){
            toggle_err_led();
        }
        if (!(i % 3)){
            toggle_rw_led();
        }
        if (!(i % 5)){
            toggle_usb_led();
        }
        sleep_ms(100);
    }
    toggle_err_led();
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
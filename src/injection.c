/*
*        SPDX-License-Identifier: BSD-3-Clause
*
*        Copyright (c) 2025, Dennis B. Lewis
*        All rights reserved.
*        This file contains modifications to software originally licensed under the
*        BSD-3-Clause license by the Raspberry Pi Foundation.
*        See LEGAL.TXT in the root directory of this project for more details.
*/
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "injection.pio.h"
#include "mutexes.h"
#include "pico/multicore.h"
#include "developer_tools.h"
#include "abstract_layer.h"
#include "tusb.h"
/*
Example for assembly program written below however the end developer can write their own how they see fit.
Methodology:
    Set all address pins (A0–A14) to the target address.
    Set all data pins (D0-D7) to the target data.
    Set (CE = LOW, , WE = LOW, OE = HIGH) i.e. 0b001 or 1 for disable Output Enabled
    The selected address will be written with data (IO(0) – IO(7)).
    This will be done in ASM
*/
static uint32_t injection_data;                                                         // Used for bit wise concat of injection data    
static uint16_t start_address;                                                          // address where to start for micro writes
static uint16_t address;                                                                // Address to send to RAM
static bool connected;                                                                  // Temp connection status
static uint32_t* owner;                                                                 // Dummy place holder for mutex owner
static uint8_t macro_data;                                                              // 32kb data to send to RAM
static uint16_t amount;                                                                 // Amount to write during micro writes
static bool success;                                                                    // Checks if data from mutex was retrieved
static uint8_t bank;                                                                    // Activates extra bank pin
static uint8_t* micro_buffer;                                                           // Add a buffer for micro data
static uint32_t alive_counter;                                                          // Bad guy points record
static bool is_alive;
static PIO pio;                                                                         // Pio statemachine select as pio0


/*
Gets the shared master Ostrich Tune Binary and sets this to data. 
*/
void get_macro_data(){
    while (1){                                                                          // Loop until mutex is granted
        multicore_lockout_victim_init();                                                // Go here if flash is writing to wait it out
        if (mutex_try_enter(&tune_data.tune_flag, owner)){                              // Try to get a mutex                         
            macro_data = *(tune_data.tune_binary + address);                            // Add offset and dereference and copy to macro data
            mutex_exit(&tune_data.tune_flag);                                           // Exit mutex like a burning building
            break;                                                                      // Break out of loop
            }   
    }        
}

/*
Gets 1-256 byte(s) from the 256 available buffer size from tune_data.tune_binary 
guarded var by mutex. sets micro data to the dereferenced value of tune_data.
*/
void get_micro_data(){
    const void* address_ptr;                                                            // Create var for storing ptr (anti compiler flag)
    while (1){                                                                          // This loop definitly returns
        multicore_lockout_victim_init();                                                // related to flash writing
        if (mutex_try_enter(&tune_data.tune_flag, owner)){                              // Obtain a mutex for binary use
            start_address = tune_data.tune_byte_start;                                  // Copies the start address
            address_ptr = (const void *)(tune_data.tune_binary + start_address);        // Store in ptr to keep code comments unfactored
            memcpy(micro_buffer, address_ptr, amount);                                  // Memory copy from that address to data amount
            mutex_exit(&tune_data.tune_flag);                                           // Exit mutex 
            return;                                                                     // Return that we got some data
        }        
    }    
}

/*
Gets the amount value of tune_data.amount. this is later used to decide 
to inject and how much to inject. value ranges from 1 - 256 bytes.
*/
void get_amount(){
    while (1){                                                                          // Loop until mutex achieved
        multicore_lockout_victim_init();                                                // If flash write: go and loop here
        if (mutex_try_enter(&tune_data.tune_flag, owner)){                              // Obtain a mutex for binary use
            amount = tune_data.amount;                                                  // Get the amount value to decide on inject or (not to) or to break
            mutex_exit(&tune_data.tune_flag);                                           // Exit mutex gracefully like an angle
            return;
        }        
    }    
}

/*
Normalizes tune_data.amount to prevent continous write operations.
Yes, could theoretically overwrite tune_data.amount although timing is faster than 200us.
tune_data.amount = 0
*/
void set_amount(){
    while (1){                                                                          // This loop definitly breaks
        multicore_lockout_victim_init();                                                // Gets vitimized by flash writes (just in case)
        if (mutex_try_enter(&tune_data.tune_flag, owner)){                              // Obtain a mutex for binary use
            tune_data.amount = 0;                                                       // Set the data to zero so we do not keep writing
            mutex_exit(&tune_data.tune_flag);                                           // Exit mutex gracefully
            break;                                                                      // Definitely break out of loop
        }        
    }    
}

/*
Gets the shared master Ostrich Connected via USB and sets this to local connected variable. 
*/
void get_connected(){
    // its not needed to force connection due if micro data is being uploaded
    if (mutex_try_enter(&ostrich_usb.data_flag, owner)){                                // Try to obtain mutex
        connected = ostrich_usb.data_ready;                                             // Set local variable to master USB connect variable
        mutex_exit(&ostrich_usb.data_flag);                                             // Exit mutex blocking
        return;                                                                         // return void to prevent local method var reset
    }  
}

/*
Sets the current connection status back to false, this prevents the RAM being written
or unwritten (common failure with time based methods of sync).
Should only be used as a local function.
*/
void set_connect(){
    while (1){
        multicore_lockout_victim_init();                                                // Become a victim to flash writes
        if (mutex_try_enter(&ostrich_usb.data_flag, owner)){                            // Obtain a mutex for USB Connection
            ostrich_usb.data_ready = false;                                             // Set back to false so we do not keep writing RAM.
            mutex_exit(&ostrich_usb.data_flag);                                         // Close the shared resource with some dignity.
            break;                                                                      // Breaking loop
        }        
    }
}

/*
Gets the currently set bank number from mutex structure. 
ostrich(persistant bank) -> injection.
*/
void get_bank(){
    while (1){                                                                          // Loop it until success
        multicore_lockout_victim_init();                                                // If flash want to write, wait here (buttons should never intermingle but just in case.)
        if (mutex_try_enter(&bank_number.bank_flag, owner)){                            // Enter the mutex and tell ostrich to wait
            bank = bank_number.current_bank;                                            // Get the routing number really quick
            mutex_exit(&bank_number.bank_flag);                                         // Give mutex back to ostrich
            return;                                                                     // Return with new found humanity
        }                
    }
}

/*
Concats the injection_data with address and write data.
Creates an address+data payload to inject into RAM. 
*/
void create_macro_payload(){
    injection_data = 0;                                                                 // Make sure to reset before building
    injection_data |= ((uint32_t)address & 0x7FFF) << 8;                                // Set bits 8–22 to address (15 bits)
    injection_data |= ((uint32_t)macro_data & 0xFF);                                    // Set bits 0–7 to data (8 bits) (15 + 8 - 1)
    if (bank){                                                                          // Check if we are on bank 1
        injection_data |=  (1U << 23);                                                  // Set 23rd bit address pin for bank 1
        return;                                                                         // Return payload is made for bank 1
    }
    injection_data &= ~(1U << 23);                                                      // or clear bit 23 (we want bank 0)
}

/*
Ureates a micro payload with address and offset for small injections.
Used only with micro_injection().
*/
void create_micro_payload(){
    injection_data = 0;                                                                 // make sure to reset before building
    injection_data |= ((uint32_t)(address + start_address) & 0x7FFF) << 8;              // set bits 8–22 to address (15 bits)
    injection_data |= ((uint32_t)(micro_buffer[address]) & 0xFF);                       // set bits 0–7 to data (8 bits) (15 + 8 - 1)
    if (bank){                                                                          // Check which bank we are emulating from
        injection_data |=  (1U << 23);                                                  // set 23rd bit address pin for dual bank or single
        return;                                                                         // return if bank 1
    }
    injection_data &= ~(1U << 23);                                                      // clear bit 23 if we dont want second bank active
}

/*
Sends injectable data over the FIFO to be injected into RAM within nanoseconds. 
*/
void macro_injection(){
    while (address != 0x8000){                                                          // Loop until 2**15 has been achieved (32kb)
        multicore_lockout_victim_init();                                                // make it the victim
        get_macro_data();                                                               // See if we can get a byte of data out its pockets
        create_macro_payload();                                                         // If successful create a payload with address+data(concat)
        pio_sm_put_blocking(pio, 0, injection_data);                                    // Get that payload and inject the stuff
        address++;                                                                      // Add 1 to address and find the next guy to knock out.
    }                                                                                   // break when we found all the dudes at all the addresses.
    set_connect();                                                                      // Set the connect mutex value back to false.
    address = 0;                                                                        // zero out that address so we can do it again
}

/*
Real time update, writes 1:256 bytes at a time.
*/
void micro_injection(){
    get_micro_data();                                                                   // See if we can get 1-256 byte(s) of data  
    while (address != amount){                                                          // Loop until 2**15 has been achieved (32kb)
        multicore_lockout_victim_init();                                                // Set the break area for core 0                                                                           
        create_micro_payload();                                                         // If successful create a payload with address+data(concat)
        pio_sm_put_blocking(pio, 0, injection_data);                                    // Send that payload over the FIFO to be injected
        address++;                                                                      // Add 1 to address and get the next byte of data...        
    }                                                                                   // break when all bytes have been written.
    memset(micro_buffer, 0, 256);                                                       // Wash our dirty little hands lol  
    set_amount();                                                                       // Set the new amount if any
    address = 0;                                                                        // Set address to zero for reuse
}

/*
Checks on core 0 working status, returns false if core 0 has ran into an error.
*/
static bool core_alive(){
    while (1){
        if (mutex_try_enter(&ostrich_usb.data_flag, owner)){                            // Capture the flag
            is_alive = ostrich_usb.keep_alive;                                          // See if core 0 is twitching
            ostrich_usb.keep_alive = true;                                              // Set the keep alive to tell core 0 we alive here
            mutex_exit(&ostrich_usb.data_flag);                                         // Have some dignity
            break;                                                                      // break the chain
        }        
    }
    if (is_alive){                                                                      // Check if core 0 is alive
        alive_counter++;                                                                // Add a bad guy check mark
    }
    else{
        alive_counter = 0;                                                              // if core is alive reset counter
    }
    if (alive_counter >= 1000000){                                                      // Set alive really high because core 1 is fast
        return false;                                                                   // return false
    }
    return true;                                                                        // return true if we dont return false
}
/*
Writes to Random Access Memory on PCB from core 1 of RP2 device.
Using either state machines or analog depending on the use case.

let C = Clock Cycle
MCU Injection Specs: 
(1 second over 200.00MHz) || (C = 1 / 200,000,000) || C = 5 Nanoseconds

5 Nanosecond per instruction execution. 
(Can only be achieved cleanly in Assembly (ASM))
*/
void inject_memory(){
    pio = pio0;                                                                         // Specify which pio instance we will use.     
    micro_buffer = malloc(256);                                                         // Cut out some memory for RTP
    injection_program_init(pio, 0,                              
    pio_add_program(pio, &injection_program), 2, 24, 1);                                // Initalize the helper script and assembly
    pio_sm_set_enabled(pio, 0, true);                                                   // Enable pio instance zero in state machine zero 
    while (1){                                                                          // Enter Core 1 primary loop (never exits... ever)
        multicore_lockout_victim_init();                                                // set the victim state for blocking during flash write
        get_bank();                                                                     // get the bank data
        get_amount();                                                                   // gets amount of data for micro injection
        get_connected();                                                                // If the USB is connected set local variable "connected"  to true.
        if (connected && !amount){macro_injection();}                                   // checks connection without amount for macro injection
        if (amount){micro_injection();}                                                 // if data resides in the 256 buffer init a micro inject
        if (!core_alive()){break;}                                                      // check if core 0 is alive if not alive break and show error light
    }
    while (1){
        /*
        Technically doesnt need a mutex
        if no core to provide race condition.
        !!!USB IRQs are served by core 0!!!
        No COMs if core 0 crashes sorry :/
        Activate ERROR_LED here!
        */
        for (uint8_t i = 0; i < 2; i++){                                                // Blink once every second to indicate core 0 or first core
            toggle_err_led();                                                           // toggle the ERROR light
            sleep_ms(250);                                                              // Wait for user to see shiny
        }
        print("CORE_0 ERROR: Ostrich timed out.", -1, false);                           // tell if core 0 still alive DEV-COM out that core 0 errored out
    }
}
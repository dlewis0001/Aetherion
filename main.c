#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"
#include "pico/multicore.h"
#include "ostrich.h"
#include "injection.h"
#include "mutexes.h"
#include "flash_memory.h"
#include <string.h>

// Retrieve anything in flash to be mapped to RP2 RAM                        
uint8_t* bank_data;
uint8_t* bank_zero;
uint8_t* bank_one;

/*
Sets the clock settings to get the desired execution timing.
*/
void over_clock(){
    vreg_set_voltage(VREG_VOLTAGE_1_10);                                                // Sets voltage regulation to 1.1 volts
    set_sys_clock_khz(200000, true);                                                    // Sets system clock from X to 200.00MHz    
}

/*
Open the block for mutexes as they must be open to set.
*/
void enter_block(){
    mutex_enter_blocking(&tune_data.tune_flag);                                         // Enter mutex for tune_data
    mutex_enter_blocking(&bank_number.bank_flag);                                       // Enter mutex for bank_number
}

/*
Sets the heap memory for flash and ostrich temp
*/
void set_memory(){
    ostrich_temp = malloc(32768);                                                       // cut some memory for ostrich data in ram 
    flash_temp = malloc(32768);                                                         // cut some memory for flash data in ram
}

/*
Reads the flash for memory offsets to the location where data is stored.
*/
void read_flash(){
    bank_zero = flash_bank_zero();                                                      // Returns XIP_BASE + BANK_ZERO_OFFSET
    bank_one  = flash_bank_one();                                                       // Returns XIP_BASE + BANK_ONE_OFFSET
    bank_data = read_persist();                                                         // Must return valid uint8_t* from flash
}

/*
Sets the persistant data i.e. which bank to read and where.
*/
void set_banks(){
    memcpy(persist_data, bank_data, 2);                                                 // Copy 2 bytes from flash into RAM
    persist_bank   = persist_data[0];                                                   // Not a pointer, just a byte flag
    volitile_bank  = persist_data[1];                                                   // Same
    bank_number.current_bank = persist_bank;                                            // set the current bank to persist
}

/*
Performs the conditional setting up of mutex vars and ostrich temp data.
*/
void conditional(){
    tune_data.tune_binary = ostrich_temp;                                               // Sets the temp data address to the pointer mutex
    tune_data.tune_bytes = 0;                                                           // Set tune bytes to zero so nothing is injecting at start
    if (persist_bank){
        memcpy(ostrich_temp, bank_one, 32768);                                          // copy bank one data over if bank 1 is requested
    } else {
        memcpy(ostrich_temp, bank_zero, 32768);                                         // set bank zero data if bank zero is requested
    }
}

/*
Closes the mutex block so that it can be used later.
*/
void exit_block(){
    mutex_exit(&tune_data.tune_flag);                                                   // Exit mutexes
    mutex_exit(&bank_number.bank_flag);                                                 // Exit mutexes
}

/*
Emulates ostrich and memory injection to on board RAM.
*/
void start_emulate(){
    multicore_launch_core1(inject_memory);                                              // Launches multi-core process on core 1 to parallel process Chip emulation
    ostrich_init();                                                                     // Continues multi-core process on core 0 to parallel process Ostrich emulation          
}

/*
Used to execute;
voltage regulation, system clock, memory allocation, dual core operation, ostrich initialization.
*/ 
int main(){          
    mutexes_init();            
    over_clock();
    enter_block();
    set_memory();
    read_flash();
    set_banks();
    conditional();
    exit_block();
    start_emulate();
    return 0;
}


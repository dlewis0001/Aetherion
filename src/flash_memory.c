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
#include "hardware/vreg.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "flash_memory.h"
#include "mutexes.h"

/*
Takes uint16_t start address between (0x0000 - 0x8000)
Writes to the Flash Memory to store long term data. 
(Probably for a 2 years max or 1000 tunes. "I've never tuned 1000 cars regardless xD")
Using temp data speeds up writing to flash and injection processes
cannot erase a page at a time it must be a full sector
*/
__not_in_flash("save_to_flash")
void save_to_flash(uint16_t start_address, uint8_t* save_data, bool save_tune){
    uint16_t base_address = start_address - (start_address % FLASH_SECTOR_SIZE);                                        // Normalized address to sector start
    uint32_t location = ((save_tune) ? BANK_ZERO_OFFSET : FLASH_USER_OFFSET);                                           // Location: either bank zero or user presets
    if (persist_bank){location = ((save_tune) ? BANK_ONE_OFFSET : FLASH_USER_OFFSET);}                                  // Location: either bank one or user presets
    uint32_t sector_offset = location + (uint32_t)base_address;                                                         // Create a base address on decision
    uint32_t interrupts = save_and_disable_interrupts();                                                                // Lock out flash for writing (will lock out XIP) (WARNING: Core 1)   
    flash_range_erase(sector_offset, FLASH_SECTOR_SIZE);                                                                // Erase the first sector of 4096 bytes by base address
    flash_range_program(sector_offset, (save_data + base_address), FLASH_SECTOR_SIZE);                                  // Write the first sector of 4096 bytes from temp to flash
    restore_interrupts(interrupts);                                                                                     // Exit lockout of flash data (will unlock XIP)
}

/*
Returns: (XIP_BASE + BANK_ZERO_OFFSET)
*/
uint8_t* flash_bank_zero(){
    return (uint8_t *)(XIP_BASE + BANK_ZERO_OFFSET);                                                                    // return the pointer of eXecute In Place base address + our target offset
}

/*
Flash memory address of flash bank 1:
memcpy(buffer, flash_b1, 2**12);
*/
uint8_t* flash_bank_one(){
    return (uint8_t *)(XIP_BASE + BANK_ONE_OFFSET);                                                                     // return the pointer of eXecute In Place base address + our target offset
}

/*
Flash memory address of user settings:
memcpy(buffer, stngs_usr, 2**12);
*/
uint8_t* read_persist(){
    return (uint8_t *)(XIP_BASE + FLASH_USER_OFFSET);                                                                    // return the pointer of eXecute In Place base address + our target offset
}

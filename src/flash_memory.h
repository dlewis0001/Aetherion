/*
*        SPDX-License-Identifier: BSD-3-Clause
*
*        Copyright (c) 2025, Dennis B. Lewis
*        All rights reserved.
*        This file contains modifications to software originally licensed under the
*        BSD-3-Clause license by the Raspberry Pi Foundation.
*        See LEGAL.TXT in the root directory of this project for more details.
*/
#ifndef FLASH_MEMORY_H
#define FLASH_MEMORY_H

/*
If not previous defined; define flash target offset
This value is chosen by random. This value needs to 
be higher than the flash space required to store this 
program. if program takes up 100 bytes we need our
flash target offset at 101 bytes. As this would be 
a little silly and too tight of a constraint. 
this program is probably slightly larger than 700kb.
Therefore: 1MB flash target is okay for now.
*/
// resides at 1MB
#ifndef BANK_ZERO_OFFSET  
    #define BANK_ZERO_OFFSET (0x100000u)
#endif

// must start at bank zero offset + 32768 bytes + 4096 for one roll over sector rewrite.
#ifndef BANK_ONE_OFFSET  
    #define BANK_ONE_OFFSET (0x100000u + 0x9000u)
#endif

#ifndef FLASH_USER_OFFSET  
    #define FLASH_USER_OFFSET (0x100000u - 0x1000u)
#endif

void save_to_flash(uint16_t start_address, uint8_t* save_data, bool save_ostrich);
uint8_t* flash_bank_zero();
uint8_t* flash_bank_one();
uint8_t* read_persist();

#endif
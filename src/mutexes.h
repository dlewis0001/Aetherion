/*
*        SPDX-License-Identifier: BSD-3-Clause
*
*        Copyright (c) 2025, Dennis B. Lewis
*        All rights reserved.
*        This file contains modifications to software originally licensed under the
*        BSD-3-Clause license by the Raspberry Pi Foundation.
*        See LEGAL.TXT in the root directory of this project for more details.
*/
#ifndef MUTEXES_H
#define MUTEXES_H
#include "pico/sync.h"

/*
Structure for the TUNE BINARY mutex:

    mutex_t tune_flag;
    uint8_t* tune_binary;
    volatile uint8_t* tune_bytes;
*/
typedef struct {
    mutex_t tune_flag;
    volatile uint8_t* tune_binary;
    volatile uint16_t tune_byte_start;
    volatile uint16_t amount;
    volatile uint8_t* tune_bytes;
} shared_binary_t;

/*
Structure for the USB CONNECTION mutex:

    mutex_t data_flag;
    volatile bool data_ready;
*/
typedef struct {
    mutex_t data_flag;
    volatile bool data_ready;
} shared_bool_t;

/*
Structure for the BANK NUMBER mutex:

    mutex_t bank_flag;
    volatile uint8_t current_bank;
*/
typedef struct {
    mutex_t bank_flag;
    volatile uint8_t current_bank;
} shared_bank_t;

/*
variable list found in mutexes.c
*/

extern shared_binary_t tune_data;
extern shared_bool_t ostrich_usb;
extern shared_bank_t bank_number;
extern uint8_t* ostrich_temp;
extern uint8_t* flash_temp;
extern uint8_t* micro_ostrich_temp;
extern uint8_t persist_data[3];
extern uint8_t volitile_bank;
extern uint8_t persist_bank;

/*
function abstraction in mutexes.c
*/

void mutexes_init(void);

#endif
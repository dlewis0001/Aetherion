/*
*        SPDX-License-Identifier: BSD-3-Clause
*
*        Copyright (c) 2025, Dennis B. Lewis
*        All rights reserved.
*        This file contains modifications to software originally licensed under the
*        BSD-3-Clause license by the Raspberry Pi Foundation.
*        See LEGAL.TXT in the root directory of this project for more details.
*/
#include "mutexes.h"

/*
Has Value: tune_data.tune_flag


Example:

if (!mutex_try_enter(&tune_data.tune_flag, owner)){
    memcpy(tune_data.tune_binary, ostrich_temp, 0x8000);
    mutex_exit(&tune_data.tune_flag);
}
*/
shared_binary_t tune_data = {
    .amount = 0,    
    .tune_binary = NULL,
    .tune_bytes = NULL,
    .tune_byte_start = 0,
};

/*
Example:

if (!mutex_try_enter(&ostrich_usb.data_flag, owner)){
    connected = ostrich_usb.data_ready;
    mutex_exit(&ostrich_usb.data_flag);
}
*/
shared_bool_t ostrich_usb = {
    .data_ready = false
};

/*
Has Value: dual.bank_number


Example:

if (!mutex_try_enter(&bank_number.bank_flag, owner)){
    bank = bank_number.current_bank;
    mutex_exit(&bank_number.bank_flag);
}
*/
shared_bank_t bank_number = {
    .current_bank = 0
};

/*
Pointer to temporary BIN data.
Only used in ostrich.c

DO NOT USE AS MUTEX OR STRUCT CALL
*/
uint8_t* ostrich_temp = NULL;

uint8_t* flash_temp = NULL;

/*
Pointer to temporary bytes data.
Only used in ostrich.c

DO NOT USE AS MUTEX OR STRUCT CALL
*/
uint8_t* micro_ostrich_temp = NULL;

/*
Holds the address to the persistant user settings.
DO NOT USE AS MUTEX OR STRUCT CALL
*/
uint8_t persist_data[3] = {0};

/*
Holds the address to the volitile bank number.
DO NOT USE AS MUTEX OR STRUCT CALL
*/
uint8_t volitile_bank = 0;

/*
Holds the address to the persistant bank number.
DO NOT USE AS MUTEX OR STRUCT CALL
*/
uint8_t persist_bank = 0;

/*
quickly initalizes both available mutex structures
it is equivalent to calling:

    mutex_init(&tune_data.tune_flag);
    mutex_init(&ostrich_connected.bool_in_use);
    mutex_init(&dual.number_in_use);
*/
void mutexes_init(){
    mutex_init(&tune_data.tune_flag);
    mutex_init(&ostrich_usb.data_flag);
    mutex_init(&bank_number.bank_flag);
}





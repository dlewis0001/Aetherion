/*
*        SPDX-License-Identifier: BSD-3-Clause
*
*        Copyright (c) 2025, Dennis B. Lewis
*        All rights reserved.
*        This file contains modifications to software originally licensed under the
*        BSD-3-Clause license by the Raspberry Pi Foundation.
*        See LEGAL.TXT in the root directory of this project for more details.
*/
#ifndef DEVELOPER_RESET_H
#define DEVELOPER_RESET_H

/*
Provides abstraction for developer_reset.c
so that these functions can be called later.
*/

void set_clean(uint8_t* command);
void set_reset(uint8_t* command);
void close_binary_in_use();
void close_bool_in_use();

#endif
/*
*        SPDX-License-Identifier: BSD-3-Clause
*
*        Copyright (c) 2025, Dennis B. Lewis
*        All rights reserved.
*        This file contains modifications to software originally licensed under the
*        BSD-3-Clause license by the Raspberry Pi Foundation.
*        See LEGAL.TXT in the root directory of this project for more details.
*/
#ifndef DEVELOPER_TOOLS_H
#define DEVELOPER_TOOLS_H
/*
    Firmware Constants
*/
#define DEVELOPER_CONSOLE  1
/*
Provides abstraction for developer_tools.c
so that these functions can be called later.
*/
void print(char* message, int32_t value, bool hex);

#endif

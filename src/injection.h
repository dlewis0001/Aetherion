/*
*        SPDX-License-Identifier: BSD-3-Clause
*
*        Copyright (c) 2025, Dennis B. Lewis
*        All rights reserved.
*        This file contains modifications to software originally licensed under the
*        BSD-3-Clause license by the Raspberry Pi Foundation.
*        See LEGAL.TXT in the root directory of this project for more details.
*/
#ifndef INJECTION_H
#define INJECTION_H

/*
Provides abstraction for injection.c
so that this function can be called later
such there in main.c when splitting cores off.
*/

void inject_memory();

#endif

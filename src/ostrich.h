/*                         Copyright (c) 2012, Keith Daigle
 *                              All rights reserved.
 *
 * Rebuilt Firmware Protocol based on Ostrich Protocol v2.0 created by Keith Daigle
 * Original Source: https://github.com/keith-daigle/moates
 * 
 * Modifications Copyright (c) 2025 Dennis B. Lewis
 * 
 * Licensed under the Keith Daigle (see LICENSE.TXT file for details)
 * 
 * This file is part of a modified version of Ostrich Protocol v2.0.
 * All modifications are documented and compliant with the original license.
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2025, Dennis B. Lewis
 * All rights reserved.
 * This file contains modifications to software originally licensed under the
 * BSD-3-Clause license by the Raspberry Pi Foundation.
 * See LEGAL.TXT in the root directory of this project for more details.
 */
#ifndef OSTRICH_H
#define OSTRICH_H

/*
    Protocol Commands 
*/
#define CMD_VV   0x5656           // Version Command: responds with version.
#define CMD_Nx   0x4E00           // N(x) Commands: NS, Nn or N(byte) (changing vendor ID).
#define CMD_NS   0x0001           // Nx::Serial Number Command: respons with serial number.
#define CMD_Nn   0x0001           // Nx::Set Vendor ID Command: respons with 'O'.
#define CMD_Bx   0x4200           // Bx::B(x) Commands: BR, BS, BRR, BE, BEE, BER and BES
#define CMD_BR   0x0001           // Bx::Emulation Bank Command: sends back read/write bank info. e.g. 1, 2, 3, ...
#define CMD_BS   0x0001           // BS::Bank Select Command: selects which bank to emulate from.
#define CMD_BER  0x0001           // Bx::Volitile Emulation Bank Command: send back which bank is used for volitile memory. (1)
#define CMD_BEE  0x0001           // Bx::Volitile Emulation Bank Command: send back which bank is used for volitile memory. (2)
#define CMD_BES  0x0001           // Bx::Non-volitile Emulation Bank Command: send back which bank is non-volitile memory.
#define CMD_Rx   0x5200           // Rx::Disperse Read Memory Command: sends back bytes(set{ 256 }) of read memory.
#define CMD_Wx   0x5700           // Wx::Disperse Write Memory Command: writes bytes(set{ 256 }) of recieved memory.
#define CMD_ZR   0x5A52           // Bulk Read Memory Command: sends back bytes(set{ 256 * i }) of read memory.
#define CMD_ZW   0x5A57           // Bulk Write Memory Command: writes bytes(set{ 256 * i }) of recieved memory.
#define CMD_DS   0x1000           // Datalog Start Command: forwards datalog requests.
#define CMD_RT   0x1010           // Datalog Reset Command: forwards datalog requests.
#define CMD_DR   0x2000           // Datalog Read Command: requests datalog array.
#define CMD_DM   0x5000           // Datalog Read Command: requests datalog array.
#define CMD_F1   0x2201           // Erase Flash Command: developer erase flash command.
#define CMD_F2   0x2202           // Rest Device Command: developer reset device command.
#define CMD_FF   0xFF00           // Vendor ID Command: sends back the vendor identification
#define CMD_DC   0x0088           // Disconnect Command: send 'O'.
#define NUL_BY   0x0000           // Null Byte Command: tells loop when to stop parsing struct.
/*          
    Function Declaration
*/
void ostrich_init();

#endif
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

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "ostrich.h"
#include "tusb.h"
#include "mutexes.h"
#include "abstract_layer.h"
#include "flash_memory.h"
#include "developer_reset.h"
#include "hardware/flash.h"
#include "pico/multicore.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "developer_tools.h"

#define UART_ID uart0
#define BAUD_RATE 38400
#define UART_TX_PIN 0
#define UART_RX_PIN 1
/*
As previously mentioned you can add in the Pi Pico descriptors
 if you so choose or a hex character sheet of the serial number
in serial_id. Version will be as follows below as an in line comment.
Vendor id will also be as follows below as an in line comment
data_logging checks to see if we are data logging
start_time is used for connection and injection sychronizing.
connected variable is used for injection synchronizing.
upload_count is used for measuring when to write to flash.
*/
 
static bool connected; 
static uint32_t* owner; 
static uint64_t start_time;   
static uint8_t upload_count;                                                
static uint8_t emulation_bank;
static uint8_t random_access_bank;                                           
static uint8_t serial_id[10] = {0x00, 0x01, 0x02, 0x03, 0x04,
                                0x05, 0x06, 0x07, 0x08, 0x00};                          // BMTune: 0x00; serial ID: {0x01 ... 0x08}; checksum byte: 0x00
static uint8_t version_n[3] = {0x14, 0x09, 0x4F};                                       // Ostrich v2.0 if version 10.12.O
static uint8_t vendor_id[2] = {0x01, 0x00};                                             // Vendor ID: 0x00 for ostrich  
                                
/*
End statement
*/

/*
Command Structure: used for mapping serial data to functions.
*/
typedef struct {
    uint16_t command;
    void (*function)(uint8_t *);
} Command;

/*
Bx Command Structure: used for mapping B commands to thier functions.
*/
typedef struct {                                                                                                                                       
    void (*function)(uint8_t *);                                                                                                                                        
} Bx_list;                                                                                                                                       

/*
Calculates a CRC8
*/
uint8_t checksum(uint8_t* array, size_t amount){                                                                                                                                       
    uint8_t sum = 0;                                                                    // Zero out sum
    for (size_t i = 0; i < amount; i++){                                                // Enter loop for a specific amount of data
        sum += (uint8_t)array[i];                                                       // Add that data together 
    }
    return sum;                                                                         // Return trunicated data
}

bool checksum_wrong(uint8_t* array, size_t amount, uint8_t received){                                                                                                                                       
    uint8_t sum = 0;                                                                    // Zero out sum
    for (size_t i = 0; i < amount; i++){                                                // Enter loop for a specific amount of data
        sum += (uint8_t)array[i];                                                       // Add that data together 
    }
    return sum != received;                                                             // Return if checksums do not match
}

/*
Blocks other cores from performing XIP execution. 
Correct way to save to flash and prevents core 1 crashes.
*/
void save_with_blocking(uint16_t start_address, uint8_t* data, bool is_binary){
    multicore_lockout_start_blocking();                                                 // Blocks core 1 from XIP operations
    save_to_flash(start_address, data, is_binary);                                      // Saves captured data to flash memory (BMTune is gentle on this... sometimes)  
    multicore_lockout_end_blocking();                                                   // Lifts block on core 1
}

/*
Updates the mutex variables for respective use in injection.c
*/
void micro_update_mutexes(uint16_t start_byte, uint16_t length){  
    while (1){                                                                          // Loop until we get that mutex
        if (mutex_try_enter(&tune_data.tune_flag, owner)){                              // Obtain a mutex for binary use
            tune_data.amount = length;                                                  // Specify the length i.e. "amount" in core 1 operation
            tune_data.tune_binary = ostrich_temp;                                       // Sets the address of ostrich_temp to tune_binary pointer
            tune_data.tune_byte_start = start_byte;
            mutex_exit(&tune_data.tune_flag);                                           // Exit mutex like a moral person
            break;                                                                      // Definitely break out of loop
        }        
    }
}

/*
Updates binary and connection status mutex struct values.
Informs core 1 when to inject data.
*/
void bulk_update_mutexes(){
    while (1){
        if (mutex_try_enter(&ostrich_usb.data_flag, owner)){                            // Obtain a mutex for USB Connection
            ostrich_usb.data_ready = true;                                              // Tell Mr.Injection that the Echilada is hot and ready.
            mutex_exit(&ostrich_usb.data_flag);                                         // Close the shared resource with some dignity.
            break;                                                                      // Break that loop!@
        }        
    }
    while (1){
        if (mutex_try_enter(&bank_number.bank_flag, owner)){                            // Obtain a mutex for bank number
            bank_number.current_bank = persist_bank;                                    // Tell Mr.Injection that the Echilada is found on a different bank.
            mutex_exit(&bank_number.bank_flag);                                         // Clean that mutex up for later use.
            break;                                                                      // Break this loop out
        }        
    }
}

/*
Sends the confirmation byte of 'O' used during:
Connections
Write processes
*/
void send_confirm(){
    tud_cdc_write_char('O');                                                            // Write data for output
    tud_cdc_write_flush();                                                              // Flush to tuning software
    connected = true;                                                                   // Set the connection status (used for mutex)
}

/*
Sends the corrupt byte of '?' used during:
bad CRC8 checks
For corrupt data
*/
void send_corrupt(){
    tud_cdc_write_char('?');                                                            // Write data for output
    tud_cdc_write_flush();                                                              // Flush to tuning software
}

/*
Reads bytes with a time out to ensure no bytes get left behind.
*/
void read_bytes(uint8_t* byte, uint32_t start, uint32_t amount){                        // Read bytes and put then into command pointer
    uint16_t ms = 50;                                                                   // Set timeout
    uint32_t bytes_read = 0;                                                            // Set amount of bytes read
    uint64_t start_time = time_us_64();                                                 // Set current time
    while ((time_us_64() - start_time) < (ms * 1000)){                                  // Check for condition of current time being greater than timeout
        tud_task();                                                                     // Absolutely must call this when using tusb, performs the task of data retrieval
        if (tud_cdc_available()){                                                       // Check if bytes are ready to be seen
            uint32_t chunk = tud_cdc_read(&byte[bytes_read + start],
                                             amount - bytes_read);                      // Read bytes and stick into buffer
            bytes_read += chunk;                                                        // Add number of bytes to bytes read already
            if (bytes_read >= amount){                                                  // If the amount of bytes read are equal to or greater than the amount we need... return
                return;                                                                 // <-- return home
            }
        }
    }
}

/*
Reads 2 byte in the Datalog COMPORT.
*/
void datalog_get_request(uint8_t* buffer){ 
    tud_task();                                                                         // Generate task for USB (Get data etc)
    if (tud_cdc_n_available(1)){                                                        // Write data for output
        tud_cdc_n_read(1, buffer, 2);}                                                  // Write data for output
}

/*
Reads 2 bytes in the Developer COMPORT when DEVELOPER_CONSOLE is true.
*/
void developer_get_request(uint8_t* buffer){
    tud_task();                                                                         // Must be called or lock up
    if (tud_cdc_n_available(2)){                                                        // Write data for output
        tud_cdc_n_read(2, buffer, 2);                                                   // Write data for output
    }
}

/*
Sends the version of the Ostrich Protocol to the tuning software.
*/
void post_version(uint8_t* command){
    tud_cdc_write(version_n, sizeof(version_n));                                        // Write data for output
    tud_cdc_write_flush();                                                              // Flush to tuning software
}

/*
Changes the vendor ID byte when command is received
*/
void change_vendor(uint8_t* command){
    read_bytes(command, 2, 9);                                                          // Read bytes and put then into command pointer
    uint8_t cs = checksum(command, 10);                                              // Process checksum
    bool check_sum_match = (cs == command[10]);                                         // Compair checksums gives bool to var
    bool serial_match;                                                                  // Create serial match bool var
    for (uint8_t i = 0; i < 8; i++){                                                    // Iterate 8 times starting from 0
        serial_match = command[i + 2] == serial_id[i + 1];                              // Compair serial 
        if (!serial_match){                                                             // Check for mismatched serial bytes (am i the device?)                          
            break;                                                                      // Break this loop if its not matching up...
        }
    }
    if (!serial_match || !check_sum_match){                                             // Check if serial or check sums are mismatching (intensive!)
        send_corrupt();                                                                 // Mean mug BMTune for wasting processing power.
        return;                                                                         // return and go find some more commands to execute
    }
    vendor_id[1] = command[1];                                                          // If we were the device: comply and set that vendor byte
    send_confirm();                                                                     // Inform BMTune that we did that! *thumbs up*
}

/*
Changes the serial number when command is received.
*/
void change_serial(uint8_t* command){
    read_bytes(command, 2, 9);                                                          // Read bytes and put then into command pointer
    uint8_t cs = checksum(command, 10);                                                 // Checksum the command
    if (cs != command[10]){                                                             // Is data corrupt?
        send_corrupt();                                                                 // Say data is corrupt
        return;                                                                         // return to command processing
    }
    for (uint8_t i = 0; i < 8; i++){                                                    // Loop 8 times starting with 0 
        serial_id[i + 1] = command[i + 2];                                              // Set serial ID to the request
    }
    send_confirm();                                                                     // send confirmation byte that the process is finished.
}

/*
Sends the serial number to the tuning software.
*/
void post_serial(uint8_t* command){
    read_bytes(command, 2, 1);                                                          // Read bytes and put then into command pointer
    uint8_t cs = checksum(command, 2);                                                  // Checksum the command
    if (cs != command[2]){                                                              // Is data corrupt?
        send_corrupt();                                                                 // Say data is corrupt
        return;                                                                         // return to command processing
    }
    serial_id[9] = checksum(serial_id, sizeof(serial_id));                              // Process checksum 
    tud_cdc_write(serial_id, sizeof(serial_id));                                        // Write data for output
    tud_cdc_write_flush();                                                              // Flush to tuning software
}

/*
Deploys correct function for CMD_Nx commands:

post_serial
change_serial
change_vendor
*/
void deploy_nx(uint8_t* command){
    uint16_t Nx = ((uint16_t)command[0] << 8) | command[1];                             // Concat 2 bytes of command and store
    uint16_t ns = 0x4E53;                                                               // Set the NS command bytes
    uint16_t nn = 0x4E6E;                                                               // Set the Nn command bytes
    if (Nx == ns){                                                                      // Check if the NS equals the NX
        post_serial(command);                                                           // Give serial if does
        return;                                                                         // return command processing (Home)
    }
    if (Nx == nn){                                                                      // Check if the Nn equals the NX
        change_serial(command);                                                         // Gives serial change if does
        return;                                                                         // Mountain ma'ma
    }
    change_vendor(command);                                                             // If none: then must be change vendor
}

/*
Sends the vendor ID to the tuning software.
*/
void post_vendor(uint8_t* command){
    tud_cdc_write(vendor_id, 2);                                                        // Write vendor ID for output
    tud_cdc_write_flush();                                                              // Flush to tuning software
}

/*
BR: Sets the bank to read and write from.
*/
void bank_select(uint8_t* command){
    uint8_t cs = checksum(command, 2);                                                  // Checksum the command
    if (cs != command[2]){                                                              // Is data corrupt?
        send_corrupt();                                                                 // Say data is corrupt
        return;                                                                         // return to command processing
    }
    persist_bank = command[2];                                                          // else... set persistant bank
    send_confirm();                                                                     // Send confirmation operation is complete
}

/*
BE: Select a volitile bank to emulate from.
*/
void bank_select_v(uint8_t* command){
    uint8_t cs = checksum(command, 2);                                                  // Little redundant could refactor (checksum)
    if (cs != command[2]){                                                              // Validate checksum
        send_corrupt();                                                                 // Send BMTune a "Nope"
        return;                                                                         // Get on with my day.
    }
    volitile_bank = command[2];                                                         // else... set volatile bank to number
    send_confirm();                                                                     // Send BMTune a "Yup"
}

/*
BS: Sets persistant bank data.
*/
void bank_persist(uint8_t* command){
    uint8_t cs = checksum(command, 2);                                                  // Definitely will need a refactor (checksum)
    if (cs != command[2]){                                                              // Check for inequality
        send_corrupt();                                                                 // If .9 on the dollar send corrupt
        return;                                                                         // Go back home and cry
    }
    persist_bank = command[2];                                                          // else... set persitant bank
    volitile_bank = command[2];                                                         // Set volatile bank (must look into that a little more) 
    uint8_t new_data[2] = {persist_bank, volitile_bank};                                // Set buffer of both persist and volatile
    memcpy(persist_data, new_data, 2);                                                  // Copy memory from new_data to persist_data
    save_with_blocking(0, persist_data, false);                                         // Save to Flash
    send_confirm();                                                                     // Send Tuning software an "Okay"
}

/*
BRR: Sends back which bank is the emulating bank.
*/
void bank_current(uint8_t* command){
    read_bytes(command, 3, 1);                                                          // Read 1 extra bytes into buffer starting at position [3] of buffer 
    uint8_t cs = checksum(command, 3);                                                  // 100% need to refactor this (checksum)
    if (cs != command[3]){                                                              // See if checksums match
        send_corrupt();                                                                 // Send corrupt if they dont
        return;                                                                         // return
    }
    tud_cdc_write(&persist_bank, 1);                                                    // Write data for output
    tud_cdc_write_flush();                                                              // Flush to tuning software
}

/*
BER or BEE: Reads back which is the volitile emulation bank.
*/
void bank_volitile(uint8_t* command){                                       
    read_bytes(command, 3, 1);                                                          // Read 1 extra bytes into buffer starting at position [3] of buffer 
    uint8_t cs = checksum(command, 3);                                                  // Get checksum
    if (cs != command[3]){                                                              // Checks checksums
        send_corrupt();                                                                 // Checksums not checking? -> corrupt 
        return;                                                                         // To main loop
    }
    tud_cdc_write(&volitile_bank, 1);                                                   // Write data for output
    tud_cdc_write_flush();                                                              // Flush to tuning software
}

/*
BES: Sets volitile persistant bank.
*/
void bank_v_persist(uint8_t* command){
    read_bytes(command, 3, 1);                                                          // Read 1 extra bytes into buffer starting at position [3] of buffer 
    uint8_t cs = checksum(command, 3);                                                  // Checksum
    if (cs != command[3]){                                                              // Validate
        send_corrupt();                                                                 // Post "?" packet
        return;                                                                         // Command processing
    }
    tud_cdc_write(&persist_bank, 1);                                                    // push data out to buffer
    tud_cdc_write_flush();                                                              // ...and flush
}

/*
Sends the bank info to the tuning software and should set bank.
needs its own parsing function for this one...
*/
void deploy_bx(uint8_t* command) {
    read_bytes(command, 2, 1);                                                          // Read 2 extra bytes into buffer starting at position [2] of buffer 
    uint16_t short_Bx = ((uint16_t)command[0] << 8) | ((uint16_t)command[1]);           // Concat the command to hold 2 bytes
    uint32_t long_Bx  = ((uint32_t)short_Bx << 8)  | ((uint32_t)command[2]);            // Concat new command to hold 3 bytes
    void (*Bx_fn[7])(uint8_t *) = {                                                     // Initalize a void array of casted uint8 pointers (functions)
        bank_current, bank_volitile, bank_volitile, bank_v_persist,             
        bank_select, bank_select_v, bank_persist
    };
    uint32_t bx_cmd[7] = {                                                              // Initialize our array of "B" commands
        0x425252,  // "BRR"
        0x424545,  // "BEE"
        0x424552,  // "BER"
        0x424553,  // "BES"
        0x4252,    // "BR"
        0x4245,    // "BE"
        0x4253     // "BS"
    };

    Bx_list bx_struct[7];                                                               // Initalize a Bx_list structure holding 7 items
    for (uint8_t i = 0; i < 7; i++){                                                    // Begin iterating 7 times starting with 0
        bx_struct[i].function = Bx_fn[i];                                               // Map the command to its reletive function
    }
    for (uint8_t i = 0; i < 7; i++) {                                                   // Start loop
        if ((i > 3) && ((uint16_t)bx_cmd[i] == short_Bx)){                              // find command in function struct
            bx_struct[i].function(command);                                             // if true execute function related to bx_cmd and forward command pointer to next func
            return;                                                                     // return after function execution
        }
        if ((i <= 3) && (bx_cmd[i] == long_Bx)){                                        // Match command sequence to function
            bx_struct[i].function(command);                                             // If command sequence found execute related functions
            return;
        }
    }
}

/*
Performs security check to see if requested address is out of bounds.
Returns true if the address range is out of bounds.
*/
bool out_bounds(uint16_t start_address, uint16_t length){
    uint32_t address_range = (uint32_t)start_address + (uint32_t)length;                // Add the start address and length to be read together
    return !(0 <= address_range && address_range <= 0x8000);                            // Checks to see if that address is in bounds or out of bounds
}

/*
Returns length between 1-256 based on requested command.
*/
uint16_t length256(uint8_t address){
    return (address == 0) ? (uint16_t)256 : (uint16_t)address;                          // Return address for 1-256 micro offset address
}

/*
Returns length between 0-4096 based on bulk command request.
*/
uint16_t length4096(uint8_t address){
    return (uint16_t)address * 256;                                                     // Return address for 0-4096 bulk offset address
}

/*
Returns an uint16 address based on micro command request.
*/
uint16_t micro_address(uint8_t msb, uint8_t lsb){
    return (((uint16_t)lsb << 8) | msb) - 0x8000;                                       // Return base address for micro start address
}

/*
Returns a uint16 address based on bulk command request.
*/
uint16_t bulk_address(uint8_t msb, uint8_t lsb){
    return (((uint16_t)msb << 8) | lsb) - 0x8000;                                       // Return base address for bulk start address
}

/*
Processes command for a short or small 1-256 read from device.
*/
void micro_read(uint8_t* command){                                                      // R[0], n[1] + MSB[2] + LSB[3] + CS[4]
    uint8_t cs;                                                                         // Create checksum variable
    read_bytes(command, 2, 2);                                                          // Read bytes and put then into command pointer
    uint16_t length = length256(command[1]);                                            // Create dynamic length based on command sequence
    uint16_t start_address = micro_address(command[3], command[2]);                     // Concat start address and subtract 2^15
    uint8_t received_cs[1];
    read_bytes(received_cs, 0, 1);
    bool ncs = checksum_wrong(command, 6, received_cs[0]);                              // Check if data arrived undamaged...
    if (ncs){return;}
    if (out_bounds(start_address, length)){return;}                                     // Check for data in boundry                                                 
    while (1){                                                                          // Enter temp loop to get mutex
        if (mutex_try_enter(&tune_data.tune_flag, owner)){                              // Obtain a mutex for USB Connection
            cs = checksum(&ostrich_temp[start_address], (size_t)length);                // Create checksum with locked content
            tud_cdc_write(&ostrich_temp[start_address], (uint32_t)length);              // Put data into output buffer
            tud_cdc_write(&cs, 1);                                                      // Put checksum at the end of output buffer
            tud_cdc_write_flush();                                                      // Flush buffer to tuning software 
            tune_data.tune_binary = ostrich_temp;                                       // Reset the pointer address to real address
            mutex_exit(&tune_data.tune_flag);                                           // close the shared resource with some dignity.
            break;                                                                      // Break
        }        
    }
}

/*
Processes command for a short or small 1-256 write to device.
*/
void micro_write(uint8_t* command){                                                     // W[0], n[1], MSB[2], LSB[3], bytes[n] checksum[lim~bytes[n] + 1]
    uint16_t length = length256(command[1]);                                            // Create dynamic length based on command sequence
    read_bytes(command, 2, (uint32_t)(length + 2));                                     // Read bytes and put then into command pointer
    uint16_t start_address = micro_address(command[3], command[2]);                     // Concat start address and subtract 2^15
    uint8_t received_cs[1];
    read_bytes(received_cs, 0, 1);
    bool ncs = checksum_wrong(command, length + 4, received_cs[0]);                     // Check if data arrived undamaged...
    if (ncs){return;}
    if (out_bounds(start_address, length)){return;}                                     // Check for data in boundry
    while (1){                                                                          // Enter loop to grantee mutex obtainment
        if (mutex_try_enter(&tune_data.tune_flag, owner)){                              // Obtain a mutex for USB Connection
            memcpy(&ostrich_temp[start_address], &command[4], (size_t)length);          // Copy data to temp   
            memcpy(&flash_temp[start_address], &command[4], (size_t)length);            // Copy temp to flash temp
            tune_data.tune_binary = ostrich_temp;                                       // Set the mutex var pointer to the real address
            mutex_exit(&tune_data.tune_flag);                                           // Close the shared resource with some dignity.
            break;                                                                      // Break that loop!@
        }        
    }
    save_with_blocking(start_address, flash_temp, true);                                // Save with blocking to ensure core 1 doesnt crash
    micro_update_mutexes(start_address, length);                                        // Update the micro mutexes for micro injection
    send_confirm();                                                                     // send confirmation (ready for the next bytes)
}

/*
Processes command for large bulk read (256 - 4096) bytes from device.
*/
void bulk_read(uint8_t* command){                                                       // Z[0], R[1], n[2], MMSB[3], LSB[4], Checksum[5]
    uint8_t cs;                                                                         // Create checksum variable
    read_bytes(command, 2, 3);                                                          // Read bytes and put then into command pointer
    uint16_t length = (uint16_t)command[2] * 256;                                       // Create dynamic length based on command sequence
    uint16_t start_address = bulk_address(command[4], command[3]);                      // Concat start address and subtract 2^15
    uint8_t received_cs[1];
    read_bytes(received_cs, 0, 1);
    bool ncs = checksum_wrong(command, 6, received_cs[0]);                              // Check if data arrived undamaged...
    if (ncs){return;}
    if (out_bounds(start_address, length)){return;}                                     // Check for data in boundry
    while (1){
        if (mutex_try_enter(&tune_data.tune_flag, owner)){                              // Obtain a mutex for USB Connection
            cs = checksum(&ostrich_temp[start_address], (size_t)length);                // Process checksum 
            tud_cdc_write(&ostrich_temp[start_address], (uint32_t)length);              // Write data for output
            tud_cdc_write(&cs, 1);                                                      // Write data for output
            tud_cdc_write_flush();                                                      // Flush to tuning software  
            tune_data.tune_binary = ostrich_temp;                                       // Set the mutex var to real pointer of temp data
            mutex_exit(&tune_data.tune_flag);                                           // close the shared resource with some dignity.
            break;                                                                      // End loop!
        }        
    }
}

/*
Processes command for large bulk write of (256 - 4096) bytes into device.
*/
void bulk_write(uint8_t* command){                                                      // Z[0], W[1], n[2], MMSB[3], MSB[4], bytes[n], checksum[lim~bytes[n] + 1]
    read_bytes(command, 2, 1);                                                          // Read bytes and put then into command pointer
    uint16_t length = (uint16_t)command[2] * 256;                                       // Create dynamic length based on command sequence
    read_bytes(command, 3, (uint32_t)(length + 2));                                     // Read bytes and put then into command pointer
    uint8_t received_cs[1];
    read_bytes(received_cs, 0, 1);
    uint16_t start_address = bulk_address(command[4], command[3]);                      // Concat start address and subtract 2^15
    bool ncs = checksum_wrong(command,
                            (size_t)length + 5,
                            received_cs[0]);                                            // Check if data arrived undamaged...
    if (ncs){return;}
    if (out_bounds(start_address, length)){return;}                                     // Check for data in boundry
    while (1){                                                                          // Enter short loop
            if (mutex_try_enter(&tune_data.tune_flag, owner)){                          // Obtain a mutex for USB Connection
                memcpy(&ostrich_temp[start_address], &command[5], (size_t)length);      // Copy bytes into ostrich temp buffer
                memcpy(&flash_temp[start_address], &command[5], (size_t)length);        // Copy bytes into flash temp buffer
                tune_data.tune_binary = ostrich_temp;                                   // Set mutex var to real pointed value
                mutex_exit(&tune_data.tune_flag);                                       // close the shared resource with some dignity.
            break;                                                                      // Leave loop
        }        
    }
    save_with_blocking(start_address, flash_temp, true);                                // Save with blocking to not crash core 1
    send_confirm();                                                                     // Send confirmation (ready for the next bytes)
    upload_count++;                                                                     // Update the upload count
}

/*
Starts Datalogging if recieved datalog command from tuning software.
*/
void datalog_init(){
    uart_init(UART_ID, BAUD_RATE);                                                      // Initalize the UART zero with slow baud rate
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);                                     // Specify we are using TX pin 0
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);                                     // Using RX pin 1
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);                                   // Set data bits, stop bits and parity
    uart_set_fifo_enabled(UART_ID, true);                                               // Enable it! (Datalogging is now available for reading)
}

/*
Performs the transaction between the computer and the ECU datalogging feature.
All data is merely sent over as it is with no MCU intervention other than facilitating
The comport for datalogging features. 
*/
void transact(uint8_t* datalog_buffer, uint8_t* count, uint8_t size){                   // 0x1000, 0x2000
    uint64_t start_time = time_us_64();                                                 // Log start time 
    uint64_t timeout = 100;                                                             // Set the miliseconds of timeing we want
    timeout = timeout * 1000;                                                           // Creates the value in int 64
    while ((*count < size) && (time_us_64() - start_time < (timeout))){                 // Break if timeout is exeeded or count is reached
        if (uart_is_readable(UART_ID)) {                                                // Check if UART has data
            datalog_buffer[*count] = uart_getc(UART_ID);                                // Puts data into the datalog buffer from UART
            (*count)++;                                                                 // Increments original value
        }
    }                                                                                   // NOTE*: the timeout can be decreased if you need more data quicker.
    tud_cdc_n_write(1, datalog_buffer, *count);                                         // Write the data from the ECU to the buffer
    tud_cdc_n_write_flush(1);                                                           // Flush that data to the Tuning software (very fast actually)
}

/*
Start the transaction for datalogging. This pushes data from the COMPORT over to the 
TX/RX buffer towards the ECU to perform requests for starting or restarting logging.
*/
void start_log(uint8_t* command){
    uint8_t size = 1;                                                                   // We expect to get 1 byte back (0xCD)
    uint8_t count = 0;                                                                  // The bytes we currently have
    uint8_t datalog_buffer[size];                                                       // Our buffer to capture said byte
    uint8_t start_command[2] = {0x10, 0x00};                                            // Start command byte
    uint8_t restart_command[2] = {0x10, 0x10};                                          // Restart command byte (maybe i hallucinated this one)
    if (command[0] == 0x10){                                                            // If the real command starts with 0x10 then send start_command
        uart_write_blocking(UART_ID, start_command, 2);                                 // Sends start command over to the ECU
    }
    else{
        uart_write_blocking(UART_ID, restart_command, 2);                               // If not then reset. (probably not needed)
    }
    transact(datalog_buffer, &count, size);                                             // Send what the ECU said to the Computer
}

/*
Reads and forwards datalog data via uart to datalog comport
*/
void read_and_forward(uint8_t* command){
    uint8_t size = 52;                                                                  // Set the amount of data we want
    uint8_t count = 0;                                                                  // Set the amount of data we have
    uint8_t datalog_buffer[size];                                                       // Create a buffer to store the data we will get
    uart_write_blocking(UART_ID, command, 2);                                           // Request data from the ECU
    transact(datalog_buffer, &count, size);                                             // Read 52 bytes and forward to the Computer
}

/*
literally does nothing. Needed for command struct.
*/
void post_null(uint8_t* command){
// DO NOT DELETE!
}

/*
If DEVELOPER_CONSOLE is on, this will print unknown commands to the Developer COMPORT.
*/
void unknown_command(uint16_t command, uint8_t cmd_type){
    if (cmd_type == 0){                                                                 // check for command type i.e. Emulation, Datalogging, Developer commands.
        print("Unknown Command (Emulation): ", command, true);                          // print command to Developer COMPORT and where it came from
    }    
    if (cmd_type == 1){                                                                 // check for command type i.e. Emulation, Datalogging, Developer commands.
        print("Unknown Command (DataLogging): ", command, true);                        // print command to Developer COMPORT and where it came from
    }
    if (cmd_type == 2){                                                                 // check for command type i.e. Emulation, Datalogging, Developer commands.
        print("Unknown Command (developer_command): ", command, true);                  // print command to Developer COMPORT and where it came from
    }
}

/*
Performs pattern matching to the struct command/function duo.
*/
bool search_command(Command *command_list, uint8_t *command, uint16_t key){             // Try to execute the command found in buffer
    for (size_t index = 0; command_list[index].function != NULL; index++){              // Loop through the command structure until Null
        if (command_list[index].command == key) {                                       // Check the command we have to the command we recieved
            command_list[index].function(command);                                      // if the commands match execute the corresponding fuction. (passing along command)
            return true;}}                                                              // return true that we had success
    return false;                                                                       // Otherwise if nothing was executed, we dont know that command
}

/*
Begins the execution of command data through string parsing, up to 2 bytes.
*/
uint16_t execute_command(Command* command_list,uint8_t* command){                       // try to execute the command found in buffer
    uint16_t two_key = ((command[0] << 8) | command[1]);                                // Concat start byte with end byte
    uint16_t one_key = ((command[0] << 8) | 0x00);                                      // Concat start byte with no byte
    if (!two_key){return 0;}                                                            // check for all zero key two, return if its all zeros nothing will execute
    print("Command: ", (uint32_t)two_key, true);                                        // prints out the key to Developer COMPORT
    if (!search_command(command_list, command, one_key))                                // if its not key one it must be key two: if its key one execute command  // try to execute the command found in buffer
    {if (!search_command(command_list, command, two_key)){return two_key;}}             // if its not key two then: if its key two execute command  // try to execute the command found in buffer
    return 0;                                                                           // return zero as nothing was found
}

/*
Initalizes ostrich protocol emulation.
Performs the main subroutine of core 0.
*/
void ostrich_init(){
    tusb_init();                                                                        // Call tusb_init (very! very! very! important as well as calling tud_task() or consequeses will be lock ups)
    datalog_init();                                                                     // Perform an initialization for the UART for Datalogging   
    uint16_t error;                                                                     // Create error variable for error checking later
    uint8_t log_cmd[2];                                                                 // 1 byte for Datalog command processing
    uint8_t dev_cmd[2];                                                                 // 2 bytes for Developer command processing
    uint8_t* command = malloc(8192);                                                    // Cut out dynamic memory for the command: making sure it byte aligned <- this
    Command* command_list = malloc(64);                                                 // Cut out memory for Command Structure current size is (12 * 3 = 36)
    //initialize_pins();                                                                // Call initalize pins here 
    // seting up the Command Struct with its command/function pair
    command_list[0] =  (Command){CMD_VV, post_version}; 
    command_list[1] =  (Command){CMD_Nx, deploy_nx}; 
    command_list[2] =  (Command){CMD_FF, post_vendor}; 
    command_list[3] =  (Command){CMD_Bx, deploy_bx};   
    command_list[4] =  (Command){CMD_Rx, micro_read};   
    command_list[5] =  (Command){CMD_Wx, micro_write};  
    command_list[6] =  (Command){CMD_ZR, bulk_read};   
    command_list[7] =  (Command){CMD_ZW, bulk_write}; 
    command_list[8] =  (Command){CMD_DS, start_log};   
    command_list[9] =  (Command){CMD_F1, set_clean};
    command_list[10] = (Command){CMD_F2, set_reset};
    command_list[11] = (Command){CMD_RT, start_log};
    command_list[12] = (Command){CMD_DR, read_and_forward};
    command_list[13] = (Command){CMD_DM, read_and_forward};
    command_list[14] = (Command){NUL_BY, NULL};

    while (1){
        if (connected && !(upload_count % 8)){                                          // recognize we are connected then write RAM and close.
            bulk_update_mutexes();                                                      // go to dupicate binary to master and set connected true.
            connected = false;                                                          // set connection false so we do not keep writing to RAM
        }
        read_bytes(command, 0, 2);                                                      // read bytes and put then into command pointer
        error = execute_command(command_list, command);                                 // try to execute the command found in buffer
        if (error){unknown_command(error, 0);}                                          // send the command to Developer console if unknown

        datalog_get_request(log_cmd);                                                   // read bytes for datalog command
        error = execute_command(command_list, log_cmd);                                 // try to execute the command found in buffer
        if (error){unknown_command(error, 1);}                                          // send the command to Developer console if unknown

        if (DEVELOPER_CONSOLE){
            developer_get_request(dev_cmd);                                             // read bytes for dev-log command
            error = execute_command(command_list, dev_cmd);                             // try to execute the command found in buffer
            if (error){unknown_command(error, 2);}                                      // send the command to Developer console if unknown
        }
        memset(command, 0, 8192);                                                       // reset Ostrich command when done
        memset(log_cmd, 0, 2);                                                          // reset Datalog command when done
        memset(dev_cmd, 0, 2);                                                          // reset Developer command when done
        sleep_us(200);                                                                  // Prevents mutex contention with core 1
    }
}

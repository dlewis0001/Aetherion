/*
*        SPDX-License-Identifier: BSD-3-Clause
*
*        Copyright (c) 2025, Dennis B. Lewis
*        All rights reserved.
*        This file contains modifications to software originally licensed under the
*        BSD-3-Clause license by the Raspberry Pi Foundation.
*        See LEGAL.TXT in the root directory of this project for more details.
*/
#include "tusb.h"
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "developer_tools.h"
/*
Developer print function to check data in the developer COMPORT.
Is unused when DEVELOPER_CONSOLE is false.
*/

void print(char* message, int32_t value, bool hex){
    if (!DEVELOPER_CONSOLE){return;}
    char buffer[128];
    if (value == -1 && message != ""){                                                  // For string messages only
        snprintf(buffer, sizeof(buffer), "%s\r\n", message);                            // format printable data together
        tud_cdc_n_write(2, buffer, strlen(buffer));                                     // Write data for output
        tud_cdc_n_write_flush(2);                                                       // Flush to tuning software
        return;
    }
    if (message == "" && value != -1 && hex){                                           // for only hex values
        snprintf(buffer, sizeof(buffer), "0x%X\r\n", value);                            // format printable data together
        tud_cdc_n_write(2, buffer, strlen(buffer));                                     // Write data for output
        tud_cdc_n_write_flush(2);                                                       // Flush to tuning software
        return;
    }
    if (message != "" && value != -1 && hex){                                           // for string and hex value -> "myvalue: 0x01"
        snprintf(buffer, sizeof(buffer), "%s0x%X\r\n", message, value);                 // format printable data together
        tud_cdc_n_write(2, buffer, strlen(buffer));                                     // Write data for output
        tud_cdc_n_write_flush(2);                                                       // Flush to tuning software
        return;
    }
    if (message == "" && value != -1 && !hex){                                          // only for int values
        snprintf(buffer, sizeof(buffer), "%d\r\n", value);                              // format printable data together
        tud_cdc_n_write(2, buffer, strlen(buffer));                                     // Write data for output
        tud_cdc_n_write_flush(2);                                                       // Flush to tuning software
        return;
    }
    if (message != "" && value != -1 && !hex){                                          // only for string and int values (not hex)
        snprintf(buffer, sizeof(buffer), "%s%d\r\n", message, value);                   // format printable data together
        tud_cdc_n_write(2, buffer, strlen(buffer));                                     // Write data for output
        tud_cdc_n_write_flush(2);                                                       // Flush to tuning software
        return;
    }
}
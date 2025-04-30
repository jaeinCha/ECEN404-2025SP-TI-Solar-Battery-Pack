/*
 * Copyright (c) 2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef BQ25622_DRIVER_H_
#define BQ25622_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "ti_msp_dl_config.h" // Include your MSP header file for delay_cycles

// BQ25622 initialization
void BQ25622_Init(void);

// Functionality 1: Change charge voltage and current
void BQ25622_SetChargeVoltage(uint16_t voltage_mv);
void BQ25622_SetChargeCurrent(uint16_t current_ma);

// Functionality 2: Change input voltage and current
void BQ25622_SetInputVoltageLimit(uint16_t voltage_mv);
void BQ25622_SetInputCurrentLimit(uint16_t current_ma);

// Functionality 3: Change OTG voltage and current
void BQ25622_SetOTGVoltage(uint16_t voltage_mv);
void BQ25622_SetOTGCurrent(uint16_t current_ma);

// Register control functions for OTG mode
void BQ25622_DisableTermination(void);
void BQ25622_EnableTermination(void);
void BQ25622_DisableWatchdog(void);
void BQ25622_DisableILIM(void);

// Additional utility functions
void BQ25622_SetMinimumSystemVoltage(uint16_t voltage_mv);
void BQ25622_EnableCharging(bool enable);
void BQ25622_EnableOTG(bool enable);
void BQ25622_ConfigureForUSBC(void);
uint8_t BQ25622_GetChargingStatus(void);
void BQ25622_InitWithUSBC(void);

// Helper functions
void delayUS(uint16_t us);
unsigned char Checksum(unsigned char *ptr, unsigned char len);

// Charging status values
#define BQ25622_CHARGE_STATUS_NOT_CHARGING       0x00
#define BQ25622_CHARGE_STATUS_TRICKLE_PRECHARGE  0x01
#define BQ25622_CHARGE_STATUS_FAST_CHARGE        0x02
#define BQ25622_CHARGE_STATUS_CHARGE_TERMINATION 0x03

#endif /* BQ25622_DRIVER_H_ */
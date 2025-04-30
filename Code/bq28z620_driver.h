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

#ifndef BQ28Z620_DRIVER_H_
#define BQ28Z620_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include "ti_msp_dl_config.h" // Include your MSP header file for delay_cycles

// BQ28Z620 I2C Device Address (0x55 in 7-bit format)
#define BQ28Z620_I2C_ADDR 0x55

// Battery charging state values
typedef enum {
    IDLE,
    CHARGING,
    DISCHARGING
} BQ28Z620_ChargingState;

// Battery status structure
typedef struct {
    int16_t voltage;             // Battery voltage in mV
    int16_t current;             // Battery current in mA (+ charging, - discharging)
    float temperature;           // Battery temperature in °C
    uint16_t relativeSOC;        // Relative state of charge in %
    uint16_t absoluteSOC;        // Absolute state of charge in %
    uint16_t timeToEmpty;        // Estimated time to empty in minutes
    uint16_t timeToFull;         // Estimated time to full in minutes
    uint16_t remainingCapacity;  // Remaining capacity in mAh
    uint16_t fullChargeCapacity; // Full charge capacity in mAh
    uint16_t flags;              // Status flags
    BQ28Z620_ChargingState chargingState; // Charging state
} BQ28Z620_BatteryStatus;

// Function prototypes

// Initialize the BQ28Z620
void BQ28Z620_Init(void);

// Read battery measurements
int16_t BQ28Z620_ReadVoltage(void);
int16_t BQ28Z620_ReadCurrent(void);
int16_t BQ28Z620_ReadTemperature(void);
float BQ28Z620_ReadTemperatureC(void);
uint16_t BQ28Z620_ReadRelativeSOC(void);
uint16_t BQ28Z620_ReadAbsoluteSOC(void);
uint16_t BQ28Z620_ReadTimeToEmpty(void);
uint16_t BQ28Z620_ReadTimeToFull(void);
uint16_t BQ28Z620_ReadRemainingCapacity(void);
uint16_t BQ28Z620_ReadFullChargeCapacity(void);
uint16_t BQ28Z620_ReadFlags(void);

// Get complete battery status
BQ28Z620_BatteryStatus BQ28Z620_GetBatteryStatus(void);

// Print battery status (implementation depends on your output method)
void BQ28Z620_PrintBatteryStatus(BQ28Z620_BatteryStatus status);

// Helper functions
void BQ28Z620_delayUS(uint16_t us);

#endif /* BQ28Z620_DRIVER_H_ */

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

#include "BQ28Z620_driver.h"
#include "I2C_communication.h"  // Use existing I2C communication functions

// Arrays for I2C data
unsigned char BQ28Z620_RX_data[2] = {0x00};

// BQ28Z620 command codes (from Table 14-1)
#define CMD_CONTROL          0x00
#define CMD_ATRATE           0x02
#define CMD_ATRATE_TIME_TO_EMPTY 0x04
#define CMD_TEMPERATURE      0x06
#define CMD_VOLTAGE          0x08
#define CMD_FLAGS            0x0A
#define CMD_CURRENT          0x0C
#define CMD_FULL_CHARGE_CAPACITY 0x10
#define CMD_AVERAGE_CURRENT  0x14
#define CMD_TIME_TO_EMPTY    0x16
#define CMD_TIME_TO_FULL     0x18
#define CMD_STANDBY_CURRENT  0x1A
#define CMD_STANDBY_TIME_TO_EMPTY 0x1C
#define CMD_MAX_LOAD_CURRENT 0x1E
#define CMD_MAX_LOAD_TIME_TO_EMPTY 0x20
#define CMD_RELATIVE_SOC     0x2C
#define CMD_ABSOLUTE_SOC     0x2E
#define CMD_REMAINING_CAPACITY 0x10
#define CMD_FULL_CHARGE_CAPACITY 0x12

// Function to delay in microseconds
void BQ28Z620_delayUS(uint16_t us)
{
    uint16_t ms;
    char i;
    ms = us / 1000;
    for (i = 0; i < ms; i++) delay_cycles(32000);
}

// Initialize the BQ28Z620
void BQ28Z620_Init()
{
    // No specific initialization required for BQ28Z620
    // The device is ready for communication after power-up
}

// Read battery voltage in mV
int16_t BQ28Z620_ReadVoltage()
{
    // Set I2C target address to BQ28Z620
    I2C_SetTargetAddress(BQ28Z620_I2C_ADDR);
    
    // Read the voltage register (0x08)
    I2C_ReadReg(CMD_VOLTAGE, BQ28Z620_RX_data, 2);
    BQ28Z620_delayUS(2000);
    
    // Combine bytes to get voltage in mV
    int16_t voltage = (int16_t)((BQ28Z620_RX_data[1] << 8) | BQ28Z620_RX_data[0]);
    
    return voltage;
}

// Read battery current in mA (positive for charge, negative for discharge)
int16_t BQ28Z620_ReadCurrent()
{
    // Set I2C target address to BQ28Z620
    I2C_SetTargetAddress(BQ28Z620_I2C_ADDR);
    
    // Read the current register (0x0C)
    I2C_ReadReg(CMD_CURRENT, BQ28Z620_RX_data, 2);
    BQ28Z620_delayUS(2000);
    
    // Combine bytes to get current in mA (signed value)
    int16_t current = (int16_t)((BQ28Z620_RX_data[1] << 8) | BQ28Z620_RX_data[0]);
    
    return current;
}

// Read battery temperature in 0.1°K (subtract 273.15 to get Celsius)
int16_t BQ28Z620_ReadTemperature()
{
    // Set I2C target address to BQ28Z620
    I2C_SetTargetAddress(BQ28Z620_I2C_ADDR);
    
    // Read the temperature register (0x06)
    I2C_ReadReg(CMD_TEMPERATURE, BQ28Z620_RX_data, 2);
    BQ28Z620_delayUS(2000);
    
    // Combine bytes to get temperature in 0.1°K
    int16_t temperature = (int16_t)((BQ28Z620_RX_data[1] << 8) | BQ28Z620_RX_data[0]);
    
    return temperature;
}

// Read battery temperature in Celsius
float BQ28Z620_ReadTemperatureC()
{
    int16_t temp_kelvin_x10 = BQ28Z620_ReadTemperature();
    float temp_celsius = (float)temp_kelvin_x10 / 10.0f - 273.15f;
    
    return temp_celsius;
}

// Read relative state of charge (%)
uint16_t BQ28Z620_ReadRelativeSOC()
{
    // Set I2C target address to BQ28Z620
    I2C_SetTargetAddress(BQ28Z620_I2C_ADDR);
    
    // Read the relative SOC register (0x2C)
    I2C_ReadReg(CMD_RELATIVE_SOC, BQ28Z620_RX_data, 2);
    BQ28Z620_delayUS(2000);
    
    // Combine bytes to get SOC in %
    uint16_t soc = (uint16_t)((BQ28Z620_RX_data[1] << 8) | BQ28Z620_RX_data[0]);
    
    return soc;
}

// Read absolute state of charge (%)
uint16_t BQ28Z620_ReadAbsoluteSOC()
{
    // Set I2C target address to BQ28Z620
    I2C_SetTargetAddress(BQ28Z620_I2C_ADDR);
    
    // Read the absolute SOC register (0x2E)
    I2C_ReadReg(CMD_ABSOLUTE_SOC, BQ28Z620_RX_data, 2);
    BQ28Z620_delayUS(2000);
    
    // Combine bytes to get SOC in %
    uint16_t soc = (uint16_t)((BQ28Z620_RX_data[1] << 8) | BQ28Z620_RX_data[0]);
    
    return soc;
}

// Read time to empty in minutes
uint16_t BQ28Z620_ReadTimeToEmpty()
{
    // Set I2C target address to BQ28Z620
    I2C_SetTargetAddress(BQ28Z620_I2C_ADDR);
    
    // Read the time to empty register (0x16)
    I2C_ReadReg(CMD_TIME_TO_EMPTY, BQ28Z620_RX_data, 2);
    BQ28Z620_delayUS(2000);
    
    // Combine bytes to get time in minutes
    uint16_t time = (uint16_t)((BQ28Z620_RX_data[1] << 8) | BQ28Z620_RX_data[0]);
    
    return time;
}

// Read time to full in minutes
uint16_t BQ28Z620_ReadTimeToFull()
{
    // Set I2C target address to BQ28Z620
    I2C_SetTargetAddress(BQ28Z620_I2C_ADDR);
    
    // Read the time to full register (0x18)
    I2C_ReadReg(CMD_TIME_TO_FULL, BQ28Z620_RX_data, 2);
    BQ28Z620_delayUS(2000);
    
    // Combine bytes to get time in minutes
    uint16_t time = (uint16_t)((BQ28Z620_RX_data[1] << 8) | BQ28Z620_RX_data[0]);
    
    return time;
}

// Read remaining capacity in mAh
uint16_t BQ28Z620_ReadRemainingCapacity()
{
    // Set I2C target address to BQ28Z620
    I2C_SetTargetAddress(BQ28Z620_I2C_ADDR);
    
    // Read the remaining capacity register (0x10)
    I2C_ReadReg(CMD_REMAINING_CAPACITY, BQ28Z620_RX_data, 2);
    BQ28Z620_delayUS(2000);
    
    // Combine bytes to get capacity in mAh
    uint16_t capacity = (uint16_t)((BQ28Z620_RX_data[1] << 8) | BQ28Z620_RX_data[0]);
    
    return capacity;
}

// Read full charge capacity in mAh
uint16_t BQ28Z620_ReadFullChargeCapacity()
{
    // Set I2C target address to BQ28Z620
    I2C_SetTargetAddress(BQ28Z620_I2C_ADDR);
    
    // Read the full charge capacity register (0x12)
    I2C_ReadReg(CMD_FULL_CHARGE_CAPACITY, BQ28Z620_RX_data, 2);
    BQ28Z620_delayUS(2000);
    
    // Combine bytes to get capacity in mAh
    uint16_t capacity = (uint16_t)((BQ28Z620_RX_data[1] << 8) | BQ28Z620_RX_data[0]);
    
    return capacity;
}

// Read battery status flags
uint16_t BQ28Z620_ReadFlags()
{
    // Set I2C target address to BQ28Z620
    I2C_SetTargetAddress(BQ28Z620_I2C_ADDR);
    
    // Read the flags register (0x0A)
    I2C_ReadReg(CMD_FLAGS, BQ28Z620_RX_data, 2);
    BQ28Z620_delayUS(2000);
    
    // Combine bytes to get flags
    uint16_t flags = (uint16_t)((BQ28Z620_RX_data[1] << 8) | BQ28Z620_RX_data[0]);
    
    return flags;
}

// Get battery status information
BQ28Z620_BatteryStatus BQ28Z620_GetBatteryStatus()
{
    BQ28Z620_BatteryStatus status;
    
    // Read all relevant data
    status.voltage = BQ28Z620_ReadVoltage();
    status.current = BQ28Z620_ReadCurrent();
    status.temperature = BQ28Z620_ReadTemperatureC();
    status.relativeSOC = BQ28Z620_ReadRelativeSOC();
    status.absoluteSOC = BQ28Z620_ReadAbsoluteSOC();
    status.timeToEmpty = BQ28Z620_ReadTimeToEmpty();
    status.timeToFull = BQ28Z620_ReadTimeToFull();
    status.remainingCapacity = BQ28Z620_ReadRemainingCapacity();
    status.fullChargeCapacity = BQ28Z620_ReadFullChargeCapacity();
    status.flags = BQ28Z620_ReadFlags();
    
    // Process the flags to determine charging status
    if (status.current > 0) {
        status.chargingState = CHARGING;
    } else if (status.current < 0) {
        status.chargingState = DISCHARGING;
    } else {
        status.chargingState = IDLE;
    }
    
    return status;
}

// Print battery status information
void BQ28Z620_PrintBatteryStatus(BQ28Z620_BatteryStatus status)
{
    // This function would be implemented based on your output method
    // For example, using printf or UART or other display mechanism
    
    // Example implementation assuming printf is available:
    /*
    printf("Battery Status:\n");
    printf("Voltage: %d mV\n", status.voltage);
    printf("Current: %d mA\n", status.current);
    printf("Temperature: %.1f °C\n", status.temperature);
    printf("Relative SOC: %d %%\n", status.relativeSOC);
    printf("Absolute SOC: %d %%\n", status.absoluteSOC);
    printf("Time to Empty: %d minutes\n", status.timeToEmpty);
    printf("Time to Full: %d minutes\n", status.timeToFull);
    printf("Remaining Capacity: %d mAh\n", status.remainingCapacity);
    printf("Full Charge Capacity: %d mAh\n", status.fullChargeCapacity);
    printf("Charging State: %s\n", 
           (status.chargingState == CHARGING) ? "Charging" : 
           (status.chargingState == DISCHARGING) ? "Discharging" : "Idle");
    */
}

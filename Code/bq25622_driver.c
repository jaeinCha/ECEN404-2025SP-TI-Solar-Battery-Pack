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

#include "bq25622_driver.h"
#include "I2C_communication.h"  // Use existing I2C communication functions

//******************************************************************************
// BQ Parameters ***************************************************************
//******************************************************************************
// Global Variables
uint16_t AlarmBits       = 0x00;

// Arrays for I2C data
unsigned char RX_32Byte[32] = {0x00};
unsigned char RX_data[2]    = {0x00};
uint8_t TX_Buffer[4]        = {0x00}; // Used for storing data to write

// Register addresses
#define REG_CHARGE_CURRENT     0x02
#define REG_CHARGE_VOLTAGE     0x04
#define REG_INPUT_CURRENT      0x06
#define REG_INPUT_VOLTAGE      0x08
#define REG_OTG_CURRENT        0x0A
#define REG_OTG_VOLTAGE        0x0C
#define REG_MIN_SYSTEM_VOLTAGE 0x0E
#define REG_PRECHARGE_CONTROL  0x10
#define REG_TERMINATION_CONTROL 0x12
#define REG_CHARGE_CONTROL_0   0x14
#define REG_CHARGER_CONTROL_1  0x16
#define REG_CHARGER_CONTROL_2  0x17
#define REG_CHARGER_CONTROL_3  0x18
#define REG_CHARGER_CONTROL_4  0x19
#define REG_CHARGE_STATUS_1    0x1E

// Constants for operations
#define R 0  // Read operation
#define W 1  // Write operation

//**********************************Function prototypes **********************************
void delayUS(uint16_t us)
{  // Sets the delay in microseconds.
    uint16_t ms;
    char i;
    ms = us / 1000;
    for (i = 0; i < ms; i++) delay_cycles(32000);
}

// Initialize the BQ25622
void BQ25622_Init()
{
    // Note: Need to update I2C_TARGET_ADDRESS in I2C_communication.h to 0x6B for BQ25622
    
    // Set default configuration values
    
    // Set minimum system voltage to 3.52V (default)
    BQ25622_SetMinimumSystemVoltage(3520);
    
    // Set default charging parameters
    BQ25622_SetChargeVoltage(4200); // 4.2V
    BQ25622_SetChargeCurrent(1000); // 1A
    
    // Set default input limits
    BQ25622_SetInputVoltageLimit(4600); // 4.6V
    BQ25622_SetInputCurrentLimit(2000); // 2A
    
    // Enable charging
    BQ25622_EnableCharging(true);
}

// Functionality 1: Change charge voltage and current
// Set the battery charge voltage (in mV, range 3500-4800mV)
void BQ25622_SetChargeVoltage(uint16_t voltage_mv)
{
    // Clamp the value to the allowed range
    if (voltage_mv < 3500) voltage_mv = 3500;
    if (voltage_mv > 4800) voltage_mv = 4800;
    
    // Convert voltage to register value
    // VREG register uses 10mV steps (from datasheet)
    uint16_t reg_value = (voltage_mv - 3500) / 10;
    
    // Write to Charge Voltage Limit register
    uint8_t TX_data[2] = {0x00, 0x00};
    
    // Little endian format
    TX_data[0] = reg_value & 0xff;         // LSB
    TX_data[1] = (reg_value >> 8) & 0xff;  // MSB
    
    // Write to register 0x04 (Charge Voltage Limit)
    I2C_WriteReg(REG_CHARGE_VOLTAGE, TX_data, 2);
    delayUS(2000);
}

// Set the battery charge current (in mA, range 80-3520mA)
void BQ25622_SetChargeCurrent(uint16_t current_ma)
{
    // Clamp the value to the allowed range
    if (current_ma < 80) current_ma = 80;
    if (current_ma > 3520) current_ma = 3520;
    
    // Convert current to register value
    // ICHG register uses 80mA steps (from datasheet)
    uint16_t reg_value = current_ma / 80;
    
    // Write to Charge Current Limit register
    uint8_t TX_data[2] = {0x00, 0x00};
    
    // Little endian format
    TX_data[0] = reg_value & 0xff;         // LSB
    TX_data[1] = (reg_value >> 8) & 0xff;  // MSB
    
    // Write to register 0x02 (Charge Current Limit)
    I2C_WriteReg(REG_CHARGE_CURRENT, TX_data, 2);
    delayUS(2000);
}

// Functionality 2: Change input voltage and current
// Set the input voltage limit (in mV, range 3800-16800mV)
void BQ25622_SetInputVoltageLimit(uint16_t voltage_mv)
{
    // Clamp the value to the allowed range
    if (voltage_mv < 3800) voltage_mv = 3800;
    if (voltage_mv > 16800) voltage_mv = 16800;
    
    // Convert voltage to register value
    // VINDPM register uses 40mV steps (from datasheet)
    uint16_t reg_value = (voltage_mv - 3800) / 40;
    
    // Write to Input Voltage Limit register
    uint8_t TX_data[2] = {0x00, 0x00};
    
    // Little endian format
    TX_data[0] = reg_value & 0xff;         // LSB
    TX_data[1] = (reg_value >> 8) & 0xff;  // MSB
    
    // Write to register 0x08 (Input Voltage Limit)
    I2C_WriteReg(REG_INPUT_VOLTAGE, TX_data, 2);
    delayUS(2000);
}

// Set the input current limit (in mA, range 100-3200mA)
void BQ25622_SetInputCurrentLimit(uint16_t current_ma)
{
    // Clamp the value to the allowed range
    if (current_ma < 100) current_ma = 100;
    if (current_ma > 3200) current_ma = 3200;
    
    // Convert current to register value
    // IINDPM register uses 20mA steps (from datasheet)
    uint16_t reg_value = current_ma / 20;
    
    // Write to Input Current Limit register
    uint8_t TX_data[2] = {0x00, 0x00};
    
    // Little endian format
    TX_data[0] = reg_value & 0xff;         // LSB
    TX_data[1] = (reg_value >> 8) & 0xff;  // MSB
    
    // Write to register 0x06 (Input Current Limit)
    I2C_WriteReg(REG_INPUT_CURRENT, TX_data, 2);
    delayUS(2000);
}

// Functionality 3: Change OTG voltage and current
// Set the OTG voltage (in mV, range 3840-9600mV)
void BQ25622_SetOTGVoltage(uint16_t voltage_mv)
{
    // Clamp the value to the allowed range
    if (voltage_mv < 3840) voltage_mv = 3840;
    if (voltage_mv > 9600) voltage_mv = 9600;
    
    // Convert voltage to register value
    // VOTG register uses 80mV steps (from datasheet)
    uint16_t reg_value = (voltage_mv - 3840) / 80;
    
    // Write to OTG Voltage register
    uint8_t TX_data[2] = {0x00, 0x00};
    
    // Little endian format
    TX_data[0] = reg_value & 0xff;         // LSB
    TX_data[1] = (reg_value >> 8) & 0xff;  // MSB
    
    // Write to register 0x0C (OTG Voltage Regulation)
    I2C_WriteReg(REG_OTG_VOLTAGE, TX_data, 2);
    delayUS(2000);
}

// Set the OTG current limit (in mA, range 100-2400mA)
void BQ25622_SetOTGCurrent(uint16_t current_ma)
{
    // Clamp the value to the allowed range
    if (current_ma < 100) current_ma = 100;
    if (current_ma > 2400) current_ma = 2400;
    
    // Convert current to register value
    // IOTG register uses 20mA steps (from datasheet)
    uint16_t reg_value = current_ma / 20;
    
    // Write to OTG Current register
    uint8_t TX_data[2] = {0x00, 0x00};
    
    // Little endian format
    TX_data[0] = reg_value & 0xff;         // LSB
    TX_data[1] = (reg_value >> 8) & 0xff;  // MSB
    
    // Write to register 0x0A (OTG Current regulation)
    I2C_WriteReg(REG_OTG_CURRENT, TX_data, 2);
    delayUS(2000);
}

// Additional functions
// Set the minimum system voltage (in mV, range 2560-3840mV)
void BQ25622_SetMinimumSystemVoltage(uint16_t voltage_mv)
{
    // Clamp the value to the allowed range
    if (voltage_mv < 2560) voltage_mv = 2560;
    if (voltage_mv > 3840) voltage_mv = 3840;
    
    // Convert voltage to register value
    // VSYSMIN register uses 80mV steps (from datasheet)
    uint16_t reg_value = (voltage_mv - 2560) / 80;
    
    // Write to Minimum System Voltage register
    uint8_t TX_data[2] = {0x00, 0x00};
    
    // Little endian format
    TX_data[0] = reg_value & 0xff;         // LSB
    TX_data[1] = (reg_value >> 8) & 0xff;  // MSB
    
    // Write to register 0x0E (Minimum System Voltage)
    I2C_WriteReg(REG_MIN_SYSTEM_VOLTAGE, TX_data, 2);
    delayUS(2000);
}

// Disable termination (bit 2 of 0x14)
void BQ25622_DisableTermination()
{
    // Read current register value
    I2C_ReadReg(REG_CHARGE_CONTROL_0, RX_data, 2);
    
    // Clear EN_TERM bit (bit 2)
    RX_data[0] &= ~(1 << 2); // Clear bit 2
    
    // Write back to register
    I2C_WriteReg(REG_CHARGE_CONTROL_0, RX_data, 2);
    delayUS(2000);
}

// Enable termination
void BQ25622_EnableTermination()
{
    // Read current register value
    I2C_ReadReg(REG_CHARGE_CONTROL_0, RX_data, 2);
    
    // Set EN_TERM bit (bit 2)
    RX_data[0] |= (1 << 2); // Set bit 2
    
    // Write back to register
    I2C_WriteReg(REG_CHARGE_CONTROL_0, RX_data, 2);
    delayUS(2000);
}

// Disable watchdog timer (bits 1:0 of 0x16)
void BQ25622_DisableWatchdog()
{
    // Read current register value
    I2C_ReadReg(REG_CHARGER_CONTROL_1, RX_data, 2);
    
    // Clear WATCHDOG bits (bits 1:0)
    RX_data[0] &= ~(0x03); // Clear bits 1:0
    
    // Write back to register
    I2C_WriteReg(REG_CHARGER_CONTROL_1, RX_data, 2);
    delayUS(2000);
}

// Disable external ILIM pin (bit 2 of 0x19)
void BQ25622_DisableILIM()
{
    // Read current register value
    I2C_ReadReg(REG_CHARGER_CONTROL_4, RX_data, 2);
    
    // Clear EN_EXTILIM bit (bit 2)
    RX_data[0] &= ~(1 << 2); // Clear bit 2
    
    // Write back to register
    I2C_WriteReg(REG_CHARGER_CONTROL_4, RX_data, 2);
    delayUS(2000);
}

// Enable/disable charging
void BQ25622_EnableCharging(bool enable)
{
    // Read current register value
    I2C_ReadReg(REG_CHARGER_CONTROL_1, RX_data, 2);
    
    // Modify EN_CHG bit (bit 5)
    if (enable) {
        RX_data[0] |= (1 << 5);  // Set bit 5
    } else {
        RX_data[0] &= ~(1 << 5); // Clear bit 5
    }
    
    // Write back to register
    I2C_WriteReg(REG_CHARGER_CONTROL_1, RX_data, 2);
    delayUS(2000);
}

// Enable/disable OTG mode
void BQ25622_EnableOTG(bool enable)
{
    if (enable) {
        // Per datasheet, must disable termination, disable watchdog, and disable ILIM
        BQ25622_DisableTermination();
        BQ25622_DisableWatchdog();
        BQ25622_DisableILIM();
        
        // Read current register value
        I2C_ReadReg(REG_CHARGER_CONTROL_3, RX_data, 2);
        
        // Set EN_OTG bit (bit 6)
        RX_data[0] |= (1 << 6);  // Set bit 6
        
        // Write back to register
        I2C_WriteReg(REG_CHARGER_CONTROL_3, RX_data, 2);
    } else {
        // Read current register value
        I2C_ReadReg(REG_CHARGER_CONTROL_3, RX_data, 2);
        
        // Clear EN_OTG bit (bit 6)
        RX_data[0] &= ~(1 << 6); // Clear bit 6
        
        // Write back to register
        I2C_WriteReg(REG_CHARGER_CONTROL_3, RX_data, 2);
        
        // Re-enable termination if needed (depends on your application)
        // BQ25622_EnableTermination();
    }
    delayUS(2000);
}

// Configure for USB-C (5V, 2.5A)
void BQ25622_ConfigureForUSBC()
{
    // Set OTG voltage to 5V
    BQ25622_SetOTGVoltage(5000);
    
    // Set OTG current to 2.4A (closest to 2.5A)
    BQ25622_SetOTGCurrent(2400);
    
    // Enable OTG mode (which will also disable termination, watchdog, and ILIM)
    BQ25622_EnableOTG(true);
}

// Get the battery charging status
uint8_t BQ25622_GetChargingStatus()
{
    // Read the Charger Status 1 register
    I2C_ReadReg(REG_CHARGE_STATUS_1, RX_data, 2);
    
    // Extract CHG_STAT field (bits 4:3)
    return (RX_data[0] >> 3) & 0x03;
}

// Initialize with USB-C configuration
void BQ25622_InitWithUSBC()
{
    BQ25622_Init();
    BQ25622_ConfigureForUSBC();
}
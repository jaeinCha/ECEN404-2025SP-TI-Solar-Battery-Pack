/*
 * Copyright (c) 2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Comprehensive battery management system implementation for:
 * - BQ25622 battery charger
 * - BQ28Z620 battery gauge
 * - TUSB320 USB-C port controller
 * - Solar panel with MPPT
 * - Battery SOC display
 */

#include "BQ25622_driver.h"
#include "BQ28Z620_driver.h"
#include "I2C_communication.h"
#include "ti_msp_dl_config.h"
#include <stdbool.h>

/* Define GPIO pins */
#define TUSB320_INT_PIN         DL_GPIO_PIN_28  // PA28 - TUSB320 interrupt pin
#define USB_C_PG_PIN            DL_GPIO_PIN_13  // PB13 - Input power good for USB C
#define POWER_MUX_SEL_PIN       DL_GPIO_PIN_14  // PB14 - Power MUX select pin
#define SOLAR_PG_PIN            DL_GPIO_PIN_15  // PB15 - Input power good for solar panel
#define SOC_DISPLAY_BUTTON_PIN  DL_GPIO_PIN_0   // PA0 - SOC display wake button
#define SOC_LED_PINS            (DL_GPIO_PIN_7 | DL_GPIO_PIN_8 | DL_GPIO_PIN_9 | DL_GPIO_PIN_10 | DL_GPIO_PIN_11)

/* Define charging modes */
typedef enum {
    CHARGING_OFF,
    USB_C_SOURCE_MODE,   // Connected device is drawing power
    USB_C_SINK_MODE,     // Device is being charged from USB C
    SOLAR_CHARGING_MODE  // Device is being charged from solar panel
} ChargingMode_t;

/* Define safety thresholds */
#define MAX_CHARGING_POWER_W   15.0f     // Max charging power (W)
#define MAX_BATTERY_TEMP_C     60.0f     // Max battery temperature (°C)
#define MIN_CHARGING_CURRENT   50        // Min charging current to maintain charging (mA)
#define FULL_CHARGE_SOC        97        // SOC threshold for full charge (%)
#define MPPT_PERTURB_STEP      50        // MPPT current adjustment step (mA)
#define MPPT_INTERVAL          5000000   // MPPT algorithm interval (cycles)
#define LED_DISPLAY_DURATION   300000000 // LED display duration (cycles)

/* TUSB320 I2C definitions */
#define TUSB320_I2C_ADDR       0x47      // TUSB320 I2C address
#define TUSB320_REG_CURRENT_MODE 0x0A    // Current mode register
#define TUSB320_DFP_MASK       0x04      // Device is providing power (Source)
#define TUSB320_UFP_MASK       0x08      // Device is consuming power (Sink)

/* Global variables */
ChargingMode_t gCurrentChargingMode = CHARGING_OFF;
bool gPreviousButtonState = false;
uint32_t gMpptCounter = 0;
uint16_t gCurrentChargeCurrent = 0;
uint16_t gPreviousPower = 0;
bool gIncreasingCurrent = true;

/* Function prototypes */
void initGPIO(void);
void processUSBC_Interrupt(void);
void processUSBC_PowerGood(void);
void processSolarPowerGood(void);
void processSOCDisplayButton(void);
void setSOCLEDDisplay(uint16_t soc);
void stopCharging(void);
void readAndCheckBatteryStatus(void);
bool isBatteryChargingSafe(void);
void executeMPPT(void);
bool readTUSB320Mode(bool *isSource, bool *isSink);

/*
 * Main application
 */
int main(void)
{
    /* Initialize the system components */
    SYSCFG_DL_init();
    initGPIO();
    
    /* Set I2C target address to BQ25622 charger and initialize */
    I2C_SetTargetAddress(I2C_BQCHARGER_ADDR);
    BQ25622_Init();
    
    /* Set I2C target address to BQ28Z620 gauge and initialize */
    I2C_SetTargetAddress(I2C_BQGAUGE_ADDR);
    BQ28Z620_Init();
    
    /* Main application loop */
    while(1) {
        /* Process TUSB320 interrupt (USB-C device insertion/mode change) */
        if (DL_GPIO_readPins(GPIOA, TUSB320_INT_PIN)) {
            processUSBC_Interrupt();
        }
        
        /* Check USB-C power good pin */
        processUSBC_PowerGood();
        
        /* Check solar power good pin */
        processSolarPowerGood();
        
        /* Process SOC display button */
        processSOCDisplayButton();
        
        /* Safety and status monitoring */
        readAndCheckBatteryStatus();
        
        /* Execute MPPT algorithm when in solar charging mode */
        if (gCurrentChargingMode == SOLAR_CHARGING_MODE) {
            if (gMpptCounter >= MPPT_INTERVAL) {
                executeMPPT();
                gMpptCounter = 0;
            }
            gMpptCounter++;
        } else {
            gMpptCounter = 0;
        }
        
        /* Small delay for CPU relief */
        delay_cycles(1000);
    }
    
    return 0;
}

/*
 * Initialize GPIO pins
 */
void initGPIO(void)
{
    /* Initialize SOC LED pins (PA7-PA11) as outputs, initially low */
    DL_GPIO_clearPins(GPIOA, SOC_LED_PINS);
    
    /* Initialize Power MUX select pin (PB14) as output, initially low (USB-C path) */
    DL_GPIO_clearPins(GPIOB, POWER_MUX_SEL_PIN);
}

/*
 * Process TUSB320 interrupt (USB-C device insertion/mode change)
 */
void processUSBC_Interrupt(void)
{
    bool isSource = false;
    bool isSink = false;
    
    /* Read TUSB320 current mode */
    if (readTUSB320Mode(&isSource, &isSink)) {
        if (isSource) {
            /* We are source - Connected device is drawing power */
            
            /* Set power path to USB-C (PB14 low) */
            DL_GPIO_clearPins(GPIOB, POWER_MUX_SEL_PIN);
            
            /* Configure for USB-C source mode (5V/2.4A output) */
            I2C_SetTargetAddress(I2C_BQCHARGER_ADDR);
            BQ25622_ConfigureForUSBC();
            
            gCurrentChargingMode = USB_C_SOURCE_MODE;
            
        } else if (isSink) {
            /* We are sink - Device is being charged from USB C */
            
            /* Set power path to USB-C (PB14 low) */
            DL_GPIO_clearPins(GPIOB, POWER_MUX_SEL_PIN);
            
            /* Configure for charging our battery */
            I2C_SetTargetAddress(I2C_BQCHARGER_ADDR);
            BQ25622_EnableOTG(false);
            BQ25622_EnableCharging(true);
            
            gCurrentChargingMode = USB_C_SINK_MODE;
        }
    }
}

/*
 * Read TUSB320 current mode register
 */
bool readTUSB320Mode(bool *isSource, bool *isSink)
{
    uint8_t modeReg[2] = {0};
    
    /* Set I2C address to TUSB320 */
    I2C_SetTargetAddress(TUSB320_I2C_ADDR);
    
    /* Read current mode register */
    I2C_ReadReg(TUSB320_REG_CURRENT_MODE, modeReg, 1);
    
    /* Check if we are source (DFP) or sink (UFP) */
    *isSource = (modeReg[0] & TUSB320_DFP_MASK) != 0;
    *isSink = (modeReg[0] & TUSB320_UFP_MASK) != 0;
    
    return true;
}

/*
 * Process USB-C Power Good pin
 */
void processUSBC_PowerGood(void)
{
    /* Check if USB-C input power is good */
    if (DL_GPIO_readPins(GPIOB, USB_C_PG_PIN)) {
        /* Only switch to USB-C charging if we're not already in that mode */
        if (gCurrentChargingMode != USB_C_SINK_MODE) {
            /* Set power path to USB-C (PB14 low) */
            DL_GPIO_clearPins(GPIOB, POWER_MUX_SEL_PIN);
            
            /* Configure for charging from USB-C */
            I2C_SetTargetAddress(I2C_BQCHARGER_ADDR);
            BQ25622_EnableOTG(false);
            BQ25622_EnableCharging(true);
            
            gCurrentChargingMode = USB_C_SINK_MODE;
        }
        
        /* Check if battery is already fully charged */
        I2C_SetTargetAddress(I2C_BQGAUGE_ADDR);
        uint16_t soc = BQ28Z620_ReadRelativeSOC();
        
        if (soc >= FULL_CHARGE_SOC) {
            /* Battery is fully charged, stop charging */
            I2C_SetTargetAddress(I2C_BQCHARGER_ADDR);
            BQ25622_EnableCharging(false);
        } else {
            /* Check if charging current is too low (almost done charging) */
            I2C_SetTargetAddress(I2C_BQGAUGE_ADDR);
            int16_t current = BQ28Z620_ReadCurrent();
            
            if (current > 0 && current < MIN_CHARGING_CURRENT) {
                /* Current is too low, charging nearly complete */
                I2C_SetTargetAddress(I2C_BQCHARGER_ADDR);
                BQ25622_EnableCharging(false);
            }
        }
    }
}

/*
 * Process Solar Power Good pin
 */
void processSolarPowerGood(void)
{
    /* Check if solar input power is good */
    if (DL_GPIO_readPins(GPIOB, SOLAR_PG_PIN)) {
        /* Only switch to solar charging if we're not already in USB-C mode
         * USB-C charging takes precedence over solar charging */
        if (gCurrentChargingMode != USB_C_SINK_MODE && gCurrentChargingMode != USB_C_SOURCE_MODE) {
            /* Set power path to solar (PB14 high) */
            DL_GPIO_setPins(GPIOB, POWER_MUX_SEL_PIN);
            
            /* Configure for charging from solar */
            I2C_SetTargetAddress(I2C_BQCHARGER_ADDR);
            BQ25622_EnableOTG(false);
            
            /* Start with a conservative charging current for MPPT */
            BQ25622_SetChargeCurrent(500);
            gCurrentChargeCurrent = 500;
            
            /* Enable charging */
            BQ25622_EnableCharging(true);
            
            gCurrentChargingMode = SOLAR_CHARGING_MODE;
        }
        
        /* Check if battery is already fully charged */
        I2C_SetTargetAddress(I2C_BQGAUGE_ADDR);
        uint16_t soc = BQ28Z620_ReadRelativeSOC();
        
        if (soc >= FULL_CHARGE_SOC) {
            /* Battery is fully charged, stop charging */
            I2C_SetTargetAddress(I2C_BQCHARGER_ADDR);
            BQ25622_EnableCharging(false);
            gCurrentChargingMode = CHARGING_OFF;
        }
    } else if (gCurrentChargingMode == SOLAR_CHARGING_MODE) {
        /* Solar power is not good anymore, stop charging */
        I2C_SetTargetAddress(I2C_BQCHARGER_ADDR);
        BQ25622_EnableCharging(false);
        gCurrentChargingMode = CHARGING_OFF;
    }
}

/*
 * Check if battery charging is safe
 */
bool isBatteryChargingSafe(void)
{
    I2C_SetTargetAddress(I2C_BQGAUGE_ADDR);
    BQ28Z620_BatteryStatus status = BQ28Z620_GetBatteryStatus();
    
    /* Check if charging power exceeds max power threshold */
    float chargingPowerW = (float)status.voltage * (float)status.current / 1000.0f;
    if (chargingPowerW > MAX_CHARGING_POWER_W) {
        return false;
    }
    
    /* Check if battery temperature exceeds threshold */
    if (status.temperature > MAX_BATTERY_TEMP_C) {
        return false;
    }
    
    /* Check battery safety flags */
    /* The specific flags depend on your battery gauge configuration */
    uint16_t flags = status.flags;
    if (flags & 0x0010) { /* Example: Check for over-temperature flag */
        return false;
    }
    
    /* All checks passed */
    return true;
}

/*
 * Read and check battery status for safety
 */
void readAndCheckBatteryStatus(void)
{
    /* Only check if we're actually charging */
    if (gCurrentChargingMode != CHARGING_OFF) {
        if (!isBatteryChargingSafe()) {
            stopCharging();
        }
    }
}

/*
 * Stop charging immediately
 */
void stopCharging(void)
{
    I2C_SetTargetAddress(I2C_BQCHARGER_ADDR);
    BQ25622_EnableCharging(false);
    BQ25622_EnableOTG(false);
    gCurrentChargingMode = CHARGING_OFF;
}

/*
 * Process SOC Display Button (PA0) - show battery level on LEDs
 */
void processSOCDisplayButton(void)
{
    /* Read current button state */
    bool currentButtonState = DL_GPIO_readPins(GPIOA, SOC_DISPLAY_BUTTON_PIN);
    
    /* Check for rising edge (button press) */
    if (currentButtonState && !gPreviousButtonState) {
        /* Read battery SOC from gauge */
        I2C_SetTargetAddress(I2C_BQGAUGE_ADDR);
        uint16_t soc = BQ28Z620_ReadRelativeSOC();
        
        /* Display SOC on LEDs */
        setSOCLEDDisplay(soc);
        
        /* Keep LEDs on for display duration */
        delay_cycles(LED_DISPLAY_DURATION);
        
        /* Turn off all LEDs */
        DL_GPIO_clearPins(GPIOA, SOC_LED_PINS);
    }
    
    /* Update previous button state */
    gPreviousButtonState = currentButtonState;
}

/*
 * Set SOC LED display based on battery level
 */
void setSOCLEDDisplay(uint16_t soc)
{
    /* Clear all LEDs first */
    DL_GPIO_clearPins(GPIOA, SOC_LED_PINS);
    
    /* Set LEDs based on SOC level */
    if (soc <= 20) {
        /* 0-20%: Only turn on PA11 */
        DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_11);
    } else if (soc <= 40) {
        /* 21-40%: Turn on PA11 and PA10 */
        DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_11 | DL_GPIO_PIN_10);
    } else if (soc <= 60) {
        /* 41-60%: Turn on PA11, PA10, and PA9 */
        DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_11 | DL_GPIO_PIN_10 | DL_GPIO_PIN_9);
    } else if (soc <= 80) {
        /* 61-80%: Turn on PA11, PA10, PA9, and PA8 */
        DL_GPIO_setPins(GPIOA, DL_GPIO_PIN_11 | DL_GPIO_PIN_10 | DL_GPIO_PIN_9 | DL_GPIO_PIN_8);
    } else {
        /* 81-100%: Turn on all LEDs */
        DL_GPIO_setPins(GPIOA, SOC_LED_PINS);
    }
}

/*
 * Execute Maximum Power Point Tracking algorithm for solar charging
 * Uses Perturb & Observe method to find maximum power point
 */
void executeMPPT(void)
{
    /* Only execute MPPT if we're in solar charging mode */
    if (gCurrentChargingMode != SOLAR_CHARGING_MODE) {
        return;
    }
    
    /* Read battery voltage and current */
    I2C_SetTargetAddress(I2C_BQGAUGE_ADDR);
    int16_t voltage = BQ28Z620_ReadVoltage();
    int16_t current = BQ28Z620_ReadCurrent();
    
    /* Calculate current power */
    uint16_t currentPower = (uint16_t)((uint32_t)voltage * (uint32_t)current / 1000);
    
    /* Perturb and Observe algorithm */
    if (currentPower > gPreviousPower) {
        /* We're going in the right direction */
        if (gIncreasingCurrent) {
            /* Increase charge current further */
            gCurrentChargeCurrent += MPPT_PERTURB_STEP;
        } else {
            /* Decrease charge current further */
            gCurrentChargeCurrent -= MPPT_PERTURB_STEP;
        }
    } else {
        /* We're going in the wrong direction, reverse */
        if (gIncreasingCurrent) {
            /* Switch to decreasing current */
            gCurrentChargeCurrent -= MPPT_PERTURB_STEP;
            gIncreasingCurrent = false;
        } else {
            /* Switch to increasing current */
            gCurrentChargeCurrent += MPPT_PERTURB_STEP;
            gIncreasingCurrent = true;
        }
    }
    
    /* Apply limits to current */
    if (gCurrentChargeCurrent < 100) {
        gCurrentChargeCurrent = 100;
    } else if (gCurrentChargeCurrent > 3000) {
        gCurrentChargeCurrent = 3000;
    }
    
    /* Set new charge current */
    I2C_SetTargetAddress(I2C_BQCHARGER_ADDR);
    BQ25622_SetChargeCurrent(gCurrentChargeCurrent);
    
    /* Store current power for next comparison */
    gPreviousPower = currentPower;
}

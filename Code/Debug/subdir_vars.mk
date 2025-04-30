################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
SYSCFG_SRCS += \
../empty_mspm0g3507.syscfg 

C_SRCS += \
../I2C_communication.c \
../bq25622_driver.c \
../bq25622_usage_example.c \
./syscfg/ti_msp_dl_config.c \
C:/ti/mspm0_sdk_2_04_00_06/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c 

GEN_CMDS += \
./syscfg/device_linker.cmd 

GEN_FILES += \
./syscfg/device_linker.cmd \
./syscfg/device.opt \
./syscfg/ti_msp_dl_config.c 

GEN_MISC_DIRS += \
./syscfg 

C_DEPS += \
./I2C_communication.d \
./bq25622_driver.d \
./bq25622_usage_example.d \
./syscfg/ti_msp_dl_config.d \
./startup_mspm0g350x_ticlang.d 

GEN_OPTS += \
./syscfg/device.opt 

OBJS += \
./I2C_communication.o \
./bq25622_driver.o \
./bq25622_usage_example.o \
./syscfg/ti_msp_dl_config.o \
./startup_mspm0g350x_ticlang.o 

GEN_MISC_FILES += \
./syscfg/device.cmd.genlibs \
./syscfg/ti_msp_dl_config.h \
./syscfg/Event.dot 

GEN_MISC_DIRS__QUOTED += \
"syscfg" 

OBJS__QUOTED += \
"I2C_communication.o" \
"bq25622_driver.o" \
"bq25622_usage_example.o" \
"syscfg\ti_msp_dl_config.o" \
"startup_mspm0g350x_ticlang.o" 

GEN_MISC_FILES__QUOTED += \
"syscfg\device.cmd.genlibs" \
"syscfg\ti_msp_dl_config.h" \
"syscfg\Event.dot" 

C_DEPS__QUOTED += \
"I2C_communication.d" \
"bq25622_driver.d" \
"bq25622_usage_example.d" \
"syscfg\ti_msp_dl_config.d" \
"startup_mspm0g350x_ticlang.d" 

GEN_FILES__QUOTED += \
"syscfg\device_linker.cmd" \
"syscfg\device.opt" \
"syscfg\ti_msp_dl_config.c" 

C_SRCS__QUOTED += \
"../I2C_communication.c" \
"../bq25622_driver.c" \
"../bq25622_usage_example.c" \
"./syscfg/ti_msp_dl_config.c" \
"C:/ti/mspm0_sdk_2_04_00_06/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c" 

SYSCFG_SRCS__QUOTED += \
"../empty_mspm0g3507.syscfg" 



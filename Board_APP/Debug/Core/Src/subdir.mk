################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/actuators.c \
../Core/Src/buzzer.c \
../Core/Src/crc16.c \
../Core/Src/devices.c \
../Core/Src/freertos.c \
../Core/Src/globals.c \
../Core/Src/i2c.c \
../Core/Src/keys.c \
../Core/Src/leds.c \
../Core/Src/main.c \
../Core/Src/panel_serial.c \
../Core/Src/phases.c \
../Core/Src/pid.c \
../Core/Src/print.c \
../Core/Src/printer.c \
../Core/Src/programs.c \
../Core/Src/sensors.c \
../Core/Src/serial.c \
../Core/Src/settings.c \
../Core/Src/stm32f1xx_hal_msp.c \
../Core/Src/stm32f1xx_hal_timebase_tim.c \
../Core/Src/stm32f1xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system.c \
../Core/Src/system_stm32f1xx.c \
../Core/Src/time.c 

OBJS += \
./Core/Src/actuators.o \
./Core/Src/buzzer.o \
./Core/Src/crc16.o \
./Core/Src/devices.o \
./Core/Src/freertos.o \
./Core/Src/globals.o \
./Core/Src/i2c.o \
./Core/Src/keys.o \
./Core/Src/leds.o \
./Core/Src/main.o \
./Core/Src/panel_serial.o \
./Core/Src/phases.o \
./Core/Src/pid.o \
./Core/Src/print.o \
./Core/Src/printer.o \
./Core/Src/programs.o \
./Core/Src/sensors.o \
./Core/Src/serial.o \
./Core/Src/settings.o \
./Core/Src/stm32f1xx_hal_msp.o \
./Core/Src/stm32f1xx_hal_timebase_tim.o \
./Core/Src/stm32f1xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system.o \
./Core/Src/system_stm32f1xx.o \
./Core/Src/time.o 

C_DEPS += \
./Core/Src/actuators.d \
./Core/Src/buzzer.d \
./Core/Src/crc16.d \
./Core/Src/devices.d \
./Core/Src/freertos.d \
./Core/Src/globals.d \
./Core/Src/i2c.d \
./Core/Src/keys.d \
./Core/Src/leds.d \
./Core/Src/main.d \
./Core/Src/panel_serial.d \
./Core/Src/phases.d \
./Core/Src/pid.d \
./Core/Src/print.d \
./Core/Src/printer.d \
./Core/Src/programs.d \
./Core/Src/sensors.d \
./Core/Src/serial.d \
./Core/Src/settings.d \
./Core/Src/stm32f1xx_hal_msp.d \
./Core/Src/stm32f1xx_hal_timebase_tim.d \
./Core/Src/stm32f1xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system.d \
./Core/Src/system_stm32f1xx.d \
./Core/Src/time.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F105xC -c -I../Core/Inc -I../Core/Devs -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I"D:/Galbiati/Prima Firmware/Source/DB_Protocol" -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/Third_Party/FatFs/src/drivers -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/actuators.d ./Core/Src/actuators.o ./Core/Src/actuators.su ./Core/Src/buzzer.d ./Core/Src/buzzer.o ./Core/Src/buzzer.su ./Core/Src/crc16.d ./Core/Src/crc16.o ./Core/Src/crc16.su ./Core/Src/devices.d ./Core/Src/devices.o ./Core/Src/devices.su ./Core/Src/freertos.d ./Core/Src/freertos.o ./Core/Src/freertos.su ./Core/Src/globals.d ./Core/Src/globals.o ./Core/Src/globals.su ./Core/Src/i2c.d ./Core/Src/i2c.o ./Core/Src/i2c.su ./Core/Src/keys.d ./Core/Src/keys.o ./Core/Src/keys.su ./Core/Src/leds.d ./Core/Src/leds.o ./Core/Src/leds.su ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/panel_serial.d ./Core/Src/panel_serial.o ./Core/Src/panel_serial.su ./Core/Src/phases.d ./Core/Src/phases.o ./Core/Src/phases.su ./Core/Src/pid.d ./Core/Src/pid.o ./Core/Src/pid.su ./Core/Src/print.d ./Core/Src/print.o ./Core/Src/print.su ./Core/Src/printer.d ./Core/Src/printer.o ./Core/Src/printer.su ./Core/Src/programs.d ./Core/Src/programs.o ./Core/Src/programs.su ./Core/Src/sensors.d ./Core/Src/sensors.o ./Core/Src/sensors.su ./Core/Src/serial.d ./Core/Src/serial.o ./Core/Src/serial.su ./Core/Src/settings.d ./Core/Src/settings.o ./Core/Src/settings.su ./Core/Src/stm32f1xx_hal_msp.d ./Core/Src/stm32f1xx_hal_msp.o ./Core/Src/stm32f1xx_hal_msp.su ./Core/Src/stm32f1xx_hal_timebase_tim.d ./Core/Src/stm32f1xx_hal_timebase_tim.o ./Core/Src/stm32f1xx_hal_timebase_tim.su ./Core/Src/stm32f1xx_it.d ./Core/Src/stm32f1xx_it.o ./Core/Src/stm32f1xx_it.su ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system.d ./Core/Src/system.o ./Core/Src/system.su ./Core/Src/system_stm32f1xx.d ./Core/Src/system_stm32f1xx.o ./Core/Src/system_stm32f1xx.su ./Core/Src/time.d ./Core/Src/time.o ./Core/Src/time.su

.PHONY: clean-Core-2f-Src


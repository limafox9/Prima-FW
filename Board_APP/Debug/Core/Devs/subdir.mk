################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Devs/24Cxx.c \
../Core/Devs/7705.c \
../Core/Devs/AD7705.c \
../Core/Devs/BAT2U.c \
../Core/Devs/BMP280.c \
../Core/Devs/PCF8563.c \
../Core/Devs/TM7705.c 

OBJS += \
./Core/Devs/24Cxx.o \
./Core/Devs/7705.o \
./Core/Devs/AD7705.o \
./Core/Devs/BAT2U.o \
./Core/Devs/BMP280.o \
./Core/Devs/PCF8563.o \
./Core/Devs/TM7705.o 

C_DEPS += \
./Core/Devs/24Cxx.d \
./Core/Devs/7705.d \
./Core/Devs/AD7705.d \
./Core/Devs/BAT2U.d \
./Core/Devs/BMP280.d \
./Core/Devs/PCF8563.d \
./Core/Devs/TM7705.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Devs/%.o Core/Devs/%.su Core/Devs/%.cyclo: ../Core/Devs/%.c Core/Devs/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F105xC -c -I../Core/Inc -I../Core/Devs -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I"D:/Galbiati/Prima-FW/DB_Protocol" -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/Third_Party/FatFs/src/drivers -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Devs

clean-Core-2f-Devs:
	-$(RM) ./Core/Devs/24Cxx.cyclo ./Core/Devs/24Cxx.d ./Core/Devs/24Cxx.o ./Core/Devs/24Cxx.su ./Core/Devs/7705.cyclo ./Core/Devs/7705.d ./Core/Devs/7705.o ./Core/Devs/7705.su ./Core/Devs/AD7705.cyclo ./Core/Devs/AD7705.d ./Core/Devs/AD7705.o ./Core/Devs/AD7705.su ./Core/Devs/BAT2U.cyclo ./Core/Devs/BAT2U.d ./Core/Devs/BAT2U.o ./Core/Devs/BAT2U.su ./Core/Devs/BMP280.cyclo ./Core/Devs/BMP280.d ./Core/Devs/BMP280.o ./Core/Devs/BMP280.su ./Core/Devs/PCF8563.cyclo ./Core/Devs/PCF8563.d ./Core/Devs/PCF8563.o ./Core/Devs/PCF8563.su ./Core/Devs/TM7705.cyclo ./Core/Devs/TM7705.d ./Core/Devs/TM7705.o ./Core/Devs/TM7705.su

.PHONY: clean-Core-2f-Devs


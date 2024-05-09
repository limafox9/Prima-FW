################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Devs/W5500/W5500_spi.c \
../Core/Devs/W5500/dhcp.c \
../Core/Devs/W5500/dns.c \
../Core/Devs/W5500/loopback.c \
../Core/Devs/W5500/socket.c \
../Core/Devs/W5500/wizchip_conf.c 

OBJS += \
./Core/Devs/W5500/W5500_spi.o \
./Core/Devs/W5500/dhcp.o \
./Core/Devs/W5500/dns.o \
./Core/Devs/W5500/loopback.o \
./Core/Devs/W5500/socket.o \
./Core/Devs/W5500/wizchip_conf.o 

C_DEPS += \
./Core/Devs/W5500/W5500_spi.d \
./Core/Devs/W5500/dhcp.d \
./Core/Devs/W5500/dns.d \
./Core/Devs/W5500/loopback.d \
./Core/Devs/W5500/socket.d \
./Core/Devs/W5500/wizchip_conf.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Devs/W5500/%.o Core/Devs/W5500/%.su Core/Devs/W5500/%.cyclo: ../Core/Devs/W5500/%.c Core/Devs/W5500/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F105xC -c -I../Core/Inc -I../Core/Devs -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I"D:/Galbiati/Prima-FW/DB_Protocol" -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/Third_Party/FatFs/src/drivers -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -I"D:/Galbiati/Prima-FW/Board_APP/Core/Devs/W5500" -I"D:/Galbiati/Prima-FW/Board_APP/Core/Devs/W5500/W5500" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Devs-2f-W5500

clean-Core-2f-Devs-2f-W5500:
	-$(RM) ./Core/Devs/W5500/W5500_spi.cyclo ./Core/Devs/W5500/W5500_spi.d ./Core/Devs/W5500/W5500_spi.o ./Core/Devs/W5500/W5500_spi.su ./Core/Devs/W5500/dhcp.cyclo ./Core/Devs/W5500/dhcp.d ./Core/Devs/W5500/dhcp.o ./Core/Devs/W5500/dhcp.su ./Core/Devs/W5500/dns.cyclo ./Core/Devs/W5500/dns.d ./Core/Devs/W5500/dns.o ./Core/Devs/W5500/dns.su ./Core/Devs/W5500/loopback.cyclo ./Core/Devs/W5500/loopback.d ./Core/Devs/W5500/loopback.o ./Core/Devs/W5500/loopback.su ./Core/Devs/W5500/socket.cyclo ./Core/Devs/W5500/socket.d ./Core/Devs/W5500/socket.o ./Core/Devs/W5500/socket.su ./Core/Devs/W5500/wizchip_conf.cyclo ./Core/Devs/W5500/wizchip_conf.d ./Core/Devs/W5500/wizchip_conf.o ./Core/Devs/W5500/wizchip_conf.su

.PHONY: clean-Core-2f-Devs-2f-W5500


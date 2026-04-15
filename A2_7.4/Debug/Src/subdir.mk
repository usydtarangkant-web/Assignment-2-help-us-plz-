################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/compass.c \
../Src/compass_led.c \
../Src/main.c 

OBJS += \
./Src/compass.o \
./Src/compass_led.o \
./Src/main.o 

C_DEPS += \
./Src/compass.d \
./Src/compass_led.d \
./Src/main.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F303VCTx -DSTM32 -DSTM32F3 -DSTM32F3DISCOVERY -c -I../Inc -I"C:/Users/Jennifer/Downloads/stm32cubef3/STM32Cube_FW_F3_V1.11.0/Drivers/CMSIS/Device/ST/STM32F3xx/Include" -I"C:/Users/Jennifer/Downloads/stm32cubef3/STM32Cube_FW_F3_V1.11.0/Drivers/CMSIS/Include" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/compass.cyclo ./Src/compass.d ./Src/compass.o ./Src/compass.su ./Src/compass_led.cyclo ./Src/compass_led.d ./Src/compass_led.o ./Src/compass_led.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su

.PHONY: clean-Src


################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/led.c \
../Src/main.c \
../Src/pwm.c \
../Src/serial.c \
../Src/timer.c 

OBJS += \
./Src/led.o \
./Src/main.o \
./Src/pwm.o \
./Src/serial.o \
./Src/timer.o 

C_DEPS += \
./Src/led.d \
./Src/main.d \
./Src/pwm.d \
./Src/serial.d \
./Src/timer.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F303VCTx -DSTM32 -DSTM32F3 -DSTM32F3DISCOVERY -c -I../Inc -I/Users/tarangkant/Desktop/ASSIGNMENT2/stm32f303-definitions/Core/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/led.cyclo ./Src/led.d ./Src/led.o ./Src/led.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/pwm.cyclo ./Src/pwm.d ./Src/pwm.o ./Src/pwm.su ./Src/serial.cyclo ./Src/serial.d ./Src/serial.o ./Src/serial.su ./Src/timer.cyclo ./Src/timer.d ./Src/timer.o ./Src/timer.su

.PHONY: clean-Src


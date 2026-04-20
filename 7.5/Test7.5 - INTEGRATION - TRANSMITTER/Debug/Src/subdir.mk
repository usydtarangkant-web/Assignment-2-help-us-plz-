################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/button.c \
../Src/compass.c \
../Src/main.c \
../Src/north_ref.c \
../Src/serial.c 

OBJS += \
./Src/button.o \
./Src/compass.o \
./Src/main.o \
./Src/north_ref.o \
./Src/serial.o 

C_DEPS += \
./Src/button.d \
./Src/compass.d \
./Src/main.d \
./Src/north_ref.d \
./Src/serial.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F303VCTx -DSTM32 -DSTM32F3 -DSTM32F3DISCOVERY -c -I../Inc -I/Users/tarangkant/Desktop/ASSIGNMENT2/stm32f303-definitions/Core/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/button.cyclo ./Src/button.d ./Src/button.o ./Src/button.su ./Src/compass.cyclo ./Src/compass.d ./Src/compass.o ./Src/compass.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/north_ref.cyclo ./Src/north_ref.d ./Src/north_ref.o ./Src/north_ref.su ./Src/serial.cyclo ./Src/serial.d ./Src/serial.o ./Src/serial.su

.PHONY: clean-Src


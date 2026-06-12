################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/App.c \
../Core/Src/Mode_Select.c \
../Core/Src/State_Machine.c \
../Core/Src/Stepper_Motion.c \
../Core/Src/TMC_Drivers.c \
../Core/Src/audio_processing.c \
../Core/Src/audio_system.c \
../Core/Src/live_harmonizer.c \
../Core/Src/main.c \
../Core/Src/pressure_system.c \
../Core/Src/servo_system.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c 

OBJS += \
./Core/Src/App.o \
./Core/Src/Mode_Select.o \
./Core/Src/State_Machine.o \
./Core/Src/Stepper_Motion.o \
./Core/Src/TMC_Drivers.o \
./Core/Src/audio_processing.o \
./Core/Src/audio_system.o \
./Core/Src/live_harmonizer.o \
./Core/Src/main.o \
./Core/Src/pressure_system.o \
./Core/Src/servo_system.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o 

C_DEPS += \
./Core/Src/App.d \
./Core/Src/Mode_Select.d \
./Core/Src/State_Machine.d \
./Core/Src/Stepper_Motion.d \
./Core/Src/TMC_Drivers.d \
./Core/Src/audio_processing.d \
./Core/Src/audio_system.d \
./Core/Src/live_harmonizer.d \
./Core/Src/main.d \
./Core/Src/pressure_system.d \
./Core/Src/servo_system.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/App.cyclo ./Core/Src/App.d ./Core/Src/App.o ./Core/Src/App.su ./Core/Src/Mode_Select.cyclo ./Core/Src/Mode_Select.d ./Core/Src/Mode_Select.o ./Core/Src/Mode_Select.su ./Core/Src/State_Machine.cyclo ./Core/Src/State_Machine.d ./Core/Src/State_Machine.o ./Core/Src/State_Machine.su ./Core/Src/Stepper_Motion.cyclo ./Core/Src/Stepper_Motion.d ./Core/Src/Stepper_Motion.o ./Core/Src/Stepper_Motion.su ./Core/Src/TMC_Drivers.cyclo ./Core/Src/TMC_Drivers.d ./Core/Src/TMC_Drivers.o ./Core/Src/TMC_Drivers.su ./Core/Src/audio_processing.cyclo ./Core/Src/audio_processing.d ./Core/Src/audio_processing.o ./Core/Src/audio_processing.su ./Core/Src/audio_system.cyclo ./Core/Src/audio_system.d ./Core/Src/audio_system.o ./Core/Src/audio_system.su ./Core/Src/live_harmonizer.cyclo ./Core/Src/live_harmonizer.d ./Core/Src/live_harmonizer.o ./Core/Src/live_harmonizer.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/pressure_system.cyclo ./Core/Src/pressure_system.d ./Core/Src/pressure_system.o ./Core/Src/pressure_system.su ./Core/Src/servo_system.cyclo ./Core/Src/servo_system.d ./Core/Src/servo_system.o ./Core/Src/servo_system.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su

.PHONY: clean-Core-2f-Src


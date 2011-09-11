################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../disk.c \
../fat.c \
../fatAddressing.c \
../fatHelpers.c 

OBJS += \
./disk.o \
./fat.o \
./fatAddressing.o \
./fatHelpers.o 

C_DEPS += \
./disk.d \
./fat.d \
./fatAddressing.d \
./fatHelpers.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0  -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



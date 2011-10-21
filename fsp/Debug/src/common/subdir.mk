################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/common/nipc.c 

OBJS += \
./src/common/nipc.o 

C_DEPS += \
./src/common/nipc.d 


# Each subdirectory must supply rules for building sources it contributes
src/common/%.o: ../src/common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/fuse -O0 -g3 -Wall -c -fmessage-length=0 -g -D_FILE_OFFSET_BITS=64 -licui18n -licuuc -licudata -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



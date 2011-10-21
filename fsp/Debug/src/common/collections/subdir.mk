################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/common/collections/blist.c \
../src/common/collections/list.c 

OBJS += \
./src/common/collections/blist.o \
./src/common/collections/list.o 

C_DEPS += \
./src/common/collections/blist.d \
./src/common/collections/list.d 


# Each subdirectory must supply rules for building sources it contributes
src/common/collections/%.o: ../src/common/collections/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/fuse -O0 -g3 -Wall -c -fmessage-length=0 -g -D_FILE_OFFSET_BITS=64 -licui18n -licuuc -licudata -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



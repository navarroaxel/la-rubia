################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/common/utils/array.c \
../src/common/utils/commands.c \
../src/common/utils/config.c \
../src/common/utils/log.c \
../src/common/utils/queue.c \
../src/common/utils/scanner.c \
../src/common/utils/serializer.c \
../src/common/utils/sockets.c \
../src/common/utils/utils.c 

OBJS += \
./src/common/utils/array.o \
./src/common/utils/commands.o \
./src/common/utils/config.o \
./src/common/utils/log.o \
./src/common/utils/queue.o \
./src/common/utils/scanner.o \
./src/common/utils/serializer.o \
./src/common/utils/sockets.o \
./src/common/utils/utils.o 

C_DEPS += \
./src/common/utils/array.d \
./src/common/utils/commands.d \
./src/common/utils/config.d \
./src/common/utils/log.d \
./src/common/utils/queue.d \
./src/common/utils/scanner.d \
./src/common/utils/serializer.d \
./src/common/utils/sockets.d \
./src/common/utils/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/common/utils/%.o: ../src/common/utils/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/fuse -O0 -g3 -Wall -c -fmessage-length=0 -g -D_FILE_OFFSET_BITS=64 -licui18n -licuuc -licudata -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



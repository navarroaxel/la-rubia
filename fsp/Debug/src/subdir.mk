################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cache.c \
../src/console.c \
../src/disk.c \
../src/fat.c \
../src/fatAddressing.c \
../src/fatHelpers.c \
../src/fuse.c 

OBJS += \
./src/cache.o \
./src/console.o \
./src/disk.o \
./src/fat.o \
./src/fatAddressing.o \
./src/fatHelpers.o \
./src/fuse.o 

C_DEPS += \
./src/cache.d \
./src/console.d \
./src/disk.d \
./src/fat.d \
./src/fatAddressing.d \
./src/fatHelpers.d \
./src/fuse.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/fuse -O0 -g3 -Wall -c -fmessage-length=0 -g -D_FILE_OFFSET_BITS=64 -licui18n -licuuc -licudata -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../libmodbus/modbus-rtu.cpp \
../libmodbus/modbus-tcp.cpp \
../libmodbus/modbus.cpp 

OBJS += \
./libmodbus/modbus-rtu.o \
./libmodbus/modbus-tcp.o \
./libmodbus/modbus.o 

CPP_DEPS += \
./libmodbus/modbus-rtu.d \
./libmodbus/modbus-tcp.d \
./libmodbus/modbus.d 


# Each subdirectory must supply rules for building sources it contributes
libmodbus/%.o: ../libmodbus/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



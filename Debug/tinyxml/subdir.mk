################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../tinyxml/tinyxml2.cpp 

OBJS += \
./tinyxml/tinyxml2.o 

CPP_DEPS += \
./tinyxml/tinyxml2.d 


# Each subdirectory must supply rules for building sources it contributes
tinyxml/%.o: ../tinyxml/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



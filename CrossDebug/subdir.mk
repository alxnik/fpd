################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CBlueTooth.cpp \
../CFroniusProbe.cpp \
../CLog.cpp \
../CModbusMTU.cpp \
../CMysqlDb.cpp \
../CScanner.cpp \
../CSerial.cpp \
../CSmaProbe.cpp \
../CSocket.cpp \
../CSolutronicProbe.cpp \
../CSunergyProbe.cpp \
../CXMLDb.cpp \
../fpd.cpp 

OBJS += \
./CBlueTooth.o \
./CFroniusProbe.o \
./CLog.o \
./CModbusMTU.o \
./CMysqlDb.o \
./CScanner.o \
./CSerial.o \
./CSmaProbe.o \
./CSocket.o \
./CSolutronicProbe.o \
./CSunergyProbe.o \
./CXMLDb.o \
./fpd.o 

CPP_DEPS += \
./CBlueTooth.d \
./CFroniusProbe.d \
./CLog.d \
./CModbusMTU.d \
./CMysqlDb.d \
./CScanner.d \
./CSerial.d \
./CSmaProbe.d \
./CSocket.d \
./CSolutronicProbe.d \
./CSunergyProbe.d \
./CXMLDb.d \
./fpd.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



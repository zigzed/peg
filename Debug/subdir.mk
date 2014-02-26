################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../context.cpp \
../expr.cpp \
../parser.cpp \
../rule.cpp \
../scanner.cpp \
../sqlgrammar.cpp \
../testpeg.cpp 

OBJS += \
./context.o \
./expr.o \
./parser.o \
./rule.o \
./scanner.o \
./sqlgrammar.o \
./testpeg.o 

CPP_DEPS += \
./context.d \
./expr.d \
./parser.d \
./rule.d \
./scanner.d \
./sqlgrammar.d \
./testpeg.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



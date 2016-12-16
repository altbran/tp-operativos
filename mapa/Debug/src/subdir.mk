################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/deadlock.c \
../src/dibujador.c \
../src/funciones.c \
../src/mapa.c \
../src/planificadores.c 

OBJS += \
./src/deadlock.o \
./src/dibujador.o \
./src/funciones.o \
./src/mapa.o \
./src/planificadores.o 

C_DEPS += \
./src/deadlock.d \
./src/dibujador.d \
./src/funciones.d \
./src/mapa.d \
./src/planificadores.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2016-2c-A-cara-de-rope/LibreriasSO" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



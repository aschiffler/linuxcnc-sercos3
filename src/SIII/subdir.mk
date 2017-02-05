################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
./SIII/SIII_CONF.c \
./SIII/SIII_INIT.c \
./SIII/SIII_PHASE.c \
./SIII/SIII_REDUNDANCY.c \
./SIII/SIII_SVC.c \
./SIII/SIII_CYCLIC.c

OBJS += \
./SIII/SIII_CONF.o \
./SIII/SIII_INIT.o \
./SIII/SIII_PHASE.o \
./SIII/SIII_REDUNDANCY.o \
./SIII/SIII_SVC.o \
./SIII/SIII_CYCLIC.o

C_DEPS += \
./SIII/SIII_CONF.d \
./SIII/SIII_INIT.d \
./SIII/SIII_PHASE.d \
./SIII/SIII_REDUNDANCY.d \
./SIII/SIII_SVC.d \
./SIII/SIII_CYCLIC.d


# Each subdirectory must supply rules for building sources it contributes
SIII/SIII_CONF.o: ./SIII/SIII_CONF.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

SIII/SIII_INIT.o: ./SIII/SIII_INIT.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

SIII/SIII_PHASE.o: ./SIII/SIII_PHASE.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

SIII/SIII_REDUNDANCY.o: ./SIII/SIII_REDUNDANCY.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

SIII/SIII_SVC.o: ./SIII/SIII_SVC.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

SIII/SIII_CYCLIC.o: ./SIII/SIII_CYCLIC.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"


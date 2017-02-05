################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
./SICE/SICE_CYCLIC.c \
./SICE/SICE_INIT.c \
./SICE/SICE_NIC_TIMED.c \
./SICE/SICE_RX.c \
./SICE/SICE_SIII.c \
./SICE/SICE_TX.c \
./SICE/SICE_UCC.c \
./SICE/SICE_UTIL.c 

OBJS += \
./SICE/SICE_CYCLIC.o \
./SICE/SICE_INIT.o \
./SICE/SICE_NIC_TIMED.o \
./SICE/SICE_RX.o \
./SICE/SICE_SIII.o \
./SICE/SICE_TX.o \
./SICE/SICE_UCC.o \
./SICE/SICE_UTIL.o 

C_DEPS += \
./SICE/SICE_CYCLIC.d \
./SICE/SICE_INIT.d \
./SICE/SICE_NIC_TIMED.d \
./SICE/SICE_RX.d \
./SICE/SICE_SIII.d \
./SICE/SICE_TX.d \
./SICE/SICE_UCC.d \
./SICE/SICE_UTIL.d 


# Each subdirectory must supply rules for building sources it contributes
SICE/SICE_CYCLIC.o: ./SICE/SICE_CYCLIC.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

SICE/SICE_INIT.o: ./SICE/SICE_INIT.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

SICE/SICE_NIC_TIMED.o: ./SICE/SICE_NIC_TIMED.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

SICE/SICE_RX.o: ./SICE/SICE_RX.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

SICE/SICE_SIII.o: ./SICE/SICE_SIII.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

SICE/SICE_TX.o: ./SICE/SICE_TX.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

SICE/SICE_UCC.o: ./SICE/SICE_UCC.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

SICE/SICE_UTIL.o: ./SICE/SICE_UTIL.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"



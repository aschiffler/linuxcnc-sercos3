################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
./RTLX/RTLX_SOCK.c \
./RTLX/RTLX_SEMA.c \
./RTLX/RTLX_THREAD.c \
./RTLX/RTLX_TIME.c

OBJS += \
./RTLX/RTLX_SOCK.o \
./RTLX/RTLX_SEMA.o \
./RTLX/RTLX_THREAD.o \
./RTLX/RTLX_TIME.o 

C_DEPS += \
./RTLX/RTLX_SOCK.d \
./RTLX/RTLX_SEMA.d \
./RTLX/RTLX_THREAD.d \
./RTLX/RTLX_TIME.d 


# Each subdirectory must supply rules for building sources it contributes
RTLX/RTLX_SOCK.o: ./RTLX/RTLX_SOCK.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

RTLX/RTLX_SEMA.o: ./RTLX/RTLX_SEMA.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

RTLX/RTLX_THREAD.o: ./RTLX/RTLX_THREAD.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

RTLX/RTLX_TIME.o: ./RTLX/RTLX_TIME.c
	$(CC) -O3 -Wall -fPIC -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"



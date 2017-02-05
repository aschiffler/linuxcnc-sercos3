################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
./S3SM/S3SM_LC_MAIN.c \

OBJS += \
./S3SM/S3SM_LC_MAIN.o \

C_DEPS += \
./S3SM/S3SM_LC_MAIN.d \


# Each subdirectory must supply rules for building sources it contributes
S3SM/S3SM_LC_MAIN.o: ./S3SM/S3SM_LC_MAIN.c
	@echo 'Building file: $<'
	@echo 'Invoking: $(CC) C Compiler'
	$(CC) -O3 -Wall -fPIC -c $(EXTRA_CFLAGS) -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


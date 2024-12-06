################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.c \
/Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.c \
/Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.c \
/Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.c \
/Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.c \
/Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.c \
/Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.c \
/Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.c \
/Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.c \
/Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.c 

OBJS += \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.o \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.o 

C_DEPS += \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.d \
./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.o: /Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.c Drivers/BSP/B-U585I-IOT02A/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DSTM32U585xx -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../Drivers/CMSIS/Include -I../../../../../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../../../../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../../../../../Drivers/BSP/B-U585I-IOT02A -I../../../../../../Drivers/BSP/Components/Common -I../../../../../../Drivers/BSP/Components/vl53l5cx/porting -I../../../../../../Drivers/BSP/Components/vl53l5cx/modules -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.o: /Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.c Drivers/BSP/B-U585I-IOT02A/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DSTM32U585xx -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../Drivers/CMSIS/Include -I../../../../../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../../../../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../../../../../Drivers/BSP/B-U585I-IOT02A -I../../../../../../Drivers/BSP/Components/Common -I../../../../../../Drivers/BSP/Components/vl53l5cx/porting -I../../../../../../Drivers/BSP/Components/vl53l5cx/modules -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.o: /Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.c Drivers/BSP/B-U585I-IOT02A/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DSTM32U585xx -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../Drivers/CMSIS/Include -I../../../../../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../../../../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../../../../../Drivers/BSP/B-U585I-IOT02A -I../../../../../../Drivers/BSP/Components/Common -I../../../../../../Drivers/BSP/Components/vl53l5cx/porting -I../../../../../../Drivers/BSP/Components/vl53l5cx/modules -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.o: /Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.c Drivers/BSP/B-U585I-IOT02A/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DSTM32U585xx -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../Drivers/CMSIS/Include -I../../../../../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../../../../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../../../../../Drivers/BSP/B-U585I-IOT02A -I../../../../../../Drivers/BSP/Components/Common -I../../../../../../Drivers/BSP/Components/vl53l5cx/porting -I../../../../../../Drivers/BSP/Components/vl53l5cx/modules -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.o: /Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.c Drivers/BSP/B-U585I-IOT02A/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DSTM32U585xx -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../Drivers/CMSIS/Include -I../../../../../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../../../../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../../../../../Drivers/BSP/B-U585I-IOT02A -I../../../../../../Drivers/BSP/Components/Common -I../../../../../../Drivers/BSP/Components/vl53l5cx/porting -I../../../../../../Drivers/BSP/Components/vl53l5cx/modules -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.o: /Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.c Drivers/BSP/B-U585I-IOT02A/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DSTM32U585xx -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../Drivers/CMSIS/Include -I../../../../../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../../../../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../../../../../Drivers/BSP/B-U585I-IOT02A -I../../../../../../Drivers/BSP/Components/Common -I../../../../../../Drivers/BSP/Components/vl53l5cx/porting -I../../../../../../Drivers/BSP/Components/vl53l5cx/modules -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.o: /Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.c Drivers/BSP/B-U585I-IOT02A/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DSTM32U585xx -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../Drivers/CMSIS/Include -I../../../../../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../../../../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../../../../../Drivers/BSP/B-U585I-IOT02A -I../../../../../../Drivers/BSP/Components/Common -I../../../../../../Drivers/BSP/Components/vl53l5cx/porting -I../../../../../../Drivers/BSP/Components/vl53l5cx/modules -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.o: /Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.c Drivers/BSP/B-U585I-IOT02A/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DSTM32U585xx -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../Drivers/CMSIS/Include -I../../../../../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../../../../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../../../../../Drivers/BSP/B-U585I-IOT02A -I../../../../../../Drivers/BSP/Components/Common -I../../../../../../Drivers/BSP/Components/vl53l5cx/porting -I../../../../../../Drivers/BSP/Components/vl53l5cx/modules -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.o: /Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.c Drivers/BSP/B-U585I-IOT02A/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DSTM32U585xx -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../Drivers/CMSIS/Include -I../../../../../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../../../../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../../../../../Drivers/BSP/B-U585I-IOT02A -I../../../../../../Drivers/BSP/Components/Common -I../../../../../../Drivers/BSP/Components/vl53l5cx/porting -I../../../../../../Drivers/BSP/Components/vl53l5cx/modules -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.o: /Users/vishal/Desktop/Engr1IT/T_8/IoT_8/Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.c Drivers/BSP/B-U585I-IOT02A/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DSTM32U585xx -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../Drivers/CMSIS/Include -I../../../../../../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../../../../../../Drivers/STM32U5xx_HAL_Driver/Inc -I../../../../../../Drivers/BSP/B-U585I-IOT02A -I../../../../../../Drivers/BSP/Components/Common -I../../../../../../Drivers/BSP/Components/vl53l5cx/porting -I../../../../../../Drivers/BSP/Components/vl53l5cx/modules -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-B-2d-U585I-2d-IOT02A

clean-Drivers-2f-BSP-2f-B-2d-U585I-2d-IOT02A:
	-$(RM) ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_audio.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_bus.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_camera.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_eeprom.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_env_sensors.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_light_sensor.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_motion_sensors.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ospi.su ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.cyclo ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.d ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.o ./Drivers/BSP/B-U585I-IOT02A/b_u585i_iot02a_ranging_sensor.su

.PHONY: clean-Drivers-2f-BSP-2f-B-2d-U585I-2d-IOT02A


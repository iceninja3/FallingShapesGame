/*
 * embedded_ML.h
 *
 *  Created on: Dec 5, 2024
 *      Author: vishal
 */
#ifndef EMBEDDED_ML_H
#define EMBEDDED_ML_H

#include "stm32u5xx.h"
#include "stm32u5xx_hal.h"

// Declare your UART functions
void UART1_Init(void);
void UART_SendString(char* str);
void UART_SendChar(uint8_t c);
void UART_ReceiveString(char* buffer, uint16_t bufferSize);

#endif

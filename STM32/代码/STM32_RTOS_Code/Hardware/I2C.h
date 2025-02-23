#ifndef I2C_H
#define I2C_H

#include "stm32f10x.h"
#include "Experiment_Selected.h"

void I2C_Config(void);

void I2C_Soft_Start(void);

void I2C_Soft_End(void);

void I2C_Soft_SendByte(uint8_t byte);

uint8_t I2C_Soft_ReceiveByte(void);

void I2C_Soft_SendAck(uint8_t ackBit);

uint8_t I2C_Soft_ReceiveAck(void);

#endif

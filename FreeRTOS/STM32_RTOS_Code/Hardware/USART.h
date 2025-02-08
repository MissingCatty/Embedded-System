#ifndef USART_H
#define USART_H

#include "stm32f10x.h"
#include "Experiment_Selected.h"

void USART_Config(void);
void USART_SendByte(USART_TypeDef *USARTx, uint8_t byte);
uint8_t USART_ReadByte(USART_TypeDef *USARTx);

#ifdef EXP_USART
    void USART_Generate_And_Send(void);
    uint8_t* USART_Get_Send_Buffer(void);
    uint8_t USART_Get_Send_Buffer_Length(void);
    uint8_t* USART_Get_Receive_Buffer(void);
    uint8_t USART_Get_Receive_Buffer_Length(void);
#endif

#endif

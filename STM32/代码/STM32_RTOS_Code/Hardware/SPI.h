#ifndef SPI_H
#define SPI_H

#include "stm32f10x.h"
#include "Experiment_Selected.h"

void SPI_Config(void);

#ifdef EXP_SPI
    void MASTER_NSS_WRITE(uint8_t bitval);
    void Master_Send_Byte(uint8_t byte);
    uint16_t Slave_Read_Byte(void);
#endif

#endif

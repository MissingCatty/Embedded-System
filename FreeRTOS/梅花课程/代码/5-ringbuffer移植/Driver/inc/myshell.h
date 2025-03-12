#ifndef MYSHELL_H
#define MYSHELL_H

#include "stm32f4xx.h"
#include "shell.h"
#include "usart.h"

extern Shell shell;
extern char shellBuffer[512];

void myshell_init(void);

#endif

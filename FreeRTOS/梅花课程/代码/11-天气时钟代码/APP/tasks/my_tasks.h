#ifndef MY_TASKS_H
#define MY_TASKS_H

#include "lvgl.h"

void foreground_task_create(void);


void background_task_create(void);
void backgroud_queue_put(void (*func)(void *), void *param);

#endif
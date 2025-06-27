void vTaskSuspend(TaskHandle_t xTaskToSuspend)
{
    TCB_t *pxTCB;
    taskENTER_CRITICAL();
    {
        // 如果参数为 NULL 的话说明挂起自身
        pxTCB = prvGetTCBFromHandle(xTaskToSuspend);
        // 将任务从就绪或者延时列表中删除，并且将任务放到挂起列表中
        if (uxListRemove(&(pxTCB->xStateListItem)) == (UBaseType_t)0)
        {
            taskRESET_READY_PRIORITY(pxTCB->uxPriority);
        } else
        {
            mtCOVERAGE_TEST_MARKER();
        }
        // 任务是否还在等待其他事件
        if (listLIST_ITEM_CONTAINER(&(pxTCB->xEventListItem)) != NULL)
            (void)uxListRemove(&(pxTCB->xEventListItem));
    }
    else
    {
        mtCOVERAGE_TEST_MARKER();
    }
    vListInsertEnd(&xSuspendedTaskList, &(pxTCB->xStateListItem));
}
taskEXIT_CRITICAL();
if (xSchedulerRunning != pdFALSE)
{
    taskENTER_CRITICAL();
    {
        prvResetNextTaskUnblockTime();
    }
    taskEXIT_CRITICAL();
} else
{
    mtCOVERAGE_TEST_MARKER();
}
if (pxTCB == pxCurrentTCB)
{
    if (xSchedulerRunning != pdFALSE)
    {
        configASSERT(uxSchedulerSuspended == 0);
        portYIELD_WITHIN_API();
    } else
    {
        if (listCURRENT_LIST_LENGTH(&xSuspendedTaskList) == uxCurrentNumberOfTasks)
        {
            pxCurrentTCB = NULL;
        } else
        {
            vTaskSwitchContext();
        }
    }
} else
{
    mtCOVERAGE_TEST_MARKER();
}
}
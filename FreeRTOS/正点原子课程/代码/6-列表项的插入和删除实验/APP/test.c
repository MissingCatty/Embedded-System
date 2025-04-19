static void prvInitialiseNewTask(TaskFunction_t pxTaskCode, const char *const pcName, const uint32_t ulStackDepth, void *const pvParameters, UBaseType_t uxPriority, TaskHandle_t *const pxCreatedTask, TCB_t *pxNewTCB, const MemoryRegion_t *const xRegions)
{
    StackType_t *pxTopOfStack; // 创建栈顶指针
    UBaseType_t  x;
#if ((configCHECK_FOR_STACK_OVERFLOW > 1) || (configUSE_TRACE_FACILITY == 1) || (INCLUDE_uxTaskGetStackHighWaterMark == 1))
    {
        (void)memset(pxNewTCB->pxStack, (int)tskSTACK_FILL_BYTE, (size_t)ulStackDepth * sizeof(StackType_t)); // (1)
    }
#endif
    pxTopOfStack = pxNewTCB->pxStack + (ulStackDepth - (uint32_t)1); // (2)
    pxTopOfStack = (StackType_t *)(((portPOINTER_SIZE_TYPE)pxTopOfStack) & (~((portPOINTER_SIZE_TYPE)portBYTE_ALIGNMENT_MASK)));
    for (x = (UBaseType_t)0; x < (UBaseType_t)configMAX_TASK_NAME_LEN; x++)
    {
        pxNewTCB->pcTaskName[x] = pcName[x]; // (3)
        if (pcName[x] == 0x00)
        {
            break;
        } else
        {
            mtCOVERAGE_TEST_MARKER();
        }
    }
    pxNewTCB->pcTaskName[configMAX_TASK_NAME_LEN - 1] = '\0'; // (4)
    if (uxPriority >= (UBaseType_t)configMAX_PRIORITIES)      // (5)
    {
        uxPriority = (UBaseType_t)configMAX_PRIORITIES - (UBaseType_t)1U;
    } else
    {
        mtCOVERAGE_TEST_MARKER();
    }
    pxNewTCB->uxPriority = uxPriority; // (6)
#if (configUSE_MUTEXES == 1)           // (7)
    {
        pxNewTCB->uxBasePriority = uxPriority;
        pxNewTCB->uxMutexesHeld  = 0;
    }
#endif                                                /* configUSE_MUTEXES */
    vListInitialiseItem(&(pxNewTCB->xStateListItem)); // (8)

    vListInitialiseItem(&(pxNewTCB->xEventListItem)); // (9)

    listSET_LIST_ITEM_OWNER(&(pxNewTCB->xStateListItem), pxNewTCB); // (10)

    listSET_LIST_ITEM_VALUE(&(pxNewTCB->xEventListItem), (TickType_t)configMAX_PRIORITIES - (TickType_t)uxPriority); // (11)
    listSET_LIST_ITEM_OWNER(&(pxNewTCB->xEventListItem), pxNewTCB);                                                  // (12)
#if (portCRITICAL_NESTING_IN_TCB == 1)                                                                               // 使能临界区嵌套
    {
        pxNewTCB->uxCriticalNesting = (UBaseType_t)0U;
    }
#endif                                    /* portCRITICAL_NESTING_IN_TCB */
#if (configUSE_APPLICATION_TASK_TAG == 1) // 使能任务标签功能
    {
        pxNewTCB->pxTaskTag = NULL;
    }
#endif                                   /* configUSE_APPLICATION_TASK_TAG */
#if (configGENERATE_RUN_TIME_STATS == 1) // 使能时间统计功能
    {
        pxNewTCB->ulRunTimeCounter = 0UL;
    }
#endif /* configGENERATE_RUN_TIME_STATS */
#if (configNUM_THREAD_LOCAL_STORAGE_POINTERS != 0)
    {
        for (x = 0; x < (UBaseType_t)configNUM_THREAD_LOCAL_STORAGE_POINTERS;
             x++)
        {
            pxNewTCB->pvThreadLocalStoragePointers[x] = NULL; // (13)
        }
    }
#endif
#if (configUSE_TASK_NOTIFICATIONS == 1) // 使能任务通知功能
    {
        pxNewTCB->ulNotifiedValue = 0;
        pxNewTCB->ucNotifyState   = taskNOT_WAITING_NOTIFICATION;
    }
#endif
#if (configUSE_NEWLIB_REENTRANT == 1) // 使能 NEWLIB
    {
        _REENT_INIT_PTR((&(pxNewTCB->xNewLib_reent)));
    }
#endif
#if (INCLUDE_xTaskAbortDelay == 1) // 使能函数 xTaskAbortDelay()
    {
        pxNewTCB->ucDelayAborted = pdFALSE;
    }
#endif
    pxNewTCB->pxTopOfStack = pxPortInitialiseStack(pxTopOfStack, pxTaskCode, pvParameters); // (14)
    if ((void *)pxCreatedTask != NULL)
    {
        *pxCreatedTask = (TaskHandle_t)pxNewTCB; // (15)
    } else
    {
        mtCOVERAGE_TEST_MARKER();
    }
}
# 1 概念

## 1.1 列表

列表是 FreeRTOS 中的一个数据结构，概念上和链表有点类似，列表被用来跟踪 FreeRTOS 中的任务。

与列表相关的全部东西都在文件 `list.c` 和 `list.h` 中。在 `list.h` 中定义了一个叫 `List_t` 的结构体，如下：

```c
typedef struct xLIST
{
    listFIRST_LIST_INTEGRITY_CHECK_VALUE
    volatile UBaseType_t uxNumberOfItems;
    ListItem_t * configLIST_VOLATILE pxIndex;
    MiniListItem_t xListEnd;
    listSECOND_LIST_INTEGRITY_CHECK_VALUE
} List_t;
```

- `listFIRST_LIST_INTEGRITY_CHECK_VALUE`和`listSECOND_LIST_INTEGRITY_CHECK_VALUE`：用于**检查列表完整性**，需要将宏 `configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES`设置为 1，设置完成后，这个两个宏被定义为：

  ```c
  #define listFIRST_LIST_INTEGRITY_CHECK_VALUE          TickType_t xListIntegrityValue1;
  #define listSECOND_LIST_INTEGRITY_CHECK_VALUE         TickType_t xListIntegrityValue2;
  ```

  在初始化列表的时候会这两个变量中写入一个特殊的值。

  在 FreeRTOS 操作链表时检查：

  ```c
  configASSERT( pxItem->ucListItemIntegrityValue1 == 特殊值 );
  configASSERT( pxItem->ucListItemIntegrityValue2 == 特殊值 );
  ```

- `uxNumberOfItems`：用来**记录列表中列表项的数量**。
- `pxIndex`：用来**记录当前列表项索引号**，用于遍历列表。  

- `xListEnd`：列表中最后一个列表项，用来表示列表结束，此变量类型为 `MiniListItem_t`，这是一个迷你列表项，关于列表项稍后讲解。

## 1.2 列表项

列表项就是存放在列表中的项目， FreeRTOS 提供了两种列表项： 列表项和迷你列表项。这两个都在文件 `list.h`中有定义，先来看一下列表项，定义如下：

```c
struct xLIST_ITEM
{
    listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE
    configLIST_VOLATILE TickType_t xItemValue;
    struct xLIST_ITEM * configLIST_VOLATILE pxNext;
    struct xLIST_ITEM * configLIST_VOLATILE pxPrevious;
    void * pvOwner;
    struct xLIST * configLIST_VOLATILE pxContainer;
    listSECOND_LIST_ITEM_INTEGRITY_CHECK_VALUE
};
typedef struct xLIST_ITEM ListItem_t; 
```

- `xItemValue`：列表项值 ，它的具体意义取决于这个 `list_item` 被用在什么地方。
- `pxNext`：指向下一个列表项。  
- `pxPrevious`：指向前一个列表项。
- `pvOwner`：记录此**列表项归谁拥有**，通常是任务控制块（该列表项代表的哪个任务）。
- `pvContainer`：记录此列表项归哪个列表。

> 在前面讲解任务控制块 `TCB_t`的时候说了在 `TCB_t`中有两个变量 `xStateListItem`和 `xEventListItem`，这两个变量的类型就是 `ListItem_t`，也就是说这两个成员变量都是列表项。
>
> 以 `xStateListItem`为例，当创建一个任务以后 `xStateListItem`的 `pvOwner` 变量就指向这个任务的任务控制块，表示 `xSateListItem`属于此任务。当任务就绪态以后 `xStateListItem`的变量 `pvContainer`就指向就绪列表，表明此列表项在就绪列表中。
>
> 为什么要在`TCB_t`中放一个列表项的数据类型？因为只有在创建TCB时才能让列表项知道该列表项代表哪个任务。

## 1.3 迷你列表项

在文件 list.h 中有定义，如下：

```c
struct xMINI_LIST_ITEM
{
    listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE
    configLIST_VOLATILE TickType_t xItemValue;
    struct xLIST_ITEM * configLIST_VOLATILE pxNext;
    struct xLIST_ITEM * configLIST_VOLATILE pxPrevious;
};
typedef struct xMINI_LIST_ITEM MiniListItem_t;
```

迷你列表项只是比列表项少了几个成员变量，那为什么要弄个迷你列表项出来呢？

因为有些情况下我们**不需要列表项这么全的功能**，可能只需要其中的某几个成员变量，如果此时用列表项的话会造成内存浪费！  

## 2 列表和列表项操作

```c
void vListInitialise( List_t * const pxList )
{
    pxList->pxIndex = ( ListItem_t * ) &( pxList->xListEnd );

    // 将List_t中的迷你列表的xListItemIntegrityValue1设置为pdINTEGRITY_CHECK_VALUE
    listSET_FIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE( &( pxList->xListEnd ) );

    pxList->xListEnd.xItemValue = portMAX_DELAY;

    pxList->xListEnd.pxNext = ( ListItem_t * ) &( pxList->xListEnd );
    
    pxList->xListEnd.pxPrevious = ( ListItem_t * ) &( pxList->xListEnd );

    pxList->uxNumberOfItems = ( UBaseType_t ) 0U;

    listSET_LIST_INTEGRITY_CHECK_1_VALUE( pxList );
    listSET_LIST_INTEGRITY_CHECK_2_VALUE( pxList );
}
```

